#ifndef _XPECTR
#define _XPECTR

#include <stdint.h>

#include "zxbase.h"
#include "video.h"
#include "keyboard.h"
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

class ZXComp : public ZXSystem {
	public:
		ZXComp();
		HardWare *hw;
//		std::vector<HardWare> hwlist;
		ZXBase* sys;
		Video* vid;
		Keyboard* keyb;
		Mouse* mouse;
		Tape* tape;
		BDI* bdi;
		IDE* ide;
		GS* gs;
		AYSys* aym;
		bool beeplev;
		bool block7ffd;
		struct {
			std::string GSRom;
			std::string hwName;
			std::string rsName;
		} opt;
		uint32_t exec();
		void reset(int);
		void mapMemory();
		int32_t getPort(int32_t);
		uint8_t in(uint16_t);
		void out(uint16_t,uint8_t);
		void INTHandle();
		void NMIHandle();
};

#endif
