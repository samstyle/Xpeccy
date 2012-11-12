#ifndef _XCORE_H
#define	_XCORE_H

#include <string>
#include <vector>

#include "../libxpeccy/spectrum.h"
#include "../emulwin.h"

#ifdef _WIN32
	#define	SLASH "\\"
#else
	#define	SLASH "/"
#endif

// common

extern ZXComp* zx;

std::string getTimeString(int);
std::string int2str(int);
void setFlagBit(bool, int*, int);
bool str2bool(std::string);
std::vector<std::string> splitstr(std::string,const char*);
std::pair<std::string,std::string> splitline(std::string);

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
void setRomset(std::string, std::string);
RomSet* findRomset(std::string);
std::vector<RomSet> getRomsetList();
void setRomsetList(std::vector<RomSet>);

// hardwares

#define	MEM_48	0
#define	MEM_128	1
#define	MEM_256 (1<<1)
#define	MEM_512	(1<<2)
#define	MEM_1M	(1<<3)
#define	MEM_2M	(1<<4)
#define	MEM_4M	(1<<5)

void initHardware();
void setHardware(ZXComp*, std::string);
std::vector<std::string> getHardwareNames();
std::vector<HardWare> getHardwareList();

// profiles

typedef struct {
	std::string name;
	std::string file;
	std::string layName;
	std::string hwName;
	std::string rsName;
//	std::string gsFile;
	ZXComp* zx;
} XProfile;

#define	DELP_ERR	-1
#define	DELP_OK		0
#define	DELP_OK_CURR	1

bool addProfile(std::string,std::string);
int delProfile(std::string);
bool setProfile(std::string);
void clearProfiles();
std::vector<XProfile> getProfileList();
XProfile* getCurrentProfile();
XProfile* getProfile(std::string);

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
bool emulSetLayout(Video*, std::string);

#endif
