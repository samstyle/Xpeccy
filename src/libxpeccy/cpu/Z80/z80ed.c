#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"
#include "z80_nop.h"

// 40	in regB,(c)	4 4in		regWZ = regBC+1
void ed40(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regB = z80_iord(cpu, cpu->regWZ++); // IORD(cpu->mptr++,4);
	cpu->f.s = !!(cpu->regB & 0x80);
	cpu->f.z = !cpu->regB;
	cpu->f.f5 = !!(cpu->regB & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regB & 0x08);
	cpu->f.pv = parity(cpu->regB);
	cpu->f.n = 0;
	//cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->b];
}

// 41	out (c),regB	4 4out		regWZ = (a<<8) | ((port + 1) & 0xff)
void ed41(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regB);
//	cpu->hptr = cpu->a;
}

// 42	sbc regHL,regBC	11
void ed42(CPU* cpu) {
	cpu->regHL = z80_sub16(cpu, cpu->regHL, cpu->regBC, cpu->f.c); //SBC16(cpu->bc);
}

// 43	ld (nn),bc	4 3rd 3rd 3wr 3wr	regWZ = nn + 1
void ed43(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regWZ++, cpu->regC);
	z80_mwr(cpu, cpu->regWZ, cpu->regB);
}

// 44	neg	4
void ed44(CPU* cpu) {
	cpu->tmpb = cpu->regA;
	cpu->regA = 0;
	cpu->regA = z80_sub8(cpu, cpu->tmpb, 0); //SUB(cpu->tmpb);
}

// 45	retn	4 3rd 3rd
void ed45(CPU* cpu) {
	cpu->f.iff1 = cpu->f.iff2;
	z80_ret(cpu);
}

// 46	im0	4
void ed46(CPU* cpu) {
	cpu->f.im = 0;
}

// 47	ld i,regA	5
void ed47(CPU* cpu) {
	cpu->i = cpu->regA;
}

// 48	in regC,(regC)	4 4in		regWZ = port + 1
void ed48(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regC = z80_iord(cpu, cpu->regWZ++);
	cpu->f.s = !!(cpu->regC & 0x80);
	cpu->f.z = !cpu->regC;
	cpu->f.f5 = !!(cpu->regC & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regC & 0x08);
	cpu->f.pv = parity(cpu->regC);
	cpu->f.n = 0;
}

// 49	out (regC),regC	4 4out		regWZ = (a<<8) | ((port + 1) & 0xff)
void ed49(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regC);
//	cpu->hptr = cpu->a;
}

// 4a	adc regHL,regBC	11
void ed4A(CPU* cpu) {
	cpu->regHL = z80_adc16(cpu, cpu->regHL, cpu->regBC, cpu->f.c); // ADC16(cpu->bc);
}

// 4b	ld bc,(nn)	4 3rd 3rd 3rd 3rd	regWZ = nn+1
void ed4B(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	cpu->regC = z80_mrd(cpu, cpu->regWZ++);
	cpu->regB = z80_mrd(cpu, cpu->regWZ);
}

// 4d	reti	4 3rd 3rd
void ed4D(CPU* cpu) {
	// cpu->iff1 = cpu->iff2;
	z80_ret(cpu);
}

// 4f	ld r,regA	5
void ed4F(CPU* cpu) {
	cpu->r = cpu->regA;
	cpu->r7 = cpu->regA & 0x80;
}

// 50	in regD,(c)	4 4in	regWZ = port + 1
void ed50(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regD = z80_iord(cpu, cpu->regWZ++);
	cpu->f.s = !!(cpu->regD & 0x80);
	cpu->f.z = !cpu->regD;
	cpu->f.f5 = !!(cpu->regD & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regD & 0x08);
	cpu->f.pv = parity(cpu->regD);
	cpu->f.n = 0;
}

// 51	out (c),regD	4 4out	regWZ = (a<<8) | ((port+1) & ff)
void ed51(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regD);
//	cpu->hptr = cpu->a;
}

// 52	sbc regHL,regDE	11
void ed52(CPU* cpu) {
	cpu->regHL = z80_sub16(cpu, cpu->regHL, cpu->regDE, cpu->f.c); //SBC16(cpu->de);
}

// 53	ld (nn),de	4 3rd 3rd 3wr 3wr	regWZ = nn + 1
void ed53(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regWZ++,cpu->regE);
	z80_mwr(cpu, cpu->regWZ,cpu->regD);
}

// 56	im1		4
void ed56(CPU* cpu) {
	cpu->f.im = 1;
}

// 57	ld regA,i		5
void ed57(CPU* cpu) {
	cpu->regA = cpu->i;
	// cpu->f = (cpu->f & Z80_FC) | (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | (cpu->iff2 ? Z80_FV : 0);
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = cpu->f.iff2;
	cpu->f.n = 0;
	cpu->resPV = 1;			// if INT coming after this opcode, reset PV flag
}

// 58	in regE,(c)	4 4in		regWZ = port + 1
void ed58(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regE = z80_iord(cpu, cpu->regWZ++);
	cpu->f.s = !!(cpu->regE & 0x80);
	cpu->f.z = !cpu->regE;
	cpu->f.f5 = !!(cpu->regE & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regE & 0x08);
	cpu->f.pv = parity(cpu->regE);
	cpu->f.n = 0;
}

// 59	out (c),regE	4 4out		regWZ = ((port+1) & ff) | (a << 8)
void ed59(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regE);
//	cpu->hptr = cpu->a;
}

// 5a	adc regHL,regDE	11
void ed5A(CPU* cpu) {
	cpu->regHL = z80_adc16(cpu, cpu->regHL, cpu->regDE, cpu->f.c); //ADC16(cpu->de);
}

// 5b	ld de,(nn)	4 3rd 3rd 3rd 3rd	regWZ = nn + 1
void ed5B(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	cpu->regE = z80_mrd(cpu, cpu->regWZ++);
	cpu->regD = z80_mrd(cpu, cpu->regWZ);
}

// 5e	im2		4
void ed5E(CPU* cpu) {
	cpu->f.im = 2;
}

// 5f	ld regA,r		5
void ed5F(CPU* cpu) {
	cpu->regA = (cpu->r & 0x7f) | (cpu->r7 & 0x80);
//	cpu->f = (cpu->f & Z80_FC) | (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | (cpu->iff2 ? Z80_FV : 0);
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = cpu->f.iff2;
	cpu->f.n = 0;
	cpu->resPV = 1;
}

// 60	in regH,(c)	4 4in		regWZ = port + 1
void ed60(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regH = z80_iord(cpu, cpu->regWZ++);
	cpu->f.s = !!(cpu->regH & 0x80);
	cpu->f.z = !cpu->regH;
	cpu->f.f5 = !!(cpu->regH & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regH & 0x08);
	cpu->f.pv = parity(cpu->regH);
	cpu->f.n = 0;
}

// 61	out (c),regH	4 4out		regWZ = ((port + 1) & FF) | (a << 8)
void ed61(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regH);
//	cpu->hptr = a;
}

// 62	sbc regHL,regHL	11
void ed62(CPU* cpu) {
	cpu->regHL = z80_sub16(cpu, cpu->regHL, cpu->regHL, cpu->f.c); //SBC16(cpu->hl);
}

// 67	rrd		4 3rd 4 3wr	regWZ = regHL + 1
void ed67(CPU* cpu) {
	cpu->regWZ = cpu->regHL;
	cpu->tmpb = z80_mrd(cpu, cpu->regWZ);
	cpu->t += 4;
	z80_mwr(cpu, cpu->regWZ++, (cpu->regA << 4) | (cpu->tmpb >> 4));
	cpu->regA = (cpu->regA & 0xf0) | (cpu->tmpb & 0x0f);
//	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->a];
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = parity(cpu->regA);
	cpu->f.n = 0;
}

// 68	in regL,(c)	4 4in		regWZ = port + 1
void ed68(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regL = z80_iord(cpu, cpu->regWZ++);
	cpu->f.s = !!(cpu->regL & 0x80);
	cpu->f.z = !cpu->regL;
	cpu->f.f5 = !!(cpu->regL & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regL & 0x08);
	cpu->f.pv = parity(cpu->regL);
	cpu->f.n = 0;
}

// 69	out (c),regL	4 4out		regWZ = ((port+1)&FF)|(a<<8)
void ed69(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regL);
//	cpu->hptr = cpu->a;
}

// 6a	adc regHL,regHL	11
void ed6A(CPU* cpu) {
	cpu->regHL = z80_adc16(cpu, cpu->regHL, cpu->regHL, cpu->f.c); //ADC16(cpu->hl);
}

// 6f	rld		4 3rd 4 3wr	regWZ = regHL+1
void ed6F(CPU* cpu) {
	cpu->regWZ = cpu->regHL;
	cpu->tmpb = z80_mrd(cpu, cpu->regWZ);
	cpu->t += 4;
	z80_mwr(cpu, cpu->regWZ++, (cpu->tmpb << 4 ) | (cpu->regA & 0x0f));
	cpu->regA = (cpu->regA & 0xf0) | (cpu->tmpb >> 4);
//	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->a];
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = parity(cpu->regA);
	cpu->f.n = 0;
}

// 70	in (c)		4 4in		regWZ = port + 1
void ed70(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->tmp = z80_iord(cpu, cpu->regWZ++);
//	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->tmp];
	cpu->f.s = !!(cpu->tmp & 0x80);
	cpu->f.z = !cpu->tmp;
	cpu->f.f5 = !!(cpu->tmp & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->tmp & 0x08);
	cpu->f.pv = parity(cpu->tmp);
	cpu->f.n = 0;
}

// 71	out (c),0	4 4out		regWZ = ((port+1)&FF)|(a<<8)
void ed71(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, 0);
//	cpu->hptr = cpu->a;
}

// 72	sbc regHL,regSP	11
void ed72(CPU* cpu) {
	cpu->regHL = z80_sub16(cpu, cpu->regHL, cpu->regSP, cpu->f.c); //SBC16(cpu->sp);
}

// 73	ld (nn),sp	4 3rd 3rd 3wr 3wr	regWZ = nn + 1
void ed73(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regWZ++, cpu->regSPl);
	z80_mwr(cpu, cpu->regWZ,cpu->regSPh);
}

// 78	in regA,(c)	4 4in		regWZ = port + 1
void ed78(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	cpu->regA = z80_iord(cpu, cpu->regWZ++);
//	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->a];
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = parity(cpu->regA);
	cpu->f.n = 0;
}

// 79	out (c),regA	4 4out		regWZ = ((port+1)&FF)|(regA<<8)
void ed79(CPU* cpu) {
	cpu->regWZ = cpu->regBC;
	z80_iowr(cpu, cpu->regWZ++, cpu->regA);
//	cpu->hptr = cpu->a;
}

// 7a	adc regHL,regSP	11
void ed7A(CPU* cpu) {
	cpu->regHL = z80_adc16(cpu, cpu->regHL, cpu->regSP, cpu->f.c); //ADC16(cpu->sp);
}

// 7b	ld sp,(nn)	4 3rd 3rd 3rd 3rd	regWZ = nn + 1
void ed7B(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	cpu->regSPl = z80_mrd(cpu, cpu->regWZ++);
	cpu->regSPh = z80_mrd(cpu, cpu->regWZ);
}

// a0	ldi	4 3rd 5wr
void edA0(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regHL++);
	z80_mwr(cpu, cpu->regDE++, cpu->tmp);
	cpu->t += 2;
	cpu->regBC--;
	cpu->tmp += cpu->regA;
	//cpu->f = (cpu->f & (Z80_FC | Z80_FZ | Z80_FS)) | (cpu->bc ? Z80_FV : 0 ) | (cpu->tmp & Z80_F3) | ((cpu->tmp & 0x02) ? Z80_F5 : 0);
	cpu->f.f5 = !!(cpu->tmp & 2);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->tmp & 8);
	cpu->f.pv = !!cpu->regBC;
	cpu->f.n = 0;
}

// a1	cpi	4 3rd 5?	regWZ++
void edA1(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regHL);
	cpu->tmpw = cpu->regA - cpu->tmpb;
	cpu->tmp = ((cpu->regA & 0x08) >> 3) | ((cpu->tmpb & 0x08) >> 2) | ((cpu->tmpw & 0x08 ) >> 1);
	cpu->regHL++;
	cpu->regBC--;
	//cpu->f = (cpu->f & Z80_FC) | (cpu->bc ? Z80_FV : 0) | Z80_FN | (cpu->tmpw ? 0 : Z80_FZ) | (cpu->tmpw & Z80_FS);
	cpu->f.s = !!(cpu->tmpw & 0x80);
	cpu->f.z = !cpu->tmpw;
	cpu->f.h = !!FHsubTab[cpu->tmp];
	cpu->f.pv = !!cpu->regBC;
	cpu->f.n = 1;
	if (cpu->f.h) cpu->tmpw--;
	//cpu->f |= (cpu->tmpw & Z80_F3) | ((cpu->tmpw & 0x02) ? Z80_F5 : 0);
	cpu->f.f5 = !!(cpu->tmpw & 2);
	cpu->f.f3 = !!(cpu->tmpw & 3);
	cpu->regWZ++;
	cpu->t += 5;
}

// TODO: about in/out block instructions PV flag - https://rk.nvg.ntnu.no/sinclair/faq/tech_z80.html#UNDOC

// a2	ini	5 4in 3wr	regWZ = regBC + 1 (before dec)
void edA2(CPU* cpu) {
	cpu->regWZ = cpu->regBC + 1;
	cpu->tmp = z80_iord(cpu, cpu->regBC);
	z80_mwr(cpu, cpu->regHL++, cpu->tmp);
	cpu->regB--;
	//cpu->f = (cpu->tmp & 0x80 ? Z80_FN : 0) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);
	cpu->f.s = !!(cpu->regB & 0x80);
	cpu->f.z = !cpu->regB;
	cpu->f.f5 = !!(cpu->regB & 0x20);
	cpu->f.f3 = !!(cpu->regB & 0x08);
	cpu->f.n = !!(cpu->tmp & 0x80);
	cpu->tmpw = cpu->tmp + ((cpu->regC + 1) & 0xff);
	cpu->f.c = !!(cpu->tmpw > 255);
	cpu->f.h = cpu->f.c;
	// cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
	cpu->f.pv = parity((cpu->tmpw & 7) ^ cpu->regB);
}

// a3	outi	5 3rd 4wr	regWZ = regBC + 1 (after dec)
void edA3(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regHL);
	cpu->regB--;
	cpu->regWZ = cpu->regBC + 1;
	z80_iowr(cpu, cpu->regBC, cpu->tmp);
	cpu->regHL++;
//	cpu->f = (cpu->tmp & 0x80 ? Z80_FN : 0 ) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);
	cpu->f.s = !!(cpu->regB & 0x80);
	cpu->f.z = !cpu->regB;
	cpu->f.f5 = !!(cpu->regB & 0x20);
	cpu->f.f3 = !!(cpu->regB & 0x08);
	cpu->f.n = !!(cpu->tmp & 0x80);
	cpu->tmpw = cpu->tmp + cpu->regL;
	cpu->f.c = !!(cpu->tmpw > 255);
	cpu->f.h = cpu->f.c;
	//cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
	cpu->f.pv = parity((cpu->tmpw & 7) ^ cpu->regB);
}

// a8	ldd	4 3rd 5wr
void edA8(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regHL--);
	z80_mwr(cpu, cpu->regDE--,cpu->tmp);
	cpu->t += 2;
	cpu->regBC--;
	cpu->tmp += cpu->regA;
	//cpu->f = (cpu->f & (Z80_FC | Z80_FZ | Z80_FS)) | (cpu->bc ? Z80_FV : 0 ) | (cpu->tmp & Z80_F3) | ((cpu->tmp & 0x02) ? Z80_F5 : 0);
	cpu->f.f5 = !!(cpu->tmp & 2);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->tmp & 8);
	cpu->f.pv = !!cpu->regBC;
	cpu->f.n = 0;
}

// a9	cpd	4 3rd 5?	regWZ--
void edA9(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->regHL);
	cpu->tmpw = cpu->regA - cpu->tmpb;
	cpu->tmp = ((cpu->regA & 0x08) >> 3) | ((cpu->tmpb & 0x08) >> 2) | ((cpu->tmpw & 0x08 ) >> 1);
	cpu->regHL--;
	cpu->regBC--;
	//cpu->f = (cpu->f & Z80_FC) | (cpu->bc ? Z80_FV : 0) | Z80_FN | (cpu->tmpw ? 0 : Z80_FZ) | (cpu->tmpw & Z80_FS);
	cpu->f.s = !!(cpu->tmpw & 0x80);
	cpu->f.z = !cpu->tmpw;
	cpu->f.h = !!FHsubTab[cpu->tmp];
	cpu->f.pv = !!cpu->regBC;
	cpu->f.n = 1;
	if (cpu->f.h) cpu->tmpw--;
	//cpu->f |= (cpu->tmpw & Z80_F3) | ((cpu->tmpw & 0x02) ? Z80_F5 : 0);
	cpu->f.f5 = !!(cpu->tmpw & 2);
	cpu->f.f3 = !!(cpu->tmpw & 8);
	cpu->regWZ--;
	cpu->t += 5;
}

// aa	ind	5 4in 3wr	regWZ = regBC - 1 (before dec)
void edAA(CPU* cpu) {
	cpu->regWZ = cpu->regBC - 1;
	cpu->tmp = z80_iord(cpu, cpu->regBC);
	z80_mwr(cpu, cpu->regHL--, cpu->tmp);
	cpu->regB--;
//	cpu->f = ((cpu->tmp & 0x80) ? Z80_FN : 0) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);
	cpu->f.s = !!(cpu->regB & 0x80);
	cpu->f.z = !cpu->regB;
	cpu->f.f5 = !!(cpu->regB & 0x20);
	cpu->f.f3 = !!(cpu->regB & 0x08);
	cpu->f.n = !!(cpu->tmp & 0x80);
	cpu->tmpw = cpu->tmp + ((cpu->regC - 1) & 0xff);
	cpu->f.c = !!(cpu->tmpw > 255);
	cpu->f.h = cpu->f.c;
	//cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
	cpu->f.pv = parity((cpu->tmpw & 7) ^ cpu->regB);
}

// ab	outd	5 3rd 4wr	regWZ = regBC - 1 (after dec)
void edAB(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->regHL);
	cpu->regB--;
	cpu->regWZ = cpu->regBC - 1;
	z80_iowr(cpu, cpu->regBC, cpu->tmp);
	cpu->regHL--;
	//cpu->f = (cpu->tmp & 0x80 ? Z80_FN : 0 ) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);
	cpu->f.s = !!(cpu->regB & 0x80);
	cpu->f.z = !cpu->regB;
	cpu->f.f5 = !!(cpu->regB & 0x20);
	cpu->f.f3 = !!(cpu->regB & 0x08);
	cpu->f.n = !!(cpu->tmp & 0x80);
	cpu->tmpw = cpu->tmp + cpu->regL;
	cpu->f.c = !!(cpu->tmpw > 255);
	cpu->f.h = cpu->f.c;
	//cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
	cpu->f.pv = parity((cpu->tmpw & 7) ^ cpu->regB);
}

// for ldxr/cpxr: f3,f5 from regPCh (TODO: or from hi.regWZ=regPC+1?)
void blkRepeat(CPU* cpu) {
	cpu->regPC -= 2;
	cpu->t += 5;
	cpu->regWZ = cpu->regPC + 1;
	cpu->f.f5 = !!(cpu->regPCh & 0x20);
	cpu->f.f3 = !!(cpu->regPCh & 0x08);
}

// b0	ldir	= ldi until regBC!=0	[+5T, mptr = pc+1]
void edB0(CPU* cpu) {
	edA0(cpu);
	if (cpu->regBC) {
		blkRepeat(cpu);
	}
}

// b1	cpir	= cpi until (FV & !FZ)
void edB1(CPU* cpu) {
	edA1(cpu);
	if (cpu->f.pv && !cpu->f.z) {
		blkRepeat(cpu);
	}
}

void blkioRepeat(CPU* cpu) {
	cpu->regPC -= 2;
	cpu->t += 5;
	cpu->regWZ = cpu->regPC + 1;
	cpu->f.f5 = !!(cpu->regPCh & 0x20);
	cpu->f.f3 = !!(cpu->regPCh & 0x08);
	if (cpu->f.c) {
		cpu->f.h = 0;
		// NOTE: act like inc/dec b (look at N flag), H is half-carry, PV changed if
		if (cpu->f.n) {
			if (!parity((cpu->regB - 1) & 7))
				cpu->f.pv ^= 1;
			if ((cpu->regB & 15) == 0)
				cpu->f.h = 1;
		} else {
			if (!parity((cpu->regB + 1) & 7))
				cpu->f.pv ^= 1;
			if ((cpu->regB & 15) == 15)
				cpu->f.h = 1;
		}
	} else if (!parity(cpu->regB & 7)) {
		cpu->f.pv ^= 1;
	}
}

// b2	inir	= ini until regB!=0
void edB2(CPU* cpu) {
	edA2(cpu);
	if (cpu->regB) {
		blkioRepeat(cpu);
	}
}

// b3	otir	= outi until regB!=0
void edB3(CPU* cpu) {
	edA3(cpu);
	if (cpu->regB) {
		blkioRepeat(cpu);
	}
}

// b8	lddr	= ldd until regBC!=0
void edB8(CPU* cpu) {
	edA8(cpu);
	if (cpu->regBC) {
		blkRepeat(cpu);
	}
}

// b9	cpdr	= cpd until (FV & !FZ)
void edB9(CPU* cpu) {
	edA9(cpu);
	if (cpu->f.pv && !cpu->f.z) {
		blkRepeat(cpu);
	}
}

// ba	indr	= ind until regB!=0
void edBA(CPU* cpu) {
	edAA(cpu);
	if (cpu->regB) {
		blkioRepeat(cpu);
	}
}

// bb	otdr	= outd until regB!=0
void edBB(CPU* cpu) {
	edAB(cpu);
	if (cpu->regB) {
		blkioRepeat(cpu);
	}
}

opCode edTab[256]={
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,ed40,NULL,"in b,(c)"},
	{0,4,ed41,NULL,"out (c),b"},
	{0,11,ed42,NULL,"sbc hl,bc"},
	{OF_MWORD | OF_MEMADR,4,ed43,NULL,"ld (:2),bc"},
	{0,4,ed44,NULL,"neg"},
	{0,4,ed45,NULL,"retn"},
	{0,4,ed46,NULL,"im 0"},
	{0,5,ed47,NULL,"ld i,a"},

	{0,4,ed48,NULL,"in c,(c)"},
	{0,4,ed49,NULL,"out (c),c"},
	{0,11,ed4A,NULL,"adc hl,bc"},
	{OF_MWORD | OF_MEMADR,4,ed4B,NULL,"ld bc,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti"},
	{0,4,ed46,NULL,"im 0 *"},
	{0,5,ed4F,NULL,"ld r,a"},

	{0,4,ed50,NULL,"in d,(c)"},
	{0,4,ed51,NULL,"out (c),d"},
	{0,11,ed52,NULL,"sbc hl,de"},
	{OF_MWORD | OF_MEMADR,4,ed53,NULL,"ld (:2),de"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed45,NULL,"retn *"},
	{0,4,ed56,NULL,"im 1"},
	{0,5,ed57,NULL,"ld a,i"},

	{0,4,ed58,NULL,"in e,(c)"},
	{0,4,ed59,NULL,"out (c),e"},
	{0,11,ed5A,NULL,"adc hl,de"},
	{OF_MWORD | OF_MEMADR,4,ed5B,NULL,"ld de,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti *"},
	{0,4,ed5E,NULL,"im 2"},
	{0,5,ed5F,NULL,"ld a,r"},

	{0,4,ed60,NULL,"in h,(c)"},
	{0,4,ed61,NULL,"out (c),h"},
	{0,11,ed62,NULL,"sbc hl,hl"},
	{OF_MWORD | OF_MEMADR,4,npr22,NULL,"ld (:2),hl"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed45,NULL,"retn *"},
	{0,4,ed46,NULL,"im 0 *"},
	{0,4,ed67,NULL,"rrd"},

	{0,4,ed68,NULL,"in l,(c)"},
	{0,4,ed69,NULL,"out (c),l"},
	{0,11,ed6A,NULL,"adc hl,hl"},
	{OF_MWORD | OF_MEMADR,4,npr2A,NULL,"ld hl,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti *"},
	{0,4,ed46,NULL,"im 0 *"},
	{0,4,ed6F,NULL,"rld"},

	{0,4,ed70,NULL,"in (c)"},
	{0,4,ed71,NULL,"out (c),0"},
	{0,11,ed72,NULL,"sbc hl,sp"},
	{OF_MWORD | OF_MEMADR,4,ed73,NULL,"ld (:2),sp"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed45,NULL,"retn *"},
	{0,4,ed56,NULL,"im 1 *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,ed78,NULL,"in a,(c)"},
	{0,4,ed79,NULL,"out (c),a"},
	{0,11,ed7A,NULL,"adc hl,sp"},
	{OF_MWORD | OF_MEMADR,4,ed7B,NULL,"ld sp,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti *"},
	{0,4,ed5E,NULL,"im 2 *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,edA0,NULL,"ldi"},
	{0,4,edA1,NULL,"cpi"},
	{0,5,edA2,NULL,"ini"},
	{0,5,edA3,NULL,"outi"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,edA8,NULL,"ldd"},
	{0,4,edA9,NULL,"cpd"},
	{0,5,edAA,NULL,"ind"},
	{0,5,edAB,NULL,"outd"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{OF_SKIPABLE,4,edB0,NULL,"ldir"},
	{OF_SKIPABLE,4,edB1,NULL,"cpir"},
	{OF_SKIPABLE,5,edB2,NULL,"inir"},
	{OF_SKIPABLE,5,edB3,NULL,"otir"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{OF_SKIPABLE,4,edB8,NULL,"lddr"},
	{OF_SKIPABLE,4,edB9,NULL,"cpdr"},
	{OF_SKIPABLE,5,edBA,NULL,"indr"},
	{OF_SKIPABLE,5,edBB,NULL,"otdr"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
};
