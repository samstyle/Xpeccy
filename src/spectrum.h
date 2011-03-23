#ifndef _XPECTR
#define _XPECTR

#include <stdint.h>

#include "z80.h"
#include "memory.h"
#include "iosys.h"
#include "video.h"
#include "tape.h"
#include "bdi.h"

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

class ZXBase;
class ZOp {
	public:
		ZOp(void(*)(ZXBase*),int32_t,int32_t,int32_t,int32_t,const char*,int32_t);
		ZOp(void(*)(ZXBase*),int32_t,const char*);
/*		
		ZOp(void(*fn)(ZXBase*),int32_t a,const char *nm) {name = nm; func = fn; t = a; prf = false;}
		ZOp(void(*fn)(ZXBase*),int32_t a,const char *nm, bool pr) {name = nm; func = fn; t = a; prf = pr;}
*/
		const char *name;	// mnemonic
		void (*func)(ZXBase*);	// execution func
		int32_t t;		// Z80 ticks on execution (base, w/o prefixes & conditions)
		int32_t cond;		// condition type
		int32_t tcn1,tcn0;	// ticks if condition true or false
		int32_t flags;
//		bool prf;		// is prefix (CB,DD,ED,FD)
};

struct ZOpResult {
	int ticks;
	void(*exec)(ZXBase*);
};

class ZXBase {
	public:
		ZXBase();
		bool istrb;
		bool fstrb;
		bool nmi;
		Z80 *cpu;
		Memory* mem;
		IOSys* io;
//		Video* vid;
		ZOp* inst[9];
//		void nmihandle();
		ZOpResult exec();
		int32_t interrupt();
};

class ZXComp {
	public:
		ZXComp();
		ZXBase* sys;
		Video* vid;
		Tape* tape;
//		BDI* bdi;
		void exec();
		void reset();
//		uint8_t in(int);
//		void out(int,uint8_t);
		void INTHandle();
		void NMIHandle();
};

// extern Spec *sys;

#endif
