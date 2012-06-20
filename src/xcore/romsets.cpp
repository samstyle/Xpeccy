#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "xcore.h"

std::vector<RomSet> rsList;

RomSet* findRomset(std::string nm) {
	RomSet* res = NULL;
	for (unsigned int i=0; i<rsList.size(); i++) {
		if (rsList[i].name == nm) {
			res = &rsList[i];
		}
	}
	return res;
}

bool addRomset(RomSet rs) {
	if (findRomset(rs.name) != NULL) return false;
	rsList.push_back(rs);
	return true;
}

void setRomsetList(std::vector<RomSet> rsl) {
	rsList.clear();
	unsigned int i;
	for (i=0; i<rsl.size(); i++) {
		addRomset(rsl[i]);
	}
	std::vector<XProfile> profileList = getProfileList();
	for (i=0; i<profileList.size(); i++) {
		setRomset(profileList[i].zx, profileList[i].rsName);
//		profileList[i].zx->mem->loadromset(optGetString(OPT_ROMDIR));
	}
}

void setRomset(ZXComp* comp, std::string nm) {
	XProfile* curProf = getCurrentProfile();
	RomSet* rset = findRomset(nm);
	int i,ad;
#ifdef WIN32
	std::string romDir = std::string(".\\config\\roms");
#else
	std::string romDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy/roms";
#endif
	std::string fpath = "";
	std::ifstream file;
	char* pageBuf = new char[0x4000];
	int prts = 0;
	int profMask = 0;
	if (rset == NULL) {
		rset = findRomset(curProf->rsName);
	} else {
		curProf->rsName = rset->name;
	}
	if (rset == NULL) {
		for (i=0; i<16; i++) {
			for (ad=0; ad<0x4000; ad++) pageBuf[i] = 0xff;
			memSetPage(comp->mem,MEM_ROM,i,pageBuf);
		}
	} else {
		if (rset->file.size() != 0) {
			fpath = romDir + SLASH + rset->file;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.seekg(0,std::ios_base::end);
				prts = file.tellg() / 0x4000;
				profMask = 3;
				if (prts < 9) profMask = 1;
				if (prts < 5) profMask = 0;
				if (prts > 16) prts = 16;
				file.seekg(0,std::ios_base::beg);
				comp->mem->profMask = profMask;
				for (i = 0; i < prts; i++) {
					file.read(pageBuf,0x4000);
					memSetPage(comp->mem,MEM_ROM,i,pageBuf);
				}
				for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
				for (i=prts; i<16; i++) memSetPage(comp->mem,MEM_ROM,i,pageBuf);
			} else {
				printf("Can't open single rom '%s'\n",rset->file.c_str());
				for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
				for (i = 0; i < 16; i++) memSetPage(comp->mem,MEM_ROM,i,pageBuf);
			}
			file.close();
		} else {
			for (i = 0; i < 4; i++) {
				if (rset->roms[i].path == "") {
					for (ad = 0; ad < 0x4000; ad++) pageBuf[ad]=0xff;
				} else {
					fpath = romDir + SLASH + rset->roms[i].path;
					file.open(fpath.c_str(),std::ios::binary);
					if (file.good()) {
						file.seekg(rset->roms[i].part<<14);
						file.read(pageBuf,0x4000);
					} else {
						printf("Can't open rom '%s:%i'\n",rset->roms[i].path.c_str(),rset->roms[i].part);
						for (ad=0;ad<0x4000;ad++) pageBuf[ad]=0xff;
					}
					file.close();
				}
				memSetPage(comp->mem,MEM_ROM,i,pageBuf);
			}
		}
	}
	for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
	if (curProf->gsFile.empty()) {
		gsSetRom(comp->gs,0,pageBuf);
		gsSetRom(comp->gs,1,pageBuf);
	} else {
		fpath = romDir + SLASH + curProf->gsFile;
		file.open(fpath.c_str(),std::ios::binary);
		if (file.good()) {
			file.read(pageBuf,0x4000);
			gsSetRom(comp->gs,0,pageBuf);
			file.read(pageBuf,0x4000);
			gsSetRom(comp->gs,1,pageBuf);
		} else {
			printf("Can't load gs rom '%s'\n",curProf->gsFile.c_str());
			gsSetRom(comp->gs,0,pageBuf);
			gsSetRom(comp->gs,1,pageBuf);
		}
		file.close();
	}
	free(pageBuf);
}

std::vector<RomSet> getRomsetList() {
	return rsList;
}
