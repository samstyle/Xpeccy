#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

#include "spectrum.h"

struct optEntry {
	std::string group;
	std::string name;
	std::string value;
};

class Settings {
	public:
		Settings();
		struct {
			std::string workDir,romDir,profPath;
		} opt;
		void loadProfiles();
		void saveProfiles();
		void load(bool);
		void save();
};

bool optGetBool(std::string,std::string);
int optGetInt(std::string,std::string);
std::string optGetString(std::string,std::string);

void optSet(std::string,std::string,bool);
void optSet(std::string,std::string,int);
void optSet(std::string,std::string,std::string);

#endif
