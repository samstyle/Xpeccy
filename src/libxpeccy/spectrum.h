#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu/cpu.h"
#include "video/video.h"
#include "memory.h"

#include "input/input.h"
#include "tape.h"
#include "fdc.h"
#include "hdd.h"
#include "sdcard.h"
#include "cartridge.h"
#include "i8255_ppi.h"
#include "i8253_pit.h"
#include "i8259_pic.h"
#include "i8042_kbd.h"
#include "i8237_dma.h"
#include "mos6526_cia.h"
#include "uart.h"
#include "upd4990_rtc.h"

#include "sound/ayym.h"
#include "sound/gs.h"
#include "sound/saa1099.h"
#include "sound/soundrive.h"
#include "sound/gbsound.h"
#include "sound/nesapu.h"

#ifdef HAVEZLIB
	#include <zlib.h>
#endif

extern int compflags;

#define CFLG_PANIC	1

// hw reset rompage
enum {
	RES_DEFAULT = 0,
	RES_48,
	RES_128,
	RES_DOS,
	RES_SHADOW
};

enum {
	DBG_VIEW_CODE = 0x00,
	DBG_VIEW_BYTE = 0x10,
	DBG_VIEW_WORD = 0x20,
	DBG_VIEW_ADDR = 0x30,
	DBG_VIEW_TEXT = 0x40,
	DBG_VIEW_EXEC = 0x50
};

// TODO: DMAaddr = xreg32 (x -> ih)
typedef struct {
	unsigned char l;
	unsigned char h;
	unsigned char x;
} DMAaddr;

typedef struct {
	unsigned char flag;
	unsigned char page;
} memEntry;

typedef struct {
	int t;
	int a;
	unsigned char* ptr;
} bpChecker;

#define p7FFD	reg[0]
#define p1FFD	reg[1]
#define pEFF7	reg[2]
#define prt2	reg[3]
#define regNEST	reg[4]		// nes type

#define flgBRK	sysflag[0]		// breakpoint catched
#define flgDBG	sysflag[1]		// debug execution
#define flgMAP	sysflag[2]		// map memory on runtime
#define flgFRM	sysflag[3]		// new frame ready
#define flgNMIRQ sysflag[4]		// NMI requested
#define flgFRN	sysflag[5]		// need to reset when switch to profile
#define flgDDP	sysflag[6]		// used ATM2+ only, but have options entry
#define flgEM1	sysflag[7]		// (same for ZS Scorpion)
#define flgCNTM	sysflag[8]		// contended mem (zx)
#define flgCNTI	sysflag[9]		// contended i/o (zx)
#define flgIBRK	sysflag[10]		// break on interrupt
#define flgROM	sysflag[11]
#define flgDOS	sysflag[12]
#define flgCPM	sysflag[13]
#define flgEXT	sysflag[14]
#define flgBDI	sysflag[15]

typedef struct {
	double fps;
	double cpuFrq;
	double frqMul;
	unsigned char intVector;

	struct HardWare *hw;	// computer core - misc params, callbacks

	int brkt;		// breakpoint type (cpu, ram, rom...)
	int brka;		// breakpoint addr

	char* msg;		// message ptr for displaying outside
	int resbank;		// rompart active after reset

	int tickCount;		// accumulate T
	int frmtCount;		// accumulate T, but reset each INT
	int hCount;		// T before HALT = frmtCount @ HALT
	int fCount;		// T in last frame
	int nsPerTick;

	bool flag[128];
	bool sysflag[32];
	unsigned char reg[512];		// internal registers
	xreg32 xreg[32];		// 32/16/8 bits registers
// base
	CPU* cpu;
// TODO: align memory/brkMaps, respect ram/rom size
	Memory* mem;
	Video* vid;
// input
	Keyboard* keyb;
	Joystick* joy;
	Joystick* joyb;
	Mouse* mouse;
// data storage
	Tape* tape;
	DiskIF* dif;
	IDE* ide;
	SDCard* sdc;
	xCartridge* slot;		// cartrige slot (MSX, GB, NES)
// sound
	bitChan* beep;
	TSound* ts;
	GSound* gs;
	SDrive* sdrv;
	saaChip* saa;
	gbSound* gbsnd;
	nesAPU* nesapu;
// misc
	PPI* ppi;			// i8255-like chip
	CMOS cmos;

#ifdef HAVEZLIB

	struct {
		unsigned start:1;
		unsigned stop:1;
		unsigned play:1;
		unsigned overio:1;
		int fTotal;
		int fCurrent;
		int fCount;
		FILE* file;		// converted tmp-file
		struct {
			int fetches;
			int size;
			unsigned char data[0x10000];
			int pos;
		} frm;
	} rzx;

#endif

	memEntry memMap[16];			// memory map for ATM2, PentEvo
	unsigned char brkRamMap[MEM_4M];	// ram brk/type : b0..3:brk flags, b4..7:type
	unsigned char brkRomMap[MEM_512K];	// rom brk/type : b0..3:brk flags, b4..7:type
	unsigned char brkAdrMap[MEM_64K];	// adr brk
	unsigned char brkIOMap[MEM_64K];	// io brk

	struct {
		DMAaddr src;		// -> xreg32 ???
		DMAaddr dst;
		unsigned char len;
		unsigned char num;
	} dma;
	struct {
		unsigned char Page0;
		unsigned char p21af;
		unsigned char pwr_up;		// 1 on 1st run, 0 after reading 00AF
	} tsconf;
	struct {
		struct {
			struct {
				long per;
				long cnt;
			} div;		// divider (16KHz, inc FF04)
			struct {
				long per;
				long cnt;
			} t;		// manual timer
		} timer;

		unsigned char iram[256];	// internal ram (FF80..FFFE)
		unsigned char iomap[128];
	} gb;
// c64
	CIA* cia1;		// mos6526
	CIA* cia2;
// ibm
	PIT* pit;		// timer
	PIC* mpic;		// master pic
	PIC* spic;		// slave pic
	PS2Ctrl* ps2c;
	i8237DMA* dma1;		// 8-bit dma
	i8237DMA* dma2;		// 16-bit dma
	upd4990* rtc;
	UART* uart;		// com1 (mouse) controller
} Computer;

#include "hardware/hardware.h"

Computer* compCreate();
void compDestroy(Computer*);
void compReset(Computer*,int);
int compExec(Computer*);
void comp_irq(int, void*);

//void compKeyPress(Computer*, int, keyEntry*);
//void compKeyRelease(Computer*, int, keyEntry*);
void comp_kbd_release(Computer*);

void compSetBaseFrq(Computer*,double);
void compSetTurbo(Computer*,double);
int compSetHardware(Computer*,const char*);
void comp_set_layout(Computer*, vLayout*);

// read-write cmos
unsigned char cmsRd(Computer*);
void cmsWr(Computer*, int);

void rzxStop(Computer*);

void comp_brk(Computer*, int);
unsigned char* getBrkPtr(Computer*, int);
unsigned char getBrk(Computer*, int);
void setBrk(Computer*, int, unsigned char);

#ifdef __cplusplus
}
#endif
