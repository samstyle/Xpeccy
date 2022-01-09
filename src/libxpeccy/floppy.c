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
	// flp->rtrk = 0;
	flp->pos = 0;
	flp->path = NULL;
	return flp;
}

void flpDestroy(Floppy* flp) {
	free(flp);
}

void flp_set_path(Floppy* flp, const char* path) {
	if (path != NULL) {
		flp->path = realloc(flp->path, strlen(path) + 1);
		strcpy(flp->path, path);
	} else {
		free(flp->path);
		flp->path = NULL;
	}
}

void flpWr(Floppy* flp, int hd, unsigned char val) {
	flp->wr = 1;
	hd &= 1;
	if (hd & !flp->doubleSide) return;	// saving on HD1 for SS Floppy
	if (flp->insert) {
		flp->changed = 1;
		flp->data[(flp->trk << 1) | hd].byte[flp->pos] = val;
	}
}

unsigned char flpRd(Floppy* flp, int hd) {
	flp->rd = 1;
	hd &= 1;
	if (!flp->insert) return 0xff;
	if (hd && !flp->doubleSide) return 0xff;
	return flp->data[(flp->trk << 1) | hd].byte[flp->pos];
}

void flpStep(Floppy* flp, int dir) {
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
	int rtrk = (flp->trk << 1);
	if (flp->doubleSide && fdcSide) rtrk++;
	if (flp->insert) {
		flp->pos++;
		if (flp->pos >= TRKLEN_DD) {
			flp->pos = 0;
			res = 1;
		}
		flp->index = (flp->pos < 4) ? 1 : 0;		// ~90ms index pulse
		flp->field = flp->data[rtrk].field[flp->pos] & 0x0f;
	} else {
		flp->field = 0;
	}
	return res;
}

unsigned short getCrc(unsigned char* ptr, int len) {
	int crc = 0xffff;
	unsigned char val;
	int i;
	while (len--) {
		val = *ptr;
		crc ^= val << 8;
		for (i = 0; i < 8 ; i++) {
			if ((crc *= 2) & 0x10000) crc ^= 0x1021;
		}
		ptr++;
	}
	return  (crc & 0xffff);
}

static unsigned char trk_mark[4] = {0xc1,0xc1,0xc1,0xfc};
static unsigned char hd_mark[4] = {0xa1,0xa1,0xa1,0xfe};
static unsigned char dat_mark[4] = {0xa1,0xa1,0xa1,0xfb};

int flp_format_trk_buf(int trk, int spt, int slen, char* data, unsigned char* buf) {
	int res = 1;
	int t = 128;
	int n = 0;		// 0:128, 1:256, 2:512, 3:1024, 4:2048
	int spc;
	int tr;
	int sc;
	int hd;
	unsigned char* ptr;
	unsigned char* stp;
	unsigned short crc;
	while (t < slen) {
		t <<= 1;
		n++;
	}
	if ((n > 4) || (trk > 255) || (spt < 1)) {
		res = 0;
	} else {
		ptr = buf;
		slen = t;
		spc = (TRKLEN_DD - 20) / spt;
		spc -= (72 + slen);		// space 3 size
		if (spc < 10) {
			res = 0;
		} else {
			hd = trk & 1;
			tr = trk >> 1;
			memset(ptr, 0x4e, TRKLEN_DD);
			ptr += 12;
			memcpy(ptr, trk_mark, 4); ptr += 4;
			for (sc = 1; sc <= spt; sc++) {
				ptr += 10;
				memset(ptr, 0x00, 12); ptr += 12;
				stp = ptr;
				memcpy(ptr, hd_mark, 4); ptr += 4;
				*(ptr++) = tr & 0xff;
				*(ptr++) = hd & 1;
				*(ptr++) = sc & 0xff;
				*(ptr++) = n & 0xff;
				crc = getCrc(stp, ptr - stp);
				*(ptr++) = (crc >> 8) & 0xff;
				*(ptr++) = crc & 0xff;
				ptr += 22;
				memset(ptr, 0x00, 12); ptr += 12;
				stp = ptr;
				memcpy(ptr, dat_mark, 4); ptr += 4;
				if (data) {
					memcpy(ptr, data, slen);
					ptr += slen;
					data += slen;
				} else {
					memset(ptr, 0x00, slen);
					ptr += slen;
				}
				crc = getCrc(stp, ptr - stp);
				*(ptr++) = (crc >> 8) & 0xff;
				*(ptr++) = crc & 0xff;
				ptr += spc;
			}
		}
	}
	return res;
}

int flp_format_trk(Floppy* flp, int trk, int spt, int slen, char* data) {
	int res = 1;
	if (trk > 255) {
		res = 0;
	} else {
		res = flp_format_trk_buf(trk, spt, slen, data, flp->data[trk].byte);
		if (res) flpFillFields(flp, trk, 0);
	}
	return res;
}

void flpPrev(Floppy* flp, int fdcSide) {
	int rtrk = (flp->trk << 1);
	if (flp->doubleSide && fdcSide) rtrk++;
	if (flp->insert) {
		if (flp->pos > 0) {
			flp->pos--;
		} else {
			flp->pos = TRKLEN_DD - 1;
		}
		flp->field = flp->data[rtrk].field[flp->pos] & 0x0f;
	} else {
		flp->field = 0;
	}
}

void flpClearTrack(Floppy* flp,int tr) {
	memset(flp->data[tr].byte, 0, TRKLEN_DD);
	memset(flp->data[tr].field, 0, TRKLEN_DD);
}

void flpClearDisk(Floppy* flp) {
	int i;
	for (i = 0; i < 160; i++) flpClearTrack(flp,i);
}

void flpFillFields(Floppy* flp,int tr, int flag) {
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
	for (i = 0; i < TRKLEN_DD; i++) {
		flp->data[tr].field[i] = fld;
		fld &= 0x0f;		// reset flags
		if (flag & 1) {
			if (fld == 0) {
					if ((*bpos) == 0xf5) *bpos = 0xa1;
					if ((*bpos) == 0xf6) *bpos = 0xc2;
			}
		}
		if (bcnt > 0) {
			bcnt--;
			if (bcnt==0) {
				if (fld < 4) {
					if (flag & 1) {
						cpos -= 3;		// including a1,a1,a1
						crc = getCrc(cpos, bpos - cpos + 1);
						*(bpos + 1) = ((crc & 0xff00) >> 8);
						*(bpos + 2) = (crc & 0xff);
					}
					fld = 4;
					bcnt = 2;
				} else {
					fld = 0;
				}
			}
		} else {					// TODO: find A1 here, then skip until !A1, this byte will be marker
			if (flp->data[tr].byte[i] == 0xa1) {
				fld = 0x0f;
			} else {
				if (fld == 0x0f) {
					switch (flp->data[tr].byte[i] & 0xfe) {
						case 0xfe:			// fe/ff : IDAM, head
							cpos = bpos;
							fld = 0x01;
							bcnt = 4;
							scn = flp->data[tr].byte[i+3];
							sct = flp->data[tr].byte[i+4];
							break;
						// case 0xfc:			// fc/fd : IAM, index marker
						//	break
						case 0xfa:			// fa/fb : DAM, data
							cpos = bpos;
							fld = 0x02;
							bcnt = (128 << (sct & 3));
							if (scn > 0) {
								flp->data[tr].map[scn] = i + 1;
								scn = 0;
							}
							break;
						case 0xf8:			// f8/f9 : DDAM, deleted data
							cpos = bpos;
							fld = 0x03;
							bcnt = (128 << (sct & 3));
							if (scn > 0) {
								flp->data[tr].map[scn] = i + 1;
								scn = 0;
							}
							break;
						default:
							fld = 0;
							break;

					}
				}
			}
		}
		bpos++;
	}
}

int flpEject(Floppy* flp) {
	flp_set_path(flp, NULL);
	flp->insert = 0;
	flp->changed = 0;
	return 1;
}


void flpGetTrack(Floppy* flp,int tr,unsigned char* dst) {
	memcpy(dst,flp->data[tr].byte,TRKLEN_DD);
}

void flpGetTrackFields(Floppy* flp,int tr,unsigned char* dst) {
	memcpy(dst,flp->data[tr].field,TRKLEN_DD);
}

void flpPutTrack(Floppy* flp,int tr,unsigned char* src,int len) {
	flpClearTrack(flp,tr);
	memcpy(flp->data[tr].byte,src,len);
	flpFillFields(flp,tr,0);
}
