#ifndef X_Z80_H
#define X_Z80_H

#include "../cpu.h"
#include "z80_macro.h"

// typedef struct CPU CPU;

enum {
	Z80_REG_PC = 1,
	Z80_REG_SP,
	Z80_REG_AF,
	Z80_REG_BC,
	Z80_REG_DE,
	Z80_REG_HL,
	Z80_REG_AFA,
	Z80_REG_BCA,
	Z80_REG_DEA,
	Z80_REG_HLA,
	Z80_REG_IX,
	Z80_REG_IY,
	Z80_REG_I,
	Z80_REG_R
};

#define FS	0x80
#define	FZ	0x40
#define	F5	0x20
#define	FH	0x10
#define	F3	0x08
#define	FP	0x04
#define	FV	FP
#define	FN	0x02
#define	FC	0x01

#define Z80_INT	1
#define Z80_NMI	(1<<1)

// typedef struct CPU CPU;

void z80_reset(CPU*);
int z80_exec(CPU*);
//int z80_int(CPU*);
// int z80_nmi(CPU*);
xAsmScan z80_asm(const char*, char*);
xMnem z80_mnem(CPU*, unsigned short, cbdmr, void*);

void z80_get_regs(CPU*, xRegBunch*);
void z80_set_regs(CPU*, xRegBunch);

#endif
