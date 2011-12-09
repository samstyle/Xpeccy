#include "filetypes.h"

#include <string.h>

void putint(uint8_t* ptr, uint32_t val) {
	*ptr++ = val & 0xff;
	*ptr++ = (val & 0xff00) >> 8;
	*ptr++ = (val & 0xff0000) >> 16;
	*ptr++ = (val & 0xff000000) >> 24;
}

// crc32 for UDI, taken from Unreal 0.32.7
void crc32(int &crc, uint8_t *buf, unsigned len) {
	while (len--) {
		crc ^= -1 ^ *buf++;
		for(int k = 8; k--; ) {
			int temp = -(crc & 1); crc >>= 1, crc ^= 0xEDB88320 & temp;
		}
	crc ^= -1;
	}
}

void loadUDITrack(Floppy* flp,std::ifstream* file, uint8_t tr, bool sd) {
	int rt = (tr << 1) + (sd ? 1 : 0);
	uint8_t type = file->get();
	uint32_t len;
	int i;
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
			for (i = 0; i < TRACKLEN; i++) flp->data[tr].byte[i] = 0x00;
			//flp->nulltrack(rt);
			file->read((char*)flp->data[rt].byte,len);		// read track
			flpFillFields(flp,rt,false);
			len = (len >> 3) + (((len & 7) == 0)?0:1);	// skip bit field
			file->seekg(len,std::ios_base::cur);
		}
	}
}

void getUDIBitField(Floppy* flp,uint8_t tr, uint8_t* buf) {
	int i;
	int msk=0x01;
	for (i = 0; i < TRACKLEN; i++) {
		if (msk == 0x100) {
			msk = 0x01;
			*(++buf)=0x00;
		}
		if ((flp->data[tr].field[i] == 0) && (flp->data[tr].byte[i] == 0xa1)) *buf |= msk;
		msk <<= 1;
	}
	buf++;
}

int loadUDI(Floppy* flp, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	char* buf = new char[16];
	uint8_t tmp;
	bool sides;
	file.read(buf,16);
	if (std::string(buf,4) != "UDI!") return ERR_UDI_SIGN;
	if (*(buf + 8) != 0x00) return ERR_UDI_SIGN;
	tmp = *(buf + 9);		// max track;
	sides = (*(buf + 10) == 0x01);	// true if double side
	for (int i = 0; i < tmp + 1; i++) {
		loadUDITrack(flp,&file,i,false);
		if (sides) loadUDITrack(flp,&file,i,true);
	}
	flp->path = std::string(name);
	flpSetFlag(flp,FLP_INSERT,true);
	flpSetFlag(flp,FLP_CHANGED,false);
	return ERR_OK;
}

int saveUDI(Floppy* flp, const char* name) {
	const char* sign = "UDI!";
	uint8_t* img = new uint8_t[0x112cf4];	// 0x112cf4 for 160 tracks in UDI
	uint8_t* dptr = img;
	uint8_t* bptr;
	int i,j;
	
	memcpy(dptr,sign,4);
	bptr = img + 4;
	dptr += 8;
	*(dptr++) = 0x00;			// version
	*(dptr++) = flpGetFlag(flp,FLP_TRK80) ? 79 : 39;		// maximun track number
	*(dptr++) = flpGetFlag(flp,FLP_DS) ? 1 : 0;		// double side (due to floppy property)
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	*(dptr++) = 0x00;
	for (i = 0; i < (flpGetFlag(flp,FLP_TRK80) ? 160 : 80); i++) {
		*(dptr++) = 0x00;		// MFM
		*(dptr++) = (TRACKLEN & 0xff);	// track len
		*(dptr++) = ((TRACKLEN & 0xff00) >> 8);
		memcpy((char*)dptr,(char*)flp->data[i].byte,TRACKLEN);	// track image
		dptr += TRACKLEN;
		getUDIBitField(flp,i,dptr);
		dptr += 782;			// 6250 / 8 + 1
		if (!flpGetFlag(flp,FLP_DS)) i++;		// if single-side skip 
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
	flpSetFlag(flp,FLP_CHANGED,false);
	return ERR_OK;
}
