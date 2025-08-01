#include <stdlib.h>
#include <stdio.h>

#include "lr35902.h"
#include <string.h>

extern opCode lrTab[256];
extern opCode lrcbTab[256];

void lr_set_flag(CPU* cpu, int v) {
	cpu->flgC = !!(v & 0x10);
	cpu->flgH = !!(v & 0x20);
	cpu->flgN = !!(v & 0x40);
	cpu->flgZ = !!(v & 0x80);
}

int lr_get_flag(CPU* cpu) {
	return (cpu->flgC << 4) | (cpu->flgH << 5) | (cpu->flgN << 6) | (cpu->flgZ << 7);
}

void lr_reset(CPU* cpu) {
	cpu->regPC = 0;
	cpu->regBC = cpu->regDE = cpu->regHL = 0xffff;
	cpu->regA = 0xff;
	lr_set_flag(cpu, 0xff);
	cpu->regSP = 0xffff;
	cpu->flgLOCK = 0;
	cpu->flgIFF1 = 0;
	cpu->intrq = 0;
	cpu->flgHALT = 0;
	cpu->flgSTOP = 0;
	cpu->flgIFFC = 0;
	cpu->intrq = 0;
	cpu->inten = 0;
}

typedef struct {
	unsigned char mask;
	unsigned short inta;
} xLRInt;

xLRInt lr_intab[] = {{1,0x40},{2,0x48},{4,0x50},{8,0x58},{16,0x60},{0,0}};

void lr_call(CPU*, unsigned short);

// TODO: how interrupt works
int lr_int(CPU* cpu) {
//	cpu->intoc = cpu->intrq;
	if (cpu->flgHALT && (cpu->intrq & cpu->inten)) {				// HALT exit if any
		cpu->flgHALT = 0;
		cpu->regPC++;
		if (!cpu->flgIFF1) {
			cpu->flgDIHALT = 1;
			cpu->regTPC = cpu->regPC;
		}
	}
	int res = 0;
	int idx = 0;
	if ((cpu->intrq & cpu->inten) && cpu->flgIFF1) {			// have enabled interrupt (IME and IE)
		while (lr_intab[idx].mask) {
			if (!res && ((cpu->intrq & cpu->inten) & lr_intab[idx].mask)) {
				cpu->flgIFF1 = 0;
				cpu->intrq &= ~lr_intab[idx].mask;	// reset highest priority INT request flag. if there is others, they suspended (not switched off)
				lr_call(cpu, lr_intab[idx].inta);	// execute call	{RST(lr_intab[idx].inta);}
				res = 20;				// TODO: to know how much T eats INT handle (5M = 20T)
			}
			idx++;
		}
	}
	return res;
}

int lr_exec(CPU* cpu) {
	if (cpu->flgLOCK) {
		cpu->t = 4;
	} else {
		cpu->t = 0;
		if (cpu->intrq) {
			cpu->t = lr_int(cpu);
		}
		if (!cpu->t) {
			if (cpu->flgIFFC) {			// if last instruction was ei/di, change current IFF
				cpu->flgIFFC = 0;
				cpu->flgIFF1 = cpu->flgIFFN;
			}
//			cpu->t = 0;
			cpu->opTab = lrTab;
			do {
				cpu->tmp = cpu->mrd(cpu->regPC++, 1, cpu->xptr);
				cpu->op = &cpu->opTab[cpu->tmp];
				cpu->t += cpu->op->t;
				cpu->op->exec(cpu);
			} while (cpu->op->flag & OF_PREFIX);
			if (cpu->flgDIHALT) {		// LR35902 bug (?) : repeat opcode after HALT with disabled interrupts (DI)
				cpu->flgDIHALT = 0;
				cpu->regPC = cpu->regTPC;
			}
		}
	}
	return cpu->t;
}

// disasm

xAsmScan lr_asm(int adr, const char* cbuf, char* buf) {
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

//static unsigned char lr_cnd[4] = {Z80_FZ, Z80_FC, Z80_FP, Z80_FS};

xMnem lr_mnem(CPU* cpu, int qadr, cbdmr mrd, void* data) {
	int res = 0;
	opCode* opt = lrTab;
	opCode* opc;
	unsigned char op;
	unsigned short adr = qadr & 0xffff;
	unsigned short madr;
	do {
		op = mrd(adr++,data);
		res++;
		opc = &opt[op];
		if (opc->flag & OF_PREFIX) opt = opc->tab;
	} while (opc->flag & OF_PREFIX);
	xMnem mn;
	mn.oadr = -1;
	mn.mop = 0;
	mn.len = res;
	mn.mnem = opc->mnem;
	mn.flag = opc->flag;
	// mem
	if (strstr(mn.mnem, "(hl)") && strcmp(mn.mnem, "jp (hl)")) {
		mn.mem = 1;
		mn.mop = mrd(cpu->regHL, data);
	} else if (strstr(mn.mnem, "(de)")) {
		mn.mem = 1;
		mn.mop = mrd(cpu->regDE, data);
	} else if (strstr(mn.mnem, "(bc)")) {
		mn.mem = 1;
		mn.mop = mrd(cpu->regBC, data);
	} else if (strstr(mn.mnem, "(:1)")) {
		mn.mem = 1;
		madr = mrd(adr, data) & 0xff;
		madr |= (mrd(adr+1, data) << 8);
		mn.mop = mrd(madr, data);
	} else if (strstr(mn.mnem, "ldh")) {
		if ((op & 0xef) == 0xe0) {		// ldh (:1)
			madr = mrd(adr, data) & 0xff;
		} else {				// ldh (c)
			madr = cpu->regC & 0xff;
		}
		mn.mem = 1;
		mn.mop = mrd(madr | 0xff00, data);
	}
	// cond (TODO)
	mn.cond = 0;
	mn.met = 0;
	if (strstr(opc->mnem, "djnz")) {
		mn.cond = 1;
		mn.met = (cpu->regB == 1) ? 0 : 1;
	} else if (opt == lrTab) {
		if (((op & 0xc7) == 0xc2) || ((op & 0xc7) == 0xc4) || ((op & 0xc7) == 0xc0)) {		// call, jp, ret
			mn.cond = 1;
			mn.met = (op & 0x10) ? !cpu->flgZ : !cpu->flgC;
			//mn.met = (cpu->f & lr_cnd[(op & 0x30) >> 4]) ? 0 : 1;
			if (op & 8)
				mn.met ^= 1;
		} else if ((op & 0xe7) == 0x20) {							// jr
			mn.cond = 1;
			//mn.met = (cpu->f & lr_cnd[(op & 0x10) >> 4] ? 0 : 1);
			mn.met = (op & 0x10) ? !cpu->flgZ : !cpu->flgC;
			if (op & 8)
				mn.met ^= 1;
		}
	}

	return mn;
}

// registers

xRegDsc lrRegTab[] = {
	{LR_REG_PC, "PC", REG_WORD, REG_RDMP | REG_PC, offsetof(CPU, regPC)},
	{LR_REG_AF, "AF", REG_WORD, 0, 0},
	{LR_REG_BC, "BC", REG_WORD, REG_RDMP, offsetof(CPU, regBC)},
	{LR_REG_DE, "DE", REG_WORD, REG_RDMP, offsetof(CPU, regDE)},
	{LR_REG_HL, "HL", REG_WORD, REG_RDMP, offsetof(CPU, regHL)},
	{LR_REG_SP, "SP", REG_WORD, REG_RDMP | REG_SP, offsetof(CPU, regSP)},
	{LR_FLG_IFF, "IME", REG_BIT, 0, offsetof(CPU, flgIFF1)},
	{LR_REG_IF, "IF", REG_WORD, REG_RO, offsetof(CPU, intrq)},
	{LR_REG_IE, "IE", REG_WORD, REG_RO, offsetof(CPU, inten)},
	{REG_EMPTY, "A", REG_BYTE, 0, offsetof(CPU, regA)},
	{REG_EMPTY, "F", REG_32, 0, 0},
	{REG_NONE, "", 0, 0, 0}
};

static char* lrFlags = "ZNHC----";

void lr_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	PAIR(w,h,l)rx;
	while(lrRegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = lrRegTab[idx].id;
		bunch->regs[idx].name = lrRegTab[idx].name;
		bunch->regs[idx].type = lrRegTab[idx].type;
		bunch->regs[idx].flag = lrRegTab[idx].flag;
		switch(lrRegTab[idx].id) {
			case LR_REG_PC: bunch->regs[idx].value = cpu->regPC; break;
			case LR_REG_SP: bunch->regs[idx].value = cpu->regSP; break;
			case LR_REG_AF: rx.h = cpu->regA;
					rx.l = lr_get_flag(cpu) & 0xff;
					bunch->regs[idx].value = rx.w;
					break;
			case LR_REG_BC: bunch->regs[idx].value = cpu->regBC; break;
			case LR_REG_DE: bunch->regs[idx].value = cpu->regDE; break;
			case LR_REG_HL: bunch->regs[idx].value = cpu->regHL; break;
			case LR_REG_IF: bunch->regs[idx].value = cpu->intrq; break;
			case LR_REG_IE: bunch->regs[idx].value = cpu->inten; break;
			case LR_FLG_IFF: bunch->regs[idx].value = cpu->flgIFF1; break;
		}
		idx++;
	}
	bunch->regs[idx].id = REG_NONE;
	bunch->flags = lrFlags;
	//memcpy(bunch->flags, "ZNHC----", 8);
}

void lr_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx;
	PAIR(w,h,l)rx;
	for (idx = 0; idx < 32; idx++) {
		switch(bunch.regs[idx].id) {
			case LR_REG_PC: cpu->regPC = bunch.regs[idx].value; break;
			case LR_REG_SP: cpu->regSP = bunch.regs[idx].value; break;
			case LR_REG_AF: rx.w = bunch.regs[idx].value;
					cpu->regA = rx.h;
					lr_set_flag(cpu, rx.l);
					break;
			case LR_REG_BC: cpu->regBC = bunch.regs[idx].value; break;
			case LR_REG_DE: cpu->regDE = bunch.regs[idx].value; break;
			case LR_REG_HL: cpu->regHL = bunch.regs[idx].value; break;
			case LR_FLG_IFF: cpu->flgIFF1 = bunch.regs[idx].value; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
