#include "xcore.h"

#include <stdio.h>

xLayout* findLayout(std::string nm) {
	xLayout* res = NULL;
	for (unsigned int i = 0; i < conf.layList.size(); i++) {
		if (conf.layList[i].name == nm) res = &conf.layList[i];
	}
	return res;
}

bool addLayout(std::string nm,int fh,int fv,int bh,int bv,int sh,int sv,int ih,int iv,int is) {
//	printf("add Layout: %s\n",nm.c_str());
	if (findLayout(nm) != NULL) return false;
	if ((iv < 0) || (iv >= fv) || (ih < 0) || (ih >= fh) || (is < 1)) {
		printf("WARNING: Layout %s : INT position and/or length isn't correct\n",nm.c_str());
		iv = 0; ih = 0; is = 64;
	}
	xLayout nlay;
	nlay.name = nm;
	nlay.full.x = fh;
	nlay.full.y = fv;
	nlay.bord.x = bh;
	nlay.bord.y = bv;
	nlay.sync.x = sh;
	nlay.sync.y = sv;
	nlay.intpos.x = ih;
	nlay.intpos.y = iv;
	nlay.intsz = is;
	conf.layList.push_back(nlay);
	return true;
}

bool addLayout(xLayout lay) {
//	printf("add Layout: %s (%i x %i)\n",lay.name.c_str(), lay.full.h, lay.full.v);
	if (findLayout(lay.name) != NULL) return false;
	conf.layList.push_back(lay);
	return true;
}
