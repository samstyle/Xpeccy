#include "6502.h"

#include <string.h>
#include <stdio.h>

extern opCode mosTab[256];

void mos_set_flag(CPU* cpu, int v) {
	cpu->f.c = v & 1;
	cpu->f.z = !!(v & 2);
	cpu->f.i = !!(v & 4);
	cpu->f.d = !!(v & 8);
	cpu->f.b = !!(v & 16);
	cpu->f.f5 = !!(v & 32);
	cpu->f.v = !!(v & 64);
	cpu->f.n = !!(v & 128);
}

int mos_get_flag(CPU* cpu) {
	return cpu->f.c | (cpu->f.z << 1) | (cpu->f.i << 2) | (cpu->f.d << 3) | (cpu->f.b << 4) | (1 << 5) | (cpu->f.v << 6) | (cpu->f.n << 7);
}

void m6502_reset(CPU* cpu) {
	cpu->lock = 0;
	cpu->intrq = 0;
	cpu->inten = MOS6502_INT_IRQ | MOS6502_INT_NMI;	// brk/nmi enabled. irq is allways enabled, controlled by I flag
	cpu->regS = 0xfd;				// segment 01xx is stack
	// cpu->f = MF5 | MFI;
	cpu->f.f5 = 1;
	cpu->f.i = 1;
	cpu->regA = 0;
	cpu->regX = 0;
	cpu->regY = 0;
	cpu->regPCl = cpu->mrd(0xfffc, 0, cpu->xptr);
	cpu->regPCh = cpu->mrd(0xfffd, 0, cpu->xptr);
//	printf("mos pc = %.4X\n", cpu->pc);
}

void m6502_push(CPU* cpu, int v) {
	cpu->mwr(cpu->regS | 0x100, v & 0xff, cpu->xptr);
	cpu->regS--;
}

int m6502_pop(CPU* cpu) {
	cpu->regS++;
	return cpu->mrd(cpu->regS | 0x100, 0, cpu->xptr) & 0xff;
}

void m6502_push_int(CPU* cpu) {
	m6502_push(cpu, cpu->regPCh);
	m6502_push(cpu, cpu->regPCl);
	m6502_push(cpu, mos_get_flag(cpu));
}

int m6502_int(CPU* cpu) {
	if (cpu->intrq & MOS6502_INT_NMI) {		// NMI (VBlank)
		cpu->intrq &= ~MOS6502_INT_NMI;
		m6502_push_int(cpu);
		cpu->regPCl = cpu->mrd(0xfffa, 0, cpu->xptr);
		cpu->regPCh = cpu->mrd(0xfffb, 0, cpu->xptr);
	} else if (cpu->intrq & MOS6502_INT_IRQ) {	// IRQ
		cpu->intrq &= ~MOS6502_INT_IRQ;
		//if (!(cpu->f & MFI)) {			// IRQ enabled, I flag = 0
		if (!cpu->f.i) {
			//cpu->f &= ~MFB;			// reset B flag
			cpu->f.b = 0;
			m6502_push_int(cpu);
			//cpu->f |= MFI;			// disable IRQ
			cpu->f.i = 1;
			cpu->regPCl = cpu->mrd(0xfffe, 0, cpu->xptr);
			cpu->regPCh = cpu->mrd(0xffff, 0, cpu->xptr);
		}
	}
	return 7;				// real: 7T
}

int m6502_exec(CPU* cpu) {
	int res = 0;
	if (cpu->lock) return 1;
	unsigned char com;
	cpu->intrq &= cpu->inten;
//	if (cpu->f & MFI)
	if (cpu->f.i)
		cpu->intrq &= ~MOS6502_INT_IRQ;
	if (cpu->intrq && !cpu->noint) {
		res = m6502_int(cpu);
	} else {
		cpu->noint = 0;
		com = cpu->mrd(cpu->regPC++, 1, cpu->xptr);
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
#if 1
		switch((op >> 6) & 3) {
			case 0: mn.met = !cpu->f.n; break;
			case 1: mn.met = !cpu->f.v; break;
			case 2: mn.met = !cpu->f.c; break;
			case 3: mn.met = !cpu->f.z; break;
		}
#else
		mn.met = (cpu->f & m6502_cond[(op >> 6) & 3]) ? 0 : 1;		// true if 0
#endif
		if (op & 0x20)							// true if 1
			mn.met ^= 1;
	} else {
		mn.cond = 0;
	}
	// mem
	mn.mem = 0;				// todo
	return mn;
}

xAsmScan m6502_asm(int a, const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, mosTab);
	res.ptr = buf;
	if (res.match) {
		*res.ptr++ = res.idx;
	}
	return res;
}

xRegDsc m6502RegTab[] = {
	{M6502_REG_PC, "PC", REG_WORD | REG_RDMP, offsetof(CPU, regPC)},
	{M6502_REG_A, "A", REG_BYTE, offsetof(CPU, regA)},
	{M6502_REG_X, "X", REG_BYTE, offsetof(CPU, regX)},
	{M6502_REG_Y, "Y", REG_BYTE, offsetof(CPU, regY)},
	{M6502_REG_S, "S", REG_BYTE, offsetof(CPU, regS)},
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
			case M6502_REG_PC: bunch->regs[idx].value = cpu->regPC; break;
			case M6502_REG_S: bunch->regs[idx].value = cpu->regS; break;
			case M6502_REG_A: bunch->regs[idx].value = cpu->regA; break;
			case M6502_REG_F: bunch->regs[idx].value = mos_get_flag(cpu); break;
			case M6502_REG_X: bunch->regs[idx].value = cpu->regX; break;
			case M6502_REG_Y: bunch->regs[idx].value = cpu->regY; break;
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
			case M6502_REG_PC: cpu->regPC = bunch.regs[idx].value; break;
			case M6502_REG_S: cpu->regS = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_A: cpu->regA = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_F: mos_set_flag(cpu, bunch.regs[idx].value & 0xff); break;
			case M6502_REG_X: cpu->regX = bunch.regs[idx].value & 0xff; break;
			case M6502_REG_Y: cpu->regY = bunch.regs[idx].value & 0xff; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
