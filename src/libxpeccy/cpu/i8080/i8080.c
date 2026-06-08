#include "i8080.h"
#include <string.h>

extern opCode i8080_tab[256];

int i8080_get_flag(CPU* cpu) {
	return cpu->flgC | (cpu->flgF1 << 1) | (cpu->flgP << 2) | (cpu->flgF3 << 3) | (cpu->flgA << 4) | (cpu->flgF5 << 5) | (cpu->flgZ << 6) | (cpu->flgS << 7);
}

void i8080_set_flag(CPU* cpu, int v) {
	cpu->flgC = v & 1;
	cpu->flgF1 = !!(v & 2);
	cpu->flgP = !!(v & 4);
	cpu->flgF3 = !!(v & 8);
	cpu->flgA = !!(v & 16);
	cpu->flgF5 = !!(v & 32);
	cpu->flgZ = !!(v & 64);
	cpu->flgS = !!(v & 128);
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
	cpu->flgIFF1 = 0;
	if (cpu->flgHALT) {
		cpu->regPC++;
		cpu->flgHALT = 0;
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
	cpu->flgF5 = 0;
	cpu->flgF3 = 0;
	cpu->flgF1 = 1;
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

void i8080_set_pc(CPU* cpu, int v) {cpu->regPC = v;}
void i8080_set_af(CPU* cpu, int v) {xreg16 r; r.w = v & 0xffff; cpu->regA = r.h; i8080_set_flag(cpu, r.l);}
void i8080_set_bc(CPU* cpu, int v) {cpu->regBC = v;}
void i8080_set_de(CPU* cpu, int v) {cpu->regDE = v;}
void i8080_set_hl(CPU* cpu, int v) {cpu->regHL = v;}
void i8080_set_sp(CPU* cpu, int v) {cpu->regSP = v;}
void i8080_set_iff(CPU* cpu, int v) {cpu->flgIFF1 = !!v;}
void i8080_set_a(CPU* cpu, int v) {cpu->regA = v;}

int i8080_get_pc(CPU* cpu) {return cpu->regPC;}
int i8080_get_af(CPU* cpu) {xreg16 r; r.h = cpu->regA; r.l = i8080_get_flag(cpu); return r.w;}
int i8080_get_bc(CPU* cpu) {return cpu->regBC;}
int i8080_get_de(CPU* cpu) {return cpu->regDE;}
int i8080_get_hl(CPU* cpu) {return cpu->regHL;}
int i8080_get_sp(CPU* cpu) {return cpu->regSP;}
int i8080_get_iff(CPU* cpu) {return cpu->flgIFF1;}
int i8080_get_a(CPU* cpu) {return cpu->regA;}

//static char* i8080_flags = "SZ5A3P1C";

xRegDsc i8080RegTab[] = {
	{I8080_REG_PC, "PC", REG_WORD, REG_RDMP | REG_PC, i8080_get_pc, i8080_set_pc},
	{I8080_REG_AF, "AF", REG_WORD, 0, i8080_get_af, i8080_set_af},
	{I8080_REG_BC, "BC", REG_WORD, REG_RDMP, i8080_get_bc, i8080_set_bc},
	{I8080_REG_DE, "DE", REG_WORD, REG_RDMP, i8080_get_de, i8080_set_de},
	{I8080_REG_HL, "HL", REG_WORD, REG_RDMP, i8080_get_hl, i8080_set_hl},
	{I8080_REG_SP, "SP", REG_WORD, REG_RDMP | REG_SP, i8080_get_sp, i8080_set_sp},
	{I8080_FLG_IFF, "IFF", REG_BIT, 0, i8080_get_iff, i8080_set_iff},
	{REG_EMPTY, "A", REG_BYTE, 0, i8080_get_a, i8080_set_a},
	{REG_EMPTY, "F", REG_32, REG_FLG, i8080_get_flag, i8080_set_flag},
	{REG_EOT, "SZ5A3P1C", 0, 0, NULL, NULL}
};

/*
void i8080_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	PAIR(w,h,l)rx;
	while(i8080RegTab[idx].id != REG_EOT) {
		bunch->regs[idx].id = i8080RegTab[idx].id;
		bunch->regs[idx].name = i8080RegTab[idx].name;
		bunch->regs[idx].type = i8080RegTab[idx].size;
		bunch->regs[idx].flag = i8080RegTab[idx].flag;
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
	bunch->regs[idx].id = REG_EOT;
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
			case REG_EOT: idx = 100; break;
		}
	}
}
*/
