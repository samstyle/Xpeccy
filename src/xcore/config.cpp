#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <stdio.h>

#include <QtCore>

#include "xcore.h"
#include "vscalers.h"
#include "../xgui/xgui.h"
#include "sound.h"

#ifdef _WIN32
	#include <windows.h>
	#include <direct.h>
#endif

enum {
	SECT_NONE = 0,
	SECT_BOOKMARK,
	SECT_PROFILES,
	SECT_VIDEO,
	SECT_ROMSETS,
	SECT_SOUND,
	SECT_TOOLS,
	SECT_GAMEPAD,
	SECT_GENERAL,
	SECT_SCRSHOT,
	SECT_DISK,
	SECT_IDE,
	SECT_MACHINE,
	SECT_MENU,
	SECT_TAPE,
	SECT_LEDS,
	SECT_INPUT,
	SECT_SDC,
	SECT_DEBUGA,
	SECT_PALETTE,
	SECT_KEYS,
};

std::map<std::string, int> shotFormat;
xConfig conf;

void conf_init(char* wpath) {
	conf.scrShot.dir = std::string(getenv(ENVHOME));
	conf.port = 30000;
#if defined(__linux) || defined(__APPLE__) || defined(__BSD)
	conf.path.confDir = std::string(getenv(ENVHOME)) + "/.config";
	mkdir(conf.path.confDir.c_str(), 0777);
	conf.path.confDir += "/samstyle";
	mkdir(conf.path.confDir.c_str(), 0777);
	conf.path.confDir += "/xpeccy";
	mkdir(conf.path.confDir.c_str(), 0777);
	conf.path.romDir = conf.path.confDir + "/roms";
	mkdir(conf.path.romDir.c_str() ,0777);
	conf.path.prfDir = conf.path.confDir + "/profiles";
	mkdir(conf.path.prfDir.c_str() ,0777);
	conf.path.shdDir = conf.path.confDir + "/shaders";
	mkdir(conf.path.shdDir.c_str() ,0777);
	conf.path.confFile = conf.path.confDir + "/config.conf";
	conf.path.boot = conf.path.confDir + "/boot.$B";
#elif defined(__WIN32)
	conf.path.confDir = std::string(wpath);
	size_t pos = conf.path.confDir.find_last_of(SLSH);
	if (pos != std::string::npos) {
		conf.path.confDir = conf.path.confDir.substr(0, pos);
	}
	conf.path.confDir += "\\config";
	conf.path.romDir = conf.path.confDir + "\\roms";
	conf.path.prfDir = conf.path.confDir + "\\profiles";
	conf.path.shdDir = conf.path.confDir + "\\shaders";
	conf.path.confFile = conf.path.confDir + "\\config.conf";
	conf.path.boot = conf.path.confDir + "\\boot.$B";
	mkdir(conf.path.confDir.c_str());
	mkdir(conf.path.romDir.c_str());
	mkdir(conf.path.prfDir.c_str());
	mkdir(conf.path.shdDir.c_str());
#endif
	conf.scrShot.format = "png";
// Pentagon geometry:
// rows: 16Vblk + (16 invis + 48 vis) top border + 192 screen + 48 bottom border = 320 rows
// cols: 64Hblk + 72 left border + 256 screen + 56 right border = 448 dots (224T)
	vLayout vlay = {{448,320},{72,64},{64,16},{256,192},{0,0},64};
	addLayout("default", vlay);
	conf.running = 0;
	conf.boot = 1;
	conf.emu.pause = 0;
	conf.emu.fast = 0;
	conf.joy.dead = 8192;
	conf.prof.changed = 0;
	addProfile("default","xpeccy.conf");
}

void saveConfig() {
	FILE* cfile = fopen(conf.path.confFile.c_str(), "wb");
	if (!cfile) {
		shitHappens("Can't write main config");
		throw(0);
	}

	fprintf(cfile,"[GENERAL]\n\n");
	fprintf(cfile, "startdefault = %s\n", YESNO(conf.defProfile));
	fprintf(cfile, "savepaths = %s\n", YESNO(conf.storePaths));
	fprintf(cfile, "fdcturbo = %s\n", YESNO(fdcFlag & FDC_FAST));
	fprintf(cfile, "addboot = %s\n", YESNO(conf.boot));
	fprintf(cfile, "exit.confirm = %s\n",YESNO(conf.confexit));
	fprintf(cfile, "port = %i\n", conf.port);
	fprintf(cfile, "winpos = %i,%i\n",conf.xpos,conf.ypos);

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
	fprintf(cfile, "scanlines = %s\n", YESNO(scanlines));
	fprintf(cfile, "bordersize = %i\n", int(conf.brdsize * 100));
	fprintf(cfile, "noflick = %i\n", noflic);
	fprintf(cfile, "shader = %s\n", conf.vid.shader.c_str());

	fprintf(cfile, "\n[ROMSETS]\n");
	foreach(xRomset rms, conf.rsList) {
		fprintf(cfile, "\nname = %s\n", rms.name.c_str());
		foreach(xRomFile rf, rms.roms) {
			fprintf(cfile, "rom = %s:%i:%i:%i\n",rf.name.c_str(), rf.foffset, rf.fsize, rf.roffset);
		}
		if (!rms.gsFile.empty())
			fprintf(cfile, "gs = %s\n", rms.gsFile.c_str());
		if (!rms.fntFile.empty())
			fprintf(cfile, "font = %s\n", rms.fntFile.c_str());
		if (!rms.vBiosFile.empty())
			fprintf(cfile, "vga = %s\n", rms.vBiosFile.c_str());
	}

	fprintf(cfile, "\n[SOUND]\n\n");
	fprintf(cfile, "enabled = %s\n", YESNO(conf.snd.enabled));
	fprintf(cfile, "soundsys = %s\n", sndOutput->name);
	fprintf(cfile, "rate = %i\n", conf.snd.rate);
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
	fprintf(cfile, "deadzone = %i\n", conf.joy.dead);

	fprintf(cfile, "\n[LEDS]\n\n");
	fprintf(cfile, "mouse = %s\n", YESNO(conf.led.mouse));
	fprintf(cfile, "joystick = %s\n", YESNO(conf.led.joy));
	fprintf(cfile, "keyscan = %s\n", YESNO(conf.led.keys));
	fprintf(cfile, "tape = %s\n", YESNO(conf.led.tape));
	fprintf(cfile, "disk = %s\n", YESNO(conf.led.disk));
	fprintf(cfile, "message = %s\n", YESNO(conf.led.message));
	fprintf(cfile, "fps = %s\n", YESNO(conf.led.fps));
	fprintf(cfile, "halt = %s\n", YESNO(conf.led.halt));

	fprintf(cfile, "\n[DEBUGA]\n\n");
	fprintf(cfile, "dbsize = %i\n", conf.dbg.dbsize);
	fprintf(cfile, "dwsize = %i\n", conf.dbg.dwsize);
	fprintf(cfile, "dmsize = %i\n", conf.dbg.dmsize);
	fprintf(cfile, "font = %s\n", conf.dbg.font.toString().toUtf8().data());

	fprintf(cfile, "\n[PALETTE]\n\n");
	QStringList lst = conf.pal.keys();
	QString nam;
	QColor col;
	foreach(nam, lst) {
		col = conf.pal[nam];
		if (col.isValid())
			fprintf(cfile, "%s = %s\n", nam.toLocal8Bit().data(), col.name().toLocal8Bit().data());
	}

	fprintf(cfile, "\n[KEYS]\n\n");
	xShortcut* tab = shortcut_tab();
	int i = 0;
	while (tab[i].id > 0) {
		fprintf(cfile, "%s = %s\n", tab[i].name, tab[i].seq.toString().toLocal8Bit().data());
		i++;
	}
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
		copyFile(":/conf/config.conf", conf.path.confFile.c_str());
		strcpy(fname, conf.path.confDir.c_str());
		strcat(fname, SLASH);
		strcat(fname, "xpeccy.conf");
		copyFile(":/conf/xpeccy.conf", fname);
		strcpy(fname, conf.path.romDir.c_str());
		strcat(fname, SLASH);
		strcat(fname, "1982.rom");
		copyFile(":/conf/1982.rom", fname);
		file.open(conf.path.confFile);
		if (!file.good()) {
			printf("%s\n",conf.path.confFile.c_str());
			shitHappens("<b>Doh! Something going wrong</b>");
			throw(0);
		}
	}
	clearProfiles();
	conf.bookmarkList.clear();
	char buf[0x4000];
	QColor col;
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
	newrs.fntFile.clear();
	newrs.gsFile.clear();
	newrs.vBiosFile.clear();
	newrs.roms.clear();
	conf.pal.clear();
	shortcut_init();
	conf.xpos = -1;
	conf.ypos = -1;
	conf.dbg.dbsize = 8;
	conf.dbg.dwsize = 4;
	conf.dbg.dmsize = 127;
#if defined(__WIN32)
	conf.dbg.font = QFont("Consolas", 10);
#else
	conf.dbg.font = QFont("DejaVu Sans Mono", 10);		// default
#endif
// init volumes
	conf.snd.vol.master = 100;
	conf.snd.vol.beep = 100;
	conf.snd.vol.tape = 100;
	conf.snd.vol.ay = 100;
	conf.snd.vol.gs = 100;
	conf.snd.vol.sdrv = 100;
	conf.snd.vol.saa = 100;
// init palette
	conf.pal["dbg.brk.txt"] = "#ef2929";
	conf.pal["dbg.changed.bg"] = "#ffcece";
	conf.pal["dbg.changed.txt"] = "#000000";
	conf.pal["dbg.header.bg"] = "#4f96ed";
	conf.pal["dbg.header.txt"] = "#000000";
	conf.pal["dbg.pc.bg"] = "#2ccc00";
	conf.pal["dbg.pc.txt"] = "#000000";
	conf.pal["dbg.sel.bg"] = "#98ff98";
	conf.pal["dbg.sel.txt"] = "#000000";

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
			if (pnam=="[DEBUGA]") section = SECT_DEBUGA;
			if (pnam=="[PALETTE]") section = SECT_PALETTE;
			if (pnam=="[KEYS]") section = SECT_KEYS;
		} else {
			switch (section) {
				case SECT_KEYS:
					set_shortcut_name(pnam.c_str(), QKeySequence(pval.c_str()));
					break;
				case SECT_PALETTE:
					col = QColor(pval.c_str());
					if (col.isValid())
						conf.pal[QString(pnam.c_str())] = col;
					break;
				case SECT_DEBUGA:
					if (pnam == "dbsize") conf.dbg.dbsize = atoi(pval.c_str());
					if (pnam == "dwsize") conf.dbg.dwsize = atoi(pval.c_str());
					if (pnam == "dmsize") conf.dbg.dmsize = atoi(pval.c_str());
					if (pnam == "font") conf.dbg.font.fromString(QString(pval.c_str()));
					break;
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
					if (pnam=="greyscale") vid_set_grey(str2bool(pval) ? 1 : 0);
					if (pnam=="scanlines") scanlines = str2bool(pval) ? 1 : 0;
					if (pnam=="shader") conf.vid.shader = pval;
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
						if (pnam=="vga") rsListist.back().vBiosFile=fnam;
					}
					break;
				case SECT_SOUND:
					if (pnam=="enabled") conf.snd.enabled = str2bool(pval) ? 1 : 0;
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
					if (pnam=="startdefault") conf.defProfile = str2bool(pval) ? 1 : 0;
					if (pnam=="savepaths") conf.storePaths = str2bool(pval) ? 1 : 0;
					if (pnam == "fdcturbo") setFlagBit(str2bool(pval),&fdcFlag,FDC_FAST);
					if (pnam == "port") conf.port = strtol(pval.c_str(), NULL, 10) & 0xffff;
					if (pnam == "winpos") {
						vect = splitstr(pval, ",");
						if (vect.size() > 1) {
							conf.xpos = strtol(vect[0].c_str(), NULL, 10);
							conf.ypos = strtol(vect[1].c_str(), NULL, 10);
						}
					}
					if (pnam == "addboot") conf.boot = str2bool(pval.c_str()) ? 1 : 0;
					if (pnam == "exit.confirm") conf.confexit = str2bool(pval.c_str()) ? 1 : 0;
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
					if (pnam=="halt") conf.led.halt = str2bool(pval) ? 1 : 0;
					break;
			}
		}
	}
	vid_set_zoom(conf.vid.scale);
	vid_set_fullscreen(conf.vid.fullScreen);
	vid_set_ratio(conf.vid.keepRatio);
	foreach(xRomset rs, rsListist) addRomset(rs);
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
