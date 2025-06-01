#include "i8080.h"
#include <string.h>

extern opCode i8080_tab[256];

int i8080_get_flag(CPU* cpu) {
	return cpu->f.c | (cpu->f.f1 << 1) | (cpu->f.p << 2) | (cpu->f.f3 << 3) | (cpu->f.a << 4) | (cpu->f.f5 << 5) | (cpu->f.z << 6) | (cpu->f.s << 7);
}

void i8080_set_flag(CPU* cpu, int v) {
	cpu->f.c = v & 1;
	cpu->f.f1 = !!(v & 2);
	cpu->f.p = !!(v & 4);
	cpu->f.f3 = !!(v & 8);
	cpu->f.a = !!(v & 16);
	cpu->f.f5 = !!(v & 32);
	cpu->f.z = !!(v & 64);
	cpu->f.s = !!(v & 128);
}


void i8080_reset(CPU* cpu) {
	cpu->regPC = 0x0000;
	cpu->regBC = cpu->regDE = cpu->regHL = 0xffff;
	cpu->regSP = 0xffff;
	cpu->regA = 0xff;
	i8080_set_flag(cpu, 0x02);
	cpu->inten = 0;
}

int i8080_int(CPU* cpu) {
	cpu->f.iff1 = 0;
	if (cpu->halt) {
		cpu->regPC++;
		cpu->halt = 0;
	}
	cpu->t = 2 + 5;			// 2 extra + 5 on RST38 fetch
	i8080_tab[0xff].exec(cpu);	// +3 +3 execution. 13 total
	return cpu->t;
}

int i8080_exec(CPU* cpu) {
	cpu->t = 0;
	if (cpu->intrq & cpu->inten)
		cpu->t = i8080_int(cpu);
	if (cpu->t) return cpu->t;
	cpu->com = cpu->mrd(cpu->regPC++, 1, cpu->xptr) & 0xff;
	cpu->op = &i8080_tab[cpu->com];
	cpu->t = cpu->op->t;
	cpu->op->exec(cpu);
	cpu->f.f5 = 0;
	cpu->f.f3 = 0;
	cpu->f.f1 = 1;
	return cpu->t;
}

xAsmScan i8080_asm(int adr, const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, i8080_tab);
	res.ptr = buf;
	if (res.match)
		*res.ptr++ = res.idx;
	return res;
}

xMnem i8080_mnem(CPU* cpu, int qadr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.oadr = -1;
	opCode* opt = i8080_tab;
	opCode* opc;
	unsigned char op;
	unsigned short adr = qadr & 0xffff;
	unsigned short madr = 0;
	mn.len = 0;
	// get opcode
	op = mrd(adr++,data);
	mn.len++;
	opc = &opt[op];
	mn.mnem = opc->mnem;
	mn.flag = opc->flag;
	// mem reading
	mn.mem = 0;
	mn.mop = 0xff;
	xpair mop;
	mop.w = 0;
	if (strstr(opc->mnem, "(:2)")) {
		mn.mem = 1;
		madr = mrd(adr, data) & 0xff;
		madr |= (mrd(adr+1, data) << 8);
	}
	if (mn.mem) {
		mop.l = mrd(madr++, data);
		if (mn.flag & OF_MWORD)
			mop.h = mrd(madr, data);
	}
	mn.mop = mop.w;
	// conditions
	mn.cond = 0;
	mn.met = 0;
	return mn;
}

xRegDsc i8080RegTab[] = {
	{I8080_REG_PC, "PC", REG_WORD | REG_RDMP, offsetof(CPU, regPC)},
	{I8080_REG_AF, "AF", REG_WORD, 0},
	{I8080_REG_BC, "BC", REG_WORD | REG_RDMP, offsetof(CPU, regBC)},
	{I8080_REG_DE, "DE", REG_WORD | REG_RDMP, offsetof(CPU, regDE)},
	{I8080_REG_HL, "HL", REG_WORD | REG_RDMP, offsetof(CPU, regHL)},
	{I8080_REG_SP, "SP", REG_WORD | REG_RDMP, offsetof(CPU, regSP)},
	{REG_EMPTY, "A", REG_BYTE, offsetof(CPU, regA)},
	{REG_EMPTY, "F", REG_32, offsetof(CPU, f)},
	{REG_NONE, "", 0, 0}
};

static char* i8080_flags = "SZ5A3P1C";

void i8080_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	PAIR(w,h,l)rx;
	while(i8080RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = i8080RegTab[idx].id;
		bunch->regs[idx].name = i8080RegTab[idx].name;
		bunch->regs[idx].type = i8080RegTab[idx].type;
		switch(i8080RegTab[idx].id) {
			case I8080_REG_PC: bunch->regs[idx].value = cpu->regPC; break;
			case I8080_REG_SP: bunch->regs[idx].value = cpu->regSP; break;
			case I8080_REG_AF: rx.h = cpu->regA;
					rx.l = i8080_get_flag(cpu) & 0xff;
					bunch->regs[idx].value = rx.w;
					break;
			case I8080_REG_BC: bunch->regs[idx].value = cpu->regBC; break;
			case I8080_REG_DE: bunch->regs[idx].value = cpu->regDE; break;
			case I8080_REG_HL: bunch->regs[idx].value = cpu->regHL; break;
		}
		idx++;
	}
	bunch->regs[idx].id = REG_NONE;
	bunch->flags = i8080_flags;
	//memcpy(bunch->flags, "SZ5A3P1C", 8);
}

void i8080_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx;
	PAIR(w,h,l)rx;
	for (idx = 0; idx < 32; idx++) {
		switch(bunch.regs[idx].id) {
			case I8080_REG_PC: cpu->regPC = bunch.regs[idx].value; break;
			case I8080_REG_SP: cpu->regSP = bunch.regs[idx].value; break;
			case I8080_REG_AF: rx.w = bunch.regs[idx].value;
					cpu->regA = rx.h;
					i8080_set_flag(cpu, rx.l);
					break;
			case I8080_REG_BC: cpu->regBC = bunch.regs[idx].value; break;
			case I8080_REG_DE: cpu->regDE = bunch.regs[idx].value; break;
			case I8080_REG_HL: cpu->regHL = bunch.regs[idx].value; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
