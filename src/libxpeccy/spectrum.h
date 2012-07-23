#ifndef _XPECTR
#define _XPECTR

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <assert.h>	// for debug

#include "z80ex.h"
#include "memory.h"
#include "video.h"
#include "input.h"
#include "tape.h"
#include "bdi.h"
#include "ayym.h"		// sound
#include "gs.h"
#include "hdd.h"
// hw type
#define HW_NULL		0
#define HW_ZX48		1
#define	HW_PENT		2
#define	HW_P1024	3
#define	HW_SCORP	4
#define	HW_PLUS2	5
#define	HW_PLUS3	6
// hw flags
#define	IO_WAIT		1
#define WAIT_ON		(1<<1)
// hw reset rompage
#define	RES_DEFAULT	0
#define	RES_48		1
#define	RES_128		2
#define	RES_DOS		3
#define	RES_SHADOW	4
// zx flags
#define	ZX_BREAK	1
#define	ZX_JUSTBORN	(1<<1)	// just created zx. need to reset after selection
// zx after-effect ops
#define	ZX_NONE		0
#define	ZX_OUT		1	// last op was OUT: real out must be after vidSync
#define	ZX_MWR		2	// last op has memWr: real writing must be after vidSync

typedef struct {
	const char* name;
	int mask;		// mem size mask (b0:128, b1:256, b2:512, b3:1024); =0 for 48K
	int flag;
	int type;
} HardWare;

typedef struct {
	int fetches;
	int frmSize;
	uint8_t* frmData;
} RZXFrame;

typedef struct {
	int flags;
	HardWare *hw;
	Z80EX_CONTEXT* cpu;
	Memory* mem;
	Video* vid;
	Keyboard* keyb;
	Joystick* joy;
	Mouse* mouse;
	Tape* tape;
	BDI* bdi;
	IDE* ide;
	GSound* gs;
	TSound* ts;
	int rzxSize;
	RZXFrame* rzxData;
	uint64_t rzxFrame;
	uint32_t rzxPos;
	int rzxFetches;
	int rzxPlay;	// true if rzx playing now
	int intStrobe;
	int frmStrobe;
	int nmiRequest;
	int beeplev;
	int block7ffd;
	float cpuFrq;
	float dotPerTick;
	int hwFlags;
	uint8_t prt0;		// 7ffd value
	uint8_t prt1;		// extend port value
	uint8_t prt2;		// scorpion ProfROM layer (0..3)
	int resbank;		// rompart active after reset
	int gsCount;
	int aeOp;		// after-effect op type
	unsigned short adr;	// remember it for out or mwr
	unsigned char value;
} ZXComp;

ZXComp* zxCreate();
void zxDestroy(ZXComp*);
void zxReset(ZXComp*,int);
void zxOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
double zxExec(ZXComp*);
void zxSetFrq(ZXComp*,float);

void rzxClear(ZXComp*);

#ifdef __cplusplus
}
#endif

#endif
