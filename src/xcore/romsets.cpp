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
