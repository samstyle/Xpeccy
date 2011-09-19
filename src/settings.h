#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

#include "spectrum.h"

#define	OPT_WORKDIR	0
#define	OPT_ROMDIR	1

struct optEntry {
	std::string group;
	std::string name;
	std::string value;
};

void initPaths();
void loadProfiles();
void saveProfiles();
void loadConfig(bool);
void saveConfig();
std::string optGetPath(int);

bool optGetBool(std::string,std::string);
int optGetInt(std::string,std::string);
std::string optGetString(std::string,std::string);

void optSet(std::string,std::string,bool);
void optSet(std::string,std::string,int);
void optSet(std::string,std::string,std::string);

#endif
