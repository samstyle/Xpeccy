#include "xcore.h"

#include <stdio.h>

xLayout* findLayout(std::string nm) {
	xLayout* res = NULL;
	for (unsigned int i = 0; i < conf.layList.size(); i++) {
		if (conf.layList[i].name == nm) res = &conf.layList[i];
	}
	return res;
}

bool addLayout(std::string nm, vLayout vlay) {
	if (findLayout(nm) != NULL) return false;
	xLayout nlay;
	nlay.name = nm;
	nlay.lay = vlay;
	conf.layList.push_back(nlay);
	return true;
}
