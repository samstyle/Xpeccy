#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <sys/stat.h>

#include "xcore.h"
#include "../xgui/xgui.h"
#include "sound.h"
#include "filer.h"
#include "gamepad.h"

void prf_load_cmos(xProfile* prf, std::string path) {
	FILE* file = fopen(path.c_str(), "rb");
	if (file) {
		fread((char*)prf->zx->cmos.data, 256, 1, file);
		fclose(file);
	}
}

void prf_save_cmos(xProfile* prf, std::string path) {
	FILE* file = fopen(path.c_str(), "wb");
	if (file) {
		fwrite((char*)prf->zx->cmos.data, 256, 1, file);
		fclose(file);
	}
}

void prf_load_nvram(xProfile* prf, std::string path) {
	FILE* file = fopen(path.c_str(), "rb");
	if (file) {
		fread((char*)prf->zx->ide->smuc.nv->mem, 256, 1, file);
		fclose(file);
	}
}

void prf_save_nvram(xProfile* prf, std::string path) {
	if (prf->zx->ide->type != IDE_SMUC) return;
	FILE* file = fopen(path.c_str(), "wb");
	if (file) {
		fwrite((char*)prf->zx->ide->smuc.nv->mem, 256, 1, file);
		fclose(file);
	}
}

xProfile* findProfile(std::string nm) {
	if (nm == "") return conf.prof.cur;
	xProfile* res = NULL;
	for (int i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->name == nm)
			res = conf.prof.list[i];
	}
	return res;
}

xProfile* addProfile(std::string nm, std::string fp) {
//	printf("add Profile: %s : %s\n",nm.c_str(),fp.c_str());
	if (findProfile(nm) != NULL) return NULL;
	xProfile* nprof = new xProfile;
	nprof->name = nm;
	nprof->file = fp;
	nprof->layName = std::string("default");
	nprof->zx = compCreate();
	std::string fname;
	fname = conf.path.prfDir + SLASH + nprof->name;
#if defined(__linux) || defined(__APPLE__) || defined(__BSD)
	mkdir(fname.c_str(), 0777);
#elif defined(__WIN32)
	mkdir(fname.c_str());
#endif
	prf_load_cmos(nprof, conf.path.prfDir + SLASH + nprof->name + SLASH + nprof->name + ".cmos");
	prf_load_nvram(nprof, conf.path.prfDir + SLASH + nprof->name + SLASH + nprof->name + ".nvram");
	/*
	fname = conf.path.prfDir + SLASH + nprof->name + SLASH + nprof->name + ".cmos";
	FILE* file = fopen(fname.c_str(), "rb");
	if (file) {
		fread((char*)nprof->zx->cmos.data,256,1,file);
		fclose(file);
	}
	fname = conf.path.confDir + SLASH + nprof->name + SLASH + nprof->name + ".nvram";
	file = fopen(fname.c_str(), "rb");
	if (file) {
		fread((char*)nprof->zx->ide->smuc.nv->mem,0x800,1,file);
		fclose(file);
	}
	*/
	compSetHardware(nprof->zx,"Dummy");
	conf.prof.list.push_back(nprof);
	return nprof;
}

int copyProfile(std::string src, std::string dst) {
	xProfile* sprf = findProfile(src);
	if (sprf == NULL)
		return 0;
	xProfile* dprf = findProfile(dst);
	std::string dfile = dst + ".conf";
	if (dprf == NULL) {
		dprf = addProfile(dst, dfile);
	} else {
		dprf->file = dfile;
	}
	std::string sfname = conf.path.prfDir + SLASH + sprf->name + SLASH + sprf->file;
	std::string dfname = conf.path.prfDir + SLASH + dprf->name + SLASH + dfile;
	copyFile(sfname.c_str(), dfname.c_str());
	prfLoad(dst);
	return 1;
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
	std::string cdir;
	// set default profile if current deleted
	if (conf.prof.cur) {
		if (conf.prof.cur->name == nm) {
			prfSetCurrent("default");
			res = DELP_OK_CURR;
		}
	} else {
		// prfSetCurrent("default");
	}
	// remove all such profiles from list & free mem
	for (int i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->name == nm) {
			cdir = conf.path.prfDir + SLASH + prf->name + SLASH;
			cpath = cdir + prf->file;
			remove(cpath.c_str());					// remove config file
			cpath = cdir + prf->name + ".cmos";
			remove(cpath.c_str());					// remove cmos dump
			cpath = cdir + prf->name + ".nvram";
			remove(cpath.c_str());					// remove nvram dump
			rmdir(cdir.c_str());					// remove directory (leave it if there is files)
			compDestroy(prf->zx);					// delete computer
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
	prfSetLayout(nprf, nprf->layName);
	kbdReleaseAll(nprf->zx->keyb);
	mouseReleaseAll(nprf->zx->mouse);
	padLoadConfig(nprf->jmapName);
	loadKeys();
	compSetHardware(nprf->zx, NULL);
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
	comp_set_layout(prf->zx, &lay->lay);
	vidSetBorder(prf->zx->vid, conf.brdsize);
	return true;
}

void prfChangeRsName(std::string oldName, std::string newName) {
	for (int i = 0; i < conf.prof.list.size(); i++) {
		if (conf.prof.list[i]->rsName == oldName)
			conf.prof.list[i]->rsName = newName;
	}
}

void prfChangeLayName(std::string oldName, std::string newName) {
	for (int i = 0; i < conf.prof.list.size(); i++) {
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
	// TODO: do not load files before set hw
	if (st.size() > 1) {
		flp_set_path(flp, st.c_str());		// delayed loading after set hw
	}
}

// set specified romset to specified profile & load into ROM of this profile ZX
void prfSetRomset(xProfile* prf, std::string rnm) {
	if (prf == NULL)
		prf = conf.prof.cur;
	prf->rsName = rnm;
	xRomset* rset = findRomset(rnm);
	std::string fpath;
	int romsz = prf->zx->mem->romSize;
	int foff;
	int fsze;
	int roff;
	FILE* file;
	if (rset) {
		memset(prf->zx->mem->romData, 0xff, MEM_512K);
		foreach(xRomFile xrf, rset->roms) {
			foff = xrf.foffset * 1024;
			roff = xrf.roffset * 1024;
			fpath = conf.path.romDir + SLASH + xrf.name;
			file = fopen(fpath.c_str(), "rb");
			if (file) {
				if (xrf.fsize <= 0) {			// check part size
					fseek(file, 0, SEEK_END);
					fsze = ftell(file);
					rewind(file);
				} else {
					fsze = xrf.fsize * 1024;
				}
				if (roff + fsze > romsz) {	// check crossing rom top
					romsz = toLimits(roff + fsze, MEM_256, MEM_512K);
					romsz = toPower(romsz);
				}
				if (roff + fsze > romsz)
					fsze = romsz - roff;
				if ((foff >= 0) && (roff >= 0) && (roff < MEM_512K) && (fsze > 0)) {	// load rom if all is ok
					fseek(file, foff, SEEK_SET);
					fread(prf->zx->mem->romData + roff, fsze, 1, file);
				}
				fclose(file);
			}
		}
		memSetSize(prf->zx->mem, -1, romsz);
// load GS ROM
		fpath = conf.path.romDir + SLASH + rset->gsFile;
		file = fopen(fpath.c_str(), "rb");
		if (file) {
			fread(prf->zx->gs->mem->romData, MEM_32K, 1, file);
			fclose(file);
		} else {
			memset((char*)prf->zx->gs->mem->romData, 0xff, MEM_32K);
		}
// load ATM2 font data
		if (!rset->fntFile.empty()) {
			fpath = conf.path.romDir + SLASH + rset->fntFile;
			file = fopen(fpath.c_str(), "rb");
			if (file) {
				fread(prf->zx->vid->font, MEM_2K, 1, file);
				fclose(file);
			}
		}
	}
}

int prf_load_conf(xProfile* prf, std::string cfname, int flag) {
	Computer* comp = prf->zx;
	Floppy* flp;
	int i;
	std::ifstream file(cfname);
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char buf[0x4000];
	int tmask = -1;
	int tmp2;
	int chatype = SND_NONE;
	int chbtype = SND_NONE;
	int chctype = SND_NONE;
	double tmpd;
	int section = PS_NONE;
	if (!file.good() && flag) {
		printf("Profile config is missing. Default one will be created\n");
		copyFile(":/conf/xpeccy.conf", cfname.c_str());
		file.open(cfname, std::ifstream::in);
	}
	if (!file.good()) {
		if (flag) printf("Damn! I can't open config file");
		return PLOAD_OF;
	}
#if defined(__WIN32)
	prf->lastDir = ".";
#elif defined(__linux) || defined(__APPLE__) || defined(__BSD)
	prf->lastDir = std::string(getenv("HOME"));
#endif

	xArg arg;

	while (!file.eof()) {
		file.getline(buf,2048);
		line = std::string(buf);
		pos = line.find_first_of("#"); if (pos != std::string::npos) line.erase(pos);
		pos = line.find_first_of(";"); if (pos != std::string::npos) line.erase(pos);
		spl = splitline(line);
		pnam = spl.first;
		pval = spl.second;

		arg.b = str2bool(pval) ? 1 : 0;
		arg.s = pval.c_str();
		arg.i = strtol(arg.s, NULL, 0);
		arg.d = strtod(arg.s, NULL);

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
						if ((pval == "dos") || (pval=="3")) comp->resbank = RES_DOS;
					}
					if (pnam=="current") prf->rsName = pval;
					break;
				case PS_VIDEO:
					if (pnam == "geometry") prf->layName = pval;
					if (pnam == "4t-border") comp->vid->brdstep = arg.b ? 7 : 1;
					if (pnam == "ULAplus") comp->vid->ula->enabled = arg.b;
					if (pnam == "DDpal") comp->ddpal = arg.b;
					break;
				case PS_SOUND:
					if (pnam == "chip1") chatype = arg.i;
					if (pnam == "chip1.stereo") comp->ts->chipA->stereo = arg.i;
					if (pnam == "chip1.frq") comp->ts->chipA->frq = arg.d;

					if (pnam == "chip2") chbtype = arg.i;
					if (pnam == "chip2.stereo") comp->ts->chipB->stereo = arg.i;
					if (pnam == "chip2.frq") comp->ts->chipB->frq = arg.d;

					if (pnam == "chip3") chctype = arg.i;
					if (pnam == "chip3.stereo") comp->ts->chipC->stereo = arg.i;
					if (pnam == "chip3.frq") comp->ts->chipC->frq = arg.d;

					if (pnam == "gs") comp->gs->enable = arg.b;
					if (pnam == "gs.reset") comp->gs->reset = arg.b;
					if (pnam == "gs.stereo") comp->gs->stereo = arg.b ? GS_12_34 : GS_MONO;

					if (pnam == "ts.type") comp->ts->type = arg.i;
					if (pnam == "soundrive_type") comp->sdrv->type = arg.i;
					if (pnam == "saa") comp->saa->enabled = arg.b;

					break;
				case PS_TAPE:
					if ((pnam == "path") && conf.storePaths) {
						tape_set_path(comp->tape, pval.c_str());
					}
					break;
				case PS_DISK:
					if (pnam == "A") setDiskString(comp,comp->dif->fdc->flop[0],pval);
					if (pnam == "B") setDiskString(comp,comp->dif->fdc->flop[1],pval);
					if (pnam == "C") setDiskString(comp,comp->dif->fdc->flop[2],pval);
					if (pnam == "D") setDiskString(comp,comp->dif->fdc->flop[3],pval);
					if (pnam == "type") difSetHW(comp->dif, arg.i);
					break;
				case PS_MACHINE:
					if (pnam == "current") prf->hwName = pval;
					if (pnam == "cpu.type") cpuSetType(comp->cpu, getCoreID(arg.s));
					if (pnam == "cpu.frq") {
						tmp2 = arg.i;
						if ((tmp2 > 1) && (tmp2 < 58)) tmp2 *= 5e5;	// old 2..28 -> 500000..14000000
						if (tmp2 < 1e5) tmp2 = 1e5;
						if (tmp2 > 28e6) tmp2 = 28e6;
						compSetBaseFrq(comp, tmp2 / 1e6);
					}
					if (pnam == "frq.mul") {
						tmpd = arg.d;
						if (tmpd < 0.1) tmpd = 0.1;
						if (tmpd > 8.0) tmpd = 8.0;
						compSetTurbo(comp, tmpd);
					}
					if (pnam == "memory") {
						tmp2 = arg.i;
						if (!tmp2) tmp2=64;
						tmp2 <<= 10;			// KB to bytes
						tmp2 = toPower(tmp2);
						tmp2 = toLimits(tmp2, MEM_256, MEM_4M);
						tmask = tmp2;
					}
					if (pnam == "contmem") comp->contMem = arg.b;
					if (pnam == "contio") comp->contIO = arg.b;
					if (pnam == "scrp.wait") comp->evenM1 = arg.b;
					if (pnam == "lastdir") prf->lastDir = pval;
					break;
				case PS_IDE:
					if (pnam == "iface") comp->ide->type = arg.i;
					if (pnam == "master.type") comp->ide->master->type = arg.i;
					if (pnam == "master.lba") comp->ide->master->hasLBA = arg.b;
					if (pnam == "master.image") ideSetImage(comp->ide,IDE_MASTER, arg.s);
					if (pnam == "slave.type") comp->ide->slave->type = arg.i;
					if (pnam == "slave.lba") comp->ide->slave->hasLBA = arg.b;
					if (pnam == "slave.image") ideSetImage(comp->ide,IDE_SLAVE, arg.s);
					break;
				case PS_INPUT:
					if (pnam == "mouse") comp->mouse->enable = arg.b;
					if (pnam == "mouse.wheel") comp->mouse->hasWheel = arg.b;
					if (pnam == "mouse.swapButtons") comp->mouse->swapButtons = arg.b;
					if (pnam == "joy.extbuttons") comp->joy->extbuttons = arg.b;
					if (pnam == "keymap") {
						prf->kmapName = pval;
						loadKeys();
					}
					if (pnam == "gamepad.map") {
						prf->jmapName = pval;
						padLoadConfig(prf->jmapName);
					}
					break;
				case PS_SDC:
					if (pnam == "sdcimage") sdcSetImage(comp->sdc, arg.s);
					if (pnam == "sdclock") comp->sdc->lock = arg.b;
					// if (pnam == "capacity") sdcSetCapacity(comp->sdc, arg.i);
					break;
				case PS_SLOT:
					if ((pnam == "slot.type") || (pnam == "slotA.type"))
						comp->slot->mapType = arg.i;
					break;
			}
		}
	}

	chip_set_type(comp->ts->chipA, chatype);
	chip_set_type(comp->ts->chipB, chbtype);
	chip_set_type(comp->ts->chipC, chctype);

	tmp2 = PLOAD_OK;

	if (!compSetHardware(comp, prf->hwName.c_str())) {
		sprintf(buf, "Profile: %s\nHardware was set to 'dummy'", prf->name.c_str());
		shitHappens(buf);
		tmp2 = PLOAD_HW;
		compSetHardware(comp,"Dummy");
	} else if (conf.storePaths) {			// loading files
		if (comp->tape->path) {
			load_file(comp, comp->tape->path, FG_TAPE, 0);
		}
		for (i = 0; i < 4; i++) {
			flp = comp->dif->fdc->flop[i];
			if (flp->path)
				load_file(comp, flp->path, FG_DISK, flp->id);
		}
	}
	prfSetRomset(prf, prf->rsName);

	if (findRomset(prf->rsName) == NULL) {
		tmp2 = PLOAD_RS;
	}

	// printf("%i: %.X & %.X\n",comp->hw->id,comp->hw->mask, tmask);
	if ((comp->hw->mask != 0) && (~comp->hw->mask & tmask)) {
		tmask = MEM_4M;
		while (!(comp->hw->mask & tmask) && tmask)
			tmask >>= 1;
	}
	memSetSize(comp->mem, tmask, -1);
	if (!prfSetLayout(prf, prf->layName)) prfSetLayout(prf,"default");

	compReset(comp,RES_DEFAULT);

	return tmp2;
}

int prfLoad(std::string nm) {
	xProfile* prf = findProfile(nm);
	if (prf == NULL) return PLOAD_NF;
	//char cfname[FILENAME_MAX];
	std::string cfname = conf.path.prfDir + SLASH + prf->name + SLASH + prf->file;		// new location: $CONFDIR/profiles/$PROFILENAME/$FILENAME
	std::string ofname = conf.path.confDir + SLASH + prf->file;				// old location: $CONFDIR/$FILENAME

	std::string ocmos = conf.path.confDir + SLASH + prf->name + ".cmos";
	std::string ncmos = conf.path.prfDir + SLASH + prf->name + SLASH + prf->name + ".cmos";
	std::string onvr = conf.path.confDir + SLASH + prf->name + ".nvram";
	std::string nnvr = conf.path.prfDir + SLASH + prf->name + SLASH + prf->name + ".nvram";

	int res = prf_load_conf(prf, ofname, 0);
	if (res == PLOAD_OK) {
		copyFile(ofname.c_str(), cfname.c_str());						// copy old file to new location
		remove(ofname.c_str());									// remove old conf file
		prf_load_cmos(prf, ocmos);
		copyFile(ocmos.c_str(), ncmos.c_str());
		remove(ocmos.c_str());
		prf_load_nvram(prf, onvr);
		copyFile(onvr.c_str(), nnvr.c_str());
		remove(onvr.c_str());
	} else {
		res = prf_load_conf(prf, cfname, 1);
		prf_load_cmos(prf, ncmos);
		prf_load_nvram(prf, nnvr);
	}
	return res;
}

void prfLoadAll() {
	xProfile* prf;
	foreach(prf, conf.prof.list) {
		prfLoad(prf->name);
	}
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
	xProfile* prf = findProfile(nm);
	if (prf == NULL)
		prf = conf.prof.cur;
	if (prf == NULL) return PSAVE_NF;
	Computer* comp = prf->zx;

	std::string cfname;
	cfname = conf.path.prfDir + SLASH + prf->name;
#if defined(__linux) || defined(__APPLE__) || defined(__BSD)
	mkdir(cfname.c_str(), 0777);
#elif defined(__WIN32)
	mkdir(cfname.c_str());
#endif
	cfname = cfname + SLASH + prf->file;
//	cfname = conf.path.confDir + SLASH + prf->file;		// old file location

	prf_save_cmos(prf, conf.path.prfDir + SLASH + prf->name + SLASH + prf->name + ".cmos");
	prf_save_nvram(prf, conf.path.prfDir + SLASH + prf->name + SLASH + prf->name + ".nvram");

	FILE* file = fopen(cfname.c_str(), "wb");
	if (!file) {
		printf("Can't write settings\n");
		return PSAVE_OF;
	}

	fprintf(file, "[GENERAL]\n\n");
	fprintf(file, "lastdir = %s\n", prf->lastDir.c_str());

	fprintf(file, "\n[MACHINE]\n\n");
	fprintf(file, "current = %s\n", prf->hwName.c_str());
	fprintf(file, "memory = %i\n", comp->mem->ramSize >> 10);		// bytes to KB
	fprintf(file, "cpu.type = %s\n", getCoreName(comp->cpu->type));
	fprintf(file, "cpu.frq = %i\n", int(comp->cpuFrq * 1e6));
	fprintf(file, "frq.mul = %f\n", comp->frqMul);
	fprintf(file, "scrp.wait = %s\n", YESNO(comp->evenM1));
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
	fprintf(file, "4t-border = %s\n", YESNO(comp->vid->brdstep & 0x06));
	fprintf(file, "ULAplus = %s\n", YESNO(comp->vid->ula->enabled));
	fprintf(file, "DDpal = %s\n", YESNO(comp->ddpal));
	// fprintf(file, "fps = %i\n",comp->vid->fps);

	fprintf(file, "\n[SOUND]\n\n");
	fprintf(file, "chip1 = %i\n", comp->ts->chipA->type);
	fprintf(file, "chip1.stereo = %i\n", comp->ts->chipA->stereo);
	fprintf(file, "chip1.frq = %f\n", comp->ts->chipA->frq);
	fprintf(file, "chip2 = %i\n", comp->ts->chipB->type);
	fprintf(file, "chip2.stereo = %i\n", comp->ts->chipB->stereo);
	fprintf(file, "chip2.frq = %f\n", comp->ts->chipB->frq);
	fprintf(file, "chip3 = %i\n", comp->ts->chipC->type);
	fprintf(file, "chip3.stereo = %i\n", comp->ts->chipC->stereo);
	fprintf(file, "chip3.frq = %f\n", comp->ts->chipC->frq);
	fprintf(file, "ts.type = %i\n", comp->ts->type);

	fprintf(file, "gs = %s\n", YESNO(comp->gs->enable));
	fprintf(file, "gs.reset = %s\n", YESNO(comp->gs->stereo));
	fprintf(file, "gs.stereo = %i\n", comp->gs->stereo);

	fprintf(file, "soundrive_type = %i\n", comp->sdrv->type);

	fprintf(file, "saa = %s\n", YESNO(comp->saa->enabled));
	// fprintf(file, "saa.stereo = %s\n", YESNO(!comp->saa->mono));

	fprintf(file, "\n[INPUT]\n\n");
	fprintf(file, "mouse = %s\n", YESNO(comp->mouse->enable));
	fprintf(file, "mouse.wheel = %s\n", YESNO(comp->mouse->hasWheel));
	fprintf(file, "mouse.swapButtons = %s\n", YESNO(comp->mouse->swapButtons));
	fprintf(file, "joy.extbuttons = %s\n", YESNO(comp->joy->extbuttons));
	fprintf(file, "gamepad.map = %s\n", prf->jmapName.c_str());
	if ((prf->kmapName != "") && (prf->kmapName != "default"))
		fprintf(file, "keymap = %s\n", prf->kmapName.c_str());

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
	fprintf(file, "master.image = %s\n", comp->ide->master->image ? comp->ide->master->image : "");
	fprintf(file, "master.lba = %s\n", YESNO(comp->ide->master->hasLBA));
	fprintf(file, "slave.type = %i\n", comp->ide->slave->type);
	fprintf(file, "slave.image = %s\n", comp->ide->slave->image ? comp->ide->slave->image : "");
	fprintf(file, "slave.lba = %s\n", YESNO(comp->ide->slave->hasLBA));

	fprintf(file, "\n[SDC]\n\n");
	fprintf(file, "sdcimage = %s\n", comp->sdc->image ? comp->sdc->image : "");
	fprintf(file, "sdclock = %s\n", YESNO(comp->sdc->lock));
//	fprintf(file, "capacity = %i\n", comp->sdc->capacity);

	fprintf(file, "\n[SLOT]\n");
	fprintf(file, "slot.type = %i\n",comp->slot->mapType);

	fclose(file);

	return PSAVE_OK;
}
