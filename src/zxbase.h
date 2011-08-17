#ifndef _ZXBASE
#define _ZXBASE

#include "z80.h"
#include "memory.h"
#include "iosys.h"

#define CND_NONE	0
#define	CND_Z		1
#define	CND_C		2
#define CND_P		3
#define CND_S		4
#define CND_DJNZ	5
#define CND_LDIR	6
#define CND_CPIR	7

#define	ZPREF		1

class ZXBase;
struct ZOp {
	int32_t t;		// Z80 ticks on execution (base, w/o prefixes & conditions)
	int32_t cond;		// condition type
	int32_t tcn1;
	int32_t tcn0;
	int32_t flags;
	void (*func)(ZXBase*);	// execution func
	const char *name;	// mnemonic
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
		int32_t hwflags;
		Z80 *cpu;
		Memory* mem;
		IOSys* io;
		ZOp* inst[9];
		ZOpResult fetch();
		int32_t interrupt();
};

#endif
