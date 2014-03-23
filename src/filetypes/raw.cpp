#include <string.h>
#include "filetypes.h"

int loadRaw(Floppy* flp, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	file.seekg(0,std::ios::end);
	size_t len = file.tellg();
	if (len > 0xff00) return ERR_RAW_LONG;
	if (!flp->insert) {
		flpFormat(flp);
		flp->insert = 1;
	}
	if (flpGet(flp,FLP_DISKTYPE) != DISK_TYPE_TRD) return ERR_NOTRD;
	file.seekg(0,std::ios::beg);
	TRFile nfle;

	std::string fnam(name);
	size_t pos = fnam.find_last_of(SLASH);
	if (pos != std::string::npos) fnam = fnam.substr(pos+1);		// get filename
	pos = fnam.find_last_of(".");
	if (pos != std::string::npos) fnam = fnam.substr(0,pos);		// cut extension
	fnam.resize(8,' ');

	memcpy((char*)&nfle.name[0],fnam.c_str(),8);
	nfle.ext = 'C';
	nfle.lst = 0;
	nfle.hst = 0;
	nfle.llen = len & 0xff;
	nfle.hlen = (len & 0xff00) >> 8;
	nfle.slen = nfle.hlen + (nfle.llen ? 1 : 0);
	if (flpCreateFile(flp,&nfle) != ERR_OK) return ERR_HOB_CANT;
	int i;
	unsigned char* buf = new unsigned char[256];
	for (i = 0; i < nfle.slen; i++) {
		file.read((char*)buf,256);
		if (!flpPutSectorData(flp, nfle.trk, nfle.sec + 1, buf, 256)) return ERR_HOB_CANT;
		nfle.sec++;
		if (nfle.sec > 15) {
			nfle.trk++;
			nfle.sec -= 16;
		}
	}
	for (i=0; i<256; i++) flpFillFields(flp,i,true);
	delete(buf);

	return ERR_OK;
}

int saveRawFile(Floppy* flp, int num, const char* dir) {
	TRFile dsc = flpGetCatalogEntry(flp,num);
	unsigned char* buf = new unsigned char[0xffff];
	if (!flpGetSectorsData(flp,dsc.trk, dsc.sec+1, buf, dsc.slen)) return ERR_TRD_SNF;
	std::string name((char*)&dsc.name[0],8);
	size_t pos = name.find_last_not_of(' ');
	if (pos != std::string::npos) name = name.substr(0,pos+1);
	name = std::string(dir).append(std::string(SLASH)).append(name).append(std::string(".")).append(std::string((char*)&dsc.ext,1));
	std::ofstream file(name.c_str(),std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	int len;
	if (dsc.slen == (dsc.hlen + ((dsc.llen == 0) ? 0 : 1))) {
		len = (dsc.hlen << 8) + dsc.llen;
	} else {
		len = (dsc.slen << 8);
	}
	file.write((char*)buf,len);
	file.close();
	delete(buf);
	return ERR_OK;
}
