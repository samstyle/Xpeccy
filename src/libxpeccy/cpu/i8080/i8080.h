#pragma once

#include "../cpu.h"

#define I8080_INT	1

#define IFL_S	0x80
#define	IFL_Z	0x40
#define IFL_5	0x20
#define IFL_A	0x10
#define	IFL_3	0x08
#define IFL_P	0x04
#define IFL_1	0x02
#define IFL_C	0x01

enum {
	I8080_REG_PC = 1,
	I8080_REG_SP,
	I8080_REG_AF,
	I8080_REG_BC,
	I8080_REG_DE,
	I8080_REG_HL,
};

extern opCode i8080_tab[256];

void i8080_reset(CPU*);
int i8080_exec(CPU*);
xAsmScan i8080_asm(const char*, char*);
xMnem i8080_mnem(CPU*, unsigned short, cbdmr, void*);
void i8080_get_regs(CPU*, xRegBunch*);
void i8080_set_regs(CPU*, xRegBunch);
