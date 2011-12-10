#ifndef _XPECTR
#define _XPECTR

#include <stdint.h>

#include "z80ex.h"
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
	int mask;		// mem size mask (0:128, 1:256, 2:512, 3:1024); =0 for 48K
	int flags;
	int type;
};

struct RZXFrame {
	int fetches;
	std::vector<uint8_t> in;
};

class ZXComp {
	public:
		ZXComp();
		HardWare *hw;
		Z80EX_CONTEXT* cpu;
		Memory* mem;
		Video* vid;
		Keyboard* keyb;
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
		float cpuFreq;
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
		double exec();
		void reset(int);
		void mapMemory();
		uint8_t in(uint16_t);
		void out(uint16_t,uint8_t);
		int gsCount;
		int tapCount;
};

#endif
