#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <vector>
#include <stdint.h>

#include "spectrum.h"

struct IniEntry {
	std::string group;
	std::string name;
	std::string value;
};

class Settings {
	public:
		Settings();
		int32_t sscnt,ssint;
		struct {
			std::string sndOutputName;
			std::string scrshotDir,scrshotFormat;
			std::string workDir,romDir,profPath;
			std::string asmPath,projectsDir;
		} opt;
		void loadProfiles();
		void saveProfiles();
		void load(bool);
		void save();
};

#endif
