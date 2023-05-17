#include <stdio.h>
#include <string.h>

#include "i80286.h"

extern opCode i80286_tab[256];
extern opCode i286_0f_tab[256];

extern xSegPtr i286_cash_seg(CPU*, unsigned short);

void i286_reset(CPU* cpu) {
	cpu->f = 0x0002;
	cpu->msw = 0xfff0;
	cpu->pc = 0xfff0;	// ip
	cpu->cs = i286_cash_seg(cpu, 0xf000);
	cpu->ss = i286_cash_seg(cpu, 0x0000);
	cpu->ds = i286_cash_seg(cpu, 0x0000);
	cpu->es = i286_cash_seg(cpu, 0x0000);
	cpu->idtr = i286_cash_seg(cpu, 0x0000);
	cpu->intrq = 0;
}

// REAL mode:

extern void i286_push(CPU*, unsigned short);

void i286_int_real(CPU* cpu, int vec) {
	if (cpu->halt) {
		cpu->halt = 0;
		cpu->pc++;
	}
	i286_push(cpu, cpu->f);
	i286_push(cpu, cpu->cs.idx);
	i286_push(cpu, cpu->pc);
	cpu->f &= ~(I286_FI | I286_FT);
	cpu->tmpi = (vec & 0xff) << 2;
	PAIR(w,h,l)seg;
	PAIR(w,h,l)off;
	off.l = i286_mrd(cpu, cpu->idtr, 0, cpu->tmpi++);	// adr
	off.h = i286_mrd(cpu, cpu->idtr, 0, cpu->tmpi++);
	seg.l = i286_mrd(cpu, cpu->idtr, 0, cpu->tmpi++);	// seg
	seg.h = i286_mrd(cpu, cpu->idtr, 0, cpu->tmpi);
	cpu->cs = i286_cash_seg(cpu, seg.w);
	cpu->pc = off.w;
}

// PRT mode: descriptors @ IDTR segment
// interrupt will clear IF flag
// trap will not
// NT flag must be cleared after pushing flag on stack
// +0,1 = offset of INT code
// +2,3 = segment selector of INT code
// +4 unused
// +5	bit 7: present (must be 1)
//	bit5,6: DPL. if (DPL > CPL=CS PL) then INT GPT
//	bit1-4: 0011
//	bit0: 1 for trap, 0 for interrupt
// +6,7 unused

void i286_int_prt(CPU* cpu, int vec) {
	if (cpu->idtr.limit < (vec & 0xfff8)) {		// check idtr limit
		i286_int_prt(cpu, 13);
		cpu->t = 1;
	} else if (cpu->f & I286_FI) {
		PAIR(w,h,l)seg;
		PAIR(w,h,l)off;
		int adr = cpu->idtr.base + (vec & 0xfff8);
		off.l = cpu->mrd(adr, 0, cpu->xptr);	// offset
		off.w = cpu->mrd(adr+1, 0, cpu->xptr);
		seg.l = cpu->mrd(adr+2, 0, cpu->xptr);	// segment
		seg.h = cpu->mrd(adr+3, 0, cpu->xptr);
		unsigned char fl = cpu->mrd(adr+5, 0, cpu->xptr);	// flags
		cpu->tmpdr = i286_cash_seg(cpu, seg.w);
		if ((cpu->tmpdr.flag & 0x60) < (fl & 0x60)) {	// check priv
			i286_int_prt(cpu, 13);
		} else {					// do interrupt
			if (cpu->halt) {
				cpu->halt = 0;
				cpu->pc++;
			}
			i286_push(cpu, cpu->f);
			if (!(fl & 1)) cpu->f &= ~I286_FI;
			i286_push(cpu, cpu->cs.idx);
			i286_push(cpu, cpu->pc);
			i286_push(cpu, cpu->errcod & 0xffff);
			cpu->cs = i286_cash_seg(cpu, seg.w);
			cpu->pc = off.w;
		}
		cpu->t = 1;
	}
}

void i286_interrupt(CPU* cpu, int vec) {
	if (cpu->msw & I286_FPE) {
		i286_int_prt(cpu, vec);
	} else {
		i286_int_real(cpu, vec);
	}
}

// external INT
void i286_ext_int(CPU* cpu) {
	if (cpu->f & I286_FI) {
		cpu->intrq &= ~I286_INT;
		i286_interrupt(cpu, cpu->intvec);
	} else if ((cpu->intrq & I286_NMI) && !(cpu->inten & I286_BLK_NMI)) {
		cpu->inten |= I286_BLK_NMI;				// NMI is blocking until RETI, but next NMI will be remembered until then
		cpu->intrq &= ~I286_NMI;
		i286_interrupt(cpu, 2);
	}
}

int i286_exec(CPU* cpu) {
	cpu->t = 0;
	cpu->opTab = i80286_tab;
	cpu->seg.idx = -1;
	cpu->lock = 0;
	cpu->rep = I286_REP_NONE;
	cpu->oldpc = cpu->pc;
	if (cpu->intrq)
		i286_ext_int(cpu);
	if (cpu->t == 0) {
		cpu->t++;
		do {
			cpu->t++;
			cpu->com = i286_mrd(cpu, cpu->cs, 0, cpu->pc);
			cpu->pc++;
			cpu->op = &cpu->opTab[cpu->com & 0xff];
			cpu->op->exec(cpu);
		} while (cpu->op->flag & OF_PREFIX);
	}
	return cpu->t;
}

static char mnembuf[128];
static char strbuf[128];

const char* str_regb[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};
const char* str_regw[8] = {"ax","cx","dx","bx","sp","bp","si","di"};
const char* str_regs[4] = {"es","cs","ss","ds"};
const char* str_ea[8] = {"bx+si","bx+di","bp+si","bp+di","si","di","bp","bx"};
int seg_ea[8] = {3,3,2,2,3,3,2,3};
const char* str_alu[8] = {"add","or","adc","sbb","and","sub","xor","cmp"};
const char* str_rot[8] = {"rol","ror","rcl","rcr","shl","shr","sal","sar"};
const char* str_opX[8] = {"test","*test","not","neg","mul","imul","div","idiv"};
const char* str_opE[8] = {"inc","dec","call","callf","jmp","jmpf","push","fe /7"};
const char* str_opF[8] = {"incw","decw","call","callf","jmp","jmpf","push","ff /7"};
const char* str_opQ[8] = {"sldt","str","lldt","ltr","verr","verw","0f00 /6","0f00 /7"};
const char* str_opW[8] = {"sgdt","sidt","lgdt","lidt","smsw","0f01 /5","lmsw","0f01 /7"};

xMnem i286_mnem(CPU* cpu, int sadr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.cond = 0;
	mn.met = 0;
	mn.mem = 0;
	mn.oadr = -1;
	int rep = I286_REP_NONE;
	int rseg = -1;
	opCode* tab = i80286_tab;
	opCode* op;
	int adr = sadr;
	unsigned char com;
	do {
		com = cpu->mrd(adr, 0, cpu->xptr);
		adr++;
		op = &tab[com];
		if (tab == i80286_tab) {
			switch (com) {
				case 0x0f: tab = i286_0f_tab; break;
				case 0x26: rseg = 0; break;		// es
				case 0x2e: rseg = 1; break;		// cs
				case 0x36: rseg = 2; break;		// ss
				case 0x3e: rseg = 3; break;		// ds
				case 0xf2: rep = I286_REPNZ; break;
				case 0xf3: rep = I286_REPZ; break;
			}
		}
		mn.flag = op->flag;
		mn.len++;
	} while (op->flag & OF_PREFIX);
	strcpy(strbuf, op->mnem);
	char* ptr = strbuf;
	char* dptr = mnembuf;
	int mod = 0;
	unsigned char mb = 0;
	PAIR(w,h,l)rx;
	PAIR(w,h,l)seg;
	PAIR(w,h,l)disp;
	do {
		if (*ptr == ':') {
			ptr++;
			switch(*ptr) {
				case '1':
					rx.l = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					dptr += sprintf(dptr, "#%.2X", rx.l);
					break;
				case '2':
					rx.l = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					rx.h = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					dptr += sprintf(dptr, "#%.4X", rx.w);
					mn.oadr = rx.w + cpu->cs.base;
					break;
				case '3':
					rx.l = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					rx.h = (rx.l & 0x80) ? 0xff : 0x00;
					rx.w += adr - cpu->cs.base;
					dptr += sprintf(dptr, "short #%.4X", rx.w);
					mn.oadr = rx.w + cpu->cs.base;
					break;
				case 'n':
					rx.l = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					rx.h = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					rx.w += (adr - cpu->cs.base);
					dptr += sprintf(dptr, "near #%.4X", rx.w);
					mn.oadr = rx.w + cpu->cs.base;
					break;
				case '4':
				case 'p':
					rx.l = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					rx.h = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					seg.l = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					seg.h = cpu->mrd(adr, 0, cpu->xptr);
					adr++;
					dptr += sprintf(dptr, "#%.4X::%.4X", seg.w, rx.w);
					// mn.oadr = rx.w;
					break;
				case 'r': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					if (op->flag & OF_WORD) {	// word
						dptr += sprintf(dptr, "%s", str_regw[(mb >> 3) & 7]);
					} else {
						dptr += sprintf(dptr, "%s", str_regb[(mb >> 3) & 7]);
					}
					break;
				case 's': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_regs[(mb >> 3) & 3]);
					break;
				case 'e': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					if ((mb & 0xc0) == 0xc0) {		// reg
						if (op->flag & OF_WORD) {
							dptr += sprintf(dptr, "%s", str_regw[mb & 7]);
						} else {
							dptr += sprintf(dptr, "%s", str_regb[mb & 7]);
						}
					} else {
						disp.w = 0;
						if ((mb & 0xc0) == 0x40) {
							disp.l = cpu->mrd(adr, 0, cpu->xptr);
							disp.h = (disp.l & 0x80) ? 0xff : 0x00;
							adr++;
						} else if ((mb & 0xc0) == 0x80) {
							disp.l = cpu->mrd(adr, 0, cpu->xptr);
							adr++;
							disp.h = cpu->mrd(adr, 0, cpu->xptr);
							adr++;
						}
						*(dptr++) = '[';
						if (rseg < 0)
							rseg = ((mb & 7)==6) ? ((mb & 0xc0) ? 2 : 3) : seg_ea[mb & 7];	// default segment if not overriden
						dptr += sprintf(dptr, "%s::", str_regs[rseg & 3]);
						if ((mb & 0xc7) == 0x06) {	// immw, [seg:disp]
							disp.l = cpu->mrd(adr, 0, cpu->xptr);
							adr++;
							disp.h = cpu->mrd(adr, 0, cpu->xptr);
							adr++;
							dptr += sprintf(dptr, "#%.4X", disp.w);
							// disp.w = 0;
						} else if ((mb & 0xc0) == 0x40) {			// disp is 1 byte
							dptr += sprintf(dptr, "%s", str_ea[mb & 7]);
							if (disp.l) {
								if (disp.w & 0x80) {	// negative
									dptr += sprintf(dptr,"-#%.2X", 0x100-disp.l);
								} else {		// positive
									dptr += sprintf(dptr,"+#%.2X", disp.l);
								}
							}
						} else {						// disp is 2 byte or 0
							if (disp.w)		// if disp!=0
								dptr += sprintf(dptr, "#%.4X+", disp.w);
							dptr += sprintf(dptr, "%s", str_ea[mb & 7]);	// TODO: do something to show segment override

						}
						*(dptr++) = ']';
					}
					break;
				case 'D':
					if (rseg < 0) rseg = 3; // ds default
					dptr += sprintf(dptr, "%s", str_regs[rseg & 3]);
					break;
				case 'L':
					if ((com & 0xf6) == 0xa6) {		// a6/a7/ae/af
						if (rep == I286_REPZ) {
							dptr += sprintf(dptr, "REPZ ");
						} else if (rep == I286_REPNZ) {
							dptr += sprintf(dptr, "REPNZ ");
						}
					} else if (rep != I286_REP_NONE) {
						dptr += sprintf(dptr, "REP ");
					}
					break;
				case 'A': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_alu[(mb >> 3) & 7]);
					break;
				case 'R': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_rot[(mb >> 3) & 7]);
					break;
				case 'X': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					if (!(mb & 0x30)) {		// f6 /0 and f6 /1 : test :e,:1
						strcat(ptr, ",:1");
					}
					dptr += sprintf(dptr, "%s", str_opX[(mb >> 3) & 7]);
					break;
				case 'Y': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					if (!(mb & 0x30)) {		// f7 /0 and f7 /1 : test :e,:2
						strcat(ptr, ",:2");
					}
					dptr += sprintf(dptr, "%s", str_opX[(mb >> 3) & 7]);
					break;
				case 'E': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_opE[(mb >> 3) & 7]);
					break;
				case 'F': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_opF[(mb >> 3) & 7]);
					break;
				case 'Q': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_opQ[(mb >> 3) & 7]);
					break;
				case 'W': if (!mod) {mod = 1; mb = cpu->mrd(adr, 0, cpu->xptr); adr++;}
					dptr += sprintf(dptr, "%s", str_opW[(mb >> 3) & 7]);
					break;
				case ':':
					*(dptr++) = ':';	// "::" will be replaced by ":" in cpuDisasm
					*(dptr++) = ':';
					break;
				default:
					*dptr = *ptr;
					dptr++;
					break;
			}
			ptr++;
		} else {
			*dptr = *ptr;
			dptr++;
			ptr++;
		}
	} while (*ptr != 0);
	*dptr = 0;
	mn.mnem = mnembuf;
	mn.len = adr - sadr;
	return mn;
}

// asm

xRegDsc i286RegTab[] = {
	{I286_IP, "IP", REG_WORD, offsetof(CPU, pc)},
	{I286_SP, "SP", REG_WORD, offsetof(CPU, sp)},
	{I286_BP, "BP", REG_WORD, offsetof(CPU, bp)},
	{I286_SI, "SI", REG_WORD, offsetof(CPU, si)},
	{I286_DI, "DI", REG_WORD, offsetof(CPU, di)},
	{I286_AX, "AX", REG_WORD, offsetof(CPU, ax)},
	{I286_BX, "BX", REG_WORD, offsetof(CPU, bx)},
	{I286_CX, "CX", REG_WORD, offsetof(CPU, cx)},
	{I286_DX, "DX", REG_WORD, offsetof(CPU, dx)},
	{I286_CS, "CS", REG_WORD|REG_SEG, offsetof(CPU, cs)},
	{I286_SS, "SS", REG_WORD|REG_SEG, offsetof(CPU, ss)},
	{I286_DS, "DS", REG_WORD|REG_SEG, offsetof(CPU, ds)},
	{I286_ES, "ES", REG_WORD|REG_SEG, offsetof(CPU, es)},
	{I286_MSW, "MSW", REG_RO|REG_WORD, offsetof(CPU, msw)},
	{I286_LDT, "LDT", REG_RO|REG_WORD|REG_SEG, offsetof(CPU, ldtr)},
	{I286_GDT, "GDT", REG_RO|REG_24, offsetof(CPU, gdtr)},
	{I286_IDT, "IDT", REG_RO|REG_24, offsetof(CPU, idtr)},
	{REG_NONE, "", 0, 0}
};

xAsmScan i286_asm(const char* mnm, char* buf) {
	xAsmScan res;
	res.match = 0;
	return res;
}

char* i286_flags = "-N**ODITSZ-A-P-C";

void i286_get_regs(CPU* cpu, xRegBunch* bnch) {
	int idx = 0;
	int val, bas;
	while (i286RegTab[idx].id != REG_NONE) {
		bnch->regs[idx].id = i286RegTab[idx].id;
		bnch->regs[idx].name = i286RegTab[idx].name;
		bnch->regs[idx].type = i286RegTab[idx].type;
		val = -1;
		bas = 0;
		switch (i286RegTab[idx].id) {
			case I286_IP: val = cpu->pc; break;
			case I286_SP: val = cpu->sp; break;
			case I286_BP: val = cpu->bp; break;
			case I286_SI: val = cpu->si; break;
			case I286_DI: val = cpu->di; break;
			case I286_AX: val = cpu->ax; break;
			case I286_CX: val = cpu->cx; break;
			case I286_DX: val = cpu->dx; break;
			case I286_BX: val = cpu->bx; break;
			case I286_CS: val = cpu->cs.idx; bas = cpu->cs.base; break;
			case I286_SS: val = cpu->ss.idx; bas = cpu->ss.base; break;
			case I286_DS: val = cpu->ds.idx; bas = cpu->ds.base; break;
			case I286_ES: val = cpu->es.idx; bas = cpu->es.base; break;
			case I286_MSW: val = cpu->msw; break;
			case I286_GDT: val = cpu->gdtr.base; break;
			case I286_LDT: val = cpu->ldtr.idx; break;
			case I286_IDT: val = cpu->idtr.base; break;
		}
		bnch->regs[idx].value = val;
		bnch->regs[idx].base = bas;
		idx++;
	}
	bnch->regs[idx].id = REG_NONE;
	bnch->flags = i286_flags;
}

void i286_set_regs(CPU* cpu, xRegBunch bnch) {
	int idx = 0;
	int val;
	while (bnch.regs[idx].id != REG_NONE) {
		val = bnch.regs[idx].value;
		switch(bnch.regs[idx].id) {
			case I286_IP: cpu->pc = val; break;
			case I286_SP: cpu->sp = val; break;
			case I286_BP: cpu->bp = val; break;
			case I286_SI: cpu->si = val; break;
			case I286_DI: cpu->di = val; break;
			case I286_AX: cpu->ax = val; break;
			case I286_CX: cpu->cx = val; break;
			case I286_DX: cpu->dx = val; break;
			case I286_BX: cpu->bx = val; break;
			case I286_CS: cpu->cs = i286_cash_seg(cpu, val); break;
			case I286_SS: cpu->ss = i286_cash_seg(cpu, val); break;
			case I286_DS: cpu->ds = i286_cash_seg(cpu, val); break;
			case I286_ES: cpu->es = i286_cash_seg(cpu, val); break;
		}
		idx++;
	}
}
