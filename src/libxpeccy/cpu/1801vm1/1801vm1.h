#pragma once

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

#define regWZ regs[8].w
#define regRN(_n) regs[_n].w

// external signals, working by IOWR
enum {
	PDP11_INIT = 1
};

/*
#define PDP_FC	(1 << 0)
#define PDP_FV	(1 << 1)
#define	PDP_FZ	(1 << 2)
#define PDP_FN	(1 << 3)
#define PDP_FT	(1 << 4)
#define PDP_F7	(1 << 7)
#define PDP_F10	(1 << 10)
#define PDP_F11 (1 << 11)
*/

#ifdef WORDS_LITTLE_ENDIAN
typedef struct {
	unsigned c:1;
	unsigned v:1;
	unsigned z:1;
	unsigned n:1;
	unsigned t:1;
	unsigned _nu5:2;
	unsigned f7:1;
	unsigned _nu8:2;
	unsigned f10:1;
	unsigned f11:1;
	unsigned _nu:20;
} vm1flag_t;
#else
typedef struct {
	unsigned _nu:20;
	unsigned f11:1;
	unsigned f10:1;
	unsigned _nu8:2;
	unsigned f7:1;
	unsigned _nu5:2;
	unsigned t:1;
	unsigned n:1;
	unsigned z:1;
	unsigned v:1;
	unsigned c:1;
} vm1flag_t;
#endif

typedef struct {
	unsigned short mask;
	unsigned short code;
	int flag;
	const char* mnem;
} xPdpDasm;

#define PDP_INT_IRQ1	(1 << 0)
#define PDP_INT_IRQ2	(1 << 1)
#define PDP_INT_IRQ3	(1 << 2)
#define PDP_INT_VIRQ	(1 << 3)
#define PDP_INT_TIMER	(1 << 4)

void pdp11_reset(CPU*);
int pdp11_exec(CPU*);

void vm1_init(CPU*);
void vm2_init(CPU*);

xMnem pdp11_mnem(CPU*, int, cbdmr, void*);
xAsmScan pdp11_asm(int, const char*, char*);

void pdp11_get_regs(CPU*, xRegBunch*);
void pdp11_set_regs(CPU*, xRegBunch);
void pdp_set_flag(CPU*, int);
int pdp_get_flag(CPU*);
