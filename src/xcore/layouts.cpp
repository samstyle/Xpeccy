#include "xcore.h"
#include "../sound.h"

#include <stdio.h>

std::vector<VidLayout> layList;

bool addLayout(std::string nm,int fh,int fv,int bh,int bv,int sh,int sv,int ih,int iv,int is) {
//	printf("add Layout: %s\n",nm.c_str());
	for (unsigned int i = 0; i < layList.size(); i++) {
		if (layList[i].name == nm) return false;
	}
	if ((iv < 0) || (iv >= fv) || (ih < 0) || (ih >= fh) || (is < 1)) {
		printf("WARNING: Layout %s : INT position and/or length isn't correct\n",nm.c_str());
		iv = 0; ih = 0; is = 64;
	}
	VidLayout nlay;
	nlay.name = nm;
	nlay.full.h = fh;
	nlay.full.v = fv;
	nlay.bord.h = bh;
	nlay.bord.v = bv;
	nlay.sync.h = sh;
	nlay.sync.v = sv;
	nlay.intpos.h = ih;
	nlay.intpos.v = iv;
	nlay.intsz = is;
	layList.push_back(nlay);
	return true;
}

bool addLayout(VidLayout lay) {
//	printf("add Layout: %s\n",lay.name.c_str());
	for (unsigned int i = 0; i < layList.size(); i++) {
		if (layList[i].name == lay.name) return false;
	}
	layList.push_back(lay);
	return true;
}

void setLayoutList(std::vector<VidLayout> lst) {
	layList = lst;
}

std::vector<VidLayout> getLayoutList() {
	return layList;
}

// stop, shit here!
bool emulSetLayout(ZXComp* comp, std::string nm) {
	XProfile* currentProfile = getCurrentProfile();
	for (unsigned int i = 0; i < layList.size(); i++) {
		if (layList[i].name == nm) {
			currentProfile->layName = nm;
			zxSetLayout(comp,
				     layList[i].full.h, layList[i].full.v,
				     layList[i].bord.h, layList[i].bord.v,
				     layList[i].sync.h, layList[i].sync.v,
				     layList[i].intpos.h, layList[i].intpos.v, layList[i].intsz);
			sndCalibrate();
			return true;
		}
	}
	return false;
}
