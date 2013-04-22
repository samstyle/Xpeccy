#include <stdlib.h>
#include <string.h>

#include "floppy.h"

unsigned char trd_8e0[] = {
	0x00,0x00,0x01,0x16,0x00,0xf0,0x09,0x10,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00
};

Floppy* flpCreate(int id) {
	Floppy* flp = (Floppy*)malloc(sizeof(Floppy));
	flp->id = id;
	flp->flag = FLP_TRK80 | FLP_DS;
	flp->trk = 0;
	flp->rtrk = 0;
	flp->pos = 0;
	flp->path = NULL;
	return flp;
}

void flpDestroy(Floppy* flp) {
	free(flp);
}

void flpWr(Floppy* flp,unsigned char val) {
	flp->data[flp->rtrk].byte[flp->pos] = val;
	flp->flag |= FLP_CHANGED;
}

unsigned char flpRd(Floppy* flp) {
	return flp->data[flp->rtrk].byte[flp->pos];
}

unsigned char flpGetField(Floppy* flp) {
	return flp->data[flp->rtrk].field[flp->pos];
}

void flpStep(Floppy* flp,int dir) {
	switch (dir) {
		case FLP_FORWARD:
			if (flp->trk < ((flp->flag & FLP_TRK80) ? 86 : 43)) flp->trk++;
			break;
		case FLP_BACK:
			if (flp->trk > 0) flp->trk--;
			break;
	}
}

int flpNext(Floppy* flp, int fdcSide) {
	int res = 0;
	flp->rtrk = (flp->trk << 1);
	if ((flp->flag & FLP_DS) && !fdcSide) flp->rtrk++;		// /SIDE1 = 0 when upper head (1) selected
	if (flp->flag & FLP_INSERT) {
		flp->pos++;
		if (flp->pos >= TRACKLEN) {
			flp->pos = 0;
			res = 1;
		}
		flp->field = flp->data[flp->rtrk].field[flp->pos];
	} else {
		flp->field = 0;
	}
	return res;
}

void flpPrev(Floppy* flp, int fdcSide) {
	flp->rtrk = (flp->trk << 1);
	if ((flp->flag & FLP_DS) && !fdcSide) flp->rtrk++;
	if (flp->flag & FLP_INSERT) {
		if (flp->pos > 0) {
			flp->pos--;
		} else {
			flp->pos = TRACKLEN - 1;
		}
		flp->field = flp->data[flp->rtrk].field[flp->pos];
	} else {
		flp->field = 0;
	}

}

void flpClearDisk(Floppy* flp) {
	int i;
	for (i = 0; i < 160; i++) flpClearTrack(flp,i);
}

void flpClearTrack(Floppy* flp,int tr) {
	int i;
	for (i = 0; i < TRACKLEN; i++) {
		flp->data[tr].byte[i] = 0x00;
		flp->data[tr].field[i] = 0x00;
	}
}

void flpFormat(Floppy* flp) {
	int i;
	unsigned char *buf = (unsigned char*)malloc(0x1000 * sizeof(unsigned char));
	for (i = 0; i < 0x1000; i++) buf[i]=0x00;
	for (i = 1;i < 168; i++) flpFormTRDTrack(flp,i,buf);
	memcpy(buf + 0x8e0, trd_8e0, 0x20);
	flpFormTRDTrack(flp,0,buf);
	free(buf);
}

void flpFormTRDTrack(Floppy* flp, int tr, unsigned char* bpos) {
	Sector lst[16];
	Sector sct;
	int sc;
	unsigned char* ppos = bpos;
	sct.type = 0xfb;
	sct.crc = -1;
	sct.cyl = ((tr & 0xfe) >> 1);
	sct.side = (tr & 0x01) ? 1 : 0;
	sct.len = 1;
	for (sc = 0; sc < 16; sc++) {
		sct.sec = sc + 1;
		sct.data = ppos;
		lst[sc] = sct;
		ppos += 256;
	}
	flpFormTrack(flp,tr,lst,16);
}

unsigned short getCrc(unsigned char* ptr, int len) {
	unsigned int crc = 0xcdb4;
	int i;
	while (len--) {
		crc ^= *ptr << 8;
		for (i = 0; i<8 ; i++) {
			if ((crc *= 2) & 0x10000) crc ^= 0x1021;
		}
		ptr++;
	}
	return  (crc & 0xffff);
}

void flpFillFields(Floppy* flp,int tr, int fcrc) {
	int i, bcnt = 0, sct = 1;
	unsigned char fld = 0;
	unsigned char* cpos = flp->data[tr].byte;
	unsigned char* bpos = cpos;
	unsigned short crc;
	if (tr > 255) return;
	for (i = 0; i < TRACKLEN; i++) {
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

void flpFormTrack(Floppy* flp, int tr, Sector* sdata, int scount) {
	unsigned char *ppos = flp->data[tr].byte;
	int i,ln;
	unsigned int sc;
	if (tr > 255) return;
	for (i=0; i<12; i++) *(ppos++) = 0x00;		// 12	space
	*(ppos++) = 0xc2; *(ppos++) = 0xc2;		// 	track mark
	*(ppos++) = 0xc2; *(ppos++) = 0xfc;
	for (sc = 0; sc < scount; sc++) {
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
	flpFillFields(flp,tr,1);
}

int flpEject(Floppy* flp) {
	free(flp->path);
	flp->path = NULL;
	flp->flag &= ~(FLP_INSERT | FLP_CHANGED);
	return 1;
}

unsigned char fbuf[0x100];

int flpGet(Floppy* flp, int wut) {
	int res = -1;
	switch (wut) {
		case FLP_DISKTYPE:
			if (flpGetSectorData(flp,0,9,fbuf,0x100)) {
				if (fbuf[0xe7] == 0x10) res = DISK_TYPE_TRD;
			}
			break;
	}
	return res;
}

int flpCreateFile(Floppy* flp,TRFile* dsc) {
	unsigned char files;
	unsigned short freesec;
	if (!flpGetSectorData(flp,0,9,fbuf,256)) return ERR_SHIT;
	dsc->sec = fbuf[0xe1];
	dsc->trk = fbuf[0xe2];
	files = fbuf[0xe4];
	if (files > 127) return ERR_MANYFILES;
	files++;
	fbuf[0xe4] = files;
	freesec = fbuf[0xe5] + (fbuf[0xe6] << 8);
	if (freesec < dsc->slen) return ERR_NOSPACE;
	freesec -= dsc->slen;
	fbuf[0xe5] = freesec & 0xff;
	fbuf[0xe6] = ((freesec & 0xff00) >> 8);
	fbuf[0xe1] += (dsc->slen & 0x0f);
	fbuf[0xe2] += ((dsc->slen & 0xf0) >> 4);
	if (fbuf[0xe1] > 0x0f) {
		fbuf[0xe1] -= 0x10;
		fbuf[0xe2]++;
	}
	flpPutSectorData(flp,0,9,fbuf,256);
	freesec = ((files & 0xf0) >> 4)+1;
	if (!flpGetSectorData(flp,0,freesec,fbuf,256)) return ERR_SHIT;
	memcpy(fbuf + (((files - 1) & 0x0f) << 4), (char*)dsc, 16);
	flpPutSectorData(flp,0,freesec,fbuf,256);
	flp->flag |= FLP_CHANGED;
	return ERR_OK;
}

TRFile flpGetCatalogEntry(Floppy* flp, int num) {
	TRFile res;
	int sec,pos;
	if (flpGet(flp,FLP_DISKTYPE) != DISK_TYPE_TRD) return res;
	if (num > 127) return res;
	sec = ((num & 0xf0) >> 4);	// sector
	pos = ((num & 0x0f) << 4);	// file number inside sector
	if (!flpGetSectorData(flp,0,sec + 1,fbuf,256)) return res;
	memcpy((char*)&res,fbuf + pos,16);
	return res;
}

int flpGetTRCatalog(Floppy *flp, TRFile *dst) {
	int cnt = 0;
	if (flpGet(flp,FLP_DISKTYPE) == DISK_TYPE_TRD) {
		int sc;
		int fc;
		unsigned char* ptr;
		unsigned char* dpt = (unsigned char*)dst;
		for (sc = 1; sc < 9; sc++) {
			if (flpGetSectorData(flp,0,sc,fbuf,256)) {
				ptr = fbuf;
				for (fc = 0; fc < 16; fc++) {
					if (*ptr == 0) break;
					memcpy(dpt,ptr,16);
					dpt += 16;
					ptr += 16;
					cnt++;
				}
				if (fc < 16) break;
			} else {
				break;
			}
		}
	}
	return cnt;
}

/*
std::vector<TRFile> flpGetTRCatalog(Floppy* flp) {
	std::vector<TRFile> res;
	if (flpGet(flp,FLP_DISKTYPE) == DISK_TYPE_TRD) {
		TRFile file;
		unsigned char* buf = new unsigned char[256];
		unsigned char* ptr;
		int i,j;
		for (i=1; i<9; i++) {
			if (flpGetSectorData(flp,0,i,buf,256)) {
				ptr = buf;
				for (j=0; j<15; j++) {
					if (*ptr == 0x00) break;
					memcpy((char*)&file,ptr,16);
					res.push_back(file);
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
*/

unsigned char* flpGetSectorDataPtr(Floppy* flp, unsigned char tr, unsigned char sc) {
	int tpos = 0;
	int fnd;
	if (~flp->flag & FLP_INSERT) return NULL;
	while (1) {
		while (flp->data[tr].field[tpos] != 1) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		fnd = (flp->data[tr].byte[tpos+2] == sc) ? 1 : 0;
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

int flpPutSectorData(Floppy* flp, unsigned char tr,unsigned char sc,unsigned char* buf,int len) {
	unsigned short crc;
	unsigned char* ptr = flpGetSectorDataPtr(flp,tr,sc);
	if (ptr == NULL) return 0;
	memcpy(ptr,buf,len);
	crc = getCrc(ptr-1,len+1);
	*(ptr + len) = ((crc & 0xff00) >> 8);
	*(ptr + len + 1) = (crc & 0x00ff);
	return 1;
}

int flpGetSectorData(Floppy* flp, unsigned char tr,unsigned char sc,unsigned char* buf,int len) {
	unsigned char* ptr = flpGetSectorDataPtr(flp,tr,sc);
	if (ptr == NULL) return 0;
	memcpy(buf,ptr,len);
	return 1;
}

int flpGetSectorsData(Floppy* flp, unsigned char tr, unsigned char sc, unsigned char* ptr, int sl) {
	while (sl > 0) {
		if (!flpGetSectorData(flp,tr,sc,ptr,256)) return 0;
		ptr += 256;
		sc++;
		if (sc > 16) {
			sc = 1;
			tr++;
		}
		sl--;
	}
	return 1;
}

void flpGetTrack(Floppy* flp,int tr,unsigned char* dst) {
	memcpy(dst,flp->data[tr].byte,TRACKLEN);
}

void flpGetTrackFields(Floppy* flp,int tr,unsigned char* dst) {
	memcpy(dst,flp->data[tr].field,TRACKLEN);
}

void flpPutTrack(Floppy* flp,int tr,unsigned char* src,int len) {
	flpClearTrack(flp,tr);
	memcpy(flp->data[tr].byte,src,len);
	flpFillFields(flp,tr,0);
}
