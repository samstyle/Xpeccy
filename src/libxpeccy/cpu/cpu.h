#ifndef _CPU_H
#define _CPU_H

typedef struct {
	unsigned cond:1;	// condition present
	unsigned met:1;		// condition met
	unsigned mem:1;		// operand mem rd :(nn),(hl),(de) etc
	int flag;
	int len;
	int oadr;		// direct addressation adr
	unsigned char mop;	// operand
	const char* mnem;
} xMnem;

#define REG_NONE 0
#define REG_EMPTY -1
#define REG_MPTR -2

typedef struct {
	int id;
	unsigned byte:1;
	char name[8];
	unsigned short value;
} xRegister;

typedef struct {
	char flags[8];		// name of flags
	xRegister regs[32];	// registers
} xRegBunch;

typedef struct {
	int id;
	const char* name;
	unsigned byte:1;
} xRegDsc;

// memrq rd
typedef unsigned char(*cbmr)(unsigned short,int,void*);
// memrq wr
typedef void(*cbmw)(unsigned short,unsigned char,void*);
// iorq rd
typedef unsigned char(*cbir)(unsigned short,void*);
// iorq wr
typedef void(*cbiw)(unsigned short,unsigned char,void*);
// iorq int : interrupt vector request
typedef unsigned char(*cbirq)(void*);
// memrd external
typedef unsigned char(*cbdmr)(unsigned short,void*);

#ifdef WORDS_BIG_ENDIAN
	#define PAIR(p,h,l) union{unsigned short p; struct {unsigned char h; unsigned char l;};}
#else
	#define PAIR(p,h,l) union{unsigned short p; struct {unsigned char l; unsigned char h;};}
#endif

#define OF_PREFIX	1
#define OF_EXT		OF_PREFIX
#define OF_SKIPABLE	(1<<1)
#define OF_RELJUMP	(1<<2)

struct CPU;
struct opCode {
	int flag;
	int t;				// T-states
	void(*exec)(struct CPU *);	// fuction to exec
	struct opCode *tab;		// next opCode tab (for prefixes)
	const char* mnem;		// mnemonic
};
typedef struct opCode opCode;

typedef struct {
	unsigned match:1;
	int idx;
	opCode* op;
	char* ptr;
	char arg[8][256];
} xAsmScan;

#include "Z80/z80.h"
#include "LR35902/lr35902.h"
#include "MOS6502/6502.h"

enum {
	CPU_NONE = 0,		// dummy
	CPU_Z80,		// ZX, MSX
	CPU_LR35902,		// GB, GBC
	CPU_6502		// ...
};

// typedef struct CPU CPU;

struct CPU {
	unsigned halt:1;		// cpu halted, undo on interrput
	unsigned resPV:1;		// Z80: reset PV flag on INT
	unsigned noint:1;		// Z80: don't handle INT after EI
	unsigned wait:1;		// Z80: WAIT signal
	unsigned lock:1;		// LR35902: CPU locked
	unsigned stop:1;		// LR35902: CPU stoped, unlock on keypress
	unsigned speed:1;		// LR35902: double speed mode (TODO)
	unsigned speedrq:1;		// LR35902: request speed change after STOP command
	unsigned dihalt:1;		// LR35902: HALT when DI: repeat next opcode
	unsigned sta:1;			// MOS6502: don't add 1T on (ABSX,ABSY,INDY)
	unsigned nod:1;			// MOS6502: ignore flag D in ADC/SBC

	int type;			// cpu type id

	unsigned char intrq;		// interrupts request. 8 bits = 8 INT types, 1 = requested
	unsigned char inten;		// interrupts enabled mask

	PAIR(pc,hpc,lpc);
	PAIR(sp,hsp,lsp);
	PAIR(ix,hx,lx);
	PAIR(iy,hy,ly);
	PAIR(mptr,hptr,lptr);
	unsigned char i;
	unsigned char r;
	unsigned char r7;
	unsigned char iff1;
	unsigned char iff2;
	unsigned char imode;		// Z80:int mode

	PAIR(af,a,f);
	PAIR(bc,b,c);
	PAIR(de,d,e);
	PAIR(hl,h,l);

	PAIR(af_,a_,f_);
	PAIR(bc_,b_,c_);
	PAIR(de_,d_,e_);
	PAIR(hl_,h_,l_);

	cbmr mrd;
	cbmw mwr;
	cbir ird;
	cbiw iwr;
	cbirq irq;
	void* data;

	unsigned char com;
	opCode* tab;
	opCode* opTab;
	opCode* op;

	void (*reset)(CPU*);
	int (*exec)(CPU*);
//	int (*intr)(CPU*);		// handle interrupt. intrq = requested INT types
	xAsmScan (*asmbl)(const char*, char*);
	xMnem (*mnem)(CPU*, unsigned short, cbdmr, void*);
	void (*getregs)(CPU*, xRegBunch*);
	void (*setregs)(CPU*, xRegBunch);

	int t;
	unsigned char tmp;
	unsigned char tmpb;
	PAIR(tmpw,htw,ltw);
	int tmpi;
};

typedef struct {
	int type;				// cpu type
	const char* name;			// printable name
	opCode* tab;				// start opcode tab;
	void (*reset)(CPU*);			// reset
	int (*exec)(CPU*);			// exec opcode, return T
//	int (*intr)(CPU*);			// handle interrupt, return T
	xAsmScan (*asmbl)(const char*, char*);	// compile mnemonic
	xMnem (*mnem)(CPU*, unsigned short, cbdmr, void*);
	void(*getregs)(CPU*,xRegBunch*);	// get cpu registers: name,id,value
	void(*setregs)(CPU*,xRegBunch);		// set cpu registers
} cpuCore;

CPU* cpuCreate(int,cbmr,cbmw,cbir,cbiw,cbirq,void*);
void cpuDestroy(CPU*);
void cpuSetType(CPU*, int);

int getCoreID(const char*);
const char* getCoreName(int);

extern cpuCore cpuTab[];

xMnem cpuDisasm(CPU*, unsigned short, char*, cbdmr, void*);
int cpuAsm(CPU*, const char*, char*, unsigned short);
xAsmScan scanAsmTab(const char*, opCode*);

xRegBunch cpuGetRegs(CPU*);
void cpuSetRegs(CPU*, xRegBunch);

#endif
