#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"

extern opCode cbTab[256];
extern opCode edTab[256];
extern opCode ddTab[256];
extern opCode fdTab[256];

void z80_iowr(CPU* cpu, int adr, int data) {
	cpu->iwr(adr, data, cpu->xptr);
	cpu->t += 4;
}

int z80_iord(CPU* cpu, int adr) {
	int v = cpu->ird(adr, cpu->xptr);
	cpu->t += 4;
	return v & 0xff;
}

int z80_mrd(CPU*, int);
void z80_mwr(CPU*, int, int);

// push/pop

unsigned short z80_pop(CPU* cpu) {
	xpair p;
	p.l = z80_mrd(cpu, cpu->regSP++);
	p.h = z80_mrd(cpu, cpu->regSP++);
	return p.w;
}

void z80_push(CPU* cpu, unsigned short w) {
	xpair p;
	p.w = w;
	z80_mwr(cpu, --cpu->regSP, p.h);
	z80_mwr(cpu, --cpu->regSP, p.l);
}

void z80_call(CPU* cpu, unsigned short a) {
	cpu->regWZ = a;
	z80_push(cpu, cpu->regPC);
	cpu->regPC = cpu->regWZ;
}

void z80_ret(CPU* cpu) {
	cpu->regPC = z80_pop(cpu);
	cpu->regWZ = cpu->regPC;
}

// opcodes
// 00	nop		4
void npr00(CPU* cpu) {}

// 01	ld bc,nn	4 3rd 3rd
void npr01(CPU* cpu) {
	cpu->regC = z80_mrd(cpu, cpu->regPC++);
	cpu->regB = z80_mrd(cpu, cpu->regPC++);
}

// 02	ld (regBC),regA	4 3wr		regWZ = (regA << 8) | ((regBC + 1) & 0xff)
void npr02(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_mwr(cpu, cpu->regWZ++, cpu->regA);
	cpu->regWZh = cpu->regA;
}

// 03	inc regBC		6
void npr03(CPU* cpu) {
	cpu->regBC++;
}

// 04	inc regB		4
void npr04(CPU* cpu) {
	cpu->regB = z80_inc8(cpu, cpu->regB); //	INC(cpu->b);
}

// 05	dec regB		4
void npr05(CPU* cpu) {
	cpu->regB = z80_dec8(cpu, cpu->regB); //	DEC(cpu->b);
}

// 06	ld regB,n		4 3rd
void npr06(CPU* cpu) {
	cpu->regB = z80_mrd(cpu, cpu->regPC++);
}

// 07	rlca		4
void npr07(CPU* cpu) {
	cpu->regA = (cpu->regA << 1) | (cpu->regA >> 7);
	//cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3 | Z80_FC));
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = (cpu->regA & 1);
}

// 08	ex af,af'	4
void npr08(CPU* cpu) {
	cpu->ltw = cpu->regA; cpu->regA = cpu->a_; cpu->a_ = cpu->ltw;
	cpu->tmpi = cpu->f; cpu->f = cpu->f_; cpu->f_ = cpu->tmpi;
}

// 09	add regHL,regBC	11		mptr = regHL+1 before adding
void npr09(CPU* cpu) {
	cpu->regHL = z80_add16(cpu, cpu->regHL, cpu->regBC); //ADD16(cpu->hl, cpu->bc);
}

// 0A	ld regA,(regBC)	4 3rd		regWZ = regBC+1
void npr0A(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regA = z80_mrd(cpu, cpu->regWZ++);
}

// 0B	dec regBC		6
void npr0B(CPU* cpu) {
	cpu->regBC--;
}

// 0C	inc regC		4
void npr0C(CPU* cpu) {
	cpu->regC = z80_inc8(cpu, cpu->regC); //	INC(cpu->c);
}

// 0D	dec regC		4
void npr0D(CPU* cpu) {
	cpu->regC = z80_dec8(cpu, cpu->regC);	//	DEC(cpu->c);
}

// 0E	ld regC,n		4 3rd
void npr0E(CPU* cpu) {
	cpu->regC = z80_mrd(cpu, cpu->regPC++);
}

// 0F	rrca		4
void npr0F(CPU* cpu) {
	// cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & Z80_FC);
	cpu->fz.c = (cpu->regA & 1);
	cpu->regA = (cpu->regA >> 1) | (cpu->regA << 7);
	// cpu->f |= (cpu->a & (Z80_F5 | Z80_F3));
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 0;
}

// 10	djnz		5 3rd [5jr]
void npr10(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	cpu->regB--;
	if (cpu->regB) {
		cpu->regPC += (signed char)cpu->tmp;
		cpu->regWZ = cpu->regPC;
		cpu->t += 5;
	}
}

// 11	ld de,nn	4 3rd 3rd
void npr11(CPU* cpu) {
	cpu->regE = z80_mrd(cpu, cpu->regPC++);
	cpu->regD = z80_mrd(cpu, cpu->regPC++);
}

// 12	ld (regDE),regA	4 3wr		regWZ = (regA << 8) | ((regDE + 1) & 0xff)
void npr12(CPU* cpu) {
	cpu->regWZ = cpu->regDE;
	z80_mwr(cpu, cpu->regWZ++, cpu->regA);
	cpu->regWZh = cpu->regA;
}

// 13	inc regDE		6
void npr13(CPU* cpu) {
	cpu->regDE++;
}

// 14	inc regD		4
void npr14(CPU* cpu) {
	cpu->regD = z80_inc8(cpu, cpu->regD);	// INC(cpu->d);
}

// 15	dec regD		4
void npr15(CPU* cpu) {
	cpu->regD = z80_dec8(cpu, cpu->regD); // DEC(cpu->d);
}

// 16	ld regD,n		4 3rd
void npr16(CPU* cpu) {
	cpu->regD = z80_mrd(cpu, cpu->regPC++);
}

// 17	rla		4
void npr17(CPU* cpu) {
	cpu->tmp = cpu->regA;
	cpu->regA = (cpu->regA << 1) | cpu->fz.c;
	// cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3)) | (cpu->tmp >> 7);
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = !!(cpu->tmp & 0x80);
}

// 18	jr e		4 3rd 5jr
void npr18(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	cpu->regPC += (signed char)cpu->tmp;
	cpu->regWZ = cpu->regPC;
	cpu->t += 5;
}

// 19	add regHL,regDE	11	mptr = regHL+1 before adding
void npr19(CPU* cpu) {
	cpu->regHL = z80_add16(cpu, cpu->regHL, cpu->regDE); //ADD16(cpu->hl,cpu->de);
}

// 1A	ld regA,(regDE)	4 3rd	regWZ = regDE + 1
void npr1A(CPU* cpu) {
	cpu->regA = z80_mrd(cpu, cpu->regDE);
	cpu->regWZ = cpu->regDE + 1;
}

// 1B	dec regDE		6
void npr1B(CPU* cpu) {
	cpu->regDE--;
}

// 1C	inc regE		4
void npr1C(CPU* cpu) {
	cpu->regE = z80_inc8(cpu, cpu->regE); // INC(cpu->e);
}

// 1D	dec regE		4
void npr1D(CPU* cpu) {
	cpu->regE = z80_dec8(cpu, cpu->regE); // DEC(cpu->e);
}

// 1E	ld regE,n		4 3rd
void npr1E(CPU* cpu) {
	cpu->regE = z80_mrd(cpu, cpu->regPC++);
}

// 1F	rra		4
void npr1F(CPU* cpu) {
	cpu->tmp = cpu->regA;
	cpu->regA = (cpu->regA >> 1) | (cpu->f << 7);
	// cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3)) | (cpu->tmp & Z80_FC);
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = cpu->tmp & 1;
}

// 20	jr nz,e		4 3rd [5jr]
void npr20(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.z) {
		cpu->regPC += (signed char)cpu->tmp;
		cpu->regWZ = cpu->regPC;
		cpu->t += 5;
	}
}

// 21	ld hl,nn	4 3rd 3rd
void npr21(CPU* cpu) {
	cpu->regL = z80_mrd(cpu, cpu->regPC++);
	cpu->regH = z80_mrd(cpu, cpu->regPC++);
}

// 22	ld (nn),hl	4 3rd 3rd 3wr 3wr	regWZ = nn+1
void npr22(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regWZ++, cpu->regL);
	z80_mwr(cpu, cpu->regWZ, cpu->regH);
}

// 23	inc regHL		6
void npr23(CPU* cpu) {
	cpu->regHL++;
}

// 24	inc regH		4
void npr24(CPU* cpu) {
	cpu->regH = z80_inc8(cpu, cpu->regH);	// INC(cpu->h);
}

// 25	dec regH		4
void npr25(CPU* cpu) {
	cpu->regH = z80_dec8(cpu, cpu->regH); // DEC(cpu->h);
}

// 26	ld regH,n		4 3rd
void npr26(CPU* cpu) {
	cpu->regH = z80_mrd(cpu, cpu->regPC++);
}

// 27	daa		4
void npr27(CPU* cpu) {
	const unsigned char* tdaa = daaTab + 2 * (cpu->regA + 0x100 * ((cpu->f & 3) + ((cpu->f >> 2) & 4)));
	cpu->f = *tdaa;
	cpu->regA = *(tdaa + 1);
}

// 28	jr z,e		4 3rd [5jr]
void npr28(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.z) {
		cpu->regPC += (signed char)cpu->tmp;
		cpu->regWZ = cpu->regPC;
		cpu->t += 5;
	}
}

// 29	add regHL,regHL	11
void npr29(CPU* cpu) {
	cpu->regHL = z80_add16(cpu, cpu->regHL, cpu->regHL); //ADD16(cpu->hl,cpu->hl);
}

// 2A	ld hl,(nn)	4 3rd 3rd 3rd 3rd	regWZ = nn+1
void npr2A(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	cpu->regL = z80_mrd(cpu, cpu->regWZ++);
	cpu->regH = z80_mrd(cpu, cpu->regWZ);
}

// 2B	dec regHL		6
void npr2B(CPU* cpu) {
	cpu->regHL--;
}

// 2C	inc regL		4
void npr2C(CPU* cpu) {
	cpu->regL = z80_inc8(cpu, cpu->regL); // INC(cpu->l);
}

// 2D	dec regL		4
void npr2D(CPU* cpu) {
	cpu->regL = z80_dec8(cpu, cpu->regL); // DEC(cpu->l);
}

// 2E	ld regL,n		4 3rd
void npr2E(CPU* cpu) {
	cpu->regL = z80_mrd(cpu, cpu->regPC++);
}

// 2F	cpl		4
void npr2F(CPU* cpu) {
	cpu->regA ^= 0xff;
//	cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP | Z80_FC)) | (cpu->a & (Z80_F5 | Z80_F3)) | Z80_FH | Z80_FN;
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 1;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 1;
}

// 30	jr nc,e		4 3rd [5jr]
void npr30(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.c) {
		cpu->regPC += (signed char)cpu->tmp;
		cpu->regWZ = cpu->regPC;
		cpu->t += 5;
	}
}

// 31	ld sp,nn	4 3rd 3rd
void npr31(CPU* cpu) {
	cpu->regSPl = z80_mrd(cpu, cpu->regPC++);
	cpu->regSPh = z80_mrd(cpu, cpu->regPC++);
}

// 32	ld (nn),regA	4 3rd 3rd 3wr		regWZ = (regA << 8) | ((nn + 1) & 0xff)
void npr32(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regWZ++, cpu->regA);
	cpu->regWZh = cpu->regA;
}

// 33	inc regSP		6
void npr33(CPU* cpu) {
	cpu->regSP++;
}

// 34	inc (regHL)	4 3rd 4wr
void npr34(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regHL);
	cpu->tmpb = z80_inc8(cpu, cpu->tmpb); // INC(cpu->tmpb);
	cpu->t++;
	z80_mwr(cpu, cpu->regHL, cpu->tmpb);
}

// 35	dec (regHL)	4 3rd 4wr
void npr35(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regHL);
	cpu->tmpb = z80_dec8(cpu, cpu->tmpb); // DEC(cpu->tmpb);
	cpu->t++;
	z80_mwr(cpu, cpu->regHL, cpu->tmpb);
}

// 36	ld (regHL),n	4 3rd 3wr
void npr36(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regHL, cpu->tmp);
}

// 37	scf		4
void npr37(CPU* cpu) {
	//cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3)) | Z80_FC;
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = 1;
}

// 38	jr c,e		4 3rd [5jr]
void npr38(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.c) {
		cpu->regPC += (signed char)cpu->tmp;
		cpu->regWZ = cpu->regPC;
		cpu->t += 5;
	}
}

// 39	add regHL,regSP	11
void npr39(CPU* cpu) {
	cpu->regHL = z80_add16(cpu, cpu->regHL, cpu->regSP); //ADD16(cpu->hl,cpu->sp);
}

// 3A	ld regA,(nn)	4 3rd 3rd 3rd	regWZ = nn+1
void npr3A(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	cpu->regA = z80_mrd(cpu, cpu->regWZ++);
}

// 3B	dec regSP		6
void npr3B(CPU* cpu) {
	cpu->regSP--;
}

// 3C	inc regA		4
void npr3C(CPU* cpu) {
	cpu->regA = z80_inc8(cpu, cpu->regA); // INC(cpu->a);
}

// 3D	dec regA		4
void npr3D(CPU* cpu) {
	cpu->regA = z80_dec8(cpu, cpu->regA); // DEC(cpu->a);
}

// 3E	ld regA,n		4 3rd
void npr3E(CPU* cpu) {
	cpu->regA = z80_mrd(cpu, cpu->regPC++);
}

// 3F	ccf		4
void npr3F(CPU* cpu) {
	//cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | ((cpu->f & Z80_FC ) ? Z80_FH : Z80_FC) | (cpu->a & (Z80_F5 | Z80_F3));
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = cpu->fz.c;		// old C in H
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c ^= 1;
}

// 40..47	ld b,r		4 [3rd]
void npr40(CPU* cpu) {}
void npr41(CPU* cpu) {cpu->regB = cpu->regC;}
void npr42(CPU* cpu) {cpu->regB = cpu->regD;}
void npr43(CPU* cpu) {cpu->regB = cpu->regE;}
void npr44(CPU* cpu) {cpu->regB = cpu->regH;}
void npr45(CPU* cpu) {cpu->regB = cpu->regL;}
void npr46(CPU* cpu) {cpu->regB = z80_mrd(cpu, cpu->regHL);}
void npr47(CPU* cpu) {cpu->regB = cpu->regA;}
// 48..4f	ld regC,r		4 [3rd]
void npr48(CPU* cpu) {cpu->regC = cpu->regB;}
void npr49(CPU* cpu) {}
void npr4A(CPU* cpu) {cpu->regC = cpu->regD;}
void npr4B(CPU* cpu) {cpu->regC = cpu->regE;}
void npr4C(CPU* cpu) {cpu->regC = cpu->regH;}
void npr4D(CPU* cpu) {cpu->regC = cpu->regL;}
void npr4E(CPU* cpu) {cpu->regC = z80_mrd(cpu, cpu->regHL);}
void npr4F(CPU* cpu) {cpu->regC = cpu->regA;}
// 50..57	ld regD,r		4 [3rd]
void npr50(CPU* cpu) {cpu->regD = cpu->regB;}
void npr51(CPU* cpu) {cpu->regD = cpu->regC;}
void npr52(CPU* cpu) {}
void npr53(CPU* cpu) {cpu->regD = cpu->regE;}
void npr54(CPU* cpu) {cpu->regD = cpu->regH;}
void npr55(CPU* cpu) {cpu->regD = cpu->regL;}
void npr56(CPU* cpu) {cpu->regD = z80_mrd(cpu, cpu->regHL);}
void npr57(CPU* cpu) {cpu->regD = cpu->regA;}
// 58..5f	ld regE,r		4 [3rd]
void npr58(CPU* cpu) {cpu->regE = cpu->regB;}
void npr59(CPU* cpu) {cpu->regE = cpu->regC;}
void npr5A(CPU* cpu) {cpu->regE = cpu->regD;}
void npr5B(CPU* cpu) {}
void npr5C(CPU* cpu) {cpu->regE = cpu->regH;}
void npr5D(CPU* cpu) {cpu->regE = cpu->regL;}
void npr5E(CPU* cpu) {cpu->regE = z80_mrd(cpu, cpu->regHL);}
void npr5F(CPU* cpu) {cpu->regE = cpu->regA;}
// 60..67	ld regH,r		4 [3rd]
void npr60(CPU* cpu) {cpu->regH = cpu->regB;}
void npr61(CPU* cpu) {cpu->regH = cpu->regC;}
void npr62(CPU* cpu) {cpu->regH = cpu->regD;}
void npr63(CPU* cpu) {cpu->regH = cpu->regE;}
void npr64(CPU* cpu) {}
void npr65(CPU* cpu) {cpu->regH = cpu->regL;}
void npr66(CPU* cpu) {cpu->regH = z80_mrd(cpu, cpu->regHL);}
void npr67(CPU* cpu) {cpu->regH = cpu->regA;}
// 68..6f	ld regL,r		4 [3rd]
void npr68(CPU* cpu) {cpu->regL = cpu->regB;}
void npr69(CPU* cpu) {cpu->regL = cpu->regC;}
void npr6A(CPU* cpu) {cpu->regL = cpu->regD;}
void npr6B(CPU* cpu) {cpu->regL = cpu->regE;}
void npr6C(CPU* cpu) {cpu->regL = cpu->regH;}
void npr6D(CPU* cpu) {}
void npr6E(CPU* cpu) {cpu->regL = z80_mrd(cpu, cpu->regHL);}
void npr6F(CPU* cpu) {cpu->regL = cpu->regA;}
// 70..77	ld (regHL),r	4 3wr
void npr70(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regB);}
void npr71(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regC);}
void npr72(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regD);}
void npr73(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regE);}
void npr74(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regH);}
void npr75(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regL);}
void npr76(CPU* cpu) {cpu->halt = 1; cpu->regPC--;}
void npr77(CPU* cpu) {z80_mwr(cpu, cpu->regHL, cpu->regA);}
// 78..7f	ld regA,r		4 [3rd]
void npr78(CPU* cpu) {cpu->regA = cpu->regB;}
void npr79(CPU* cpu) {cpu->regA = cpu->regC;}
void npr7A(CPU* cpu) {cpu->regA = cpu->regD;}
void npr7B(CPU* cpu) {cpu->regA = cpu->regE;}
void npr7C(CPU* cpu) {cpu->regA = cpu->regH;}
void npr7D(CPU* cpu) {cpu->regA = cpu->regL;}
void npr7E(CPU* cpu) {cpu->regA = z80_mrd(cpu, cpu->regHL);}
void npr7F(CPU* cpu) {}
// 80..87	add regA,r		4 [3rd]
void npr80(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regB, 0);} //ADD(cpu->b);}
void npr81(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regC, 0);} //ADD(cpu->c);}
void npr82(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regD, 0);} //ADD(cpu->d);}
void npr83(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regE, 0);} //ADD(cpu->e);}
void npr84(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regH, 0);} //ADD(cpu->h);}
void npr85(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regL, 0);} //ADD(cpu->l);}
void npr86(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->regHL); cpu->regA = z80_add8(cpu, cpu->tmpb, 0);} //ADD(cpu->tmpb);}
void npr87(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regA, 0);} //ADD(cpu->regA);}
// 88..8F	adc regA,r		4 [3rd]
void npr88(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regB, cpu->fz.c);} //ADC(cpu->b);}
void npr89(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regC, cpu->fz.c);} //ADC(cpu->c);}
void npr8A(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regD, cpu->fz.c);} //ADC(cpu->d);}
void npr8B(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regE, cpu->fz.c);} //ADC(cpu->e);}
void npr8C(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regH, cpu->fz.c);} //ADC(cpu->h);}
void npr8D(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regL, cpu->fz.c);} //ADC(cpu->l);}
void npr8E(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->regHL); cpu->regA = z80_add8(cpu, cpu->tmpb, cpu->fz.c);} //ADC(cpu->tmpb);}
void npr8F(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regA, cpu->fz.c);} //ADC(cpu->regA);}
// 90..97	sub r		4 [3rd]
void npr90(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regB, 0);} //SUB(cpu->b);}
void npr91(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regC, 0);} //SUB(cpu->c);}
void npr92(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regD, 0);} //SUB(cpu->d);}
void npr93(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regE, 0);} //SUB(cpu->e);}
void npr94(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regH, 0);} //SUB(cpu->h);}
void npr95(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regL, 0);} //SUB(cpu->l);}
void npr96(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->regHL); cpu->regA = z80_sub8(cpu, cpu->tmpb, 0);} //SUB(cpu->tmpb);}
void npr97(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regA, 0);} //SUB(cpu->regA);}
// 98..9F	sbc regA,r		4 [3rd]
void npr98(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regB, cpu->fz.c);} //SBC(cpu->b);}
void npr99(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regC, cpu->fz.c);} //SBC(cpu->c);}
void npr9A(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regD, cpu->fz.c);} //SBC(cpu->d);}
void npr9B(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regE, cpu->fz.c);} //SBC(cpu->e);}
void npr9C(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regH, cpu->fz.c);} //SBC(cpu->h);}
void npr9D(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regL, cpu->fz.c);} //SBC(cpu->l);}
void npr9E(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->regHL); cpu->regA = z80_sub8(cpu, cpu->tmpb, cpu->fz.c);} //SBC(cpu->tmpb);}
void npr9F(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regA, cpu->fz.c);} //SBC(cpu->a);}
// a0..a7	and r		4 [3rd]
void nprA0(CPU* cpu) {z80_and8(cpu, cpu->regB);}
void nprA1(CPU* cpu) {z80_and8(cpu, cpu->regC);}
void nprA2(CPU* cpu) {z80_and8(cpu, cpu->regD);}
void nprA3(CPU* cpu) {z80_and8(cpu, cpu->regE);}
void nprA4(CPU* cpu) {z80_and8(cpu, cpu->regH);}
void nprA5(CPU* cpu) {z80_and8(cpu, cpu->regL);}
void nprA6(CPU* cpu) {cpu->tmp = z80_mrd(cpu, cpu->regHL); z80_and8(cpu, cpu->tmp);}
void nprA7(CPU* cpu) {z80_and8(cpu, cpu->regA);}
// a8..af	xor r		4 [3rd]
void nprA8(CPU* cpu) {z80_xor8(cpu, cpu->regB);}
void nprA9(CPU* cpu) {z80_xor8(cpu, cpu->regC);}
void nprAA(CPU* cpu) {z80_xor8(cpu, cpu->regD);}
void nprAB(CPU* cpu) {z80_xor8(cpu, cpu->regE);}
void nprAC(CPU* cpu) {z80_xor8(cpu, cpu->regH);}
void nprAD(CPU* cpu) {z80_xor8(cpu, cpu->regL);}
void nprAE(CPU* cpu) {cpu->tmp = z80_mrd(cpu, cpu->regHL); z80_xor8(cpu, cpu->tmp);}
void nprAF(CPU* cpu) {z80_xor8(cpu, cpu->regA);}
// b0..b8	or r		4 [3rd]
void nprB0(CPU* cpu) {z80_or8(cpu, cpu->regB);}
void nprB1(CPU* cpu) {z80_or8(cpu, cpu->regC);}
void nprB2(CPU* cpu) {z80_or8(cpu, cpu->regD);}
void nprB3(CPU* cpu) {z80_or8(cpu, cpu->regE);}
void nprB4(CPU* cpu) {z80_or8(cpu, cpu->regH);}
void nprB5(CPU* cpu) {z80_or8(cpu, cpu->regL);}
void nprB6(CPU* cpu) {cpu->tmp = z80_mrd(cpu, cpu->regHL); z80_or8(cpu, cpu->tmp);}
void nprB7(CPU* cpu) {z80_or8(cpu, cpu->regA);}
// b9..bf	cp r		4 [3rd]
void nprB8(CPU* cpu) {z80_cp8(cpu, cpu->regB);} //CP(cpu->b);}
void nprB9(CPU* cpu) {z80_cp8(cpu, cpu->regC);} //CP(cpu->c);}
void nprBA(CPU* cpu) {z80_cp8(cpu, cpu->regD);} //CP(cpu->d);}
void nprBB(CPU* cpu) {z80_cp8(cpu, cpu->regE);} //CP(cpu->e);}
void nprBC(CPU* cpu) {z80_cp8(cpu, cpu->regH);} //CP(cpu->h);}
void nprBD(CPU* cpu) {z80_cp8(cpu, cpu->regL);} //CP(cpu->l);}
void nprBE(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->regHL); z80_cp8(cpu, cpu->tmpb);} //CP(cpu->tmpb);}
void nprBF(CPU* cpu) {z80_cp8(cpu, cpu->regA);} //CP(cpu->a);}

// c0	ret nz		5 [3rd 3rd]	mptr = ret.adr (if ret)
void nprC0(CPU* cpu) {
	if (!cpu->fz.z)
		z80_ret(cpu);
}

// c1	pop regBC		4 3rd 3rd
void nprC1(CPU* cpu) {
	cpu->regBC = z80_pop(cpu); // POP(cpu->b,cpu->c);
}

// c2	jp nz,nn	4 3rd 3rd	regWZ = nn
void nprC2(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.z) cpu->regPC = cpu->regWZ;
}

// c3	jp nn		4 3rd 3rd	regWZ = nn
void nprC3(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC);
	cpu->regPC = cpu->regWZ;
}

// c4	call nz,nn	4 3rd 3rd[+1] [3wr 3wr]	regWZ = nn
void nprC4(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.z) {
		cpu->t++;
		z80_push(cpu, cpu->regPC); //PUSH(cpu->hpc,cpu->lpc);
		cpu->regPC = cpu->regWZ;
	}
}

// c5	push regBC		5 3wr 3wr
void nprC5(CPU* cpu) {
	z80_push(cpu, cpu->regBC); // PUSH(cpu->b, cpu->c);
}

// c6	add regA,n		4 3rd
void nprC6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	cpu->regA = z80_add8(cpu, cpu->tmpb, 0); //ADD(cpu->tmpb);
}

// c7	rst00		5 3wr 3wr	mptr = 0
void nprC7(CPU* cpu) {
	z80_call(cpu, 0x00);
}

// c8	ret z		5 [3rd 3rd]	[mptr = ret.adr]
void nprC8(CPU* cpu) {
	if (cpu->fz.z) z80_ret(cpu);
}

// c9	ret		5 3rd 3rd	mptr = ret.adr
void nprC9(CPU* cpu) {
	z80_ret(cpu);
}

// ca	jp z,nn		4 3rd 3rd	regWZ = nn
void nprCA(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.z) cpu->regPC = cpu->regWZ;
}

// cb	prefix		4
void nprCB(CPU* cpu) {
	cpu->opTab = cbTab;
}

// cc	call z,nn	4 3rd 3rd[+1] [3wr 3wr]		regWZ = nn
void nprCC(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.z) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// cd	call nn		4 3rd 4rd 3wr 3wr		regWZ = nn
void nprCD(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++); cpu->t++;
	z80_push(cpu, cpu->regPC);
	cpu->regPC = cpu->regWZ;
}

// ce	adc regA,n		4 3rd
void nprCE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	cpu->regA = z80_add8(cpu, cpu->tmpb, cpu->fz.c); //ADC(cpu->tmpb);
}

// cf	rst08		5 3wr 3wr		mptr = 8
void nprCF(CPU* cpu) {
	z80_call(cpu, 0x08);
}

// d0	ret nc		5 [3rd 3rd]		[mptr = ret.adr]
void nprD0(CPU* cpu) {
	if (!cpu->fz.c) z80_ret(cpu);
}

// d1	pop regDE		4 3rd 3rd
void nprD1(CPU* cpu) {
	cpu->regDE = z80_pop(cpu);
}

// d2	jp nc,nn	4 3rd 3rd		regWZ = nn
void nprD2(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.c) cpu->regPC = cpu->regWZ;
}

// d3	out(n),regA	4 3rd 4out		regWZ = (regA<<8) | ((n + 1) & 0xff)
void nprD3(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = cpu->regA;
	z80_iowr(cpu, cpu->regWZ, cpu->regA);
	cpu->regWZl++;
}

// d4	call nc,nn	4 3rd 3rd[+1] [3wr 3wr]		regWZ = nn
void nprD4(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.c) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// d5	push regDE		5 3wr 3wr
void nprD5(CPU* cpu) {
	z80_push(cpu, cpu->regDE);
}

// d6	sub n		4 3rd
void nprD6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	cpu->regA = z80_sub8(cpu, cpu->tmpb, 0); // SUB(cpu->tmpb);
}

// d7	rst10		5 3wr 3wr	mptr = 0x10
void nprD7(CPU* cpu) {
	z80_call(cpu, 0x10);
}

// d8	ret c		5 [3rd 3rd]	[mptr = ret.adr]
void nprD8(CPU* cpu) {
	if (cpu->fz.c) z80_ret(cpu);
}

// d9	exx		4
void nprD9(CPU* cpu) {
	SWAP(cpu->regBC,cpu->bc_);
	SWAP(cpu->regDE,cpu->de_);
	SWAP(cpu->regHL,cpu->hl_);
}

// da	jp c,nn		4 3rd 3rd	memptr = nn
void nprDA(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.c) cpu->regPC = cpu->regWZ;
}

// db	in regA,(n)	4 3rd 4in	memptr = ((regA<<8) | n) + 1
void nprDB(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = cpu->regA;
	cpu->regA = z80_iord(cpu, cpu->regWZ++);
}

// dc	call c,nn	4 3rd 3rd[+1] [3wr 3wr]		regWZ = nn
void nprDC(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.c) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// dd	prefix IX	4
void nprDD(CPU* cpu) {
	cpu->opTab = ddTab;
}

// de	sbc regA,n		4 3rd
void nprDE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	cpu->regA = z80_sub8(cpu, cpu->tmpb, cpu->fz.c); // SBC(cpu->tmpb);
}

// df	rst18		5 3wr 3wr	mptr = 0x18;
void nprDF(CPU* cpu) {
	z80_call(cpu, 0x18);
}

// e0	ret po		5 [3rd 3rd]	[mptr = ret.adr]
void nprE0(CPU* cpu) {
	if (!cpu->fz.pv) z80_ret(cpu);
}

// e1	pop regHL		4 3rd 3rd
void nprE1(CPU* cpu) {
	cpu->regHL = z80_pop(cpu);
}

// e2	jp po,nn	4 3rd 3rd	regWZ = nn
void nprE2(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.pv) cpu->regPC = cpu->regWZ;
}

// e3	ex (sp),regHL	4 3rd 4rd 3wr 5wr	regWZ = regHL
void nprE3(CPU* cpu) {
	cpu->tmpw = z80_pop(cpu); cpu->t++;	// 3,3+1
	z80_push(cpu, cpu->regHL); cpu->t += 2;	// 3,3+2
	cpu->regHL = cpu->tmpw;
	cpu->regWZ = cpu->regHL;
}

// e4	call po,nn	4 3rd 3rd[+1] [3wr 3wr]		regWZ = nn
void nprE4(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.pv) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// e5	push regHL		5 3wr 3wr
void nprE5(CPU* cpu) {
	z80_push(cpu, cpu->regHL);
}

// e6	and n		4 3rd
void nprE6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	z80_and8(cpu, cpu->tmpb);
}

// e7	rst20		5 3wr 3wr	mptr = 0x20
void nprE7(CPU* cpu) {
	z80_call(cpu, 0x20);
}

// e8	ret pe		5 [3rd 3rd]	[mptr = ret.adr]
void nprE8(CPU* cpu) {
	if (cpu->fz.pv) z80_ret(cpu);
}

// e9	jp (regHL)		4
void nprE9(CPU* cpu) {
	cpu->regPC = cpu->regHL;
}

// ea	jp pe,nn	4 3rd 3rd	regWZ = nn
void nprEA(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.pv) cpu->regPC = cpu->regWZ;
}

// eb	ex regDE,regHL
void nprEB(CPU* cpu) {
	SWAP(cpu->regHL,cpu->regDE);
}

// ec	call pe,nn	4 3rd 3rd[+1] 3wr 3wr	regWZ = nn
void nprEC(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.pv) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// ed	prefix		4
void nprED(CPU* cpu) {
	cpu->opTab = edTab;
}

// ee	xor n		4 3rd
void nprEE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	z80_xor8(cpu, cpu->tmpb);
}

// ef	rst28		5 3wr 3wr	mptr = 0x28
void nprEF(CPU* cpu) {
	z80_call(cpu, 0x28);
}

// f0	ret p		5 [3rd 3rd]	[mptr = ret.adr]
void nprF0(CPU* cpu) {
	if (!cpu->fz.s) z80_ret(cpu);
}

// f1	pop af		4 3rd 3rd
void nprF1(CPU* cpu) {
	cpu->tmpw = z80_pop(cpu);
	cpu->f = cpu->ltw;
	cpu->regA = cpu->htw;
}

// f2	jp p,nn		4 3rd 3rd	regWZ = nn
void nprF2(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.s) cpu->regPC = cpu->regWZ;
}

// f3	di		4
void nprF3(CPU* cpu) {
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->inten &= ~Z80_INT;
}

// f4	call p,nn	4 3rd 3rd[+1] [3wr 3wr]		memptr = nn
void nprF4(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (!cpu->fz.s) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// f5	push af		5 3wr 3wr
void nprF5(CPU* cpu) {
	cpu->ltw = cpu->f;
	cpu->htw = cpu->regA;
	z80_push(cpu, cpu->tmpw);
}

// f6	or n		4 3rd
void nprF6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	z80_or8(cpu, cpu->tmpb);
}

// f7	rst30		5 3wr 3wr		mptr = 0x30
void nprF7(CPU* cpu) {
	z80_call(cpu, 0x30);
}

// f8	ret m		5 [3rd 3rd]		[mptr = ret.adr]
void nprF8(CPU* cpu) {
	if (cpu->fz.s) z80_ret(cpu);
}

// f9	ld regSP,regHL	6
void nprF9(CPU* cpu) {
	cpu->regSP = cpu->regHL;
}

// fa	jp m,nn		4 3rd 3rd		regWZ = nn
void nprFA(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.s) cpu->regPC = cpu->regWZ;
}

// fb	ei		4
void nprFB(CPU* cpu) {
	cpu->iff1 = 1;
	cpu->iff2 = 1;
	cpu->noint = 1;
	cpu->inten |= Z80_INT;
}

// fc	call m,nn	4 3rd 3rd[+1] [3wr 3wr]		regWZ = nn
void nprFC(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	if (cpu->fz.s) {
		cpu->t++;
		z80_push(cpu, cpu->regPC);
		cpu->regPC = cpu->regWZ;
	}
}

// fd	prefix IY	4
void nprFD(CPU* cpu) {
	cpu->opTab = fdTab;
}

// fe	cp n		4 3rd
void nprFE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regPC++);
	z80_cp8(cpu, cpu->tmpb); // CP(cpu->tmpb);
}

// ff	rst38		5 3rd 3rd	mptr = 0x38;
void nprFF(CPU* cpu) {
	z80_call(cpu, 0x38);
}

//==================

opCode npTab[256]={
	{0,4,npr00,NULL,"nop"},
	{0,4,npr01,NULL,"ld bc,:2"},
	{0,4,npr02,NULL,"ld (bc),a"},
	{0,6,npr03,NULL,"inc bc"},
	{0,4,npr04,NULL,"inc b"},
	{0,4,npr05,NULL,"dec b"},
	{0,4,npr06,NULL,"ld b,:1"},
	{0,4,npr07,NULL,"rlca"},

	{0,4,npr08,NULL,"ex af,af'"},
	{0,11,npr09,NULL,"add hl,bc"},
	{0,4,npr0A,NULL,"ld a,(bc)"},
	{0,6,npr0B,NULL,"dec bc"},
	{0,4,npr0C,NULL,"inc c"},
	{0,4,npr0D,NULL,"dec c"},
	{0,4,npr0E,NULL,"ld c,:1"},
	{0,4,npr0F,NULL,"rrca"},

	{OF_SKIPABLE | OF_RELJUMP,5,npr10,NULL,"djnz :3"},
	{0,4,npr11,NULL,"ld de,:2"},
	{0,4,npr12,NULL,"ld (de),a"},
	{0,6,npr13,NULL,"inc de"},
	{0,4,npr14,NULL,"inc d"},
	{0,4,npr15,NULL,"dec d"},
	{0,4,npr16,NULL,"ld d,:1"},
	{0,4,npr17,NULL,"rla"},

	{OF_RELJUMP,4,npr18,NULL,"jr :3"},
	{0,11,npr19,NULL,"add hl,de"},
	{0,4,npr1A,NULL,"ld a,(de)"},
	{0,6,npr1B,NULL,"dec de"},
	{0,4,npr1C,NULL,"inc e"},
	{0,4,npr1D,NULL,"dec e"},
	{0,4,npr1E,NULL,"ld e,:1"},
	{0,4,npr1F,NULL,"rra"},

	{OF_RELJUMP,4,npr20,NULL,"jr nz,:3"},
	{0,4,npr21,NULL,"ld hl,:2"},
	{OF_MWORD | OF_MEMADR,4,npr22,NULL,"ld (:2),hl"},		// 4,3rd,3rd,3wr,3wr
	{0,6,npr23,NULL,"inc hl"},
	{0,4,npr24,NULL,"inc h"},
	{0,4,npr25,NULL,"dec h"},
	{0,4,npr26,NULL,"ld h,:1"},
	{0,4,npr27,NULL,"daa"},

	{OF_RELJUMP,4,npr28,NULL,"jr z,:3"},
	{0,11,npr29,NULL,"add hl,hl"},
	{OF_MWORD | OF_MEMADR,4,npr2A,NULL,"ld hl,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,npr2B,NULL,"dec hl"},
	{0,4,npr2C,NULL,"inc l"},
	{0,4,npr2D,NULL,"dec l"},
	{0,4,npr2E,NULL,"ld l,:1"},
	{0,4,npr2F,NULL,"cpl"},

	{OF_RELJUMP,4,npr30,NULL,"jr nc,:3"},
	{0,4,npr31,NULL,"ld sp,:2"},
	{OF_MEMADR,4,npr32,NULL,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,npr33,NULL,"inc sp"},
	{0,4,npr34,NULL,"inc (hl)"},
	{0,4,npr35,NULL,"dec (hl)"},
	{0,4,npr36,NULL,"ld (hl),:1"},
	{0,4,npr37,NULL,"scf"},

	{OF_RELJUMP,4,npr38,NULL,"jr c,:3"},
	{0,11,npr39,NULL,"add hl,sp"},
	{OF_MEMADR,4,npr3A,NULL,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,npr3B,NULL,"dec sp"},
	{0,4,npr3C,NULL,"inc a"},
	{0,4,npr3D,NULL,"dec a"},
	{0,4,npr3E,NULL,"ld a,:1"},
	{0,4,npr3F,NULL,"ccf"},

	{0,4,npr40,NULL,"ld b,b"},
	{0,4,npr41,NULL,"ld b,c"},
	{0,4,npr42,NULL,"ld b,d"},
	{0,4,npr43,NULL,"ld b,e"},
	{0,4,npr44,NULL,"ld b,h"},
	{0,4,npr45,NULL,"ld b,l"},
	{0,4,npr46,NULL,"ld b,(hl)"},
	{0,4,npr47,NULL,"ld b,a"},

	{0,4,npr48,NULL,"ld c,b"},
	{0,4,npr49,NULL,"ld c,c"},
	{0,4,npr4A,NULL,"ld c,d"},
	{0,4,npr4B,NULL,"ld c,e"},
	{0,4,npr4C,NULL,"ld c,h"},
	{0,4,npr4D,NULL,"ld c,l"},
	{0,4,npr4E,NULL,"ld c,(hl)"},
	{0,4,npr4F,NULL,"ld c,a"},

	{0,4,npr50,NULL,"ld d,b"},
	{0,4,npr51,NULL,"ld d,c"},
	{0,4,npr52,NULL,"ld d,d"},
	{0,4,npr53,NULL,"ld d,e"},
	{0,4,npr54,NULL,"ld d,h"},
	{0,4,npr55,NULL,"ld d,l"},
	{0,4,npr56,NULL,"ld d,(hl)"},
	{0,4,npr57,NULL,"ld d,a"},

	{0,4,npr58,NULL,"ld e,b"},
	{0,4,npr59,NULL,"ld e,c"},
	{0,4,npr5A,NULL,"ld e,d"},
	{0,4,npr5B,NULL,"ld e,e"},
	{0,4,npr5C,NULL,"ld e,h"},
	{0,4,npr5D,NULL,"ld e,l"},
	{0,4,npr5E,NULL,"ld e,(hl)"},
	{0,4,npr5F,NULL,"ld e,a"},

	{0,4,npr60,NULL,"ld h,b"},
	{0,4,npr61,NULL,"ld h,c"},
	{0,4,npr62,NULL,"ld h,d"},
	{0,4,npr63,NULL,"ld h,e"},
	{0,4,npr64,NULL,"ld h,h"},
	{0,4,npr65,NULL,"ld h,l"},
	{0,4,npr66,NULL,"ld h,(hl)"},
	{0,4,npr67,NULL,"ld h,a"},

	{0,4,npr68,NULL,"ld l,b"},
	{0,4,npr69,NULL,"ld l,c"},
	{0,4,npr6A,NULL,"ld l,d"},
	{0,4,npr6B,NULL,"ld l,e"},
	{0,4,npr6C,NULL,"ld l,h"},
	{0,4,npr6D,NULL,"ld l,l"},
	{0,4,npr6E,NULL,"ld l,(hl)"},
	{0,4,npr6F,NULL,"ld l,a"},

	{0,4,npr70,NULL,"ld (hl),b"},
	{0,4,npr71,NULL,"ld (hl),c"},
	{0,4,npr72,NULL,"ld (hl),d"},
	{0,4,npr73,NULL,"ld (hl),e"},
	{0,4,npr74,NULL,"ld (hl),h"},
	{0,4,npr75,NULL,"ld (hl),l"},
	{OF_SKIPABLE,4,npr76,NULL,"halt"},
	{0,4,npr77,NULL,"ld (hl),a"},

	{0,4,npr78,NULL,"ld a,b"},
	{0,4,npr79,NULL,"ld a,c"},
	{0,4,npr7A,NULL,"ld a,d"},
	{0,4,npr7B,NULL,"ld a,e"},
	{0,4,npr7C,NULL,"ld a,h"},
	{0,4,npr7D,NULL,"ld a,l"},
	{0,4,npr7E,NULL,"ld a,(hl)"},
	{0,4,npr7F,NULL,"ld a,a"},

	{0,4,npr80,NULL,"add a,b"},
	{0,4,npr81,NULL,"add a,c"},
	{0,4,npr82,NULL,"add a,d"},
	{0,4,npr83,NULL,"add a,e"},
	{0,4,npr84,NULL,"add a,h"},
	{0,4,npr85,NULL,"add a,l"},
	{0,4,npr86,NULL,"add a,(hl)"},
	{0,4,npr87,NULL,"add a,a"},

	{0,4,npr88,NULL,"adc a,b"},
	{0,4,npr89,NULL,"adc a,c"},
	{0,4,npr8A,NULL,"adc a,d"},
	{0,4,npr8B,NULL,"adc a,e"},
	{0,4,npr8C,NULL,"adc a,h"},
	{0,4,npr8D,NULL,"adc a,l"},
	{0,4,npr8E,NULL,"adc a,(hl)"},
	{0,4,npr8F,NULL,"adc a,a"},

	{0,4,npr90,NULL,"sub b"},
	{0,4,npr91,NULL,"sub c"},
	{0,4,npr92,NULL,"sub d"},
	{0,4,npr93,NULL,"sub e"},
	{0,4,npr94,NULL,"sub h"},
	{0,4,npr95,NULL,"sub l"},
	{0,4,npr96,NULL,"sub (hl)"},
	{0,4,npr97,NULL,"sub a"},

	{0,4,npr98,NULL,"sbc a,b"},
	{0,4,npr99,NULL,"sbc a,c"},
	{0,4,npr9A,NULL,"sbc a,d"},
	{0,4,npr9B,NULL,"sbc a,e"},
	{0,4,npr9C,NULL,"sbc a,h"},
	{0,4,npr9D,NULL,"sbc a,l"},
	{0,4,npr9E,NULL,"sbc a,(hl)"},
	{0,4,npr9F,NULL,"sbc a,a"},

	{0,4,nprA0,NULL,"and b"},
	{0,4,nprA1,NULL,"and c"},
	{0,4,nprA2,NULL,"and d"},
	{0,4,nprA3,NULL,"and e"},
	{0,4,nprA4,NULL,"and h"},
	{0,4,nprA5,NULL,"and l"},
	{0,4,nprA6,NULL,"and (hl)"},
	{0,4,nprA7,NULL,"and a"},

	{0,4,nprA8,NULL,"xor b"},
	{0,4,nprA9,NULL,"xor c"},
	{0,4,nprAA,NULL,"xor d"},
	{0,4,nprAB,NULL,"xor e"},
	{0,4,nprAC,NULL,"xor h"},
	{0,4,nprAD,NULL,"xor l"},
	{0,4,nprAE,NULL,"xor (hl)"},
	{0,4,nprAF,NULL,"xor a"},

	{0,4,nprB0,NULL,"or b"},
	{0,4,nprB1,NULL,"or c"},
	{0,4,nprB2,NULL,"or d"},
	{0,4,nprB3,NULL,"or e"},
	{0,4,nprB4,NULL,"or h"},
	{0,4,nprB5,NULL,"or l"},
	{0,4,nprB6,NULL,"or (hl)"},
	{0,4,nprB7,NULL,"or a"},

	{0,4,nprB8,NULL,"cp b"},
	{0,4,nprB9,NULL,"cp c"},
	{0,4,nprBA,NULL,"cp d"},
	{0,4,nprBB,NULL,"cp e"},
	{0,4,nprBC,NULL,"cp h"},
	{0,4,nprBD,NULL,"cp l"},
	{0,4,nprBE,NULL,"cp (hl)"},
	{0,4,nprBF,NULL,"cp a"},

	{0,5,nprC0,NULL,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,nprC1,NULL,"pop bc"},
	{0,4,nprC2,NULL,"jp nz,:2"},
	{0,4,nprC3,NULL,"jp :2"},
	{OF_SKIPABLE,4,nprC4,NULL,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,nprC5,NULL,"push bc"},		// 5 3wr 3wr
	{0,4,nprC6,NULL,"add a,:1"},
	{OF_SKIPABLE,5,nprC7,NULL,"rst #00"},		// 5 3wr 3wr

	{0,5,nprC8,NULL,"ret z"},
	{0,4,nprC9,NULL,"ret"},
	{0,4,nprCA,NULL,"jp z,:2"},
	{OF_PREFIX,4,nprCB,cbTab,"#CB"},
	{OF_SKIPABLE,4,nprCC,NULL,"call z,:2"},
	{OF_SKIPABLE,4,nprCD,NULL,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,nprCE,NULL,"adc a,:1"},
	{OF_SKIPABLE,5,nprCF,NULL,"rst #08"},

	{0,5,nprD0,NULL,"ret nc"},
	{0,4,nprD1,NULL,"pop de"},
	{0,4,nprD2,NULL,"jp nc,:2"},
	{0,4,nprD3,NULL,"out (:1),a"},
	{OF_SKIPABLE,4,nprD4,NULL,"call nc,:2"},
	{0,5,nprD5,NULL,"push de"},
	{0,4,nprD6,NULL,"sub :1"},
	{OF_SKIPABLE,5,nprD7,NULL,"rst #10"},

	{0,5,nprD8,NULL,"ret c"},
	{0,4,nprD9,NULL,"exx"},
	{0,4,nprDA,NULL,"jp c,:2"},
	{0,4,nprDB,NULL,"in a,(:1)"},
	{OF_SKIPABLE,4,nprDC,NULL,"call c,:2"},
	{OF_PREFIX,4,nprDD,ddTab,"#DD"},
	{0,4,nprDE,NULL,"sbc a,:1"},
	{OF_SKIPABLE,5,nprDF,NULL,"rst #18"},

	{0,5,nprE0,NULL,"ret po"},
	{0,4,nprE1,NULL,"pop hl"},
	{0,4,nprE2,NULL,"jp po,:2"},
	{0,4,nprE3,NULL,"ex (sp),hl"},		// 4 3rd 4rd 3wr 5wr
	{OF_SKIPABLE,4,nprE4,NULL,"call po,:2"},
	{0,5,nprE5,NULL,"push hl"},
	{0,4,nprE6,NULL,"and :1"},
	{OF_SKIPABLE,5,nprE7,NULL,"rst #20"},

	{0,5,nprE8,NULL,"ret pe"},
	{0,4,nprE9,NULL,"jp (hl)"},
	{0,4,nprEA,NULL,"jp pe,:2"},
	{0,4,nprEB,NULL,"ex de,hl"},
	{OF_SKIPABLE,4,nprEC,NULL,"call pe,:2"},
	{OF_PREFIX,4,nprED,edTab,"#ED"},
	{0,4,nprEE,NULL,"xor :1"},
	{OF_SKIPABLE,5,nprEF,NULL,"rst #28"},

	{0,5,nprF0,NULL,"ret p"},
	{0,4,nprF1,NULL,"pop af"},
	{0,4,nprF2,NULL,"jp p,:2"},
	{0,4,nprF3,NULL,"di"},
	{OF_SKIPABLE,4,nprF4,NULL,"call p,:2"},
	{0,5,nprF5,NULL,"push af"},
	{0,4,nprF6,NULL,"or :1"},
	{OF_SKIPABLE,5,nprF7,NULL,"rst #30"},

	{0,5,nprF8,NULL,"ret m"},
	{0,6,nprF9,NULL,"ld sp,hl"},
	{0,4,nprFA,NULL,"jp m,:2"},
	{0,4,nprFB,NULL,"ei"},
	{OF_SKIPABLE,4,nprFC,NULL,"call m,:2"},
	{OF_PREFIX,4,nprFD,fdTab,"#FD"},
	{0,4,nprFE,NULL,"cp :1"},
	{OF_SKIPABLE,5,nprFF,NULL,"rst #38"}
};
