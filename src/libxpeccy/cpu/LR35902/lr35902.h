#pragma once

#define FLZ 0x80
#define FLN 0x40
#define FLH 0x20
#define FLC 0x10

#include "../cpu.h"

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

xAsmScan lr_asm(const char*, char*);
xMnem lr_mnem(CPU*, int, cbdmr, void*);

void lr_get_regs(CPU*, xRegBunch*);
void lr_set_regs(CPU*, xRegBunch);
