#include "xcore.h"

#include <stdio.h>

xLayout* findLayout(std::string nm) {
	xLayout* res = NULL;
	for (int i = 0; i < conf.layList.size(); i++) {
		if (conf.layList[i].name == nm) res = &conf.layList[i];
	}
	return res;
}

// todo: case insensitive

bool ly_compare(const xLayout lay1, const xLayout lay2) {
	if (lay1.name == "default") return true;		// allways on top
	if (lay2.name == "default") return false;
	return (lay1.name < lay2.name);
}

bool addLayout(std::string nm, vLayout vlay) {
	if (findLayout(nm) != NULL) return false;
	xLayout nlay;
	nlay.name = nm;
	nlay.lay = vlay;
	conf.layList.push_back(nlay);
	std::sort(conf.layList.begin(), conf.layList.end(), ly_compare);
	return true;
}

void rmLayout(std::string nm) {
	for (int i = 0; i < conf.layList.size(); i++) {
		if (conf.layList[i].name == nm) {
			conf.layList.erase(conf.layList.begin() + i);
		}
	}
	std::sort(conf.layList.begin(), conf.layList.end(), ly_compare);
}
