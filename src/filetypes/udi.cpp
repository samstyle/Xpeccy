#include "filetypes.h"

#include <string.h>

void putint(unsigned char* ptr, unsigned int val) {
	*ptr++ = val & 0xff;
	*ptr++ = (val & 0xff00) >> 8;
	*ptr++ = (val & 0xff0000) >> 16;
	*ptr++ = (val & 0xff000000) >> 24;
}

// crc32 for UDI, taken from Unreal 0.32.7
void crc32(int &crc, unsigned char *buf, unsigned len) {
	while (len--) {
		crc ^= -1 ^ *buf++;
		for(int k = 8; k--; ) {
			int temp = -(crc & 1); crc >>= 1, crc ^= 0xEDB88320 & temp;
		}
	crc ^= -1;
	}
}

void loadUDITrack(Floppy* flp,std::ifstream* file, unsigned char tr, bool sd) {
	int rt = (tr << 1) + (sd ? 1 : 0);
	unsigned char type = file->get();
	unsigned int len;
//	int i;
	unsigned char* trackBuf = new unsigned char[TRACKLEN];
	if (type != 0x00) {
		printf("TRK %i: unknown format %.2X\n",rt,type);
		len = getlen(file,4);							// field len
		file->seekg(len,std::ios_base::cur);					// skip unknown field
	} else {
		len = getlen(file,2);		// track size
		if (len > TRACKLEN) {
			printf("TRK %i: too long (%i)\n",rt,len);
			file->seekg(len,std::ios_base::cur);		// skip track image
			len = (len >> 3) + (((len & 7) == 0) ? 0 : 1);	// and bit field
			file->seekg(len,std::ios_base::cur);
		} else {
//			flpClearTrack(flp,tr);
			file->read((char*)trackBuf,len);
			flpPutTrack(flp,rt,trackBuf,len);
//			file->read((char*)flp->data[rt].byte,len);		// read track
//			flpFillFields(flp,rt,false);
			len = (len >> 3) + (((len & 7) == 0)?0:1);	// skip bit field
			file->seekg(len,std::ios_base::cur);
		}
	}
}

void getUDIBitField(Floppy* flp,unsigned char tr, unsigned char* buf) {
	int i;
	int msk=0x01;
	unsigned char* fieldBuf = new unsigned char[TRACKLEN];
	unsigned char* trackBuf = new unsigned char[TRACKLEN];
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
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	char* buf = new char[16];
	unsigned char tmp;
	bool sides;
	file.read(buf,16);
	if (strncmp((const char*)buf,"UDI!",4) != 0) return ERR_UDI_SIGN;
	if (*(buf + 8) != 0x00) return ERR_UDI_SIGN;
	tmp = *(buf + 9);		// max track;
	sides = (*(buf + 10) == 0x01);	// true if double side
	for (int i = 0; i < tmp + 1; i++) {
		loadUDITrack(flp,&file,i,false);
		if (sides) loadUDITrack(flp,&file,i,true);
	}
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->insert = 1;
	loadBoot(flp);
	flp->changed = 0;
	return ERR_OK;
}

int saveUDI(Floppy* flp, const char* name) {
	const char* sign = "UDI!";
	unsigned char* img = new unsigned char[0x112cf4];	// 0x112cf4 for 160 tracks in UDI
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
		flpPutTrack(flp,i,dptr,TRACKLEN); // memcpy((char*)dptr,(char*)flp->data[i].byte,TRACKLEN);	// track image
		dptr += TRACKLEN;
		getUDIBitField(flp,i,dptr);
		dptr += 782;			// 6250 / 8 + 1
		if (!flp->doubleSide) i++;		// if single-side skip
	}
	i = dptr - img;
	putint(bptr,i);
	j=-1;
	crc32(j,img,i);
	printf("crc = %X\n",j);
	putint(dptr,j);
	dptr += 4;
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	file.write((char*)img,dptr-img);
	file.close();
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->changed = 0;
	return ERR_OK;
}
