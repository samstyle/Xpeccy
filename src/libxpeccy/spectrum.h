#ifndef _XPECTR
#define _XPECTR

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.h"
#include "memory.h"
#include "video.h"
#include "input.h"
#include "tape.h"
#include "bdi.h"
#include "ayym.h"		// AY/YM/TS sound
#include "soundrive.h"		// covox/soundrive
#include "gs.h"
#include "hdd.h"
#include "sdcard.h"

// zx flag
#define	ZX_BREAK	1	// breakpoint reached
//#define	ZX_JUSTBORN	(1<<1)	// just created zx. need to reset after selection
#define	ZX_PALCHAN	(1<<2)	// signal: palete changed
// hwFlag
#define HW_WAIT		1	// scorpion wait
#define	HW_CONTMEM	(1<<1)	// contended mem
#define	HW_CONTIO	(1<<2)	// contended io
// hw reset rompage
#define	RES_DEFAULT	0
#define	RES_48		1
#define	RES_128		2
#define	RES_DOS		3
#define	RES_SHADOW	4

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

struct ZXComp {
	int flag;		// states
	int hwFlag;		// hardware properties
	struct HardWare *hw;
	CPU* cpu;
	Memory* mem;
	Video* vid;
	Keyboard* keyb;
	Joystick* joy;
	Mouse* mouse;
	Tape* tape;
	BDI* bdi;
	IDE* ide;
	SDCard* sdc;
	GSound* gs;
	TSound* ts;
	SDrive* sdrv;
	int rzxSize;
	RZXFrame* rzxData;
	unsigned long rzxFrame;
	unsigned int rzxPos;
	int rzxFetches;
	int rzxPlay;	// true if rzx playing now
	int intStrobe;
	int frmStrobe;
	int nmiRequest;
	int beeplev;
	float cpuFrq;

	int nsPerTick;
	int nsPerFrame;
	int nsCount;

	memEntry memMap[16];		// memory map for ATM2, PentEvo
	unsigned char colMap[16];	// color map
	unsigned char prt0;		// 7ffd value
	unsigned char prt1;		// extend port value
	unsigned char prt2;		// scorpion ProfROM layer (0..3)
	struct {
		unsigned char evoBF;		// PentEvo rw ports
		unsigned char evo2F;
		unsigned char evo4F;
		unsigned char evo6F;
		unsigned char evo8F;
	} evo;
	struct {
		DMAaddr src;
		DMAaddr dst;
		unsigned char len;
		unsigned char num;
	} dma;
	struct {
		int flag;
		unsigned char TSConf;
		unsigned char tsScrPal;
		unsigned char tsAMem[0x1000];	// ALTERA mem: 512 bytes TSConf palette, ... (tiles,sprites)
		unsigned short tsMapAdr;	// adr for palette mapping
		unsigned char FDDVirt;
		unsigned char Page0;
		unsigned char Page1;
		unsigned char Page2;
		unsigned char Page3;
		unsigned char TMPage;		// tile map page
		unsigned char T0GPage;		// tile 0 graphics page
		unsigned char T1GPage;		// tile 1 graphics page
		unsigned char SGPage;		// sprite graphics page
	} tsconf;
	CMOS cmos;
	unsigned char dosen;		// active trdos (dosen and b4,prt0 sets rompart)
	int resbank;			// rompart active after reset
	int tapCount;
	unsigned long tickCount;
};

typedef struct ZXComp ZXComp;

#include "hardware/hardware.h"

ZXComp* zxCreate();
void zxDestroy(ZXComp*);
void zxReset(ZXComp*,int);
void zxOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
int zxExec(ZXComp*);
void zxSetFrq(ZXComp*,float);
void zxSetLayout(ZXComp*, int, int, int, int, int, int, int, int, int);
void zxSetHardware(ZXComp*,const char*);

void rzxClear(ZXComp*);

#ifdef __cplusplus
}
#endif

#endif
