#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
//#include <stdint.h>

#include "libxpeccy/spectrum.h"

#define	OPT_WORKDIR	0
#define	OPT_ROMDIR	1
#define	OPT_FLAG	8
#define	OPT_SHOTDIR	0x10
#define OPT_SHOTFRM	0x11
#define	OPT_SHOTINT	0x12
#define	OPT_SHOTCNT	0x13
#define	OPT_BRGLEV	0x14
#define	OPT_PROJDIR	0x20
#define	OPT_ASMPATH	0x21
#define OPT_JOYNAME	0x30
#define OPT_JOYDIRS	0x31
#define	OPT_KEYNAME	0x40

//screenshot format
#define	SCR_BMP		1
#define	SCR_PNG		2
#define	SCR_JPG		3
#define	SCR_SCR		4
#define	SCR_HOB		5
#define	SCR_DISK	6

// flags
#define	OF_TAPEAUTO	1
#define	OF_TAPEFAST	(1<<1)
#define	OF_DEFAULT	(1<<2)
#define	OF_PATHS	(1<<3)
#define	OF_FASTDISK	(1<<4)

struct optEntry {
	std::string group;
	std::string name;
	std::string value;
};

struct OptName {
	int id;
	std::string name;
};

#define XJ_NONE		0
#define	XJ_BUTTON	1
#define	XJ_AXIS		2
#define	XJ_KEY		3
#define	XJ_JOY		4

struct extButton {
	int type;		// button / axis
	int num;		// number of button or axis
	bool dir;		// direction of axis
	bool operator == (extButton);
};

struct intButton {
	int dev;		// keyboard / joystick
	const char* name;	// key name / joystick direction
};

typedef std::pair<extButton,intButton> joyPair;

void initPaths();
void loadProfiles();
void saveProfiles();
void loadKeys();

std::string optGetString(int);
int optGetInt(int);
void optSet(int,std::string);
void optSet(int,int);
void optSet(int,bool);

void optSetFlag(int,bool);
bool optGetFlag(int);

std::vector<joyPair> getJMap();
void setJMap(std::vector<joyPair>);
intButton optGetJMap(extButton);
void optSetJMap(extButton,intButton);
void optDelJMap(extButton);

std::string optGetName(int,int);
int optGetId(int,std::string);
OptName* getGetPtr(int prt);

#endif
