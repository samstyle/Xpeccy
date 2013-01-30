#ifndef _XPECTR
#define _XPECTR

#ifdef __cplusplus
extern "C" {
#endif

#include "z80ex.h"
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
#define	ZX_JUSTBORN	(1<<1)	// just created zx. need to reset after selection
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
	int fetches;
	int frmSize;
	unsigned char* frmData;
} RZXFrame;

typedef struct {
	unsigned char flag;
	unsigned char page;
} memEntry;

typedef struct {
	int flag;		// states
	int hwFlag;		// hardware properties
	struct HardWare *hw;
	Z80EX_CONTEXT* cpu;
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
	float dotPerTick;
	int nsPerTick;
	memEntry memMap[16];		// memory map for ATM2, PentEvo
	unsigned char colMap[16];	// color map (--GgRrBb)
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
	CMOS cmos;
	unsigned char dosen;		// active trdos (dosen and b4,prt0 sets rompart)
	int resbank;		// rompart active after reset
//	int gsCount;
	int tapCount;
	unsigned long tickCount;
	float frmDot;
	int syncTick;
	float tickPerFrame;
} ZXComp;

struct HardWare {
	const char* name;
	int mask;		// mem size bits (b0:128, b1:256, b2:512, b3:1M, b4:2M, b5:4M); =0 for 48K
	int type;
	void (*mapMem)(ZXComp*);
	void (*out)(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
	Z80EX_BYTE (*in)(ZXComp*,Z80EX_WORD,int);
};

#include "hardware/hardware.h"

ZXComp* zxCreate();
void zxDestroy(ZXComp*);
void zxReset(ZXComp*,int);
void zxOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
double zxExec(ZXComp*);
void zxSetFrq(ZXComp*,float);
void zxSetLayout(ZXComp*, int, int, int, int, int, int, int, int, int);

void rzxClear(ZXComp*);

#ifdef __cplusplus
}
#endif

#endif
