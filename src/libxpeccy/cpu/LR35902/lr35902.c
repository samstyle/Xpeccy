#include <stdlib.h>
#include <stdio.h>

#include "../cpu.h"

#include "lr_pref_cb.c"
#include "lr_nopref.c"

void lr_reset(CPU* cpu) {
	cpu->pc = 0x100;
	cpu->af = cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->sp = 0xfff0;
	cpu->lock = 0;
}

int lr_exec(CPU* cpu) {
	if (cpu->lock) return 1;
	cpu->t = 0;
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
	return 0;
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
