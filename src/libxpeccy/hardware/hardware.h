#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../spectrum.h"
#include <assert.h>

// hw type
enum {
	HW_NULL = 0,
	HW_DUMMY,	// nothing-to-do-here
	HW_ZX48,	// ZX48K (Classic)
	HW_PENT,	// Pentagon
	HW_P1024,	// Pentagon1024SL
	HW_SCORP,	// ZS Scorpion
	HW_PLUS2,	// Spectrum 2+
	HW_PLUS3,	// Spectrum 3+
	HW_ATM1,	// ATM 1
	HW_ATM2,	// ATM 2+
	HW_PENTEVO,	// ZXEvo (Baseconf)
	HW_TSLAB,	// PentEvo (TSConf)
	HW_PROFI,	// Profi
	HW_PHOENIX,	// ZXM Phoenix
	HW_ALF,		// ALF (ZX48K-like console)
	HW_MSX,		// MSX 1
	HW_MSX2,	// MSX 2
	HW_GBC,		// Game boy color (gameboy capatible)
	HW_NES,		// Nintendo Entertaiment System (Dendy)
	HW_C64,		// Commodore 64
	HW_BK0010,	// BK0010
	HW_BK0011M,	// BK0011m
	HW_SPCLST,	// PC Specialist
	HW_IBM_PC,	// IBM PC AT/XT
	HW_PC9801	// NEC PC 9801
};

// hw group
enum {
	HWG_NULL = 0,
	HWG_ZX,
	HWG_ALF,
	HWG_MSX,
	HWG_GB,
	HWG_NES,
	HWG_COMMODORE,
	HWG_BK,
	HWG_SPCLST,
	HWG_PC,
	HWG_PC98XX
};

enum {
	NES_DENDY = 0,
	NES_NTSC,
	NES_PAL
};

// Hardware callbacks

// std callback
typedef void(*cbhwcomp)(Computer*);
// sync: calls after every CPU command
typedef void(*cbHwSnc)(Computer*, int);
// memory read
typedef int(*cbHwMrd)(Computer*, int, int);
// memory write
typedef void(*cbHwMwr)(Computer*, int, int);
// io read
typedef int(*cbHwIrd)(Computer*, int);
// io write
typedef void(*cbHwIwr)(Computer*, int, int);
// int request
typedef void(*cbHwIrq)(Computer*, int);
// int ack
typedef int(*cbHwAck)(Computer*);
// key press/release
typedef void(*cbHwKey)(Computer*, keyEntry*);
// get volume
typedef sndPair(*cbHwVol)(Computer*, sndVolume*);

typedef struct {
	int port;
	int type;
	size_t offset;
} xPortDsc;

struct HardWare {
	int id;			// id
	int grp;
	const char* name;	// name used in conf file
	const char* optName;	// name used in options window
	int base;		// numbers base (8/10/16)
	int mask;		// mem size bits (see memory.h)
	double xscale;		// pixel ratio (x:y)
	vLayout* lay;		// fixed layout ptr. if NULL, use from config
	int adrbus;		// cpu adr bus width (16/20/24), pgsize = 2^(n-8)	TODO: must be in CPU core
	xPortDsc* portab;	// tab of ports descriptors
	cbhwcomp init;		// init (call on setting comp hardware)
	cbhwcomp mapMem;	// map memory
	cbHwIwr out;		// io wr
	cbHwIrd in;		// io rd
	cbHwMrd mrd;		// mem rd
	cbHwMwr mwr;		// mem wr
	cbHwIrq irq;		// int rq
	cbHwAck ack;		// int ack
	cbhwcomp reset;		// reset
	cbHwSnc sync;		// sync time
	cbHwKey keyp;		// key press
	cbHwKey keyr;		// key release
	cbHwVol vol;		// read volume
};
typedef struct HardWare HardWare;

typedef struct {
	int mask;		// if (port & mask == value & mask) port is catched
	int value;
	unsigned dos:2;		// 00:!dos only; 01:dos only; 1x:nevermind
	unsigned rom:2;		// same: b4,7FFD
	unsigned cpm:2;		// cpm mode (for profi)
	int (*in)(Computer*, int);
	void (*out)(Computer*, int, int);
} xPort;

typedef struct {
	int port;
	int value;
} xPortValue;

typedef struct {
	int id;
	HardWare* core;
} tabHwItem;

int hwIn(xPort* ptab, Computer* comp, int port);
void hwOut(xPort* ptab, Computer* comp, int port, int val, int mult);
xPortValue* hwGetPorts(Computer*);

// extern HardWare hwTab[];

HardWare* findHardware(const char*);
int stdMRd(Computer*, int, int);
void stdMWr(Computer*, int, int);

// debug IO

int brkIn(Computer*, int);
void brkOut(Computer*, int, int);

int dummyIn(Computer*, int);
void dummyOut(Computer*, int, int);

// common IO

int zx_dev_wr(Computer*, int, int);
int zx_dev_rd(Computer*, int, int*);
void zx_irq(Computer*, int);
int zx_ack(Computer*);

void xOutFE(Computer*, int, int);
void xOutBFFD(Computer*, int, int);
void xOutFFFD(Computer*, int, int);

int xIn1F(Computer*, int);
int xInFE(Computer*, int);
int xInFFFD(Computer*, int);
int xInFADF(Computer*, int);
int xInFBDF(Computer*, int);
int xInFFDF(Computer*, int);

// common_zx calls
void zx_init(Computer*);
void zx_reset(Computer*);
void zx_keyp(Computer*, keyEntry*);
void zx_keyr(Computer*, keyEntry*);
void zx_sync(Computer*, int);
sndPair zx_vol(Computer*, sndVolume*);
void zx_set_pal(Computer*);	// todo: called from zx_reset only

// nes calls (need for deBUGa widget)
int nes_apu_ext_rd(int, void*);
int nes_ppu_ext_rd(int, void*);

#ifdef __cplusplus
}
#endif
