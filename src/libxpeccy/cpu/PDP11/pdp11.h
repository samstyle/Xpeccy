#ifndef X_PDP11_H
#define X_PDP11_H

#include "../cpu.h"

enum {
	PDP11_REG0 = 1,
	PDP11_REG1,
	PDP11_REG2,
	PDP11_REG3,
	PDP11_REG4,
	PDP11_REG5,
	PDP11_REG6,
	PDP11_REG7,
	PDP11_REGF
};

void pdp11_reset(CPU*);
int pdp11_exec(CPU*);
int pdp11_int(CPU*);

xMnem pdp11_mnem(CPU*, unsigned short, cbdmr, void*);
xAsmScan pdp11_asm(const char*, char*);

void pdp11_get_regs(CPU*, xRegBunch*);
void pdp11_set_regs(CPU*, xRegBunch);

#endif
