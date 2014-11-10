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

	std::string fnam(name);
	std::string ext;
	size_t pos = fnam.find_last_of(SLASH);
	if (pos != std::string::npos) fnam = fnam.substr(pos+1);		// get filename
	pos = fnam.find_last_of(".");
	if (pos != std::string::npos) {
		ext = fnam.substr(pos+1,3);
		fnam = fnam.substr(0,pos);		// cut extension
	}
	fnam.resize(8,' ');
	ext.resize(3,' ');

	TRFile nfle = flpMakeDescriptor(fnam.c_str(),ext.at(0),0,len);
	nfle.lst = ext.at(1);
	nfle.hst = ext.at(2);
	unsigned char buf[0x10000];
	file.read((char*)buf,len);
	file.close();
	if (flpCreateFile(flp, nfle, buf, len) != ERR_OK) return ERR_HOB_CANT;
	for (int i=0; i<256; i++) flpFillFields(flp,i,true);
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
