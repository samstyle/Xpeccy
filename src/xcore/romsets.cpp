#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "xcore.h"

std::vector<xRomset> rsList;

xRomset* findRomset(std::string nm) {
	xRomset* res = NULL;
	for (unsigned int i=0; i<rsList.size(); i++) {
		if (rsList[i].name == nm) {
			res = &rsList[i];
		}
	}
	return res;
}

bool addRomset(xRomset rs) {
	if (findRomset(rs.name) != NULL) return false;
	rsList.push_back(rs);
	return true;
}

/*
void setRomsetList(std::vector<xRomset> rsl) {
	rsList.clear();
	unsigned int i;
	for (i=0; i<rsl.size(); i++) {
		addRomset(rsl[i]);
	}
	std::vector<xProfile> profileList = getProfileList();
	for (i=0; i<profileList.size(); i++) {
		rsSetRomset(profileList[i].zx, profileList[i].rsName);
	}
}
*/

void rsSetRomset(ZXComp* comp, std::string nm) {
//	XProfile* prof = getProfile(pn);
//	if (prof == NULL) return;
//	printf("try to load romset %s\n",nm.c_str());
	xRomset* rset = findRomset(nm);
	int i;//,ad;
#ifdef _WIN32
	std::string romDir = std::string(".\\config\\roms");
#elif __linux
	std::string romDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy/roms";
#endif
	std::string fpath = "";
	std::ifstream file;
	char* pageBuf = new char[0x4000];
	int prts = 0;
//	if (rset == NULL) {
//		rset = findRomset(prof->rsName);
//	} else {
//		prof->rsName = rset->name;
//	}
	comp->mem->romMask = 0x03;
	if (rset == NULL) {
//		printf("none found\n");
		memset(pageBuf,0xff,0x4000);
		for (i=0; i<16; i++) {
			memSetPage(comp->mem,MEM_ROM,i,pageBuf);
		}
	} else {
		if (rset->file.size() != 0) {
//			printf("single file\n");
			fpath = romDir + SLASH + rset->file;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.seekg(0,std::ios_base::end);
				prts = file.tellg() / 0x4000;
				if (prts > 4) comp->mem->romMask = 0x07;
				if (prts > 8) comp->mem->romMask = 0x0f;
				if (prts > 16) comp->mem->romMask = 0x1f;
				if (prts > 32) prts = 32;
//				printf("mask %.2X\n",comp->mem->romMask);
				file.seekg(0,std::ios_base::beg);
				for (i = 0; i < prts; i++) {
					file.read(pageBuf,0x4000);
					memSetPage(comp->mem,MEM_ROM,i,pageBuf);
				}
				memset(pageBuf,0xff,0x4000);
				//for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
				for (i=prts; i<16; i++) memSetPage(comp->mem,MEM_ROM,i,pageBuf);
			} else {
				printf("Can't open single rom '%s'\n",rset->file.c_str());
				memset(pageBuf,0xff,0x4000);
				//for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
				for (i = 0; i < 16; i++) memSetPage(comp->mem,MEM_ROM,i,pageBuf);
			}
			file.close();
		} else {
			for (i = 0; i < 4; i++) {
				if (rset->roms[i].path == "") {
					memset(pageBuf,0xff,0x4000);
					// for (ad = 0; ad < 0x4000; ad++) pageBuf[ad]=0xff;
				} else {
					fpath = romDir + SLASH + rset->roms[i].path;
					file.open(fpath.c_str(),std::ios::binary);
					if (file.good()) {
						file.seekg(rset->roms[i].part << 14);
						file.read(pageBuf,0x4000);
					} else {
						printf("Can't open rom '%s:%i'\n",rset->roms[i].path.c_str(),rset->roms[i].part);
						memset(pageBuf,0xff,0x4000);
						//for (ad=0;ad<0x4000;ad++) pageBuf[ad]=0xff;
					}
					file.close();
				}
				memSetPage(comp->mem,MEM_ROM,i,pageBuf);
			}
		}
		memset(pageBuf,0xff,0x4000);
		// for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
		if (rset->gsFile.empty()) {
			gsSetRom(comp->gs,0,pageBuf);
			gsSetRom(comp->gs,1,pageBuf);
		} else {
			fpath = romDir + SLASH + rset->gsFile;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.read(pageBuf,0x4000);
				gsSetRom(comp->gs,0,pageBuf);
				file.read(pageBuf,0x4000);
				gsSetRom(comp->gs,1,pageBuf);
			} else {
	//			printf("Can't load gs rom '%s'\n",prof->gsFile.c_str());
				gsSetRom(comp->gs,0,pageBuf);
				gsSetRom(comp->gs,1,pageBuf);
			}
			file.close();
		}
		if (!rset->fntFile.empty()) {
			fpath = romDir + SLASH + rset->fntFile;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.read(pageBuf,0x800);
				vidSetFont(comp->vid,pageBuf);
			}
		}
	}
	free(pageBuf);
}

std::vector<xRomset>& getRomsetList() {
	return rsList;
}
