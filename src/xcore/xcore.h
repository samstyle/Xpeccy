#ifndef _XCORE_H
#define	_XCORE_H

#include <string>
#include <vector>
#include <map>

#include "../libxpeccy/spectrum.h"
#include "../libxpeccy/filetypes/filetypes.h"

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

// profiles

typedef struct {
	std::string name;
	std::string file;
	std::string layName;
	std::string hwName;
	std::string rsName;
	ZXComp* zx;
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

void initPaths();
void loadConfig();
void saveConfig();
void loadKeys();

extern std::map<std::string, int> shotFormat;

// keymap

typedef struct {
	const char* name;
	signed int key;		// qint32, nativeScanCode()
	unsigned char key1;	// key 1,2 -> Keyboard::map
	unsigned char key2;
	unsigned char key3;	// key 3,4 -> Keyboard::extMap (Profi)
	unsigned char key4;
	int keyCode;		// 0xXXYYZZ = ZZ,YY,XX in buffer (ZZ,YY,0xf0,XX if released)
} keyEntry;

void initKeyMap();
void setKey(const char*,const char,const char);
keyEntry getKeyEntry(signed int);

// bookmarks

typedef struct {
	std::string name;
	std::string path;
} xBookmark;

extern std::vector<xBookmark> bookmarkList;

void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void clearBookmarks();
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

extern std::vector<xRomset> rsList;

xRomset* findRomset(std::string);
bool addRomset(xRomset);

// layouts

typedef struct {
	std::string name;
	VSize full;
	VSize sync;
	VSize bord;
	VSize intpos;
	int intsz;
} xLayout;

extern std::vector<xLayout> layList;

bool addLayout(std::string,int,int,int,int,int,int,int,int,int);
bool addLayout(xLayout);
xLayout* findLayout(std::string);

// config

#define	YESNO(cnd) ((cnd) ? "yes" : "no")

struct xConfig {
	unsigned sysclock:1;		// system time in cmos
	unsigned storePaths:1;		// store tape/disk paths
	unsigned defProfile:1;		// start @ default profile
	std::string keyMapName;		// use this keymap
	float brdsize;			// 0.0 - 1.0 : border size
	struct {
		std::vector<xProfile*> list;
		xProfile* cur;
	} prof;
	struct {
		unsigned grayScale:1;
		unsigned noFlick:1;
		unsigned fullScreen:1;
		int scale;		// 1..4
	} vid;
	struct {
		unsigned enabled:1;
		unsigned mute:1;
		int rate;
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
	} led;
	struct {
		std::string confDir;
		std::string confFile;
		std::string romDir;
		std::string boot;
	} path;
};

extern xConfig conf;

#endif
