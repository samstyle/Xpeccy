#include "i8080.h"
#include <string.h>

void i8080_reset(CPU* cpu) {
	cpu->pc = 0x0000;
	cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->sp = 0xffff;
	cpu->a = 0xff;
	cpu->f = 0x02;
	cpu->inten = 0;
}

int i8080_int(CPU* cpu) {
	cpu->iff1 = 0;
	if (cpu->halt) {
		cpu->pc++;
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
	cpu->com = cpu->mrd(cpu->pc++, 1, cpu->data) & 0xff;
	cpu->op = &i8080_tab[cpu->com];
	cpu->t = cpu->op->t;
	cpu->op->exec(cpu);
	cpu->f &= ~(IFL_5 | IFL_3);
	cpu->f |= IFL_1;
	return cpu->t;
}

xAsmScan i8080_asm(const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, i8080_tab);
	res.ptr = buf;
	if (res.match)
		*res.ptr++ = res.idx;
	return res;
}

xMnem i8080_mnem(CPU* cpu, unsigned short adr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.oadr = -1;
	opCode* opt = i8080_tab;
	opCode* opc;
	unsigned char op;
	// unsigned char e = 0;
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

static xRegDsc i8080RegTab[] = {
	{I8080_REG_PC, "PC", REG_WORD},
	{I8080_REG_AF, "AF", REG_WORD},
	{I8080_REG_BC, "BC", REG_WORD},
	{I8080_REG_DE, "DE", REG_WORD},
	{I8080_REG_HL, "HL", REG_WORD},
	{I8080_REG_SP, "SP", REG_WORD},
	{REG_NONE, "", 0}
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
			case I8080_REG_PC: bunch->regs[idx].value = cpu->pc; break;
			case I8080_REG_SP: bunch->regs[idx].value = cpu->sp; break;
			case I8080_REG_AF: rx.h = cpu->a;
					rx.l = cpu->f & 0xff;
					bunch->regs[idx].value = rx.w;
					break;
			case I8080_REG_BC: bunch->regs[idx].value = cpu->bc; break;
			case I8080_REG_DE: bunch->regs[idx].value = cpu->de; break;
			case I8080_REG_HL: bunch->regs[idx].value = cpu->hl; break;
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
			case I8080_REG_PC: cpu->pc = bunch.regs[idx].value; break;
			case I8080_REG_SP: cpu->sp = bunch.regs[idx].value; break;
			case I8080_REG_AF: rx.w = bunch.regs[idx].value;
					cpu->a = rx.h;
					cpu->f = rx.l;
					break;
			case I8080_REG_BC: cpu->bc = bunch.regs[idx].value; break;
			case I8080_REG_DE: cpu->de = bunch.regs[idx].value; break;
			case I8080_REG_HL: cpu->hl = bunch.regs[idx].value; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
