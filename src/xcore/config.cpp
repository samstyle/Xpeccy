#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

#include <QtCore>

#include "xcore.h"
#include "../xgui/xgui.h"
#include "../sound.h"
//#include "../emulwin.h"

#ifdef _WIN32
	#include <direct.h>
#endif

#define	SECT_NONE	0
#define SECT_BOOKMARK	1
#define	SECT_PROFILES	2
#define	SECT_VIDEO	3
#define	SECT_ROMSETS	4
#define	SECT_SOUND	5
#define	SECT_TOOLS	6
#define	SECT_JOYSTICK	7
#define	SECT_GENERAL	8
#define	SECT_SCRSHOT	9
#define	SECT_DISK	10
#define	SECT_IDE	11
#define	SECT_MACHINE	12
#define	SECT_MENU	13
#define	SECT_TAPE	14
#define	SECT_LEDS	15
#define	SECT_INPUT	16
#define	SECT_SDC	17

std::map<std::string, int> shotFormat;
xConfig conf;

void initPaths() {
#if __linux
	conf.path.confDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy";
	conf.path.romDir = conf.path.confDir + "/roms";
	conf.path.confFile = conf.path.confDir + "/config.conf";
	conf.scrShot.dir = std::string(getenv("HOME"));
	mkdir(conf.path.confDir.c_str(),0777);
	mkdir(conf.path.romDir.c_str(),0777);
#endif
#ifdef __WIN32
	conf.path.confDir = std::string(".\\config");
	conf.path.romDir = conf.path.confDir + "\\roms";
	conf.path.confFile = conf.path.confDir + "\\config.conf";
	conf.scrShot.dir = std::string(getenv("HOMEPATH"));
	mkdir(conf.path.confDir.c_str());
	mkdir(conf.path.romDir.c_str());
#endif
}

void saveConfig() {
	std::ofstream cfile(conf.path.confFile.c_str());
	if (!cfile.good()) {
		shitHappens("Can't write main config");
		throw(0);
	}
	uint i,j;

	cfile << "[GENERAL]\n\n";
	if ((conf.keyMapName != "default") && (conf.keyMapName != "")) {
		cfile << "keys = " << conf.keyMapName.c_str() << "\n";
	}
	cfile << "startdefault = " << YESNO(conf.defProfile) << "\n";
	cfile << "savepaths = " << YESNO(conf.storePaths) << "\n";
	cfile << "fdcturbo = " << YESNO(fdcFlag & FDC_FAST) << "\n";
	cfile << "systime = " << YESNO(conf.sysclock) << "\n";

	cfile << "\n[BOOKMARKS]\n\n";
	std::vector<xBookmark> bml = getBookmarkList();
	for (i=0; i<bml.size(); i++) {
		cfile << bml[i].name << " = " << bml[i].path << "\n";
	}
	cfile << "\n[PROFILES]\n\n";
	std::vector<xProfile> prl = getProfileList();
	for (i=1; i<prl.size(); i++) {			// nr.0 skipped ('default' profile)
		cfile << prl[i].name << " = " << prl[i].file << "\n";
	}
	cfile << "current = " << getCurrentProfile()->name << "\n";

	cfile << "\n[VIDEO]\n\n";
	for (i=1; i < layList.size(); i++) {
		cfile << "layout = ";
		cfile << layList[i].name.c_str() << ":";
		cfile << int2str(layList[i].full.h) << ":";
		cfile << int2str(layList[i].full.v) << ":";
		cfile << int2str(layList[i].bord.h) << ":";
		cfile << int2str(layList[i].bord.v) << ":";
		cfile << int2str(layList[i].sync.h) << ":";
		cfile << int2str(layList[i].sync.v) << ":";
		cfile << int2str(layList[i].intsz) << ":";
		cfile << int2str(layList[i].intpos.v) << ":";
		cfile << int2str(layList[i].intpos.h) << "\n";
	}
	cfile << "scrDir = " << conf.scrShot.dir.c_str() << "\n";
	cfile << "scrFormat = " << conf.scrShot.format.c_str() << "\n";
	cfile << "scrCount = " << int2str(conf.scrShot.count) << "\n";
	cfile << "scrInterval = " << int2str(conf.scrShot.interval) << "\n";
	cfile << "fullscreen = " << YESNO(conf.vid.fullScreen) << "\n";
	cfile << "doublesize = " << YESNO(conf.vid.doubleSize) << "\n";
	cfile << "greyscale = " << YESNO(conf.vid.grayScale) << "\n";
	cfile << "bordersize = " << int2str(conf.brdsize * 100) << "\n";
	cfile << "noflic = " << YESNO(conf.vid.noFlick) << "\n";
	cfile << "\n[ROMSETS]\n";
	for (i=0; i<rsList.size(); i++) {
		cfile<< "\nname = " << rsList[i].name.c_str() << "\n";
		if (rsList[i].file != "") {
			cfile << "file = " << rsList[i].file.c_str() << "\n";
		} else {
			for (j=0; j<4; j++) {
				if (rsList[i].roms[j].path != "") {
					cfile << int2str(j) << " = " << rsList[i].roms[j].path.c_str();
					if (rsList[i].roms[j].part != 0) cfile << ":" << int2str(rsList[i].roms[j].part);
					cfile << "\n";
				}
			}
		}
		if (!rsList[i].gsFile.empty()) cfile << "gs = " << rsList[i].gsFile.c_str() << "\n";
		if (!rsList[i].fntFile.empty()) cfile << "font = " << rsList[i].fntFile.c_str() << "\n";
	}
	cfile << "\n[SOUND]\n\n";
	cfile << "enabled = " << YESNO(conf.snd.enabled) << "\n";
	cfile << "dontmute = " << YESNO(conf.snd.mute) << "\n";
	cfile << "soundsys = " << sndOutput->name << "\n";
	cfile << "rate = " << int2str(conf.snd.rate) << "\n";
	cfile << "volume.beep = " << int2str(conf.snd.vol.beep) << "\n";
	cfile << "volume.tape = " << int2str(conf.snd.vol.tape) << "\n";
	cfile << "volume.ay = " << int2str(conf.snd.vol.ay) << "\n";
	cfile << "volume.gs = " << int2str(conf.snd.vol.gs) << "\n";

	cfile << "\n[TAPE]\n\n";
	cfile << "autoplay = " << YESNO(conf.tape.autostart) << "\n";
	cfile << "fast = " << YESNO(conf.tape.fast) << "\n";

	cfile << "\n[LEDS]\n\n";
	cfile << "mouse = " << YESNO(conf.led.mouse) << "\n";
	cfile << "joystick = " << YESNO(conf.led.joy) << "\n";
	cfile << "keyscan = " << YESNO(conf.led.keys) << "\n";
	cfile.close();
}


void loadKeys() {
	std::string sfnam = conf.path.confDir + SLASH + conf.keyMapName;
	initKeyMap();
	if ((conf.keyMapName == "") || (conf.keyMapName == "default")) return;
	std::ifstream file(sfnam.c_str());
	if (!file.good()) {
		printf("Can't open keyboard layout. Default one will be used\n");
		return;
	}
	char* buf = new char[1024];
	std::pair<std::string,std::string> spl;
	std::string line;
	std::vector<std::string> vec;
	char key1;
	char key2;
	while (!file.eof()) {
		file.getline(buf,1023);
		line = std::string(buf);
		vec = splitstr(line,"\t");
		if (vec.size() > 1) {
			if (vec.size() == 2) vec.push_back("");
			key1 = (vec[1].size() > 0) ? vec[1].at(0) : 0;
			key2 = (vec[2].size() > 0) ? vec[2].at(0) : 0;
			setKey(vec[0].c_str(),key1,key2);
		}
	}
	free(buf);
}

void copyFile(const char* src, const char* dst) {
	QFile fle(src);
	fle.open(QFile::ReadOnly);
	QByteArray fdata = fle.readAll();
	fle.close();
	fle.setFileName(dst);
	if (fle.open(QFile::WriteOnly)) {
		fle.write(fdata);
		fle.close();
	}
}

// emulator config

void loadConfig() {
	std::string soutnam = "NULL";
	std::ifstream file(conf.path.confFile.c_str());
	if (!file.good()) {
		printf("Main config is missing. Default files will be copied\n");
		copyFile(":/conf/config.conf",std::string(conf.path.confFile).c_str());
		copyFile(":/conf/xpeccy.conf",std::string(conf.path.confDir + SLASH + "xpeccy.conf").c_str());
		copyFile(":/conf/1982.rom",std::string(conf.path.romDir + SLASH + "1982.rom").c_str());
		file.open(conf.path.confFile.c_str());
		if (!file.good()) {
			printf("%s\n",conf.path.confFile.c_str());
			shitHappens("<b>Doh! Something going wrong</b>");
			throw(0);
		}
	}
	clearProfiles();
	clearBookmarks();
	char buf[0x4000];
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::string pnm = "default";
	int section = SECT_NONE;
	std::vector<std::string> vect;
	xLayout vlay;
	std::vector<xRomset> rsListist;
	xRomset newrs;
	size_t pos;
	std::string tms,fnam;
	int fprt;
	newrs.file.clear();
	newrs.gsFile.clear();
	for (int i=0; i<32; i++) {
		newrs.roms[i].path = "";
		newrs.roms[i].part = 0;
	}
	while (!file.eof()) {
		file.getline(buf,2048);
		line = std::string(buf);
		spl = splitline(line);
		pnam = spl.first;
		pval = spl.second;
		if (pval=="") {
			if (pnam=="[BOOKMARKS]") section = SECT_BOOKMARK;
			if (pnam=="[PROFILES]") section = SECT_PROFILES;
			if (pnam=="[VIDEO]") section = SECT_VIDEO;
			if (pnam=="[ROMSETS]") section = SECT_ROMSETS;
			if (pnam=="[SOUND]") section = SECT_SOUND;
			if (pnam=="[TOOLS]") section = SECT_TOOLS;
			if (pnam=="[GENERAL]") section = SECT_GENERAL;
			if (pnam=="[TAPE]") section = SECT_TAPE;
			if (pnam=="[LEDS]") section = SECT_LEDS;
		} else {
			switch (section) {
				case SECT_BOOKMARK:
					addBookmark(pnam,pval);
					break;
				case SECT_PROFILES:
					if (pnam == "current") {
						pnm = pval;
					} else {
						addProfile(pnam,pval);
					}
					break;
				case SECT_VIDEO:
					if (pnam=="layout") {
						vect = splitstr(pval,":");
						if (vect.size() > 8) {
							vlay.name = vect[0];
							vlay.full.h = atoi(vect[1].c_str());
							vlay.full.v = atoi(vect[2].c_str());
							vlay.bord.h = atoi(vect[3].c_str());
							vlay.bord.v = atoi(vect[4].c_str());
							vlay.sync.h = atoi(vect[5].c_str());
							vlay.sync.v = atoi(vect[6].c_str());
							vlay.intsz = atoi(vect[7].c_str());
							vlay.intpos.v = atoi(vect[8].c_str());
							if (vect.size() > 9) {
								vlay.intpos.h = atoi(vect[9].c_str());
							} else {
								vlay.intpos.h = 0;
							}
							if (vlay.full.h < vlay.bord.h + 256) vlay.full.h = vlay.bord.h + 256;
							if (vlay.sync.h > vlay.bord.h) vlay.sync.h = vlay.bord.h;
							if (vlay.full.v < vlay.bord.v + 192) vlay.full.v = vlay.bord.v + 256;
							if (vlay.sync.v > vlay.bord.v) vlay.sync.v = vlay.bord.v;
							addLayout(vlay);
						}
					}
					if (pnam=="scrDir") conf.scrShot.dir = pval;
					if (pnam=="scrFormat") conf.scrShot.format = pval;
					if (pnam=="scrCount") conf.scrShot.count = atoi(pval.c_str());
					if (pnam=="scrInterval") conf.scrShot.interval = atoi(pval.c_str());
					if (pnam=="fullscreen") conf.vid.fullScreen = str2bool(pval) ? 1 : 0;
					if (pnam=="bordersize") conf.brdsize = getRanged(pval.c_str(), 0, 100) / 100.0;
					if (pnam=="doublesize") conf.vid.doubleSize = str2bool(pval) ? 1 : 0;
					if (pnam=="noflic") conf.vid.noFlick = str2bool(pval) ? 1 : 0;
					if (pnam=="greyscale") conf.vid.grayScale = str2bool(pval) ? 1 : 0;
					break;
				case SECT_ROMSETS:
					pos = pval.find_last_of(":");
					if (pos != std::string::npos) {
						fnam = std::string(pval,0,pos);
						tms = std::string(pval,pos+1);
						if (tms=="") {
							fprt = 0;
						} else {
							fprt = atoi(tms.c_str());
						}
					} else {
						fnam = pval;
						fprt = 0;
					}
					if (pnam=="name") {
						newrs.name = pval;
						rsListist.push_back(newrs);
					}
					if (rsListist.size() != 0) {
						if (pnam=="file") {
							rsListist.back().file = fnam;
						}
						if ((pnam=="basic128") || (pnam=="0")) {
							rsListist.back().roms[0].path=fnam;
							rsListist.back().roms[0].part=fprt;
						}
						if ((pnam=="basic48") || (pnam=="1")) {
							rsListist.back().roms[1].path=fnam;
							rsListist.back().roms[1].part=fprt;
						}
						if ((pnam=="shadow") || (pnam=="2")) {
							rsListist.back().roms[2].path=fnam;
							rsListist.back().roms[2].part=fprt;
						}
						if ((pnam=="trdos") || (pnam=="3")) {
							rsListist.back().roms[3].path=fnam;
							rsListist.back().roms[3].part=fprt;
						}
						if (pnam=="gs") rsListist.back().gsFile=fnam;
						if (pnam=="font") rsListist.back().fntFile=fnam;
					}
					break;
				case SECT_SOUND:
					if (pnam=="enabled") conf.snd.enabled = str2bool(pval) ? 1 : 0;
					if (pnam=="dontmute") conf.snd.mute = str2bool(pval) ? 1 : 0;
					if (pnam=="soundsys") soutnam = pval;
					if (pnam=="rate") conf.snd.rate = atoi(pval.c_str());
					if (pnam=="volume.beep") conf.snd.vol.beep = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.tape") conf.snd.vol.tape = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.ay") conf.snd.vol.ay = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.gs") conf.snd.vol.gs = getRanged(pval.c_str(), 0, 100);
					break;
				case SECT_TOOLS:
					break;
				case SECT_GENERAL:
					if (pnam=="keys") {
						conf.keyMapName = pval;
						loadKeys();
					}
					if (pnam=="startdefault") conf.defProfile = str2bool(pval) ? 1 : 0;
					if (pnam=="savepaths") conf.storePaths = str2bool(pval) ? 1 : 0;
					if (pnam == "fdcturbo") setFlagBit(str2bool(pval),&fdcFlag,FDC_FAST);
					if (pnam == "systime") conf.sysclock = str2bool(pval) ? 1 : 0;
					break;
				case SECT_TAPE:
					if (pnam=="autoplay") conf.tape.autostart = str2bool(pval) ? 1 : 0;
					if (pnam=="fast") conf.tape.fast = str2bool(pval) ? 1 : 0;
					break;
				case SECT_LEDS:
					if (pnam=="mouse") conf.led.mouse = str2bool(pval) ? 1 : 0;
					if (pnam=="joystick") conf.led.joy = str2bool(pval) ? 1 : 0;
					if (pnam=="keyscan") conf.led.keys = str2bool(pval) ? 1 : 0;
					break;
			}
		}
	}
	uint i;
	for (i=0; i<rsListist.size(); i++) addRomset(rsListist[i]);
	prfLoadAll();
	setOutput(soutnam.c_str());
	if (conf.defProfile) {
		if (!selProfile("default")) {
			printf("Can't set default profile. GRR!\n");
			throw(0);
		}
	} else {
		if (!selProfile(pnm.c_str())) {
			shitHappens("Cannot set current profile\nDefault will be used");
			if (!selProfile("default")) {
				shitHappens("...and default too?\nReally, shit happens");
				throw(0);
			}
		}
	}
}
