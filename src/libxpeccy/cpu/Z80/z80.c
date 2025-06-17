#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "z80.h"

extern opCode npTab[256];
extern opCode ddTab[256];
extern opCode fdTab[256];
extern opCode cbTab[256];
extern opCode edTab[256];
extern opCode ddcbTab[256];
extern opCode fdcbTab[256];

void z80_set_flag(CPU* cpu, int v) {
	cpu->flgC = (v & 1);
	cpu->flgN = !!(v & 2);
	cpu->flgPV = !!(v & 4);
	cpu->flgF3 = !!(v & 8);
	cpu->flgH = !!(v & 16);
	cpu->flgF5 = !!(v & 32);
	cpu->flgZ = !!(v & 64);
	cpu->flgS = !!(v & 128);
}

int z80_get_flag(CPU* cpu) {
	return cpu->flgC | (cpu->flgN << 1) | (cpu->flgPV << 2) | (cpu->flgF3 << 3) | (cpu->flgH << 4) | (cpu->flgF5 << 5) | (cpu->flgZ << 6) | (cpu->flgS << 7);
}

void z80_call(CPU*, unsigned short);
//void lr_push(CPU*, unsigned short);

void z80_reset(CPU* cpu) {
	cpu->regPC = 0;
	cpu->flgIFF1 = 0;
	cpu->flgIFF2 = 0;
	cpu->regIM = 0;
	cpu->regBC = cpu->regDE = cpu->regHL = 0xffff;
	cpu->regA = 0xff;
	z80_set_flag(cpu, 0xff);
	cpu->regBCa = cpu->regDEa = cpu->regHLa = 0xffff;
	cpu->regAa = 0xff;
	cpu->regFa = 0xff;
	cpu->regIX = cpu->regIY = 0xffff;
	cpu->regSP = 0xffff;
	cpu->regI = cpu->regR = cpu->regR7 = 0;
	cpu->flgHALT = 0;
	cpu->intrq = 0;
	cpu->inten = Z80_NMI;	// NMI allways enabled, INT is controlled by ei/di
	cpu->flgWAIT = 0;
}

// https://sinclair.wiki.zxnet.co.uk/wiki/Contended_memory
// > For memory access, this happens on the first tstate (T1) of any instruction fetch, memory read or memory write operation

int z80_mrdx(CPU* cpu, int adr, int m1) {
	cpu->adr = adr;
	cpu->t++;		// T1
	do {
		cpu->t++;	// T2 while wait
		cpu->xirq(IRQ_CPU_SYNC, cpu->xptr);
	} while (cpu->flgWAIT);
	int r = cpu->mrd(adr, m1, cpu->xptr) & 0xff;
	cpu->t++;		// T3
	return r;
}

int z80_fetch(CPU* cpu) {
	return z80_mrdx(cpu, cpu->regPC++, 1) & 0xff;
	// TODO: @T4 IR on adr bus + MREQ = ULA snow effect, if video read data at same time
}

int z80_mrd(CPU* cpu, int adr) {
	return z80_mrdx(cpu, adr, 0) & 0xff;
}

void z80_mwr(CPU *cpu, int adr, int data) {
	cpu->adr = adr;
	cpu->t++;		// T1
	do {
		cpu->t++;	// T2 while wait
		cpu->xirq(IRQ_CPU_SYNC, cpu->xptr);
	} while (cpu->flgWAIT);
	cpu->mwr(adr, data, cpu->xptr);
	cpu->t++;		// T3
}

// if block opcode is interrupted, flags will be like this:

int z80_int(CPU* cpu) {
	int res = 0;
	if (cpu->flgWAIT) return res;
	if (cpu->intrq & Z80_INT) {		// int
		if (cpu->flgIFF1 && !cpu->flgNOINT && cpu->flgACK) {
			cpu->flgIFF1 = 0;
			cpu->flgIFF2 = 0;
			if (cpu->flgHALT) {
				cpu->regPC++;
				cpu->flgHALT = 0;
			} else if (cpu->flgResPV) {
				cpu->flgPV = 0;
			}
			cpu->opTab = npTab;
			switch(cpu->regIM) {
				case 0:
					cpu->t = 2;
					cpu->op = &cpu->opTab[cpu->xack(cpu->xptr)];
					cpu->regR++;
					cpu->t += cpu->op->t;		// +5 (RST38 fetch)
					cpu->op->exec(cpu);		// +3 +3 execution. 13 total
					while (cpu->op->flag & OF_PREFIX) {
						cpu->op = &cpu->opTab[z80_fetch(cpu)]; // cpu->mrd(cpu->pc++,1,cpu->xptr)];
						cpu->regR++;
						cpu->t += cpu->op->t - 3;
						cpu->op->exec(cpu);
					}
					break;
				case 1:
					cpu->regR++;
					cpu->t = 2 + 5;	// 2 extra + 5 on RST38 fetch
					z80_call(cpu, 0x38);	// +3 +3 execution. 13 total
					break;
				case 2:
					cpu->regR++;
					cpu->t = 7;
					z80_push(cpu, cpu->regPC);			// +3 (10) +3 (13)
					cpu->regWZl = cpu->xack(cpu->xptr);	// int vector (FF)
					cpu->regWZh = cpu->regI;
					cpu->regPCl = z80_mrd(cpu, cpu->regWZ++);	// +3 (16)
					cpu->regPCh = z80_mrd(cpu, cpu->regWZ);	// +3 (19)
					cpu->regWZ = cpu->regPC;
					break;
			}
			res = cpu->t;
			cpu->intrq &= ~Z80_INT;
		}
	} else if (cpu->intrq & Z80_NMI) {			// nmi
		if (!cpu->flgNOINT) {
			cpu->regR++;
			cpu->flgIFF2 = cpu->flgIFF1;
			cpu->flgIFF1 = 0;
			cpu->t = 5;
			z80_push(cpu, cpu->regPC);
			cpu->regPC = 0x0066;
			cpu->regWZ = cpu->regPC;
			res = cpu->t;		// always 11
		}
		cpu->intrq &= ~Z80_NMI;
	}
	return res;
}

int z80_exec(CPU* cpu) {
	int res = 0;
	if (cpu->intrq & cpu->inten) {
		res = z80_int(cpu);
	}
	cpu->flgResPV = 0;
	cpu->flgNOINT = 0;
	if (!res) {
		cpu->t = 0;
		cpu->opTab = npTab;
		do {
			cpu->com = z80_fetch(cpu); // cpu->mrd(cpu->pc++,1,cpu->xptr);
			cpu->op = &cpu->opTab[cpu->com];
			cpu->regR++;
			cpu->t += cpu->op->t - 3;
			cpu->op->exec(cpu);
		} while (cpu->op->flag & OF_PREFIX);
		res = cpu->t;
	}
	return res;
}

// disasm

//static unsigned char z80_cnd[4] = {Z80_FZ, Z80_FC, Z80_FP, Z80_FS};

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
		madr = cpu->regHL;
	} else if (strstr(opc->mnem, "(de)")) {
		mn.mem = 1;
		madr = cpu->regDE;
	} else if (strstr(opc->mnem, "(bc)")) {
		mn.mem = 1;
		madr = cpu->regBC;
	} else if (strstr(opc->mnem, "(ix")) {
		mn.mem = 1;
		if (opt != ddcbTab) e = mrd(adr, data);
		madr = cpu->regIX + (signed char)e;
	} else if (strstr(opc->mnem, "(iy")) {
		mn.mem = 1;
		if (opt != fdcbTab) e = mrd(adr, data);
		madr = cpu->regIY + (signed char)e;
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
		mn.met = (cpu->regB == 1) ? 0 : 1;
	} else if (opt == npTab) {
		if (((op & 0xc7) == 0xc2) || ((op & 0xc7) == 0xc4) || ((op & 0xc7) == 0xc0)) {		// call, jp, ret
			mn.cond = 1;
			// mn.met = (cpu->f & z80_cnd[(op >> 4) & 3]) ? 0 : 1;
			switch((op >> 4) & 3) {
				case 0: mn.met = !cpu->flgZ; break;
				case 1: mn.met = !cpu->flgC; break;
				case 2: mn.met = !cpu->flgPV; break;
				case 3: mn.met = !cpu->flgS; break;
			}
			if (op & 8)
				mn.met ^= 1;
		} else if (op == 0x18) {								// jr
			mn.cond = 1;
			mn.met = 1;
		} else if ((op & 0xe7) == 0x20) {							// jr cc
			mn.cond = 1;
			//mn.met = (cpu->f & z80_cnd[(op >> 4) & 1] ? 0 : 1);
			mn.met = (op & 0x10) ? !cpu->flgZ : !cpu->flgC;
			if (op & 8)
				mn.met ^= 1;
		}
	}
	return mn;
}

// asm

xAsmScan z80_asm(int adr, const char* cbuf, char* buf) {
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

xRegDsc z80RegTab[] = {
	{Z80_REG_PC, "PC", REG_WORD | REG_RDMP | REG_PC, offsetof(CPU, regPC)},
	{Z80_REG_AF, "AF", REG_WORD, 0},
	{Z80_REG_BC, "BC", REG_WORD | REG_RDMP, offsetof(CPU, regBC)},
	{Z80_REG_DE, "DE", REG_WORD | REG_RDMP, offsetof(CPU, regDE)},
	{Z80_REG_HL, "HL", REG_WORD | REG_RDMP, offsetof(CPU, regHL)},

	{Z80_REG_SP, "SP", REG_WORD | REG_RDMP | REG_SP, offsetof(CPU, regSP)},
	{Z80_REG_AFA, "AF'", REG_WORD, 0},
	{Z80_REG_BCA, "BC'", REG_WORD | REG_RDMP, offsetof(CPU, regBCa)},
	{Z80_REG_DEA, "DE'", REG_WORD | REG_RDMP, offsetof(CPU, regDEa)},
	{Z80_REG_HLA, "HL'", REG_WORD | REG_RDMP, offsetof(CPU, regHLa)},

	{Z80_REG_IX, "IX", REG_WORD | REG_RDMP, offsetof(CPU, regIX)},
	{Z80_REG_IY, "IY", REG_WORD | REG_RDMP, offsetof(CPU, regIY)},
	{Z80_REG_I, "I", REG_BYTE, offsetof(CPU, regI)},
	{Z80_REG_R, "R", REG_BYTE, offsetof(CPU, regR)},
	{REG_EMPTY, "A", REG_BYTE, offsetof(CPU, regA)},
	{REG_EMPTY, "F", REG_32, 0},
	{REG_EMPTY, "A'", REG_BYTE, offsetof(CPU, regAa)},
	{REG_EMPTY, "F'", REG_32, offsetof(CPU, regFa)},
#ifdef ISDEBUG
	{REG_MPTR, "WZ", REG_WORD | REG_RDMP, offsetof(CPU, regWZ)},
#endif
	{Z80_REG_IM, "IM", REG_2, offsetof(CPU, regIM)},
	{Z80_FLG_IFF1, "IFF1", REG_BIT, offsetof(CPU, flgIFF1)},
	{Z80_FLG_IFF2, "IFF2", REG_BIT, offsetof(CPU, flgIFF2)},
	{REG_NONE, "", 0, 0}
};

static char* z80Flags = "SZ5H3PNC";

void z80_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	int bidx = 0;
	PAIR(w,h,l)rx;
	xRegister reg;
	while(z80RegTab[idx].id != REG_NONE) {
		reg.id = z80RegTab[idx].id;
		reg.name = z80RegTab[idx].name;
		reg.type = z80RegTab[idx].type;
		switch(z80RegTab[idx].id) {
			case Z80_REG_PC: reg.value = cpu->regPC; break;
			case Z80_REG_SP: reg.value = cpu->regSP; break;
			case Z80_REG_AF: rx.h = cpu->regA;
					rx.l = z80_get_flag(cpu) & 0xff;
					reg.value = rx.w;
					break;
			case Z80_REG_BC: reg.value = cpu->regBC; break;
			case Z80_REG_DE: reg.value = cpu->regDE; break;
			case Z80_REG_HL: reg.value = cpu->regHL; break;
			case Z80_REG_AFA: rx.h = cpu->regAa;
					rx.l = cpu->regFa;
					reg.value = rx.w;
					break;
			case Z80_REG_BCA: reg.value = cpu->regBCa; break;
			case Z80_REG_DEA: reg.value = cpu->regDEa; break;
			case Z80_REG_HLA: reg.value = cpu->regHLa; break;
			case Z80_REG_IX: reg.value = cpu->regIX; break;
			case Z80_REG_IY: reg.value = cpu->regIY; break;
			case Z80_REG_I: reg.value = cpu->regI; break;
			case Z80_REG_R: reg.value = (cpu->regR & 0x7f) | (cpu->regR7 & 0x80); break;
			case Z80_REG_IM: reg.value = cpu->regIM & 3; break;
			case Z80_FLG_IFF1: reg.value = cpu->flgIFF1; break;
			case Z80_FLG_IFF2: reg.value = cpu->flgIFF2; break;
			case REG_MPTR: reg.value = cpu->regWZ; break;
		}
		if (reg.id != REG_EMPTY) {
			bunch->regs[bidx] = reg;
			bidx++;
		}
		idx++;
	}
	bunch->regs[idx].id = REG_NONE;
	bunch->flags = z80Flags;
	// memcpy(bunch->flags, "SZ5H3PNC", 8);
}

void z80_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx;
	xRegister* rd;
	PAIR(w,h,l)rx;
	for (idx = 0; idx < 32; idx++) {
		rd = &bunch.regs[idx];
		switch(rd->id) {
			case Z80_REG_PC: cpu->regPC = rd->value; break;
			case Z80_REG_SP: cpu->regSP = rd->value; break;
			case Z80_REG_AF: rx.w = rd->value;
					cpu->regA = rx.h;
					z80_set_flag(cpu, rx.l);
					break;
			case Z80_REG_BC: cpu->regBC = rd->value; break;
			case Z80_REG_DE: cpu->regDE = rd->value; break;
			case Z80_REG_HL: cpu->regHL = rd->value; break;
			case Z80_REG_AFA: rx.w = rd->value;
					cpu->regAa = rx.h;
					cpu->regFa = rx.l;
					break;
			case Z80_REG_BCA: cpu->regBCa = rd->value; break;
			case Z80_REG_DEA: cpu->regDEa = rd->value; break;
			case Z80_REG_HLA: cpu->regHLa = rd->value; break;
			case Z80_REG_IX: cpu->regIX = rd->value; break;
			case Z80_REG_IY: cpu->regIY = rd->value; break;
			case Z80_REG_I: cpu->regI = rd->value & 0xff; break;
			case Z80_REG_R:
				cpu->regR = rd->value;
				cpu->regR7 = rd->value & 0x80;
				break;
			case Z80_REG_IM: cpu->regIM = (rd->value & 2) ? 2 : rd->value & 1; break;
			case Z80_FLG_IFF1: cpu->flgIFF1 = !!rd->value; break;
			case Z80_FLG_IFF2: cpu->flgIFF2 = !!rd->value; break;
			case REG_MPTR: cpu->regWZ = rd->value; break;
			case REG_NONE: idx = 100; break;
		}
	}
}

// test:
static cpuCore z80core = {CPU_Z80, 0,"Z80ext", z80RegTab, NULL, z80_reset, z80_exec, z80_asm, z80_mnem, z80_get_regs, z80_set_regs, z80_get_flag, z80_set_flag};
EXPORTDLL cpuCore* getCore() {return &z80core;}
