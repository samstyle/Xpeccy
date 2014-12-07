// TODO: rewrite this shit

#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

#include <QtCore>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "sound.h"
#include "emulwin.h"
#include "settings.h"
#include "filer.h"

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

void initPaths() {
#if __linux
	conf.path.confDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy";
	conf.path.romDir = conf.path.confDir + "/roms";
	conf.path.confFile = conf.path.confDir + "/config.conf";
	conf.scrShot.dir = std::string(getenv("HOME"));
	mkdir(conf.path.confDir.c_str(),0777);
	mkdir(conf.path.romDir.c_str(),0777);
#else
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
	cfile << "startdefault = " << (conf.defProfile ? "yes" : "no") << "\n";
	cfile << "savepaths = " << (conf.storePaths ? "yes" : "no") << "\n";
	cfile << "fdcturbo = " << ((fdcFlag & FDC_FAST) ? "yes" : "no") << "\n";
	cfile << "systime = " << (conf.sysclock ? "yes" : "no") << "\n";

	cfile << "\n[BOOKMARKS]\n\n";
	std::vector<XBookmark> bml = getBookmarkList();
	for (i=0; i<bml.size(); i++) {
		cfile << bml[i].name << " = " << bml[i].path << "\n";
	}
	cfile << "\n[PROFILES]\n\n";
	std::vector<XProfile> prl = getProfileList();
	for (i=1; i<prl.size(); i++) {			// nr.0 skipped ('default' profile)
		cfile << prl[i].name << " = " << prl[i].file << "\n";
	}
	cfile << "current = " << getCurrentProfile()->name << "\n";
	cfile << "\n[VIDEO]\n\n";
	std::vector<VidLayout> lays = getLayoutList();
	for (i=1; i < lays.size(); i++) {
		cfile << "layout = ";
		cfile << lays[i].name.c_str() << ":";
		cfile << int2str(lays[i].full.h) << ":" << int2str(lays[i].full.v) << ":";
		cfile << int2str(lays[i].bord.h) << ":" << int2str(lays[i].bord.v) << ":";
		cfile << int2str(lays[i].sync.h) << ":" << int2str(lays[i].sync.v) << ":";
		cfile << int2str(lays[i].intsz) << ":" << int2str(lays[i].intpos.v) << ":" << int2str(lays[i].intpos.h) << "\n";
	}
	cfile << "scrDir = " << conf.scrShot.dir.c_str() << "\n";
	cfile << "scrFormat = " << conf.scrShot.format.c_str() << "\n";
	cfile << "scrCount = " << int2str(conf.scrShot.count) << "\n";
	cfile << "scrInterval = " << int2str(conf.scrShot.interval) << "\n";
	cfile << "colorLevel = " << int2str(conf.bright) << "\n";
	cfile << "fullscreen = " << ((vidFlag & VF_FULLSCREEN) ? "yes" : "no") << "\n";
	cfile << "doublesize = " << ((vidFlag & VF_DOUBLE) ? "yes" : "no") << "\n";
	cfile << "greyscale = " << ((vidFlag & VF_GREY) ? "yes" : "no") << "\n";
	cfile << "bordersize = " << int2str(conf.brdsize * 100) << "\n";
	cfile << "noflic = " << ((vidFlag & VF_NOFLIC) ? "yes" : "no") << "\n";
	cfile << "\n[ROMSETS]\n";
	std::vector<RomSet> rsl = getRomsetList();
	for (i=0; i<rsl.size(); i++) {
		cfile<< "\nname = " << rsl[i].name.c_str() << "\n";
		if (rsl[i].file != "") {
			cfile << "file = " << rsl[i].file.c_str() << "\n";
		} else {
			for (j=0; j<4; j++) {
				if (rsl[i].roms[j].path != "") {
					cfile << int2str(j).c_str() << " = " << rsl[i].roms[j].path.c_str();
					if (rsl[i].roms[j].part != 0) cfile << ":" << int2str(rsl[i].roms[j].part).c_str();
					cfile << "\n";
				}
			}
		}
		if (!rsl[i].gsFile.empty()) cfile << "gs = " << rsl[i].gsFile.c_str() << "\n";
		if (!rsl[i].fntFile.empty()) cfile << "font = " << rsl[i].fntFile.c_str() << "\n";
	}
	cfile << "\n[SOUND]\n\n";
	cfile << "enabled = " << (sndEnabled ? "yes" : "no") << "\n";
	cfile << "soundsys = " << sndOutput->name << "\n";
	cfile << "dontmute = " << (sndMute ? "yes" : "no") << "\n";
	cfile << "rate = " << int2str(sndRate).c_str() << "\n";
	cfile << "volume.beep = " << int2str(beepVolume).c_str() << "\n";
	cfile << "volume.tape = " << int2str(tapeVolume).c_str() << "\n";
	cfile << "volume.ay = " << int2str(ayVolume).c_str() << "\n";
	cfile << "volume.gs = " << int2str(gsVolume).c_str() << "\n";

	cfile << "\n[TAPE]\n\n";
	cfile << "autoplay = " << (conf.tape.autostart ? "yes" : "no") << "\n";
	cfile << "fast = " << (conf.tape.fast ? "yes" : "no") << "\n";

	cfile << "\n[LEDS]\n\n";
	cfile << "mouse = " << (conf.led.mouse ? "yes" : "no") << "\n";
	cfile << "joystick = " << (conf.led.joy ? "yes" : "no") << "\n";
	cfile << "keyscan = " << (conf.led.keys ? "yes" : "no") << "\n";
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
	char* buf = new char[0x4000];
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::string pnm = "default";
	int section = SECT_NONE;
	std::vector<std::string> vect;
	VidLayout vlay;
	std::vector<RomSet> rslist;
	RomSet newrs;
	size_t pos;
	std::string tms,fnam;
	int test,fprt;
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
							vlay.full.h = atoi(vect[1].c_str()); vlay.full.v = atoi(vect[2].c_str());
							vlay.bord.h = atoi(vect[3].c_str()); vlay.bord.v = atoi(vect[4].c_str());
							vlay.sync.h = atoi(vect[5].c_str()); vlay.sync.v = atoi(vect[6].c_str());
							vlay.intsz = atoi(vect[7].c_str()); vlay.intpos.v = atoi(vect[8].c_str());
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
					if (pnam=="scrFormat") {
						conf.scrShot.format = pval;
					}
					if (pnam=="scrCount") conf.scrShot.count = atoi(pval.c_str());
					if (pnam=="scrInterval") conf.scrShot.interval = atoi(pval.c_str());
					if (pnam=="colorLevel") {
						test=atoi(pval.c_str());
						if ((test < 50) || (test > 250)) test=192;
						conf.bright = test;
					}
					if (pnam=="fullscreen") setFlagBit(str2bool(pval),&vidFlag,VF_FULLSCREEN);
					if (pnam=="bordersize") {
						test=atoi(pval.c_str());
						if ((test >= 0) && (test <= 100)) conf.brdsize = test / 100.0;
					}
					if (pnam=="doublesize") setFlagBit(str2bool(pval),&vidFlag,VF_DOUBLE);
					if (pnam=="noflic") setFlagBit(str2bool(pval),&vidFlag,VF_NOFLIC);
					if (pnam=="greyscale") setFlagBit(str2bool(pval),&vidFlag,VF_GREY);
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
						rslist.push_back(newrs);
					}
					if (rslist.size() != 0) {
						if (pnam=="file") {
							rslist.back().file = fnam;
						}
						if ((pnam=="basic128") || (pnam=="0")) {
							rslist.back().roms[0].path=fnam;
							rslist.back().roms[0].part=fprt;
						}
						if ((pnam=="basic48") || (pnam=="1")) {
							rslist.back().roms[1].path=fnam;
							rslist.back().roms[1].part=fprt;
						}
						if ((pnam=="shadow") || (pnam=="2")) {
							rslist.back().roms[2].path=fnam;
							rslist.back().roms[2].part=fprt;
						}
						if ((pnam=="trdos") || (pnam=="3")) {
							rslist.back().roms[3].path=fnam;
							rslist.back().roms[3].part=fprt;
						}
						if (pnam=="gs") rslist.back().gsFile=fnam;
						if (pnam=="font") rslist.back().fntFile=fnam;
					}
					break;
				case SECT_SOUND:
					if (pnam=="enabled") sndEnabled = str2bool(pval);
					if (pnam=="dontmute") sndMute = str2bool(pval);
					if (pnam=="soundsys") soutnam = pval;
					if (pnam=="rate") sndRate = atoi(pval.c_str());
					if (pnam=="volume.beep") {
						test = atoi(pval.c_str());
						if (test > 100) test = 100;
						if (test < 0) test = 0;
						beepVolume = test;
					}
					if (pnam=="volume.tape") {
						test = atoi(pval.c_str());
						if (test > 100) test = 100;
						if (test < 0) test = 0;
						tapeVolume = test;
					}
					if (pnam=="volume.ay") {
						test = atoi(pval.c_str());
						if (test > 100) test = 100;
						if (test < 0) test = 0;
						ayVolume = test;
					}
					if (pnam=="volume.gs") {
						test = atoi(pval.c_str());
						if (test > 100) test = 100;
						if (test < 0) test = 0;
						gsVolume = test;
					}
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
	for (i=0; i<rslist.size(); i++) addRomset(rslist[i]);
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
