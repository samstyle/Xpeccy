#pragma once

#include "../cpu.h"

/*
#define FLZ 0x80
#define FLN 0x40
#define FLH 0x20
#define FLC 0x10
*/

#define regA regs[0].l
#define regBC regs[1].w
#define regB regs[1].h
#define regC regs[1].l
#define regDE regs[2].w
#define regD regs[2].h
#define regE regs[2].l
#define regHL regs[3].w
#define regH regs[3].h
#define regL regs[3].l
#define regIX regs[4].w
#define regIXh regs[4].h
#define regIXl regs[4].l
#define regIY regs[5].w
#define regIYh regs[5].h
#define regIYl regs[5].l
#define regSP regs[6].w
#define regSPh regs[6].h
#define regSPl regs[6].l
#define regPC regs[7].w
#define regPCh regs[7].h
#define regPCl regs[7].l
#define regWZ regs[8].w
#define regWZh regs[8].h
#define regWZl regs[8].l
#define regI regs[9].l
#define regR regs[9].h
#define regR7 regs[9].ih
#define regAa regs[0].h
#define regBCa regs[1].ih
#define regDEa regs[2].ih
#define regHLa regs[3].ih

#ifdef WORDS_BIG_ENDIAN
typedef struct {
	unsigned _nu:24;
	unsigned z:1;
	unsigned n:1;
	unsigned h:1;
	unsigned c:1;
	unsigned _nu0:4;
} lrflags_t;
#else
typedef struct {
	unsigned _nu0:4;	// b0..3 not used
	unsigned c:1;
	unsigned h:1;
	unsigned n:1;
	unsigned z:1;
	unsigned _nu:24;	// padding
} lrflag_t;
#endif

enum {
	LR_REG_PC = 1,
	LR_REG_SP,
	LR_REG_AF,
	LR_REG_BC,
	LR_REG_DE,
	LR_REG_HL
};

void lr_reset(CPU*);
int lr_exec(CPU*);

xAsmScan lr_asm(int, const char*, char*);
xMnem lr_mnem(CPU*, int, cbdmr, void*);

void lr_get_regs(CPU*, xRegBunch*);
void lr_set_regs(CPU*, xRegBunch);

int lr_get_flag(CPU*);
void lr_set_flag(CPU*, int);

// internal

int lr_mrd(CPU*, int);
void lr_mwr(CPU*, int, int);
