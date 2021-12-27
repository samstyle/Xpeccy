#pragma once

#include <string>
#include <vector>
#include <map>

#if defined(__linux) || defined(__BSD)
#include <linux/limits.h>
#endif

#include <SDL_joystick.h>

#include <QKeySequence>
#include <QString>
#include <QColor>
#include <QFont>
#include <QMap>

#include "../libxpeccy/spectrum.h"
#include "../libxpeccy/filetypes/filetypes.h"

#ifndef USEMUTEX
#define USEMUTEX 0
#endif

#define NEW_SMP_METHOD 1
// init: smpNeed = 0
// each sdl_sound_callback smpNeed += (samples needed = bytes/4)
// each sample in buffer: smpNeed--
// if (smpNeed == 0) conf.snd.fill = 0 (end of emulation cycle)
// after emulation cycle: wait for smpNeed!=0 (instead of sleepy)

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#define yDelta angleDelta().y()
#else
#define yDelta delta()
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define X_SkipEmptyParts Qt::SkipEmptyParts
#define X_KeepEmptyParts Qt::KeepEmptyParts
#else
#define X_SkipEmptyParts QString::SkipEmptyParts
#define X_KeepEmptyParts QString::KeepEmptyParts
#endif

// common

std::string getTimeString(int);
std::string int2str(int);
std::string float2str(float);
int getRanged(const char*, int, int);
void setFlagBit(bool, int*, int);
bool str2bool(std::string);
std::vector<std::string> splitstr(std::string,const char*);
std::pair<std::string,std::string> splitline(std::string);
void copyFile(const char*, const char*);

int toPower(int);
int toLimits(int, int, int);

QString getbinbyte(unsigned char);
QString gethexshift(char);
QString getdecshift(char);
QString gethexbyte(int);
QString gethexword(int);

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

void add_label(xAdr, QString);
void del_label(QString);
QString find_label(xAdr);
void clear_labels();

int loadLabels(const char*);
int saveLabels(const char*);

// brk points

#define DELBREAKS 0		// delete breakpoint on FRW=000

enum {
	BRK_UNKNOWN = 0,
	BRK_IOPORT,
	BRK_CPUADR,
	BRK_MEMCELL,
	BRK_MEMRAM,
	BRK_MEMROM,
	BRK_MEMSLT,
	BRK_MEMEXT,
	BRK_IRQ,
	BRK_HBLANK
};

typedef struct {
	unsigned off:1;
	unsigned fetch:1;
	unsigned read:1;
	unsigned write:1;
	unsigned temp:1;
	int type;
	int adr;	// (start) adr (mem)
	int eadr;	// end adr
	int mask;	// io: if (port & mask == adr & mask)
} xBrkPoint;

void brkSet(int, int, int, int);
void brkXor(int, int, int, int, int);
void brkAdd(xBrkPoint);
void brkInstall(xBrkPoint, int);
void brkDelete(xBrkPoint);
void brkInstallAll();
void brk_clear_tmp(Computer*);

// profiles

typedef struct {
	std::string name;
 	std::string file;
	std::string layName;
	std::string hwName;
	std::string rsName;
	std::string jmapName;
	std::string kmapName;
	std::string lastDir;
	std::vector<xBrkPoint> brkList;
	Computer* zx;
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

int prfSave(std::string);

//screenshot format
#define	SCR_BMP		1
#define	SCR_PNG		2
#define	SCR_JPG		3
#define	SCR_SCR		4
#define	SCR_HOB		5
#define	SCR_DISK	6

void conf_init(char*);
void loadConfig();
void saveConfig();

extern std::map<std::string, int> shotFormat;

// keymap

enum {
	XCUT_SIZEX1 = 0x10000,
	XCUT_SIZEX2,
	XCUT_SIZEX3,
	XCUT_SIZEX4,
	XCUT_FULLSCR,
	XCUT_RATIO,
	XCUT_SCRSHOT,
	XCUT_COMBOSHOT,
	XCUT_RES_DOS,
	XCUT_KEYBOARD,
	XCUT_FAST,
	XCUT_NOFLICK,
	XCUT_MOUSE,
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
	XCUT_TVLINES,
	XCUT_WAV_OUT,
	XCUT_RELOAD_SHD,

	XCUT_STEPIN,
	XCUT_STEPOVER,
	XCUT_FASTSTEP,
	XCUT_TMPBRK,
	XCUT_TRACE,
	XCUT_OPEN_DUMP,
	XCUT_SAVE_DUMP,
	XCUT_FINDER,
	XCUT_LABELS,

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

// joystick

enum {
	JOY_NONE = 0,
	JOY_AXIS,
	JOY_BUTTON,
	JOY_HAT
};

enum {
	JMAP_NONE = 0,
	JMAP_KEY,
	JMAP_JOY,
	JMAP_MOUSE
};

typedef struct {
	int type;		// axis/button
	int num;		// number of axis/button
	int state;		// -x/+x for axis, 0/x for button
	int dev;		// device for action JMAP_*
	int key;		// key XKEY_* for keyboard
	int dir;		// XJ_* for kempston
	int rps;		// repeat state (0:released, !0:pressed)
	int rpt;		// repeat period (0 = no repeat)
	int cnt;		// repeat counter
} xJoyMapEntry;

void mapJoystick(Computer*, int, int, int);

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
	struct {
		QMap<int,QString> ram;		// on mem cell: phys.adr,name
		QMap<int,QString> rom;
		QMap<int,QString> cpu;		// on cpu adr
	} labmap;
	QMap<QString, xAdr> labels;
	QMap<QString, QColor> pal;
	QString labpath;
	unsigned short port;
	struct {
		unsigned fast:1;
		int pause;
	} emu;
	struct {
		unsigned changed:1;
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
	struct {
		SDL_Joystick* joy;
		int dead;
		QList<xJoyMapEntry> map;	// gamepad map for current profile
	} joy;
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
		// char lastDir[PATH_MAX];
		std::string confDir;
		std::string confFile;
		std::string romDir;
		std::string prfDir;
		std::string shdDir;
		std::string font;
		std::string boot;
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
		QMap<int, QString> comments;
	} dbg;
};

extern xConfig conf;
