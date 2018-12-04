#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "floppy.h"

Floppy* flpCreate(int id) {
	Floppy* flp = (Floppy*)malloc(sizeof(Floppy));
	memset(flp,0x00,sizeof(Floppy));
	flp->id = id;
	flp->trk80 = 1;
	flp->doubleSide = 1;
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
	flp->wr = 1;
	if (flp->insert) {
		flp->changed = 1;
		flp->data[flp->rtrk].byte[flp->pos] = val;
	}
}

unsigned char flpRd(Floppy* flp) {
	flp->rd = 1;
	return flp->insert ? flp->data[flp->rtrk].byte[flp->pos] : 0xff;
}

unsigned char flpGetField(Floppy* flp) {
	return flp->data[flp->rtrk].field[flp->pos];
}

void flpStep(Floppy* flp,int dir) {
	switch (dir) {
		case FLP_FORWARD:
			if (flp->trk < (flp->trk80 ? 86 : 43)) flp->trk++;
			break;
		case FLP_BACK:
			if (flp->trk > 0) flp->trk--;
			break;
	}
}

int flpNext(Floppy* flp, int fdcSide) {
	int res = 0;
	flp->rtrk = (flp->trk << 1);
	if (flp->doubleSide && !fdcSide) flp->rtrk++;		// /SIDE1 = 0 when upper head (1) selected
	if (flp->insert) {
		flp->pos++;
		if (flp->pos >= TRACKLEN) {
			flp->pos = 0;
			res = 1;
		}
		flp->index = (flp->pos < 4) ? 1 : 0;		// ~90ms index pulse
		flp->field = flp->data[flp->rtrk].field[flp->pos];
	} else {
		flp->field = 0;
	}
	return res;
}

static unsigned char trk_mark[4] = {0xc1,0xc1,0xc1,0xfc};
static unsigned char hd_mark[4] = {0xa1,0xa1,0xa1,0xfe};
static unsigned char dat_mark[4] = {0xa1,0xa1,0xa1,0xfb};

int flp_format_trk(Floppy* flp, int trk, int spt, int slen, char* data) {
	int res = 1;
	int t = 128;
	int n = 0;		// 0:128, 1:256, 2:512, 3:1024, 4:2048
	int spc;
	int tr;
	int sc;
	int hd;
	unsigned char* ptr;
	while (t < slen) {
		t <<= 1;
		n++;
	}
	if ((n > 4) || (trk > 255) || (spt < 1)) {
		res = 0;
	} else {
		ptr = flp->data[trk].byte;
		slen = t;
		spc = (TRACKLEN - 20) / spt;
		spc -= (72 + slen);		// space 3 size
		if (spc < 10) {
			res = 0;
		} else {
			hd = trk & 1;
			tr = trk >> 1;
			memset(ptr, 0x4e, TRACKLEN);
			ptr += 12;
			memcpy(ptr, trk_mark, 4); ptr += 4;
			for (sc = 0; sc < spt; sc++) {
				ptr += 10;
				memset(ptr, 0x00, 12); ptr += 12;
				memcpy(ptr, hd_mark, 4); ptr += 4;
				*(ptr++) = tr & 0xff;
				*(ptr++) = hd & 1;
				*(ptr++) = sc & 0xff;
				*(ptr++) = n & 0xff;
				*(ptr++) = 0xf7;
				*(ptr++) = 0xf7;
				ptr += 22;
				memset(ptr, 0x00, 12); ptr += 12;
				memcpy(ptr, dat_mark, 4); ptr += 4;
				if (data) {
					memcpy(ptr, data, slen);
					ptr += slen;
					data += slen;
				} else {
					memset(ptr, 0x00, slen); ptr += slen;
				}
				*(ptr++) = 0xf7;
				*(ptr++) = 0xf7;
				ptr += spc;
			}
			flpFillFields(flp, trk, 1);
		}
	}
	return res;
}

void flpPrev(Floppy* flp, int fdcSide) {
	flp->rtrk = (flp->trk << 1);
	if (flp->doubleSide && !fdcSide) flp->rtrk++;
	if (flp->insert) {
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

void flpClearTrack(Floppy* flp,int tr) {
	memset(flp->data[tr].byte, 0, TRACKLEN);
	memset(flp->data[tr].field, 0, TRACKLEN);
}

void flpClearDisk(Floppy* flp) {
	int i;
	for (i = 0; i < 160; i++) flpClearTrack(flp,i);
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
	int i, bcnt = 0;
	unsigned char fld = 0;
	unsigned char* cpos = flp->data[tr].byte;
	unsigned char* bpos = cpos;
	unsigned short crc;
	unsigned char scn = 0;		// sector number (1+)
	unsigned char sct = 1;		// sector size code
	if (tr > 255) return;
	for (i = 0; i < 256; i++) {
		flp->data[tr].map[i] = 0;
	}
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
				scn = flp->data[tr].byte[i+3];
				sct = flp->data[tr].byte[i+4];
			}
			if (flp->data[tr].byte[i] == 0xfb) {
				cpos = bpos;
				fld = 2;
				bcnt = (128 << (sct & 3));
				if (scn > 0) {
					flp->data[tr].map[scn] = i + 1;
					scn = 0;
				}
			}
			if (flp->data[tr].byte[i] == 0xf8) {
				cpos = bpos;
				fld = 3;
				bcnt = (128 << (sct & 3));
				if (scn > 0) {
					flp->data[tr].map[scn] = i + 1;
					scn = 0;
				}
			}
		}
		bpos++;
	}
}

int flpEject(Floppy* flp) {
	free(flp->path);
	flp->path = NULL;
	flp->insert = 0;
	flp->changed = 0;
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
