#ifndef _XCORE_H
#define	_XCORE_H

#include <string>
#include <vector>
#include <map>

#include "../libxpeccy/spectrum.h"

#ifdef _WIN32
	#define	SLASH "\\"
#else
	#define	SLASH "/"
#endif

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

// config

#define	YESNO(cnd) ((cnd) ? "yes" : "no")

struct xConfig {
	unsigned sysclock:1;
	unsigned storePaths:1;
	unsigned defProfile:1;
	std::string keyMapName;
	float brdsize;
	struct {
		unsigned grayScale:1;
		unsigned noFlick:1;
		unsigned fullScreen:1;
		unsigned doubleSize:1;
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
		int count;
		int interval;
		std::string format;
		std::string dir;
	} scrShot;
	struct {
		unsigned mouse:1;
		unsigned joy:1;
		unsigned keys:1;
	} led;
	struct {
		std::string confDir;
		std::string confFile;
		std::string romDir;
	} path;
};


extern xConfig conf;

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
	char key1;
	char key2;
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

void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void clearBookmarks();
void swapBookmarks(int,int);
std::vector<xBookmark> getBookmarkList();
int getBookmarksCount();

// romsets

typedef struct {
	std::string name;
	std::string file;	// set when romfile is single file
	std::string gsFile;
	std::string fntFile;
	int mask;
	struct {
		std::string path;
		unsigned char part;
	} roms[32];
} xRomset;

extern std::vector<xRomset> rsList;

bool addRomset(xRomset);
void rsSetRomset(ZXComp*, std::string);
xRomset* findRomset(std::string);
// std::vector<xRomset>* getRomsetList();
// void setRomsetList(std::vector<xRomset>);

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

bool addProfile(std::string,std::string);
int delProfile(std::string);
bool selProfile(std::string);
void clearProfiles();
void prfSetRomset(std::string,std::string);
void prfLoadAll();
std::vector<xProfile> getProfileList();
xProfile* getCurrentProfile();
xProfile* findProfile(std::string);
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

#endif
