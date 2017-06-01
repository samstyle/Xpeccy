#include "6502.h"

#include <string.h>

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
//	cpu->inth = cpu->intrq ? 1 : 0;		// if both INT happened in one time
	return 7;
}

int m6502_exec(CPU* cpu) {
	int res = 0;
	unsigned char com;
	if (cpu->intrq & cpu->inten) {
		res = m6502_int(cpu);
	} else {
		com = cpu->mrd(cpu->pc++, 1, cpu->data);
		opCode* op = &mosTab[com];
		cpu->t = op->t;			// 2T fetch
		op->exec(cpu);
		res = cpu->t;
	}
	return res;
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

xRegDsc m6502RegTab[] = {
	{M6502_REG_PC, "PC"},
	{M6502_REG_S, "S"},
	{M6502_REG_AF, "AF"},
	{M6502_REG_X, "X"},
	{M6502_REG_Y, "Y"},
	{REG_NONE, ""}
};

void m6502_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	while(m6502RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = m6502RegTab[idx].id;
		strncpy(bunch->regs[idx].name, m6502RegTab[idx].name, 7);
		switch(m6502RegTab[idx].id) {
			case M6502_REG_PC: bunch->regs[idx].value = cpu->pc; break;
			case M6502_REG_S: bunch->regs[idx].value = cpu->lsp; break;
			case M6502_REG_AF: bunch->regs[idx].value = cpu->af; break;
			case M6502_REG_X: bunch->regs[idx].value = cpu->lx; break;
			case M6502_REG_Y: bunch->regs[idx].value = cpu->ly; break;
		}
		idx++;
	}
	bunch->regs[idx].id = REG_NONE;
}

void m6502_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx;
	for (idx = 0; idx < 32; idx++) {
		switch(bunch.regs[idx].id) {
			case M6502_REG_PC: cpu->pc = bunch.regs[idx].value; break;
			case M6502_REG_S: cpu->lsp = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_AF: cpu->af = bunch.regs[idx].value; break;
			case M6502_REG_X: cpu->lx = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_Y: cpu->ly = bunch.regs[idx].value & 0xff; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
