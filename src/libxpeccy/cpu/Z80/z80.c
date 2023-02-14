#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../cpu.h"


extern opCode npTab[256];
extern opCode ddTab[256];
extern opCode fdTab[256];
extern opCode cbTab[256];
extern opCode edTab[256];
extern opCode ddcbTab[256];
extern opCode fdcbTab[256];

void z80_reset(CPU* cpu) {
	cpu->pc = 0;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->imode = 0;
	cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->a = 0xff;
	cpu->f = 0xff;
	cpu->bc_ = cpu->de_ = cpu->hl_ = 0xffff;
	cpu->a_ = 0xff;
	cpu->f_ = 0xff;
	cpu->ix = cpu->iy = 0xffff;
	cpu->sp = 0xffff;
	cpu->i = cpu->r = cpu->r7 = 0;
	cpu->halt = 0;
	cpu->intrq = 0;
	cpu->inten = Z80_NMI;	// NMI allways enabled, INT is controlled by ei/di
	cpu->wait = 0;
}

// if block opcode is interrupted, flags will be like this:

void z80_blkio_interrupt(CPU* cpu) {
	cpu->f &= ~(Z80_F3 | Z80_F5);
	cpu->f |= cpu->hpc & (Z80_F3 | Z80_F5);
	if (cpu->f & Z80_FC) {
		cpu->f &= ~Z80_FH;
		if (cpu->tmp & 0x80) {
			if (parity((cpu->b - 1) & 7) ^ 1)
				cpu->f ^= Z80_FP;
			if ((cpu->b & 15) == 0)
				cpu->f |= Z80_FH;
		} else {
			if (parity((cpu->b + 1) & 7) ^ 1)
				cpu->f ^= Z80_FP;
			if ((cpu->b & 15) == 15)
				cpu->f |= Z80_FH;

		}
	} else if (parity(cpu->b & 7) ^ 1) {
		cpu->f ^= Z80_FP;
	}
}

int z80_int(CPU* cpu) {
	int res = 0;
	if (cpu->wait) return res;
	if (cpu->intrq & Z80_INT) {		// int
		if (cpu->iff1 && !cpu->noint && cpu->ack) {
			cpu->iff1 = 0;
			cpu->iff2 = 0;
			if (cpu->halt) {
				cpu->pc++;
				cpu->halt = 0;
			} else if (cpu->resPV) {
				cpu->f &= ~Z80_FP;
			} else if (cpu->blk) {
				cpu->f &= ~(Z80_F3 | Z80_F5);
				cpu->f |= cpu->hpc & (Z80_F3 | Z80_F5);
			} else if (cpu->blkio) {
				z80_blkio_interrupt(cpu);
			}
			cpu->opTab = npTab;
			switch(cpu->imode) {
				case 0:
					cpu->t = 2;
					cpu->op = &cpu->opTab[cpu->irq(cpu->data)];
					cpu->r++;
					cpu->t += cpu->op->t;		// +5 (RST38 fetch)
					cpu->op->exec(cpu);		// +3 +3 execution. 13 total
					while (cpu->op->flag & OF_PREFIX) {
						cpu->op = &cpu->opTab[cpu->mrd(cpu->pc++,1,cpu->data)];
						cpu->r++;
						cpu->t += cpu->op->t;
						cpu->op->exec(cpu);
					}
					break;
				case 1:
					cpu->r++;
					cpu->t = 2 + 5;	// 2 extra + 5 on RST38 fetch
					RST(0x38);	// +3 +3 execution. 13 total
					break;
				case 2:
					cpu->r++;
					cpu->t = 7;
					PUSH(cpu->hpc,cpu->lpc);		// +3 (10) +3 (13)
					cpu->lptr = cpu->irq(cpu->data);	// int vector (FF)
					cpu->hptr = cpu->i;
					cpu->lpc = MEMRD(cpu->mptr++,3);	// +3 (16)
					cpu->hpc = MEMRD(cpu->mptr,3);		// +3 (19)
					cpu->mptr = cpu->pc;
					break;
			}
			res = cpu->t;
			cpu->intrq &= ~Z80_INT;
		}
	} else if (cpu->intrq & Z80_NMI) {			// nmi
		if (!cpu->noint) {
			cpu->r++;
			cpu->iff2 = cpu->iff1;
			cpu->iff1 = 0;
			cpu->t = 5;
			PUSH(cpu->hpc,cpu->lpc);
			cpu->pc = 0x0066;
			cpu->mptr = cpu->pc;
			res = cpu->t;		// always 11
		}
		cpu->intrq &= ~Z80_NMI;
	}
	return res;
}

int z80_exec(CPU* cpu) {
	int res = 0;
	if (cpu->wait) {
		res = 1;
	} else if (cpu->intrq & cpu->inten) {
		res = z80_int(cpu);
	}
	cpu->resPV = 0;
	cpu->noint = 0;
	if (!res) {
		cpu->t = 0;
		cpu->opTab = npTab;
		do {
			cpu->com = cpu->mrd(cpu->pc++,1,cpu->data);
			cpu->op = &cpu->opTab[cpu->com];
			cpu->r++;
			cpu->t += cpu->op->t;
			cpu->op->exec(cpu);
		} while (cpu->op->flag & OF_PREFIX);
		res = cpu->t;
	}
	return res;
}

// disasm

static unsigned char z80_cnd[4] = {Z80_FZ, Z80_FC, Z80_FP, Z80_FS};

xMnem z80_mnem(CPU* cpu, int qadr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.oadr = -1;
	opCode* opt = npTab;
	opCode* opc;
	unsigned char op;
	unsigned char e = 0;
	unsigned short adr = qadr & 0xffff;
	unsigned short madr = 0;
	mn.len = 0;
	do {
		op = mrd(adr++,data);
		mn.len++;
		opc = &opt[op];
		if (opc->flag & OF_PREFIX) {
			opt = opc->tab;
			if ((opt == ddcbTab) || (opt == fdcbTab)) {
				e = mrd(adr, data);
				adr++;
				mn.len++;
			}
		}
	} while (opc->flag & OF_PREFIX);
	mn.mnem = opc->mnem;
	mn.flag = opc->flag;
	// mem reading
	mn.mem = 0;
	mn.mop = 0xff;
	xpair mop;
	mop.w = 0;
	if (strstr(opc->mnem, "(hl)") && strcmp(opc->mnem, "jp (hl)")) {
		mn.mem = 1;
		madr = cpu->hl;
	} else if (strstr(opc->mnem, "(de)")) {
		mn.mem = 1;
		madr = cpu->de;
	} else if (strstr(opc->mnem, "(bc)")) {
		mn.mem = 1;
		madr = cpu->bc;
	} else if (strstr(opc->mnem, "(ix")) {
		mn.mem = 1;
		if (opt != ddcbTab) e = mrd(adr, data);
		madr = cpu->ix + (signed char)e;
	} else if (strstr(opc->mnem, "(iy")) {
		mn.mem = 1;
		if (opt != fdcbTab) e = mrd(adr, data);
		madr = cpu->iy + (signed char)e;
	} else if (strstr(opc->mnem, "(:2)")) {
		mn.mem = 1;
		madr = mrd(adr, data) & 0xff;
		madr |= (mrd(adr+1, data) << 8);
	}
	if (mn.mem) {
		mop.l = mrd(madr++, data);
		if (mn.flag & OF_MWORD)
			mop.h = mrd(madr, data);
	}
	if (strstr(opc->mnem, "bit") || strstr(opc->mnem, "res") || strstr(opc->mnem, "set")) {
		mop.w = (mop.l & (1 << ((op >> 3) & 7))) ? 1 : 0;
	}
	mn.mop = mop.w;
	// conditions
	mn.cond = 0;
	mn.met = 0;
	if (strstr(opc->mnem, "djnz")) {
		mn.cond = 1;
		mn.met = (cpu->b == 1) ? 0 : 1;
	} else if (opt == npTab) {
		if (((op & 0xc7) == 0xc2) || ((op & 0xc7) == 0xc4) || ((op & 0xc7) == 0xc0)) {		// call, jp, ret
			mn.cond = 1;
			mn.met = (cpu->f & z80_cnd[(op >> 4) & 3]) ? 0 : 1;
			if (op & 8)
				mn.met ^= 1;
		} else if (op == 0x18) {								// jr
			mn.cond = 1;
			mn.met = 1;
		} else if ((op & 0xe7) == 0x20) {							// jr cc
			mn.cond = 1;
			mn.met = (cpu->f & z80_cnd[(op >> 4) & 1] ? 0 : 1);
			if (op & 8)
				mn.met ^= 1;
		}
	}
	return mn;
}

// asm

xAsmScan z80_asm(const char* cbuf, char* buf) {
	xAsmScan res = scanAsmTab(cbuf, npTab);
	res.ptr = buf;
	if (!res.match) {
		res = scanAsmTab(cbuf, ddTab);
		res.ptr = buf;
		*res.ptr++ = 0xdd;
	}
	if (!res.match) {
		res = scanAsmTab(cbuf, fdTab);
		res.ptr = buf;
		*res.ptr++ = 0xfd;
	}
	if (!res.match) {
		res = scanAsmTab(cbuf, cbTab);
		res.ptr = buf;
		*res.ptr++ = 0xcb;
	}
	if (!res.match) {
		res = scanAsmTab(cbuf, edTab);
		res.ptr = buf;
		*res.ptr++ = 0xed;
	}
	if (!res.match) {
		res = scanAsmTab(cbuf, ddcbTab);
		res.ptr = buf;
		*res.ptr++ = 0xdd;
		*res.ptr++ = 0xcb;
	}
	if (!res.match) {
		res = scanAsmTab(cbuf, fdcbTab);
		res.ptr = buf;
		*res.ptr++ = 0xfd;
		*res.ptr++ = 0xcb;
	}
	if (res.match) {
		*res.ptr++ = res.idx;
	}
	return res;
}

// registers

static xRegDsc z80RegTab[] = {
	{Z80_REG_PC, "PC", REG_WORD},
	{Z80_REG_AF, "AF", REG_WORD},
	{Z80_REG_BC, "BC", REG_WORD},
	{Z80_REG_DE, "DE", REG_WORD},
	{Z80_REG_HL, "HL", REG_WORD},

	{Z80_REG_SP, "SP", REG_WORD},
	{Z80_REG_AFA, "AF'", REG_WORD},
	{Z80_REG_BCA, "BC'", REG_WORD},
	{Z80_REG_DEA, "DE'", REG_WORD},
	{Z80_REG_HLA, "HL'", REG_WORD},

	{Z80_REG_IX, "IX", REG_WORD},
	{Z80_REG_IY, "IY", REG_WORD},
	{Z80_REG_I, "I", REG_BYTE},
	{Z80_REG_R, "R", REG_BYTE},
#ifdef ISDEBUG
	{REG_MPTR, "WZ", REG_WORD},
#endif
	{REG_NONE, "", 0}
};

static char* z80Flags = "SZ5H3PNC";

void z80_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	PAIR(w,h,l)rx;
	while(z80RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = z80RegTab[idx].id;
		bunch->regs[idx].name = z80RegTab[idx].name;
		bunch->regs[idx].type = z80RegTab[idx].type;
		switch(z80RegTab[idx].id) {
			case Z80_REG_PC: bunch->regs[idx].value = cpu->pc; break;
			case Z80_REG_SP: bunch->regs[idx].value = cpu->sp; break;
			case Z80_REG_AF: rx.h = cpu->a;
					rx.l = cpu->f & 0xff;
					bunch->regs[idx].value = rx.w;
					break;
			case Z80_REG_BC: bunch->regs[idx].value = cpu->bc; break;
			case Z80_REG_DE: bunch->regs[idx].value = cpu->de; break;
			case Z80_REG_HL: bunch->regs[idx].value = cpu->hl; break;
			case Z80_REG_AFA: rx.h = cpu->a_;
					rx.l = cpu->f_;
					bunch->regs[idx].value = rx.w;
					break;
			case Z80_REG_BCA: bunch->regs[idx].value = cpu->bc_; break;
			case Z80_REG_DEA: bunch->regs[idx].value = cpu->de_; break;
			case Z80_REG_HLA: bunch->regs[idx].value = cpu->hl_; break;
			case Z80_REG_IX: bunch->regs[idx].value = cpu->ix; break;
			case Z80_REG_IY: bunch->regs[idx].value = cpu->iy; break;
			case Z80_REG_I: bunch->regs[idx].value = cpu->i; break;
			case Z80_REG_R: bunch->regs[idx].value = (cpu->r & 0x7f) | (cpu->r7 & 0x80); break;
			case REG_MPTR: bunch->regs[idx].value = cpu->mptr; break;
		}
		idx++;
	}
	bunch->regs[idx].id = REG_NONE;
	bunch->flags = z80Flags;
	// memcpy(bunch->flags, "SZ5H3PNC", 8);
}

void z80_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx;
	PAIR(w,h,l)rx;
	for (idx = 0; idx < 32; idx++) {
		switch(bunch.regs[idx].id) {
			case Z80_REG_PC: cpu->pc = bunch.regs[idx].value; break;
			case Z80_REG_SP: cpu->sp = bunch.regs[idx].value; break;
			case Z80_REG_AF: rx.w = bunch.regs[idx].value;
					cpu->a = rx.h;
					cpu->f = rx.l;
					break;
			case Z80_REG_BC: cpu->bc = bunch.regs[idx].value; break;
			case Z80_REG_DE: cpu->de = bunch.regs[idx].value; break;
			case Z80_REG_HL: cpu->hl = bunch.regs[idx].value; break;
			case Z80_REG_AFA: rx.w = bunch.regs[idx].value;
					cpu->a_ = rx.h;
					cpu->f_ = rx.l;
					break;
			case Z80_REG_BCA: cpu->bc_ = bunch.regs[idx].value; break;
			case Z80_REG_DEA: cpu->de_ = bunch.regs[idx].value; break;
			case Z80_REG_HLA: cpu->hl_ = bunch.regs[idx].value; break;
			case Z80_REG_IX: cpu->ix = bunch.regs[idx].value; break;
			case Z80_REG_IY: cpu->iy = bunch.regs[idx].value; break;
			case Z80_REG_I: cpu->i = bunch.regs[idx].value & 0xff; break;
			case Z80_REG_R:
				cpu->r = bunch.regs[idx].value;
				cpu->r7 = bunch.regs[idx].value & 0x80;
				break;
			case REG_MPTR: cpu->mptr = bunch.regs[idx].value; break;
			case REG_NONE: idx = 100; break;
		}
	}
}
