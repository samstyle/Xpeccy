#include <stdio.h>
#include <string.h>

#include "i80286.h"

extern opCode i80286_tab[256];
extern opCode i286_0f_tab[256];

extern xSegPtr i286_cash_seg(CPU*, unsigned short);
extern xSegPtr i286_cash_seg_a(CPU*, int);
extern void i286_push(CPU*, unsigned short);
extern unsigned short i286_sys_mrdw(CPU*, xSegPtr, unsigned short);
extern void i286_switch_task(CPU*, int, int, int);

void i086_init(CPU* cpu) {cpu->gen = 0;}
void i186_init(CPU* cpu) {cpu->gen = 1;}
void i286_init(CPU* cpu) {cpu->gen = 2;}

/*
#define I286_FC	0x0001	// carry
#define I286_FP 0x0004	// parity
#define I286_FA 0x0010	// half-carry
#define I286_FZ 0x0040	// zero
#define I286_FS 0x0080	// sign
#define I286_FT 0x0100	// trap
#define I286_FI 0x0200	// interrupt
#define I286_FD	0x0400	// direction
#define I286_FO 0x0800	// overflow
#define I286_FIP 0x3000	// 2bits: IOPL
#define I286_FN	0x4000	// nested flag
*/

void x86_set_flag(CPU* cpu, int v) {
	cpu->flgC = (v & 1);
	cpu->flgP = !!(v & 4);
	cpu->flgA = !!(v & 16);
	cpu->flgZ = !!(v & 64);
	cpu->flgS = !!(v & 128);
	cpu->flgT = !!(v & 256);
	cpu->flgI = !!(v & 512);
	cpu->flgD = !!(v & 1024);
	cpu->flgO = !!(v & 2048);
	cpu->regIOPL = (v >> 12) & 3;
	cpu->flgN = !!(v & 16384);
}

int x86_get_flag(CPU* cpu) {
	int f = cpu->flgC | 2\
			| (cpu->flgP << 2) | (cpu->flgA << 4) | (cpu->flgZ << 6) | (cpu->flgS << 7)\
			| (cpu->flgT << 8) | (cpu->flgI << 9) | (cpu->flgD << 10) | (cpu->flgO << 11)\
			| (cpu->regIOPL << 12) | (cpu->flgN << 14);
	switch(cpu->gen) {
		case 0:
		case 1: f |= 0xff00; break;
		case 2: f &= 0x0fff; break;
	}
	return f;
}

void i286_reset(CPU* cpu) {
	x86_set_mode(cpu, X86_REAL);
	x86_set_flag(cpu, 0x0002);
	cpu->regMSW = 0xfff0;
	switch (cpu->gen) {
		case 0:
		case 1:
			cpu->regIP = 0x0000;
			cpu->cs = i286_cash_seg(cpu, 0xffff);
			break;
		case 2:
			cpu->regIP = 0xfff0;	// ip
			cpu->cs = i286_cash_seg(cpu, 0xf000);
			break;
	}
	cpu->ss = i286_cash_seg(cpu, 0x0000);
	cpu->ds = i286_cash_seg(cpu, 0x0000);
	cpu->es = i286_cash_seg(cpu, 0x0000);
	cpu->idtr = i286_cash_seg(cpu, 0x0000);
	cpu->intrq = 0;

	cpu->regX87top = 0;
}

// REAL mode:


void i286_int_real(CPU* cpu, int vec) {
	if (cpu->flgHALT) {
		cpu->flgHALT = 0;
		cpu->regIP++;
	}
	i286_push(cpu, x86_get_flag(cpu));
	i286_push(cpu, cpu->cs.idx);
	i286_push(cpu, cpu->regIP);
	cpu->flgI = 0;
	cpu->flgT = 0;
	cpu->tmpi = (vec & 0xff) << 2;
	cpu->regIP = i286_sys_mrdw(cpu, cpu->idtr, cpu->tmpi);
	cpu->cs = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->idtr, cpu->tmpi+2));
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
//	bit0-4: type (5 task gate, 6 int gate, 7 trap gate)
// +6,7 unused

void i286_int_prt(CPU* cpu, int vec) {
	if (cpu->idtr.limit < (vec & 0xfff8)) {		// check idtr limit
		THROW_EC(I286_INT_GP, cpu->idtr.idx);
	} else if (cpu->flgI) {
		if (cpu->flgHALT) {
			cpu->flgHALT = 0;
			cpu->regIP++;
		}
		PAIR(w,h,l)seg;
		PAIR(w,h,l)off;
		int adr = cpu->idtr.base + (vec << 3);	// 8 bytes/entry
		cpu->gate = i286_cash_seg_a(cpu, adr);	// get gate descriptor inside IDT
		cpu->gate.idx = vec;
		switch(cpu->gate.ar) {
			case 5:		// task gate
				off.w = cpu->regIP;
				seg.w = cpu->cs.idx;
				adr = x86_get_flag(cpu);
				i286_switch_task(cpu, cpu->gate.base & 0xffff, 1, 0);
				i286_push(cpu, adr);
				i286_push(cpu, seg.w);
				i286_push(cpu, off.w);
				break;
			case 6:		// interrupt gate
			case 7:		// trap gate
				if (!cpu->gate.pr) {
					THROW_EC(I286_INT_GP, (vec << 3));
				} else {
					if (cpu->cs.pl > cpu->gate.pl) {	// priv.transitions
						off.w = cpu->regSP;
						seg.w = cpu->ss.idx;
						cpu->regSP = i286_sys_mrdw(cpu, cpu->tsdr, 2 + 4 * cpu->gate.pl);				// new stack
						cpu->ss = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 4 + 4 * cpu->gate.pl));
						i286_push(cpu, seg.w);		// push old ss:sp
						i286_push(cpu, off.w);
					}
					i286_push(cpu, x86_get_flag(cpu));
					i286_push(cpu, cpu->cs.idx);
					i286_push(cpu, cpu->regIP);
					if (cpu->errcod >= 0) {
						i286_push(cpu, cpu->errcod);
						cpu->errcod = -1;
					}
					off.l = cpu_mrd(cpu, adr);		// offset
					off.h = cpu_mrd(cpu, adr + 1);
					seg.l = cpu_mrd(cpu, adr + 2);	// cs segment selector
					seg.h = cpu_mrd(cpu, adr + 3);
					cpu->tmpdr = i286_cash_seg(cpu, seg.w);
					if (!cpu->tmpdr.code) {
						THROW_EC(I286_INT_GP, (vec << 3));
					} else if (!cpu->tmpdr.pr) {
						THROW_EC(I286_INT_NP, seg.w);
					} else if (cpu->regIP > cpu->tmpdr.limit) {
						THROW_EC(I286_INT_GP, 0);
					}
					cpu->regIP = off.w;
					cpu->cs = cpu->tmpdr;
					cpu->flgN = 0;			// clear N flag
					if (!(cpu->gate.ar & 1)) {	// disable interrupts on int.gate / don't change on trap gate
						cpu->flgI = 0;
					}
				}
				break;
			default:
				THROW_EC(I286_INT_GP, vec);
				break;
		}
		/*
		off.l = cpu_mrd(cpu, adr);	// offset
		off.h = cpu_mrd(cpu, adr+1);
		seg.l = cpu_mrd(cpu, adr+2);	// segment
		seg.h = cpu_mrd(cpu, adr+3);
		unsigned char fl = cpu_mrd(cpu, adr+5);	// flags
		cpu->tmpdr = i286_cash_seg(cpu, seg.w);		// [code] segment: TODO if this is tss gate
		if ((cpu->tmpdr.pl) < ((fl >> 5) & 3)) {	// check priv
			THROW_EC(I286_INT_GP, 0);
		} else {					// do interrupt
			if (cpu->halt) {
				cpu->halt = 0;
				cpu->pc++;
			}
			// i286_check_gate(cpu, off.w, seg.w);	// check [&execute] traps/gates, destination is cpu->ea.seg:cpu->ea.adr
			i286_push(cpu, cpu->f);
			if (!(fl & 1)) cpu->f &= ~I286_FI;
			i286_push(cpu, cpu->cs.idx);
			i286_push(cpu, cpu->pc);
			i286_push(cpu, cpu->errcod & 0xffff);
			cpu->cs = cpu->tmpdr;
			cpu->pc = off.w;
			//cpu->cs = i286_cash_seg(cpu, seg.w);
			//cpu->pc = off.w;
		}
		*/
	}
	cpu->t = 1;
}

void i286_interrupt(CPU* cpu, int vec) {
	if (cpu->regMSW & I286_FPE) {
		i286_int_prt(cpu, vec);
	} else {
		i286_int_real(cpu, vec);
	}
	cpu->errcod = -1;
}

// external INT (NMI have higher priority)
void i286_ext_int(CPU* cpu) {
	if ((cpu->intrq & I286_NMI) && !(cpu->inten & I286_BLK_NMI)) {
		cpu->inten |= I286_BLK_NMI;				// NMI is blocking until RETI, but next NMI will be remembered until then
		cpu->intrq &= ~I286_NMI;
		i286_interrupt(cpu, I286_INT_NMI);
	} else 	if (cpu->flgI) {
		cpu->intrq &= ~I286_INT;
		cpu->intvec = cpu->xack(cpu->xptr);
		i286_interrupt(cpu, cpu->intvec);
	}
}

int i286_exec(CPU* cpu) {
	cpu->t = 0;
	cpu->opTab = i80286_tab;
	cpu->seg.idx = -1;
	cpu->flgLOCK = 0;
	cpu->regREP = I286_REP_NONE;
	cpu->oldpc = cpu->regIP;

	int val = setjmp(cpu->jbuf);	// set THROW return point (val = err.code)
	if (!val) {
		if (cpu->intrq)
			i286_ext_int(cpu);
		if (cpu->t == 0) {
			cpu->t++;
			do {
				cpu->t++;
				cpu->com = cpu->x86fetch(cpu);
				cpu->op = &cpu->opTab[cpu->com & 0xff];
				while (cpu->op->flag & OF_GEN) {
					cpu->op = &cpu->op->tab[cpu->gen];
				}
				cpu->op->exec(cpu);
			} while (cpu->op->flag & OF_PREFIX);
		}
	} else {
		if (val < 0) val = I286_INT_DE;		// cuz INT_DE has code 0 (success for setjmp)
		cpu->regIP = cpu->oldpc;
		i286_interrupt(cpu, val);
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

extern const char* x87_get_mnem(CPU*, int);

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
	unsigned char mb = 0;
	char* ptr = strbuf;
	const char* p;
	char* dptr = mnembuf;
	do {
		com = cpu_mrd(cpu, adr);
		adr++;
		op = &tab[com];
		while (op->flag & OF_GEN) {
			op = &op->tab[cpu->gen];
		}
		if (tab == i80286_tab) {
			switch (com) {
				case 0x0f:
					if (cpu->gen >= 2) {
						tab = i286_0f_tab;
					}
					break;
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
	PAIR(w,h,l)rx;
	PAIR(w,h,l)seg;
	PAIR(w,h,l)disp;
	// check mod r/m
	if (op->flag & OF_MODRM) {
		mb = cpu_mrd(cpu, adr++);
		switch (mb & 0xc0) {
			case 0x80:
				disp.l = cpu_mrd(cpu, adr++);
				disp.h = cpu_mrd(cpu, adr++);
				break;
			case 0x40:
				disp.l = cpu_mrd(cpu, adr++);
				disp.h = (disp.l & 0x80) ? 0xff : 0x00;
				break;
			case 0x00:
				if ((mb & 7) == 6) {
					disp.l = cpu_mrd(cpu, adr++);
					disp.h = cpu_mrd(cpu, adr++);
				} else {
					disp.w = 0;
				}
				break;
		}
#if 1
		if ((op->flag & OF_COMEXT) && op->tab) {		// mod r/m b3-5 contains opcode extension, op->tab is tab[8] of ext opcodes
			op = &op->tab[(mb >> 3) & 7];
		}
#endif
	}
	// check ESC to x87
	if ((com & 0xf8) == 0xd8) {	// x87 opcodes
		p = x87_get_mnem(cpu, ((com << 8) | mb) & 0x7ff);
		if (p) {
			strcpy(strbuf, p);
		} else {
			strcpy(strbuf, op->mnem);
		}
	} else {			// x86 opcodes
		strcpy(strbuf, op->mnem);
	}

	do {
		if (*ptr == ':') {
			ptr++;
			switch(*ptr) {
				case '1':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					dptr += sprintf(dptr, "#%.2X", rx.l);
					break;
				case '2':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					rx.h = cpu_mrd(cpu, adr);
					adr++;
					dptr += sprintf(dptr, "#%.4X", rx.w);
					mn.oadr = rx.w + cpu->cs.base;
					break;
				case '3':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					rx.h = (rx.l & 0x80) ? 0xff : 0x00;
					rx.w += adr - cpu->cs.base;
					dptr += sprintf(dptr, "short #%.4X", rx.w);
					mn.oadr = rx.w + cpu->cs.base;
					break;
				case 'n':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					rx.h = cpu_mrd(cpu, adr);
					adr++;
					rx.w += (adr - cpu->cs.base);
					dptr += sprintf(dptr, "near #%.4X", rx.w);
					mn.oadr = rx.w + cpu->cs.base;
					break;
				case '4':
				case 'p':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					rx.h = cpu_mrd(cpu, adr);
					adr++;
					seg.l = cpu_mrd(cpu, adr);
					adr++;
					seg.h = cpu_mrd(cpu, adr);
					adr++;
					dptr += sprintf(dptr, "#%.4X::%.4X", seg.w, rx.w);
					// mn.oadr = rx.w;
					break;
				case '7':
					sprintf(dptr, "%i", mb & 7);
					break;
				case 'r':
					if (op->flag & OF_WORD) {	// word
						dptr += sprintf(dptr, "%s", str_regw[(mb >> 3) & 7]);
					} else {
						dptr += sprintf(dptr, "%s", str_regb[(mb >> 3) & 7]);
					}
					break;
				case 's':
					dptr += sprintf(dptr, "%s", str_regs[(mb >> 3) & 3]);
					break;
				case 'z':				// skip EA & disp
					break;
				case 'e':
					if ((mb & 0xc0) == 0xc0) {		// reg
						if (op->flag & OF_WORD) {
							dptr += sprintf(dptr, "%s", str_regw[mb & 7]);
						} else {
							dptr += sprintf(dptr, "%s", str_regb[mb & 7]);
						}
					} else {
						*(dptr++) = '[';
						if (rseg < 0)
							rseg = ((mb & 7)==6) ? ((mb & 0xc0) ? 2 : 3) : seg_ea[mb & 7];	// default segment if not overriden
						dptr += sprintf(dptr, "%s::", str_regs[rseg & 3]);
						if ((mb & 0xc7) == 0x06) {	// immw, [seg:disp]
							dptr += sprintf(dptr, "#%.4X", disp.w);
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
							dptr += sprintf(dptr, "%s", str_ea[mb & 7]);

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
				case 'A':
					dptr += sprintf(dptr, "%s", str_alu[(mb >> 3) & 7]);
					break;
				case 'R':
					dptr += sprintf(dptr, "%s", str_rot[(mb >> 3) & 7]);
					break;
				case 'X':
					if (!(mb & 0x30)) {		// f6 /0 and f6 /1 : test :e,:1
						strcat(ptr, ",:1");
					}
					dptr += sprintf(dptr, "%s", str_opX[(mb >> 3) & 7]);
					break;
				case 'Y':
					if (!(mb & 0x30)) {		// f7 /0 and f7 /1 : test :e,:2
						strcat(ptr, ",:2");
					}
					dptr += sprintf(dptr, "%s", str_opX[(mb >> 3) & 7]);
					break;
				case 'E':
					dptr += sprintf(dptr, "%s", str_opE[(mb >> 3) & 7]);
					break;
				case 'F':
					dptr += sprintf(dptr, "%s", str_opF[(mb >> 3) & 7]);
					break;
				case 'Q':
					dptr += sprintf(dptr, "%s", str_opQ[(mb >> 3) & 7]);
					break;
				case 'W':
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
	{I286_IP, "IP", REG_WORD | REG_RDMP, offsetof(CPU, regIP)},
	{I286_SP, "SP", REG_WORD | REG_RDMP, offsetof(CPU, regSP)},
	{I286_BP, "BP", REG_WORD | REG_RDMP, offsetof(CPU, regBP)},
	{I286_SI, "SI", REG_WORD | REG_RDMP, offsetof(CPU, regSI)},
	{I286_DI, "DI", REG_WORD | REG_RDMP, offsetof(CPU, regDI)},
	{I286_AX, "AX", REG_WORD | REG_RDMP, offsetof(CPU, regAX)},
	{I286_BX, "BX", REG_WORD | REG_RDMP, offsetof(CPU, regBX)},
	{I286_CX, "CX", REG_WORD | REG_RDMP, offsetof(CPU, regCX)},
	{I286_DX, "DX", REG_WORD | REG_RDMP, offsetof(CPU, regDX)},
	{I286_CS, "CS", REG_WORD|REG_SEG, offsetof(CPU, cs)},
	{I286_SS, "SS", REG_WORD|REG_SEG, offsetof(CPU, ss)},
	{I286_DS, "DS", REG_WORD|REG_SEG, offsetof(CPU, ds)},
	{I286_ES, "ES", REG_WORD|REG_SEG, offsetof(CPU, es)},
	{I286_MSW, "MSW", REG_RO|REG_32, offsetof(CPU, regMSW)},
	{I286_LDT, "LDT", REG_RO|REG_24, offsetof(CPU, ldtr)},
	{I286_GDT, "GDT", REG_RO|REG_24, offsetof(CPU, gdtr)},
	{I286_IDT, "IDT", REG_RO|REG_24, offsetof(CPU, idtr)},
	{I286_TSS, "TSS", REG_RO|REG_24, offsetof(CPU, tsdr)},
	{REG_NONE, "", 0, 0}
};

xRegDsc i8086RegTab[] = {
	{I286_IP, "IP", REG_WORD | REG_RDMP, offsetof(CPU, regIP)},
	{I286_SP, "SP", REG_WORD | REG_RDMP, offsetof(CPU, regSP)},
	{I286_BP, "BP", REG_WORD | REG_RDMP, offsetof(CPU, regBP)},
	{I286_SI, "SI", REG_WORD | REG_RDMP, offsetof(CPU, regSI)},
	{I286_DI, "DI", REG_WORD | REG_RDMP, offsetof(CPU, regDI)},
	{I286_AX, "AX", REG_WORD | REG_RDMP, offsetof(CPU, regAX)},
	{I286_BX, "BX", REG_WORD | REG_RDMP, offsetof(CPU, regBX)},
	{I286_CX, "CX", REG_WORD | REG_RDMP, offsetof(CPU, regCX)},
	{I286_DX, "DX", REG_WORD | REG_RDMP, offsetof(CPU, regDX)},
	{I286_CS, "CS", REG_WORD|REG_SEG, offsetof(CPU, cs)},
	{I286_SS, "SS", REG_WORD|REG_SEG, offsetof(CPU, ss)},
	{I286_DS, "DS", REG_WORD|REG_SEG, offsetof(CPU, ds)},
	{I286_ES, "ES", REG_WORD|REG_SEG, offsetof(CPU, es)},
	{REG_NONE, "", 0, 0}
};

xAsmScan i286_asm(int adr, const char* mnm, char* buf) {
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
			case I286_IP: val = cpu->regIP; bas = cpu->cs.base; break;
			case I286_SP: val = cpu->regSP; bas = cpu->ss.base; break;
			case I286_BP: val = cpu->regBP; bas = cpu->ss.base; break;
			case I286_SI: val = cpu->regSI; bas = cpu->ds.base; break;
			case I286_DI: val = cpu->regDI; bas = cpu->ds.base; break;
			case I286_AX: val = cpu->regAX; bas = cpu->ds.base; break;
			case I286_CX: val = cpu->regCX; bas = cpu->ds.base; break;
			case I286_DX: val = cpu->regDX; bas = cpu->ds.base; break;
			case I286_BX: val = cpu->regBX; bas = cpu->ds.base; break;
			case I286_CS: val = cpu->cs.idx; bas = cpu->cs.base; break;
			case I286_SS: val = cpu->ss.idx; bas = cpu->ss.base; break;
			case I286_DS: val = cpu->ds.idx; bas = cpu->ds.base; break;
			case I286_ES: val = cpu->es.idx; bas = cpu->es.base; break;
			case I286_MSW: val = cpu->regMSW; break;
			case I286_GDT: val = cpu->gdtr.base; break;
			case I286_LDT: val = cpu->ldtr.base; break;
			case I286_IDT: val = cpu->idtr.base; break;
			case I286_TSS: val = cpu->tsdr.base; break;
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
			case I286_IP: cpu->regIP = val; break;
			case I286_SP: cpu->regSP = val; break;
			case I286_BP: cpu->regBP = val; break;
			case I286_SI: cpu->regSI = val; break;
			case I286_DI: cpu->regDI = val; break;
			case I286_AX: cpu->regAX = val; break;
			case I286_CX: cpu->regCX = val; break;
			case I286_DX: cpu->regDX = val; break;
			case I286_BX: cpu->regBX = val; break;
			case I286_CS: cpu->cs = i286_cash_seg(cpu, val); break;
			case I286_SS: cpu->ss = i286_cash_seg(cpu, val); break;
			case I286_DS: cpu->ds = i286_cash_seg(cpu, val); break;
			case I286_ES: cpu->es = i286_cash_seg(cpu, val); break;
		}
		idx++;
	}
}
