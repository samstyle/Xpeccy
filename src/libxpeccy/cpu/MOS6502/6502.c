#include "6502.h"

extern opCode mosTab[256];

void m6502_reset(CPU* cpu) {
	cpu->hsp = 1;		// segment 01xx is stack

}

void m6502_push_int(CPU* cpu) {
	cpu->mwr(cpu->sp, cpu->hpc, cpu->data);
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->lpc, cpu->data);
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->f, cpu->data);
	cpu->lsp--;
}

int m6502_int(CPU* cpu) {
	if (cpu->intrq & 2) {		// NMI (VBlank)
		cpu->intrq &= ~2;
		m6502_push_int(cpu);
		cpu->pc = 0xfffa;
	} else if (cpu->intrq & 1) {	// BRK
		cpu->intrq &= ~1;
		m6502_push_int(cpu);
		cpu->pc = 0xfffe;
	}
	cpu->inth = cpu->intrq ? 1 : 0;		// if both INT happened in one time
	return 7;
}

int m6502_exec(CPU* cpu) {
	unsigned char com = cpu->mrd(cpu->pc++, 1, cpu->data);
	opCode* op = &mosTab[com];
	cpu->t = op->t;			// 2T fetch
	op->exec(cpu);
	return cpu->t;
}

xMnem m6502_mnem(CPU* cpu, unsigned short adr, cbdmr mrd, void* data) {
	xMnem mn;
	unsigned char op = mrd(adr++,data);
	mn.len = 1;
	mn.mnem = mosTab[op].mnem;
	mn.cond = 0;		// TODO
	mn.mem = 0;
	return mn;
}

xAsmScan m6502_asm(const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, mosTab);
	res.ptr = buf;
	if (res.match) {
		*res.ptr++ = res.idx;
	}
	return res;
}
