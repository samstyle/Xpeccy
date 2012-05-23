#ifndef _XCORE_H
#define	_XCORE_H

#include <string>
#include <vector>

#include "../libxpeccy/spectrum.h"

#ifdef WIN32
	#define	SLASH "\\"
#else
	#define	SLASH "/"
#endif

// common

std::string getTimeString(int);
std::string int2str(int);

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
	struct {
		std::string path;
		uint8_t part;
	} roms[32];
} RomSet;

bool addRomset(RomSet);
void setRomsetList(std::vector<RomSet>);
void setRomset(ZXComp*, std::string);
std::vector<RomSet> getRomsetList();

// hardwares

void initHardware();
void setHardware(ZXComp*, std::string);
std::vector<std::string> getHardwareNames();
std::vector<HardWare> getHardwareList();

// profiles

typedef struct {
	std::string name;
	std::string file;
	std::string layName;
	ZXComp* zx;
	RomSet* rset;
} XProfile;

void addProfile(std::string,std::string);
bool setProfile(std::string);
void clearProfiles();
std::vector<XProfile> getProfileList();
XProfile* getCurrentProfile();

// liayouts

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
bool emulSetLayout(Video*, std::string);

#endif
