#include <stdio.h>
#include <string.h>

#include "i80286.h"

extern opCode i80286_tab[256];

void i80286_reset(CPU* cpu) {
	cpu->flag = 0x0002;
	cpu->msw = 0xfff0;
	cpu->pc = 0xfff0;	// ip
	cpu->cs = 0xf000;
	cpu->ds = 0x0000;
	cpu->ss = 0x0000;
	cpu->es = 0x0000;
	cpu->mode = I286_MOD_REAL;
}

int i80286_exec(CPU* cpu) {
	cpu->t = 1;
	cpu->opTab = i80286_tab;
	cpu->seg = -1;
	cpu->lock = 0;
	cpu->rep = I286_REP_NONE;
	cpu->oldpc = cpu->pc;
	do {
		cpu->t++;
		cpu->com = cpu->mrd((cpu->cs << 4) + cpu->pc, 1, cpu->data);
		cpu->pc++;
		cpu->op = &cpu->opTab[cpu->com & 0xff];
		cpu->op->exec(cpu);
	} while (!(cpu->op->flag & OF_PREFIX));

	return cpu->t;
}

static char mnembuf[128];

const char* str_regb[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};
const char* str_regw[8] = {"ax","cx","dx","bx","sp","bp","si","di"};
const char* str_regs[4] = {"es","cs","ds","ss"};
const char* str_ea[8] = {"bx+si","bx+di","bp+si","bp+di","si","di","bp","bx"};
const char* str_alu[8] = {"add","or","adc","sbb","and","sub","xor","cmp"};
const char* str_rot[8] = {"rol","ror","rcl","rcr","sal","shr","*rot/6","sar"};
const char* str_opX[8] = {"test","*test","not","neg","mul","imul","div","idiv"};
const char* str_opE[8] = {"inc","dec","call","callf","jmp","jmpf","push","ff /7"};

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
		com = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
		adr++;
		op = &tab[com];
		if ((tab == i80286_tab) && (com == 0x0f)) {
			// tab = i80286_0f_tab;
		}
		mn.flag = op->flag;
		mn.len++;
	} while (op->flag & OF_PREFIX);
	const char* ptr = op->mnem;
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
					rx.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					dptr += sprintf(dptr, "0x%.2X", rx.l);
					break;
				case '2':
					rx.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					rx.h = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					dptr += sprintf(dptr, "0x%.4X", rx.w);
					break;
				case '3':
					rx.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					rx.w = adr + (signed char)rx.l;
					dptr += sprintf(dptr, "short %.4X", rx.w);
					break;
				case '4':
				case 'p':
					rx.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					rx.h = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					seg.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					seg.h = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
					adr++;
					dptr += sprintf(dptr, "%.4X:%.4X", seg.w, rx.w);
					break;
				case 'r': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					if (com & 1) {	// word
						dptr += sprintf(dptr, "%s", str_regb[(mb >> 3) & 7]);
					} else {
						dptr += sprintf(dptr, "%s", str_regw[(mb >> 3) & 7]);
					}
					break;
				case 's': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					dptr += sprintf(dptr, "%s", str_regs[(mod >> 3) & 3]);
					break;
				case 'e': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					if ((mod & 0xc0) == 0xc0) {		// reg
						if (com & 1) {
							dptr += sprintf(dptr, "%s", str_regb[mb & 7]);
						} else {
							dptr += sprintf(dptr, "%s", str_regw[mb & 7]);
						}
					} else {
						disp.w = 0;
						if ((mod & 0xc0) == 0x40) {
							disp.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
							disp.h = (disp.l & 0x80) ? 0xff : 0x00;
							adr++;
						} else if ((mod & 0xc0) == 0x80) {
							disp.l = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
							adr++;
							disp.h = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff;
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
				case 'A': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					dptr += sprintf(dptr, "%s", str_alu[(mb >> 3) & 7]);
					break;
				case 'R': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					dptr += sprintf(dptr, "%s", str_rot[(mb >> 3) & 7]);
					break;
				case 'X': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					dptr += sprintf(dptr, "%s", str_opX[(mb >> 3) & 7]);
					break;
				case 'E': if (!mod) {mod = 1; mb = cpu->mrd((cpu->cs << 4) + adr, 0, cpu->data) & 0xff; adr++;}
					dptr += sprintf(dptr, "%s", str_opE[(mb >> 3) & 7]);
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
