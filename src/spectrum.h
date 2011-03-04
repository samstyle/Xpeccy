#ifndef _XPECTR
#define _XPECTR

#include <stdint.h>

#include "z80.h"
#include "memory.h"
#include "iosys.h"
#include "video.h"

#define TYP_RZX		0

class Spec;
class ZOp {
	public:
		ZOp(void(*fn)(Spec*),int32_t a,const char *nm) {name = nm; func = fn; t = a; prf = false;}
		ZOp(void(*fn)(Spec*),int32_t a,const char *nm, bool pr) {name = nm; func = fn; t = a; prf = pr;}
		const char *name;	// mnemonic
		void (*func)(Spec*);	// execution func
		int32_t t;		// Z80 ticks on execution
		bool prf;		// is prefix (CB,DD,ED,FD)
};

class Spec {
	public:
		Spec();
		bool istrb;
		bool fstrb;
		bool nmi;
		Z80 *cpu;
		Memory* mem;
		IOSys* io;
		Video* vid;
		ZOp* inst[9];
		void nmihandle();
		int32_t exec();
		int32_t interrupt();
};

extern Spec *sys;

#endif
