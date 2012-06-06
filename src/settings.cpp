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

#ifdef WIN32
	#include <direct.h>
#endif

extern XProfile* currentProfile;

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

extern ZXComp* zx;
std::vector<optEntry> config;
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
	intButton res = {XJ_NONE,XJ_NONE};
	for (uint32_t i=0; i<joyMap.size(); i++) {
		if (joyMap[i].first == extb) {
			res = joyMap[i].second;
			break;
		}
	}
	return res;
}

void optSetJMap(extButton extb,intButton intb) {
	bool exist = false;
	for (uint32_t i=0; i<joyMap.size(); i++) {
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
	for (uint32_t i=0; i<joyMap.size(); i++) {
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

// group-name vars base

std::vector<std::string> optGroupsList() {
	std::vector<std::string> res;
	std::string grp;
	uint i,j;
	bool prs;
	for (i=0; i<config.size(); i++) {
		grp = config[i].group;
		prs = false;
		for (j=0; j<res.size(); j++) {
			if (res[j] == grp) {
				prs = true;
				break;
			}
		}
		if (!prs) res.push_back(grp);
	}
	return res;
}

std::vector<optEntry> optGroupEntries(std::string grp) {
	std::vector<optEntry> res;
	for (uint i=0; i<config.size(); i++) {
		if (config[i].group == grp) res.push_back(config[i]);
	}
	return res;
}

optEntry* optFindEntry(std::string grp, std::string nam) {
	optEntry* res = NULL;
	for (uint i=0; i<config.size(); i++) {
		if ((config[i].group == grp) && (config[i].name == nam)) {
			res = &config[i];
			break;
		}
	}
	return res;
}

void delOption(std::string grp, std::string nam) {
	uint i;
	for (i = 0; i < config.size(); i++) {
		if ((config[i].group == grp) && (config[i].name == nam)) config.erase(config.begin() + i);
	}
}

optEntry* addOption(std::string grp, std::string nam) {
	optEntry* res = optFindEntry(grp, nam);
	if (res != NULL) return res;
	optEntry nent;
	nent.group = grp;
	nent.name = nam;
	nent.value = "";
	config.push_back(nent);
	return &config[config.size() - 1];
}

void optSet(std::string grp, std::string nam, std::string val) {
	optEntry* res = addOption(grp, nam);
	res->value = val;
}

void optSet(std::string grp, std::string nam, int val) {
	optEntry* res = addOption(grp, nam);
	res->value = int2str(val);
}

void optSet(std::string grp, std::string nam, bool val) {
	optEntry* res = addOption(grp, nam);
	res->value = val ? "yes" : "no";
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

std::string optGetString(std::string grp, std::string nam) {
	std::string res = "";
	optEntry* ent = optFindEntry(grp,nam);
	if (ent != NULL) res = ent->value;
	return res;
}

int optGetInt(std::string grp, std::string nam) {
	std::string res = optGetString(grp,nam);
	return atoi(res.c_str());
}

bool optGetBool(std::string grp, std::string nam) {
	std::string res = optGetString(grp,nam);
	return str2bool(res);
}

// old

void initPaths() {
#ifndef WIN32
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
		cfile << int2str(lays[i].intsz) << ":" << int2str(lays[i].intpos.v) << "\n";
	}
	cfile << "scrDir = " << shotDir.c_str() << "\n";
	cfile << "scrFormat = " << optGetName(OPT_SHOTFRM,shotExt).c_str() << "\n";
	cfile << "scrCount = " << int2str(shotCount) << "\n";
	cfile << "scrInterval = " << int2str(shotInterval) << "\n";
	cfile << "colorLevel = " << int2str(brgLevel) << "\n";
	cfile << "fullscreen = " << ((zx->vid->flag & VF_FULLSCREEN) ? "yes" : "no") << "\n";
	cfile << "doublesize = " << ((zx->vid->flag & VF_DOUBLE) ? "yes" : "no") << "\n";
	cfile << "bordersize = " << int2str(zx->vid->brdsize * 100) << "\n";
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
	}
	cfile << "\n[SOUND]\n\n";
	cfile << "enabled = " << ((sndGet(SND_ENABLE) != 0) ? "yes" : "no") << "\n";
	cfile << "soundsys = " << sndGetName().c_str() << "\n";
	cfile << "dontmute = " << ((sndGet(SND_MUTE) != 0) ? "yes" : "no") << "\n";
	cfile << "rate = " << int2str(sndGet(SND_RATE)).c_str() << "\n";
	cfile << "volume.beep = " << int2str(sndGet(SND_BEEP)).c_str() << "\n";
	cfile << "volume.tape = " << int2str(sndGet(SND_TAPE)).c_str() << "\n";
	cfile << "volume.ay = " << int2str(sndGet(SND_AYVL)).c_str() << "\n";
	cfile << "volume.gs = " << int2str(sndGet(SND_GSVL)).c_str() << "\n";
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
	cfile << "disk = " << ((emulGetFlags() & FL_LED_DISK) ? "yes" : "no") << "\n";
	cfile << "scrshot = " << ((emulGetFlags() & FL_LED_SHOT) ? "yes" : "no") << "\n";
	cfile.close();
}

std::string getDiskString(Floppy* flp) {
	std::string res = "80DW";
	if (!(flp->flag & FLP_TRK80)) res[0]='4';
	if (!(flp->flag & FLP_DS)) res[2]='S';
	if (flp->flag & FLP_PROTECT) res[3]='R';
	return res;
}

void setDiskString(Floppy* flp,std::string st) {
	if (st.size() < 4) return;
	flp->flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (st.substr(0,2) == "80") flp->flag |= FLP_TRK80;
	if (st.substr(2,1) == "D") flp->flag |= FLP_DS;
	if (st.substr(3,1) == "R") flp->flag |= FLP_PROTECT;
}

void saveConfig() {
	saveProfiles();
	optSet("GENERAL","cpu.frq",int(zx->cpuFrq * 2));
	optSet("MACHINE","current",std::string(zx->opt.hwName));
	optSet("MACHINE","restart",(emulGetFlags() & FL_RESET) != 0);
	optSet("MACHINE","memory",zx->mem->memSize);
	optSet("MACHINE","scrp.wait",(zx->hwFlags & WAIT_ON) != 0);
	optSet("ROMSET","gs",std::string(zx->opt.GSRom));
	optSet("ROMSET","current",std::string(zx->opt.rsName));
	optSet("ROMSET","reset",rmnam[zx->resbank]);
	optSet("VIDEO","geometry",currentProfile->layName);
	optSet("SOUND","chip1",zx->ts->chipA->type);
	optSet("SOUND","chip2",zx->ts->chipB->type);
	optSet("SOUND","chip1.stereo",zx->ts->chipA->stereo);
	optSet("SOUND","chip2.stereo",zx->ts->chipB->stereo);
	optSet("SOUND","ts.type",zx->ts->type);
	optSet("SOUND","gs",(zx->gs->flag & GS_ENABLE) != 0);
	optSet("SOUND","gs.reset",(zx->gs->flag & GS_RESET) != 0);
	optSet("SOUND","gs.stereo",zx->gs->stereo);
	optSet("DISK","type",zx->bdi->fdc->type);
	optSet("DISK","fast",std::string((zx->bdi->fdc->turbo) ? "yes" : "no"));
	optSet("DISK","A",getDiskString(zx->bdi->fdc->flop[0]));
	optSet("DISK","B",getDiskString(zx->bdi->fdc->flop[1]));
	optSet("DISK","C",getDiskString(zx->bdi->fdc->flop[2]));
	optSet("DISK","D",getDiskString(zx->bdi->fdc->flop[3]));

	optSet("IDE","iface",zx->ide->type);

	optSet("IDE","master.type",zx->ide->master->type);
	ATAPassport pass = ideGetPassport(zx->ide,IDE_MASTER);
	optSet("IDE","master.model",std::string(pass.model,40));
	optSet("IDE","master.serial",std::string(pass.serial,20));
	optSet("IDE","master.image",std::string(zx->ide->master->image));
	optSet("IDE","master.lba",(zx->ide->master->flags & ATA_LBA) != 0);
	optSet("IDE","master.maxlba",zx->ide->master->maxlba);
	std::string chs = int2str(pass.spt) + "/" + int2str(pass.hds) + "/" + int2str(pass.cyls);
	optSet("IDE","master.chs",chs);

	optSet("IDE","slave.type",zx->ide->slave->type);
	pass = ideGetPassport(zx->ide,IDE_SLAVE);
	optSet("IDE","slave.model",std::string(pass.model,40));
	optSet("IDE","slave.serial",std::string(pass.serial,20));
	optSet("IDE","slave.image",std::string(zx->ide->slave->image));
	optSet("IDE","slave.lba",(zx->ide->slave->flags & ATA_LBA) != 0);
	optSet("IDE","slave.maxlba",zx->ide->slave->maxlba);
	chs = int2str(pass.spt) + "/" + int2str(pass.hds) + "/" + int2str(pass.cyls);
	optSet("IDE","slave.chs",chs);

	std::string cfname = workDir + SLASH + getCurrentProfile()->file;
	std::ofstream sfile(cfname.c_str());
	if (!sfile.good()) {
		shitHappens("Can't write settings");
		throw(0);
	}

	uint i,j;
	std::vector<optEntry> ents;
	std::vector<std::string> grps = optGroupsList();
	for (i=0; i<grps.size(); i++) {
		sfile << "[" << grps[i].c_str() << "]\n\n";
		ents = optGroupEntries(grps[i]);
		for (j=0; j<ents.size(); j++) {
			sfile << ents[j].name.c_str() << " = " << ents[j].value.c_str() << "\n";
		}
		sfile << "\n";
	}
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

void loadProfiles() {
	std::string soutnam = "NULL";
	std::ifstream file(profPath.c_str());
	if (!file.good()) {
		printf("Main config is missing. Default files will be copied\n");
		QFile fle(":/conf/config.conf");
		fle.copy(QString(std::string(workDir + SLASH + "config.conf").c_str()));
		fle.setFileName(":/conf/xpeccy.conf");
		fle.copy(QString(std::string(workDir + SLASH + "xpeccy.conf").c_str()));
		fle.setFileName(":/conf/1982.rom");
		fle.copy(QString(std::string(romDir + "/1982.rom").c_str()));
		fle.setPermissions(QString(std::string(workDir + SLASH + "config.conf").c_str()), QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
		fle.setPermissions(QString(std::string(workDir + SLASH + "xpeccy.conf").c_str()), QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
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
	newrs.file = "";
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
						if (vect.size() == 9) {
							vlay.name = vect[0];
							vlay.full.h = atoi(vect[1].c_str()); vlay.full.v = atoi(vect[2].c_str());
							vlay.bord.h = atoi(vect[3].c_str()); vlay.bord.v = atoi(vect[4].c_str());
							vlay.sync.h = atoi(vect[5].c_str()); vlay.sync.v = atoi(vect[6].c_str());
							vlay.intsz = atoi(vect[7].c_str()); vlay.intpos.v = atoi(vect[8].c_str()); vlay.intpos.h = 0;
							if ((vlay.full.h > vlay.bord.h + 256) && (vlay.bord.h > vlay.sync.h) && (vlay.full.v > vlay.bord.v + 192) && (vlay.bord.v > vlay.sync.v)) {
								addLayout(vlay);
							}
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
					if (pnam=="fullscreen") setFlagBit(str2bool(pval),&zx->vid->flag,VF_FULLSCREEN);
					if (pnam=="bordersize") {
						test=atoi(pval.c_str());
						if ((test >= 0) && (test <= 100)) zx->vid->brdsize = test / 100.0;
					}
					if (pnam=="doublesize") setFlagBit(str2bool(pval),&zx->vid->flag,VF_DOUBLE);
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
					}
					break;
				case SECT_SOUND:
					if (pnam=="enabled") sndSet(SND_ENABLE, str2bool(pval));
					if (pnam=="dontmute") sndSet(SND_MUTE, str2bool(pval));
					if (pnam=="soundsys") soutnam = pval;
					if (pnam=="rate") sndSet(SND_RATE,atoi(pval.c_str()));
					if (pnam=="volume.beep") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_BEEP,test);}
					if (pnam=="volume.tape") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_TAPE,test);}
					if (pnam=="volume.ay") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_AYVL,test);}
					if (pnam=="volume.gs") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_GSVL,test);}
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
	for (uint i=0; i<rslist.size(); i++) addRomset(rslist[i]);
	setOutput(soutnam);
	if (!setProfile(pnm.c_str())) {
		shitHappens("Cannot set current profile\nCheck it's name");
		throw(0);
	}
	emulSetColor(brgLevel);
	emulOpenJoystick(joyName);
}

void loadConfig(bool dev) {
	std::string cfname = workDir + SLASH + getCurrentProfile()->file;
	std::ifstream file(cfname.c_str());
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char* buf = new char[0x4000];
	int tmask = 0xff;
	int tmp,tmp2;
	int section = SECT_NONE;
	ATAPassport masterPass = ideGetPassport(zx->ide,IDE_MASTER);
	ATAPassport slavePass = ideGetPassport(zx->ide,IDE_SLAVE);
//	int flg;
	if (!dev) memSetSize(zx->mem,48);
	if (!file.good()) {
//		shithappens(std::string("Can't find config file<br><b>") + cfname + std::string("</b><br>Default one will be created."));
		printf("Profile config is missing. Default one will be created\n");
		QFile fle(":/conf/xpeccy.conf");
		fle.copy(QString(cfname.c_str()));
		fle.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
		file.open(cfname.c_str(),std::ifstream::in);
	}
	if (!file.good()) {
		shitHappens("Damn! I can't open config file<br>Zetsuboushita!");
		throw(0);
	} else {
		RomSet newrs;
		VidLayout vlay;
		std::string fnam,tms;
		config.clear();
		std::string grp = "";
		optEntry nent;
		while (!file.eof()) {
			file.getline(buf,2048);
			line = std::string(buf);
			pos = line.find_first_of("#"); if (pos != std::string::npos) line.erase(pos);
			pos = line.find_first_of(";"); if (pos != std::string::npos) line.erase(pos);
			spl = splitline(line);
			pnam = spl.first;
			pval = spl.second;
			if (pval=="") {
				if (pnam=="[ROMSET]") {grp = pnam; section = SECT_ROMSETS;}
				if (pnam=="[VIDEO]") {grp = pnam; section = SECT_VIDEO;}
				if (pnam=="[SCREENSHOTS]") {grp = ""; section = SECT_SCRSHOT;}
				if (pnam=="[SOUND]") {grp = pnam; section = SECT_SOUND;}
				if ((pnam=="[DISK]") || (pnam=="[BETADISK]")) {grp = "[DISK]"; section = SECT_DISK;}
				if (pnam=="[MACHINE]") {grp = pnam; section = SECT_MACHINE;}
				if (pnam=="[TOOLS]") {grp = ""; section = SECT_TOOLS;}
				if (pnam=="[MENU]") {grp = pnam; section = SECT_MENU;}
				if (pnam=="[IDE]") {grp = pnam; section = SECT_IDE;}
				if (pnam=="[GENERAL]") {grp = pnam; section = SECT_GENERAL;}
				if (dev && (section != SECT_TOOLS)) section = SECT_NONE;
			} else {
				if (grp.size() > 2) {
					line = grp;
					line.erase(line.size()-1,1).erase(0,1);		// remove [ and ] from group name
					optSet(line,pnam,pval);
				}
//printf("%s\t%s\t%s\n",config.back().group.c_str(),config.back().name.c_str(),config.back().value.c_str());
				switch (section) {
					case SECT_ROMSETS:
						if (pnam=="reset") {
							zx->resbank = 1;
							if ((pval=="basic128") || (pval=="0")) zx->resbank = 0;
							if ((pval=="basic48") || (pval=="1")) zx->resbank = 1;
							if ((pval=="shadow") || (pval=="2")) zx->resbank = 2;
							if ((pval=="trdos") || (pval=="3")) zx->resbank = 3;
						}
//						if (pnam=="current") zx->opt.romsetName = pval;
//						if (pnam=="gs") zx->opt.GSRom = pval;
						break;
					case SECT_VIDEO:
						break;
					case SECT_SCRSHOT:
						if (pnam=="folder") shotDir = pval;
						if (pnam=="format") shotExt = optGetId(OPT_SHOTFRM,pval);
						if (pnam=="combo.count") shotCount = atoi(pval.c_str());
						if (pnam=="combo.interval") shotInterval = atoi(pval.c_str());
						break;
					case SECT_SOUND:
						break;
					case SECT_DISK:
						if (pnam=="A") setDiskString(zx->bdi->fdc->flop[0],pval);
						if (pnam=="B") setDiskString(zx->bdi->fdc->flop[1],pval);
						if (pnam=="C") setDiskString(zx->bdi->fdc->flop[2],pval);
						if (pnam=="D") setDiskString(zx->bdi->fdc->flop[3],pval);
						if (pnam=="enabled") {
							zx->bdi->fdc->type = str2bool(pval) ? FDC_93 : FDC_NONE;
							delOption("DISK","enabled");
						}
						if (pnam=="type") zx->bdi->fdc->type = atoi(pval.c_str());
						break;
					case SECT_MACHINE:
						if (pnam=="memory") {
							tmp = atoi(pval.c_str());
							memSetSize(zx->mem,tmp);
							switch (tmp) {
								case 128: tmask = MEM_128; break;
								case 256: tmask = MEM_256; break;
								case 512: tmask = MEM_512; break;
								case 1024: tmask = MEM_1M; break;
							}
						}
						break;
					case SECT_TOOLS:
						if (pnam=="sjasm") asmPath = pval; break;
						if (pnam=="projectsdir") projDir = pval; break;
						break;
					case SECT_MENU:
						addBookmark(pnam.c_str(),pval.c_str());
						break;
					case SECT_IDE:
						if (pnam=="iface") zx->ide->type = atoi(pval.c_str());
						if (pnam=="master.type") zx->ide->master->type = atoi(pval.c_str());
						if (pnam=="master.model") memcpy(masterPass.model,std::string(pval,0,40).c_str(),40);
						if (pnam=="master.serial") memcpy(masterPass.serial,std::string(pval,0,20).c_str(),20);
						if (pnam=="master.lba") {
							//flg = ideGet(zx->ide,IDE_MASTER,IDE_FLAG);
							setFlagBit(str2bool(pval),&zx->ide->master->flags, ATA_LBA);
							//ideSet(zx->ide,IDE_MASTER,IDE_FLAG,flg);
						}
						if (pnam=="master.maxlba") zx->ide->master->maxlba = atoi(pval.c_str());
						if (pnam=="master.image") ideSetImage(zx->ide,IDE_MASTER,pval.c_str());
						if (pnam=="master.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								masterPass.spt = atoi(vect.at(0).c_str());
								masterPass.hds = atoi(vect.at(1).c_str());
								masterPass.cyls = atoi(vect.at(2).c_str());
							}
						}
						if (pnam=="slave.type") zx->ide->slave->type = atoi(pval.c_str());
						if (pnam=="slave.model") memcpy(slavePass.model,std::string(pval,0,40).c_str(),40);
						if (pnam=="slave.serial") memcpy(slavePass.serial,std::string(pval,0,20).c_str(),20);
						if (pnam=="slave.lba") {
							//flg = ideGet(zx->ide,IDE_SLAVE,IDE_FLAG);
							setFlagBit(str2bool(pval),&zx->ide->slave->flags, ATA_LBA);
							//ideSet(zx->ide,IDE_SLAVE,IDE_FLAG,flg);
						}
						if (pnam=="slave.maxlba") zx->ide->slave->maxlba = atoi(pval.c_str());
						if (pnam=="slave.image") ideSetImage(zx->ide,IDE_SLAVE,pval.c_str());
						if (pnam=="slave.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								slavePass.spt = atoi(vect.at(0).c_str());
								slavePass.hds = atoi(vect.at(1).c_str());
								slavePass.cyls = atoi(vect.at(2).c_str());
							}
						}
						break;
					case SECT_GENERAL:
						break;
				}
			}
		}
	}

	if (dev) return;

	delOption("VIDEO","fullscreen");
	delOption("VIDEO","doublesize");

	tmp2 = optGetInt("GENERAL","cpu.frq"); if ((tmp2 > 0) && (tmp2 <= 14)) zxSetFrq(zx,tmp2 / 2.0);

	currentProfile->layName = optGetString("VIDEO","geometry");

	tmp2 = optGetInt("SOUND","chip1"); if (tmp2 < SND_END) aymSetType(zx->ts->chipA,tmp2);
	tmp2 = optGetInt("SOUND","chip2"); if (tmp2 < SND_END) aymSetType(zx->ts->chipB,tmp2);
	zx->ts->chipA->stereo = optGetInt("SOUND","chip1.stereo");
	zx->ts->chipB->stereo = optGetInt("SOUND","chip2.stereo");
	zx->ts->type = optGetInt("SOUND","ts.type");

	zx->gs->flag = 0;
	if (optGetBool("SOUND","gs")) zx->gs->flag |= GS_ENABLE;
	if (optGetBool("SOUND","gs.reset")) zx->gs->flag |= GS_RESET;
	zx->gs->stereo = optGetInt("SOUND","gs.stereo");

	zx->bdi->flag = 0;
	zx->bdi->fdc->turbo = 0;
//	zx->bdi->type = atoi(optGetString("DISK","type").c_str());
	zx->bdi->fdc->turbo = optGetBool("DISK","fast") ? 1 : 0;
	strcpy(zx->opt.hwName,optGetString("MACHINE","current").c_str());
	strcpy(zx->opt.rsName,optGetString("ROMSET","current").c_str());
	strcpy(zx->opt.GSRom,optGetString("ROMSET","gs").c_str());
	emulSetFlag(FL_RESET,optGetBool("MACHINE","restart"));
	setFlagBit(optGetBool("MACHINE","scrp.wait"),&zx->hwFlags,WAIT_ON);

	sndCalibrate();
//	zx->ide->refresh();
	ideSetPassport(zx->ide,IDE_MASTER,masterPass);
	ideSetPassport(zx->ide,IDE_SLAVE,slavePass);
	setHardware(zx, zx->opt.hwName);
	setRomset(zx, zx->opt.rsName);
	if (zx->hw==NULL) throw("Can't found current machine");
	if (currentProfile->rset == NULL) throw("Can't found current romset");
	if ((zx->hw->mask != 0) && (~zx->hw->mask & tmask)) throw("Incorrect memory size for this machine");
	if (!emulSetLayout(zx->vid,currentProfile->layName)) emulSetLayout(zx->vid,"default");
}
