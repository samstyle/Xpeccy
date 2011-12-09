#include <string.h>
#include "filetypes.h"

int loadRaw(Floppy* flp, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	file.seekg(0,std::ios::end);
	size_t len = file.tellg();
	if (len > 0xff00) return ERR_RAW_LONG;
	if (!flpGetFlag(flp,FLP_INSERT)) {
		flpFormat(flp);
		flpSetFlag(flp,FLP_INSERT,true);
	}
	if (flp->getDiskType() != TYPE_TRD) return ERR_NOTRD;
	file.seekg(0,std::ios::beg);
	TRFile nfle;
	
	std::string fnam(name);
	size_t pos = fnam.find_last_of("/");
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
	nfle.slen = nfle.hlen;
	if (nfle.llen != 0) nfle.slen++;
	if (flp->createFile(&nfle) != ERR_OK) return ERR_HOB_CANT;
	int i;
	uint8_t* buf = new uint8_t[256];
	for (i = 0; i < nfle.slen; i++) {
		file.read((char*)buf,256);;;
		if (!flp->putSectorData(nfle.trk, nfle.sec + 1, buf, 256)) return ERR_HOB_CANT;
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