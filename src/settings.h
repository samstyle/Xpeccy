#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

#include "spectrum.h"

#define	OPT_WORKDIR	0
#define	OPT_ROMDIR	1
#define	OPT_SHOTDIR	0x10
#define OPT_SHOTFRM	0x11
#define	OPT_SHOTINT	0x12
#define	OPT_SHOTCNT	0x13
#define	OPT_BRGLEV	0x14
#define	OPT_PROJDIR	0x20
#define	OPT_ASMPATH	0x21

//screenshot format
#define	SCR_BMP		1
#define	SCR_PNG		2
#define	SCR_JPG		3
#define	SCR_SCR		4
#define	SCR_HOB		5
#define	SCR_DISK	6

struct optEntry {
	std::string group;
	std::string name;
	std::string value;
};

typedef struct {
	int id;
	std::string name;
} OptName;

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

std::string optGetName(int,int);
int optGetId(int,std::string);
OptName* getGetPtr(int prt);

bool optGetBool(std::string,std::string);
void optSet(std::string,std::string,std::string);

#endif
