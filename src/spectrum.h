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

class ZXComp {
	public:
		ZXComp();
		ZXBase* sys;
		Video* vid;
		Keyboard* keyb;
		Mouse* mouse;
		Tape* tape;
		BDI* bdi;
		GS* gs;
		AYSys* aym;
		void exec();
		void reset();
		uint8_t in(int);
		void out(int,uint8_t);
		void INTHandle();
		void NMIHandle();
};

// extern Spec *sys;

#endif
