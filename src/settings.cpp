#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

#include <QtCore>

#include "common.h"
#include "sound.h"
#include "emulwin.h"
#include "settings.h"

#ifdef WIN32
#include <direct.h>
#endif

extern ZXComp* zx;
std::vector<optEntry> config;
std::string workDir;
std::string romDir;
std::string profPath;
std::string joyName;

int brgLevel = 192;

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
	std::string cfname = workDir + "/config.conf";
	std::ofstream cfile(cfname.c_str());
	if (!cfile.good()) {
		shitHappens("Can't write main config");
		throw(0);
	}
	uint i,j;
	cfile << "[BOOKMARKS]\n\n";
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
		cfile << int2str(lays[i].intsz) << ":" << int2str(lays[i].intpos) << "\n";
	}
	cfile << "scrDir = " << shotDir.c_str() << "\n";
	cfile << "scrFormat = " << optGetName(OPT_SHOTFRM,shotExt).c_str() << "\n";
	cfile << "scrCount = " << int2str(shotCount) << "\n";
	cfile << "scrInterval = " << int2str(shotInterval) << "\n";
	cfile << "colorLevel = " << int2str(brgLevel) << "\n";
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

	cfile << "\n[JOYSTICK]\n\n";
	cfile << "device = " << joyName.c_str() << "\n";
	for (i=0; i<joyMap.size(); i++) {
		switch(joyMap[i].first.type) {
			case XJ_BUTTON:
				cfile << "button = ";
				cfile << int2str(joyMap[i].first.num).c_str() << ":";
				cfile << ((joyMap[i].second.dev == XJ_JOY) ? "J:" : "K:");
				cfile << getJValue(joyMap[i].second.dev,joyMap[i].second.value).c_str() << "\n";
				break;
			case XJ_AXIS:
				cfile << "axis = ";
				cfile << int2str(joyMap[i].first.num).c_str() << ":";
				cfile << (joyMap[i].first.dir ? "+:" : "-:");
				cfile << ((joyMap[i].second.dev == XJ_JOY) ? "J:" : "K:");
				cfile << getJValue(joyMap[i].second.dev,joyMap[i].second.value).c_str() << "\n";
				break;
		}
	}
	cfile.close();
}

std::string getDiskString(Floppy* flp) {
	std::string res = "80DW";
	if (!flpGetFlag(flp,FLP_TRK80)) res[0]='4';
	if (!flpGetFlag(flp,FLP_DS)) res[2]='S';
	if (flpGetFlag(flp,FLP_PROTECT)) res[3]='R';
	return res;
}

void setDiskString(Floppy* flp,std::string st) {
	if (st.size() < 4) return;
	flpSetFlag(flp,FLP_TRK80 | FLP_DS | FLP_PROTECT,false);
	if (st.substr(0,2) == "80") flpSetFlag(flp,FLP_TRK80,true);
	if (st.substr(2,1) == "D") flpSetFlag(flp,FLP_DS,true);
	if (st.substr(3,1) == "R") flpSetFlag(flp,FLP_PROTECT,true);
}

void saveConfig() {
	saveProfiles();
	optSet("GENERAL","cpu.frq",int(zx->cpuFreq * 2));
	optSet("MACHINE","current",zx->opt.hwName);
	optSet("MACHINE","restart",(emulGetFlags() & FL_RESET) != 0);
	optSet("MACHINE","memory",memGet(zx->mem,MEM_MEMSIZE));
	optSet("MACHINE","scrp.wait",(zx->hwFlags & WAIT_ON) != 0);
	optSet("ROMSET","gs",zx->opt.GSRom);
	optSet("ROMSET","current",zx->opt.rsName);
	optSet("ROMSET","reset",rmnam[zx->resbank]);
	optSet("VIDEO","doublesize",(zx->vid->flags & VF_DOUBLE) != 0);
	optSet("VIDEO","bordersize",int(zx->vid->brdsize * 100));
	optSet("VIDEO","geometry",zx->vid->curlay);
	optSet("SOUND","chip1",tsGet(zx->ts,AY_TYPE,0));
	optSet("SOUND","chip2",tsGet(zx->ts,AY_TYPE,1));
	optSet("SOUND","chip1.stereo",tsGet(zx->ts,AY_STEREO,0));
	optSet("SOUND","chip2.stereo",tsGet(zx->ts,AY_STEREO,1));
	optSet("SOUND","ts.type",tsGet(zx->ts,TS_TYPE,0));
	optSet("SOUND","gs",(gsGet(zx->gs,GS_FLAG) & GS_ENABLE) != 0);
	optSet("SOUND","gs.reset",(gsGet(zx->gs,GS_FLAG) & GS_RESET) != 0);
	optSet("SOUND","gs.stereo",gsGet(zx->gs,GS_STEREO));
	optSet("BETADISK","enabled",bdiGetFlag(zx->bdi,BDI_ENABLE));
	optSet("BETADISK","fast",bdiGetFlag(zx->bdi,BDI_TURBO));
	optSet("BETADISK","A",getDiskString(bdiGetFloppy(zx->bdi,0)));
	optSet("BETADISK","B",getDiskString(bdiGetFloppy(zx->bdi,1)));
	optSet("BETADISK","C",getDiskString(bdiGetFloppy(zx->bdi,2)));
	optSet("BETADISK","D",getDiskString(bdiGetFloppy(zx->bdi,3)));

	optSet("IDE","iface",ideGet(zx->ide,IDE_NONE,IDE_TYPE));

	optSet("IDE","master.type",ideGet(zx->ide,IDE_MASTER,IDE_TYPE));
	ATAPassport pass = ideGetPassport(zx->ide,IDE_MASTER);
	optSet("IDE","master.model",pass.model);
	optSet("IDE","master.serial",pass.serial);
	optSet("IDE","master.image",ideGetPath(zx->ide,IDE_MASTER));
	optSet("IDE","master.lba",(ideGet(zx->ide,IDE_MASTER,IDE_FLAG) & ATA_LBA) != 0);
	optSet("IDE","master.maxlba",ideGet(zx->ide,IDE_MASTER,IDE_MAXLBA));
	std::string chs = int2str(pass.spt) + "/" + int2str(pass.hds) + "/" + int2str(pass.cyls);
	optSet("IDE","master.chs",chs);

	optSet("IDE","slave.type",ideGet(zx->ide,IDE_SLAVE,IDE_TYPE));
	pass = ideGetPassport(zx->ide,IDE_MASTER);
	optSet("IDE","slave.model",pass.model);
	optSet("IDE","slave.serial",pass.serial);
	optSet("IDE","slave.image",ideGetPath(zx->ide,IDE_SLAVE));
	optSet("IDE","slave.lba",(ideGet(zx->ide,IDE_MASTER,IDE_FLAG) & ATA_LBA) != 0);
	optSet("IDE","slave.maxlba",ideGet(zx->ide,IDE_MASTER,IDE_MAXLBA));
	chs = int2str(pass.spt) + "/" + int2str(pass.hds) + "/" + int2str(pass.cyls);
	optSet("IDE","slave.chs",chs);

	std::string cfname = workDir + "/" + getCurrentProfile()->file;
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

void loadProfiles() {
	std::string soutnam = "NULL";
	std::ifstream file(profPath.c_str());
	if (!file.good()) {
		printf("Main config is missing. Default files will be copied\n");
		QFile fle(":/conf/config.conf");
		fle.copy(QString(std::string(workDir + "/config.conf").c_str()));
		fle.setFileName(":/conf/xpeccy.conf");
		fle.copy(QString(std::string(workDir + "/xpeccy.conf").c_str()));
		fle.setFileName(":/conf/1982.rom");
		fle.copy(QString(std::string(romDir + "/1982.rom").c_str()));
		fle.setPermissions(QString(std::string(workDir + "/config.conf").c_str()), QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
		fle.setPermissions(QString(std::string(workDir + "/xpeccy.conf").c_str()), QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
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
	int section = 0;
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
			if (pnam=="[BOOKMARKS]") section=1;
			if (pnam=="[PROFILES]") section=2;
			if (pnam=="[VIDEO]") section=3;
			if (pnam=="[ROMSETS]") section=4;
			if (pnam=="[SOUND]") section=5;
			if (pnam=="[TOOLS]") section=6;
			if (pnam=="[JOYSTICK]") section=7;
		} else {
			switch (section) {
				case 1:
					addBookmark(pnam,pval);
					break;
				case 2:
					if (pnam == "current") {
						pnm = pval;
					} else {
						addProfile(pnam,pval);
					}
					break;
				case 3:
					if (pnam=="layout") {
						vect = splitstr(pval,":");
						if (vect.size() == 9) {
							vlay.name = vect[0];
							vlay.full.h = atoi(vect[1].c_str()); vlay.full.v = atoi(vect[2].c_str());
							vlay.bord.h = atoi(vect[3].c_str()); vlay.bord.v = atoi(vect[4].c_str());
							vlay.sync.h = atoi(vect[5].c_str()); vlay.sync.v = atoi(vect[6].c_str());
							vlay.intsz = atoi(vect[7].c_str()); vlay.intpos = atoi(vect[8].c_str());
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
					break;
				case 4:
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
				case 5:
					if (pnam=="enabled") sndSet(SND_ENABLE, str2bool(pval));
					if (pnam=="dontmute") sndSet(SND_MUTE, str2bool(pval));
					if (pnam=="soundsys") soutnam = pval;
					if (pnam=="rate") sndSet(SND_RATE,atoi(pval.c_str()));
					if (pnam=="volume.beep") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_BEEP,test);}
					if (pnam=="volume.tape") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_TAPE,test);}
					if (pnam=="volume.ay") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_AYVL,test);}
					if (pnam=="volume.gs") {test = atoi(pval.c_str()); if (test > 100) test = 100; sndSet(SND_GSVL,test);}
					break;
				case 6:
					if (pnam=="asmPath") asmPath = pval; break;
					if (pnam=="projectsDir") projDir = pval; break;
				case 7:
					if (pnam=="device") joyName = pval;
					if (pnam=="button") {			// button = num:J:{up,down,left,right,fire}
						vect = splitstr(pval,":");	// TODO: button = num:K:keyName
						if (vect.size() > 2) {
							if (vect[1] == "J") {
								fprt = 0;
								if (vect[2] == "up") fprt = XJ_UP;
								if (vect[2] == "down") fprt = XJ_DOWN;
								if (vect[2] == "left") fprt = XJ_LEFT;
								if (vect[2] == "right") fprt = XJ_RIGHT;
								if (vect[2] == "fire") fprt = XJ_FIRE;
								if (fprt != 0) {
									extb.type = XJ_BUTTON;
									extb.num = atoi(vect[0].c_str());
									extb.dir = true;
									intb.dev  =XJ_JOY;
									intb.value = fprt;
									optSetJMap(extb,intb);
								}
							}
						}
					}
					if (pnam=="axis") {			// axis = num:{+,-},J:{up,down,left,right,fire}
						vect = splitstr(pval,":");
						if (vect.size() > 3) {
							if (vect[2] == "J") {
								fprt = 0;
								if (vect[3] == "up") fprt = XJ_UP;
								if (vect[3] == "down") fprt = XJ_DOWN;
								if (vect[3] == "left") fprt = XJ_LEFT;
								if (vect[3] == "right") fprt = XJ_RIGHT;
								if (vect[3] == "fire") fprt = XJ_FIRE;
								if (fprt != 0) {
									extb.type = XJ_AXIS;
									extb.num = atoi(vect[0].c_str());
									extb.dir = (vect[1] == "+");
									intb.dev  =XJ_JOY;
									intb.value = fprt;
									optSetJMap(extb,intb);
								}
							}
						}
					}
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
	std::string cfname = workDir + "/" + getCurrentProfile()->file;
	std::ifstream file(cfname.c_str());
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char* buf = new char[0x4000];
	int tmask = 0xff;
	int tmp;
	int tmp2=0;
	ATAPassport masterPass = ideGetPassport(zx->ide,IDE_MASTER);
	ATAPassport slavePass = ideGetPassport(zx->ide,IDE_SLAVE);
	int flg;
	if (!dev) memSet(zx->mem,MEM_MEMSIZE,48);
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
				if (pnam=="[ROMSET]") {grp=pnam; tmp2=1;}
				if (pnam=="[VIDEO]") {grp=pnam; tmp2=2;}
				if (pnam=="[SCREENSHOTS]") {grp=""; tmp2=3;}
				if (pnam=="[SOUND]") {grp=pnam; tmp2=4;}
				if (pnam=="[BETADISK]") {grp=pnam; tmp2=5;}
				if (pnam=="[MACHINE]") {grp=pnam; tmp2=6;}
				if (pnam=="[TOOLS]") {grp=""; tmp2=7;}
				if (pnam=="[MENU]") {grp=pnam; tmp2=8;}
				if (pnam=="[IDE]") {grp=pnam; tmp2=9;}
				if (pnam=="[GENERAL]") {grp=pnam; tmp2=10;}
				if (dev && (tmp2 != 7)) tmp2 = 0;
			} else {
				if (grp.size() > 2) {
					line = grp;
					line.erase(line.size()-1,1).erase(0,1);		// remove [ and ] from group name
					optSet(line,pnam,pval);
				}
//printf("%s\t%s\t%s\n",config.back().group.c_str(),config.back().name.c_str(),config.back().value.c_str());
				switch (tmp2) {
					case 1:
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
					case 2:
						break;
					case 3:
						if (pnam=="folder") shotDir = pval;
						if (pnam=="format") shotExt = optGetId(OPT_SHOTFRM,pval);
						if (pnam=="combo.count") shotCount = atoi(pval.c_str());
						if (pnam=="combo.interval") shotInterval = atoi(pval.c_str());
						break;
					case 4:
						break;
					case 5:
						if (pnam=="A") setDiskString(bdiGetFloppy(zx->bdi,0),pval);
						if (pnam=="B") setDiskString(bdiGetFloppy(zx->bdi,1),pval);
						if (pnam=="C") setDiskString(bdiGetFloppy(zx->bdi,2),pval);
						if (pnam=="D") setDiskString(bdiGetFloppy(zx->bdi,3),pval);
						break;
					case 6:
						if (pnam=="memory") {
							tmp = atoi(pval.c_str());
							memSet(zx->mem,MEM_MEMSIZE,tmp);
							switch (tmp) {
								case 128: tmask = 1; break;
								case 256: tmask = 2; break;
								case 512: tmask = 4; break;
								case 1024: tmask = 8; break;
							}
						}
						break;
					case 7:
						if (pnam=="sjasm") asmPath = pval; break;
						if (pnam=="projectsdir") projDir = pval; break;
						break;
					case 8: addBookmark(pnam.c_str(),pval.c_str());
						break;
					case 9:
						if (pnam=="iface") ideSet(zx->ide,IDE_NONE,IDE_TYPE,atoi(pval.c_str()));
						if (pnam=="master.type") ideSet(zx->ide,IDE_MASTER,IDE_TYPE,atoi(pval.c_str()));
						if (pnam=="master.model") masterPass.model = std::string(pval,0,40);
						if (pnam=="master.serial") masterPass.serial = std::string(pval,0,20);
						if (pnam=="master.lba") {
							flg = ideGet(zx->ide,IDE_MASTER,IDE_FLAG);
							setFlagBit(str2bool(pval),&flg, ATA_LBA);
							ideSet(zx->ide,IDE_MASTER,IDE_FLAG,flg);
						}
						if (pnam=="master.maxlba") ideSet(zx->ide,IDE_MASTER,IDE_MAXLBA,atoi(pval.c_str()));
						if (pnam=="master.image") ideSetPath(zx->ide,IDE_MASTER,pval);
						if (pnam=="master.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								masterPass.spt = atoi(vect.at(0).c_str());
								masterPass.hds = atoi(vect.at(1).c_str());
								masterPass.cyls = atoi(vect.at(2).c_str());
							}
						}
						if (pnam=="slave.type") ideSet(zx->ide,IDE_SLAVE,IDE_TYPE,atoi(pval.c_str()));
						if (pnam=="slave.model") slavePass.model = std::string(pval,0,40);
						if (pnam=="slave.serial") slavePass.serial = std::string(pval,0,20);
						if (pnam=="slave.lba") {
							flg = ideGet(zx->ide,IDE_SLAVE,IDE_FLAG);
							setFlagBit(str2bool(pval),&flg, ATA_LBA);
							ideSet(zx->ide,IDE_SLAVE,IDE_FLAG,flg);
						}
						if (pnam=="slave.maxlba") ideSet(zx->ide,IDE_SLAVE,IDE_MAXLBA,atoi(pval.c_str()));
						if (pnam=="slave.image") ideSetPath(zx->ide,IDE_SLAVE,pval);
						if (pnam=="slave.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								slavePass.spt = atoi(vect.at(0).c_str());
								slavePass.hds = atoi(vect.at(1).c_str());
								slavePass.cyls = atoi(vect.at(2).c_str());
							}
						}
						break;
					case 10:
						break;
				}
			}
		}
	}
	if (dev) return;

	tmp2 = optGetInt("GENERAL","cpu.frq"); if ((tmp2 > 0) && (tmp2 <= 14)) zx->cpuFreq = tmp2 / 2.0;

	zx->vid->curlay = optGetString("VIDEO","geometry");
	setFlagBit(optGetBool("VIDEO","doublesize"),&zx->vid->flags, VF_DOUBLE);
	setFlagBit(optGetBool("VIDEO","fullscreen"),&zx->vid->flags, VF_FULLSCREEN);
	tmp2 = optGetInt("VIDEO","bordersize"); if ((tmp2 >= 0) && (tmp2 <= 100)) zx->vid->brdsize = tmp2 / 100.0;

	tmp2 = optGetInt("SOUND","chip1"); if (tmp2 < SND_END) tsSet(zx->ts,AY_TYPE,0,tmp2);
	tmp2 = optGetInt("SOUND","chip2"); if (tmp2 < SND_END) tsSet(zx->ts,AY_TYPE,1,tmp2);
	tsSet(zx->ts,AY_STEREO,0,optGetInt("SOUND","chip1.stereo"));
	tsSet(zx->ts,AY_STEREO,1,optGetInt("SOUND","chip2.stereo"));
	tsSet(zx->ts,TS_TYPE,0,optGetInt("SOUND","ts.type"));

	int gsf = 0;
	if (optGetBool("SOUND","gs")) gsf |= GS_ENABLE;
	if (optGetBool("SOUND","gs.reset")) gsf |= GS_RESET;
	gsSet(zx->gs,GS_FLAG,gsf);
	gsSet(zx->gs,GS_STEREO,optGetInt("SOUND","gs.stereo"));

	bdiSetFlag(zx->bdi,BDI_ENABLE,optGetBool("BETADISK","enabled"));
	bdiSetFlag(zx->bdi,BDI_TURBO,optGetBool("BETADISK","fast"));
	zx->opt.hwName = optGetString("MACHINE","current");
	zx->opt.rsName = optGetString("ROMSET","current");
	zx->opt.GSRom = optGetString("ROMSET","gs");
	emulSetFlag(FL_RESET,optGetBool("MACHINE","restart"));
	setFlagBit(optGetBool("MACHINE","scrp.wait"),&zx->hwFlags,WAIT_ON);

	sndCalibrate();
//	zx->ide->refresh();
	ideSetPassport(zx->ide,IDE_MASTER,masterPass);
	ideSetPassport(zx->ide,IDE_SLAVE,slavePass);
	setHardware(zx, zx->opt.hwName);
	setRomset(zx, zx->opt.rsName);
	if (zx->hw==NULL) throw("Can't found current machine");
	if (memGetRomset(zx->mem) == NULL) throw("Can't found current romset");
	if ((zx->hw->mask != 0) && (~zx->hw->mask & tmask)) throw("Incorrect memory size for this machine");
	if (!vidSetLayout(zx->vid,zx->vid->curlay)) vidSetLayout(zx->vid,"default");
}
