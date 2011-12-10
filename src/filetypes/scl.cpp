#include "filetypes.h"
#include <string.h>

struct FilePos {
	uint8_t trk;
	uint8_t sec;
	uint8_t slen;
};

int loadSCL(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	uint8_t* buf = new uint8_t[0x1000];
	uint8_t* bptr;
	uint8_t fcnt;
	uint16_t tmpa;
	int scnt;
	int i;
	
	file.read((char*)buf,9);
	if (std::string((const char*)buf,8) != "SINCLAIR") return ERR_SCL_SIGN;
	if (buf[8] > 0x80) return ERR_SCL_MANY;
	flpFormat(flp);
	fcnt = buf[8];
	for (i=0;i<0x1000;i++) buf[i]=0x00;
	scnt = 0x10;
	bptr = buf;
	for (i=0;i<fcnt;i++) {
		file.read((char*)bptr,14);
		*(bptr + 14) = scnt & 0x0f;
		*(bptr + 15) = ((scnt & ~0x0f) >> 4);
		scnt += *(bptr + 13);
		bptr += 16;
	}
	*(bptr)=0;
	buf[0x800] = 0;
	buf[0x8e1] = scnt & 0x0f;
	buf[0x8e2] = ((scnt & 0xf0) >> 4);
	buf[0x8e3] = 0x16;
	buf[0x8e4] = fcnt;
	tmpa = 0xa00 - scnt;
	buf[0x8e5] = (tmpa & 0xff);
	buf[0x8e6] = ((tmpa & 0xff00) >> 8);
	buf[0x8e7] = 0x10;
	flpFormTRDTrack(flp,0,buf);
	i = 1;
	while (!file.eof()) {
		file.read((char*)buf,0x1000);
		flpFormTRDTrack(flp,i,buf);
		i++;
	}
	flpSetPath(flp,name);
	flpSetFlag(flp,FLP_INSERT,true);
	flpSetFlag(flp,FLP_CHANGED,false);
	return ERR_OK;
}

int saveSCL(Floppy* flp,const char* name) {
	const char* sign = "SINCLAIR";
	uint8_t* img = new uint8_t[0xA0000];
	uint8_t* buf = new uint8_t[256];
	uint8_t* dptr = img;
	uint8_t* bptr;
	FilePos newfp;
	std::vector<FilePos> fplist;
	int i,j;
	uint8_t tr,sc;
	
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
					fplist.push_back(newfp);
					dptr += 14;
					img[8]++;
				}
				bptr += 16;
			}
		}
	}
	for (i = 0; i < (int)fplist.size(); i++) {
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
	
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	file.write((char*)img,dptr-img);
	file.close();

	flpSetFlag(flp,FLP_CHANGED,false);
	
	return ERR_OK;
}
