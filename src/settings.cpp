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

std::string workDir;
std::string romDir;
std::string profPath;
std::string joyName;
std::string keyFileName;

int brgLevel = 192;
int flag = 0;

int shotExt;
std::string shotDir;
int shotCount;
int shotInterval;

std::string projDir;
std::string asmPath;

std::string rmnam[] = {"basic128","basic48","shadow","trdos","ext4","ext5","ext6","ext7"};
OptName fexts[] = {{SCR_BMP,"bmp"},{SCR_PNG,"png"},{SCR_JPG,"jpg"},{SCR_SCR,"scr"},{SCR_HOB,"hobeta"},{-1,""}};
OptName jdirs[] = {{XJ_LEFT,"left"},{XJ_RIGHT,"right"},{XJ_UP,"up"},{XJ_DOWN,"down"},{XJ_FIRE,"fire"},{-1,""}};

std::vector<joyPair> joyMap;

// new

bool extButton::operator ==(extButton b1) {
	return ((type == b1.type) && (num == b1.num) && (dir == b1.dir));
}

std::vector<joyPair> getJMap() {
	return joyMap;
}

void setJMap(std::vector<joyPair> newmap) {
	joyMap = newmap;
}

intButton optGetJMap(extButton extb) {
	unsigned int i;
	intButton res = {XJ_NONE,XJ_NONE};
	for (i = 0; i < joyMap.size(); i++) {
		if (joyMap[i].first == extb) {
			res = joyMap[i].second;
			break;
		}
	}
	return res;
}

void optSetJMap(extButton extb,intButton intb) {
	bool exist = false;
	unsigned int i;
	for (i = 0; i < joyMap.size(); i++) {
		if (joyMap[i].first == extb) {
			joyMap[i].second = intb;
			exist = true;
			break;
		}
	}
	if (!exist) {
		joyPair jpair;
		jpair.first = extb;
		jpair.second = intb;
		joyMap.push_back(jpair);
	}
}

void optDelJMap(extButton extb) {
	for (unsigned int i=0; i<joyMap.size(); i++) {
		if (joyMap[i].first == extb) {
			joyMap.erase(joyMap.begin() + i);
		}
	}
}

// static vars base
std::string optGetString(int wut) {
	std::string res;
	switch (wut) {
		case OPT_WORKDIR: res = workDir; break;
		case OPT_ROMDIR: res = romDir; break;
		case OPT_SHOTDIR: res = shotDir; break;
//		case OPT_SHOTEXT: res = shotExt; break;
		case OPT_PROJDIR: res = projDir; break;
		case OPT_ASMPATH: res = asmPath; break;
		case OPT_JOYNAME: res = joyName; break;
		case OPT_KEYNAME: res = keyFileName; break;
	}
	return res;
}

OptName* getGetPtr(int prt) {
	OptName* res = NULL;
	switch (prt) {
		case OPT_SHOTFRM: res = fexts; break;
		case OPT_JOYDIRS: res = jdirs; break;
	}
	return res;
}

int optGetId(int prt,std::string nam) {
	int res = -1;
	OptName* ptr = getGetPtr(prt);
	if (ptr == NULL) return res;
	int i = -1;
	do {
		i++;
		if ((ptr[i].id == -1) || (ptr[i].name == nam)) {
			res = ptr[i].id;
			break;
		}
	} while ((ptr[i].id != -1) && (res == -1));
	return res;
}

std::string optGetName(int prt, int id) {
	std::string res = "";
	OptName* ptr = getGetPtr(prt);
	if (ptr == NULL) return res;
	int i = -1;
	do {
		i++;
		if ((ptr[i].id == -1) || (ptr[i].id == id)) {
			res = ptr[i].name;
			break;
		}
	} while ((ptr[i].id != -1) && (res == ""));
	return res;
}

int optGetInt(int wut) {
	int res = 0;
	switch (wut) {
		case OPT_SHOTINT: res = shotInterval; break;
		case OPT_SHOTCNT: res = shotCount; break;
		case OPT_BRGLEV: res = brgLevel; break;
		case OPT_SHOTFRM: res = shotExt; break;
	}
	return res;
}

void optSet(int wut, std::string val) {
	switch(wut) {
//		case OPT_SHOTEXT: shotExt = val; break;
		case OPT_SHOTDIR: shotDir = val; break;
		case OPT_PROJDIR: projDir = val; break;
		case OPT_ASMPATH: asmPath = val; break;
		case OPT_JOYNAME: joyName = val; break;
		case OPT_KEYNAME: keyFileName = val; break;
	}
}

void optSet(int wut, int val) {
	switch (wut) {
		case OPT_SHOTINT: shotInterval = val; break;
		case OPT_SHOTCNT: shotCount = val; break;
		case OPT_SHOTFRM: shotExt = val; break;
		case OPT_BRGLEV: brgLevel = val; break;
	}
}

void optSetFlag(int mask, bool wut) {
	if (wut) {
		flag |= mask;
	} else {
		flag &= ~mask;
	}
}

bool optGetFlag(int mask) {
	return ((flag & mask) != 0);
}

// old

void initPaths() {
#if __linux
// move config dir to new place
	QDir dir;
	QString newpath = QDir::homePath() + "/.config/samstyle/xpeccy";
	if (!dir.exists(newpath)) {
		QFile file;
		QString oldpath = QDir::homePath() + "/.samstyle/samulator";
		QString oldfile = oldpath + "/samulator.conf";
		QString newfile = newpath + "/xpeccy.conf";
		dir.mkpath(newpath);
		file.rename(oldfile,newfile);
		dir.rename(oldpath + "/roms",newpath + "/roms");
		dir.rmdir(oldpath);
	}
	workDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy";
	romDir = workDir + "/roms";
	profPath = workDir + "/config.conf";
	mkdir(workDir.c_str(),0777);
	mkdir(romDir.c_str(),0777);
	optSet(OPT_SHOTDIR,std::string(getenv("HOME")));
#else
	workDir = std::string(".\\config");
	romDir = workDir + "\\roms";
	profPath = workDir + "\\config.conf";
	mkdir(workDir.c_str());
	mkdir(romDir.c_str());
	optSet(OPT_SHOTDIR,std::string(getenv("HOMEPATH")));
#endif
}

std::string getJValue(int type,int val) {
	std::string res = "";
	switch (type) {
		case XJ_JOY:
			switch (val) {
				case XJ_UP: res = "up"; break;
				case XJ_DOWN: res = "down"; break;
				case XJ_LEFT: res = "left"; break;
				case XJ_RIGHT: res = "right"; break;
				case XJ_FIRE: res = "fire"; break;
			}
			break;
	}
	return res;
}

void saveProfiles() {
	std::string cfname = workDir + SLASH + "config.conf";
	std::ofstream cfile(cfname.c_str());
	if (!cfile.good()) {
		shitHappens("Can't write main config");
		throw(0);
	}
	uint i,j;

	cfile << "[GENERAL]\n\n";
	if ((keyFileName != "default") && (keyFileName != "")) {
		cfile << "keys = " << keyFileName.c_str() << "\n";
	}
	cfile << "startdefault = " << ((flag & OF_DEFAULT) ? "yes" : "no") << "\n";
	cfile << "savepaths = " << ((flag & OF_PATHS) ? "yes" : "no") << "\n";
	cfile << "fdcturbo = " << ((fdcFlag & FDC_FAST) ? "yes" : "no") << "\n";
	cfile << "systime = " << ((emulFlags & FL_SYSCLOCK) ? "yes" : "no") << "\n";

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
	cfile << "scrDir = " << shotDir.c_str() << "\n";
	cfile << "scrFormat = " << optGetName(OPT_SHOTFRM,shotExt).c_str() << "\n";
	cfile << "scrCount = " << int2str(shotCount) << "\n";
	cfile << "scrInterval = " << int2str(shotInterval) << "\n";
	cfile << "colorLevel = " << int2str(brgLevel) << "\n";
	cfile << "fullscreen = " << ((vidFlag & VF_FULLSCREEN) ? "yes" : "no") << "\n";
	cfile << "doublesize = " << ((vidFlag & VF_DOUBLE) ? "yes" : "no") << "\n";
	cfile << "greyscale = " << ((vidFlag & VF_GREY) ? "yes" : "no") << "\n";
	cfile << "bordersize = " << int2str(brdsize * 100) << "\n";
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
					cfile << rmnam[j].c_str()<<" = " << rsl[i].roms[j].path.c_str();
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
	cfile << "\n[TOOLS]\n\n";
	cfile << "asmPath = " << asmPath.c_str() << "\n";
	cfile << "projectsDir = " << projDir.c_str() << "\n";

	cfile << "\n[TAPE]\n\n";
	cfile << "autoplay = " << ((flag & OF_TAPEAUTO) ? "yes" : "no") << "\n";
	cfile << "fast = " << ((flag & OF_TAPEFAST) ? "yes" : "no") << "\n";

	cfile << "\n[JOYSTICK]\n\n";
	cfile << "device = " << joyName.c_str() << "\n";
	for (i=0; i<joyMap.size(); i++) {
		switch(joyMap[i].first.type) {
			case XJ_BUTTON:
				cfile << "button = ";
				cfile << int2str(joyMap[i].first.num).c_str() << ":";
				cfile << ((joyMap[i].second.dev == XJ_JOY) ? "J:" : "K:");
				cfile << joyMap[i].second.name << "\n";
				break;
			case XJ_AXIS:
				cfile << "axis = ";
				cfile << int2str(joyMap[i].first.num).c_str() << ":";
				cfile << (joyMap[i].first.dir ? "+:" : "-:");
				cfile << ((joyMap[i].second.dev == XJ_JOY) ? "J:" : "K:");
				cfile << joyMap[i].second.name << "\n";
				break;
		}
	}

	cfile << "\n[LEDS]\n\n";
	cfile << "disk = " << ((emulFlags & FL_LED_DISK) ? "yes" : "no") << "\n";
	cfile << "scrshot = " << ((emulFlags & FL_LED_SHOT) ? "yes" : "no") << "\n";
	cfile.close();
}


void loadKeys() {
	std::string sfnam = workDir + SLASH + keyFileName;
	initKeyMap();
	if ((keyFileName == "") || (keyFileName == "default")) return;
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

void loadProfiles() {
	std::string soutnam = "NULL";
	std::ifstream file(profPath.c_str());
	if (!file.good()) {
		printf("Main config is missing. Default files will be copied\n");
		copyFile(":/conf/config.conf",std::string(workDir + SLASH + "config.conf").c_str());
		copyFile(":/conf/xpeccy.conf",std::string(workDir + SLASH + "xpeccy.conf").c_str());
		copyFile(":/conf/1982.rom",std::string(romDir + SLASH + "1982.rom").c_str());
		file.open(profPath.c_str());
		if (!file.good()) {
			printf("%s\n",profPath.c_str());
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
	extButton extb;
	intButton intb;
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
			if (pnam=="[JOYSTICK]") section = SECT_JOYSTICK;
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
					if (pnam=="scrDir") shotDir = pval;
					if (pnam=="scrFormat") {
						shotExt = optGetId(OPT_SHOTFRM,pval);
					}
					if (pnam=="scrCount") shotCount = atoi(pval.c_str());
					if (pnam=="scrInterval") shotInterval = atoi(pval.c_str());
					if (pnam=="colorLevel") {
						test=atoi(pval.c_str());
						if ((test < 50) || (test > 250)) test=192;
						brgLevel = test;
					}
					if (pnam=="fullscreen") setFlagBit(str2bool(pval),&vidFlag,VF_FULLSCREEN);
					if (pnam=="bordersize") {
						test=atoi(pval.c_str());
						if ((test >= 0) && (test <= 100)) brdsize = test / 100.0;
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
					if (pnam=="asmPath") asmPath = pval;
					if (pnam=="projectsDir") projDir = pval;
					break;
				case SECT_JOYSTICK:
					if (pnam=="device") joyName = pval;
					if (pnam=="button") {			// button = num:J:{up|down|left|right|fire}
						vect = splitstr(pval,":");	// button = num:K:keyName
						if (vect.size() > 2) {
							extb.type = XJ_BUTTON;
							extb.num = atoi(vect[0].c_str());
							extb.dir = true;
							intb.name = vect[2].c_str();
							if ((vect[1] == "J") || (vect[1] == "K")) {
								intb.dev = (vect[1] == "J") ? XJ_JOY : XJ_KEY;
								optSetJMap(extb,intb);
							}
						}
					}
					if (pnam=="axis") {			// axis = num:{+|-}:J:{up|down|left|right|fire}
						vect = splitstr(pval,":");	// axis = num:{+|-}:K:keyName
						if (vect.size() > 3) {
							extb.type = XJ_AXIS;
							extb.num = atoi(vect[0].c_str());
							extb.dir = (vect[1] == "+");
							intb.name = vect[2].c_str();
							if ((vect[1] == "J") || (vect[1] == "K")) {
								intb.dev = (vect[1] == "J") ? XJ_JOY : XJ_KEY;
								optSetJMap(extb,intb);
							}
						}
					}
				case SECT_GENERAL:
					if (pnam=="keys") {
						keyFileName = pval;
						loadKeys();
					}
					if (pnam=="startdefault") optSetFlag(OF_DEFAULT,str2bool(pval));
					if (pnam=="savepaths") optSetFlag(OF_PATHS,str2bool(pval));
					if (pnam == "fdcturbo") setFlagBit(str2bool(pval),&fdcFlag,FDC_FAST);
					if (pnam == "systime") emulSetFlag(FL_SYSCLOCK,str2bool(pval));
					break;
				case SECT_TAPE:
					if (pnam=="autoplay") optSetFlag(OF_TAPEAUTO,str2bool(pval));
					if (pnam=="fast") optSetFlag(OF_TAPEFAST,str2bool(pval));
					break;
				case SECT_LEDS:
					if (pnam=="disk") emulSetFlag(FL_LED_DISK,str2bool(pval));
					if (pnam=="scrshot") emulSetFlag(FL_LED_SHOT,str2bool(pval));
					break;
			}
		}
	}
	uint i;
	for (i=0; i<rslist.size(); i++) addRomset(rslist[i]);
	prfLoadAll();
	setOutput(soutnam.c_str());
	if (flag & OF_DEFAULT) {
		if (!setProfile("default")) {
			printf("Can't set default profile. GRR!\n");
			throw(0);
		}
	} else {
		if (!setProfile(pnm.c_str())) {
			shitHappens("Cannot set current profile\nDefault will be used");
			if (!setProfile("default")) {
				shitHappens("...and default too?\nReally, shit happens");
				throw(0);
			}
		}
	}
	zx->flag |= ZX_PALCHAN;
	// emulSetPalette(zx,brgLevel);
	emulOpenJoystick(joyName);
}

// profile config
