#ifndef _XPECTR
#define _XPECTR

#ifdef __cplusplus
extern "C" {
#endif

#include "z80/z80.h"
#include "memory.h"
#include "video.h"
#include "input.h"
#include "tape.h"
#include "fdc.h"
#include "sound/sndcommon.h"
#include "hdd.h"
#include "sdcard.h"

// hw reset rompage
#define	RES_DEFAULT	0
#define	RES_48		1
#define	RES_128		2
#define	RES_DOS		3
#define	RES_SHADOW	4
// memory breaks
#define	MEM_BRK_FETCH	1
#define	MEM_BRK_RD	(1<<1)
#define	MEM_BRK_WR	(1<<2)
#define	MEM_BRK_ANY	(MEM_BRK_FETCH | MEM_BRK_RD | MEM_BRK_WR)
#define MEM_BRK_TFETCH	(1<<3)
// io breaks
#define IO_BRK_RD	1
#define IO_BRK_WR	(1<<1)

typedef struct {
	unsigned char l;
	unsigned char h;
	unsigned char x;
} DMAaddr;

typedef struct {
	int fetches;
	int frmSize;
	unsigned char* frmData;
} RZXFrame;

typedef struct {
	unsigned char flag;
	unsigned char page;
} memEntry;

// MSX cartridge mapper type
#define MSX_UNKNOWN	-1
#define MSX_NOMAPPER	0
#define	MSX_KONAMI4	1
#define	MSX_KONAMI5	2
#define	MSX_ASCII8	3
#define	MSX_ASCII16	4

typedef struct {
	unsigned char* data;
	char name[512];
	int memMask;
	int memMap[8];		// 8 of 8kb pages
	int mapType;		// user defined mapper type, if auto-detect didn't worked
	int mapAuto;		// auto detected map type OR user defined
} xCartridge;

typedef struct {
	unsigned brk:1;			// breakpoint
	unsigned debug:1;		// dont' do breakpoints
	unsigned frmStrobe:1;		// new frame started
	unsigned intStrobe:1;		// int front
	unsigned nmiRequest:1;		// Magic button pressed
	unsigned beeplev:1;		// beeper level
	unsigned rzxPlay:1;		// rzx is playing
	unsigned firstRun:1;

	unsigned rom:1;		// b4,7ffd
	unsigned dos:1;		// BDI dos
	unsigned cpm:1;

	unsigned scrpWait:1;		// scorpion wait mode
	unsigned contMem:1;		// contended mem
	unsigned contIO:1;		// contended IO

	double cpuFrq;
	unsigned char intVector;

	struct HardWare *hw;
	CPU* cpu;
	Memory* mem;
	Video* vid;
	Keyboard* keyb;
	Joystick* joy;
	Mouse* mouse;
	Tape* tape;
	DiskIF* dif;
	IDE* ide;
	SDCard* sdc;
	GSound* gs;
	TSound* ts;
	saaChip* saa;
	SDrive* sdrv;

	struct {
		FILE* file;
		int size;
		RZXFrame* data;
		int fetches;
		int frame;
		int pos;
	} rzx;

	unsigned long tickCount;
	int nsPerTick;
	int nsPerFrame;

	memEntry memMap[16];		// memory map for ATM2, PentEvo
	unsigned char p7FFD;		// 7ffd value
	union {				// extend port values
		//unsigned char prt1;
		unsigned char p1FFD;
		unsigned char pEFF7;
		unsigned char pDFFD;
		unsigned char p77hi;
	};
	unsigned char prt2;		// scorpion ProfROM layer (0..3)

	unsigned char brkRamMap[0x400000];	// ram brk
	unsigned char brkRomMap[0x80000];	// rom brk
	unsigned char brkIOMap[0x10000];	// io brk

	unsigned short padr;
	unsigned char pval;

	struct {
		unsigned char evoBF;		// PentEvo rw ports
		unsigned char evo2F;
		unsigned char evo4F;
		unsigned char evo6F;
		unsigned char evo8F;
		int hiTrig;			// trigger for hi-low byte in IDE
		unsigned char blVer[16];	// bootloader info
		unsigned char bcVer[16];	// baseconf info
	} evo;
	struct {
		DMAaddr src;
		DMAaddr dst;
		unsigned char len;
		unsigned char num;
	} dma;
	struct {
		int flag;
		unsigned short tsMapAdr;	// adr for altera mapping
		unsigned char Page0;
		unsigned char p00af;		// ports to be updated from next line
		unsigned char p01af;
		unsigned char p02af;
		unsigned char p03af;
		unsigned char p04af;
		unsigned char p05af;
		unsigned char p07af;
		unsigned char pwr_up;		// 1 on 1st run, 0 after reading 00AF
		unsigned char vdos;
	} tsconf;
	struct {
		unsigned char p7E;		// color num (if trig7E=0)
	} profi;
	struct {
		unsigned char keyLine;		// selected keyboard line
		unsigned char pA8;		// port A8
		unsigned char pAA;		// port AA
		unsigned char memMap[4];	// RAM pages (ports FC..FF)
		struct {
			unsigned char regA;
			unsigned char regB;
			unsigned char regC;
		} ppi;
		xCartridge slotA;
		xCartridge slotB;
	} msx;

	CMOS cmos;
	int resbank;			// rompart active after reset
	int tapCount;
} Computer;

#include "hardware/hardware.h"

Computer* compCreate();
void compDestroy(Computer*);
void compReset(Computer*,int);
int compExec(Computer*);
void compSetFrq(Computer*,double);
void compSetLayout(Computer*, int, int, int, int, int, int, int, int, int);
void compSetHardware(Computer*,const char*);

// read-write cmos
unsigned char cmsRd(Computer*);
void cmsWr(Computer*,unsigned char);

void rzxStop(Computer*);

unsigned char* getBrkPtr(Computer*, unsigned short);

#ifdef __cplusplus
}
#endif

#endif
