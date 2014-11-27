#ifndef _XPSETS
#define _XPSETS

#include <string>
#include <map>

#include "xcore/xcore.h"
#include "libxpeccy/spectrum.h"

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

#endif
