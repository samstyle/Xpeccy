#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <stdio.h>

#include <QtCore>

#include "xcore.h"
#include "../xgui/xgui.h"
#include "sound.h"

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

void initPaths(char* wpath) {
	conf.scrShot.dir = std::string(getenv(ENVHOME));
#if __linux || __APPLE__
	conf.path.confDir = conf.scrShot.dir + "/.config";
	mkdir(conf.path.confDir.c_str(),0777);
	conf.path.confDir += "/samstyle";
	mkdir(conf.path.confDir.c_str(),0777);
	conf.path.confDir += "/xpeccy";
	mkdir(conf.path.confDir.c_str(),0777);
	conf.path.romDir = conf.path.confDir + "/roms";
	mkdir(conf.path.romDir.c_str(),0777);
	conf.path.confFile = conf.path.confDir + "/config.conf";
	conf.path.boot = conf.path.confDir + "/boot.$B";
	//conf.path.font = conf.path.confDir + "/appfont.ttf";
#elif __WIN32
	std::string wdir(wpath);
	size_t pos = wdir.find_last_of("/\\");
	if (pos > 0) {
		wdir = wdir.substr(0, pos);
	}
	conf.path.confDir = wdir + "\\config";
	conf.path.romDir = conf.path.confDir + "\\roms";
	conf.path.confFile = conf.path.confDir + "\\config.conf";
	conf.path.boot = conf.path.confDir + "\\boot.$B";
	//conf.path.font = conf.path.confDir + "\\appfont.ttf";
	mkdir(conf.path.confDir.c_str());
	mkdir(conf.path.romDir.c_str());
#endif
	/*
	// check user app font or use built-in DejaVuSansMono.ttf
	FILE* file = fopen(conf.path.font.c_str(), "rb");
	if (file != NULL) {
		fclose(file);
	} else {
		conf.path.font = "://DejaVuSansMono.ttf";
	}
	// printf("Used font : %s\n",conf.path.font.c_str());
	*/
}

void saveConfig() {
	FILE* cfile = fopen(conf.path.confFile.c_str(),"wb");
	if (!cfile) {
		shitHappens("Can't write main config");
		throw(0);
	}
	uint j;

	fprintf(cfile,"[GENERAL]\n\n");
	if ((conf.keyMapName != "default") && (conf.keyMapName != "")) {
		fprintf(cfile,"keys = %s\n",conf.keyMapName.c_str());
	}
	fprintf(cfile, "startdefault = %s\n", YESNO(conf.defProfile));
	fprintf(cfile, "savepaths = %s\n", YESNO(conf.storePaths));
	fprintf(cfile, "fdcturbo = %s\n", YESNO(fdcFlag & FDC_FAST));
//	fprintf(cfile, "systime = %s\n", YESNO(conf.sysclock));
	fprintf(cfile, "lastdir = %s\n",conf.path.lastDir.c_str());

	fprintf(cfile, "\n[BOOKMARKS]\n\n");
	foreach(xBookmark bkm, conf.bookmarkList) {
		fprintf(cfile, "%s = %s\n", bkm.name.c_str(), bkm.path.c_str());
	}

	fprintf(cfile, "\n[PROFILES]\n\n");
	foreach(xProfile* prf, conf.prof.list) {			// nr.0 skipped ('default' profile)
		if (prf->name != "default")
			fprintf(cfile, "%s = %s\n", prf->name.c_str(), prf->file.c_str());
	}
	fprintf(cfile, "current = %s\n", conf.prof.cur->name.c_str());

	fprintf(cfile, "\n[VIDEO]\n\n");
	foreach(xLayout lay, conf.layList) {
		if (lay.name != "default") {
			fprintf(cfile, "layout = %s:%i:%i:%i:%i:%i:%i:%i:%i:%i:%i:%i\n",lay.name.c_str(),\
				lay.lay.full.x, lay.lay.full.y, lay.lay.bord.x, lay.lay.bord.y,\
				lay.lay.blank.x, lay.lay.blank.y, lay.lay.intSize, lay.lay.intpos.y, lay.lay.intpos.x,\
				lay.lay.scr.x, lay.lay.scr.y);
		}
	}
	fprintf(cfile, "scrDir = %s\n", conf.scrShot.dir.c_str());
	fprintf(cfile, "scrFormat = %s\n", conf.scrShot.format.c_str());
	fprintf(cfile, "scrCount = %i\n", conf.scrShot.count);
	fprintf(cfile, "scrInterval = %i\n", conf.scrShot.interval);
	fprintf(cfile, "scrNoLeds = %s\n", YESNO(conf.scrShot.noLeds));
	fprintf(cfile, "scrNoBord = %s\n", YESNO(conf.scrShot.noBorder));
	fprintf(cfile, "fullscreen = %s\n", YESNO(conf.vid.fullScreen));
	fprintf(cfile, "keepratio = %s\n", YESNO(conf.vid.keepRatio));
	fprintf(cfile, "scale = %i\n", conf.vid.scale);
	fprintf(cfile, "greyscale = %s\n", YESNO(conf.vid.grayScale));
	fprintf(cfile, "bordersize = %i\n", int(conf.brdsize * 100));
	fprintf(cfile, "noflic = %s\n", YESNO(conf.vid.noFlick));

	fprintf(cfile, "\n[ROMSETS]\n");
	foreach(xRomset rms, conf.rsList) {
		fprintf(cfile, "\nname = %s\n", rms.name.c_str());
		if (rms.file != "") {
			fprintf(cfile, "file = %s\n", rms.file.c_str());
		} else {
			for (j = 0; j < 4; j++) {
				if (rms.roms[j].path != "") {
					fprintf(cfile, "%i = %s:%i\n", j, rms.roms[j].path.c_str(), rms.roms[j].part);
				}
			}
		}
		if (!rms.gsFile.empty())
			fprintf(cfile, "gs = %s\n", rms.gsFile.c_str());
		if (!rms.fntFile.empty())
			fprintf(cfile, "font = %s\n", rms.fntFile.c_str());
	}

	fprintf(cfile, "\n[SOUND]\n\n");
	fprintf(cfile, "enabled = %s\n", YESNO(conf.snd.enabled));
	fprintf(cfile, "dontmute = %s\n", YESNO(conf.snd.mute));
	fprintf(cfile, "soundsys = %s\n", sndOutput->name);
	fprintf(cfile, "rate = %i\n", conf.snd.rate);
	fprintf(cfile, "volume.beep = %i\n", conf.snd.vol.beep);
	fprintf(cfile, "volume.tape = %i\n", conf.snd.vol.tape);
	fprintf(cfile, "volume.ay = %i\n", conf.snd.vol.ay);
	fprintf(cfile, "volume.gs = %i\n", conf.snd.vol.gs);

	fprintf(cfile, "\n[TAPE]\n\n");
	fprintf(cfile, "autoplay = %s\n", YESNO(conf.tape.autostart));
	fprintf(cfile, "fast = %s\n", YESNO(conf.tape.fast));

	fprintf(cfile, "\n[LEDS]\n\n");
	fprintf(cfile, "mouse = %s\n", YESNO(conf.led.mouse));
	fprintf(cfile, "joystick = %s\n", YESNO(conf.led.joy));
	fprintf(cfile, "keyscan = %s\n", YESNO(conf.led.keys));
	fprintf(cfile, "tape = %s\n", YESNO(conf.led.tape));
	fprintf(cfile, "disk = %s\n", YESNO(conf.led.disk));
	fprintf(cfile, "message = %s\n", YESNO(conf.led.message));

	fclose(cfile);
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
		copyFile(":/conf/config.conf",conf.path.confFile.c_str());
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
	vLayout vlay;
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
							vlay.full.x = atoi(vect[1].c_str());
							vlay.full.y = atoi(vect[2].c_str());
							vlay.bord.x = atoi(vect[3].c_str());
							vlay.bord.y = atoi(vect[4].c_str());
							vlay.blank.x = atoi(vect[5].c_str());
							vlay.blank.y = atoi(vect[6].c_str());
							vlay.intSize = atoi(vect[7].c_str());
							vlay.intpos.y = atoi(vect[8].c_str());
							vlay.intpos.x = (vect.size() > 9) ? atoi(vect[9].c_str()) : 0;
							vlay.scr.x = (vect.size() > 10) ? atoi(vect[10].c_str()) : 256;
							vlay.scr.y = (vect.size() > 11) ? atoi(vect[11].c_str()) : 192;
							if (vlay.full.x > 512) vlay.full.x = 512;
							if (vlay.full.y > 512) vlay.full.y = 512;
							addLayout(vect[0], vlay);
						}
					}
					if (pnam=="scrDir") conf.scrShot.dir = pval;
					if (pnam=="scrFormat") conf.scrShot.format = pval;
					if (pnam=="scrCount") conf.scrShot.count = atoi(pval.c_str());
					if (pnam=="scrInterval") conf.scrShot.interval = atoi(pval.c_str());
					if (pnam=="scrNoLeds") conf.scrShot.noLeds = str2bool(pval) ? 1 : 0;
					if (pnam=="scrNoBord") conf.scrShot.noBorder = str2bool(pval) ? 1 : 0;
					if (pnam=="fullscreen") conf.vid.fullScreen = str2bool(pval) ? 1 : 0;
					if (pnam=="keepratio") conf.vid.keepRatio = str2bool(pval) ? 1 : 0;
					if (pnam=="bordersize") conf.brdsize = getRanged(pval.c_str(), 0, 100) / 100.0;
					if (pnam=="doublesize") conf.vid.scale = str2bool(pval) ? 2 : 1;
					if (pnam=="scale") {
						conf.vid.scale = atoi(pval.c_str());
						if (conf.vid.scale < 1) conf.vid.scale = 1;
						if (conf.vid.scale > 4) conf.vid.scale = 4;
					}
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
//					if (pnam == "systime") conf.sysclock = str2bool(pval) ? 1 : 0;
					if (pnam == "lastdir") conf.path.lastDir = pval;
					break;
				case SECT_TAPE:
					if (pnam=="autoplay") conf.tape.autostart = str2bool(pval) ? 1 : 0;
					if (pnam=="fast") conf.tape.fast = str2bool(pval) ? 1 : 0;
					break;
				case SECT_LEDS:
					if (pnam=="mouse") conf.led.mouse = str2bool(pval) ? 1 : 0;
					if (pnam=="joystick") conf.led.joy = str2bool(pval) ? 1 : 0;
					if (pnam=="keyscan") conf.led.keys = str2bool(pval) ? 1 : 0;
					if (pnam=="tape") conf.led.tape = str2bool(pval) ? 1 : 0;
					if (pnam=="disk") conf.led.disk = str2bool(pval) ? 1 : 0;
					if (pnam=="message") conf.led.message = str2bool(pval) ? 1 : 0;
					break;
			}
		}
	}
	uint i;
	for (i=0; i<rsListist.size(); i++) addRomset(rsListist[i]);
	prfLoadAll();
	setOutput(soutnam.c_str());
	if (conf.defProfile) {
		if (!prfSetCurrent("default")) {
			printf("Can't set default profile! Yes, it happens\n");
			throw(0);
		}
	} else {
		if (!prfSetCurrent(pnm.c_str())) {
			printf("Cannot set profile '%s', default will be used\n",pnm.c_str());
			if (!prfSetCurrent("default")) {
				printf("...and default too? Really, shit happens\n");
				throw(0);
			}
		}
	}
}
