#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <sys/stat.h>

#include "xcore.h"
#include "migrate.h"
#include "../xgui/xgui.h"
#include "sound.h"
#include "filer.h"
#include "gamepad.h"

namespace {

// Open a file in binary mode and read up to `bytes` bytes into `dest`.
// Returns true iff the file was opened. A short read (truncated file) is
// *not* reported as failure — this matches the pre-existing fread-with-
// ignored-return behavior the old code had for ROM loading.
template <typename T>
bool loadFixedBlob(const fs::path &path, T *dest, std::size_t bytes) {
	std::ifstream f(path, std::ios::binary);
	if (!f) return false;
	f.read(reinterpret_cast<char*>(dest), bytes);
	return true;
}

// Open a file in binary mode and write exactly `bytes` bytes from `src`.
// Silently no-ops on open failure.
template <typename T>
void saveFixedBlob(const fs::path &path, const T *src, std::size_t bytes) {
	std::ofstream f(path, std::ios::binary);
	if (f) f.write(reinterpret_cast<const char*>(src), bytes);
}

// Canonical per-profile directories. Config dir holds the user-editable .conf;
// state dir holds machine-authored cmos/nvram blobs. Both are keyed by profile
// name. Single-source-of-truth so the "<root>/<name>" pattern doesn't drift.
fs::path profileConfigDir(const xProfile *prf) {
	return conf.path.prfDir / prf->name;
}
fs::path profileStateDir(const xProfile *prf) {
	return conf.path.prfStateDir / prf->name;
}

// Canonical location for a per-profile state blob (cmos/nvram) under the XDG
// state-home tree.
fs::path profileStatePath(const xProfile *prf, std::string_view ext) {
	return profileStateDir(prf) / (prf->name + std::string(ext));
}

// One-shot migration of a per-profile state file from older locations into the
// state-home tree. Checks in order: current (stateDir) → legacy profDir → deep
// legacy confDir root. First legacy hit wins; migrateSingleFile short-circuits
// any remaining attempts because `to` now exists.
void migrateProfileState(const xProfile *prf, std::string_view ext) {
	const fs::path target = profileStatePath(prf, ext);
	const std::string leaf = prf->name + std::string(ext);
	const fs::path legacy[] = {
		profileConfigDir(prf) / leaf,
		conf.path.confDir     / leaf,
	};
	std::error_code ec;
	for (const auto &src : legacy) {
		migrate::migrateSingleFile(src, target, "profile state", ec);
	}
}

} // namespace

void prf_load_cmos(xProfile* prf, const fs::path &path) {
	loadFixedBlob(path, prf->zx->cmos.data, 256);
}

void prf_save_cmos(xProfile* prf, const fs::path &path) {
	saveFixedBlob(path, prf->zx->cmos.data, 256);
}

void prf_load_nvram(xProfile* prf, const fs::path &path) {
	loadFixedBlob(path, prf->zx->ide->smuc.nv->mem, 256);
}

void prf_save_nvram(xProfile* prf, const fs::path &path) {
	if (prf->zx->ide->type != IDE_SMUC) return;
	saveFixedBlob(path, prf->zx->ide->smuc.nv->mem, 256);
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
	nprof->curlabset = nullptr;
	std::error_code ec;
	fs::create_directories(profileConfigDir(nprof), ec);
	migrateProfileState(nprof, ".cmos");
	migrateProfileState(nprof, ".nvram");
	prf_load_cmos(nprof,  profileStatePath(nprof, ".cmos"));
	prf_load_nvram(nprof, profileStatePath(nprof, ".nvram"));
	prfSetHardware(nprof,"Dummy");
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
	const fs::path sfname = profileConfigDir(sprf) / sprf->file;
	const fs::path dfname = profileConfigDir(dprf) / dfile;
	copyFile(sfname, dfname);
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
			const fs::path confDir  = profileConfigDir(prf);
			const fs::path stateDir = profileStateDir(prf);
			std::error_code ec;
			fs::remove(confDir / prf->file, ec);
			fs::remove(profileStatePath(prf, ".cmos"),  ec);
			fs::remove(profileStatePath(prf, ".nvram"), ec);
			fs::remove(confDir,  ec);        // remove directories (leaves them if non-empty)
			fs::remove(stateDir, ec);
			compDestroy(prf->zx);            // delete computer
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
	comp_kbd_release(nprf->zx);
	mouseReleaseAll(nprf->zx->mouse);
	//padLoadConfig(nprf->jmapName);
	conf.gpctrl->gpada->loadMap(nprf->jmapNameA);
	conf.gpctrl->gpadb->loadMap(nprf->jmapNameB);
	loadKeys();
	prfSetHardware(nprf, "");
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
	vid_set_border(prf->zx->vid, conf.brdsize);
	if ((prf->zx->vid->res.x > 0) && (prf->zx->vid->res.y > 0)) {
		vid_set_resolution(prf->zx->vid, prf->zx->vid->res.x, prf->zx->vid->res.y);
	}
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

int prfSetHardware(xProfile* prf, std::string nm) {
	// todo: if HWG_ZX, set system breakpoints for tape traps
	return compSetHardware(prf->zx, nm.empty() ? NULL : nm.c_str());
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
		flp_insert(flp, st.c_str());		// delayed loading after set hw
	}
}

// set specified romset to specified profile & load into ROM of this profile ZX
void prfSetRomset(xProfile* prf, std::string rnm) {
	if (prf == NULL)
		prf = conf.prof.cur;
	prf->rsName = rnm;
	xRomset* rset = findRomset(rnm);
	int romsz = MEM_256; // prf->zx->mem->romSize;	// 0?

	struct RomResolution {
		bool found;
		fs::path path;
	};
	auto resolveRom = [](const std::string &name) -> RomResolution {
		const auto maybe = conf.path.tryFind(ResourceKind::Rom, name);
		return {
			maybe.has_value(),
			maybe.value_or(conf.path.writableDir(ResourceKind::Rom) / name)
		};
	};

	if (rset) {
		memset(prf->zx->mem->romData, 0xff, MEM_512K);
		foreach(xRomFile xrf, rset->roms) {
			const int foff = xrf.foffset * 1024;
			const int roff = xrf.roffset * 1024;
			const auto r = resolveRom(xrf.name);
			std::ifstream file(r.path, std::ios::binary);
			if (file) {
				std::error_code ec;
				int fsze = (xrf.fsize <= 0)
					? static_cast<int>(fs::file_size(r.path, ec))
					: xrf.fsize * 1024;
				if (roff + fsze > romsz) {	// check crossing rom top
					romsz = toLimits(roff + fsze, MEM_256, MEM_512K);
					romsz = toPower(romsz);
				}
				if (roff + fsze > romsz)	// check again (if 512K limit)
					fsze = romsz - roff;
				if ((foff >= 0) && (roff >= 0) && (roff < MEM_512K) && (fsze > 0)) {	// load rom if all is ok
					file.seekg(foff);
					file.read(reinterpret_cast<char*>(prf->zx->mem->romData + roff), fsze);
				}
			} else if (!r.found) {
				printf("Can't find rom '%s' in any search path\n", xrf.name.c_str());
			} else {
				std::cout << "Can't load rom file " << r.path << std::endl;
			}
		}
		memSetSize(prf->zx->mem, -1, romsz);
// load GS ROM
		if (!rset->gsFile.empty()) {
			const auto r = resolveRom(rset->gsFile);
			if (!loadFixedBlob(r.path, prf->zx->gs->mem->romData, MEM_32K)) {
				if (!r.found) {
					printf("Can't find gs rom '%s' in any search path (profile %s)\n",
					       rset->gsFile.c_str(), prf->name.c_str());
				} else {
					std::cout << "Can't load gs rom " << r.path
					          << " (profile " << prf->name << ")" << std::endl;
				}
				memset((char*)prf->zx->gs->mem->romData, 0xff, MEM_32K);
			}
		}
// load font data
		if (!rset->fntFile.empty()) {
			const auto r = resolveRom(rset->fntFile);
			vid_fnt_load(prf->zx->vid, r.path.string().c_str());
			if (!r.found) {
				printf("Can't find font '%s' in any search path\n", rset->fntFile.c_str());
			}
		} else {
			vid_fnt_del(prf->zx->vid);
		}
// load ega/vga bios (64K max)
		memset(prf->zx->vid->bios, 0xff, MEM_64K);
		prf->zx->vid->vga.cga = 1;
		if (!rset->vBiosFile.empty()) {
			const auto r = resolveRom(rset->vBiosFile);
			if (loadFixedBlob(r.path, prf->zx->vid->bios, MEM_64K)) {
				prf->zx->vid->vga.cga = 0;
			} else if (!r.found) {
				printf("Can't find VGA bios '%s' in any search path\n",
				       rset->vBiosFile.c_str());
			}
		}
	}
}

int prf_load_conf(xProfile* prf, const fs::path &cfname, int flag) {
	Computer* comp = prf->zx;
	Floppy* flp;
	int i;
	std::ifstream file(cfname);
	std::pair<std::string,std::string> spl, pspl;
	std::string line,pnam,pval,str;
//	std::vector<std::string> vect;
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
		copyResource(":/conf/xpeccy.conf", cfname);
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
					if (pnam == "contPattern") comp->vid->ula->conttype = arg.i;
					if (pnam == "earlyTiming") comp->vid->ula->early = arg.b;
					if (pnam == "DDpal") comp->flgDDP = arg.b;
					if (pnam == "palette") prf->palette = pval;
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
					if ((pnam == "path") && conf.storePaths) tape_set_path(comp->tape, pval.c_str());
					if ((pnam == "speed") && (arg.i > 94) && (arg.i < 106)) comp->tape->speed = arg.i;
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
					if (pnam == "cpu.type") {
						pspl = splitline(pval, '@');		// NAME@LIBRARY = CPU from external lib
						if (pspl.second.empty()) {		// no @, use built-in
							cpu_set_type(comp->cpu, pval.c_str(), NULL, NULL);
						} else {
							const auto maybePath = conf.path.tryFind(
								ResourceKind::PluginCpu, pspl.second);
							if (maybePath) {
								const std::string dir = maybePath->parent_path().string();
								cpu_set_type(comp->cpu, pspl.first.c_str(),
								             dir.c_str(), pspl.second.c_str());
							} else {
								printf("Can't find cpu plugin '%s' in any search path\n",
								       pspl.second.c_str());
								cpu_set_type(comp->cpu, pspl.first.c_str(), NULL, NULL);
							}
						}
					}
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
					if (pnam == "contmem") comp->flgCNTM = arg.b;
					if (pnam == "contio") comp->flgCNTI = arg.b;
					if (pnam == "scrp.wait") comp->flgEM1 = arg.b;
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
					if (pnam == "mouse.sensitivity") comp->mouse->sensitivity = arg.d;
					if (pnam == "mouse.pctype") comp->mouse->pcmode = arg.i;
					if (pnam == "joy.extbuttons") comp->joy->extbuttons = arg.b;
					if (pnam == "kbd.scantab") comp->keyb->pcmode = arg.i;
					if (pnam == "keymap") {
						prf->kmapName = pval;
						loadKeys();
					}
					// NOTE: gamepad maps loaded in prfSetCurrent
					if (pnam == "gamepad.map") {
						prf->jmapNameA = pval;
						//conf.joy.gpad->loadMap(pval);
					}
					if (pnam == "gamepad2.map") {
						prf->jmapNameB = pval;
						//conf.joy.gpadb->loadMap(pval);
					}
					break;
				case PS_SDC:
					if (pnam == "sdcimage") sdcSetImage(comp->sdc, arg.s);
					if (pnam == "sdclock") comp->sdc->lock = arg.b;
					// if (pnam == "capacity") sdcSetCapacity(comp->sdc, arg.i);
					break;
				case PS_SLOT:
					if ((pnam == "slot.type") || (pnam == "slotA.type") || (pnam == "type"))
						comp->slot->mapType = arg.i;
					if ((pnam == "path") && conf.storePaths)
						sltSetPath(comp->slot, arg.s);
					break;
			}
		}
	}

	chip_set_type(comp->ts->chipA, chatype);
	chip_set_type(comp->ts->chipB, chbtype);
	chip_set_type(comp->ts->chipC, chctype);

	tmp2 = PLOAD_OK;

	if (!prfSetHardware(prf, prf->hwName)) {
		const std::string msg = "Profile: " + prf->name + "\nHardware was set to 'dummy'";
		shitHappens(msg.c_str());
		tmp2 = PLOAD_HW;
		prfSetHardware(prf, "Dummy");
	} else if (conf.storePaths) {			// loading files
		if (comp->tape->path) {
			load_file(comp, comp->tape->path, FG_TAPE, 0);
		}
		if (comp->slot->path) {
			load_file(comp, comp->slot->path, FH_SLOTS, 0);
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
	loadPalette(prf);

	// compReset(comp,RES_DEFAULT);

	return tmp2;
}

int prfLoad(std::string nm) {
	xProfile* prf = findProfile(nm);
	if (prf == NULL) return PLOAD_NF;
	const fs::path cfname  = profileConfigDir(prf) / prf->file;  // current: $CONFDIR/profiles/$NAME/$FILE
	const fs::path ofname  = conf.path.confDir     / prf->file;  // deep legacy: $CONFDIR/$FILE

	// Pre-subdir layout: the .conf, .cmos, and .nvram all lived flat in
	// confDir. If ofname exists, migrate .conf alongside the usual state-
	// file migration so the old flat layout gets fully cleared.
	std::error_code ec;
	int res = prf_load_conf(prf, ofname, 0);
	if (res == PLOAD_OK) {
		copyFile(ofname, cfname);
		fs::remove(ofname, ec);
	} else {
		res = prf_load_conf(prf, cfname, 1);
	}
	migrateProfileState(prf, ".cmos");
	migrateProfileState(prf, ".nvram");
	prf_load_cmos(prf,  profileStatePath(prf, ".cmos"));
	prf_load_nvram(prf, profileStatePath(prf, ".nvram"));
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

	std::error_code ec;
	fs::create_directories(profileConfigDir(prf), ec);
	fs::create_directories(profileStateDir(prf),  ec);
	const fs::path cfname = profileConfigDir(prf) / prf->file;

	prf_save_cmos(prf,  profileStatePath(prf, ".cmos"));
	prf_save_nvram(prf, profileStatePath(prf, ".nvram"));

	std::ofstream out(cfname, std::ios::binary);
	if (!out) {
		printf("Can't write settings\n");
		return PSAVE_OF;
	}
	out << std::fixed << std::setprecision(6);  // match old "%f" formatting

	// Small adapter for the one C-string field that may be null; the ternary
	// noise would otherwise pollute every line it appears in.
	auto orEmpty = [](const char *s) -> const char * { return s ? s : ""; };

	out << "[GENERAL]\n\n";
	writeKV(out, "lastdir", prf->lastDir);

	out << "\n[MACHINE]\n\n";
	writeKV(out, "current", prf->hwName);
	writeKV(out, "memory",  comp->mem->ramSize >> 10);   // bytes to KB
	if (comp->cpu->lib) {
		writeKV(out, "cpu.type", comp->cpu->core->name, '@', comp->cpu->libname);
	} else {
		writeKV(out, "cpu.type", comp->cpu->core->name);
	}
	writeKV(out, "cpu.frq",   int(comp->cpuFrq * 1e6));
	writeKV(out, "frq.mul",   comp->frqMul);
	writeKV(out, "scrp.wait", YESNO(comp->flgEM1));
	writeKV(out, "contio",    YESNO(comp->flgCNTI));
	writeKV(out, "contmem",   YESNO(comp->flgCNTM));

	out << "\n[ROMSET]\n\n";
	writeKV(out, "current", prf->rsName);
	switch (comp->resbank) {
		case RES_48:     writeKV(out, "reset", "basic48");  break;
		case RES_128:    writeKV(out, "reset", "basic128"); break;
		case RES_DOS:    writeKV(out, "reset", "dos");      break;
		case RES_SHADOW: writeKV(out, "reset", "shadow");   break;
	}

	out << "\n[VIDEO]\n\n";
	writeKV(out, "geometry",    prf->layName);
	writeKV(out, "4t-border",   YESNO(comp->vid->brdstep & 0x06));
	writeKV(out, "ULAplus",     YESNO(comp->vid->ula->enabled));
	writeKV(out, "contPattern", comp->vid->ula->conttype);
	writeKV(out, "earlyTiming", YESNO(comp->vid->ula->early));
	writeKV(out, "DDpal",       YESNO(comp->flgDDP));
	writeKV(out, "palette",     prf->palette);

	out << "\n[SOUND]\n\n";
	writeKV(out, "chip1",          comp->ts->chipA->type);
	writeKV(out, "chip1.stereo",   comp->ts->chipA->stereo);
	writeKV(out, "chip1.frq",      comp->ts->chipA->frq);
	writeKV(out, "chip2",          comp->ts->chipB->type);
	writeKV(out, "chip2.stereo",   comp->ts->chipB->stereo);
	writeKV(out, "chip2.frq",      comp->ts->chipB->frq);
	writeKV(out, "chip3",          comp->ts->chipC->type);
	writeKV(out, "chip3.stereo",   comp->ts->chipC->stereo);
	writeKV(out, "chip3.frq",      comp->ts->chipC->frq);
	writeKV(out, "ts.type",        comp->ts->type);
	writeKV(out, "gs",             YESNO(comp->gs->enable));
	writeKV(out, "gs.reset",       YESNO(comp->gs->stereo));
	writeKV(out, "gs.stereo",      comp->gs->stereo);
	writeKV(out, "soundrive_type", comp->sdrv->type);
	writeKV(out, "saa",            YESNO(comp->saa->enabled));

	out << "\n[INPUT]\n\n";
	writeKV(out, "mouse",             YESNO(comp->mouse->enable));
	writeKV(out, "mouse.wheel",       YESNO(comp->mouse->hasWheel));
	writeKV(out, "mouse.swapButtons", YESNO(comp->mouse->swapButtons));
	writeKV(out, "mouse.sensitivity", comp->mouse->sensitivity);
	writeKV(out, "mouse.pctype",      comp->mouse->pcmode);
	writeKV(out, "joy.extbuttons",    YESNO(comp->joy->extbuttons));
	writeKV(out, "gamepad.map",       prf->jmapNameA);
	writeKV(out, "gamepad2.map",      prf->jmapNameB);
	writeKV(out, "kbd.scantab",       comp->keyb->pcmode);
	if ((prf->kmapName != "") && (prf->kmapName != "default"))
		writeKV(out, "keymap", prf->kmapName);

	out << "\n[TAPE]\n\n";
	writeKV(out, "path",  orEmpty(comp->tape->path));
	writeKV(out, "speed", comp->tape->speed);

	out << "\n[DISK]\n\n";
	writeKV(out, "type", comp->dif->type);
	writeKV(out, "A",    getDiskString(comp->dif->fdc->flop[0]));
	writeKV(out, "B",    getDiskString(comp->dif->fdc->flop[1]));
	writeKV(out, "C",    getDiskString(comp->dif->fdc->flop[2]));
	writeKV(out, "D",    getDiskString(comp->dif->fdc->flop[3]));

	out << "\n[IDE]\n\n";
	writeKV(out, "iface",        comp->ide->type);
	writeKV(out, "master.type",  comp->ide->master->type);
	writeKV(out, "master.image", orEmpty(comp->ide->master->image));
	writeKV(out, "master.lba",   YESNO(comp->ide->master->hasLBA));
	writeKV(out, "slave.type",   comp->ide->slave->type);
	writeKV(out, "slave.image",  orEmpty(comp->ide->slave->image));
	writeKV(out, "slave.lba",    YESNO(comp->ide->slave->hasLBA));

	out << "\n[SDC]\n\n";
	writeKV(out, "sdcimage", orEmpty(comp->sdc->image));
	writeKV(out, "sdclock",  YESNO(comp->sdc->lock));

	out << "\n[SLOT]\n";
	writeKV(out, "type", comp->slot->mapType);
	writeKV(out, "path", orEmpty(comp->slot->path));

	return PSAVE_OK;
}
