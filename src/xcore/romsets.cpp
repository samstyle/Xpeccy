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

bool addRomset(xRomset rs) {
	if (findRomset(rs.name) != NULL) return false;
	conf.rsList.push_back(rs);
	return true;
}
