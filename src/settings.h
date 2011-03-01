#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>

struct MenuEntry {
	std::string name;
	std::string path;
};

struct UserMenu {
	std::vector<MenuEntry> data;
	void add(std::string,std::string);
	void del(int);
	void swap(int,int);
	void set(int,std::string,std::string);
};

struct Settings {
	Settings();
	int sscnt,ssint;
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
extern Settings *sets;

#endif
