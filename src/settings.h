#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

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

struct Settings {
	public:
	Settings();
	int32_t sscnt,ssint;
	bool wait;
	UserMenu umenu;
	std::string gsrom;
	std::string machname;
	std::string rsetname;
	std::string soutname;
	std::string ssdir,ssformat;
	std::string sjapath;
	std::string prjdir;
	std::string workdir,romdir,optpath;
	void load(bool);
	void save();
};
//extern Settings *sets;

#endif
