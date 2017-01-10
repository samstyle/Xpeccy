#include <stdlib.h>
#include <stdio.h>

#include "../cpu.h"

//#include "lr_pref_cb.c"
//#include "lr_nopref.c"

extern opCode lrTab[256];
extern opCode lrcbTab[256];

void lr_reset(CPU* cpu) {
	cpu->pc = 0;
	cpu->af = cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->sp = 0xffff;
	cpu->lock = 0;
	// not necessary
	cpu->imode = 0;
	cpu->af_ = cpu->bc_ = cpu->de_ = cpu->hl_ = 0xffff;
	cpu->ix = cpu->iy = 0xffff;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->i = cpu->r = cpu->r7 = 0;
}

int lr_exec(CPU* cpu) {
	if (cpu->lock) return 1;
	cpu->t = 0;
	cpu->noint = 0;
	cpu->opTab = lrTab;
	do {
		cpu->tmp = cpu->mrd(cpu->pc++, 1, cpu->data);
		cpu->op = &cpu->opTab[cpu->tmp];
		cpu->t += cpu->op->t;
		cpu->op->exec(cpu);
	} while (cpu->op->prefix);
	return cpu->t;
}

int lr_int(CPU* cpu) {
	if (!cpu->iff1 || cpu->noint) return 0;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	if (cpu->halt) {
		cpu->halt = 0;
		cpu->pc++;
	}
	RST(cpu->inta);
	return 15;		// TODO: to know how much T eats INT handle
}

int lr_nmi(CPU* cpu) {
	return 0;
}

// disasm



// asm

xAsmScan lr_asm(const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, lrTab);
	res.ptr = buf;
	if (!res.match) {
		*res.ptr++ = 0xcb;
		res = scanAsmTab(cbuf, lrcbTab);
	}
	if (res.match) {
		*res.ptr++ = res.idx;
	}
	return res;
}

xMnem lr_mnem(unsigned short adr, cbdmr mrd, void* data) {
	int res = 0;
	opCode* opt = lrTab;
	opCode* opc;
	unsigned char op;
	do {
		op = mrd(adr++,data);
		res++;
		opc = &opt[op];
		if (opc->prefix) opt = opc->tab;
	} while (opc->prefix);
	xMnem mn;
	mn.fetch = res;
	mn.mnem = opc->mnem;
	return mn;
}
