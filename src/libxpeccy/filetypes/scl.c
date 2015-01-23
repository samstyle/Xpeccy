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

int loadSCL(Floppy* flp,const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	unsigned char buf[0x1000];		// track image
	unsigned char* bptr;
	int tmpa;
	int scnt;
	unsigned int i;

	sclHead hd;
	fread((char*)&hd, sizeof(sclHead), 1, file);
	if (strncmp(hd.sign, "SINCLAIR", 8) != 0) return ERR_SCL_SIGN;
	if (hd.files > 128) return ERR_SCL_MANY;

	flpFormat(flp);
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
	flpFormTRDTrack(flp,0,buf);
	i = 1;
	while (!feof(file) && (i < 168)) {
		fread((char*)buf, 0x1000, 1, file);
		flpFormTRDTrack(flp,i,buf);
		i++;
	}
	fclose(file);
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->insert = 1;
	flp->changed = 0;
	return ERR_OK;
}

int saveSCL(Floppy* flp,const char* name) {
	const char* sign = "SINCLAIR";
	unsigned char img[0xA0000];
	unsigned char buf[256];
	unsigned char* dptr = img;
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
		flpGetSectorData(flp,0,i,buf,256);
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
			flpGetSectorData(flp,tr, sc + 1, dptr, 256);
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

	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->changed = 0;

	return ERR_OK;
}
