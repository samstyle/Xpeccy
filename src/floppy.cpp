#include <fstream>
#include <string.h>

#include "common.h"
#include "floppy.h"
#include "filetypes/filetypes.h"

struct Floppy {
	int flag;
	uint8_t id;
	uint8_t iback;
	uint8_t trk,rtrk;
	uint8_t field;
	int32_t pos;
	uint32_t ti;
	std::string path;
	struct {
		uint8_t byte[TRACKLEN];
		uint8_t field[TRACKLEN];
	} data[256];
};

uint8_t trd_8e1[] = {
	0x00,0x00,0x01,0x16,0x00,0xf0,0x09,0x10,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00
};

Floppy* flpCreate(int id) {
	Floppy* flp = new Floppy;
	flp->id = id;
	flp->flag = FLP_TRK80 | FLP_DS;
	flp->trk = 0;
	flp->rtrk = 0;
	flp->pos = 0;
	return flp;
}

void flpDestroy(Floppy* flp) {
	delete(flp);
}

void flpWr(Floppy* flp,uint8_t val) {
	flp->data[flp->rtrk].byte[flp->pos] = val;
	flp->flag |= FLP_CHANGED;
}

uint8_t flpRd(Floppy* flp) {
	return flp->data[flp->rtrk].byte[flp->pos];
}

uint8_t flpGetField(Floppy* flp) {
	return flp->data[flp->rtrk].field[flp->pos];
}

void flpStep(Floppy* flp,bool dir) {
	if (dir) {
		if (flp->trk < ((flp->flag & FLP_TRK80) ? 86 : 43)) flp->trk++;
	} else {
		if (flp->trk > 0) flp->trk--;
	}
}

bool flpNext(Floppy* flp, bool bdiSide) {
	bool res = false;
	flp->rtrk = flp->trk << 1;
	if (flp->flag & FLP_DS) flp->rtrk += bdiSide ? 0 : 1;		// bdiSide = zx->bdi->vg93.side
	if (flp->flag & FLP_INSERT) {
		flp->pos++;
		if (flp->pos >= TRACKLEN) {
			flp->pos = 0;
			res = true;					// tick of index begin = zx->bdi->t
		}
		flp->field = flp->data[flp->rtrk].field[flp->pos];
	} else {
		flp->field = 0;
	}
	return res;
}

void flpSetFlag(Floppy* flp,int mask,bool state) {
	if (state) {
		flp->flag |= mask;
	} else {
		flp->flag &= ~mask;
	}
}

bool flpGetFlag(Floppy* flp,int mask) {
	return ((flp->flag & mask) ? true : false);
}

void flpClearTrack(Floppy* flp,int tr) {
	for (int i = 0; i < TRACKLEN; i++) {
		flp->data[tr].byte[i] = 0x00;
		flp->data[tr].field[i] = 0x00;
	}
}

void flpFormat(Floppy* flp) {
	int i;
	uint8_t *buf = new uint8_t[0x1000];
	for (i = 0; i < 0x1000; i++) buf[i]=0x00;
	for (i = 1;i < 168; i++) flpFormTRDTrack(flp,i,buf);
	memcpy(buf + 0x8e0, trd_8e1, 0x20);
	flpFormTRDTrack(flp,0,buf);
	delete(buf);
}

void flpFormTRDTrack(Floppy* flp, int tr, uint8_t* bpos) {
	std::vector<Sector> lst;
	Sector sct;
	uint8_t* ppos = bpos;
	sct.cyl = ((tr & 0xfe) >> 1);
	sct.side = (tr & 0x01) ? 1 : 0;
	sct.len = 1;
	int32_t sc;
	for (sc = 1; sc < 17; sc++) {
		sct.sec = sc;
		sct.data = ppos;
		lst.push_back(sct);
		ppos += 256;
	}
	flpFormTrack(flp,tr,lst);
}

uint16_t getCrc(uint8_t* ptr, int32_t len) {
	uint32_t crc = 0xcdb4;
	int32_t i;
	while (len--) {
		crc ^= *ptr << 8;
		for (i = 0; i<8 ; i++) {
			if ((crc *= 2) & 0x10000) crc ^= 0x1021;
		}
		ptr++;
	}
	return  (crc & 0xffff);
}

void flpFillFields(Floppy* flp,int tr, bool fcrc) {
	if (tr > 255) return;
	int i, bcnt = 0, sct = 1;
	uint8_t fld = 0;
	uint8_t* cpos = flp->data[tr].byte;
	uint8_t* bpos = cpos;
	uint16_t crc;
	for (i=0;i<TRACKLEN;i++) {
		flp->data[tr].field[i] = fld;
		if (fcrc) {
			switch (fld) {
				case 0:
					if ((*bpos) == 0xf5) *bpos = 0xa1;
					if ((*bpos) == 0xf6) *bpos = 0xc2;
					break;
				case 4:
					if (*bpos == 0xf7) {
						crc = getCrc(cpos, bpos - cpos);
						*bpos = ((crc & 0xff00) >> 8);
						*(bpos + 1) = (crc & 0xff);
					}
					break;
			}
		}
		if (bcnt > 0) {
			bcnt--;
			if (bcnt==0) {
				if (fld < 4) {
					fld = 4; bcnt = 2;
				} else {
					fld = 0;
				}
			}
		} else {
			if (flp->data[tr].byte[i] == 0xfe) {
				cpos = bpos;
				fld = 1;
				bcnt = 4;
				sct = flp->data[tr].byte[i+4];
			}
			if (flp->data[tr].byte[i] == 0xfb) {
				cpos = bpos;
				fld = 2;
				bcnt = (128 << sct);
			}
			if (flp->data[tr].byte[i] == 0xf8) {
				cpos = bpos;
				fld = 3;
				bcnt = (128 << sct);
			}
		}
		bpos++;
	}
}

void flpFormTrack(Floppy* flp, int tr, std::vector<Sector> sdata) {
	if (tr > 255) return;
	uint8_t *ppos = flp->data[tr].byte;
	int32_t i,ln;
	uint32_t sc;
	for (i=0; i<12; i++) *(ppos++) = 0x00;		// 12	space
	*(ppos++) = 0xc2; *(ppos++) = 0xc2;		// 	track mark
	*(ppos++) = 0xc2; *(ppos++) = 0xfc;
	for (sc=0; sc<sdata.size(); sc++) {
		for(i=0;i<10;i++) *(ppos++) = 0x4e;		// 10	sync
		for(i=0;i<12;i++) *(ppos++) = 0x00;		// 12	space
		*(ppos++) = 0xa1;				//	address mark
		*(ppos++) = 0xa1;
		*(ppos++) = 0xa1;
		*(ppos++) = 0xfe;
		*(ppos++) = sdata[sc].cyl;			// 	addr field
		*(ppos++) = sdata[sc].side;
		*(ppos++) = sdata[sc].sec;
		*(ppos++) = sdata[sc].len;
		*(ppos++) = 0xf7; *(ppos++) = 0xf7;
		for(i=0;i<22;i++) *(ppos++) = 0x4e;		// 22	sync
		for(i=0;i<12;i++) *(ppos++) = 0x00;		// 12	space
		*(ppos++) = 0xa1;				//	data mark
		*(ppos++) = 0xa1;
		*(ppos++) = 0xa1;
		*(ppos++) = sdata[sc].type;
		ln = (128 << sdata[sc].len);			//	data
		for (i=0; i<ln; i++) *(ppos++) = sdata[sc].data[i];
		*(ppos++) = 0xf7; *(ppos++) = 0xf7;
		for(i=0;i<60;i++) *(ppos++) = 0x4e;		// 60	sync
	}
	while ((ppos - flp->data[tr].byte) < TRACKLEN) *(ppos++) = 0x4e;		// ?	last sync
	flpFillFields(flp,tr,true);
}

bool flpEject(Floppy* flp) {
	flp->path = "";
	flp->flag &= ~(FLP_INSERT | FLP_CHANGED);
	return true;
}

int flpGet(Floppy* flp, int wut) {
	int res = -1;
	uint8_t* buf = new uint8_t[0x100];
	switch (wut) {
		case FLP_DISKTYPE:
			if (flpGetSectorData(flp,0,9,buf,0x100)) {
				if (buf[0xe7] == 0x10) res = TYPE_TRD;
			}
			break;
		case FLP_TRK: res = flp->trk; break;
		case FLP_RTRK: res = flp->rtrk; break;
		case FLP_FIELD: res = flp->field; break;
		case FLP_POS: res = flp->pos; break;
	}
	delete(buf);
	return res;
}

std::string flpGetPath(Floppy* flp) {return flp->path;}
void flpSetPath(Floppy* flp,const char* pth) {flp->path = std::string(pth);}

int flpCreateFile(Floppy* flp,TRFile* dsc) {
	uint8_t* buf = new uint8_t[256];
	if (!flpGetSectorData(flp,0,9,buf,256)) return ERR_SHIT;
	dsc->sec = buf[0xe1];
	dsc->trk = buf[0xe2];
	uint8_t files = buf[0xe4];
	if (files > 127) return ERR_MANYFILES;
	files++;
	buf[0xe4] = files;
	uint16_t freesec = buf[0xe5] + (buf[0xe6] << 8);
	if (freesec < dsc->slen) return ERR_NOSPACE;
	freesec -= dsc->slen;
	buf[0xe5] = freesec & 0xff;
	buf[0xe6] = ((freesec & 0xff00) >> 8);
	buf[0xe1] += (dsc->slen & 0x0f);
	buf[0xe2] += ((dsc->slen & 0xf0) >> 4);
	if (buf[0xe1] > 0x0f) {
		buf[0xe1] -= 0x10;
		buf[0xe2]++;
	}
	flpPutSectorData(flp,0,9,buf,256);
	freesec = ((files & 0xf0) >> 4)+1;
	if (!flpGetSectorData(flp,0,freesec,buf,256)) return ERR_SHIT;
	memcpy(buf + (((files - 1) & 0x0f) << 4), (char*)dsc, 16);
	flpPutSectorData(flp,0,freesec,buf,256);
	flp->flag |= FLP_CHANGED;
	return ERR_OK;
}

std::vector<TRFile> flpGetTRCatalog(Floppy* flp) {
	std::vector<TRFile> res;
	if (flpGet(flp,FLP_DISKTYPE) == TYPE_TRD) {
		TRFile file;
		uint8_t* buf = new uint8_t[256];
		uint8_t* ptr;
		int i,j;
		for (i=1; i<9; i++) {
			if (flpGetSectorData(flp,0,i,buf,256)) {
				ptr = buf;
				for (j=0; j<15; j++) {
					if (*ptr == 0x00) break;
					if (*ptr > 0x1f) {
						memcpy((char*)&file,ptr,16);
						res.push_back(file);
					}
					ptr += 0x10;
				}
				if (j<15) break;
			} else {
				break;
			}
		}
	}
	return res;
}

uint8_t* flpGetSectorDataPtr(Floppy* flp, uint8_t tr, uint8_t sc) {
	if (~flp->flag & FLP_INSERT) return NULL;
	int32_t tpos = 0;
	bool fnd;
	while (1) {
		while (flp->data[tr].field[tpos] != 1) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		fnd = (flp->data[tr].byte[tpos+2] == sc);
		tpos += 6;
		while ((flp->data[tr].field[tpos] != 2) && (flp->data[tr].field[tpos] != 3)) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		if (fnd) {
			return flp->data[tr].byte + tpos;
		}
		tpos += 0x102;
		if (tpos >= TRACKLEN) return NULL;
	}
}

bool flpPutSectorData(Floppy* flp, uint8_t tr,uint8_t sc,uint8_t* buf,int len) {
	uint8_t* ptr = flpGetSectorDataPtr(flp,tr,sc);
	if (ptr == NULL) return false;
	memcpy(ptr,buf,len);
	uint16_t crc = getCrc(ptr-1,len+1);
	*(ptr + len) = ((crc & 0xff00) >> 8);
	*(ptr + len + 1) = (crc & 0x00ff);
	return true;
}

bool flpGetSectorData(Floppy* flp, uint8_t tr,uint8_t sc,uint8_t* buf,int len) {
	uint8_t* ptr = flpGetSectorDataPtr(flp,tr,sc);
	if (ptr == NULL) return false;
	memcpy(buf,ptr,len);
	return true;
}

bool flpGetSectorsData(Floppy* flp, uint8_t tr, uint8_t sc, uint8_t* ptr, int sl) {
	while (sl > 0) {
		if (!flpGetSectorData(flp,tr,sc,ptr,256)) return false;
		ptr += 256;
		sc++;
		if (sc > 16) {
			sc = 1;
			tr++;
		}
		sl--;
	}
	return true;
}

void flpGetTrack(Floppy* flp,int tr,uint8_t* dst) {memcpy(dst,flp->data[tr].byte,TRACKLEN);}
void flpGetTrackFields(Floppy* flp,int tr,uint8_t* dst) {memcpy(dst,flp->data[tr].field,TRACKLEN);}

void flpPutTrack(Floppy* flp,int tr,uint8_t* src,int len) {
	flpClearTrack(flp,tr);
	memcpy(flp->data[tr].byte,src,len);
	flpFillFields(flp,tr,false);
}

Sector::Sector() {
	type = 0xfb;
	crc = -1;
}
