#pragma once

#include "../cpu.h"

#define flgC	flags[4]
#define flgH	flags[5]
#define flgN	flags[6]
#define flgZ	flags[7]
#define flgIFF1	flags[8]		// interrupts enabled
#define flgDIHALT flags[9]		// HALT when DI: repeat next opcode
#define flgSTOP flags[10]		// CPU stoped, unlock on keypress
#define flgIFFC flags[11]		// need to change IFF (after ei/di)
#define flgIFFN flags[12]		// new IFF

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
#define regSP regs[6].w
#define regSPh regs[6].h
#define regSPl regs[6].l
#define regPC regs[7].w
#define regPCh regs[7].h
#define regPCl regs[7].l
#define regWZ regs[8].w
#define regWZh regs[8].h
#define regWZl regs[8].l
#define regTPC regs[8].ih	// temp pc storage
//#define regI regs[9].l
//#define regR regs[9].h
//#define regR7 regs[9].ih

enum {
	LR_REG_PC = 1,
	LR_REG_SP,
	LR_REG_AF,
	LR_REG_BC,
	LR_REG_DE,
	LR_REG_HL,
	LR_FLG_IFF,
	LR_REG_IE,
	LR_REG_IF
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
