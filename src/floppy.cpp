#include <fstream>

#include <QMessageBox>
#include <QChar>
#include <QString>
#include <QDebug>

#include "common.h"
#include "bdi.h"
#include "filetypes/filetypes.h"

uint8_t trd_8e1[] = {
	0x00,0x00,0x01,0x16,0x00,0xf0,0x09,0x10,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00
};

Floppy::Floppy() {
	flag = FLP_TRK80 | FLP_DS;
	trk = 0;
	rtrk = 0;
	pos = 0;
}

std::string Floppy::getString() {
	std::string res = "80DW";
	if (~flag & FLP_TRK80) res[0]='4';
	if (~flag & FLP_DS) res[2]='S';
	if (flag & FLP_PROTECT) res[3]='R';
	return res;
}

void Floppy::setString(std::string st) {
	if (st.size() < 4) return;
	flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (st.substr(0,2) == "80") flag |= FLP_TRK80;
	if (st.substr(2,1) == "D") flag |= FLP_DS;
	if (st.substr(3,1) == "R") flag |= FLP_PROTECT;
}

void Floppy::wr(uint8_t val) {
	data[rtrk].byte[pos] = val;
	flag |= FLP_CHANGED;
}

uint8_t Floppy::rd() {
	return data[rtrk].byte[pos];
}

uint8_t Floppy::getfield() {
	return data[rtrk].field[pos];
}

void flpStep(Floppy* flp,bool dir) {
	if (dir) {
		if (flp->trk < ((flp->flag & FLP_TRK80) ? 86 : 43)) flp->trk++;
	} else {
		if (flp->trk > 0) flp->trk--;
	}
}

void flpNext(Floppy* flp, bool bdiSide, uint32_t bdiTick) {
	flp->rtrk = flp->trk << 1;
	if (flp->flag & FLP_DS) flp->rtrk += bdiSide ? 0 : 1;		// bdiSide = zx->bdi->vg93.side
	if (flp->flag & FLP_INSERT) {
		flp->pos++;
		if (flp->pos >= TRACKLEN) {
			flp->pos = 0;
			flp->ti = bdiTick;		// tick of index begin = zx->bdi->t
		}
		flp->field = flp->data[flp->rtrk].field[flp->pos];
	} else {
		flp->field = 0;
	}
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
	ushort crc;
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

bool Floppy::eject() {
	path = "";
	flag &= ~(FLP_INSERT | FLP_CHANGED);
	return true;
}

int Floppy::getDiskType() {
	int res = -1;
	uint8_t* buf = new uint8_t[0x100];
	if (getSectorData(0,9,buf,0x100)) {
		if (buf[0xe7] == 0x10) res = TYPE_TRD;
	}
	return res;
}

int Floppy::createFile(TRFile* dsc) {
	uint8_t* buf = new uint8_t[256];
	if (!getSectorData(0,9,buf,256)) return ERR_SHIT;
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
	putSectorData(0,9,buf,256);
	freesec = ((files & 0xf0) >> 4)+1;
	if (!getSectorData(0,freesec,buf,256)) return ERR_SHIT;
	memcpy(buf + (((files - 1) & 0x0f) << 4), (char*)dsc, 16);
	putSectorData(0,freesec,buf,256);
	flag |= FLP_CHANGED;
	return ERR_OK;
}

std::vector<TRFile> Floppy::getTRCatalog() {
	std::vector<TRFile> res;
	if (getDiskType() == TYPE_TRD) {
		TRFile file;
		uint8_t* buf = new uint8_t[256];
		uint8_t* ptr;
		int i,j;
		for (i=1; i<9; i++) {
			if (getSectorData(0,i,buf,256)) {
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

std::vector<Sector> Floppy::getsectors(uint8_t tr) {
	std::vector<Sector> res;
	Sector sec;
	int32_t len,p = 0;
	do {
		if (data[tr].field[p] == 1) {
			sec.cyl = data[tr].field[p++];
			sec.side = data[tr].field[p++];
			sec.sec = data[tr].field[p++];
			sec.len = data[tr].field[p];
			do {
				p++;
			} while ((p < TRACKLEN) && (data[tr].field[p] != 2) && (data[tr].field[p] != 3));
			if (p < TRACKLEN) {
				sec.type = data[tr].field[p-1];
				len = (128 << sec.len);
				sec.data = new uint8_t[len];
				memcpy(sec.data, &data[tr].byte[p],len);
				p += len;
				sec.crc = (data[tr].byte[p] << 8) + data[tr].byte[p+1]; p += 2;
				res.push_back(sec);
			}
		}
	} while (p < TRACKLEN);
	return res;
}

/*
void printHexBlock(uint8_t* ptr) {
	for (int i=0; i<256; i++) {
		printf("%.2X ",*ptr);
		ptr++;
		if ((i & 15) == 15) printf("\n");
	}
}
*/

uint8_t* Floppy::getSectorDataPtr(uint8_t tr, uint8_t sc) {
	if (~flag & FLP_INSERT) return NULL;
	int32_t tpos = 0;
	bool fnd;
	while (1) {
		while (data[tr].field[tpos] != 1) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		fnd = (data[tr].byte[tpos+2] == sc);
		tpos += 6;
		while ((data[tr].field[tpos] != 2) && (data[tr].field[tpos] != 3)) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		if (fnd) {
			return data[tr].byte + tpos;
		}
		tpos += 0x102;
		if (tpos >= TRACKLEN) return NULL;
	}
}

bool Floppy::putSectorData(uint8_t tr,uint8_t sc,uint8_t* buf,int len) {
	uint8_t* ptr = getSectorDataPtr(tr,sc);
	if (ptr == NULL) return false;
	memcpy(ptr,buf,len);
	uint16_t crc = getCrc(ptr-1,len+1);
	*(ptr + len) = ((crc & 0xff00) >> 8);
	*(ptr + len + 1) = (crc & 0x00ff);
	return true;
}

bool Floppy::getSectorData(uint8_t tr,uint8_t sc,uint8_t* buf,int len) {
	uint8_t* ptr = getSectorDataPtr(tr,sc);
	if (ptr == NULL) return false;
	memcpy(buf,ptr,len);
	return true;
}

bool Floppy::getSectorsData(uint8_t tr, uint8_t sc, uint8_t* ptr, int sl) {
	while (sl > 0) {
		if (!getSectorData(tr,sc,ptr,256)) return false;
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

Sector::Sector() {
	type = 0xfb;
	crc = -1;
}

Sector::Sector(uint8_t p1,uint8_t p2,uint8_t p3,uint8_t p4,uint8_t* p5) {
	cyl = p1; side = p2; sec = p3; len = p4; data = p5;
	type = 0xfb;
	crc = -1;
}
