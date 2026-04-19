#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <string_view>

#include <QtCore>

#include "xcore.h"
#include "migrate.h"
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

namespace {

// Which XDG root a resource kind lives under. Per the XDG Base Directory spec:
// Config = settings that customize app behavior (keybindings, input maps).
// Data   = assets and large blobs (ROMs, shaders, plugins, palettes, styles).
enum class ResourceBase { Config, Data };

struct ResourceSpec {
	std::string_view subdir;       // relative to the selected base home dir
	std::string_view legacySubdir; // subdir under confDir to migrate from; "" = none
	ResourceBase base;             // Config ($XDG_CONFIG_HOME) or Data ($XDG_DATA_HOME)
};

// Pure: build a ResourceDirs value from a spec and the already-resolved XDG
// roots. Picks the Config- or Data-home pair based on spec.base. No filesystem
// access, no logging — side-effect-free, trivially testable in isolation.
ResourceDirs buildResourceDirs(const ResourceSpec &spec,
                               const fs::path &configHomeDir,
                               const fs::path &dataHomeDir,
                               const std::vector<fs::path> &xdgConfigDirBases,
                               const std::vector<fs::path> &xdgDataDirBases) {
	const bool isConfig = spec.base == ResourceBase::Config;
	const fs::path &homeDir        = isConfig ? configHomeDir     : dataHomeDir;
	const auto     &readonlyBases  = isConfig ? xdgConfigDirBases : xdgDataDirBases;
	ResourceDirs out;
	out.writable = homeDir / spec.subdir;
	out.readonly.reserve(readonlyBases.size());
	std::transform(readonlyBases.begin(), readonlyBases.end(),
	               std::back_inserter(out.readonly),
	               [&spec](const fs::path &base) { return base / spec.subdir; });
	return out;
}

// Resolved XDG roots (or their Windows-side equivalents). `conf_init` fills
// one of these per-platform; `applyPlatformRoots` consumes it to populate
// `conf.path` and run migrations. Keeps the platform-specific resolution
// separate from the shared persistence layer.
struct PlatformRoots {
	fs::path confHome;    // $XDG_CONFIG_HOME/samstyle/xpeccy (or Windows confdir)
	fs::path dataHome;    // $XDG_DATA_HOME/samstyle/xpeccy   (or confHome on Windows)
	fs::path cacheHome;   // $XDG_CACHE_HOME/samstyle/xpeccy  (or confHome/cache)
	fs::path stateHome;   // $XDG_STATE_HOME/samstyle/xpeccy  (or confHome/state)
	std::vector<fs::path> configDirs;  // $XDG_CONFIG_DIRS bases + suffix (empty on Windows)
	std::vector<fs::path> dataDirs;    // $XDG_DATA_DIRS bases + suffix   (empty on Windows)
};

void makeDir(const fs::path &p, std::string_view label) {
	std::error_code ec;
	fs::create_directories(p, ec);
	if (ec) {
		std::cout << "create " << label << " failed: " << ec.message() << std::endl;
	}
}

// Populate conf.path.* from `roots`, create every directory, and run one-shot
// legacy migrations from confDir into per-kind subdirs. Idempotent on re-run
// — migrations short-circuit once targets exist.
void applyPlatformRoots(const PlatformRoots &roots) {
	auto &p = conf.path;
	p.confDir     = roots.confHome;
	p.prfDir      = p.confDir / "profiles";
	p.confFile    = p.confDir / "config.conf";
	p.cacheDir    = roots.cacheHome;
	p.stateDir    = roots.stateHome;
	p.prfStateDir = p.stateDir / "profiles";
	makeDir(p.confDir,     "confDir");
	makeDir(roots.dataHome,"dataHomeDir");
	makeDir(p.prfDir,      "prfDir");
	makeDir(p.cacheDir,    "cacheDir");
	makeDir(p.stateDir,    "stateDir");
	makeDir(p.prfStateDir, "prfStateDir");

	// Per-kind setup: build the search set, migrate any legacy subdir, mkdir
	// the writable target. `label` doubles as the `subdir` for diagnostics
	// (identical in practice) and as the "what" argument for migrateDir.
	auto initKind = [&](ResourceDirs &out, const ResourceSpec &spec) {
		out = buildResourceDirs(spec, roots.confHome, roots.dataHome,
		                        roots.configDirs, roots.dataDirs);
		if (!spec.legacySubdir.empty()) {
			migrate::migrateDir(p.confDir / spec.legacySubdir, out.writable,
			                    spec.subdir);
		}
		makeDir(out.writable, spec.subdir);
	};
	initKind(p.rom,       {"roms",        "roms",        ResourceBase::Data});
	initKind(p.shader,    {"shaders",     "shaders",     ResourceBase::Data});
	initKind(p.palette,   {"palettes",    "palettes",    ResourceBase::Data});
	initKind(p.pluginCpu, {"plugins/cpu", "plugins/cpu", ResourceBase::Data});
	initKind(p.style,     {"styles",      "styles",      ResourceBase::Data});
	initKind(p.keymap,    {"keymaps",     "",            ResourceBase::Config});
	initKind(p.gamepad,   {"gamepads",    "",            ResourceBase::Config});
	initKind(p.boot,      {"boot",        "",            ResourceBase::Data});

	// Pull pre-subdir flat files out of confDir into their per-kind subdirs.
	// Historically *.map keymaps and *.pad gamepad maps lived flat in confDir;
	// boot.$B and debuga.layout were also confDir-rooted. No-op on Windows
	// (source equals destination) and on any run after the first.
	migrate::migrateFilesByExtension(p.confDir, p.keymap.writable,  ".map", "keymaps");
	migrate::migrateFilesByExtension(p.confDir, p.gamepad.writable, ".pad", "gamepads");
	migrate::migrateSingleFile(p.confDir / "boot.$B",
	                           p.boot.writable / "boot.$B", "boot");
	migrate::migrateSingleFile(p.confDir  / "debuga.layout",
	                           p.cacheDir / "debuga.layout", "debuga.layout");
}

} // namespace

void conf_init(char* wpath, char* confdir) {
	// Default screenshot output to the user's Pictures directory per the
	// xdg-user-dirs spec. On Windows (where xdgpp's BaseDirectories throws
	// because $HOME is unset) or whenever anything in that chain fails, fall
	// back to $ENVHOME — which is HOMEPATH on Windows, HOME elsewhere. Users
	// can override via config.conf's `scrDir` — that wins on subsequent loads.
	try {
		conf.scrShot.dir = xdg::PicturesDir().string();
	} catch (const std::exception &) {
		conf.scrShot.dir = std::string(getenv(ENVHOME));
	}
	conf.port = 30000;

	PlatformRoots roots;
#if defined(__linux) || defined(__APPLE__) || defined(__BSD)
	const fs::path dataDirsSuffix = "samstyle/xpeccy";
	const char *home = getenv(ENVHOME);

	// Wrap an xdgpp single-path getter: append dataDirsSuffix on success, fall
	// back to a precomposed path on exception.
	auto xdgPathOrFallback = [&dataDirsSuffix](auto &&getter,
	                                           const fs::path &fallback,
	                                           std::string_view label) -> fs::path {
		try { return getter() / dataDirsSuffix; }
		catch (const std::exception &e) {
			std::cout << label << ": " << e.what() << std::endl;
			return fallback;
		}
	};
	// Wrap an xdgpp dir-list getter: each element gets dataDirsSuffix appended;
	// returns an empty vector on exception.
	auto xdgDirsOrEmpty = [&dataDirsSuffix](auto &&getter,
	                                        std::string_view label) -> std::vector<fs::path> {
		std::vector<fs::path> out;
		try {
			for (auto &d : getter()) out.push_back(d / dataDirsSuffix);
		} catch (const std::exception &e) {
			std::cout << label << ": " << e.what() << std::endl;
		}
		return out;
	};

	if (confdir == NULL) {
		roots.confHome = xdgPathOrFallback(xdg::ConfigHomeDir,
			(home ? fs::path(home) : fs::path(".")) / ".config" / dataDirsSuffix,
			"xdg config home");
		// Pre-XDG-aware migration: if XDG_CONFIG_HOME points somewhere other
		// than ~/.config, move the old ~/.config/samstyle/xpeccy across. Must
		// run before applyPlatformRoots creates the new confDir — otherwise
		// migrateDir sees the destination already present and short-circuits.
		if (home) {
			migrate::migrateDir(fs::path(home) / ".config" / dataDirsSuffix,
			                    roots.confHome, "confDir");
		}
	} else {
		roots.confHome = confdir;
	}
	roots.dataHome   = xdgPathOrFallback(xdg::DataHomeDir,  roots.confHome,            "xdg data home");
	roots.cacheHome  = xdgPathOrFallback(xdg::CacheHomeDir, roots.confHome / "cache",  "xdg cache home");
	roots.stateHome  = xdgPathOrFallback(xdg::StateHomeDir, roots.confHome / "state",  "xdg state home");
	roots.configDirs = xdgDirsOrEmpty(xdg::ConfigDirs, "xdg config dirs");
	roots.dataDirs   = xdgDirsOrEmpty(xdg::DataDirs,   "xdg data dirs");
#elif defined(__WIN32)
	roots.confHome = (confdir == NULL)
		? fs::path(wpath).parent_path() / "config"
		: fs::path(confdir);
	// No XDG equivalents on Windows; everything roots at confHome. The
	// readonly dir vectors stay default-constructed (empty), making every
	// resource's `readonly` list empty and the one-shot migrations no-ops
	// (source paths equal destinations).
	roots.dataHome  = roots.confHome;
	roots.cacheHome = roots.confHome / "cache";
	roots.stateHome = roots.confHome / "state";
#endif
	applyPlatformRoots(roots);
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
	conf.gpctrl = new xGamepadController;
//	conf.joy.gpad = new xGamepad; qDebug() << conf.joy.gpad;
//	conf.joy.gpadb = new xGamepad; qDebug() << conf.joy.gpadb;
	addProfile("default","xpeccy.conf");
}

void saveConfig() {
	std::ofstream out(conf.path.confFile, std::ios::binary);
	if (!out) {
		shitHappens("Can't write main config");
		throw(0);
	}
	out << std::fixed << std::setprecision(6);  // match old "%f" formatting

	out << "[GENERAL]\n\n";
	writeKV(out, "startdefault", YESNO(conf.defProfile));
	writeKV(out, "savepaths",    YESNO(conf.storePaths));
	writeKV(out, "fdcturbo",     YESNO(fdcFlag & FDC_FAST));
	writeKV(out, "addboot",      YESNO(conf.boot));
	writeKV(out, "exit.confirm", YESNO(conf.confexit));
	writeKV(out, "port",          conf.port);
	writeKV(out, "winpos",        conf.xpos, ',', conf.ypos);
	writeKV(out, "flpinterleave", flp_get_interleave());
	writeKV(out, "style",         conf.style);

	out << "\n[BOOKMARKS]\n\n";
	foreach(xBookmark bkm, conf.bookmarkList) {
		writeKV(out, bkm.name, bkm.path);
	}

	out << "\n[PROFILES]\n\n";
	foreach(xProfile* prf, conf.prof.list) {
		if (prf->name != "default")
			writeKV(out, prf->name, prf->file);
	}
	writeKV(out, "current", conf.prof.cur->name);

	out << "\n[VIDEO]\n\n";
	foreach(xLayout lay, conf.layList) {
		if (lay.name != "default") {
			writeKV(out, "layout",
			        lay.name,
			        ':', lay.lay.full.x,  ':', lay.lay.full.y,
			        ':', lay.lay.bord.x,  ':', lay.lay.bord.y,
			        ':', lay.lay.blank.x, ':', lay.lay.blank.y,
			        ':', lay.lay.intSize,
			        ':', lay.lay.intpos.y, ':', lay.lay.intpos.x,
			        ':', lay.lay.scr.x,    ':', lay.lay.scr.y);
		}
	}
	writeKV(out, "scrDir",        conf.scrShot.dir);
	writeKV(out, "scrFormat",     conf.scrShot.format);
	writeKV(out, "scrCount",      conf.scrShot.count);
	writeKV(out, "scrInterval",   conf.scrShot.interval);
	writeKV(out, "scrNoLeds",     YESNO(conf.scrShot.noLeds));
	writeKV(out, "scrNoBord",     YESNO(conf.scrShot.noBorder));
	writeKV(out, "fullscreen",    YESNO(conf.vid.fullScreen));
	writeKV(out, "keepratio",     YESNO(conf.vid.keepRatio));
	writeKV(out, "scale",         conf.vid.scale);
	writeKV(out, "greyscale",     YESNO(greyScale));
	writeKV(out, "bordersize",    int(conf.brdsize * 100));
	writeKV(out, "noflick",       noflic);
	writeKV(out, "noflick.mode",  noflicMode);
	writeKV(out, "noflick.gamma", noflicGamma);
	writeKV(out, "shader",        conf.vid.shader);

	out << "\n[ROMSETS]\n";
	foreach(xRomset rms, conf.rsList) {
		out << "\n";
		writeKV(out, "name", rms.name);
		foreach(xRomFile rf, rms.roms) {
			writeKV(out, "rom",
			        rf.name,
			        ':', rf.foffset,
			        ':', rf.fsize,
			        ':', rf.roffset);
		}
		if (!rms.gsFile.empty())    writeKV(out, "gs",   rms.gsFile);
		if (!rms.fntFile.empty())   writeKV(out, "font", rms.fntFile);
		if (!rms.vBiosFile.empty()) writeKV(out, "vga",  rms.vBiosFile);
	}

	out << "\n[SOUND]\n\n";
	writeKV(out, "enabled",       YESNO(conf.snd.enabled));
	writeKV(out, "soundsys",      sndOutput->name);
	writeKV(out, "rate",          conf.snd.rate);
	writeKV(out, "volume.master", conf.snd.vol.master);
	writeKV(out, "volume.beep",   conf.snd.vol.beep);
	writeKV(out, "volume.tape",   conf.snd.vol.tape);
	writeKV(out, "volume.ay",     conf.snd.vol.ay);
	writeKV(out, "volume.gs",     conf.snd.vol.gs);
	writeKV(out, "volume.sdrv",   conf.snd.vol.sdrv);
	writeKV(out, "volume.saa",    conf.snd.vol.saa);

	out << "\n[TAPE]\n\n";
	writeKV(out, "autoplay", YESNO(conf.tape.autostart));
	writeKV(out, "fast",     YESNO(conf.tape.fast));

	out << "\n[INPUT]\n\n";
	writeKV(out, "gamepad",   conf.gpctrl->gpada->lastName());
	writeKV(out, "deadzone",  conf.gpctrl->gpada->deadZone());
	writeKV(out, "gamepad2",  conf.gpctrl->gpadb->lastName());
	writeKV(out, "deadzone2", conf.gpctrl->gpadb->deadZone());

	out << "\n[LEDS]\n\n";
	writeKV(out, "mouse",    YESNO(conf.led.mouse));
	writeKV(out, "joystick", YESNO(conf.led.joy));
	writeKV(out, "keyscan",  YESNO(conf.led.keys));
	writeKV(out, "tape",     YESNO(conf.led.tape));
	writeKV(out, "disk",     YESNO(conf.led.disk));
	writeKV(out, "message",  YESNO(conf.led.message));
	writeKV(out, "fps",      YESNO(conf.led.fps));
	writeKV(out, "halt",     YESNO(conf.led.halt));

	out << "\n[DEBUGA]\n\n";
	writeKV(out, "dbsize",   conf.dbg.dbsize);
	writeKV(out, "dwsize",   conf.dbg.dwsize);
	writeKV(out, "dmsize",   conf.dbg.dmsize);
	writeKV(out, "scr.zoom", conf.dbg.scrzoom);
	writeKV(out, "font",     conf.dbg.font.toString());
	writeKV(out, "window",
	        conf.dbg.pos.x(),   ':', conf.dbg.pos.y(),
	        ':', conf.dbg.siz.width(), ':', conf.dbg.siz.height());

	out << "\n[PALETTE]\n\n";
	foreach(QString nam, conf.pal.keys()) {
		const QColor col = conf.pal[nam];
		if (col.isValid())
			writeKV(out, nam.toUtf8().constData(), col.name());
	}

	out << "\n[KEYS]\n\n";
	xShortcut* tab = shortcut_tab();
	for (int i = 0; tab[i].id > 0; i++) {
		writeKV(out, tab[i].name, tab[i].seq.toString());
	}
}

void copyFile(const fs::path &src, const fs::path &dst) {
	std::error_code ec;
	fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
	if (ec) {
		std::cout << "copyFile: " << src << " -> " << dst
		          << " failed: " << ec.message() << std::endl;
	}
}

void copyResource(std::string_view src, const fs::path &dst) {
	QFile in(QString::fromUtf8(src.data(), static_cast<int>(src.size())));
	if (!in.open(QIODevice::ReadOnly)) {
		std::cout << "copyResource: can't read " << src << std::endl;
		return;
	}
	const QByteArray data = in.readAll();
	in.close();
	std::ofstream out(dst, std::ios::binary);
	if (!out) {
		std::cout << "copyResource: can't write " << dst << std::endl;
		return;
	}
	out.write(data.constData(), data.size());
}

// emulator config

void loadConfig() {
	std::string soutnam = "NULL";
	std::ifstream file(conf.path.confFile);
	if (!file.good()) {
		printf("Main config is missing. Default files will be copied\n");
		copyResource(":/conf/config.conf", conf.path.confFile);
		copyResource(":/conf/xpeccy.conf", conf.path.confDir / "xpeccy.conf");
		// Only seed the default ROM if no system-wide copy already resolves —
		// otherwise we'd permanently shadow a distro-shipped 1982.rom with a
		// private copy that never gets updated.
		if (!conf.path.rom.tryFind("1982.rom")) {
			copyResource(":/conf/1982.rom",
			             conf.path.rom.writable / "1982.rom");
		}
		file.open(conf.path.confFile);
		if (!file.good()) {
			std::cout << conf.path.confFile << std::endl;
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
	conf.dbg.scrzoom = 1;
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

	xArg arg;

	while (!file.eof()) {
		file.getline(buf,2048);
		line = std::string(buf);
		spl = splitline(line);
		pnam = spl.first;
		pval = spl.second;
		arg.b = str2bool(pval);
		arg.s = pval.c_str();
		arg.i = strtol(arg.s, NULL, 0);
		arg.d = strtod(arg.s, NULL);
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
					set_shortcut_name(pnam.c_str(), QKeySequence(arg.s));
					break;
				case SECT_PALETTE:
					col = QColor(arg.s);
					if (col.isValid())
						conf.pal[QString(pnam.c_str())] = col;
					break;
				case SECT_DEBUGA:
					if (pnam == "dbsize") conf.dbg.dbsize = arg.i;
					if (pnam == "dwsize") conf.dbg.dwsize = arg.i;
					if (pnam == "dmsize") conf.dbg.dmsize = arg.i;
					if (pnam == "font") conf.dbg.font.fromString(QString(arg.s));
					if (pnam == "window") {
						vect = splitstr(pval,":");
						if (vect.size() < 4) {
							vect.insert(vect.begin(), "0");
							vect.insert(vect.begin(), "0");
						}
						if (vect.size() > 3) {
							fprt = atoi(vect[0].c_str()); if (fprt >= 0) conf.dbg.pos.setX(fprt);
							fprt = atoi(vect[1].c_str()); if (fprt >= 0) conf.dbg.pos.setY(fprt);
							fprt = atoi(vect[2].c_str()); if (fprt > 0) conf.dbg.siz.setWidth(fprt);
							fprt = atoi(vect[3].c_str()); if (fprt > 0) conf.dbg.siz.setHeight(fprt);
						}
					}
					if ((pnam == "scr.zoom") && (arg.i > 0) && (arg.i < 4))
						conf.dbg.scrzoom = arg.i;
					break;
				case SECT_BOOKMARK:
					addBookmark(pnam, pval);
					break;
				case SECT_PROFILES:
					if (pnam == "current") {
						pnm = pval;
					} else {
						addProfile(pnam, pval);
					}
					break;
				case SECT_INPUT:
					if (pnam=="deadzone") conf.gpctrl->gpada->setDeadZone(arg.i);
					if (pnam=="deadzone2") conf.gpctrl->gpadb->setDeadZone(arg.i);
					if (pnam=="gamepad") conf.gpctrl->gpada->setName(arg.s);
					if (pnam=="gamepad2") conf.gpctrl->gpadb->setName(arg.s);
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
					if (pnam=="scrCount") conf.scrShot.count = arg.i;
					if (pnam=="scrInterval") conf.scrShot.interval = arg.i;
					if (pnam=="scrNoLeds") conf.scrShot.noLeds = arg.b;
					if (pnam=="scrNoBord") conf.scrShot.noBorder = arg.b;
					if (pnam=="fullscreen") conf.vid.fullScreen = arg.b;
					if (pnam=="keepratio") conf.vid.keepRatio = arg.b;
					if (pnam=="bordersize") conf.brdsize = getRanged(arg.s, 0, 100) / 100.0;
					if (pnam=="doublesize") conf.vid.scale = arg.b ? 2 : 1;
					if (pnam=="scale") {
						conf.vid.scale = arg.i;
						if (conf.vid.scale < 1) conf.vid.scale = 1;
						if (conf.vid.scale > 6) conf.vid.scale = 6;
					}
					if (pnam=="noflic") noflic = arg.b ? 50 : 25;		// old parameter
					if (pnam=="noflick") noflic = getRanged(arg.s, 0, 50);	// new parameter
					if (pnam=="noflick.mode") noflicMode = arg.i;
					if (pnam=="noflick.gamma") {
						noflicGamma = arg.d;
						if (noflicGamma < 1) noflicGamma = 1;
						if (noflicGamma > 3) noflicGamma = 3;
					}
					if (pnam=="greyscale") vid_set_grey(arg.b);
//					if (pnam=="scanlines") scanlines = arg.b;
					if (pnam=="shader") conf.vid.shader = pval;
					// if (pnam=="palette") conf.vid.palette = pval;
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
					if (pnam=="enabled") conf.snd.enabled = arg.b;
					if (pnam=="soundsys") soutnam = pval;
					if (pnam=="rate") conf.snd.rate = arg.i;
					if (pnam=="volume.master") conf.snd.vol.master = getRanged(arg.s, 0, 100);
					if (pnam=="volume.beep") conf.snd.vol.beep = getRanged(arg.s, 0, 100);
					if (pnam=="volume.tape") conf.snd.vol.tape = getRanged(arg.s, 0, 100);
					if (pnam=="volume.ay") conf.snd.vol.ay = getRanged(arg.s, 0, 100);
					if (pnam=="volume.gs") conf.snd.vol.gs = getRanged(arg.s, 0, 100);
					if (pnam=="volume.sdrv") conf.snd.vol.sdrv = getRanged(arg.s, 0, 100);
					if (pnam=="volume.saa") conf.snd.vol.saa = getRanged(arg.s, 0, 100);
					break;
				case SECT_TOOLS:
					break;
				case SECT_GENERAL:
					if (pnam=="startdefault") conf.defProfile = arg.b;
					if (pnam=="savepaths") conf.storePaths = arg.b;
					if (pnam == "fdcturbo") setFlagBit(arg.b, &fdcFlag, FDC_FAST);
					if (pnam == "port") conf.port = arg.i & 0xffff;
					if (pnam == "winpos") {
						vect = splitstr(pval, ",");
						if (vect.size() > 1) {
							conf.xpos = strtol(vect[0].c_str(), NULL, 10);
							conf.ypos = strtol(vect[1].c_str(), NULL, 10);
						}
					}
					if (pnam == "addboot") conf.boot = arg.b;
					if (pnam == "exit.confirm") conf.confexit = arg.b;
					if (pnam == "flpinterleave") flp_set_interleave(arg.i);
					if (pnam == "style") conf.style = std::string(arg.s);
					break;
				case SECT_TAPE:
					if (pnam=="autoplay") conf.tape.autostart = arg.b;
					if (pnam=="fast") conf.tape.fast = arg.b;
					break;
				case SECT_LEDS:
					if (pnam=="mouse") conf.led.mouse = arg.b;
					if (pnam=="joystick") conf.led.joy = arg.b;
					if (pnam=="keyscan") conf.led.keys = arg.b;
					if (pnam=="tape") conf.led.tape = arg.b;
					if (pnam=="disk") conf.led.disk = arg.b;
					if (pnam=="message") conf.led.message = arg.b;
					if (pnam=="fps") conf.led.fps = arg.b;
					if (pnam=="halt") conf.led.halt = arg.b;
					break;
			}
		}
	}
	conf.gpctrl->gpada->open();
	conf.gpctrl->gpadb->open();
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
			printf("Can't set profile '%s', default will be used\n",pnm.c_str());
			if (!prfSetCurrent("default")) {
				printf("...and default too? Really, shit happens\n");
				throw(0);
			}
		}
	}
}
