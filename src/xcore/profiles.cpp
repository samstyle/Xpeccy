#include <stdio.h>
#include <fstream>
#include <math.h>

#include "xcore.h"
#include "../filer.h"

xProfile* findProfile(std::string nm) {
	if (nm == "") return conf.prof.cur;
	xProfile* res = NULL;
	for (uint i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->name == nm)
			res = conf.prof.list[i];
	}
	return res;
}

bool addProfile(std::string nm, std::string fp) {
//	printf("add Profile: %s : %s\n",nm.c_str(),fp.c_str());
	if (findProfile(nm) != NULL) return false;
	xProfile* nprof = new xProfile;
	nprof->name = nm;
	nprof->file = fp;
	nprof->layName = std::string("default");
	nprof->zx = compCreate();
	std::string fname = conf.path.confDir + SLASH + nprof->name + ".cmos";
	std::ifstream file(fname.c_str());
	if (file.good()) {
		file.read((char*)nprof->zx->cmos.data,256);
		file.close();
	}
	fname = conf.path.confDir + SLASH + nprof->name + ".nvram";
	file.open(fname.c_str());
	if (file.good()) {
		file.read((char*)nprof->zx->ide->smuc.nv->mem,0x800);
		file.close();
	}
	compSetHardware(nprof->zx,"ZX48K");
	conf.prof.list.push_back(nprof);
	return true;
}

void prfClose() {
	if (!conf.prof.cur) return;
	ideCloseFiles(conf.prof.cur->zx->ide);
	sdcCloseFile(conf.prof.cur->zx->sdc);
}

int delProfile(std::string nm) {
	xProfile* prf = findProfile(nm);
	if (prf == NULL) return DELP_ERR;		// no such profile
	if (prf->name == "default") return DELP_ERR;	// can't touch this
	int res = DELP_OK;
	std::string cpath;
	// set default profile if current deleted
	if (conf.prof.cur) {
		if (conf.prof.cur->name == nm) {
			prfSetCurrent("default");
			res = DELP_OK_CURR;
		}
	} else {
		prfSetCurrent("default");
	}
	// remove all such profiles from list & free mem
	for (uint i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->name == nm) {
			cpath = conf.path.confDir + SLASH + prf->file;
			remove(cpath.c_str());				// remove config file
			cpath = conf.path.confDir + SLASH + prf->name + ".cmos";
			remove(cpath.c_str());
			cpath = conf.path.confDir + SLASH + prf->name + ".nvram";
			remove(cpath.c_str());
			compDestroy(prf->zx);				// delete ZX
			delete(prf);
			conf.prof.list.erase(conf.prof.list.begin() + i);
		}
	}
	return res;
}

bool prfSetCurrent(std::string nm) {
	xProfile* nprf = findProfile(nm);
	if (nprf == NULL) return false;
	prfClose();
	conf.prof.cur = nprf;
	ideOpenFiles(nprf->zx->ide);
	sdcOpenFile(nprf->zx->sdc);
	prfSetLayout(conf.prof.cur, conf.prof.cur->layName);
	keyReleaseAll(conf.prof.cur->zx->keyb);
	conf.prof.cur->zx->mouse->buttons = 0xff;
	return true;
}

void clearProfiles() {
	while (conf.prof.list.size() > 1) {
		conf.prof.list.pop_back();
	}
	prfSetCurrent(conf.prof.list[0]->name);
}

bool prfSetLayout(xProfile* prf, std::string nm) {
	if (prf == NULL) prf = conf.prof.cur;
	xLayout* lay = findLayout(nm);
	if (lay == NULL) return false;
	prf->layName = nm;
	compSetLayout(prf->zx, lay->lay);
	vidUpdateLayout(prf->zx->vid, conf.brdsize);
	return true;
}

void prfChangeRsName(std::string oldName, std::string newName) {
	for (uint i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->rsName == oldName)
			conf.prof.list[i]->rsName = newName;
	}
}

void prfChangeLayName(std::string oldName, std::string newName) {
	for (uint i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->layName == oldName)
			conf.prof.list[i]->layName = newName;
	}
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
#define PS_SLOT		10

void setDiskString(Computer* comp,Floppy* flp,std::string st) {
	if (st.size() < 4) return;
	flp->trk80 = (st.substr(0,2) == "80") ? 1 : 0;
	flp->doubleSide = (st.substr(2,1) == "D") ? 1 : 0;
	flp->protect = (st.substr(3,1) == "R") ? 1 : 0;
	if (flp->path || (st.size() < 5) || !conf.storePaths) return;
	st = st.substr(5);
	if (st.size() > 1) loadFile(comp,st.c_str(),FT_DISK,flp->id);
}

// set specified romset to specified profile & load into ROM of this profile ZX
void prfSetRomset(xProfile* prf, std::string rnm) {
	if (prf == NULL) prf = conf.prof.cur;
	prf->rsName = rnm;
	xRomset* rset = findRomset(rnm);
	//printf("romset %s : %p\n",rnm.c_str(), rset);
	int i;
	std::string fpath = "";
	std::ifstream file;
	char pageBuf[0x4000];
	int prts = 0;
	prf->zx->mem->romMask = 0x03;
	if (rset == NULL) {		// romset not found : fill all ROM with 0xFF
		memset(pageBuf,0xff,0x4000);
		for (i=0; i<16; i++) {
			memSetPageData(prf->zx->mem,MEM_ROM,i,pageBuf);
		}
	} else {			// romset found
		if (rset->file.size() != 0) {			// single rom file
			fpath = conf.path.romDir + SLASH + rset->file;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.seekg(0,std::ios_base::end);
				prts = file.tellg() / 0x4000;
				if (file.tellg() & 0x3fff) prts++;
				if (prts > 4) prf->zx->mem->romMask = 0x07;
				if (prts > 8) prf->zx->mem->romMask = 0x0f;
				if (prts > 16) prf->zx->mem->romMask = 0x1f;
				if (prts > 32) prts = 32;
				file.seekg(0,std::ios_base::beg);
				//printf("%s : prts = %i (mask %.2X)\n",rnm.c_str(), prts,prf->zx->mem->romMask);
				for (i = 0; i < prts; i++) {
					file.read(pageBuf,0x4000);
					memSetPageData(prf->zx->mem,MEM_ROM,i,pageBuf);
/*
					printf("PART %i:\n",i);
					for (int j = 0; j < 0x100; j++) {
						if (!(j & 0x0f)) {
							printf("\n%.4X : ",j);
						}
						printf("%.2X ",prf->zx->mem->romData[j] & 0xff);
					}
					printf("\n");
*/
				}
				memset(pageBuf,0xff,0x4000);
				for (i = prts; i < 32; i++)
					memSetPageData(prf->zx->mem,MEM_ROM,i,pageBuf);
			} else {
				printf("Can't open single rom '%s'\n",rset->file.c_str());
				memset(pageBuf,0xff,0x4000);
				for (i = 0; i < 32; i++)
					memSetPageData(prf->zx->mem,MEM_ROM,i,pageBuf);
			}
			file.close();
		} else {					// separate files
			for (i = 0; i < 4; i++) {
				if (rset->roms[i].path == "") {
					memset(pageBuf,0xff,0x4000);
					// for (ad = 0; ad < 0x4000; ad++) pageBuf[ad]=0xff;
				} else {
					fpath = conf.path.romDir + SLASH + rset->roms[i].path;
					file.open(fpath.c_str(),std::ios::binary);
					if (file.good()) {
						file.seekg(rset->roms[i].part << 14);
						file.read(pageBuf,0x4000);
					} else {
						printf("Can't open rom '%s:%i'\n",rset->roms[i].path.c_str(),rset->roms[i].part);
						memset(pageBuf,0xff,0x4000);
						//for (ad=0;ad<0x4000;ad++) pageBuf[ad]=0xff;
					}
					file.close();
				}
				memSetPageData(prf->zx->mem,MEM_ROM,i,pageBuf);
			}
		}
		memset(pageBuf,0xff,0x4000);
		// for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
		if (rset->gsFile.empty()) {
			gsSetRom(prf->zx->gs,0,pageBuf);
			gsSetRom(prf->zx->gs,1,pageBuf);
		} else {
			fpath = conf.path.romDir + SLASH + rset->gsFile;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.read(pageBuf,0x4000);
				gsSetRom(prf->zx->gs,0,pageBuf);
				file.read(pageBuf,0x4000);
				gsSetRom(prf->zx->gs,1,pageBuf);
			} else {
				//			printf("Can't load gs rom '%s'\n",prof->gsFile.c_str());
				gsSetRom(prf->zx->gs,0,pageBuf);
				gsSetRom(prf->zx->gs,1,pageBuf);
			}
			file.close();
		}
		if (!rset->fntFile.empty()) {
			fpath = conf.path.romDir + SLASH + rset->fntFile;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.read(pageBuf,0x800);
				vidSetFont(prf->zx->vid,pageBuf);
			}
		}
	}
}

void prfLoadAll() {
	xProfile* prf;
	foreach(prf, conf.prof.list) {
		prfLoad(prf->name);
	}
}

int prfLoad(std::string nm) {
	xProfile* prf = findProfile(nm);
	if (prf == NULL) return PLOAD_NF;
	Computer* comp = prf->zx;

	std::string cfname = conf.path.confDir + SLASH + prf->file;
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
			if (pnam=="[SLOT]") section = PS_SLOT;
		} else {
			switch (section) {
				case PS_ROMSET:
					if (pnam=="reset") {
						comp->resbank = RES_48;
						if ((pval == "basic128") || (pval=="0")) comp->resbank = RES_128;
						if ((pval == "basic48") || (pval=="1")) comp->resbank = RES_48;
						if ((pval == "shadow") || (pval=="2")) comp->resbank = RES_SHADOW;
						if ((pval == "trdos") || (pval=="3")) comp->resbank = RES_DOS;
					}
					if (pnam=="current") prf->rsName = pval;
					break;
				case PS_VIDEO:
					if (pnam == "geometry") prf->layName = pval;
					if (pnam == "4t-border") comp->vid->border4t = str2bool(pval) ? 1 : 0;
					if (pnam == "ULAplus") comp->vid->ula->enabled = str2bool(pval) ? 1 : 0;
					if (pnam == "fps") {
						tmp2 = atoi(pval.c_str());
						if (tmp2 < 25) tmp2 = 25;
						else if (tmp2 > 100) tmp2 = 100;
						vidSetFps(comp->vid, tmp2);
					}
					break;
				case PS_SOUND:
					if (pnam == "chip1") aymSetType(comp->ts->chipA,atoi(pval.c_str()));
					if (pnam == "chip1.stereo") comp->ts->chipA->stereo = atoi(pval.c_str());
					if (pnam == "chip2") aymSetType(comp->ts->chipB,atoi(pval.c_str()));
					if (pnam == "chip2.stereo") comp->ts->chipB->stereo = atoi(pval.c_str());
					if (pnam == "ts.type") comp->ts->type = atoi(pval.c_str());
					if (pnam == "gs") comp->gs->enable = str2bool(pval) ? 1 : 0;
					if (pnam == "gs.reset") comp->gs->reset = str2bool(pval) ? 1 : 0;
					if (pnam == "gs.stereo") comp->gs->stereo = atoi(pval.c_str());
					if (pnam == "soundrive_type") comp->sdrv->type = atoi(pval.c_str());
					if (pnam == "saa.mode") {
						tmp2 = atoi(pval.c_str());
						switch (tmp2) {
							case 0: comp->saa->enabled = 0; break;
							case 1: comp->saa->enabled = 1; comp->saa->mono = 1; break;
							case 2: comp->saa->enabled = 1; comp->saa->mono = 0; break;
						}
					}
					break;
				case PS_TAPE:
					if (pnam == "path" && conf.storePaths) loadFile(comp,pval.c_str(),FT_TAPE,0);
					break;
				case PS_DISK:
					if (pnam == "A") setDiskString(comp,comp->dif->fdc->flop[0],pval);
					if (pnam == "B") setDiskString(comp,comp->dif->fdc->flop[1],pval);
					if (pnam == "C") setDiskString(comp,comp->dif->fdc->flop[2],pval);
					if (pnam == "D") setDiskString(comp,comp->dif->fdc->flop[3],pval);
					if (pnam == "type") difSetHW(comp->dif, atoi(pval.c_str()));
					break;
				case PS_MACHINE:
					if (pnam == "current") prf->hwName = pval;
					if (pnam == "cpu.type") cpuSetType(comp->cpu, getCoreID(pval.c_str()));
					if (pnam == "cpu.frq") {
						tmp2 = atoi(pval.c_str());
						if ((tmp2 > 1) && (tmp2 < 29)) tmp2 *= 5e5;	// old 2..28 -> 500000..14000000
						if (tmp2 < 1e6) tmp2 = 1e6;
						if (tmp2 > 14e6) tmp2 = 14e6;
						compSetBaseFrq(comp, tmp2 / 1e6);
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
					if (pnam == "contmem") comp->contMem = str2bool(pval) ? 1 : 0;
					//if (pnam == "contmemP3") setFlagBit(str2bool(pval),&comp->vid->flags,VID_CONT2);
					if (pnam == "contio") comp->contIO = str2bool(pval) ? 1 : 0;
					if (pnam == "scrp.wait") comp->scrpWait = str2bool(pval) ? 1 : 0;
					break;
				case PS_IDE:
					if (pnam == "iface") comp->ide->type = atoi(pval.c_str());
					if (pnam == "master.type") comp->ide->master->type = atoi(pval.c_str());
					if (pnam == "master.lba") comp->ide->master->hasLBA = str2bool(pval) ? 1 : 0;
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
					if (pnam == "slave.lba") comp->ide->slave->hasLBA = str2bool(pval) ? 1 : 0;
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
					if (pnam == "mouse") comp->mouse->enable = str2bool(pval) ? 1 : 0;
					if (pnam == "mouse.wheel") comp->mouse->hasWheel = str2bool(pval) ? 1 : 0;
					if (pnam == "mouse.swapButtons") comp->mouse->swapButtons = str2bool(pval) ? 1 : 0;
					break;
				case PS_SDC:
					if (pnam == "sdcimage") sdcSetImage(comp->sdc,pval.c_str());
					if (pnam == "sdclock") comp->sdc->lock = str2bool(pval) ? 1 : 0;
					if (pnam == "capacity") sdcSetCapacity(comp->sdc,atoi(pval.c_str()));
					break;
				case PS_SLOT:
					if (pnam == "slotA.type") comp->msx.slotA.mapType = atoi(pval.c_str());
					if (pnam == "slotB.type") comp->msx.slotB.mapType = atoi(pval.c_str());
					break;
			}
		}
	}

	ideSetPassport(comp->ide,IDE_MASTER,masterPass);
	ideSetPassport(comp->ide,IDE_SLAVE,slavePass);

	compSetHardware(comp, prf->hwName.c_str());
	prfSetRomset(prf, prf->rsName);

	tmp2 = PLOAD_OK;

	if (comp->hw == NULL) {
		tmp2 = PLOAD_HW;
		compSetHardware(comp,"ZX48K");
	}

	if (findRomset(prf->rsName) == NULL) {
		tmp2 = PLOAD_RS;
	}

	if ((comp->hw->mask != 0) && (~comp->hw->mask & tmask)) throw("Incorrect memory size for this machine");
	memSetSize(comp->mem,memsz);
	if (!prfSetLayout(prf, prf->layName)) prfSetLayout(prf,"default");

	compReset(comp,RES_DEFAULT);

	return tmp2;
}

std::string getDiskString(Floppy* flp) {
	std::string res = "40SW";
	if (flp->trk80) res[0]='8';
	if (flp->doubleSide) res[2]='D';
	if (flp->protect) res[3]='R';
	if (flp->path) {
		res += ':';
		res += std::string(flp->path);
	}
	return res;
}

int prfSave(std::string nm) {
	xProfile* prf = conf.prof.cur;
	if (prf == NULL) return PSAVE_NF;
	Computer* comp = prf->zx;

	std::string cfname = conf.path.confDir + SLASH + prf->file;
	FILE* file = fopen(cfname.c_str(),"wb");
	if (!file) {
		printf("Can't write settings\n");
		return PSAVE_OF;
	}

	fprintf(file, "[MACHINE]\n\n");
	fprintf(file, "current = %s\n", prf->hwName.c_str());
	fprintf(file, "memory = %i\n", comp->mem->memSize);
	fprintf(file, "cpu.type = %s\n", getCoreName(comp->cpu->type));
	fprintf(file, "cpu.frq = %i\n", int(comp->cpuFrq * 1e6));
	fprintf(file, "scrp.wait = %s\n", YESNO(comp->scrpWait));
	fprintf(file, "contio = %s\n", YESNO(comp->contIO));
	fprintf(file, "contmem = %s\n", YESNO(comp->contMem));

	fprintf(file, "\n[ROMSET]\n\n");
	fprintf(file, "current = %s\n", prf->rsName.c_str());
	fprintf(file, "reset = ");
	switch (comp->resbank) {
		case RES_48: fprintf(file, "basic48\n"); break;
		case RES_128: fprintf(file, "basic128\n"); break;
		case RES_DOS: fprintf(file, "dos\n"); break;
		case RES_SHADOW: fprintf(file, "shadow\n"); break;
	}

	fprintf(file, "\n[VIDEO]\n\n");
	fprintf(file, "geometry = %s\n", prf->layName.c_str());
	fprintf(file, "4t-border = %s\n", YESNO(comp->vid->border4t));
	fprintf(file, "ULAplus = %s\n", YESNO(comp->vid->ula->enabled));
	fprintf(file, "fps = %i\n",comp->vid->fps);

	fprintf(file, "\n[SOUND]\n\n");
	fprintf(file, "chip1 = %i\n", comp->ts->chipA->type);
	fprintf(file, "chip1.stereo = %i\n", comp->ts->chipA->stereo);
	fprintf(file, "chip2 = %i\n", comp->ts->chipB->type);
	fprintf(file, "chip2.stereo = %i\n", comp->ts->chipB->stereo);
	fprintf(file, "ts.type = %i\n", comp->ts->type);
	fprintf(file, "gs = %s\n", YESNO(comp->gs->enable));
	fprintf(file, "gs.reset = %s\n", YESNO(comp->gs->reset));
	fprintf(file, "gs.stereo = %i\n", comp->gs->stereo);
	fprintf(file, "soundrive_type = %i\n", comp->sdrv->type);
	fprintf(file, "saa.mode = %i\n", comp->saa->enabled ? (comp->saa->mono ? 1 : 2) : 0);

	fprintf(file, "\n[INPUT]\n\n");
	fprintf(file, "mouse = %s\n", YESNO(comp->mouse->enable));
	fprintf(file, "mouse.wheel = %s\n", YESNO(comp->mouse->hasWheel));
	fprintf(file, "mouse.swapButtons = %s\n", YESNO(comp->mouse->swapButtons));

	fprintf(file, "\n[TAPE]\n\n");
	fprintf(file, "path = %s\n", comp->tape->path ? comp->tape->path : "");

	fprintf(file, "\n[DISK]\n\n");
	fprintf(file, "type = %i\n", comp->dif->type);
	fprintf(file, "A = %s\n", getDiskString(comp->dif->fdc->flop[0]).c_str());
	fprintf(file, "B = %s\n", getDiskString(comp->dif->fdc->flop[1]).c_str());
	fprintf(file, "C = %s\n", getDiskString(comp->dif->fdc->flop[2]).c_str());
	fprintf(file, "D = %s\n", getDiskString(comp->dif->fdc->flop[3]).c_str());

	fprintf(file, "\n[IDE]\n\n");
	fprintf(file, "iface = %i\n", comp->ide->type);
	fprintf(file, "master.type = %i\n", comp->ide->master->type);
	ATAPassport pass = ideGetPassport(comp->ide,IDE_MASTER);
	fprintf(file, "master.image = %s\n", comp->ide->master->image ? comp->ide->master->image : "");
	fprintf(file, "master.lba = %s\n", YESNO(comp->ide->master->hasLBA));
	fprintf(file, "master.maxlba = %i\n", comp->ide->master->maxlba);
	fprintf(file, "master.chs = %i/%i/%i\n", pass.spt, pass.hds, pass.cyls);
	fprintf(file, "slave.type = %i\n", comp->ide->slave->type);
	pass = ideGetPassport(comp->ide,IDE_SLAVE);
	fprintf(file, "slave.image = %s\n", comp->ide->slave->image ? comp->ide->slave->image : "");
	fprintf(file, "slave.lba = %s\n", YESNO(comp->ide->slave->hasLBA));
	fprintf(file, "slave.maxlba = %i\n", comp->ide->slave->maxlba);
	fprintf(file, "slave.chs = %i/%i/%i\n", pass.spt, pass.hds, pass.cyls);

	fprintf(file, "\n[SDC]\n\n");
	fprintf(file, "sdcimage = %s\n", comp->sdc->image ? comp->sdc->image : "");
	fprintf(file, "sdclock = %s\n", YESNO(comp->sdc->lock));
	fprintf(file, "capacity = %i\n", comp->sdc->capacity);

	fprintf(file, "\n[SLOT]\n");
	fprintf(file, "slotA.type = %i\n",comp->msx.slotA.mapType);
	fprintf(file, "slotB.type = %i\n",comp->msx.slotB.mapType);

	fclose(file);

	return PSAVE_OK;
}
