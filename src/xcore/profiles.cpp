#include "xcore.h"
#include "../settings.h"
#include "../filer.h"
#include <stdio.h>
#include <fstream>

XProfile* currentProfile = NULL;
std::vector<XProfile> profileList;

int findProfile(std::string nm) {
	int idx = 0;
	while (idx < (int)profileList.size()) {
		if (profileList[idx].name == nm) return idx;
		idx++;
	}
	return -1;
}

XProfile* getProfile(std::string nm) {
	int idx = findProfile(nm);
	if (idx < 0) return NULL;
	return &profileList[idx];
}

bool addProfile(std::string nm, std::string fp) {
	printf("add Profile: %s : %s\n",nm.c_str(),fp.c_str());
	int idx = findProfile(nm);
	if (idx > -1) return false;
	XProfile nprof;
	nprof.name = nm;
	nprof.file = fp;
	nprof.layName = std::string("default");
	nprof.zx = zxCreate();
	std::string cmosFile = optGetString(OPT_WORKDIR) + std::string(SLASH) + nprof.name + std::string(".cmos");
	std::ifstream file(cmosFile.c_str());
	if (file.good()) file.read((char*)nprof.zx->cmos.data,256);
	zxSetHardware(nprof.zx,"ZX48K");
	if (currentProfile != NULL) {
		nm = currentProfile->name;
		profileList.push_back(nprof);		// PUSH_BACK reallocate profileList and breaks current profile pointer!
		setProfile(nm);				// then it must be setted again
	} else {
		profileList.push_back(nprof);
	}
//	prfLoad(nprof.name);
	return true;
}

int delProfile(std::string nm) {
	if (nm == "default") return DELP_ERR;			// can't touch this
	int idx = findProfile(nm);
	if (idx < 0) return DELP_ERR;				// no such profile
	int res = DELP_OK;
	zxDestroy(profileList[idx].zx);
	if (currentProfile->name == nm) {
		setProfile("default");	// if current profile deleted, set default
		res = DELP_OK_CURR;
	}
	if (currentProfile != NULL) {
		nm = currentProfile->name;
		profileList.erase(profileList.begin() + idx);
		setProfile(nm);
	} else {
		profileList.erase(profileList.begin() + idx);
	}
	return res;
}

bool setProfile(std::string nm) {
	int idx = findProfile(nm);
	if (idx < 0) return false;
	currentProfile = &profileList[idx];
	printf("set profile %s\n",currentProfile->name.c_str());
	zx = currentProfile->zx;
	vidUpdate(zx->vid);
	zx->flag |= ZX_PALCHAN;
	// emulSetPalette(zx,0);
	return true;
}

void clearProfiles() {
	XProfile defprof = profileList[0];
	profileList.clear();
	profileList.push_back(defprof);
	setProfile(profileList[0].name);
}

std::vector<XProfile> getProfileList() {
	return profileList;
}

XProfile* getCurrentProfile() {
	return currentProfile;
}

// load-save

#define	PS_NONE		0
#define	PS_MACHINE	1
#define	PS_ROMSET	2
#define	PS_VIDEO	3
#define	PS_SOUND	4
#define	PS_INPUT	5
#define	PS_TAPE		6
#define	PS_DISK		7
#define	PS_IDE		8
#define	PS_SDC		9

void setDiskString(ZXComp* comp,Floppy* flp,std::string st) {
	if (st.size() < 4) return;
	flp->flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (st.substr(0,2) == "80") flp->flag |= FLP_TRK80;
	if (st.substr(2,1) == "D") flp->flag |= FLP_DS;
	if (st.substr(3,1) == "R") flp->flag |= FLP_PROTECT;
	if (flp->path || (st.size() < 5) || !optGetFlag(OF_PATHS)) return;
	st = st.substr(5);
	loadFile(comp,st.c_str(),FT_DISK,flp->id);
}

void prfSetRomset(std::string pnm, std::string rnm) {
	XProfile* prf = (pnm == "") ? currentProfile : getProfile(pnm);
	if (prf == NULL) return;
	prf->rsName = rnm;
	rsSetRomset(prf->zx,rnm);
}

void prfLoadAll() {
	for (uint i = 0; i < profileList.size(); i++) prfLoad(profileList[i].name);
}

int prfLoad(std::string nm) {
	XProfile* prf = (nm == "") ? currentProfile : getProfile(nm);
	if (prf == NULL) return PLOAD_NF;
	printf("%s\n",prf->name.c_str());
	ZXComp* comp = prf->zx;

	std::string cfname = optGetString(OPT_WORKDIR) + SLASH + prf->file;
	printf("load config %s\n",cfname.c_str());
	std::ifstream file(cfname.c_str());
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char* buf = new char[0x4000];
	int tmask = 0xff;
	int tmp2;
	int section = PS_NONE;
	int memsz = 48;
	ATAPassport masterPass = ideGetPassport(comp->ide,IDE_MASTER);
	ATAPassport slavePass = ideGetPassport(comp->ide,IDE_SLAVE);
	if (!file.good()) {
		printf("Profile config is missing. Default one will be created\n");
		copyFile(":/conf/xpeccy.conf",cfname.c_str());
		file.open(cfname.c_str(),std::ifstream::in);
	}
	if (!file.good()) {
		printf("Damn! I can't open config file");
		return PLOAD_OF;
	}

	while (!file.eof()) {
		file.getline(buf,2048);
		line = std::string(buf);
		pos = line.find_first_of("#"); if (pos != std::string::npos) line.erase(pos);
		pos = line.find_first_of(";"); if (pos != std::string::npos) line.erase(pos);
		spl = splitline(line);
		pnam = spl.first;
		pval = spl.second;
		if (pval=="") {
			if (pnam=="[MACHINE]") section = PS_MACHINE;
			if (pnam=="[GENERAL]") section = PS_MACHINE;
			if (pnam=="[ROMSET]") section = PS_ROMSET;
			if (pnam=="[VIDEO]") section = PS_VIDEO;
			if (pnam=="[SOUND]") section = PS_SOUND;
			if (pnam=="[TAPE]") section = PS_TAPE;
			if (pnam=="[DISK]") section = PS_DISK;
			if (pnam=="[IDE]") section = PS_IDE;
			if (pnam=="[INPUT]") section = PS_INPUT;
			if (pnam=="[SDC]") section = PS_SDC;
		} else {
			switch (section) {
				case PS_ROMSET:
					if (pnam=="reset") {
						comp->resbank = 1;
						if ((pval == "basic128") || (pval=="0")) comp->resbank = 0;
						if ((pval == "basic48") || (pval=="1")) comp->resbank = 1;
						if ((pval == "shadow") || (pval=="2")) comp->resbank = 2;
						if ((pval == "trdos") || (pval=="3")) comp->resbank = 3;
					}
					if (pnam=="current") prf->rsName = pval;
					break;
				case PS_VIDEO:
					if (pnam == "geometry") prf->layName = pval;
					if (pnam == "4t-border") setFlagBit(str2bool(pval),&comp->vid->flags,VID_BORDER_4T);
					break;
				case PS_SOUND:
					if (pnam == "chip1") aymSetType(comp->ts->chipA,atoi(pval.c_str()));
					if (pnam == "chip1.stereo") comp->ts->chipA->stereo = atoi(pval.c_str());
					if (pnam == "chip2") aymSetType(comp->ts->chipB,atoi(pval.c_str()));
					if (pnam == "chip2.stereo") comp->ts->chipB->stereo = atoi(pval.c_str());
					if (pnam == "ts.type") comp->ts->type = atoi(pval.c_str());
					if (pnam == "gs") setFlagBit(str2bool(pval),&comp->gs->flag,GS_ENABLE);
					if (pnam == "gs.reset") setFlagBit(str2bool(pval),&comp->gs->flag,GS_RESET);
					if (pnam == "gs.stereo") comp->gs->stereo = atoi(pval.c_str());
					if (pnam == "soundrive_type") comp->sdrv->type = atoi(pval.c_str());
					break;
				case PS_TAPE:
					if (pnam == "path" && optGetFlag(OF_PATHS)) loadFile(comp,pval.c_str(),FT_TAPE,0);
					break;
				case PS_DISK:
					if (pnam == "A") setDiskString(comp,comp->bdi->fdc->flop[0],pval);
					if (pnam == "B") setDiskString(comp,comp->bdi->fdc->flop[1],pval);
					if (pnam == "C") setDiskString(comp,comp->bdi->fdc->flop[2],pval);
					if (pnam == "D") setDiskString(comp,comp->bdi->fdc->flop[3],pval);
					if (pnam == "type") comp->bdi->fdc->type = atoi(pval.c_str());
					if (pnam == "fast") comp->bdi->fdc->turbo = str2bool(pval);
					break;
				case PS_MACHINE:
					if (pnam == "current") prf->hwName = pval;
					if (pnam == "cpu.frq") {
						tmp2 = atoi(pval.c_str());
						if ((tmp2 > 0) && (tmp2 <= 28)) zxSetFrq(comp,tmp2 / 2.0);
					}
					if (pnam == "memory") {
						memsz = atoi(pval.c_str());
						switch (memsz) {
							case 128: tmask = MEM_128; break;
							case 256: tmask = MEM_256; break;
							case 512: tmask = MEM_512; break;
							case 1024: tmask = MEM_1M; break;
							case 2048: tmask = MEM_2M; break;
							case 4096: tmask = MEM_4M; break;
						}
					}
					if (pnam == "contmem") setFlagBit(str2bool(pval),&comp->hwFlag,HW_CONTMEM);
					if (pnam == "contio") setFlagBit(str2bool(pval),&comp->hwFlag,HW_CONTIO);
					if (pnam == "scrp.wait") setFlagBit(str2bool(pval),&comp->hwFlag,HW_WAIT);
					break;
				case PS_IDE:
					if (pnam == "iface") comp->ide->type = atoi(pval.c_str());
					if (pnam == "master.type") comp->ide->master->type = atoi(pval.c_str());
//					if (pnam == "master.model") memcpy(masterPass.model,std::string(pval,0,40).c_str(),40);
//					if (pnam == "master.serial") memcpy(masterPass.serial,std::string(pval,0,20).c_str(),20);
					if (pnam == "master.lba") setFlagBit(str2bool(pval),&comp->ide->master->flags, ATA_LBA);
					if (pnam == "master.maxlba") comp->ide->master->maxlba = atoi(pval.c_str());
					if (pnam == "master.image") ideSetImage(comp->ide,IDE_MASTER,pval.c_str());
					if (pnam == "master.chs") {
						vect = splitstr(pval,"/");
						if (vect.size() > 2) {
							masterPass.spt = atoi(vect.at(0).c_str());
							masterPass.hds = atoi(vect.at(1).c_str());
							masterPass.cyls = atoi(vect.at(2).c_str());
						}
					}
					if (pnam == "slave.type") comp->ide->slave->type = atoi(pval.c_str());
//					if (pnam == "slave.model") memcpy(slavePass.model,std::string(pval,0,40).c_str(),40);
//					if (pnam == "slave.serial") memcpy(slavePass.serial,std::string(pval,0,20).c_str(),20);
					if (pnam == "slave.lba") setFlagBit(str2bool(pval),&comp->ide->slave->flags, ATA_LBA);
					if (pnam == "slave.maxlba") comp->ide->slave->maxlba = atoi(pval.c_str());
					if (pnam == "slave.image") ideSetImage(comp->ide,IDE_SLAVE,pval.c_str());
					if (pnam == "slave.chs") {
						vect = splitstr(pval,"/");
						if (vect.size() > 2) {
							slavePass.spt = atoi(vect.at(0).c_str());
							slavePass.hds = atoi(vect.at(1).c_str());
							slavePass.cyls = atoi(vect.at(2).c_str());
						}
					}
					break;
				case PS_INPUT:
					if (pnam == "mouse") setFlagBit(str2bool(pval),&comp->mouse->flags,INF_ENABLED);
					if (pnam == "mouse.wheel") setFlagBit(str2bool(pval),&comp->mouse->flags,INF_WHEEL);
					break;
				case PS_SDC:
					if (pnam == "sdcimage") sdcSetImage(comp->sdc,pval.c_str());
					if (pnam == "sdclock") setFlagBit(str2bool(pval),&comp->sdc->flag,SDC_LOCK);
					if (pnam == "capacity") sdcSetCapacity(comp->sdc,atoi(pval.c_str()));
					break;
			}
		}
	}

	comp->bdi->fdc->turbo = optGetFlag(OF_FASTDISK) ? 1 : 0;

	ideSetPassport(comp->ide,IDE_MASTER,masterPass);
	ideSetPassport(comp->ide,IDE_SLAVE,slavePass);

	zxSetHardware(comp, prf->hwName.c_str());
	rsSetRomset(comp,prf->rsName);

	tmp2 = PLOAD_OK;

	if (comp->hw == NULL) {
		tmp2 = PLOAD_HW;
		zxSetHardware(comp,"ZX48K");
	}

	if (findRomset(prf->rsName) == NULL) {
		tmp2 = PLOAD_RS;
	}

	if ((comp->hw->mask != 0) && (~comp->hw->mask & tmask)) throw("Incorrect memory size for this machine");
	memSetSize(comp->mem,memsz);
	if (!emulSetLayout(comp, prf->layName)) emulSetLayout(zx,"default");


	zxReset(comp,RES_DEFAULT);

	return tmp2;
}

std::string getDiskString(Floppy* flp) {
	std::string res = "40SW";
	if (flp->flag & FLP_TRK80) res[0]='8';
	if (flp->flag & FLP_DS) res[2]='D';
	if (flp->flag & FLP_PROTECT) res[3]='R';
	if (flp->path) {
		res += ':';
		res += std::string(flp->path);
	}
	return res;
}

#define	YESNO(cnd) ((cnd) ? "yes" : "no")

int prfSave(std::string nm) {
	XProfile* prf = (nm == "") ? currentProfile : getProfile(nm);
	if (prf == NULL) return PSAVE_NF;
	ZXComp* comp = prf->zx;

	std::string cfname = optGetString(OPT_WORKDIR) + SLASH + prf->file;
	std::ofstream file(cfname.c_str());
	if (!file.good()) {
		printf("Can't write settings\n");
		return PSAVE_OF;
	}

	file << "[MACHINE]\n\n";
	file << "current = " << prf->hwName.c_str() << "\n";
	file << "memory = " << int2str(comp->mem->memSize) << "\n";
	file << "cpu.frq = " << int2str(comp->cpuFrq * 2) << "\n";
	file << "scrp.wait = " << YESNO(comp->hwFlag & HW_WAIT) << "\n";
	file << "contio = " << YESNO(comp->hwFlag & HW_CONTIO) << "\n";
	file << "contmem = " << YESNO(comp->hwFlag & HW_CONTMEM) << "\n";

	file << "\n[ROMSET]\n\n";
	file << "current = " << prf->rsName.c_str() << "\n";
	file << "reset = " << int2str(comp->resbank) << "\n";

	file << "\n[VIDEO]\n\n";
	file << "geometry = " << prf->layName.c_str() << "\n";
	file << "4t-border = " << YESNO(comp->vid->flags & VID_BORDER_4T) << "\n";

	file << "\n[SOUND]\n\n";
	file << "chip1 = " << int2str(comp->ts->chipA->type) << "\n";
	file << "chip1.stereo = " << int2str(comp->ts->chipA->stereo) << "\n";
	file << "chip2 = " << int2str(comp->ts->chipB->type) << "\n";
	file << "chip2.stereo = " << int2str(comp->ts->chipB->stereo) << "\n";
	file << "ts.type = " << int2str(comp->ts->type) << "\n";
	file << "gs = " << YESNO(comp->gs->flag & GS_ENABLE) << "\n";
	file << "gs.reset = " << YESNO(comp->gs->flag & GS_RESET) << "\n";
	file << "gs.stereo = " << int2str(comp->gs->stereo) << "\n";
	file << "soundrive_type = " << int2str(comp->sdrv->type) << "\n";

	file << "\n[INPUT]\n\n";
	file << "mouse = " << YESNO(comp->mouse->flags & INF_ENABLED) << "\n";
	file << "mouse.wheel = " << YESNO(comp->mouse->flags & INF_WHEEL) << "\n";

	file << "\n[TAPE]\n\n";
	file << "path = " << (comp->tape->path ? comp->tape->path : "") << "\n";

	file << "\n[DISK]\n\n";
	file << "type = " << int2str(comp->bdi->fdc->type) << "\n";
	file << "fast = " << YESNO(comp->bdi->fdc->turbo) << "\n";
	file << "A = " << getDiskString(comp->bdi->fdc->flop[0]).c_str() << "\n";
	file << "B = " << getDiskString(comp->bdi->fdc->flop[1]).c_str() << "\n";
	file << "C = " << getDiskString(comp->bdi->fdc->flop[2]).c_str() << "\n";
	file << "D = " << getDiskString(comp->bdi->fdc->flop[3]).c_str() << "\n";

	file << "\n[IDE]\n\n";
	file << "iface = " << int2str(comp->ide->type) << "\n";
	file << "master.type = " << int2str(comp->ide->master->type) << "\n";
	ATAPassport pass = ideGetPassport(comp->ide,IDE_MASTER);
	file << "master.image = " << ((comp->ide->master->image) ? comp->ide->master->image : "") << "\n";
	file << "master.lba = " << YESNO(comp->ide->master->flags & ATA_LBA) << "\n";
	file << "master.maxlba = " << int2str(comp->ide->master->maxlba) << "\n";
	file << "master.chs = " << int2str(pass.spt) << "/" << int2str(pass.hds) << "/" << int2str(pass.cyls) << "\n";
	file << "slave.type = " << int2str(comp->ide->slave->type) << "\n";
	pass = ideGetPassport(comp->ide,IDE_SLAVE);
	file << "slave.image = " << ((comp->ide->slave->image) ? comp->ide->slave->image : "") << "\n";
	file << "slave.lba = " << YESNO(comp->ide->slave->flags & ATA_LBA) << "\n";
	file << "slave.maxlba = " << int2str(comp->ide->slave->maxlba) << "\n";
	file << "slave.chs = " << int2str(pass.spt) << "/" << int2str(pass.hds) << "/" << int2str(pass.cyls) << "\n";

	file << "\n[SDC]\n\n";
	file << "sdcimage = " << (comp->sdc->image ? comp->sdc->image : "") << "\n";
	file << "sdclock = " << YESNO(comp->sdc->flag & SDC_LOCK) << "\n";
	file << "capacity" << int2str(comp->sdc->capacity) << "\n";

	return PSAVE_OK;
}
