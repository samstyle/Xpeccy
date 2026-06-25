// uPD70108 (V20)
// uPD70116 (v30)

#include "v30.h"
#include "v30regs.h"

#include <stdio.h>
#include <string.h>

void v30_push(CPU*, unsigned short);
int v30_mrdb(CPU*, unsigned short, unsigned short);
int v30_mrdw(CPU*, unsigned short, unsigned short);


void v30_reset(CPU* cpu) {
	cpu->regPC = 0;
	cpu->regSS = 0;
	cpu->regDS0 = 0;
	cpu->regDS1 = 0;
	cpu->regPS = 0xffff; cpu->cs.base = 0xffff0;
	cpu->flgMD = 1;
	cpu->flgDIR = 0;
	cpu->flgIE = 0;
	cpu->flgBRK = 0;
	cpu->flgSOVR = 0;
	cpu->flgBNMI = 0;
	cpu->flgBLKM = 1;	// MD can be changed only between BRKEM and RETEM, only by RETI/POP PSW
}

// TODO: INT switched native mode on (MD = 1)
void v30_int(CPU* cpu, int vec) {
	if (cpu->flgHALT) {		// exit standby mode
		cpu->flgHALT = 0;
		cpu->regPC++;
	}
	v30_push(cpu, v30_getflag(cpu));
	v30_push(cpu, cpu->regPS);
	v30_push(cpu, cpu->regPC);
	cpu->flgIE = 0;
	cpu->flgMD = 1;
	cpu->tmpi = (vec & 0xff) << 2;
	cpu->regPC = v30_mrdw(cpu, 0, cpu->tmpi);
	cpu->regPS = v30_mrdw(cpu, 0, cpu->tmpi+2);
}

// external INT (NMI have higher priority) = ask vector
void v30_ext_int(CPU* cpu) {
	if ((cpu->intrq & V30_NMI) && !cpu->flgBNMI) {
		cpu->flgBNMI = 1;				// NMI is blocking until RETI, but next NMI will be remembered until then
		cpu->intrq &= ~V30_NMI;
		v30_int(cpu, V30_INT_NMI);
	} else 	if (cpu->flgIE) {
		cpu->intrq &= ~V30_INT;
		cpu->intvec = cpu->xack(cpu->xptr);
		v30_int(cpu, cpu->intvec);
	}
}

extern opCode v30_tab[256];
extern opCode v30_0f_tab[256];
extern opCode v30emu_tab[256];	// 8080 emulation tab, +2 additional opcodes (ED-ED and ED-FD)

int v30_exec(CPU* cpu) {
	cpu->flgSOVR = 0;
	cpu->t = 0;
	cpu->opTab = cpu->flgMD ? v30_tab : v30emu_tab;
	cpu->flgLOCK = 0;
	cpu->regREP = V30_REP_NONE;
	cpu->oldpc = cpu->regPC;

	int val = setjmp(cpu->jbuf);	// set THROW return point (val = err.code)
	if (!val) {
		if (cpu->intrq)
			v30_ext_int(cpu);
		if (cpu->t == 0) {
			cpu->t++;
			do {
				cpu->t++;
				cpu->com = v30_mrdb(cpu, cpu->regPS, cpu->regPC++);
				cpu->op = &cpu->opTab[cpu->com & 0xff];
				cpu->op->exec(cpu);
			} while (cpu->op->flag & OF_PREFIX);
		}
	} else {
		if (val < 0) val = 0;
		cpu->regPC = cpu->oldpc;
		v30_int(cpu, val);
	}
	return cpu->t;
}

// asm
xAsmScan v30_asm(int adr, const char* inbuf, char* outbuf) {	// compile mnemonic (adr,src.text,result.buf)
	xAsmScan scn;
	scn.match = 0;
	return scn;
}

// disasm

static char mnembuf[128];
static char strbuf[128];

static const char* str_regb[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};
static const char* str_regw[8] = {"aw","cw","dw","bw","sp","bp","ix","iy"};
static const char* str_regs[4] = {"ds1","ps","ss","ds0"};
static const char* str_ea[8] = {"bw+ix","bw+iy","bp+ix","bp+iy","ix","iy","bp","bw"};
static int seg_ea[8] = {3,3,2,2,3,3,2,3};
static const char* str_alu[8] = {"add","or","addc","subc","and","sub","xor","cmp"};
static const char* str_rot[8] = {"rol","ror","rolc","rorc","shl","shr","shla","shra"};
static const char* str_opX[8] = {"test","*test","not","neg","mulu","mul","divu","div"};
static const char* str_opE[8] = {"inc","dec","call","callf","jmp","jmpf","push","fe /7"};
static const char* str_opF[8] = {"incw","decw","call","callf","jmp","jmpf","push","ff /7"};
//static const char* str_opQ[8] = {"sldt","str","lldt","ltr","verr","verw","0f00 /6","0f00 /7"};
//static const char* str_opW[8] = {"sgdt","sidt","lgdt","lidt","smsw","0f01 /5","lmsw","0f01 /7"};

xMnem v30_mnem(CPU* cpu, int sadr, cbdmr mrd, void* data) {
	xMnem mn;
	mn.cond = 0;
	mn.met = 0;
	mn.mem = 0;
	mn.oadr = -1;
	int rep = V30_REP_NONE;
	int rseg = -1;
	opCode* tab = cpu->flgMD ? v30_tab : v30emu_tab;
	opCode* op;
	int adr = sadr;
	unsigned char com;
	unsigned char mb = 0;
	char* ptr = strbuf;
	char* dptr = mnembuf;
	do {
		com = cpu_mrd(cpu, adr);
		adr++;
		op = &tab[com];
		while (op->flag & OF_GEN) {
			op = &op->tab[cpu->gen];
		}
		if (tab == v30_tab) {
			switch (com) {
				case 0x0f:
					tab = v30_0f_tab;
					break;
				case 0x26: rseg = 0; break;		// es
				case 0x2e: rseg = 1; break;		// cs
				case 0x36: rseg = 2; break;		// ss
				case 0x3e: rseg = 3; break;		// ds
				case 0xf2: rep = V30_REPNZ; break;
				case 0xf3: rep = V30_REPZ; break;
			}
		}
		mn.flag = op->flag;
		mn.len++;
	} while (op->flag & OF_PREFIX);
	xreg16 rx;
	xreg16 seg;
	xreg16 disp;
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
#if 0
		if ((op->flag & OF_COMEXT) && op->tab) {		// mod r/m b3-5 contains opcode extension, op->tab is tab[8] of ext opcodes
			op = &op->tab[(mb >> 3) & 7];
		}
#endif
	}
	strcpy(strbuf, op->mnem);

	int base = cpu->regPS << 4;
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
					mn.oadr = rx.w + base;
					break;
				case '3':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					rx.h = (rx.l & 0x80) ? 0xff : 0x00;
					rx.w += adr - base;
					dptr += sprintf(dptr, "short #%.4X", rx.w);
					mn.oadr = rx.w + base;
					break;
				case 'n':
					rx.l = cpu_mrd(cpu, adr);
					adr++;
					rx.h = cpu_mrd(cpu, adr);
					adr++;
					rx.w += (adr - base);
					dptr += sprintf(dptr, "near #%.4X", rx.w);
					mn.oadr = rx.w + base;
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

				case 'd':
					if (op->flag & OF_WORD) {
						dptr += sprintf(dptr, "%s", str_regw[mb & 7]);
					} else {
						dptr += sprintf(dptr, "%s", str_regb[mb & 7]);
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
						if (rep == V30_REPZ) {
							dptr += sprintf(dptr, "REPZ ");
						} else if (rep == V30_REPNZ) {
							dptr += sprintf(dptr, "REPNZ ");
						}
					} else if (rep != V30_REP_NONE) {
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
//				case 'Q':
//					dptr += sprintf(dptr, "%s", str_opQ[(mb >> 3) & 7]);
//					break;
//				case 'W':
//					dptr += sprintf(dptr, "%s", str_opW[(mb >> 3) & 7]);
//					break;
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

// flags

int v30_getflag(CPU* cpu) {
	int f = 0x72 | (cpu->flgCY) | (cpu->flgP << 2) | (cpu->flgAC << 4) | (cpu->flgZ << 6) | (cpu->flgS << 7);
	f |= (cpu->flgBRK << 8) | (cpu->flgIE << 9) | (cpu->flgDIR << 10) | (cpu->flgV << 11) | (cpu->flgMD << 15);
	return f;
}

void v30_setflag(CPU* cpu, int val) {
	cpu->flgCY = (val & 0x01);
	cpu->flgP = !!(val & 0x04);
	cpu->flgAC = !!(val & 0x10);
	cpu->flgZ = !!(val & 0x40);
	cpu->flgS = !!(val & 0x80);
	cpu->flgBRK = !!(val & 0x100);
	cpu->flgIE = !!(val & 0x200);
	cpu->flgDIR = !!(val & 0x400);
	cpu->flgV = !!(val & 0x800);
	if (!cpu->flgBLKM) {
		cpu->flgMD = !!(val & 0x8000);
	}
}

// registers

void v30_set_pc(CPU* cpu, int v) {cpu->regPC = v;}
void v30_set_aw(CPU* cpu, int v) {cpu->regAW = v;}
void v30_set_bw(CPU* cpu, int v) {cpu->regBW = v;}
void v30_set_cw(CPU* cpu, int v) {cpu->regCW = v;}
void v30_set_dw(CPU* cpu, int v) {cpu->regDW = v;}
void v30_set_ix(CPU* cpu, int v) {cpu->regIX = v;}
void v30_set_iy(CPU* cpu, int v) {cpu->regIY = v;}
void v30_set_sp(CPU* cpu, int v) {cpu->regSP = v;}
void v30_set_bp(CPU* cpu, int v) {cpu->regBP = v;}
void v30_set_ps(CPU* cpu, int v) {cpu->regPS = v; cpu->cs.base = (cpu->regPS << 4);}
void v30_set_ss(CPU* cpu, int v) {cpu->regSS = v;}
void v30_set_ds0(CPU* cpu, int v) {cpu->regDS0 = v;}
void v30_set_ds1(CPU* cpu, int v) {cpu->regDS1 = v;}

int v30_get_pc(CPU* cpu) {return cpu->regPC;}
int v30_get_aw(CPU* cpu) {return cpu->regAW;}
int v30_get_bw(CPU* cpu) {return cpu->regBW;}
int v30_get_cw(CPU* cpu) {return cpu->regCW;}
int v30_get_dw(CPU* cpu) {return cpu->regDW;}
int v30_get_ix(CPU* cpu) {return cpu->regIX;}
int v30_get_iy(CPU* cpu) {return cpu->regIY;}
int v30_get_sp(CPU* cpu) {return cpu->regSP;}
int v30_get_bp(CPU* cpu) {return cpu->regBP;}
int v30_get_ps(CPU* cpu) {return cpu->regPS;}
int v30_get_ss(CPU* cpu) {return cpu->regSS;}
int v30_get_ds0(CPU* cpu) {return cpu->regDS0;}
int v30_get_ds1(CPU* cpu) {return cpu->regDS1;}

xRegDsc v30_regtab[] = {
	{V30_REG_PC, "PC", REG_WORD, REG_RDMP | REG_PC, v30_get_pc, v30_set_pc},
	{V30_REG_AW, "AW", REG_WORD, REG_RDMP, v30_get_aw, v30_set_aw},
	{V30_REG_BW, "BW", REG_WORD, REG_RDMP, v30_get_bw, v30_set_bw},
	{V30_REG_CW, "CW", REG_WORD, REG_RDMP, v30_get_cw, v30_set_cw},
	{V30_REG_DW, "DW", REG_WORD, REG_RDMP, v30_get_dw, v30_set_dw},
	{V30_REG_IX, "IX", REG_WORD, REG_RDMP, v30_get_ix, v30_set_ix},
	{V30_REG_IY, "IY", REG_WORD, REG_RDMP, v30_get_iy, v30_set_iy},
	{V30_REG_SP, "SP", REG_WORD, REG_RDMP | REG_SP, v30_get_sp, v30_set_sp},
	{V30_REG_BP, "BP", REG_WORD, REG_RDMP, v30_get_bp, v30_set_bp},
	{V30_SEG_PS, "PS", REG_WORD, REG_SEG, v30_get_ps, v30_set_ps},
	{V30_SEG_SS, "SS", REG_WORD, REG_SEG, v30_get_ss, v30_set_ss},
	{V30_SEG_DS0, "DS0", REG_WORD, REG_SEG, v30_get_ds0, v30_set_ds0},
	{V30_SEG_DS1, "DS1", REG_WORD, REG_SEG, v30_get_ds1, v30_set_ds1},
	{REG_EMPTY, "F", REG_WORD, REG_FLG, v30_getflag, v30_setflag},
	{REG_EOT, "M---VDIBSZ-A-P-C", 0, 0, NULL, NULL}
};
