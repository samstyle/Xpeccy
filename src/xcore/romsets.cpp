#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "xcore.h"

xRomset* findRomset(std::string nm) {
	xRomset* res = NULL;
	for (int i=0; i < conf.rsList.size(); i++) {
		if (conf.rsList[i].name == nm) {
			res = &conf.rsList[i];
		}
	}
	return res;
}

bool rs_compare(const xRomset rs1, const xRomset rs2) {
	return (rs1.name < rs2.name);
}

void sortRomsetList() {
	std::sort(conf.rsList.begin(), conf.rsList.end(), rs_compare);
}

bool addRomset(xRomset rs) {
	if (findRomset(rs.name) != NULL) return false;
	conf.rsList.push_back(rs);
	sortRomsetList();
	return true;
}

void delRomset(int idx) {
	conf.rsList.erase(conf.rsList.begin() + idx);
	sortRomsetList();
}
