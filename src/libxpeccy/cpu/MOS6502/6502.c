#include "6502.h"

#include <string.h>
#include <stdio.h>

extern opCode mosTab[256];

void m6502_reset(CPU* cpu) {
	cpu->lock = 0;
	cpu->intrq = 0;
	cpu->inten = MOS6502_INT_IRQ | MOS6502_INT_NMI;		// brk/nmi enabled
	cpu->sp = 0x1fd;	// segment 01xx is stack
	cpu->f = MF5 | MFI;
	cpu->a = 0;
	cpu->lx = 0;
	cpu->ly = 0;
	cpu->lpc = cpu->mrd(0xfffc, 0, cpu->data);
	cpu->hpc = cpu->mrd(0xfffd, 0, cpu->data);
//	printf("mos pc = %.4X\n", cpu->pc);
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
	if (cpu->intrq & MOS6502_INT_NMI) {		// NMI (VBlank)
		cpu->intrq &= ~MOS6502_INT_NMI;
		m6502_push_int(cpu);
		cpu->lpc = cpu->mrd(0xfffa, 0, cpu->data);
		cpu->hpc = cpu->mrd(0xfffb, 0, cpu->data);
	} else if (cpu->intrq & MOS6502_INT_IRQ) {	// IRQ
		cpu->intrq &= ~MOS6502_INT_IRQ;
		cpu->f &= ~MFB;				// reset B flag
		cpu->f |= MFI;
		m6502_push_int(cpu);
		cpu->lpc = cpu->mrd(0xfffe, 0, cpu->data);
		cpu->hpc = cpu->mrd(0xffff, 0, cpu->data);
	}
	return 7;				// real: 7T
}

int m6502_exec(CPU* cpu) {
	int res = 0;
	if (cpu->lock) return 1;
	unsigned char com;
	cpu->intrq &= cpu->inten;
	if (cpu->intrq && !cpu->noint) {
		res = m6502_int(cpu);
	} else {
		cpu->noint = 0;
		com = cpu->mrd(cpu->pc++, 1, cpu->data);
		opCode* op = &mosTab[com];
		cpu->t = op->t;
		cpu->sta = op->prefix;
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
	{M6502_REG_PC, "PC", 0},
	{M6502_REG_A, "A", 1},
	{M6502_REG_X, "X", 1},
	{REG_EMPTY, "", 0},
	{REG_EMPTY, "", 0},

	{M6502_REG_S, "S", 1},
	{M6502_REG_F, "P", 1},
	{M6502_REG_Y, "Y", 1},
	{REG_NONE, ""}
};

void m6502_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	while(m6502RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = m6502RegTab[idx].id;
		strncpy(bunch->regs[idx].name, m6502RegTab[idx].name, 7);
		bunch->regs[idx].byte = m6502RegTab[idx].byte;
		switch(m6502RegTab[idx].id) {
			case M6502_REG_PC: bunch->regs[idx].value = cpu->pc; break;
			case M6502_REG_S: bunch->regs[idx].value = 0x100 | cpu->lsp; break;
			case M6502_REG_A: bunch->regs[idx].value = cpu->a; break;
			case M6502_REG_F: bunch->regs[idx].value = cpu->f; break;
			case M6502_REG_X: bunch->regs[idx].value = cpu->lx; break;
			case M6502_REG_Y: bunch->regs[idx].value = cpu->ly; break;
		}
		idx++;
	}
	memcpy(bunch->flags, "NV-BDIZC", 8);
	bunch->regs[idx].id = REG_NONE;
}

void m6502_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx;
	for (idx = 0; idx < 32; idx++) {
		switch(bunch.regs[idx].id) {
			case M6502_REG_PC: cpu->pc = bunch.regs[idx].value; break;
			case M6502_REG_S: cpu->sp = 0x0100 | (bunch.regs[idx].value & 0xff); break;
			case M6502_REG_A: cpu->a = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_F: cpu->f = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_X: cpu->lx = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_Y: cpu->ly = bunch.regs[idx].value & 0xff; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
