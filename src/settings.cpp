#include "sound.h"
#include "emulwin.h"
#include "settings.h"
#include <QtCore>
#include <sys/stat.h>
#include <sys/types.h>

extern ZXComp* zx;
extern EmulWin* mwin;
extern Sound* snd;

void shithappens(std::string);
std::string int2str(int);
bool str2bool(std::string);
void splitline(std::string,std::string*,std::string*);
std::vector<std::string> splitstr(std::string,const char*);
void setFlagBit(bool,int32_t*,int32_t);

Settings::Settings() {
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

	opt.workDir = std::string(getenv("HOME")) + "/.config/samstyle/xpeccy";
	opt.romDir = opt.workDir + "/roms";
	opt.profPath = opt.workDir + "/config.conf";
	mkdir(opt.workDir.c_str(),0777);
	mkdir(opt.romDir.c_str(),0777);
#else
	opt.workDir = std::string(".\\config");
	opt.romDir = opt.workDir + "\\roms";
	opt.profPath = opt.workDir + "\\config.conf";
	mkdir(opt.workDir.c_str());
	mkdir(opt.romDir.c_str());
#endif
}

void Settings::addProfile(std::string nm, std::string fn) {
	Profile np;
	np.name = nm;
	np.file = fn;
	np.zx = new ZXComp;
	profs.push_back(np);
}

bool Settings::setProfile(std::string nm) {
	cprof = NULL;
	for(uint32_t i=0; i<profs.size(); i++) {
		if (profs[i].name == nm) {
			cprof = &profs[i];
			break;
		}
	}
	if (cprof == NULL) return false;
	zx = cprof->zx;
	return true;
}

void Settings::saveProfiles() {
	std::string cfname = opt.workDir + "/config.conf";
	std::ofstream cfile(cfname.c_str());
	if (!cfile.good()) {
		shithappens("Can't write main config");
		throw(0);
	}
	uint32_t i;
	cfile << "[BOOKMARKS]\n\n";
	for (i=0; i<umenu.data.size(); i++) {
		cfile << umenu.data[i].name.c_str() << " = " << umenu.data[i].path.c_str() << "\n";
	}
	cfile << "\n[PROFILES]\n\n";
	for (i=1; i<profs.size(); i++) {			// nr.0 skipped ('default' profile)
		cfile << profs[i].name.c_str() << " = " << profs[i].file.c_str() << "\n";
	}
	cfile << "current = " << cprof->name.c_str() << "\n";
	cfile.close();
}

void Settings::save() {
	saveProfiles();
	std::string cfname = opt.workDir + "/" + cprof->file;
	std::ofstream sfile(cfname.c_str());
	if (!sfile.good()) {
		shithappens("Can't write settings");
		throw(0);
	}
	uint32_t i,j;
	sfile<<"[CPU]\n\n";
	sfile<<"# real cpu freq in MHz = this value / 2; correct range is 2 to 14 (1 to 7 MHz)\n";
	sfile<<"cpu.frq = "<<int2str((int)(zx->sys->cpu->frq * 2.0)).c_str()<<"\n";
	
	sfile<<"\n[VIDEO]\n\n";
	sfile<<"doublesize = "<<((zx->vid->flags & VF_DOUBLE)?"y":"n")<<"\n";
	sfile<<"fullscreen = "<<((zx->vid->flags & VF_FULLSCREEN)?"y":"n")<<"\n";
	sfile<<"bordersize = "<<int2str((int)(zx->vid->brdsize * 100)).c_str()<<"\n";
	for (i=1; i < zx->vid->layout.size(); i++) {
		sfile << "layout = ";
		sfile << zx->vid->layout[i].name.c_str() << ":";
		sfile << int2str(zx->vid->layout[i].full.h) << ":" << int2str(zx->vid->layout[i].full.v) << ":";
		sfile << int2str(zx->vid->layout[i].bord.h) << ":" << int2str(zx->vid->layout[i].bord.v) << ":";
		sfile << int2str(zx->vid->layout[i].sync.h) << ":" << int2str(zx->vid->layout[i].sync.v) << ":";
		sfile << int2str(zx->vid->layout[i].intsz) << ":" << int2str(zx->vid->layout[i].intpos) << "\n";
	}
	sfile<<"geometry = "<<zx->vid->curlay.c_str()<<"\n";

	sfile << "\n[SCREENSHOTS]\n\n";
	sfile << "folder = " << opt.scrshotDir.c_str() << "\n";
	sfile << "format = " << opt.scrshotFormat.c_str() << "\n";
	sfile << "combo.count = " << int2str(sscnt).c_str() << "\n";
	sfile << "combo.interval = " << int2str(ssint).c_str() << "\n";

	sfile << "\n[SOUND]\n\n";
	sfile << "enabled = " << (snd->enabled ? "y" : "n") << "\n";
	sfile<<"# possible sound systems are:";
	for (i=0;i<snd->outsyslist.size();i++) {
		if (i!=0) sfile<<", ";
		sfile<<snd->outsyslist[i].name.c_str();
	}
	sfile<<"\n";
	sfile<<"soundsys = "<<snd->outsys->name.c_str()<<"\n";
	sfile<<"dontmute = "<<(snd->mute?"y":"n")<<"\n";
	sfile<<"rate = "<<int2str(snd->rate).c_str()<<"\n";
	sfile<<"volume.beep = "<<int2str(snd->beepvol).c_str()<<"\n";
	sfile<<"volume.tape = "<<int2str(snd->tapevol).c_str()<<"\n";
	sfile<<"volume.ay = "<<int2str(snd->ayvol).c_str()<<"\n";
	sfile<<"volume.gs = "<<int2str(snd->gsvol).c_str()<<"\n";
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
	sfile<<"restart = "<<(mwin->flags & FL_RESET?"y":"n")<<"\n";
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
	sfile << "sjasm = "		<< opt.asmPath.c_str()	<< "\n";
	sfile << "projectsdir = "	<< opt.projectsDir.c_str()<< "\n";

//	sfile<<"\n[MENU]\n\n";
//	for (i=0; i<umenu.data.size(); i++) {
//		sfile<<umenu.data[i].name.c_str()<<" = "<<umenu.data[i].path.c_str()<<"\n";
//	}
}

void Settings::loadProfiles() {
	std::ifstream file(opt.profPath.c_str());
	if (!file.good()) {
		shithappens("<b>Can't open main config</b><br>It seems, update needed<br>I will made it, don't worry");
		load(false);
		save();
		file.open(opt.profPath.c_str());
		if (!file.good()) {
			shithappens("<b>Doh! Something going wrong</b><br>Now you can worry");
			throw(0);
		}
//		shithappens("Done");
	}
	while (profs.size() > 1) profs.pop_back();		// delete all existing profiles except 'default'
	umenu.data.clear();
	char* buf = new char[0x4000];
	std::string line,pnam,pval;
	std::string pnm = "default";
	int section = 0;
	Profile prf;
	while (!file.eof()) {
		file.getline(buf,2048);
		line = std::string(buf);
		splitline(line,&pnam,&pval);
		if (pval=="") {
			if (pnam=="[BOOKMARKS]") section=1;
			if (pnam=="[PROFILES]") section=2;
		} else {
			switch (section) {
				case 1:
					umenu.add(pnam,pval);
					break;
				case 2:
					if (pnam == "current") {
						pnm = pval;
					} else {
						prf.name = pnam;
						prf.file = pval;
						prf.zx = new ZXComp;
						profs.push_back(prf);
					}
					break;
			}
		}
	}
	if (!setProfile(pnm)) {
		shithappens("Cannot set current profile\nCheck it's name");
		throw(0);
	}
}

void Settings::load(bool dev) {
	std::string cfname = opt.workDir + "/" + cprof->file;
	std::ifstream file(cfname.c_str());
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char* buf = new char[0x4000];
	int tmask = 0xff;
	if (!dev) zx->sys->mem->mask = 0;
	if (!file.good()) {
		shithappens(std::string("Can't find config file<br><b>") + cfname + std::string("</b><br>Default one will be created."));
		std::ofstream ofile(cfname.c_str());
		ofile << "[MACHINE]\n\ncurrent = ZX48K\nmemory = 48\n\n";
		ofile << "[BETADISK]\n\nenabled = n\n\n";
		ofile << "[ROMSETS]\n\nname = ZX48\nbasic48 = 1982.rom\n\ncurrent = ZX48\nreset = basic48\n";
		ofile.close();
		QFile fle(":/rom/1982.rom");
		fle.copy(QString(std::string(opt.romDir + "/1982.rom").c_str()));
		file.open(cfname.c_str(),std::ifstream::in);
	}
	if (!file.good()) {
		shithappens("Damn! I can't open config file<br>Zetsuboushita!");
		throw(0);
	} else {
		RomSet newrs;
		VidLayout vlay;
		int tmp2=0;
		int test;
		std::string fnam,tms;
		int fprt;
		zx->sys->mem->rsetlist.clear();
		while (!file.eof()) {
			file.getline(buf,2048);
			line = std::string(buf);
			splitline(line,&pnam,&pval);
			if (pval=="") {
				if (pnam=="[ROMSETS]") tmp2=1;
				if (pnam=="[VIDEO]") tmp2=2;
				if (pnam=="[SCREENSHOTS]") tmp2=3;
				if (pnam=="[SOUND]") tmp2=4;
				if (pnam=="[BETADISK]") tmp2=5;
				if (pnam=="[MACHINE]") tmp2=6;
				if (pnam=="[TOOLS]") tmp2=7;
				if (pnam=="[MENU]") tmp2=8;
				if (pnam=="[IDE]") tmp2=9;
				if (pnam=="[GENERAL]") tmp2=10;
				if (dev && (tmp2 != 7)) tmp2 = 0;
			} else {
				switch (tmp2) {
					case 1:
						
						pos = pval.find_last_of(":");
						if (pos != std::string::npos) {
							fnam = std::string(pval,0,pos);
							tms = std::string(pval,pos+1);
							if (tms=="") {fprt = 0;} else {fprt = atoi(tms.c_str());}
						} else {fnam = pval; fprt = 0;}
						if (pnam=="name") {
							newrs.name = pval;
							zx->sys->mem->rsetlist.push_back(newrs);
						}
						if ((pnam=="basic128") || (pnam=="0")) {
							zx->sys->mem->rsetlist.back().roms[0].path=fnam;
							zx->sys->mem->rsetlist.back().roms[0].part=fprt;}
						if ((pnam=="basic48") || (pnam=="1")) {
							zx->sys->mem->rsetlist.back().roms[1].path=fnam;
							zx->sys->mem->rsetlist.back().roms[1].part=fprt;}
						if ((pnam=="shadow") || (pnam=="2")) {
							zx->sys->mem->rsetlist.back().roms[2].path=fnam;
							zx->sys->mem->rsetlist.back().roms[2].part=fprt;}
						if ((pnam=="trdos") || (pnam=="3")) {
							zx->sys->mem->rsetlist.back().roms[3].path=fnam;
							zx->sys->mem->rsetlist.back().roms[3].part=fprt;}
						if (pnam=="reset") {
							if ((pval=="basic128") || (pval=="0")) zx->sys->mem->res = 0;
							if ((pval=="basic48") || (pval=="1")) zx->sys->mem->res = 1;
							if ((pval=="shadow") || (pval=="2")) zx->sys->mem->res = 2;
							if ((pval=="trdos") || (pval=="3")) zx->sys->mem->res = 3;
						}
						if (pnam=="current") zx->opt.romsetName = pval;
						if (pnam=="gs") zx->opt.GSRom = pval;
						break;
					case 2: if (pnam=="doublesize") {
							if (str2bool(pval))
								zx->vid->flags |= VF_DOUBLE;
							else
								zx->vid->flags &= ~VF_DOUBLE;
						}
						if (pnam=="fullscreen") {
							if (str2bool(pval))
								zx->vid->flags |= VF_FULLSCREEN;
							else
								zx->vid->flags &= ~VF_FULLSCREEN;
						}
						if (pnam=="bordersize") {test = atoi(pval.c_str()); if ((test>-1) && (test<101)) zx->vid->brdsize = test/100.0;}
						if (pnam=="layout") {
							vect = splitstr(pval,":");
//for(uint i=0;i<vect.size();i++) printf("%s\t",vect[i].c_str());
//printf("\n");
							if (vect.size() == 9) {
//printf("oga 8\n");
								vlay.name = vect[0];
								vlay.full.h = atoi(vect[1].c_str()); vlay.full.v = atoi(vect[2].c_str());
								vlay.bord.h = atoi(vect[3].c_str()); vlay.bord.v = atoi(vect[4].c_str());
								vlay.sync.h = atoi(vect[5].c_str()); vlay.sync.v = atoi(vect[6].c_str());
								vlay.intsz = atoi(vect[7].c_str()); vlay.intpos = atoi(vect[8].c_str());
//printf("%s\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n",vlay.name.c_str(),vlay.full.h,vlay.full.v,vlay.bord.h,vlay.bord.v,vlay.sync.h,vlay.sync.v,vlay.intsz);
								if ((vlay.full.h > vlay.bord.h + 256) && (vlay.bord.h > vlay.sync.h) && (vlay.full.v > vlay.bord.v + 192) && (vlay.bord.v > vlay.sync.v)) {
									zx->vid->layout.push_back(vlay);
								}
							}
						}
						if (pnam=="geometry") zx->vid->curlay = pval;
						break;
					case 3: if (pnam=="folder") opt.scrshotDir = pval;
						if (pnam=="format") opt.scrshotFormat = pval;
						if (pnam=="combo.count") sscnt=atoi(pval.c_str());
						if (pnam=="combo.interval") ssint=atoi(pval.c_str());
						break;
					case 4: if (pnam=="enabled") snd->enabled=str2bool(pval);
						if (pnam=="dontmute") snd->mute=str2bool(pval);
						if (pnam=="soundsys") opt.sndOutputName = pval;
						if (pnam=="rate") snd->rate = atoi(pval.c_str());
						if (pnam=="volume.beep") {snd->beepvol=atoi(pval.c_str()); if (snd->beepvol > 16) snd->beepvol = 16;}
						if (pnam=="volume.tape") {snd->tapevol = atoi(pval.c_str()); if (snd->tapevol > 16) snd->tapevol = 16;}
						if (pnam=="volume.ay") {snd->ayvol = atoi(pval.c_str()); if (snd->ayvol > 16) snd->ayvol = 16;}
						if (pnam=="volume.gs") {snd->gsvol = atoi(pval.c_str()); if (snd->gsvol > 16) snd->gsvol = 16;}
						if (pnam=="chip1") {test = atoi(pval.c_str()); if (test < SND_END) zx->aym->sc1->settype(test);}
						if (pnam=="chip2") {test = atoi(pval.c_str()); if (test < SND_END) zx->aym->sc2->settype(test);}
						if (pnam=="chip1.stereo") zx->aym->sc1->stereo = atoi(pval.c_str());
						if (pnam=="chip2.stereo") zx->aym->sc2->stereo = atoi(pval.c_str());
						if (pnam=="ts.type") zx->aym->tstype = atoi(pval.c_str());
						if (pnam=="gs") {
							if (str2bool(pval)) zx->gs->flags |= GS_ENABLE; else zx->gs->flags &= ~GS_ENABLE;
						}
						if (pnam=="gs.reset") {
							if (str2bool(pval)) zx->gs->flags |= GS_RESET; else zx->gs->flags &= ~GS_RESET;
						}
						if (pnam=="gs.stereo") zx->gs->stereo = atoi(pval.c_str());
						break;
					case 5: if (pnam=="enabled") zx->bdi->enable=str2bool(pval);
						if (pnam=="fast") zx->bdi->vg93.turbo=str2bool(pval);
						break;
					case 6: if (pnam=="current") zx->opt.hwName = pval;
						if (pnam=="memory") {
							if (pval=="48") {zx->sys->mem->mask = 0x00; tmask = 0;}
							if (pval=="128") {zx->sys->mem->mask = 0x07; tmask = 1;}
							if (pval=="256") {zx->sys->mem->mask = 0x0f; tmask = 2;}
							if (pval=="512") {zx->sys->mem->mask = 0x1f; tmask = 4;}
							if (pval=="1024") {zx->sys->mem->mask = 0x3f; tmask = 8;}
						}
						if (pnam=="restart") {
							if (str2bool(pval)) mwin->flags |= FL_RESET; else mwin->flags &= ~FL_RESET;
						}
						if (pnam=="scrp.wait") {
							if (str2bool(pval)) zx->sys->hwflags |= WAIT_ON; else zx->sys->hwflags &= ~WAIT_ON;
						}
						break;
					case 7: if (pnam=="sjasm") opt.asmPath = pval;
						if (pnam=="projectsdir") opt.projectsDir = pval;
						break;
					case 8: umenu.add(pnam,pval);
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
						if (pnam=="cpu.frq") {
							fprt = atoi(pval.c_str());
							if ((fprt > 0) && (fprt < 14)) zx->sys->cpu->frq = fprt / 2.0;
						}
						break;
				}
			}
		}
	}
	if (dev) return;
	snd->defpars();
	zx->ide->refresh();
	zx->setHardware(zx->opt.hwName);
	zx->sys->mem->setromptr(zx->opt.romsetName);
	snd->setoutptr(opt.sndOutputName);
	if (zx->hw==NULL) throw("Can't found current machine");
	if (zx->sys->mem->romset==NULL) throw("Can't found current romset");
	if (~zx->hw->mask & tmask) throw("Incorrect memory size for this machine");
	if (!zx->vid->setlayout(zx->vid->curlay)) zx->vid->setlayout("default");
//	mwin->updateWin();
//	zx->vid->update();
	zx->sys->mem->loadromset(opt.romDir);
//	mwin->makeBookmarkMenu();
}
