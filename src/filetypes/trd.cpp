#include "filetypes.h"
#include "../settings.h"

void loadBoot(Floppy* flp) {
	if (flpGet(flp,FLP_DISKTYPE) == DISK_TYPE_TRD) {
		TRFile cat[128];
		int catSize = flpGetTRCatalog(flp,cat);
//		std::vector<TRFile> cat = flpGetTRCatalog(flp);
		bool gotBoot = false;
		for (int i=0; i < catSize; i++) {
			if (std::string((const char*)&cat[i].name[0],9) == "boot    B") gotBoot = true;
		}
		if (!gotBoot) {
			std::string path = conf.path.confDir + SLASH + "boot.$B";
			loadHobeta(flp,path.c_str());
		}
	}
}

int loadTRD(Floppy* flp, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	file.seekg(0,std::ios::end);
	size_t len = file.tellg();
	if (((len & 0xfff) != 0) || (len == 0) || (len > 0xa8000)) return ERR_TRD_LEN;
//	file.seekg(0x8e7,std::ios::beg);
//	if (file.peek() != 0x10) return ERR_TRD_SIGN;
	file.seekg(0);
	flpFormat(flp);
	int i=0;
	unsigned char* trackBuf = new unsigned char[0x1000];
	do {
		file.read((char*)trackBuf,0x1000);
		flpFormTRDTrack(flp,i,trackBuf);
		i++;
	} while  (!file.eof());
	delete(trackBuf);
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->insert = 1;
	loadBoot(flp);
	flp->changed = 0;
	return ERR_OK;
}

int saveTRD(Floppy* flp, const char* name) {
	unsigned char* img = new unsigned char[0xa0000];
	unsigned char* dptr = img;
	for (int i = 0; i < 160; i++) {
		for (int j = 1; j < 17; j++) {
			if (!flpGetSectorData(flp,i,j,dptr,256)) {
				delete(img);
				return ERR_TRD_SNF;
			}
			dptr+=256;
		}
	}
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) {
		delete(img);
		return ERR_CANT_OPEN;
	}
	file.write((char*)img,0xa0000);
	file.close();
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->changed = 0;
	delete(img);
	return ERR_OK;
}
