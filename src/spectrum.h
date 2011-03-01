#ifndef _XPECTR
#define _XPECTR

#include "z80.h"
#include "memory.h"
#include "iosys.h"
#include "video.h"

#define TYP_RZX		0

struct Spec;
struct ZOp {
	ZOp(void(*fn)(Spec*),int a,const char *nm) {name = nm; func = fn; t = a; prf = false;}
	ZOp(void(*fn)(Spec*),int a,const char *nm, bool pr) {name = nm; func = fn; t = a; prf = pr;}
	const char *name;	// mnemonic
	void (*func)(Spec*);	// execution func
	int t;			// Z80 ticks on execution
	bool prf;		// is prefix (CB,DD,ED,FD)
};

struct Spec {
	Spec();
	Z80 *cpu;
	Memory* mem;
	IOSys* io;
	Video* vid;
	ZOp* inst[9];
	int exec();
	int interrupt();
	void nmihandle();
	bool istrb;
	bool fstrb;
	bool nmi;
};

extern Spec *sys;

#endif
