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

// special register id
#define REG_NONE	0		// end of table
#define REG_EMPTY	-1		// don't show in debuga, but is a register
// register type
#define REG_BIT		1
#define REG_2		2		// special type, values 0,1,2 (z80 interrupt mode)
#define REG_BYTE	8
#define REG_WORD	16
#define REG_24		24
#define REG_32		32
//#define REG_TMASK	0xff
// register flags
#define REG_RO		1	// protect from changes in deBUGa
#define REG_SEG		(1<<1)	// register is segment
#define REG_RDMP	(1<<2)	// use register as line addr for regs-dump in deBUGa (new widget)
#define REG_PC		(1<<3)	// register is execution pointer (pc, ip)
#define REG_SP		(1<<4)	// register is stack (sp)

typedef struct {
	int id;
	int type;
	int flag;
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
	int flag;
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
#define OF_GEN		(1<<10)		// opcode depends on cpu generation (86/186/286 or vm1/vm2) cpu->tab is sub-table[cpu->gen]
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

enum {
	CPU_NONE = 0,		// dummy
	CPU_Z80,		// ZX, MSX, *PC88xx
	CPU_I8080,
	CPU_I8086,		// *PC98xx
	CPU_I80186,
	CPU_I80286,		// IBM
	CPU_LR35902,		// GB, GBC
	CPU_6502,		// NES, Commodore
	CPU_VM1,		// BK
	CPU_VM2
};

#define flgTMP flags[63]
#define flgHALT	flags[62]		// cpu halted, undo on interrput
//#define flgResPV flags[61]		// Z80: reset PV flag on INT
#define flgNOINT flags[60]		// Z80: don't handle INT after EI
#define flgWAIT	flags[59]		// ALL: WAIT signal (dummy 1T)
#define flgACK	flags[58]		// Z80: acknowledge INT after execution (prevent last-1T INT)
#define flgLOCK flags[57]		// LR35902: CPU locked

struct CPU {
	// common part
	int type;			// cpu type id
	int gen;			// cpu generation (for x86: 0-8086, 1-80186, 2-80286 etc)
	int intrq;			// interrupts request. each bit for each INT type, 1 = requested
	int inten;			// interrupts enabled mask
	int intoc;			// occured interrupts (for lr35902)
	int intvec;			// interrupt vector (internal/external)
	int errcod;			// error code (-1 if not present)
	int adr;			// address bus for using from outside
	int t;				// ticks counter
	unsigned short oldpc;		// address of current instruction
	// if cpu is from external lib
	unsigned lib:1;			// cpu core from exernal lib
	char* libname;			// name of lib inside libs folder
	void* libhnd;			// lib handler (dlopen)
	// external callbacks
	cbmr mrd;			// memory reading
	cbmw mwr;			// memeory writing
	cbir ird;			// i/o reading
	cbiw iwr;			// i/o writing
	cbiack xack;			// interrupt vector acknowledge
	cbirq xirq;			// send signal
	void* xptr;			// pointer to external data (almost always Computer*)
	// core: runtime callbacks (depends on type)
	struct cpuCore* core;
	// opcode
	reg16(com, hcom, lcom);
	opCode* opTab;
	opCode* op;
	// common registers block
	xreg32 regs[64];
	// TODO: common flags (unnamed, must be defined same way as registers). replace cpuFlags with it
	bool flags[64];
	// temp
	unsigned char tmp;
	unsigned char tmpb;
	reg16(tmpw,htw,ltw);
	reg16(twrd,hwr,lwr);
	int tmpi;
	jmp_buf jbuf;			// for throws
// internal timer (for vm1/2)
	xTimer timer;
// x86/87
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
	struct {xSegPtr seg; reg16(adr,adrh,adrl); unsigned reg:1; unsigned cnt:5;} ea;
	long double x87reg[9];	// x87 registers, [8] is readed from memory converted value
	// callbacks, depend on x86 mode (real/prt)
	unsigned char(*x86fetch)(CPU*);
	unsigned char(*x86mrd)(CPU*,xSegPtr,int,unsigned short);
	void(*x86mwr)(CPU*,xSegPtr,int,unsigned short,int);
};

struct cpuCore {
	int type;				// cpu type
	int gen;				// cpu generation
	const char* name;			// printable name
	xRegDsc* rdsctab;			// registers descriptors table
	void (*init)(CPU*);			// call it when core changed
	void (*reset)(CPU*);			// reset
	int (*exec)(CPU*);			// exec opcode, return T
	xAsmScan (*asmbl)(int,const char*, char*);	// compile mnemonic (adr,src.text,result.buf)
	xMnem (*mnem)(CPU*, int, cbdmr, void*);
	void (*getregs)(CPU*,xRegBunch*);	// get cpu registers: name,id,value
	void (*setregs)(CPU*,xRegBunch);	// set cpu registers
	int (*getflag)(CPU*);			// get flag value
	void (*setflag)(CPU*,int);		// set flags from value
};
typedef struct cpuCore cpuCore;

CPU* cpuCreate(int,cbmr,cbmw,cbir,cbiw,cbiack,cbirq,void*);
void cpuDestroy(CPU*);
int cpu_set_type(CPU*, const char*, const char*, const char*);

void cpu_reset(CPU*);
int cpu_exec(CPU*);

// built-in cores tab
extern cpuCore cpuTab[];

xMnem cpuDisasm(CPU*, int, char*, cbdmr, void*);
int cpuAsm(CPU*, const char*, char*, unsigned short);
xAsmScan scanAsmTab(const char*, opCode*);

xRegBunch cpuGetRegs(CPU*);
xRegister cpuGetReg(CPU*, int);
void cpuSetRegs(CPU*, xRegBunch);
int cpu_get_reg(CPU*, const char*, bool*);
int cpu_get_pc(CPU*);
int cpu_get_sp(CPU*);
bool cpu_set_reg(CPU*, const char*, int);
int cpu_get_flag(CPU*);
void cpu_set_flag(CPU*, int);

int parity(int);

int cpu_fetch(CPU*, int);
int cpu_mrd(CPU*, int);
void cpu_mwr(CPU*, int, int);
int cpu_ird(CPU*, int);
void cpu_iwr(CPU*, int, int);
void cpu_irq(CPU*, int);

#ifdef __cplusplus
}
#endif
