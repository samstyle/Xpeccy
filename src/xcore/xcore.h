#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <initializer_list>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <filesystem>

#if defined(__linux) || defined(__BSD)
#include <linux/limits.h>
#endif

#include <SDL_joystick.h>

#include <xdg.hpp>

#include <QDebug>
#include <QKeySequence>
#include <QString>
#include <QPoint>
#include <QColor>
#include <QFont>
#include <QSize>
#include <QMap>

#include "../libxpeccy/spectrum.h"
#include "../libxpeccy/filetypes/filetypes.h"
#include "gamepad.h"

#ifndef USEMUTEX
#define USEMUTEX 0
#endif

#define NEW_SMP_METHOD 1
// init: smpNeed = 0
// each sdl_sound_callback smpNeed += (samples needed = bytes/4)
// each sample in buffer: smpNeed--
// if (smpNeed == 0) conf.snd.fill = 0 (end of emulation cycle)
// after emulation cycle: wait for smpNeed!=0 (instead of sleepy)

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
	#define yDelta angleDelta().y()
	#define xEventX position().x()
	#define xEventY position().y()
	#define xGlobalX globalPosition().x()
	#define xGlobalY globalPosition().y()
#else
	#define yDelta angleDelta().y()
	#define xEventX x()
	#define xEventY y()
	#define xGlobalX globalX()
	#define xGlobalY globalY()
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
	#define	X_SkipEmptyParts Qt::SkipEmptyParts
	#define X_KeepEmptyParts Qt::KeepEmptyParts
	#include <QScreen>
	#define SCREENSIZE screen()->size()
#else
	#define X_SkipEmptyParts QString::SkipEmptyParts
	#define X_KeepEmptyParts QString::KeepEmptyParts
	#include <QDesktopWidget>
	#define SCREENSIZE QApplication::desktop()->screenGeometry().size()
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
	#include <QtCore5Compat>
	#include <QSurfaceFormat>
	#define X_BackgroundRole Qt::BackgroundRole
	#define X_MidButton Qt::MiddleButton
#else
	#include <QRegExp>
	#define X_BackgroundRole Qt::BackgroundColorRole
	#define X_MidButton Qt::MidButton
#endif

namespace fs = std::filesystem;

// Categorizes shippable resource files that may live both in a user-writable
// directory and in one or more system data directories.
enum class ResourceKind {
	Rom,
	Shader,
	Palette,
	PluginCpu,
	Style,
	Keymap,
	Gamepad,
	COUNT
};

struct ResourceDirs {
	fs::path writable;                 // User-writable dir (under dataHomeDir or confDir on Windows)
	std::vector<fs::path> readonly;    // System dirs from $XDG_DATA_DIRS (empty on Windows)
};

// Which side of the search path an enumerated entry came from. Named so call
// sites don't have to interpret a bare bool.
enum class ResourceOrigin { User, System };

struct ResolvedEntry {
	fs::path path;           // full path on disk
	fs::path name;           // basename (or relative path for enumerateRecursive)
	ResourceOrigin origin;   // User (writable dir) or System (readonly dir)
};

// Small adapter so callers don't have to write QString::fromStdString(p.string())
// at every Qt boundary. fs::path::string() returns UTF-8 on all supported
// platforms, matching QString::fromStdString's input expectation.
inline QString toQString(const fs::path &p) {
	return QString::fromStdString(p.string());
}
// Overload for std::string so every Qt-string conversion in the xdg pipeline
// funnels through one helper instead of mixing fromStdString and toQString.
inline QString toQString(const std::string &s) {
	return QString::fromStdString(s);
}

// QString has no std::ostream inserter by default; add one so writeKV and
// direct `out << qstring` work uniformly. UTF-8 is used for consistency with
// fs::path::string() and toQString, which both use UTF-8.
inline std::ostream& operator<<(std::ostream &out, const QString &s) {
	return out << s.toUtf8().constData();
}

// Helper for writing INI-style "key = value\n" entries to an ostream. Factors
// out the repeating pattern in saveConfig/prfSave where dozens of keys are
// dumped one per line. Variadic: any number of value arguments are emitted
// left-to-right (via a C++17 fold expression) between the "key = " prefix
// and the trailing newline. Stream flags (e.g. std::fixed/setprecision that
// the caller already set for %f-style formatting) are preserved because we
// write directly into the destination stream rather than an intermediate
// buffer. Works for anything with an operator<<(ostream, T), including
// const char*, std::string, int, char, YESNO() macro, fs::path, QString.
template <typename... Args>
inline void writeKV(std::ostream &out, std::string_view key, const Args&... values) {
	out << key << " = ";
	(out << ... << values);
	out << '\n';
}

// Predicate factory: returns a callable that matches a path if its extension
// is any of the given ones (leading dot included, e.g. ".txt"). Matching is
// case-insensitive so that e.g. "foo.ROM" matches ".rom". Captures the
// normalized extension list by value so the returned predicate owns its data.
inline auto byExtension(std::initializer_list<const char *> exts) {
	auto toLower = [](std::string s) {
		std::transform(s.begin(), s.end(), s.begin(),
		               [](unsigned char c) { return std::tolower(c); });
		return s;
	};
	std::vector<std::string> owned;
	owned.reserve(exts.size());
	for (const char *e : exts) owned.push_back(toLower(std::string{e}));
	return [owned = std::move(owned), toLower](const fs::path &p) {
		if (owned.empty()) return true;
		const auto ext = toLower(p.extension().string());
		return std::any_of(owned.begin(), owned.end(),
		                   [&](const std::string &e) { return ext == e; });
	};
}

// common

std::string getTimeString(int);
std::string int2str(int);
std::string float2str(float);
int getRanged(const char*, int, int);
void setFlagBit(bool, int*, int);
bool str2bool(std::string);
std::vector<std::string> splitstr(std::string,const char*);
std::pair<std::string,std::string> splitline(std::string, char = '=');
void copyFile(const fs::path &src, const fs::path &dst);
void copyResource(std::string_view src, const fs::path &dst);

int toPower(int);
int toLimits(int, int, int);
double absd(double);

QString getbinbyte(unsigned char);
QString gethexshift(char);
QString getdecshift(char);
QString gethexbyte(int);
QString gethexword(int);
QString getoctword(int);
QString gethex6(int);
QString gethexint(int);

typedef struct {
	int err;
	int value;
	const char* ptr;
} xResult;

xResult xEval(const char*, int = 0);

typedef struct {
	unsigned b:1;
	int i;
	double d;
	const char* s;
} xArg;

// pause reasons
#define	PR_MENU		1
#define	PR_FILE		(1<<1)
#define	PR_OPTS		(1<<2)
#define	PR_DEBUG	(1<<3)
#define	PR_QUIT		(1<<4)
#define	PR_PAUSE	(1<<5)
#define	PR_EXTRA	(1<<6)
#define PR_RZX		(1<<7)
#define	PR_EXIT		(1<<8)

// labels

typedef struct {
	QString name;
	QMap<QString, xAdr> list;
} xLabelSet;

xLabelSet* newLabelSet(QString);
int delLabelSet(QString);
xLabelSet* setLabelSet(QString);

void add_label(xAdr, QString, xLabelSet* = nullptr);
void del_label(QString);
QString find_label(xAdr);
xAdr find_label(QString);
void clear_labels();
void clear_all_labels();

int loadLabels(const char*);
int saveLabels(const char*);

// comments

void add_comment(xAdr, QString);
void del_comment(xAdr);
QString find_comment(xAdr);
void clear_comments();

// brk points

#define DELBREAKS 0		// delete breakpoint on FRW=000

#define BRKF_SYSTEM 1

enum {
	BRK_ACT_DBG = 1,
	BRK_ACT_SCR,
	BRK_ACT_COUNT,
};

typedef struct {
	unsigned off:1;
	unsigned fetch:1;
	unsigned read:1;
	unsigned write:1;
	unsigned temp:1;
	int type;
	int adr;	// start adr (mem), port(io)
	int eadr;	// end adr
	int mask;	// io: if (port & mask == adr & mask)
	int count;
	int action;	// what to do
} xBrkPoint;

void brkSet(int, int, int, int);
void brkXor(int, int, int, int, int);
void brkAdd(xBrkPoint, int = 0);
// void brkInstall(xBrkPoint*, int);
void brkDelete(xBrkPoint);
void brkInstallAll();
void brk_clear_tmp(Computer*);
xBrkPoint* brk_find(int, int);

// profiles

typedef struct {
	std::string name;
 	std::string file;		// config file
	std::string layName;		// screen layout
	std::string hwName;		// hardware
	std::string rsName;		// romset
	std::string jmapNameA;		// joysticks
	std::string jmapNameB;
	std::string kmapName;		// keymap
	std::string lastDir;
	std::string palette;
	struct {
		std::vector<xBrkPoint> list;
		std::vector<xBrkPoint> list_sys;
		std::map<int, std::map<int, xBrkPoint*> > map;		// [memtype][addr] = pointer
	} brk;
	Computer* zx;
	QMap<int, QMap<int, QString> > commap;	// comments: [memtype][addr] = string
	QMap<int, QMap<int, QString> > labmap;	// [memtype][addr] = name
	QList<xLabelSet*> labsets;
	xLabelSet* curlabset;			// curlabset->list = labels
//	QMap<QString,xAdr> labels;		// name->xAdr
} xProfile;

#define	DELP_ERR	-1
#define	DELP_OK		0
#define	DELP_OK_CURR	1

xProfile* findProfile(std::string);
xProfile* addProfile(std::string,std::string);
int delProfile(std::string);
int copyProfile(std::string, std::string);
void clearProfiles();
void prfLoadAll();
bool prfSetCurrent(std::string);
void prfSetRomset(xProfile*, std::string);
bool prfSetLayout(xProfile*, std::string);
int prfSetHardware(xProfile*, std::string);

void prfChangeRsName(std::string, std::string);
void prfChangeLayName(std::string, std::string);

//void prfFillBreakpoints(xProfile*);

#define	PLOAD_OK	0
#define	PLOAD_NF	1
#define	PLOAD_OF	2
#define	PLOAD_HW	3
#define	PLOAD_RS	4

int prfLoad(std::string);

#define PSAVE_OK	PLOAD_OK
#define	PSAVE_NF	PLOAD_NF
#define	PSAVE_OF	PLOAD_OF

int prfSave(std::string = "");

//screenshot format
#define	SCR_BMP		1
#define	SCR_PNG		2
#define	SCR_JPG		3
#define	SCR_SCR		4
#define	SCR_HOB		5
#define	SCR_DISK	6

void conf_init(char*, char* confdir = NULL);
QList<QColor> loadColors(std::string);
int saveColors(std::string, QList<QColor>);
void loadPalette(xProfile*);
void loadConfig();
void saveConfig();

extern std::map<std::string, int> shotFormat;

// keymap

enum {
	XCUT_SIZEX1 = 0x10000,
	XCUT_SIZEX2,
	XCUT_SIZEX3,
	XCUT_SIZEX4,
	XCUT_SIZEX5,
	XCUT_SIZEX6,
	XCUT_FULLSCR,
	XCUT_RATIO,
	XCUT_SCRSHOT,
	XCUT_COMBOSHOT,
	XCUT_RES_DOS,
	XCUT_KEYBOARD,
	XCUT_FAST,
	XCUT_NOFLICK,
	XCUT_MOUSE,
	XCUT_GRABKBD,
	XCUT_PAUSE,
	XCUT_DEBUG,
	XCUT_MENU,
	XCUT_OPTIONS,
	XCUT_SAVE,
	XCUT_LOAD,
	XCUT_TAPLAY,
	XCUT_TAPREC,
	XCUT_TAPWIN,
	XCUT_RZXWIN,
	XCUT_FASTSAVE,
	XCUT_NMI,
	XCUT_RESET,
	XCUT_TURBO,
//	XCUT_TVLINES,
	XCUT_WAV_OUT,
	XCUT_RELOAD_SHD,

	XCUT_STEPIN,
	XCUT_STEPOVER,
	XCUT_STEPOUT,
	XCUT_FASTSTEP,
	XCUT_TMPBRK,
	XCUT_TRACE,
	XCUT_OPEN_DUMP,
	XCUT_SAVE_DUMP,
	XCUT_FINDER,
	XCUT_LABELS,
	XCUT_LABLIST,
	XCUT_DBG_RELOAD,

	XCUT_TOPC,
	XCUT_SETPC,
	XCUT_SETBRK,
	XCUT_JUMPTO,
	XCUT_RETFROM,
};

enum {
	SCG_ALL = -1,
	SCG_MAIN = (1 << 0),
	SCG_DEBUGA = (1 << 1),
	SCG_DISASM = (1 << 2)
};

void loadKeys();
void setKey(const char*, const char*);
keyEntry getKeyEntry(int);
int getKeyIdByName(const char*);
const char* getKeyNameById(int);
int qKey2id(int, Qt::KeyboardModifiers = Qt::NoModifier);
int key2qid(int);

typedef struct {
	int grp;
	int id;
	const char* name;
	const char* text;
	QKeySequence seq;
	QKeySequence def;
} xShortcut;

void shortcut_init();
xShortcut* find_shortcut_id(int);
xShortcut* find_shortcut_name(const char*);
void set_shortcut_id(int, QKeySequence);
void set_shortcut_name(const char*, QKeySequence);
xShortcut* shortcut_tab();
int shortcut_check(int, QKeySequence);
int shortcut_match(int, int, QKeySequence);

// bookmarks

typedef struct {
	std::string name;
	std::string path;
} xBookmark;

void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void swapBookmarks(int,int);

// romsets

typedef struct {
	std::string name;
	int foffset;
	int fsize;
	int roffset;
} xRomFile;

typedef struct {
	std::string name;
	std::string gsFile;
	std::string fntFile;
	std::string vBiosFile;
	QList<xRomFile> roms;
} xRomset;

xRomset* findRomset(std::string);
bool addRomset(xRomset);
void delRomset(int);

// layouts

typedef struct {
	std::string name;
	vLayout lay;
} xLayout;

bool addLayout(std::string, vLayout);
void rmLayout(std::string);
xLayout* findLayout(std::string);

// xmap

void load_xmap(QString);
void save_xmap(QString);

// config

#define	YESNO(cnd) ((cnd) ? "yes" : "no")

struct xConfig {
	unsigned running:1;
	unsigned storePaths:1;		// store tape/disk paths
	unsigned defProfile:1;		// start @ default profile
	unsigned boot:1;		// add boot to trdos floppies
	unsigned confexit:1;		// confirm on exit
	double brdsize;			// 0.0 - 1.0 : border size
	int xpos;			// window position
	int ypos;
	QList<xRomset> rsList;
	QList<xLayout> layList;
	QList<xBookmark> bookmarkList;
	QMap<QString, QColor> pal;
	QString labpath;
	std::string style;
	unsigned short port;		// port to listen
	struct {
		unsigned fast:1;
		int pause;
	} emu;
	struct {
		QList<xProfile*> list;
		xProfile* cur;
	} prof;
	struct {
		unsigned fullScreen:1;	// use fullscreen
		unsigned keepRatio:1;	// keep ratio in fullscreen (add black borders)
		int scale;		// x1..x4
		int fcount;		// frames counter (for fps showing) (= fcnt ???)
		int curfps;
		std::string shader;
		int shd_support;
	} vid;
	struct {
		unsigned enabled:1;
		unsigned wavout:1;	// output to wav, rate 44100
		unsigned fill:1;	// 1 while snd buffer not filled, 0 at end of snd buffer
		int need;		// samples needed to be filled in buf
		int rate;
		int chans;
		sndVolume vol;
		FILE* wavfile;
	} snd;
	struct {
		unsigned autostart:1;
		unsigned fast:1;
	} tape;
//	struct {
//		xGamepad* gpad;
//		xGamepad* gpadb;
//	} joy;
	xGamepadController* gpctrl;
	struct {
		unsigned noLeds:1;
		unsigned noBorder:1;
		int count;
		int interval;
		std::string format;
		std::string dir;
	} scrShot;
	struct {
		unsigned mouse:1;
		unsigned joy:1;
		unsigned keys:1;
		unsigned tape:1;
		unsigned disk:1;
		unsigned message:1;
		unsigned fps:1;
		unsigned halt:1;
	} led;
	struct {
		fs::path confDir;
		fs::path confFile;

		// Per-kind writable + read-only search paths. Populated in conf_init.
		std::array<ResourceDirs, static_cast<size_t>(ResourceKind::COUNT)> resources;

		// Index-hiding accessor: removes the static_cast noise from call sites.
		const ResourceDirs& dirsFor(ResourceKind kind) const {
			return resources[static_cast<size_t>(kind)];
		}

		const fs::path& writableDir(ResourceKind kind) const {
			return dirsFor(kind).writable;
		}

		// Search for a file by name: writable dir first, then each readonly
		// dir in order. Returns nullopt if nothing exists. This is the pure
		// primitive — use it when the caller wants to distinguish "not found
		// anywhere" from "found but unusable".
		std::optional<fs::path> tryFind(ResourceKind kind, const fs::path &name) const {
			const auto &dirs = dirsFor(kind);
			const fs::path user = dirs.writable / name;
			if (fs::exists(user)) return user;
			for (const auto &ro : dirs.readonly) {
				if (const fs::path p = ro / name; fs::exists(p)) return p;
			}
			return std::nullopt;
		}

		// Convenience wrapper: falls back to the writable-dir path so callers
		// that only care about building an error message get a sensible
		// "expected location" string for free.
		fs::path find(ResourceKind kind, const fs::path &name) const {
			return tryFind(kind, name).value_or(dirsFor(kind).writable / name);
		}

		// Higher-order enumerator: walks every dir for the kind, passing each
		// candidate path to the predicate. Entries accepted by the predicate
		// are returned, tagged as user (writable dir) or system (readonly).
		// No de-duplication: if the same basename exists in both writable and
		// readonly dirs, both entries appear. Results are sorted by name so
		// the output is deterministic.
		template <typename Pred>
		std::vector<ResolvedEntry> enumerate(ResourceKind kind, Pred &&predicate) const {
			std::vector<ResolvedEntry> out;
			std::error_code ec;
			auto addFromDir = [&](const fs::path &dir, ResourceOrigin origin) {
				if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
					ec.clear();
					return;
				}
				for (auto it = fs::directory_iterator(dir, ec);
				     !ec && it != fs::directory_iterator();
				     it.increment(ec)) {
					if (!it->is_regular_file(ec)) continue;
					if (!predicate(it->path())) continue;
					out.push_back({it->path(), it->path().filename(), origin});
				}
				ec.clear();
			};
			const auto &dirs = dirsFor(kind);
			addFromDir(dirs.writable, ResourceOrigin::User);
			for (const auto &ro : dirs.readonly) addFromDir(ro, ResourceOrigin::System);
			std::sort(out.begin(), out.end(),
			          [](const ResolvedEntry &a, const ResolvedEntry &b) {
			              return a.name < b.name;
			          });
			return out;
		}

		// Convenience overload: accept every regular file.
		std::vector<ResolvedEntry> enumerate(ResourceKind kind) const {
			return enumerate(kind, [](const fs::path &) { return true; });
		}

		// Like enumerate, but descends into subdirectories. ResolvedEntry::name
		// is the relative path from the search-root dir (e.g. "zx48/48k.rom"
		// if the rom dir contains zx48/48k.rom). Symlinks are NOT followed to
		// avoid infinite loops, and permission errors in subtrees are skipped
		// rather than thrown. Results are sorted by relative path.
		template <typename Pred>
		std::vector<ResolvedEntry> enumerateRecursive(ResourceKind kind, Pred &&predicate) const {
			std::vector<ResolvedEntry> out;
			auto addFromDir = [&](const fs::path &dir, ResourceOrigin origin) {
				std::error_code ec;
				if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;
				const auto opts = fs::directory_options::skip_permission_denied;
				fs::recursive_directory_iterator it(dir, opts, ec);
				if (ec) return;
				const fs::recursive_directory_iterator end;
				while (it != end) {
					std::error_code fec;
					if (it->is_regular_file(fec) && predicate(it->path())) {
						out.push_back({it->path(),
						               it->path().lexically_relative(dir),
						               origin});
					}
					std::error_code iec;
					it.increment(iec);
					if (iec) break;
				}
			};
			const auto &dirs = dirsFor(kind);
			addFromDir(dirs.writable, ResourceOrigin::User);
			for (const auto &ro : dirs.readonly) addFromDir(ro, ResourceOrigin::System);
			std::sort(out.begin(), out.end(),
			          [](const ResolvedEntry &a, const ResolvedEntry &b) {
			              return a.name < b.name;
			          });
			return out;
		}

		// Convenience overload: accept every regular file.
		std::vector<ResolvedEntry> enumerateRecursive(ResourceKind kind) const {
			return enumerateRecursive(kind, [](const fs::path &) { return true; });
		}

		fs::path prfDir;
		fs::path boot;
		fs::path cacheDir;   // $XDG_CACHE_HOME/samstyle/xpeccy — UI dock state and other
		                     // derived, non-essential artifacts
		fs::path stateDir;   // $XDG_STATE_HOME/samstyle/xpeccy — persistent machine-
		                     // authored state (cmos/nvram blobs)
		fs::path prfStateDir;// stateDir / profiles — per-profile state subtree
	} path;
	struct {
		unsigned labels:1;
		unsigned segment:1;
		unsigned hideadr:1;
		unsigned romwr:1;
		QFont font;
		int dbsize;
		int dwsize;
		int dmsize;
		int scrzoom;
		QPoint pos;
		QSize siz;
	} dbg;

};

extern xConfig conf;
