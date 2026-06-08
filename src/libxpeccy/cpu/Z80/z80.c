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
	cpu->flgRetBRK = 0;
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
			if (cpu->flgRetBRK) {
				cpu->regCallCnt++;
			}
			switch(cpu->regIM) {
				case 0:
					cpu->t = 2;
					cpu->op = &cpu->opTab[cpu->xack(cpu->xptr) & 0xff];
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
		cpu->t--;
		cpu_irq(cpu, IRQ_CPU_ACK);
		cpu->t++;
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
			mn.met = (op & 0x10) ? !cpu->flgC : !cpu->flgZ;
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

void z80_set_pc(CPU* cpu, int v) {cpu->regPC = v;}
void z80_set_sp(CPU* cpu, int v) {cpu->regSP = v;}
void z80_set_a(CPU* cpu, int v) {cpu->regA = v;}
void z80_set_bc(CPU* cpu, int v) {cpu->regBC = v;}
void z80_set_de(CPU* cpu, int v) {cpu->regDE = v;}
void z80_set_hl(CPU* cpu, int v) {cpu->regHL = v;}
void z80_set_aa(CPU* cpu, int v) {cpu->regAa = v;}
void z80_set_abc(CPU* cpu, int v) {cpu->regBCa = v;}
void z80_set_ade(CPU* cpu, int v) {cpu->regDEa = v;}
void z80_set_ahl(CPU* cpu, int v) {cpu->regHLa = v;}
void z80_set_ix(CPU* cpu, int v) {cpu->regIX = v;}
void z80_set_iy(CPU* cpu, int v) {cpu->regIY = v;}
void z80_set_i(CPU* cpu, int v) {cpu->regI = v;}
void z80_set_r(CPU* cpu, int v) {cpu->regR = v; cpu->regR7 = v & 0x80;}
void z80_set_wz(CPU* cpu, int v) {cpu->regWZ = v;}
void z80_set_im(CPU* cpu, int v) {cpu->regIM = (v & 2) ? 2 : v & 1;}
void z80_set_iff1(CPU* cpu, int v) {cpu->flgIFF1 = !!v;}
void z80_set_iff2(CPU* cpu, int v) {cpu->flgIFF2 = !!v;}
void z80_set_fa(CPU* cpu, int v) {cpu->regFa = v;}

int z80_get_pc(CPU* cpu) {return cpu->regPC;}
int z80_get_sp(CPU* cpu) {return cpu->regSP;}
int z80_get_a(CPU* cpu) {return cpu->regA;}
int z80_get_bc(CPU* cpu) {return cpu->regBC;}
int z80_get_de(CPU* cpu) {return cpu->regDE;}
int z80_get_hl(CPU* cpu) {return cpu->regHL;}
int z80_get_aa(CPU* cpu) {return cpu->regAa;}
int z80_get_abc(CPU* cpu) {return cpu->regBCa;}
int z80_get_ade(CPU* cpu) {return cpu->regDEa;}
int z80_get_ahl(CPU* cpu) {return cpu->regHLa;}
int z80_get_ix(CPU* cpu) {return cpu->regIX;}
int z80_get_iy(CPU* cpu) {return cpu->regIY;}
int z80_get_i(CPU* cpu) {return cpu->regI;}
int z80_get_r(CPU* cpu) {return (cpu->regR & 0x7f) | (cpu->regR7);}
int z80_get_wz(CPU* cpu) {return cpu->regWZ;}
int z80_get_im(CPU* cpu) {return cpu->regIM;}
int z80_get_iff1(CPU* cpu) {return cpu->flgIFF1;}
int z80_get_iff2(CPU* cpu) {return cpu->flgIFF2;}
int z80_get_fa(CPU* cpu) {return cpu->regFa;}

//static char* z80Flags = "SZ5H3PNC";

xRegDsc z80RegTab[] = {
	{Z80_REG_PC, "PC", REG_WORD, REG_RDMP | REG_PC, z80_get_pc, z80_set_pc},
	{Z80_REG_A, "A", REG_BYTE, 0, z80_get_a, z80_set_a},
	{Z80_REG_BC, "BC", REG_WORD, REG_RDMP, z80_get_bc, z80_set_bc},
	{Z80_REG_DE, "DE", REG_WORD, REG_RDMP, z80_get_de, z80_set_de},
	{Z80_REG_HL, "HL", REG_WORD, REG_RDMP, z80_get_hl, z80_set_hl},

	{Z80_REG_SP, "SP", REG_WORD, REG_RDMP | REG_SP, z80_get_sp, z80_set_sp},
	{Z80_REG_AA, "A'", REG_BYTE, 0, z80_get_aa, z80_set_aa},
	{Z80_REG_BCA, "BC'", REG_WORD, REG_RDMP, z80_get_abc, z80_set_abc},
	{Z80_REG_DEA, "DE'", REG_WORD, REG_RDMP, z80_get_ade, z80_set_ade},
	{Z80_REG_HLA, "HL'", REG_WORD, REG_RDMP, z80_get_ahl, z80_set_ahl},

	{Z80_REG_IX, "IX", REG_WORD, REG_RDMP, z80_get_ix, z80_set_ix},
	{Z80_REG_IY, "IY", REG_WORD, REG_RDMP, z80_get_iy, z80_set_iy},
	{Z80_REG_I, "I", REG_BYTE, 0, z80_get_i, z80_set_i},
	{Z80_REG_R, "R", REG_BYTE, 0, z80_get_r, z80_set_r},
	{REG_EMPTY, "F", REG_32, REG_FLG, z80_get_flag, z80_set_flag},
	{REG_EMPTY, "F'", REG_32, 0, z80_get_fa, z80_set_fa},
#ifdef ISDEBUG
	{Z80_REG_WZ, "WZ", REG_WORD, REG_RDMP, z80_get_wz, z80_set_wz},
#endif
	{Z80_REG_IM, "IM", REG_2, 0, z80_get_im, z80_set_im},
	{Z80_FLG_IFF1, "IFF1", REG_BIT, 0, z80_get_iff1, z80_set_iff1},
	{Z80_FLG_IFF2, "IFF2", REG_BIT, 0, z80_get_iff2, z80_set_iff2},
	{REG_EOT, "SZ5H3PNC", 0, 0, NULL, NULL}				// name of REG_EOT element is flag names
};

/*
void z80_get_regs(CPU* cpu, xRegBunch* bunch) {
	int bidx = 0;
	xRegister reg;
	xRegDsc* itm = z80RegTab;
	while(itm->id != REG_EOT) {
		reg.id = itm->id;
		reg.name = itm->name;
		reg.type = itm->size;
		reg.flag = itm->flag;
		reg.value = itm->get ? itm->get(cpu) : -1;
		if (reg.id != REG_EMPTY) {
			bunch->regs[bidx] = reg;
			bidx++;
		}
		itm++;
	}
	bunch->regs[bidx].id = REG_EOT;
	bunch->flags = z80Flags;
}

void z80_set_regs(CPU* cpu, xRegBunch bunch) {
	xRegister* rd = bunch.regs;
	xRegDsc* p;
	int idx = 0;
	while((idx < 32) && (rd->id != REG_EOT)) {
		p = cpu->core->rdsctab;
		while (p->id != REG_EOT) {
			if (p->id == rd->id) {
				if (p->set) {
					p->set(cpu, rd->value);
				}
			}
			p++;
		}
		rd++;
		idx++;
	}
}
*/

// test (for external lib):
// {id, family_id, generation, name, regtab, adr.size, data.size, @init, @reset, @exec, @txt2code, @disasm}
// last entry must me with id=CPU_NONE
static cpuCore z80core[] = {
	{CPU_Z80, CPUG_X80, 0,"Z80ext", z80RegTab, 16, 8, NULL, z80_reset, z80_exec, z80_asm, z80_mnem},
	{CPU_NONE, CPUG_NONE, 0, "none", NULL, 8, 8, NULL, NULL, NULL, NULL, NULL}
};

EXPORTDLL cpuCore* getCore() {return z80core;}
