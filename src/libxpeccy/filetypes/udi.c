#include "filetypes.h"

#include <string.h>

void putint(unsigned char* ptr, unsigned int val) {
	*ptr++ = val & 0xff;
	*ptr++ = (val & 0xff00) >> 8;
	*ptr++ = (val & 0xff0000) >> 16;
	*ptr++ = (val & 0xff000000) >> 24;
}

unsigned int freadLen(FILE* file, int n) {
	unsigned int len = 0;
	for (int i = 0; i < n; i++) {
		len |= (fgetc(file) << (i << 3));
	}
	return len;
}

// crc32 for UDI, taken from Unreal 0.32.7
int uCrc32(int crc, unsigned char *buf, unsigned len) {
	while (len--) {
		crc ^= -1 ^ *buf++;
		for(int k = 8; k--; ) {
			int temp = -(crc & 1); crc >>= 1, crc ^= 0xEDB88320 & temp;
		}
		crc ^= -1;
	}
	return crc;
}

void loadUDITrack(Floppy* flp, FILE* file, unsigned char tr, int sd) {
	int rt = (tr << 1) + (sd ? 1 : 0);
	unsigned char type = fgetc(file);
	unsigned int len;
//	int i;
	unsigned char trackBuf[TRACKLEN];
	if (type != 0x00) {
		printf("TRK %i: unknown format %.2X\n",rt,type);
		len = freadLen(file,4);					// field len
		fseek(file, len, SEEK_CUR);				// skip unknown field
	} else {
		len = freadLen(file,2);					// track size
		if (len > TRACKLEN) {
			printf("TRK %i: too long (%i)\n",rt,len);
			fseek(file, len, SEEK_CUR);		// skip track image
			len = (len >> 3) + (((len & 7) == 0) ? 0 : 1);	// and bit field
			fseek(file, len, SEEK_CUR);
		} else {
			fread((char*)trackBuf, len, 1, file);
			flpPutTrack(flp,rt,trackBuf,len);
			len = (len >> 3) + (((len & 7) == 0) ? 0 : 1);	// skip bit field
			fseek(file, len, SEEK_CUR);
		}
	}
}

void getUDIBitField(Floppy* flp, unsigned char tr, unsigned char* buf) {
	int i;
	int msk=0x01;
	unsigned char fieldBuf[TRACKLEN];
	unsigned char trackBuf[TRACKLEN];
	flpGetTrack(flp,tr,trackBuf);
	flpGetTrackFields(flp,tr,fieldBuf);
	for (i = 0; i < TRACKLEN; i++) {
		if (msk == 0x100) {
			msk = 0x01;
			*(++buf)=0x00;
		}
		if ((fieldBuf[i] == 0) && (trackBuf[i] == 0xa1)) *buf |= msk;
		msk <<= 1;
	}
	buf++;
}

int loadUDI(Floppy* flp, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	char buf[16];
	unsigned char tmp;
	int dbSide;

	fread(buf, 16, 1, file);
	if (strncmp((char*)buf, "UDI!", 4) != 0) {
		err = ERR_UDI_SIGN;
	} else if (buf[8] != 0x00) {
		err = ERR_UDI_SIGN;
	} else {
		tmp = buf[9];			// max track;
		dbSide = buf[10] ? 1 : 0 ;	// if double side
		for (int i = 0; i < tmp + 1; i++) {
			loadUDITrack(flp, file, i, 0);
			if (dbSide) loadUDITrack(flp, file, i, 1);
		}
		flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
		strcpy(flp->path,name);
		flp->insert = 1;
		flp->changed = 0;
	}
	fclose(file);
	return err;
}

int saveUDI(Floppy* flp, const char* name) {
	const char sign[] = "UDI!";
	unsigned char img[0x112cf4];	// 0x112cf4 for 160 tracks in UDI
	unsigned char* dptr = img;
	unsigned char* bptr;
	int i,j;

	memcpy(dptr,sign,4);
	bptr = img + 4;
	dptr += 8;
	*(dptr++) = 0x00;			// version
	*(dptr++) = flp->trk80 ? 79 : 39;		// maximun track number
	*(dptr++) = flp->doubleSide ? 1 : 0;		// double side (due to floppy property)
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	for (i = 0; i < (flp->trk80 ? 160 : 80); i++) {
		*(dptr++) = 0x00;		// MFM
		*(dptr++) = (TRACKLEN & 0xff);	// track len
		*(dptr++) = ((TRACKLEN & 0xff00) >> 8);
		flpGetTrack(flp,i,dptr); // memcpy((char*)dptr,(char*)flp->data[i].byte,TRACKLEN);	// track image
		dptr += TRACKLEN;
		getUDIBitField(flp,i,dptr);
		dptr += 782;			// 6250 / 8 + 1
		if (!flp->doubleSide) i++;		// if single-side skip
	}
	i = dptr - img;
	putint(bptr, i);
	j = -1;
	j = uCrc32(j, img, i);
	printf("crc = %X\n",j);
	putint(dptr, j);
	dptr += 4;

	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	fwrite((char*)img, dptr - img, 1, file);
	fclose(file);

	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->changed = 0;
	return ERR_OK;
}
