#include "6502.h"

#include <string.h>
#include <stdio.h>

extern opCode mosTab[256];

void m6502_reset(CPU* cpu) {
	cpu->lock = 0;
	cpu->intrq = 0;
	cpu->inten = MOS6502_INT_IRQ | MOS6502_INT_NMI;	// brk/nmi enabled. irq is allways enabled, controlled by I flag
	cpu->sp = 0x1fd;				// segment 01xx is stack
	cpu->f = 0x24; // MF5 | MFI;
	cpu->a = 0;
	cpu->lx = 0;
	cpu->ly = 0;
	cpu->lpc = cpu->mrd(0xfffc, 0, cpu->xptr);
	cpu->hpc = cpu->mrd(0xfffd, 0, cpu->xptr);
//	printf("mos pc = %.4X\n", cpu->pc);
}

void m6502_push_int(CPU* cpu) {
	cpu->mwr(cpu->sp, cpu->hpc, cpu->xptr);
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->lpc, cpu->xptr);
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->f, cpu->xptr);
	cpu->lsp--;
}

int m6502_int(CPU* cpu) {
	if (cpu->intrq & MOS6502_INT_NMI) {		// NMI (VBlank)
		cpu->intrq &= ~MOS6502_INT_NMI;
		m6502_push_int(cpu);
		cpu->lpc = cpu->mrd(0xfffa, 0, cpu->xptr);
		cpu->hpc = cpu->mrd(0xfffb, 0, cpu->xptr);
	} else if (cpu->intrq & MOS6502_INT_IRQ) {	// IRQ
		cpu->intrq &= ~MOS6502_INT_IRQ;
		if (!cpu->fm.i) {			// IRQ enabled, I flag = 0
			cpu->fm.b = 0;			// reset B flag
			m6502_push_int(cpu);
			cpu->fm.i |= 1;			// disable IRQ
			cpu->lpc = cpu->mrd(0xfffe, 0, cpu->xptr);
			cpu->hpc = cpu->mrd(0xffff, 0, cpu->xptr);
		}
	}
	return 7;				// real: 7T
}

int m6502_exec(CPU* cpu) {
	int res = 0;
	if (cpu->lock) return 1;
	unsigned char com;
	cpu->intrq &= cpu->inten;
	if (cpu->fm.i)
		cpu->intrq &= ~MOS6502_INT_IRQ;
	if (cpu->intrq && !cpu->noint) {
		res = m6502_int(cpu);
	} else {
		cpu->noint = 0;
		com = cpu->mrd(cpu->pc++, 1, cpu->xptr);
		opCode* op = &mosTab[com];
		cpu->t = op->t;
		cpu->sta = op->flag & OF_EXT;
		op->exec(cpu);
		res = cpu->t;
	}
	return res;
}

// cond:
// 00x : MFN = x
// 01x : MFV = x
// 10x : MFC = x
// 11x : MFZ = x

// static unsigned char m6502_cond[4] = {MFN, MFV, MFC, MFZ};

xMnem m6502_mnem(CPU* cpu, int qadr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.oadr = -1;
	unsigned short adr = qadr & 0xffff;
	unsigned char op = mrd(adr++, data);
	mn.met = 0;
	mn.len = 1;
	mn.mnem = mosTab[op].mnem;
	mn.flag = mosTab[op].flag;
	// cond
	if ((op & 0x1f) == 0x10) {		// all branch ops; b5-7 = condition
		mn.cond = 1;
		switch ((op >> 3) & 3) {
			case 0: mn.met = !cpu->fm.n; break;
			case 1: mn.met = !cpu->fm.v; break;
			case 2: mn.met = !cpu->fm.c; break;
			case 3: mn.met = !cpu->fm.z; break;
		}
		// mn.met = (cpu->f & m6502_cond[(op >> 6) & 3]) ? 0 : 1;		// true if 0
		if (op & 0x20)							// true if 1
			mn.met ^= 1;
	} else {
		mn.cond = 0;
	}
	// mem
	mn.mem = 0;				// todo
	return mn;
}

xAsmScan m6502_asm(int adr, const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, mosTab);
	res.ptr = buf;
	if (res.match) {
		*res.ptr++ = res.idx;
	}
	return res;
}

xRegDsc m6502RegTab[] = {
	{M6502_REG_PC, "PC", REG_WORD, offsetof(CPU, pc)},
	{M6502_REG_A, "A", REG_BYTE, offsetof(CPU, a)},
	{M6502_REG_X, "X", REG_BYTE, offsetof(CPU, lx)},
	{M6502_REG_Y, "Y", REG_BYTE, offsetof(CPU, ly)},
	{M6502_REG_S, "S", REG_BYTE, offsetof(CPU, lsp)},
	{M6502_REG_F, "P", REG_32, offsetof(CPU, f)},
	{REG_NONE, "", 0, 0}
};

static char* mosFlags = "NV-BDIZC";

void m6502_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	while(m6502RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = m6502RegTab[idx].id;
		bunch->regs[idx].name = m6502RegTab[idx].name;
		bunch->regs[idx].type = m6502RegTab[idx].type;
		switch(m6502RegTab[idx].id) {
			case M6502_REG_PC: bunch->regs[idx].value = cpu->pc; break;
			case M6502_REG_S: bunch->regs[idx].value = cpu->lsp; break;
			case M6502_REG_A: bunch->regs[idx].value = cpu->a; break;
			case M6502_REG_F: bunch->regs[idx].value = cpu->f; break;
			case M6502_REG_X: bunch->regs[idx].value = cpu->lx; break;
			case M6502_REG_Y: bunch->regs[idx].value = cpu->ly; break;
		}
		idx++;
	}
	//memcpy(bunch->flags, "NV-BDIZC", 8);
	bunch->flags = mosFlags;
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
