#ifndef _ZXBASE
#define _ZXBASE

#include "z80.h"
#include "memory.h"
#include "iosys.h"

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

#endif
