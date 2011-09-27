#include <sys/stat.h>
#include <sys/types.h>

#include <QtCore>

#include "common.h"
#include "sound.h"
#include "emulwin.h"
#include "settings.h"

extern ZXComp* zx;
std::vector<optEntry> config;
std::string workDir;
std::string romDir;
std::string profPath;

std::string shotExt;
std::string shotDir;
int shotCount;
int shotInterval;

std::string projDir;
std::string asmPath;

std::string rmnam[] = {"basic128","basic48","shadow","trdos","ext4","ext5","ext6","ext7"};

// new
// static vars base
std::string optGetString(int wut) {
	std::string res;
	switch (wut) {
		case OPT_WORKDIR: res = workDir; break;
		case OPT_ROMDIR: res = romDir; break;
		case OPT_SHOTDIR: res = shotDir; break;
		case OPT_SHOTEXT: res = shotExt; break;
		case OPT_PROJDIR: res = projDir; break;
		case OPT_ASMPATH: res = asmPath; break;
	}
	return res;
}

int optGetInt(int wut) {
	int res = 0;
	switch (wut) {
		case OPT_SHOTINT: res = shotInterval; break;
		case OPT_SHOTCNT: res = shotCount; break;
	}
	return res;
}

void optSet(int wut, std::string val) {
	switch(wut) {
		case OPT_SHOTDIR: shotDir = val; break;
		case OPT_PROJDIR: projDir = val; break;
		case OPT_ASMPATH: asmPath = val; break;
	}
}

void optSet(int wut, int val) {
	switch (wut) {
		case OPT_SHOTINT: shotInterval = val; break;
		case OPT_SHOTCNT: shotCount = val; break;
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
// set new paths
//	ssdir = std::string(getenv("HOME"));
//	std::string mydir = ssdir + "/.config/samstyle";

	workDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy";
	romDir = workDir + "/roms";
	profPath = workDir + "/config.conf";
	mkdir(workDir.c_str(),0777);
	mkdir(romDir.c_str(),0777);
#else
	workDir = std::string(".\\config");
	romDir = workDir + "\\roms";
	profPath = workDir + "\\config.conf";
	mkdir(workDir.c_str());
	mkdir(romDir.c_str());
#endif
}

void saveProfiles() {
	std::string cfname = workDir + "/config.conf";
	std::ofstream cfile(cfname.c_str());
	if (!cfile.good()) {
		shithappens("Can't write main config");
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
	cfile << "scrFormat = " << shotExt.c_str() << "\n";
	cfile << "scrCount = " << int2str(shotCount) << "\n";
	cfile << "scrInterval = " << int2str(shotInterval) << "\n";
	cfile << "\n[ROMSETS]\n";
	std::vector<RomSet> rsl = getRomsetList();
	for (i=0; i<rsl.size(); i++) {
		cfile<< "\nname = " << rsl[i].name.c_str() << "\n";
		for (j=0; j<8; j++) {
			if (rsl[i].roms[j].path != "") {
				cfile << rmnam[j].c_str()<<" = " << rsl[i].roms[j].path.c_str();
				if (rsl[i].roms[j].part != 0) cfile << ":" << int2str(rsl[i].roms[j].part).c_str();
				cfile << "\n";
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
	cfile.close();
}

void saveConfig() {
	saveProfiles();
	std::string cfname = workDir + "/" + getCurrentProfile()->file;
	std::ofstream sfile(cfname.c_str());
	if (!sfile.good()) {
		shithappens("Can't write settings");
		throw(0);
	}
	optSet("MACHINE","current",zx->opt.hwName);
	optSet("ROMSET","current",zx->opt.rsName);
	optSet("MACHINE","restart",(emulGetFlags() & FL_RESET) != 0);
	optSet("ROMSET","reset",rmnam[zx->sys->mem->res]);
	switch(zx->sys->mem->mask) {
		case 0x00: optSet("MACHINE","memory",48); break;
		case 0x07: optSet("MACHINE","memory",128); break;
		case 0x0f: optSet("MACHINE","memory",256); break;
		case 0x1f: optSet("MACHINE","memory",512); break;
		case 0x3f: optSet("MACHINE","memory",1024); break;
	}
	optSet("GENERAL","cpu.frq",int(zx->sys->cpu->frq * 2));
	optSet("MACHINE","scrp.wait",(zx->sys->hwflags & WAIT_ON) != 0);
	optSet("VIDEO","doublesize",(zx->vid->flags & VF_DOUBLE) != 0);
	optSet("VIDEO","bordersize",int(zx->vid->brdsize * 100));
	optSet("VIDEO","geometry",zx->vid->curlay);
	optSet("SOUND","chip1",zx->aym->sc1->type);
	optSet("SOUND","chip2",zx->aym->sc2->type);
	optSet("SOUND","chip1.stereo",zx->aym->sc1->stereo);
	optSet("SOUND","chip2.stereo",zx->aym->sc2->stereo);
	optSet("SOUND","ts.type",zx->aym->tstype);
	optSet("SOUND","gs",(zx->gs->flags & GS_ENABLE) != 0);
	optSet("SOUND","gs.reset",(zx->gs->flags & GS_RESET) != 0);
	optSet("SOUND","gs.stereo",zx->gs->stereo);
	optSet("BETADISK","enabled",zx->bdi->enable);
	optSet("BETADISK","fast",zx->bdi->vg93.turbo);
	optSet("BETADISK","A",zx->bdi->flop[0].getString());
	optSet("BETADISK","B",zx->bdi->flop[1].getString());
	optSet("BETADISK","C",zx->bdi->flop[2].getString());
	optSet("BETADISK","D",zx->bdi->flop[3].getString());
	optSet("IDE","iface",zx->ide->iface);
	optSet("IDE","master.type",zx->ide->master.iface);
	optSet("IDE","master.model",zx->ide->master.pass.model);
	optSet("IDE","master.serial",zx->ide->master.pass.serial);
	optSet("IDE","master.image",zx->ide->master.image);
	optSet("IDE","master.lba",(zx->ide->master.flags & ATA_LBA) != 0);
	optSet("IDE","master.maxlba",(int)zx->ide->master.maxlba);
	optSet("IDE","master.chs",zx->ide->master.getCHS());
	optSet("IDE","slave.type",zx->ide->slave.iface);
	optSet("IDE","slave.model",zx->ide->slave.pass.model);
	optSet("IDE","slave.serial",zx->ide->slave.pass.serial);
	optSet("IDE","slave.image",zx->ide->slave.image);
	optSet("IDE","slave.lba",(zx->ide->slave.flags & ATA_LBA) != 0);
	optSet("IDE","slave.maxlba",(int)zx->ide->slave.maxlba);
	optSet("IDE","slave.chs",zx->ide->slave.getCHS());
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
/*
	uint32_t i,j;
	sfile<<"[GENERAL]\n\n";
	sfile<<"# real cpu freq in MHz is this value / 2; correct range is 2 to 14 (1 to 7 MHz)\n";
	sfile<<"cpu.frq = "<<int2str((int)(zx->sys->cpu->frq * 2.0)).c_str()<<"\n";

	sfile<<"\n[VIDEO]\n\n";
	sfile<<"doublesize = "<<((zx->vid->flags & VF_DOUBLE)?"y":"n")<<"\n";
	sfile<<"fullscreen = "<<((zx->vid->flags & VF_FULLSCREEN)?"y":"n")<<"\n";
	sfile<<"bordersize = "<<int2str((int)(zx->vid->brdsize * 100)).c_str()<<"\n";
	sfile<<"geometry = "<<zx->vid->curlay.c_str()<<"\n";

	sfile << "\n[SCREENSHOTS]\n\n";
	sfile << "folder = " << optGetString("SCREENSHOTS","folder").c_str() << "\n";
	sfile << "format = " << optGetString("SCREENSHOTS","format").c_str() << "\n";
	sfile << "combo.count = " << int2str(optGetInt("SCREENSHOTS","combo.count")).c_str() << "\n";
	sfile << "combo.interval = " << int2str(optGetInt("SCREENSHOTS","combo.interval")).c_str() << "\n";

	sfile << "\n[SOUND]\n\n";
	sfile << "enabled = " << ((sndGet(SND_ENABLE) != 0) ? "y" : "n") << "\n";
	sfile<<"# possible sound systems are:";
	std::vector<std::string> outputList = sndGetList();
	for (i=0; i<outputList.size(); i++) {
		if (i!=0) sfile<<", ";
		sfile << outputList[i].c_str();
	}
	sfile<<"\n";
	sfile<<"soundsys = " << sndGetName().c_str()<<"\n";
	sfile<<"dontmute = "<<((sndGet(SND_MUTE) != 0) ? "y" : "n")<<"\n";
	sfile<<"rate = "<<int2str(sndGet(SND_RATE)).c_str()<<"\n";
	sfile<<"volume.beep = "<<int2str(sndGet(SND_BEEP)).c_str()<<"\n";
	sfile<<"volume.tape = "<<int2str(sndGet(SND_TAPE)).c_str()<<"\n";
	sfile<<"volume.ay = "<<int2str(sndGet(SND_AYVL)).c_str()<<"\n";
	sfile<<"volume.gs = "<<int2str(sndGet(SND_GSVL)).c_str()<<"\n";
	sfile<<"chip1 = "<<int2str(zx->aym->sc1->type)<<"\n";
	sfile<<"chip2 = "<<int2str(zx->aym->sc2->type)<<"\n";
	sfile<<"chip1.stereo = "<<int2str(zx->aym->sc1->stereo)<<"\n";
	sfile<<"chip2.stereo = "<<int2str(zx->aym->sc2->stereo)<<"\n";
	sfile<<"ts.type = "<<int2str(zx->aym->tstype)<<"\n";
	sfile<<"gs = "<<((zx->gs->flags & GS_ENABLE)?"y":"n")<<"\n";
	sfile<<"gs.reset = "<<((zx->gs->flags & GS_RESET)?"y":"n")<<"\n";
	sfile<<"gs.stereo = "<<int2str(zx->gs->stereo)<<"\n";

	sfile<<"\n[BETADISK]\n\n";
	sfile<<"enabled = "<<(zx->bdi->enable?"y":"n")<<"\n";
	sfile<<"fast = "<<(zx->bdi->vg93.turbo?"y":"n")<<"\n";

	sfile<<"\n[IDE]\n\n";
	sfile<<"iface = "<<int2str(zx->ide->iface)<<"\n";
	sfile<<"master.type = "<<int2str(zx->ide->master.iface)<<"\n";
	sfile<<"master.model = "<<zx->ide->master.pass.model.c_str()<<"\n";
	sfile<<"master.serial = "<<zx->ide->master.pass.serial.c_str()<<"\n";
	sfile<<"master.image = "<<zx->ide->master.image.c_str()<<"\n";
	sfile<<"master.lba = "<<((zx->ide->master.flags & ATA_LBA) ? "y" : "n")<<"\n";
	sfile<<"master.maxlba = "<<int2str(zx->ide->master.maxlba)<<"\n";
	sfile<<"master.chs = "<<int2str(zx->ide->master.pass.spt)<<"/"<<int2str(zx->ide->master.pass.hds)<<"/"<<int2str(zx->ide->master.pass.cyls)<<"\n";
	sfile<<"slave.type = "<<int2str(zx->ide->slave.iface)<<"\n";
	sfile<<"slave.model = "<<zx->ide->slave.pass.model.c_str()<<"\n";
	sfile<<"slave.serial = "<<zx->ide->slave.pass.serial.c_str()<<"\n";
	sfile<<"slave.image = "<<zx->ide->slave.image.c_str()<<"\n";
	sfile<<"slave.lba = "<<((zx->ide->slave.flags & ATA_LBA) ? "y" : "n")<<"\n";
	sfile<<"slave.maxlba = "<<int2str(zx->ide->slave.maxlba)<<"\n";
	sfile<<"slave.chs = "<<int2str(zx->ide->slave.pass.spt)<<"/"<<int2str(zx->ide->slave.pass.hds)<<"/"<<int2str(zx->ide->slave.pass.cyls)<<"\n";

	sfile<<"\n[MACHINE]\n\n";
	sfile<<"# possible values:";
	for (i=0; i < zx->hwlist.size(); i++) {if (i!=0) sfile<<", "; sfile << zx->hwlist[i].name.c_str();}
	sfile<<"\n";
	sfile<<"current = "<<zx->hw->name.c_str()<<"\n";
	sfile<<"memory = ";
	switch (zx->sys->mem->mask) {
		case 0x07: sfile<<"128\n"; break;
		case 0x0f: sfile<<"256\n"; break;
		case 0x1f: sfile<<"512\n"; break;
		case 0x3f: sfile<<"1024\n"; break;
		default: sfile<<"48\n"; break;
	}
	sfile<<"restart = "<<(emulGetFlags() & FL_RESET?"y":"n")<<"\n";
	sfile << "scrp.wait = "<< ((zx->sys->hwflags & WAIT_ON) ? "y" : "n") << "\n";

	sfile<<"\n[ROMSETS]\n\n";
	std::vector<std::string> rmnam;
	rmnam.push_back("basic128"); rmnam.push_back("basic48"); rmnam.push_back("shadow"); rmnam.push_back("trdos");
	rmnam.push_back("ext4"); rmnam.push_back("ext5"); rmnam.push_back("ext6"); rmnam.push_back("ext7");
	for (i=0;i < zx->sys->mem->rsetlist.size();i++) {
		sfile<<"name = " << zx->sys->mem->rsetlist[i].name.c_str()<<"\n";
		for (j=0;j<8;j++) {
			if (zx->sys->mem->rsetlist[i].roms[j].path!="") {
				sfile<<rmnam[j].c_str()<<" = " << zx->sys->mem->rsetlist[i].roms[j].path.c_str();
				if (zx->sys->mem->rsetlist[i].roms[j].part!=0) sfile<<":"<<int2str(zx->sys->mem->rsetlist[i].roms[j].part).c_str();
				sfile<<"\n";
			}
		}
		sfile<<"\n";
	}
	sfile << "gs = "	<< zx->opt.GSRom.c_str()		<< "\n";
	sfile << "current = "	<< zx->sys->mem->romset->name.c_str()	<< "\n";
	sfile << "reset = "	<< rmnam[zx->sys->mem->res].c_str()	<< "\n";

	sfile << "\n[TOOLS]\n\n";
	sfile << "sjasm = "		<< optGetString("TOOLS","sjasm").c_str()	<< "\n";
	sfile << "projectsdir = "	<< optGetString("TOOLS","projectsdir").c_str()<< "\n";
*/
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
			shithappens("<b>Doh! Something going wrong</b>");
			throw(0);
		}
	}
	clearProfiles();
	clearBookmarks();
	char* buf = new char[0x4000];
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
	while (!file.eof()) {
		file.getline(buf,2048);
		line = std::string(buf);
		splitline(line,&pnam,&pval);
		if (pval=="") {
			if (pnam=="[BOOKMARKS]") section=1;
			if (pnam=="[PROFILES]") section=2;
			if (pnam=="[VIDEO]") section=3;
			if (pnam=="[ROMSETS]") section=4;
			if (pnam=="[SOUND]") section=5;
			if (pnam=="[TOOLS]") section=6;
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
					if (pnam=="scrFormat") shotExt = pval;
					if (pnam=="scrCount") shotCount = atoi(pval.c_str());
					if (pnam=="scrInterval") shotInterval = atoi(pval.c_str());
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
			}
		}
	}
	for (uint i=0; i<rslist.size(); i++) addRomset(rslist[i]);
	setOutput(soutnam);
	if (!setProfile(pnm.c_str())) {
		shithappens("Cannot set current profile\nCheck it's name");
		throw(0);
	}
}

void loadConfig(bool dev) {
	std::string cfname = workDir + "/" + getCurrentProfile()->file;
	std::ifstream file(cfname.c_str());
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char* buf = new char[0x4000];
	int tmask = 0xff;
	int tmp2=0;
	if (!dev) zx->sys->mem->mask = 0;
	if (!file.good()) {
//		shithappens(std::string("Can't find config file<br><b>") + cfname + std::string("</b><br>Default one will be created."));
		printf("Profile config is missing. Default one will be created\n");
		QFile fle(":/conf/xpeccy.conf");
		fle.copy(QString(cfname.c_str()));
		fle.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
		file.open(cfname.c_str(),std::ifstream::in);
	}
	if (!file.good()) {
		shithappens("Damn! I can't open config file<br>Zetsuboushita!");
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
			splitline(line,&pnam,&pval);
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
							if ((pval=="basic128") || (pval=="0")) zx->sys->mem->res = 0;
							if ((pval=="basic48") || (pval=="1")) zx->sys->mem->res = 1;
							if ((pval=="shadow") || (pval=="2")) zx->sys->mem->res = 2;
							if ((pval=="trdos") || (pval=="3")) zx->sys->mem->res = 3;
						}
//						if (pnam=="current") zx->opt.romsetName = pval;
//						if (pnam=="gs") zx->opt.GSRom = pval;
						break;
					case 2: 
						break;
					case 3:
						if (pnam=="folder") shotDir = pval;
						if (pnam=="format") shotExt = pval;
						if (pnam=="combo.count") shotCount = atoi(pval.c_str());
						if (pnam=="combo.interval") shotInterval = atoi(pval.c_str());
						break;
					case 4:
						break;
					case 5:
						if (pnam=="A") zx->bdi->flop[0].setString(pval);
						if (pnam=="B") zx->bdi->flop[1].setString(pval);
						if (pnam=="C") zx->bdi->flop[2].setString(pval);
						if (pnam=="D") zx->bdi->flop[3].setString(pval);
						break;
					case 6:
						if (pnam=="memory") {
							if (pval=="48") {zx->sys->mem->mask = 0x00; tmask = 0;}
							if (pval=="128") {zx->sys->mem->mask = 0x07; tmask = 1;}
							if (pval=="256") {zx->sys->mem->mask = 0x0f; tmask = 2;}
							if (pval=="512") {zx->sys->mem->mask = 0x1f; tmask = 4;}
							if (pval=="1024") {zx->sys->mem->mask = 0x3f; tmask = 8;}
						}
						break;
					case 7:
						if (pnam=="sjasm") asmPath = pval; break;
						if (pnam=="projectsdir") projDir = pval; break;
						break;
					case 8: addBookmark(pnam.c_str(),pval.c_str());
						break;
					case 9:
						if (pnam=="iface") zx->ide->iface = atoi(pval.c_str());
						if (pnam=="master.type") zx->ide->master.iface = atoi(pval.c_str());
						if (pnam=="master.model") zx->ide->master.pass.model = std::string(pval,0,40);
						if (pnam=="master.serial") zx->ide->master.pass.serial = std::string(pval,0,20);
						if (pnam=="master.lba") setFlagBit(str2bool(pval),&zx->ide->master.flags, ATA_LBA);
						if (pnam=="master.maxlba") zx->ide->master.maxlba = atoi(pval.c_str());
						if (pnam=="master.image") zx->ide->master.image = pval;
						if (pnam=="master.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								zx->ide->master.pass.spt = atoi(vect.at(0).c_str());
								zx->ide->master.pass.hds = atoi(vect.at(1).c_str());
								zx->ide->master.pass.cyls = atoi(vect.at(2).c_str());
							}
						}
						if (pnam=="slave.type") zx->ide->slave.iface = atoi(pval.c_str());
						if (pnam=="slave.model") zx->ide->slave.pass.model = std::string(pval,0,40);
						if (pnam=="slave.serial") zx->ide->slave.pass.serial = std::string(pval,0,20);
						if (pnam=="slave.lba") setFlagBit(str2bool(pval),&zx->ide->slave.flags, ATA_LBA);
						if (pnam=="slave.maxlba") zx->ide->slave.maxlba = atoi(pval.c_str());
						if (pnam=="slave.image") zx->ide->slave.image = pval;
						if (pnam=="slave.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								zx->ide->slave.pass.spt = atoi(vect.at(0).c_str());
								zx->ide->slave.pass.hds = atoi(vect.at(1).c_str());
								zx->ide->slave.pass.cyls = atoi(vect.at(2).c_str());
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

	tmp2 = optGetInt("GENERAL","cpu.frq"); if ((tmp2 > 0) && (tmp2 <= 14)) zx->sys->cpu->frq = tmp2 / 2.0;

	zx->vid->curlay = optGetString("VIDEO","geometry");
	setFlagBit(optGetBool("VIDEO","doublesize"),&zx->vid->flags, VF_DOUBLE);
	setFlagBit(optGetBool("VIDEO","fullscreen"),&zx->vid->flags, VF_FULLSCREEN);
	tmp2 = optGetInt("VIDEO","bordersize"); if ((tmp2 >= 0) && (tmp2 <= 100)) zx->vid->brdsize = tmp2 / 100.0;
	
	tmp2 = optGetInt("SOUND","chip1"); if (tmp2 < SND_END) zx->aym->sc1->settype(tmp2);
	tmp2 = optGetInt("SOUND","chip2"); if (tmp2 < SND_END) zx->aym->sc2->settype(tmp2);
	setFlagBit(optGetBool("SOUND","gs"),&zx->gs->flags,GS_ENABLE);
	setFlagBit(optGetBool("SOUND","gs.reset"),&zx->gs->flags,GS_RESET);
	zx->aym->sc1->stereo = optGetInt("SOUND","chip1.stereo");
	zx->aym->sc2->stereo = optGetInt("SOUND","chip2.stereo");
	zx->aym->tstype = optGetInt("SOUND","ts.type");
	zx->gs->stereo = optGetInt("SOUND","gs.stereo");
	
	zx->bdi->enable = optGetBool("BETADISK","enabled");
	zx->bdi->vg93.turbo = optGetBool("BETADISK","fast");
	zx->opt.hwName = optGetString("MACHINE","current");
	zx->opt.rsName = optGetString("ROMSET","current");
	zx->opt.GSRom = optGetString("ROMSET","gs");
	emulSetFlag(FL_RESET,optGetBool("MACHINE","restart"));
	setFlagBit(optGetBool("MACHINE","scrp.wait"),&zx->sys->hwflags,WAIT_ON);
	
	sndCalibrate();
	zx->ide->refresh();
	setHardware(zx, optGetString("MACHINE","current"));
	setRomset(zx, optGetString("ROMSET","current"));
	if (zx->hw==NULL) throw("Can't found current machine");
	if (zx->sys->mem->romset==NULL) throw("Can't found current romset");
	if (~zx->hw->mask & tmask) throw("Incorrect memory size for this machine");
	if (!zx->vid->setLayout(zx->vid->curlay)) zx->vid->setLayout("default");
	zx->sys->mem->loadromset(romDir);
}
