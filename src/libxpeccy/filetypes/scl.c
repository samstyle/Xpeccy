#include "filetypes.h"
#include <string.h>

#pragma pack (push, 1)

typedef struct {
	unsigned char trk;
	unsigned char sec;
	unsigned char slen;
} FilePos;

typedef struct {
	char sign[8];	// SINCLAIR
	unsigned char files;
} sclHead;

#pragma pack(pop)

int loadSCL(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;

	unsigned char buf[0x1000];		// track image
	unsigned char* bptr;
	int tmpa;
	int scnt;
	int i;

	sclHead hd;
	fread((char*)&hd, sizeof(sclHead), 1, file);
	if (strncmp(hd.sign, "SINCLAIR", 8) != 0) {
		err = ERR_SCL_SIGN;
	} else if (hd.files > 128) {
		err = ERR_SCL_MANY;
	} else {
		memset(buf,0x00,0x1000);
		scnt = 0x10;
		bptr = buf;					// start of TRK0
		for (i = 0; i < hd.files; i++) {		// make catalog
			fread((char*)bptr, 14, 1, file);	// file dsc
			bptr[14] = scnt & 0x0f;			// sector
			bptr[15] = ((scnt & 0xff0) >> 4);	// track
			scnt += bptr[13];			// +sectors size
			bptr += 16;				// next file
		}
		bptr[0] = 0;				// mark last file
		buf[0x800] = 0;
		buf[0x8e1] = scnt & 0x0f;		// free sector
		buf[0x8e2] = ((scnt & 0xff0) >> 4);	// free track
		buf[0x8e3] = 0x16;			// 80DS
		buf[0x8e4] = hd.files;			// files total
		tmpa = 0x9f0 - scnt;			// sectors free (0x9f0)	// FIXED: not 0xa00!
		buf[0x8e5] = (tmpa & 0xff);
		buf[0x8e6] = ((tmpa & 0xff00) >> 8);
		buf[0x8e7] = 0x10;			// trdos code
		memset(buf + 0x8ea, 0x20, 9);		// blank9
		memset(buf + 0x8f5, 0x20, 8);		// disk label
		flp_format_trk(flp, 0, 16, 256, (char*)buf);
		i = 1;
		while (!feof(file) && (i < 160)) {
			fread((char*)buf, 0x1000, 1, file);
			flp_format_trk(flp, i, 16, 256, (char*)buf);
			i++;
		}
		memset(buf, 0, 0x1000);
		while (i < 160) {
			flp_format_trk(flp, i, 16, 256, (char*)buf);
			i++;
		}
		flp_insert(flp, name);
	}
	fclose(file);
	return err;
}

int saveSCL(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	const char* sign = "SINCLAIR";
	unsigned char img[0xA0000];
	unsigned char buf[256];
	unsigned char* dptr;
	unsigned char* bptr;
	FilePos newfp;
	FilePos fplist[256];
	int fpidx = 0;
	int i,j;
	unsigned char tr,sc;

	memcpy(img,sign,8);
	img[8] = 0;
	dptr = img + 9;
	for (i = 1; i < 9; i++) {
		diskGetSectorData(flp,0,i,buf,256);
		bptr = buf;
		for (j = 0; j < 16; j++) {
			if (*bptr == 0) {
				i=20;
				j=20;
			} else {
				if (*bptr != 1) {
					memcpy(dptr,bptr,14);
					newfp.trk = *(bptr + 15);
					newfp.sec = *(bptr + 14);
					newfp.slen = *(bptr + 13);
					fplist[fpidx] = newfp;
					fpidx++;
					dptr += 14;
					img[8]++;
				}
				bptr += 16;
			}
		}
	}
	for (i = 0; i < fpidx; i++) {
		tr = fplist[i].trk;
		sc = fplist[i].sec;
		for(j = 0; j < fplist[i].slen; j++) {
			diskGetSectorData(flp,tr, sc + 1, dptr, 256);
			dptr += 256;
			sc++;
			if (sc > 15) {
				tr++;
				sc=0;
			}
		}
	}
	j=0;
	for(i = 0; i < dptr - img; i++) {
		j += *(img + i);
	}
	putint(dptr, j);
	dptr += 4;

	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	fwrite((char*)img, dptr-img, 1, file);
	fclose(file);

	flp_set_path(flp, name);
	flp->changed = 0;

	return ERR_OK;
}
