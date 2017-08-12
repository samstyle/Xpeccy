#ifndef XCORE_H
#define	XCORE_H

#include <string>
#include <vector>
#include <map>
#include <SDL_joystick.h>
#include <QString>

#include "spectrum.h"
#include "filetypes.h"

// common

std::string getTimeString(int);
std::string int2str(int);
std::string float2str(float);
int getRanged(const char*, int, int);
void setFlagBit(bool, int*, int);
bool str2bool(std::string);
std::vector<std::string> splitstr(std::string,const char*);
std::pair<std::string,std::string> splitline(std::string);
void copyFile(const char*, const char*);

QString getbinbyte(uchar);
QString gethexshift(char);
QString getdecshift(char);
QString gethexbyte(uchar);
QString gethexword(int);

typedef struct {
	int b:1;
	int i;
	const char* s;
} xArg;

// brk points

enum {
	BRK_IOPORT = 1,
	BRK_CPUADR,
	BRK_MEMCELL,
	BRK_MEMRAM,
	BRK_MEMROM,
	BRK_MEMSLT,
	BRK_MEMEXT
};

typedef struct {
	unsigned off:1;
	unsigned fetch:1;
	unsigned read:1;
	unsigned write:1;
	unsigned block:1;
	int type;
	int adr;
	int size;	// size of block
	int mask;	// io: if (port & mask == adr & mask)
} xBrkPoint;

void brkSet(int, int, int, int);
void brkXor(int, int, int, int);
void brkAdd(xBrkPoint);
void brkInstall(xBrkPoint);
void brkDelete(xBrkPoint);
void brkInstallAll();

// profiles

typedef struct {
	std::string name;
 	std::string file;
	std::string layName;
	std::string hwName;
	std::string rsName;
	std::string jmapName;
	std::vector<xBrkPoint> brkList;
	Computer* zx;
} xProfile;

#define	DELP_ERR	-1
#define	DELP_OK		0
#define	DELP_OK_CURR	1

xProfile* findProfile(std::string);
bool addProfile(std::string,std::string);
int delProfile(std::string);
void clearProfiles();
void prfLoadAll();
bool prfSetCurrent(std::string);
void prfSetRomset(xProfile*, std::string);
bool prfSetLayout(xProfile*, std::string);

void prfChangeRsName(std::string, std::string);
void prfChangeLayName(std::string, std::string);

void prfFillBreakpoints(xProfile*);

#define	PLOAD_OK	0
#define	PLOAD_NF	1
#define	PLOAD_OF	2
#define	PLOAD_HW	3
#define	PLOAD_RS	4

int prfLoad(std::string);

#define PSAVE_OK	PLOAD_OK
#define	PSAVE_NF	PLOAD_NF
#define	PSAVE_OF	PLOAD_OF

int prfSave(std::string);

//screenshot format
#define	SCR_BMP		1
#define	SCR_PNG		2
#define	SCR_JPG		3
#define	SCR_SCR		4
#define	SCR_HOB		5
#define	SCR_DISK	6

void initPaths(char*);
void loadConfig();
void saveConfig();
void loadKeys();

extern std::map<std::string, int> shotFormat;

// keymap

void initKeyMap();
void setKey(const char*,const char,const char);
keyEntry getKeyEntry(int);
int getKeyIdByName(const char*);
const char* getKeyNameById(int);
int qKey2id(int);
int key2qid(int);

// bookmarks

typedef struct {
	std::string name;
	std::string path;
} xBookmark;

void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void swapBookmarks(int,int);

// romsets

typedef struct {
	std::string name;
	std::string file;	// set when romfile is single file
	std::string gsFile;
	std::string fntFile;
	struct {
		std::string path;
		unsigned char part;
	} roms[32];
} xRomset;

xRomset* findRomset(std::string);
bool addRomset(xRomset);

// layouts

typedef struct {
	std::string name;
	vLayout lay;
} xLayout;

bool addLayout(std::string, vLayout);
xLayout* findLayout(std::string);

// joystick

enum {
	JOY_NONE = 0,
	JOY_AXIS,
	JOY_BUTTON,
	JOY_HAT
};

enum {
	JMAP_NONE = 0,
	JMAP_KEY,
	JMAP_JOY,
	JMAP_MOUSE
};

typedef struct {
	int type;		// axis/button
	int num;		// number of axis/button
	int state;		// -x/+x for axis, 0/x for button
	int dev;		// device for action JMAP_*
	int key;		// key XKEY_* for keyboard
	int dir;		// XJ_* for kempston
} xJoyMapEntry;

void mapJoystick(Computer*, int, int, int);

// config

#define	YESNO(cnd) ((cnd) ? "yes" : "no")

struct xConfig {
	unsigned running:1;
	unsigned storePaths:1;		// store tape/disk paths
	unsigned defProfile:1;		// start @ default profile
	std::string keyMapName;		// use this keymap
	float brdsize;			// 0.0 - 1.0 : border size
	std::vector<xRomset> rsList;
	std::vector<xLayout> layList;
	std::vector<xBookmark> bookmarkList;
	struct {
		std::vector<xProfile*> list;
		xProfile* cur;
	} prof;
	struct {
		unsigned grayScale:1;
		unsigned noFlick:1;
		unsigned fullScreen:1;	// use fullscreen
		unsigned keepRatio:1;	// keep ratio in fullscreen (add black borders)
		int scale;		// x1..x4
	} vid;
	struct {
		unsigned enabled:1;
		unsigned mute:1;
		unsigned fill:1;	// 1 while snd buffer not filled, 0 at end of snd buffer
		int rate;
		int chans;
		struct {
			int beep;
			int tape;
			int ay;
			int gs;
		} vol;
	} snd;
	struct {
		unsigned autostart:1;
		unsigned fast:1;
	} tape;
	struct {
		SDL_Joystick* joy;
		int dead;
		std::vector<xJoyMapEntry> map;	// gamepad map for current profile
	} joy;
	struct {
		unsigned noLeds:1;
		unsigned noBorder:1;
		int count;
		int interval;
		std::string format;
		std::string dir;
	} scrShot;
	struct {
		unsigned mouse:1;
		unsigned joy:1;
		unsigned keys:1;
		unsigned tape:1;
		unsigned disk:1;
		unsigned message:1;
	} led;
	struct {
		std::string lastDir;
		std::string confDir;
		std::string confFile;
		std::string romDir;
		std::string font;
		std::string boot;
	} path;
	struct {
		unsigned labels:1;
		unsigned segment:1;
		unsigned hideadr:1;
	} dbg;
};

extern xConfig conf;

#endif
