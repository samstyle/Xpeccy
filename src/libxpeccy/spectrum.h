#ifndef _XPECTR
#define _XPECTR

#include <stdint.h>
#include <string>
#include <vector>

#include <z80ex.h>
#include "memory.h"
#include "video.h"
#include "input.h"
#include "tape.h"
#include "bdi.h"
#include "ayym.h"		// sound
#include "gs.h"
#include "hdd.h"

#define HW_NULL		0
#define HW_ZX48		1
#define	HW_PENT		2
#define	HW_P1024	3
#define	HW_SCORP	4

#define	IO_WAIT		1
#define WAIT_ON		2

#define	RES_DEFAULT	0
#define	RES_48		1
#define	RES_128		2
#define	RES_DOS		3
#define	RES_SHADOW	4

struct HardWare {
	std::string name;
	int mask;		// mem size mask (b0:128, b1:256, b2:512, b3:1024); =0 for 48K
	int flag;
	int type;
};

struct RZXFrame {
	int fetches;
	std::vector<uint8_t> in;
};

struct ZXComp {
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
	std::vector<RZXFrame> rzx;
	uint64_t rzxFrame;
	uint32_t rzxPos;
	int rzxFetches;
	bool rzxPlay;	// true if rzx playing now
	bool intStrobe;
	bool nmiRequest;
	bool beeplev;
	bool block7ffd;
	float cpuFrq;
	float dotPerTick;
	int hwFlags;
	uint8_t prt0;		// 7ffd value
	uint8_t prt1;		// extend port value
	uint8_t prt2;		// scorpion ProfROM layer (0..3)
	int resbank;		// rompart active after reset
	struct {
		std::string GSRom;
		std::string hwName;
		std::string rsName;
	} opt;
	int gsCount;
};

ZXComp* zxCreate();
void zxDestroy(ZXComp*);
void zxReset(ZXComp*,int);
void zxOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
double zxExec(ZXComp*);
void zxSetFrq(ZXComp*,float);

#endif
