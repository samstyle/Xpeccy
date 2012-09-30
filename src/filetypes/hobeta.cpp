#include "filetypes.h"
#include <string.h>
#include <numeric>

int loadHobeta(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	unsigned char* buf = new unsigned char[256];
	TRFile nfle;
	int i;

	if (!(flp->flag & FLP_INSERT)) {
		flpFormat(flp);
		flp->flag |= FLP_INSERT;
	}
	if (flpGet(flp,FLP_DISKTYPE) != DISK_TYPE_TRD) return ERR_NOTRD;

	file.read((char*)buf,17);		// header
	memcpy((char*)&nfle,buf,13);
	nfle.slen = buf[14];
	if (flpCreateFile(flp,&nfle) != ERR_OK) return ERR_HOB_CANT;
	for (i = 0; i < nfle.slen; i++) {
		file.read((char*)buf,256);
		if (!flpPutSectorData(flp,nfle.trk, nfle.sec + 1, buf, 256)) return ERR_HOB_CANT;
		nfle.sec++;
		if (nfle.sec > 15) {
			nfle.trk++;
			nfle.sec -= 16;
		}
	}
	for (i=0; i<256; i++) flpFillFields(flp,i,true);
	return ERR_OK;
}

int saveHobeta(TRFile dsc,char* data,const char* name) {
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	unsigned short crc;
	unsigned char* buf = new unsigned char[17];	// header
	memcpy((char*)buf,(char*)&dsc.name[0],13);
	buf[13] = 0x00;
	buf[14] = dsc.slen;
	crc = ((105 + 257 * std::accumulate(buf, buf + 15, 0u)) & 0xffff);
	buf[15] = crc & 0xff;
	buf[16] = ((crc & 0xff00) >> 8);
	file.write((char*)buf,17);
	file.write(data,(dsc.slen) << 8);
	file.close();
	delete(buf);
	return ERR_OK;
}

int saveHobetaFile(Floppy* flp,int num,const char* dir) {
	TRFile dsc = flpGetCatalogEntry(flp,num);
	unsigned char* buf = new unsigned char[0xffff];
	if (!flpGetSectorsData(flp,dsc.trk, dsc.sec+1, buf, dsc.slen)) return ERR_TRD_SNF;	// get file data
	std::string name((char*)&dsc.name[0],8);
	size_t pos = name.find_last_not_of(' ');
	if (pos != std::string::npos) name = name.substr(0,pos+1);
	name = std::string(dir).append(std::string(SLASH)).append(name).append(std::string(".$")).append(std::string((char*)&dsc.ext,1));
	return saveHobeta(dsc,(char*)buf,name.c_str());
}
