#pragma once

#include "../cpu.h"

#define regA regs[0].l
#define regX regs[4].l
#define regY regs[5].l
#define regS regs[6].l		// stack = 0x100 | regS
#define regPC regs[7].w
#define regPCh regs[7].h
#define regPCl regs[7].l
#define regWZ regs[8].w
#define regWZh regs[8].h
#define regWZl regs[8].l

#define flgC	flags[0]
#define flgZ	flags[1]
#define flgI	flags[2]
#define flgD	flags[3]
#define flgB	flags[4]
#define flgF5	flags[5]
#define flgV	flags[6]
#define flgN	flags[7]

enum {
	M6502_REG_PC = 1,
	M6502_REG_A,
	M6502_REG_F,
	M6502_REG_S,
	M6502_REG_X,
	M6502_REG_Y
};

//#define MFN 0x80	// negative
//#define MFV 0x40	// b7 carry
//#define MF5 0x20	// =1
//#define MFB 0x10	// break
//#define MFD 0x08	// bcd mode
//#define MFI 0x04	// interrupt mask
//#define MFZ 0x02	// zero
// #define MFC 0x01	// carry

/*
#ifdef WORDS_LITTLE_ENDIAN
typedef struct {
	unsigned c:1;
	unsigned z:1;
	unsigned i:1;
	unsigned d:1;
	unsigned b:1;
	unsigned f5:1;
	unsigned v:1;
	unsigned n:1;
} mosflag_t;
#else
typedef struct {
	unsigned n:1;
	unsigned v:1;
	unsigned f5:1;
	unsigned b:1;
	unsigned d:1;
	unsigned i:1;
	unsigned z:1;
	unsigned c:1;
} mosflag_t;
#endif
*/

#define MOS6502_INT_NMI	1
#define MOS6502_INT_RES	2
#define MOS6502_INT_IRQ	4

void m6502_reset(CPU*);
int m6502_exec(CPU*);
int m6502_int(CPU*);

xMnem m6502_mnem(CPU*, int, cbdmr, void*);
xAsmScan m6502_asm(int, const char*, char*);

void m6502_get_regs(CPU*, xRegBunch*);
void m6502_set_regs(CPU*, xRegBunch);

void mos_set_flag(CPU*, int);
int mos_get_flag(CPU*);
