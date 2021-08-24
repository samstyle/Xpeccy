#include <stdio.h>
#include <string.h>

#include "i80286.h"

extern opCode i80286_tab[256];

extern xSegPtr i286_cash_seg(CPU*, unsigned short);

void i286_reset(CPU* cpu) {
	cpu->f = 0x0002;
	cpu->msw = 0xfff0;
	cpu->pc = 0xfff0;	// ip
	cpu->mode = I286_MOD_REAL;
	cpu->cs = i286_cash_seg(cpu, 0xf000);
	cpu->ss = i286_cash_seg(cpu, 0x0000);
	cpu->ds = i286_cash_seg(cpu, 0x0000);
	cpu->es = i286_cash_seg(cpu, 0x0000);
	cpu->idtr = i286_cash_seg(cpu, 0x0000);
//	cpu->cs = 0xf000;
//	cpu->ds = 0x0000;
//	cpu->ss = 0x0000;
//	cpu->es = 0x0000;
}

int i286_exec(CPU* cpu) {
	cpu->t = 1;
	cpu->opTab = i80286_tab;
	cpu->seg.idx = -1;
	cpu->lock = 0;
	cpu->rep = I286_REP_NONE;
	cpu->oldpc = cpu->pc;
	do {
		cpu->t++;
		cpu->com = i286_mrd(cpu, cpu->cs, cpu->pc); // cpu->mrd((cpu->cs << 4) + cpu->pc, 1, cpu->data);
		cpu->pc++;
		cpu->op = &cpu->opTab[cpu->com & 0xff];
		cpu->op->exec(cpu);
	} while (cpu->op->flag & OF_PREFIX);
	return cpu->t;
}

static char mnembuf[128];
static char strbuf[128];

const char* str_regb[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};
const char* str_regw[8] = {"ax","cx","dx","bx","sp","bp","si","di"};
const char* str_regs[4] = {"es","cs","ss","ds"};
const char* str_ea[8] = {"bx+si","bx+di","bp+si","bp+di","si","di","bp","bx"};
const char* str_alu[8] = {"add","or","adc","sbb","and","sub","xor","cmp"};
const char* str_rot[8] = {"rol","ror","rcl","rcr","sal","shr","*rot/6","sar"};
const char* str_opX[8] = {"test","*test","not","neg","mul","imul","div","idiv"};
const char* str_opE[8] = {"inc","dec","call","callf","jmp","jmpf","push","ff /7"};
const char* str_opQ[8] = {"sldt","str","lldt","ltr","verr","verw","0f00 /6","0f00 /7"};
const char* str_opW[8] = {"sgdt","sidt","lgdt","lidt","smsw","lmsw","0f01 /6","0f01 /7"};

xMnem i286_mnem(CPU* cpu, unsigned short adr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.cond = 0;
	mn.met = 0;
	mn.mem = 0;
	opCode* tab = i80286_tab;
	opCode* op;
	int sadr = adr;
	unsigned char com;
	do {
		com = i286_mrd(cpu, cpu->cs, adr);
		adr++;
		op = &tab[com];
		if ((tab == i80286_tab) && (com == 0x0f)) {
			// tab = i80286_0f_tab;
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
					rx.l = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					dptr += sprintf(dptr, "%.2X", rx.l);
					break;
				case '2':
					rx.l = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					rx.h = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					dptr += sprintf(dptr, "%.4X", rx.w);
					break;
				case '3':
					rx.l = i286_mrd(cpu, cpu->cs, adr);;
					adr++;
					rx.w = adr + (signed char)rx.l;
					dptr += sprintf(dptr, "short %.4X", rx.w);
					break;
				case 'n':
					rx.l = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					rx.h = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					rx.w += adr;
					dptr += sprintf(dptr, "near %.4X", rx.w);
					break;
				case '4':
				case 'p':
					rx.l = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					rx.h = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					seg.l = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					seg.h = i286_mrd(cpu, cpu->cs, adr);
					adr++;
					dptr += sprintf(dptr, "%.4X::%.4X", seg.w, rx.w);
					break;
				case 'r': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					if (op->flag & OF_WORD) {	// word
						dptr += sprintf(dptr, "%s", str_regw[(mb >> 3) & 7]);
					} else {
						dptr += sprintf(dptr, "%s", str_regb[(mb >> 3) & 7]);
					}
					break;
				case 's': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					dptr += sprintf(dptr, "%s", str_regs[(mb >> 3) & 3]);
					break;
				case 'e': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					if ((mb & 0xc0) == 0xc0) {		// reg
						if (op->flag & OF_WORD) {
							dptr += sprintf(dptr, "%s", str_regw[mb & 7]);
						} else {
							dptr += sprintf(dptr, "%s", str_regb[mb & 7]);
						}
					} else {
						disp.w = 0;
						if ((mb & 0xc0) == 0x40) {
							disp.l = i286_mrd(cpu, cpu->cs, adr);
							disp.h = (disp.l & 0x80) ? 0xff : 0x00;
							adr++;
						} else if ((mb & 0xc0) == 0x80) {
							disp.l = i286_mrd(cpu, cpu->cs, adr);
							adr++;
							disp.h = i286_mrd(cpu, cpu->cs, adr);
							adr++;
						}
						*(dptr++) = '[';
						dptr += sprintf(dptr, "%s", str_ea[mb & 7]);
						if (disp.w) {
							if (disp.w & 0x8000) {
								dptr += sprintf(dptr, "-%X", (-disp.w) & 0x7fff);
							} else {
								dptr += sprintf(dptr ,"+%X", disp.w);
							}
						}
						*(dptr++) = ']';
					}
					break;
				case 'A': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					dptr += sprintf(dptr, "%s", str_alu[(mb >> 3) & 7]);
					break;
				case 'R': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					dptr += sprintf(dptr, "%s", str_rot[(mb >> 3) & 7]);
					break;
				case 'X': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					if (!(mb & 0x30)) {	// test :e,:1 <- add ,:1 to command
						strcat(ptr, ",:1");
					}
					dptr += sprintf(dptr, "%s", str_opX[(mb >> 3) & 7]);
					break;
				case 'E': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					dptr += sprintf(dptr, "%s", str_opE[(mb >> 3) & 7]);
					break;
				case 'Q': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					dptr += sprintf(dptr, "%s", str_opQ[(mb >> 3) & 7]);
					break;
				case 'W': if (!mod) {mod = 1; mb = i286_mrd(cpu, cpu->cs, adr); adr++;}
					dptr += sprintf(dptr, "%s", str_opW[(mb >> 3) & 7]);
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

static xRegDsc i286RegTab[] = {
	{I286_CS, "CS", 0},
	{I286_IP, "IP", 0},
	{I286_SS, "SS", 0},
	{I286_SP, "SP", 0},
	{I286_BP, "BP", 0},
	{I286_DS, "DS", 0},
	{I286_ES, "ES", 0},
	{I286_SI, "SI", 0},
	{I286_DI, "DI", 0},
	{I286_AX, "AX", 0},
	{I286_CX, "CX", 0},
	{I286_DX, "DX", 0},
	{I286_BX, "BX", 0},
	{REG_NONE, "", 0}
};

xAsmScan i286_asm(const char* mnm, char* buf) {
	xAsmScan res;
	res.match = 0;
	return res;
}

char* i286_flags = "SZ-A-P-C";

void i286_get_regs(CPU* cpu, xRegBunch* bnch) {
	int idx = 0;
	while (i286RegTab[idx].id != REG_NONE) {
		bnch->regs[idx].id = i286RegTab[idx].id;
		bnch->regs[idx].name = i286RegTab[idx].name;
		bnch->regs[idx].byte = i286RegTab[idx].byte;
		switch (i286RegTab[idx].id) {
			case I286_CS: bnch->regs[idx].value = cpu->cs.idx; break;
			case I286_IP: bnch->regs[idx].value = cpu->pc; break;
			case I286_SS: bnch->regs[idx].value = cpu->ss.idx; break;
			case I286_SP: bnch->regs[idx].value = cpu->sp; break;
			case I286_BP: bnch->regs[idx].value = cpu->bp; break;
			case I286_DS: bnch->regs[idx].value = cpu->ds.idx; break;
			case I286_ES: bnch->regs[idx].value = cpu->es.idx; break;
			case I286_SI: bnch->regs[idx].value = cpu->si; break;
			case I286_DI: bnch->regs[idx].value = cpu->di; break;
			case I286_AX: bnch->regs[idx].value = cpu->ax; break;
			case I286_CX: bnch->regs[idx].value = cpu->cx; break;
			case I286_DX: bnch->regs[idx].value = cpu->dx; break;
			case I286_BX: bnch->regs[idx].value = cpu->bx; break;
		}
		idx++;
	}
	bnch->regs[idx].id = REG_NONE;
	bnch->flags = i286_flags;
}

void i286_set_regs(CPU* cpu, xRegBunch bnch) {
	int idx = 0;
	while (bnch.regs[idx].id != REG_NONE) {
		switch(bnch.regs[idx].id) {
			case I286_CS: cpu->cs = i286_cash_seg(cpu, bnch.regs[idx].value); break;
			case I286_IP: cpu->pc = bnch.regs[idx].value; break;
			case I286_SS: cpu->ss = i286_cash_seg(cpu, bnch.regs[idx].value); break;
			case I286_SP: cpu->sp = bnch.regs[idx].value; break;
			case I286_BP: cpu->bp = bnch.regs[idx].value; break;
			case I286_DS: cpu->ds = i286_cash_seg(cpu, bnch.regs[idx].value); break;
			case I286_ES: cpu->es = i286_cash_seg(cpu, bnch.regs[idx].value); break;
			case I286_SI: cpu->si = bnch.regs[idx].value; break;
			case I286_DI: cpu->di = bnch.regs[idx].value; break;
			case I286_AX: cpu->ax = bnch.regs[idx].value; break;
			case I286_CX: cpu->cx = bnch.regs[idx].value; break;
			case I286_DX: cpu->dx = bnch.regs[idx].value; break;
			case I286_BX: cpu->bx = bnch.regs[idx].value; break;
		}
		idx++;
	}
}
