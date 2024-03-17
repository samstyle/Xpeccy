#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include <setjmp.h>		// to implement try-catch
#define THROW(__N) longjmp(cpu->jbuf, __N)
#define THROW_EC(__N,__C) cpu->errcod = __C; longjmp(cpu->jbuf, __N)

#include "../defines.h"

typedef unsigned char xbyte;
typedef unsigned short xword;

typedef struct {
	unsigned cond:1;	// condition present
	unsigned met:1;		// condition met
	unsigned mem:1;		// operand mem rd :(nn),(hl),(de) etc
	int flag;
	int len;
	int oadr;		// direct addressation adr
	unsigned short mop;	// operand
	const char* mnem;
} xMnem;

#define REG_NONE	0
#define REG_EMPTY	-1
#define REG_MPTR	-2

#define REG_BIT		1
#define REG_BYTE	8
#define REG_WORD	16
#define REG_24		24
#define REG_32		32
#define REG_TMASK	0xff
#define REG_RO		0x100	// protect from changes in debuga
#define REG_SEG		0x200	// register is segment

typedef struct {
	int id;
	int type;
	const char* name;
	int value;	// register value (selector)
	int base;	// base address for segment register
} xRegister;

typedef struct {
	char* flags;		// name of flags
	xRegister regs[32];	// registers
} xRegBunch;

typedef struct {
	int id;
	const char* name;
	int type;
	size_t offset;		// = offsetof(CPU, <member>), e.g offsetof(CPU, pc)
} xRegDsc;

// memrq rd
typedef int(*cbmr)(int, int, void*);
// memrq wr
typedef void(*cbmw)(int, int, void*);
// iorq rd
typedef int(*cbir)(int, void*);
// iorq wr
typedef void(*cbiw)(int, int, void*);
// iorq int : interrupt vector request
typedef int(*cbiack)(void*);
// memrd external
typedef int(*cbdmr)(int, void*);

#define OF_PREFIX	1
#define OF_EXT		OF_PREFIX
#define OF_SKIPABLE	(1<<1)		// opcode is skipable by f8
#define OF_RELJUMP	(1<<2)
#define OF_MBYTE	(1<<3)		// operand is byte from memory
#define OF_MWORD	(1<<4)		// operand is word from memory
#define OF_MEMADR	(1<<5)		// operand contains memory address (nn)
#define OF_WORD		(1<<6)		// i286:use words for mod byte
#define OF_PRT		(1<<7)		// i286:protect mode only
#define OF_MODRM	(1<<8)		// i286:mod r/m byte present
#define OF_COMEXT	(1<<9)		// mod r/m contains command extension bits
#define OF_GEN		(1<<10)		// opcode depends on cpu generation (86/186/286) cpu->tab is sub-table[cpu->gen]
#define OF_MODCOM	(OF_MODRM | OF_COMEXT)

typedef struct CPU CPU;
typedef struct opCode opCode;

typedef void(*cbcpu)(CPU*);

struct opCode {
	int flag;
	int t;				// T-states
	cbcpu exec;			// fuction to exec
	opCode *tab;			// next opCode tab (for prefixes)
	const char* mnem;		// mnemonic
};

typedef struct {
	int flag;
	PAIR(ival,ivh,ivl);
	PAIR(val,vh,vl);
	int bper;
	int per;
	int cnt;
} xTimer;

typedef struct {
	unsigned match:1;
	int idx;
	opCode* op;
	char* ptr;
	char arg[8][256];
} xAsmScan;

typedef struct {
	int idx;			// 'visible' value
//	unsigned char flag;		// access flag
	unsigned pl:2;			// priv.level
	unsigned ar:5;			// type
	unsigned ext:1;			// code is conforming | data is growing from limit to FFFF
	unsigned wr:1;			// write enable (for data, 0 for code)
	unsigned rd:1;			// read enable (for code, 1 for data)
	unsigned pr:1;			// present
	unsigned sys:1;			// !(ar & 0x10)
	unsigned code:1;		// ar & 0x18 = 0x18
	unsigned data:1;		// ar & 0x18 = 0x10
	unsigned base:24;		// segment base addr
	unsigned short limit;		// segment size in bytes
} xSegPtr;				// aka segment table descriptor

#include "Z80/z80.h"
#include "LR35902/lr35902.h"
#include "MOS6502/6502.h"
#include "1801vm1/1801vm1.h"
#include "i8080/i8080.h"
#include "i80286/i80286.h"

enum {
	CPU_NONE = 0,		// dummy
	CPU_Z80,		// ZX, MSX
	CPU_I8080,
	CPU_I80286,
	CPU_LR35902,		// GB, GBC
	CPU_6502,		// NES, Commodore
	CPU_VM1			// BK
};

struct CPU {
	unsigned halt:1;		// cpu halted, undo on interrput
	unsigned resPV:1;		// Z80: reset PV flag on INT
	unsigned noint:1;		// Z80: don't handle INT after EI
	unsigned wait:1;		// ALL: WAIT signal (dummy 1T)
	unsigned ack:1;			// Z80: acknowledge INT after execution (prevent last-1T INT)
	unsigned blk:1;			// Z80: 1 during ldxr/cpxr
	unsigned blkio:1;		// Z80: 1 during inxr/otxr
	unsigned lock:1;		// LR35902: CPU locked
	unsigned stop:1;		// LR35902: CPU stoped, unlock on keypress
	unsigned speed:1;		// LR35902: double speed mode (TODO)
	unsigned speedrq:1;		// LR35902: request speed change after STOP command
	unsigned dihalt:1;		// LR35902: HALT when DI: repeat next opcode
	unsigned sta:1;			// MOS6502: don't add 1T on (ABSX,ABSY,INDY)
	unsigned nod:2;			// MOS6502: ignore flag D in ADC/SBC; PDP11: write flags

	int type;			// cpu type id
	int gen;			// cpu generation (for x86: 0-8086, 1-80186, 2-80286 etc)

	int intrq;			// interrupts request. each bit for each INT type, 1 = requested
	int inten;			// interrupts enabled mask
	unsigned short intvec;
	int errcod;			// 80286: interrupt error code (-1 if not present)

// z80, lr35902, i8080, 6502 registers
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

	unsigned char a;
#if 1
	union {
		unsigned int f;		// 32-bit value
		z80flag_t fz;		// bits for Z80
		lrflag_t fl;		// bits for LR35902
		mosflag_t fm;		// bits for MOS6502
		i80flag_t fi;		// bits for i8080
		vm1flag_t fv;		// bits for 1801vm1
		x86flag_t fx;		// bits for 80286
	};
//	unsigned short f;
#else
	struct {
		unsigned c:1;		// all: carry
		unsigned n:1;		// z80: substraction
		unsigned p:1;		// all: parity z80:+overflow
		unsigned f3:1;		// z80: f3
		union {
			unsigned h:1;	// all: half-carry
			unsigned a:1;	// x86: half-carry
		};
		unsigned f5:1;		// z80: f5
		unsigned z:1;		// all: zero
		unsigned s:1;		// all: sign
		unsigned b:1;		// 6502: break;
		unsigned t:1;		// vm1,x86: trap
		unsigned i:1;		// x86, 6502: interrrupt
		unsigned d:1;		// x86: direction, 6502:bcd mode
		unsigned o:1;		// x86: overflow
		union {
			unsigned ip:2;		// x86: iopl
			unsigned im:2;		// z80: int mode
		}
		unsigned f7:1;		// vm1: unnamed flags
		unsigned f10:1;
		unsigned f11:1;
	} f;
#endif
	PAIR(bc,b,c);
	PAIR(de,d,e);
	PAIR(hl,h,l);

	unsigned char a_;
	unsigned int f_;
	PAIR(bc_,b_,c_);
	PAIR(de_,d_,e_);
	PAIR(hl_,h_,l_);

// 80286 registers
	unsigned wrd:1;		// i/o: out word
	PAIR(ax,ah,al);
	PAIR(dx,dh,dl);
	PAIR(cx,ch,cl);
	PAIR(bx,bh,bl);
	unsigned short bp;
	unsigned short si;
	unsigned short di;
	// unsigned short sp;	// use sp above
	// unsigned short flag;	// use f above
	// unsigned short ip;	// use pc above
	unsigned short msw;
	// segment registers (+hidden parts)
	xSegPtr cs;		// cs value,flag,base,limit
	xSegPtr ss;		// ss value,flag,base,limit
	xSegPtr ds;		// ds value,flag,base,limit
	xSegPtr es;		// es value,flag,base,limit
	xSegPtr gdtr;		// gdt (40 bits:base,limit)
	xSegPtr idtr;		// idt (40 bits:base,limit)
	xSegPtr ldtr;		// ldt (56 bits:idx,base,limit)
	xSegPtr tsdr;		// task register (56 bits:idx,base,limit)
	xSegPtr seg;		// operating segment (for EA and 'replace segment' prefixes)
	xSegPtr gate;		// gate for far jmp/call/int
	xSegPtr tmpdr;
	long double x87reg[9];	// x87 registers, [8] is readed from memory converted value
	unsigned x87top:3;	// x87 top of stack pos (0-7)
	unsigned short x87cr;	// x87 control register
	unsigned short x87sr;	// x87 status register
	unsigned short x87tw;	// x87 tag word
	unsigned char mod;	// 80286: mod byte (EA/reg)
	struct {xSegPtr seg; PAIR(adr,adrh,adrl); unsigned reg:1; unsigned cnt:5;} ea;
	int rep;		// 80286: repeat condition id
	jmp_buf jbuf;
	// callbacks, depend on x86 mode (real/prt)
	unsigned char(*x86fetch)(CPU*);
	unsigned char(*x86mrd)(CPU*,xSegPtr,int,unsigned short);
	void(*x86mwr)(CPU*,xSegPtr,int,unsigned short,int);
// pdp registers
	unsigned mcir:3;
	unsigned vsel:4;
//	unsigned short pflag;		// pdp11 flag = f
	unsigned short preg[8];		// pdp11 registers (preg[7] -> pc)
	xTimer timer;

// external callbacks
	cbmr mrd;
	cbmw mwr;
	cbir ird;
	cbiw iwr;
	cbiack irq;
	cbirq xirq;
	void* xptr;

// opcode
	PAIR(com, hcom, lcom);
	opCode* opTab;
	opCode* op;

// polymorph callbacks (depends on type)
	void (*reset)(CPU*);
	int (*exec)(CPU*);
	xAsmScan (*asmbl)(const char*, char*);
	xMnem (*mnem)(CPU*, int, cbdmr, void*);
	void (*getregs)(CPU*, xRegBunch*);
	void (*setregs)(CPU*, xRegBunch);

	struct cpuCore* core;

// temp
	int t;			// ticks counter
	unsigned short oldpc;
	unsigned char tmp;
	unsigned char tmpb;
	PAIR(tmpw,htw,ltw);
	PAIR(twrd,hwr,lwr);
	int tmpi;
	int tmpf;
};

struct cpuCore {
	int type;				// cpu type
	const char* name;			// printable name
	xRegDsc* rdsctab;			// registers descriptors table
	void (*init)(CPU*);			// call it when core changed
	void (*reset)(CPU*);			// reset
	int (*exec)(CPU*);			// exec opcode, return T
	xAsmScan (*asmbl)(const char*, char*);	// compile mnemonic
	xMnem (*mnem)(CPU*, int, cbdmr, void*);
	void (*getregs)(CPU*,xRegBunch*);	// get cpu registers: name,id,value
	void (*setregs)(CPU*,xRegBunch);	// set cpu registers
};
typedef struct cpuCore cpuCore;

CPU* cpuCreate(int,cbmr,cbmw,cbir,cbiw,cbiack,cbirq,void*);
void cpuDestroy(CPU*);
void cpuSetType(CPU*, int);

int getCoreID(const char*);
const char* getCoreName(int);

extern cpuCore cpuTab[];

xMnem cpuDisasm(CPU*, int, char*, cbdmr, void*);
int cpuAsm(CPU*, const char*, char*, unsigned short);
xAsmScan scanAsmTab(const char*, opCode*);

xRegBunch cpuGetRegs(CPU*);
void cpuSetRegs(CPU*, xRegBunch);
int cpu_get_reg(CPU*, const char*, bool*);
bool cpu_set_reg(CPU*, const char*, int);

int parity(int);

#ifdef __cplusplus
}
#endif
