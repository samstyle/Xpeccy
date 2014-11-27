#ifndef _XCORE_H
#define	_XCORE_H

#include <string>
#include <vector>

#include "../libxpeccy/spectrum.h"

#ifdef _WIN32
	#define	SLASH "\\"
#else
	#define	SLASH "/"
#endif

// common

struct OptName {
	int id;
	std::string name;
};

struct xConfig {
	unsigned sysclock:1;
	unsigned storePaths:1;
	unsigned defProfile:1;
	std::string keyMapName;
	int bright;
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

// extern ZXComp* zx;

std::string getTimeString(int);
std::string int2str(int);
void setFlagBit(bool, int*, int);
bool str2bool(std::string);
std::vector<std::string> splitstr(std::string,const char*);
std::pair<std::string,std::string> splitline(std::string);
void copyFile(const char*, const char*);

// bookmarks

typedef struct {
	std::string name;
	std::string path;
} XBookmark;

void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void clearBookmarks();
void swapBookmarks(int,int);
std::vector<XBookmark> getBookmarkList();
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
} RomSet;

bool addRomset(RomSet);
void rsSetRomset(ZXComp*, std::string);
RomSet* findRomset(std::string);
std::vector<RomSet> getRomsetList();
void setRomsetList(std::vector<RomSet>);

// profiles

typedef struct {
	std::string name;
	std::string file;
	std::string layName;
	std::string hwName;
	std::string rsName;
	ZXComp* zx;
} XProfile;

#define	DELP_ERR	-1
#define	DELP_OK		0
#define	DELP_OK_CURR	1

bool addProfile(std::string,std::string);
int delProfile(std::string);
bool selProfile(std::string);
void clearProfiles();
void prfSetRomset(std::string,std::string);
void prfLoadAll();
std::vector<XProfile> getProfileList();
XProfile* getCurrentProfile();
XProfile* getProfile(std::string);

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
} VidLayout;

bool addLayout(std::string,int,int,int,int,int,int,int,int,int);
bool addLayout(VidLayout);
std::vector<VidLayout> getLayoutList();
void setLayoutList(std::vector<VidLayout>);
bool emulSetLayout(ZXComp*, std::string);

#endif
