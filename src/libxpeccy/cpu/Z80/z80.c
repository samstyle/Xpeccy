#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../cpu.h"
// #include "z80tables.c"
// #include "z80macros.h"

extern opCode npTab[256];
extern opCode ddTab[256];
extern opCode fdTab[256];
extern opCode cbTab[256];
extern opCode edTab[256];
//extern opCode ddcbTab[256];
//extern opCode fdcbTab[256];

#include "z80nop.c"
#include "z80ed.c"
#include "z80ddcb.c"
#include "z80fdcb.c"
#include "z80dd.c"
#include "z80fd.c"
#include "z80cb.c"

void z80_reset(CPU* cpu) {
	cpu->pc = 0;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->imode = 0;
	cpu->af = cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->af_ = cpu->bc_ = cpu->de_ = cpu->hl_ = 0xffff;
	cpu->ix = cpu->iy = 0xffff;
	cpu->sp = 0xffff;
	cpu->i = cpu->r = cpu->r7 = 0;
}

int z80_exec(CPU* cpu) {
	cpu->t = 0;
	cpu->noint = 0;
	cpu->opTab = npTab;
	do {
		cpu->op = &cpu->opTab[cpu->mrd(cpu->pc++,1,cpu->data)];
		cpu->r++;
		cpu->t += cpu->op->t;
		cpu->op->exec(cpu);
	} while (cpu->op->prefix);
	return cpu->t;
}

int z80_int(CPU* cpu) {
	if (!cpu->iff1 || cpu->noint) return 0;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	if (cpu->halt) {
		cpu->pc++;
		cpu->halt = 0;
	}
	if (cpu->resPV) {
		cpu->f &= ~FP;
		cpu->resPV = 0;
	}
	cpu->opTab = npTab;
	switch(cpu->imode) {
		case 0:
			cpu->t = 2;
			cpu->op = &cpu->opTab[cpu->irq(cpu->data)];
			cpu->r++;
			cpu->t += cpu->op->t;		// +5 (RST38 fetch)
			cpu->op->exec(cpu);		// +3 +3 execution. 13 total
			while (cpu->op->prefix) {
				cpu->op = &cpu->opTab[cpu->mrd(cpu->pc++,1,cpu->data)];
				cpu->r++;
				cpu->t += cpu->op->t;
				cpu->op->exec(cpu);
			}
			break;
		case 1:
			cpu->r++;
			cpu->t = 2 + 5;	// 2 extra + 5 on RST38 fetch
			nprFF(cpu);	// +3 +3 execution. 13 total
			break;
		case 2:
			cpu->r++;
			cpu->t = 7;
			PUSH(cpu->hpc,cpu->lpc);	// +3 (10) +3 (13)
			cpu->lptr = cpu->irq(cpu->data);	// int vector (FF)
			cpu->hptr = cpu->i;
			cpu->lpc = MEMRD(cpu->mptr++,3);	// +3 (16)
			cpu->hpc = MEMRD(cpu->mptr,3);		// +3 (19)
			cpu->mptr = cpu->pc;
			break;
	}
	return cpu->t;
}

int z80_nmi(CPU* cpu) {
	if (cpu->noint) return 0;
	cpu->r++;
	cpu->iff1 = 0;
	cpu->t = 5;
	PUSH(cpu->hpc,cpu->lpc);
	cpu->pc = 0x0066;
	cpu->mptr = cpu->pc;
	return cpu->t;		// always 11
}

// disasm

xMnem z80_mnem(unsigned short adr, cbdmr mrd, void* data) {
	int res = 0;
	opCode* opt = npTab;
	opCode* opc;
	unsigned char op;
//	unsigned char tmp;
	do {
		op = mrd(adr++,data);
		res++;
		opc = &opt[op];
		if (opc->prefix) {
			opt = opc->tab;
			if ((opt == ddcbTab) || (opt == fdcbTab)) {
				adr++;
				res++;
			}
		}
	} while (opc->prefix);
	xMnem mn;
	mn.fetch = res;
	mn.mnem = opc->mnem;
	return mn;
}

// asm

xAsmScan z80_asm(const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, npTab);
	res.ptr = buf;
	if (!res.match) {
		*res.ptr++ = 0xdd;
		res = scanAsmTab(cbuf, ddTab);
	}
	if (!res.match) {
		res.ptr = buf;
		*res.ptr++ = 0xfd;
		res = scanAsmTab(cbuf, fdTab);
	}
	if (!res.match) {
		res.ptr = buf;
		*res.ptr++ = 0xcb;
		res = scanAsmTab(cbuf, cbTab);
	}
	if (!res.match) {
		res.ptr = buf;
		*res.ptr++ = 0xed;
		res = scanAsmTab(cbuf, edTab);
	}
	if (!res.match) {
		res.ptr = buf;
		*res.ptr++ = 0xdd;
		*res.ptr++ = 0xcb;
		res = scanAsmTab(cbuf, ddcbTab);
	}
	if (!res.match) {
		res.ptr = buf;
		*res.ptr++ = 0xfd;
		*res.ptr++ = 0xcb;
		res = scanAsmTab(cbuf, fdcbTab);
	}
	if (res.match) {
		*res.ptr++ = res.idx;
	}
	return res;
}
