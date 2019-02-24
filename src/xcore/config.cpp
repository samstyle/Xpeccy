#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <stdio.h>

#include <QtCore>

#include "xcore.h"
#include "vscalers.h"
#include "xgui.h"
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
#define	SECT_GAMEPAD	7
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
    conf.port = 30000;
#if __linux || __APPLE__
	strcpy(conf.path.confDir, getenv(ENVHOME));
	strcat(conf.path.confDir, "/.config");
	mkdir(conf.path.confDir, 0777);
	strcat(conf.path.confDir, "/samstyle");
	mkdir(conf.path.confDir, 0777);
	strcat(conf.path.confDir, "/xpeccy");
	mkdir(conf.path.confDir, 0777);
	strcpy(conf.path.romDir, conf.path.confDir);
	strcat(conf.path.romDir, "/roms");
	mkdir(conf.path.romDir ,0777);
	strcpy(conf.path.confFile, conf.path.confDir);
	strcat(conf.path.confFile, "/config.conf");
	strcpy(conf.path.boot, conf.path.confDir);
	strcat(conf.path.boot, "/boot.$B");
	//conf.path.font = conf.path.confDir + "/appfont.ttf";
#elif __WIN32
	char wdir[FILENAME_MAX];
	char* pos = strrchr(wpath, SLSH);
	if (pos) {
		strncpy(wdir, wpath, pos - wpath);
		wdir[pos - wpath] = 0x00;
	} else {
		strcpy(wdir, wpath);
	}
	// strcpy(wdir, ".");
	strcpy(conf.path.confDir, wdir);
	strcat(conf.path.confDir, "\\config");
	strcpy(conf.path.romDir, conf.path.confDir);
	strcat(conf.path.romDir, "\\roms");
	strcpy(conf.path.confFile, conf.path.confDir);
	strcat(conf.path.confFile, "\\config.conf");
	strcpy(conf.path.boot, conf.path.confDir);
	strcat(conf.path.boot, "\\boot.$B");
	//conf.path.font = conf.path.confDir + "\\appfont.ttf";
	mkdir(conf.path.confDir);
	mkdir(conf.path.romDir);
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
	FILE* cfile = fopen(conf.path.confFile, "wb");
	if (!cfile) {
		shitHappens("Can't write main config");
		throw(0);
	}
//	uint j;

	fprintf(cfile,"[GENERAL]\n\n");
//	if ((conf.keyMapName != "default") && (conf.keyMapName != "")) {
//		fprintf(cfile,"keys = %s\n",conf.keyMapName.c_str());
//	}
	fprintf(cfile, "startdefault = %s\n", YESNO(conf.defProfile));
	fprintf(cfile, "savepaths = %s\n", YESNO(conf.storePaths));
	fprintf(cfile, "fdcturbo = %s\n", YESNO(fdcFlag & FDC_FAST));
//	fprintf(cfile, "systime = %s\n", YESNO(conf.sysclock));
	fprintf(cfile, "lastdir = %s\n",conf.path.lastDir);
	fprintf(cfile, "port = %i\n", conf.port);

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
	fprintf(cfile, "greyscale = %s\n", YESNO(greyScale));
	fprintf(cfile, "bordersize = %i\n", int(conf.brdsize * 100));
	fprintf(cfile, "noflick = %i%%\n", noflic);

	fprintf(cfile, "\n[ROMSETS]\n");
	foreach(xRomset rms, conf.rsList) {
		fprintf(cfile, "\nname = %s\n", rms.name.c_str());
#if 0
		if (rms.file != "") {
			fprintf(cfile, "file = %s\n", rms.file.c_str());
		} else {
			for (j = 0; j < 4; j++) {
				if (rms.roms[j].path != "") {
					fprintf(cfile, "%i = %s:%i\n", j, rms.roms[j].path.c_str(), rms.roms[j].part);
				}
			}
		}
#else
		foreach(xRomFile rf, rms.roms) {
			fprintf(cfile, "rom = %s:%i:%i:%i\n",rf.name.c_str(), rf.foffset, rf.fsize, rf.roffset);
		}
#endif
		if (!rms.gsFile.empty())
			fprintf(cfile, "gs = %s\n", rms.gsFile.c_str());
		if (!rms.fntFile.empty())
			fprintf(cfile, "font = %s\n", rms.fntFile.c_str());
	}

	fprintf(cfile, "\n[SOUND]\n\n");
	fprintf(cfile, "enabled = %s\n", YESNO(conf.snd.enabled));
//	fprintf(cfile, "dontmute = %s\n", YESNO(conf.snd.mute));
	fprintf(cfile, "soundsys = %s\n", sndOutput->name);
	fprintf(cfile, "rate = %i\n", conf.snd.rate);
//	fprintf(cfile, "dac = %s\n", YESNO(ayDac));
	fprintf(cfile, "volume.master = %i\n", conf.snd.vol.master);
	fprintf(cfile, "volume.beep = %i\n", conf.snd.vol.beep);
	fprintf(cfile, "volume.tape = %i\n", conf.snd.vol.tape);
	fprintf(cfile, "volume.ay = %i\n", conf.snd.vol.ay);
	fprintf(cfile, "volume.gs = %i\n", conf.snd.vol.gs);
	fprintf(cfile, "volume.sdrv = %i\n", conf.snd.vol.sdrv);
	fprintf(cfile, "volume.saa = %i\n", conf.snd.vol.saa);

	fprintf(cfile, "\n[TAPE]\n\n");
	fprintf(cfile, "autoplay = %s\n", YESNO(conf.tape.autostart));
	fprintf(cfile, "fast = %s\n", YESNO(conf.tape.fast));

	fprintf(cfile, "\n[INPUT]\n\n");
	fprintf(cfile, "deadzone = %i", conf.joy.dead);

	fprintf(cfile, "\n[LEDS]\n\n");
	fprintf(cfile, "mouse = %s\n", YESNO(conf.led.mouse));
	fprintf(cfile, "joystick = %s\n", YESNO(conf.led.joy));
	fprintf(cfile, "keyscan = %s\n", YESNO(conf.led.keys));
	fprintf(cfile, "tape = %s\n", YESNO(conf.led.tape));
	fprintf(cfile, "disk = %s\n", YESNO(conf.led.disk));
	fprintf(cfile, "message = %s\n", YESNO(conf.led.message));
	fprintf(cfile, "fps = %s\n", YESNO(conf.led.fps));

	fclose(cfile);
}


void copyFile(const char* src, const char* dst) {
	QFile fle(QString::fromLocal8Bit(src));
	fle.open(QFile::ReadOnly);
	QByteArray fdata = fle.readAll();
	fle.close();
	fle.setFileName(QString::fromLocal8Bit(dst));
	if (fle.open(QFile::WriteOnly)) {
		fle.write(fdata);
		fle.close();
	}
}

// emulator config

void loadConfig() {
	std::string soutnam = "NULL";
	//printf("%s\n",conf.path.confFile);
	std::ifstream file(conf.path.confFile);
	char fname[FILENAME_MAX];
	if (!file.good()) {
		printf("Main config is missing. Default files will be copied\n");
		copyFile(":/conf/config.conf", conf.path.confFile);
		strcpy(fname, conf.path.confDir);
		strcat(fname, SLASH);
		strcat(fname, "xpeccy.conf");
		copyFile(":/conf/xpeccy.conf", fname);
		strcpy(fname, conf.path.romDir);
		strcat(fname, SLASH);
		strcat(fname, "1982.rom");
		copyFile(":/conf/1982.rom", fname);
		file.open(conf.path.confFile);
		if (!file.good()) {
			printf("%s\n",conf.path.confFile);
			shitHappens("<b>Doh! Something going wrong</b>");
			throw(0);
		}
	}
	clearProfiles();
	conf.bookmarkList.clear();
	char buf[0x4000];
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::string pnm = "default";
	int section = SECT_NONE;
	std::vector<std::string> vect;
	vLayout vlay;
	std::vector<xRomset> rsListist;
	xRomset newrs;
	xRomFile rfile;
	size_t pos;
	std::string tms,fnam;
	int fprt;
//	newrs.file.clear();
	newrs.fntFile.clear();
	newrs.gsFile.clear();
	newrs.roms.clear();
//	for (int i=0; i<32; i++) {
//		newrs.roms[i].path = "";
//		newrs.roms[i].part = 0;
//	}

	conf.snd.vol.master = 100;
	conf.snd.vol.beep = 100;
	conf.snd.vol.tape = 100;
	conf.snd.vol.ay = 100;
	conf.snd.vol.gs = 100;
	conf.snd.vol.sdrv = 100;
	conf.snd.vol.saa = 100;

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
			if (pnam=="[INPUT]") section = SECT_INPUT;
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
				case SECT_INPUT:
					if (pnam=="deadzone") conf.joy.dead = strtol(pval.c_str(), NULL, 0);
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
					if (pnam=="noflic") noflic = str2bool(pval) ? 50 : 25;		// old parameter
					if (pnam=="noflick") noflic = getRanged(pval.c_str(), 0, 50);	// new parameter
					if (pnam=="greyscale") greyScale = str2bool(pval) ? 1 : 0;
					break;
				case SECT_ROMSETS:
					pos = pval.find_last_of(":");
					vect = splitstr(pval, ":");
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
						if ((pnam == "rom") && (vect.size() > 0)) {
							while (vect.size() < 4) {
								vect.push_back("0");
							}
							rfile.name = vect[0];
							rfile.foffset = atoi(vect[1].c_str());
							rfile.fsize = atoi(vect[2].c_str());
							rfile.roffset = atoi(vect[3].c_str());
							rsListist.back().roms.push_back(rfile);
						} else if (pnam=="file") {
							rfile.name = fnam;
							rfile.foffset = 0;
							rfile.fsize = 0;
							rfile.roffset = 0;
							rsListist.back().roms.push_back(rfile);
						} else if ((pnam=="basic128") || (pnam=="0")) {
							rfile.name = fnam;
							rfile.foffset = fprt * 16;
							rfile.fsize = 16;
							rfile.roffset = 0;
							rsListist.back().roms.push_back(rfile);
						} else if ((pnam=="basic48") || (pnam=="1")) {
							rfile.name = fnam;
							rfile.foffset = fprt * 16;
							rfile.fsize = 16;
							rfile.roffset = 16;
							rsListist.back().roms.push_back(rfile);
						} else if ((pnam=="shadow") || (pnam=="2")) {
							rfile.name = fnam;
							rfile.foffset = fprt * 16;
							rfile.fsize = 16;
							rfile.roffset = 32;
							rsListist.back().roms.push_back(rfile);
						} else if ((pnam=="trdos") || (pnam=="3")) {
							rfile.name = fnam;
							rfile.foffset = fprt * 16;
							rfile.fsize = 16;
							rfile.roffset = 48;
							rsListist.back().roms.push_back(rfile);
						}
						if (pnam=="gs") rsListist.back().gsFile=fnam;
						if (pnam=="font") rsListist.back().fntFile=fnam;
					}
					break;
				case SECT_SOUND:
					if (pnam=="enabled") conf.snd.enabled = str2bool(pval) ? 1 : 0;
//					if (pnam=="dac") ayDac = str2bool(pval.c_str()) ? 1 : 0;
					if (pnam=="soundsys") soutnam = pval;
					if (pnam=="rate") conf.snd.rate = strtol(pval.c_str(), NULL, 10);
					if (pnam=="volume.master") conf.snd.vol.master = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.beep") conf.snd.vol.beep = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.tape") conf.snd.vol.tape = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.ay") conf.snd.vol.ay = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.gs") conf.snd.vol.gs = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.sdrv") conf.snd.vol.sdrv = getRanged(pval.c_str(), 0, 100);
					if (pnam=="volume.saa") conf.snd.vol.saa = getRanged(pval.c_str(), 0, 100);
					break;
				case SECT_TOOLS:
					break;
				case SECT_GENERAL:
					// if (pnam=="keys") conf.keyMapName = pval;
					if (pnam=="startdefault") conf.defProfile = str2bool(pval) ? 1 : 0;
					if (pnam=="savepaths") conf.storePaths = str2bool(pval) ? 1 : 0;
					if (pnam == "fdcturbo") setFlagBit(str2bool(pval),&fdcFlag,FDC_FAST);
//					if (pnam == "systime") conf.sysclock = str2bool(pval) ? 1 : 0;
					if (pnam == "lastdir") strcpy(conf.path.lastDir, pval.c_str());
					if (pnam == "port") conf.port = strtol(pval.c_str(), NULL, 10);
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
					if (pnam=="fps") conf.led.fps = str2bool(pval) ? 1 : 0;
					break;
			}
		}
	}
	vid_set_zoom(conf.vid.scale);
	vid_set_fullscreen(conf.vid.fullScreen);
	vid_set_ratio(conf.vid.keepRatio);
	uint i;
	for (i=0; i<rsListist.size(); i++) addRomset(rsListist[i]);
	prfLoadAll();
	setOutput(soutnam.c_str());
	//loadKeys();
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
