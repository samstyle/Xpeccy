#include "filetypes.h"
#include <string.h>

int loadHobeta(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	uint8_t* buf = new uint8_t[256];
	TRFile nfle;
	int i;
	
	if (!flpGetFlag(flp,FLP_INSERT)) {
		flpFormat(flp);
		flpSetFlag(flp,FLP_INSERT,true);
	}
	if (flp->getDiskType() != TYPE_TRD) return ERR_NOTRD;
	
	file.read((char*)buf,17);		// header
	memcpy((char*)&nfle,buf,13);
	nfle.slen = buf[14];
	if (flp->createFile(&nfle) != ERR_OK) return ERR_HOB_CANT;
	for (i = 0; i < nfle.slen; i++) {
		file.read((char*)buf,256);
		if (!flp->putSectorData(nfle.trk, nfle.sec + 1, buf, 256)) return ERR_HOB_CANT;
		nfle.sec++;
		if (nfle.sec > 15) {
			nfle.trk++;
			nfle.sec -= 16;
		}
	}
	for (i=0; i<256; i++) flpFillFields(flp,i,true);
	return ERR_OK;
}
