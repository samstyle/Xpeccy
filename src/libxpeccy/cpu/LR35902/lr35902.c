#include <stdlib.h>
#include <stdio.h>

#include "lr35902.h"
#include <string.h>

extern opCode lrTab[256];
extern opCode lrcbTab[256];

void lr_reset(CPU* cpu) {
	cpu->pc = 0;
	cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->a = 0xff;
	cpu->f = 0xff;
	cpu->sp = 0xffff;
	cpu->lock = 0;
	cpu->iff1 = 0;
	cpu->intrq = 0;
	cpu->halt = 0;
	cpu->stop = 0;
	cpu->intrq = 0;
	cpu->inten = 0;
	// not necessary
	cpu->imode = 0;
	cpu->bc_ = cpu->de_ = cpu->hl_ = 0xffff;
	cpu->a_ = 0xff;
	cpu->f_ = 0xff;
	cpu->ix = cpu->iy = 0xffff;
	cpu->i = cpu->r = 0xff;
	cpu->r7 = 0x80;
}

typedef struct {
	unsigned char mask;
	unsigned short inta;
} xLRInt;

xLRInt lr_intab[] = {{1,0x40},{2,0x48},{4,0x50},{8,0x58},{16,0x60},{0,0}};

void z80_push(CPU*, unsigned short);
void z80_call(CPU*, unsigned short);

int lr_int(CPU* cpu) {
	if (cpu->halt) {		// free HALT anyway
		cpu->halt = 0;
		cpu->pc++;
		if (!cpu->iff1) {
			cpu->dihalt = 1;
			cpu->tmpw = cpu->pc;		// tmpw doesn't used on LR35902, store PC there
		}
	}
	if (!cpu->iff1) return 0;
	int idx = 0;
	int res = 0;
	cpu->intrq &= cpu->inten;
	while (lr_intab[idx].mask) {
		if (cpu->intrq & lr_intab[idx].mask) {
			cpu->iff1 = 0;
			cpu->intrq ^= lr_intab[idx].mask;	// reset int request flag
			z80_call(cpu, lr_intab[idx].inta);	// execute call	{RST(lr_intab[idx].inta);}
			res = 15;				// TODO: to know how much T eats INT handle
			break;
		}
		idx++;
	}
	return res;
}

int lr_exec(CPU* cpu) {
	int res = 0;
	if ((cpu->intrq & cpu->inten) && cpu->iff1) {
		res = lr_int(cpu);
	} else if (cpu->lock) {
		res = 1;
	} else {
		cpu->t = 0;
		cpu->opTab = lrTab;
		do {
			cpu->tmp = cpu->mrd(cpu->pc++, 1, cpu->xptr);
			cpu->op = &cpu->opTab[cpu->tmp];
			cpu->t += cpu->op->t;
			cpu->op->exec(cpu);
		} while (cpu->op->flag & OF_PREFIX);
		if (cpu->dihalt) {		// LR35902 bug (?) : repeat opcode after HALT with disabled interrupts (DI)
			cpu->dihalt = 0;
			cpu->pc = cpu->tmpw;
		}
		res = cpu->t;
	}
	return res;
}

// disasm

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
		mn.mop = mrd(cpu->hl, data);
	} else if (strstr(mn.mnem, "(de)")) {
		mn.mem = 1;
		mn.mop = mrd(cpu->de, data);
	} else if (strstr(mn.mnem, "(bc)")) {
		mn.mem = 1;
		mn.mop = mrd(cpu->bc, data);
	} else if (strstr(mn.mnem, "(:1)")) {
		mn.mem = 1;
		madr = mrd(adr, data) & 0xff;
		madr |= (mrd(adr+1, data) << 8);
		mn.mop = mrd(madr, data);
	} else if (strstr(mn.mnem, "ldh")) {
		if ((op & 0xef) == 0xe0) {		// ldh (:1)
			madr = mrd(adr, data) & 0xff;
		} else {				// ldh (c)
			madr = cpu->c & 0xff;
		}
		mn.mem = 1;
		mn.mop = mrd(madr | 0xff00, data);
	}
	// cond (TODO)
	mn.cond = 0;
	mn.met = 0;
	if (strstr(opc->mnem, "djnz")) {
		mn.cond = 1;
		mn.met = (cpu->b == 1) ? 0 : 1;
	} else if (opt == lrTab) {
		if (((op & 0xc7) == 0xc2) || ((op & 0xc7) == 0xc4) || ((op & 0xc7) == 0xc0)) {		// call, jp, ret
			mn.cond = 1;
			mn.met = (op & 0x10) ? !cpu->fl.z : !cpu->fl.c;
			//mn.met = (cpu->f & lr_cnd[(op & 0x30) >> 4]) ? 0 : 1;
			if (op & 8)
				mn.met ^= 1;
		} else if ((op & 0xe7) == 0x20) {							// jr
			mn.cond = 1;
			//mn.met = (cpu->f & lr_cnd[(op & 0x10) >> 4] ? 0 : 1);
			mn.met = (op & 0x10) ? !cpu->fl.z : !cpu->fl.c;
			if (op & 8)
				mn.met ^= 1;
		}
	}

	return mn;
}

// registers

xRegDsc lrRegTab[] = {
	{LR_REG_PC, "PC", REG_WORD, offsetof(CPU, pc)},
	{LR_REG_AF, "AF", REG_WORD, 0},
	{LR_REG_BC, "BC", REG_WORD, offsetof(CPU, bc)},
	{LR_REG_DE, "DE", REG_WORD, offsetof(CPU, de)},
	{LR_REG_HL, "HL", REG_WORD, offsetof(CPU, hl)},
	{LR_REG_SP, "SP", REG_WORD, offsetof(CPU, sp)},
	{REG_EMPTY, "A", REG_BYTE, offsetof(CPU, a)},
	{REG_EMPTY, "F", REG_32, offsetof(CPU, f)},
	{REG_NONE, "", 0, 0}
};

static char* lrFlags = "ZNHC----";

void lr_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	PAIR(w,h,l)rx;
	while(lrRegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = lrRegTab[idx].id;
		bunch->regs[idx].name = lrRegTab[idx].name;
		bunch->regs[idx].type = lrRegTab[idx].type;
		switch(lrRegTab[idx].id) {
			case LR_REG_PC: bunch->regs[idx].value = cpu->pc; break;
			case LR_REG_SP: bunch->regs[idx].value = cpu->sp; break;
			case LR_REG_AF: rx.h = cpu->a;
					rx.l = cpu->f & 0xff;
					bunch->regs[idx].value = rx.w;
					break;
			case LR_REG_BC: bunch->regs[idx].value = cpu->bc; break;
			case LR_REG_DE: bunch->regs[idx].value = cpu->de; break;
			case LR_REG_HL: bunch->regs[idx].value = cpu->hl; break;
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
			case LR_REG_PC: cpu->pc = bunch.regs[idx].value; break;
			case LR_REG_SP: cpu->sp = bunch.regs[idx].value; break;
			case LR_REG_AF: rx.w = bunch.regs[idx].value;
					cpu->a = rx.h;
					cpu->f = rx.l;
					break;
			case LR_REG_BC: cpu->bc = bunch.regs[idx].value; break;
			case LR_REG_DE: cpu->de = bunch.regs[idx].value; break;
			case LR_REG_HL: cpu->hl = bunch.regs[idx].value; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
