#pragma once

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

// external signals, working by IOWR
enum {
	PDP11_INIT = 1
};

#define PDP_FC	(1 << 0)
#define PDP_FV	(1 << 1)
#define	PDP_FZ	(1 << 2)
#define PDP_FN	(1 << 3)
#define PDP_FT	(1 << 4)
#define PDP_F7	(1 << 7)
#define PDP_F10	(1 << 10)
#define PDP_F11 (1 << 11)

#define PDP_INT_IRQ1	(1 << 0)
#define PDP_INT_IRQ2	(1 << 1)
#define PDP_INT_IRQ3	(1 << 2)
#define PDP_INT_VIRQ	(1 << 3)
#define PDP_INT_TIMER	(1 << 4)

#include "../cpu.h"

void pdp11_reset(CPU*);
int pdp11_exec(CPU*);
// int pdp11_int(CPU*);

xMnem pdp11_mnem(CPU*, int, cbdmr, void*);
xAsmScan pdp11_asm(const char*, char*);

void pdp11_get_regs(CPU*, xRegBunch*);
void pdp11_set_regs(CPU*, xRegBunch);
