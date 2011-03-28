#ifndef _XPECTR
#define _XPECTR

#include <stdint.h>

#include "zxbase.h"
#include "video.h"
#include "keyboard.h"
#include "tape.h"
#include "bdi.h"
#include "sound.h"
#include "gs.h"
#include "hdd.h"

#define TYP_RZX		0
// conditions
#define CND_NONE	0
#define	CND_Z		1
#define	CND_C		2
#define CND_P		3
#define CND_S		4
#define CND_DJNZ	5
#define CND_LDIR	6
#define CND_CPIR	7

#define	ZOP_PREFIX	(1<<0)

#define HW_NULL		0
#define HW_ZX48		1
#define	HW_PENT		2
#define	HW_P1024	3
#define	HW_SCORP	4

class ZXComp {
	public:
		ZXComp();
		HardWare *hw;
		std::vector<HardWare> hwlist;
		ZXBase* sys;
		Video* vid;
		Keyboard* keyb;
		Mouse* mouse;
		Tape* tape;
		BDI* bdi;
		IDE* ide;
		GS* gs;
		AYSys* aym;
		void exec();
		void reset();
//		void addHardware(std::string,int(*)(int),void(*)(int,uint8_t),uint8_t(*)(int),void(*)(),int,int);
		void addHardware(std::string,int,int,int);
		void setHardware(std::string);
		void mapMemory();
		int32_t getPort(int32_t);
		uint8_t in(int32_t);
		void out(int32_t,uint8_t);
		void INTHandle();
		void NMIHandle();
};

// extern Spec *sys;

#endif
