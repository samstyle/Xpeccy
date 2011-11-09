#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

#include "spectrum.h"

#define	OPT_WORKDIR	0
#define	OPT_ROMDIR	1
#define	OPT_SHOTDIR	0x10
#define OPT_SHOTEXT	0x11
#define	OPT_SHOTINT	0x12
#define	OPT_SHOTCNT	0x13
#define	OPT_BRGLEV	0x14
#define	OPT_PROJDIR	0x20
#define	OPT_ASMPATH	0x21

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
std::string optGetString(int);
int optGetInt(int);
void optSet(int,std::string);
void optSet(int,int);
void optSet(int,bool);

bool optGetBool(std::string,std::string);
//int optGetInt(std::string,std::string);
//std::string optGetString(std::string,std::string);

//void optSet(std::string,std::string,bool);
//void optSet(std::string,std::string,int);
void optSet(std::string,std::string,std::string);

#endif
