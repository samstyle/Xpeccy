#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

#include "spectrum.h"

struct MenuEntry {
	std::string name;
	std::string path;
};

class UserMenu {
	public:
		std::vector<MenuEntry> data;
		void add(std::string,std::string);
		void del(int32_t);
		void swap(int32_t,int32_t);
		void set(int32_t,std::string,std::string);
};

struct Profile {
	std::string name;
	std::string file;
	ZXComp* zx;
};

class Settings {
	public:
		Settings();
		int32_t sscnt,ssint;
		UserMenu umenu;		// bookmarks
		std::vector<Profile> profs;
		Profile* cprof;
		struct {
			std::string workDir,romDir,profPath;
		} opt;
		void addProfile(std::string,std::string);
		bool setProfile(std::string);
		void loadProfiles();
		void saveProfiles();
		void load(bool);
		void save();
};
//extern Settings *sets;

#endif
