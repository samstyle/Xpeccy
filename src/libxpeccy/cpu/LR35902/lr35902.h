#pragma once

/*
#define FLZ 0x80
#define FLN 0x40
#define FLH 0x20
#define FLC 0x10
*/

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

#include "../cpu.h"

void lr_reset(CPU*);
int lr_exec(CPU*);

xAsmScan lr_asm(int, const char*, char*);
xMnem lr_mnem(CPU*, int, cbdmr, void*);

void lr_get_regs(CPU*, xRegBunch*);
void lr_set_regs(CPU*, xRegBunch);

// internal

int lr_mrd(CPU*, int);
void lr_mwr(CPU*, int, int);
