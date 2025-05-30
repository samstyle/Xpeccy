#include "i8080.h"

#include <stdio.h>

// extern const unsigned char sz53pTab[0x100];

static int iop_add_h[8] = {0, 0, 1, 0, 1, 0, 1, 1};
static int iop_sub_h[8] = {1, 0, 0, 0, 1, 1, 1, 0};

// common

unsigned char iop_inr(CPU* cpu, unsigned char val) {
	val++;
	//cpu->f &= IFL_C;
	cpu->fi.s = !!(val & 0x80);
	cpu->fi.z = !val;
	cpu->fi.a = !!((val & 0x0f) == 0);
	cpu->fi.p = parity(val);
	return val;
}

unsigned char iop_dcr(CPU* cpu, unsigned char val) {
	val--;
	//cpu->f &= IFL_C;
	cpu->fi.s = !!(val & 0x80);
	cpu->fi.z = !val;
	cpu->fi.a = !!((val & 0x0f) != 0x0f);	// A flag is inverted (1:no b4-carry, 0:b4-carry)
	cpu->fi.p = parity(val);
	return val;
}

unsigned char iop_add(CPU* cpu, unsigned char val, unsigned char add) {
	cpu->tmpw = val + add;
	cpu->f = 0;
	cpu->fi.s = !!(cpu->ltw & 0x80);
	cpu->fi.z = !cpu->ltw;
	cpu->fi.a = !!iop_add_h[((val & 8) >> 1) | ((add & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->fi.p = parity(cpu->ltw);
	cpu->fi.c = !!cpu->htw;
	return cpu->ltw;
}

unsigned char iop_adc(CPU* cpu, unsigned char val, unsigned char add) {
	cpu->tmpw = val + add + cpu->fi.c;
	//cpu->f = 0;
	cpu->fi.s = !!(cpu->ltw & 0x80);
	cpu->fi.z = !cpu->ltw;
	cpu->fi.a = !!iop_add_h[((val & 8) >> 1) | ((add & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->fi.p = parity(cpu->ltw);
	cpu->fi.c = !!cpu->htw;
	return cpu->ltw;
}

unsigned char iop_sub(CPU* cpu, unsigned char val, unsigned char sub) {
	cpu->tmpw = val - sub;
	//cpu->f = 0;
	cpu->fi.s = !!(cpu->ltw & 0x80);
	cpu->fi.z = !cpu->ltw;
	cpu->fi.a = !!iop_sub_h[((val & 8) >> 1) | ((sub & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->fi.p = parity(cpu->ltw);
	cpu->fi.c = !!cpu->htw;
	return cpu->ltw;
}

unsigned char iop_sbb(CPU* cpu, unsigned char val, unsigned char sub) {
	cpu->tmpw = val - sub - cpu->fi.c;
	//cpu->f = 0;
	cpu->fi.s = !!(cpu->ltw & 0x80);
	cpu->fi.z = !cpu->ltw;
	cpu->fi.a = !!iop_sub_h[((val & 8) >> 1) | ((sub & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->fi.p = parity(cpu->ltw);
	cpu->fi.c = !!cpu->htw;
	return cpu->ltw;
}

unsigned char iop_ana(CPU* cpu, unsigned char val, unsigned char arg) {
	cpu->fi.a = !!((val | arg) & 0x08);	// from sPycialist
	val &= arg;
	cpu->fi.c = !!(val & 0x80);
	cpu->fi.z = !val;
	cpu->fi.p = parity(val);
	cpu->fi.c = 0;
	return  val;
}

unsigned char iop_xra(CPU* cpu, unsigned char val, unsigned char arg) {
	val ^= arg;
	// cpu->f = 0;
	cpu->fi.s = !!(val & 0x80);
	cpu->fi.z = !val;
	cpu->fi.c = parity(val);
	cpu->fi.a = 0;
	cpu->fi.c = 0;
	return val;
}

unsigned char iop_ora(CPU* cpu, unsigned char val, unsigned char arg) {
	val |= arg;
	// cpu->f = 0;
	cpu->fi.s = !!(val & 0x80);
	cpu->fi.z = !val;
	cpu->fi.p = parity(val);
	cpu->fi.a = 0;
	cpu->fi.c = 0;
	return val;
}

unsigned short iop_pop(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regSP++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regSP++, 0, cpu->xptr) & 0xff;
	return cpu->regWZ;
}

void iop_push(CPU* cpu, unsigned short val) {
	cpu->regWZ = val;
	cpu->t += 3;
	cpu->mwr(--cpu->regSP, cpu->regWZh, cpu->xptr);
	cpu->t += 3;
	cpu->mwr(--cpu->regSP, cpu->regWZl, cpu->xptr);
}

// 00, 08, 10, 18, 20, 28 : nop
void iop_00(CPU* cpu) {}

// 01:lxi regB,nn
void iop_01(CPU* cpu) {
	cpu->t += 3;
	cpu->regC = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regB = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 02:stax b
void iop_02(CPU* cpu) {
	cpu->t += 3;
	cpu->mwr(cpu->regBC, cpu->regA, cpu->xptr);
}

// 03:inx b
void iop_03(CPU* cpu) {
	cpu->regBC++;
}

// 04:inr regB
void iop_04(CPU* cpu) {
	cpu->regB = iop_inr(cpu, cpu->regB);
}

// 05:dcr regB
void iop_05(CPU* cpu) {
	cpu->regB = iop_dcr(cpu, cpu->regB);
}

// 06:mvi regB,n
void iop_06(CPU* cpu) {
	cpu->t += 3;
	cpu->regB = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 07:rlc
void iop_07(CPU* cpu) {
	//cpu->f &= ~IFL_C;
	cpu->fi.c = !!(cpu->regA & 0x80);
	cpu->regA <<= 1;
	if (cpu->fi.c) cpu->regA |= 1;
}

// 08:nop*
// 09:dad b
void iop_09(CPU* cpu) {
	cpu->tmpi = cpu->regBC + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	//cpu->f &= ~IFL_C;
	cpu->fi.c = !!(cpu->tmpi > 0xffff);
}

// 0a:ldax b
void iop_0a(CPU* cpu) {
	cpu->t += 3;
	cpu->regA = cpu->mrd(cpu->regBC, 0, cpu->xptr) & 0xff;
}

// 0b:dcx b
void iop_0b(CPU* cpu) {
	cpu->regBC--;
}

// 0c:inr regC
void iop_0c(CPU* cpu) {
	cpu->regC = iop_inr(cpu, cpu->regC);
}

// 0d:dcr regC
void iop_0d(CPU* cpu) {
	cpu->regC = iop_dcr(cpu, cpu->regC);
}

// 0e:mvi regC,n
void iop_0e(CPU* cpu) {
	cpu->t += 3;
	cpu->regC = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 0f:rrc
void iop_0f(CPU* cpu) {
	//cpu->f &= ~IFL_C;
	cpu->fi.c = cpu->regA & 0x01;
	cpu->regA >>= 1;
	if (cpu->fi.c) cpu->regA |= 0x80;
}

// 11:lxi regD,nn
void iop_11(CPU* cpu) {
	cpu->t += 3;
	cpu->regE = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regD = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 12:stax d
void iop_12(CPU* cpu) {
	cpu->t += 3;
	cpu->mwr(cpu->regDE, cpu->regA, cpu->xptr);
}

// 13:inx d
void iop_13(CPU* cpu) {
	cpu->regDE++;
}

// 14:inr regD
void iop_14(CPU* cpu) {
	cpu->regD = iop_inr(cpu, cpu->regD);
}

// 15:dcr regD
void iop_15(CPU* cpu) {
	cpu->regD = iop_dcr(cpu, cpu->regD);
}

// 16:mvi regD,n
void iop_16(CPU* cpu) {
	cpu->t += 3;
	cpu->regD = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 17:ral
void iop_17(CPU* cpu) {
	cpu->ltw = cpu->regA;
	cpu->tmpw <<= 1;
	if (cpu->fi.c) cpu->ltw |= 1;
	cpu->fi.c = cpu->htw & 1;
	cpu->regA = cpu->ltw;
}

// 19:dad d
void iop_19(CPU* cpu) {
	cpu->tmpi = cpu->regDE + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	//cpu->f &= ~IFL_C;
	cpu->fi.c = !!(cpu->tmpi > 0xffff);
}

// 1a:ldax d
void iop_1a(CPU* cpu) {
	cpu->t += 3;
	cpu->regA = cpu->mrd(cpu->regDE, 0, cpu->xptr) & 0xff;
}

// 1b:dcx d
void iop_1b(CPU* cpu) {
	cpu->regDE--;
}

// 1c:inr regE
void iop_1c(CPU* cpu) {
	cpu->regE = iop_inr(cpu, cpu->regE);
}

// 1d:dcr regE
void iop_1d(CPU* cpu) {
	cpu->regE = iop_dcr(cpu, cpu->regE);
}

// 1e:mvi regE,n
void iop_1e(CPU* cpu) {
	cpu->t += 3;
	cpu->regE = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 1f:rar
void iop_1f(CPU* cpu) {
	cpu->htw = cpu->regA;
	cpu->tmpw >>= 1;
	if (cpu->fi.c) cpu->htw |= 0x80;
	cpu->fi.c = !!(cpu->ltw & 0x80);
	cpu->regA = cpu->htw;
}

// 21:lxi regH,nn
void iop_21(CPU* cpu) {
	cpu->t += 3;
	cpu->regL = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regH = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 22:shld nn
void iop_22(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->mwr(cpu->regWZ++, cpu->regL, cpu->xptr);
	cpu->t += 3;
	cpu->mwr(cpu->regWZ, cpu->regH, cpu->xptr);
}


// 23:inx h
void iop_23(CPU* cpu) {
	cpu->regHL++;
}

// 24:inr regH
void iop_24(CPU* cpu) {
	cpu->regH = iop_inr(cpu, cpu->regH);
}

// 25:dcr regH
void iop_25(CPU* cpu) {
	cpu->regH = iop_dcr(cpu, cpu->regH);
}

// 26:mvi regH,n
void iop_26(CPU* cpu) {
	cpu->t += 3;
	cpu->regH = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 27:daa
void iop_27(CPU* cpu) {
	unsigned char add = 0;
	unsigned char cf = cpu->fi.c;
	if ((cpu->fi.a) || ((cpu->regA & 0x0f) > 0x09))
		add = 6;
	if ((cpu->fi.c) || (cpu->regA > 0x9f) || ((cpu->regA > 0x8f) && ((cpu->regA & 0x0f) > 0x09)))
		add |= 0x60;
	if (cpu->regA > 0x99)
		cf = 1;
	cpu->regA = iop_add(cpu, cpu->regA, add);
	cpu->fi.c = cf; // = (cpu->f & ~IFL_C) | cf;
}

// 29:dad h
void iop_29(CPU* cpu) {
	cpu->tmpi = cpu->regHL + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	//cpu->f &= ~IFL_C;
	cpu->fi.c = !!(cpu->tmpi > 0xffff);
}

// 2a:lhld nn
void iop_2a(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regL = cpu->mrd(cpu->regWZ++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regH = cpu->mrd(cpu->regWZ, 0, cpu->xptr) & 0xff;
}

// 2b:dcx h
void iop_2b(CPU* cpu) {
	cpu->regHL--;
}

// 2c:inr regL
void iop_2c(CPU* cpu) {
	cpu->regL = iop_inr(cpu, cpu->regL);
}

// 2d:dcr regL
void iop_2d(CPU* cpu) {
	cpu->regL = iop_dcr(cpu, cpu->regL);
}

// 2e:mvi regL,n
void iop_2e(CPU* cpu) {
	cpu->t += 3;
	cpu->regL = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 2f:cma
void iop_2f(CPU* cpu) {
	cpu->regA ^= 0xff;
}

// 31:lxi sp,nn
void iop_31(CPU* cpu) {
	cpu->t += 3;
	cpu->regSPl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regSPh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 32:sta nn
void iop_32(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

// 33:inx regSP
void iop_33(CPU* cpu) {
	cpu->regSP++;
}

// 34:inr m
void iop_34(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->tmpb = iop_inr(cpu, cpu->tmpb);
	cpu->t += 3;
	cpu->mwr(cpu->regHL, cpu->tmpb, cpu->xptr);
}

// 35:dcr m
void iop_35(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->tmpb = iop_dcr(cpu, cpu->tmpb);
	cpu->t += 3;
	cpu->mwr(cpu->regHL, cpu->tmpb, cpu->xptr);
}

// 36:mvi m,n
void iop_36(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->mwr(cpu->regHL, cpu->tmpb, cpu->xptr);
}

// 37:stc
void iop_37(CPU* cpu) {
	cpu->fi.c = 1;
}

// 39:dad regSP
void iop_39(CPU* cpu) {
	cpu->tmpi = cpu->regSP + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	// cpu->f &= ~IFL_C;
	cpu->fi.c = !!(cpu->tmpi > 0xffff);
}

// 3a:lda nn
void iop_3a(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regA = cpu->mrd(cpu->regWZ, 0, cpu->xptr) & 0xff;
}

// 3b:dcx regSP
void iop_3b(CPU* cpu) {
	cpu->regSP--;
}

// 3c:inr regA
void iop_3c(CPU* cpu) {
	cpu->regA = iop_inr(cpu, cpu->regA);
}

// 3d:dcr regA
void iop_3d(CPU* cpu) {
	cpu->regA = iop_dcr(cpu, cpu->regA);
}

// 3e:mvi regA,n
void iop_3e(CPU* cpu) {
	cpu->t += 3;
	cpu->regA = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
}

// 3f:cmc
void iop_3f(CPU* cpu) {
	cpu->fi.c ^= 1;
}

// 40..47: mov regB,x
void iop_40(CPU* cpu) {cpu->regB = cpu->regB;}
void iop_41(CPU* cpu) {cpu->regB = cpu->regC;}
void iop_42(CPU* cpu) {cpu->regB = cpu->regD;}
void iop_43(CPU* cpu) {cpu->regB = cpu->regE;}
void iop_44(CPU* cpu) {cpu->regB = cpu->regH;}
void iop_45(CPU* cpu) {cpu->regB = cpu->regL;}
void iop_46(CPU* cpu) {cpu->t += 3; cpu->regB = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_47(CPU* cpu) {cpu->regB = cpu->regA;}
// 48..4f: mov regC,x
void iop_48(CPU* cpu) {cpu->regC = cpu->regB;}
void iop_49(CPU* cpu) {cpu->regC = cpu->regC;}
void iop_4a(CPU* cpu) {cpu->regC = cpu->regD;}
void iop_4b(CPU* cpu) {cpu->regC = cpu->regE;}
void iop_4c(CPU* cpu) {cpu->regC = cpu->regH;}
void iop_4d(CPU* cpu) {cpu->regC = cpu->regL;}
void iop_4e(CPU* cpu) {cpu->t += 3; cpu->regC = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_4f(CPU* cpu) {cpu->regC = cpu->regA;}
// 50..57: mov regD,x
void iop_50(CPU* cpu) {cpu->regD = cpu->regB;}
void iop_51(CPU* cpu) {cpu->regD = cpu->regC;}
void iop_52(CPU* cpu) {cpu->regD = cpu->regD;}
void iop_53(CPU* cpu) {cpu->regD = cpu->regE;}
void iop_54(CPU* cpu) {cpu->regD = cpu->regH;}
void iop_55(CPU* cpu) {cpu->regD = cpu->regL;}
void iop_56(CPU* cpu) {cpu->t += 3; cpu->regD = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_57(CPU* cpu) {cpu->regD = cpu->regA;}
// 58..5f: mov regE,x
void iop_58(CPU* cpu) {cpu->regE = cpu->regB;}
void iop_59(CPU* cpu) {cpu->regE = cpu->regC;}
void iop_5a(CPU* cpu) {cpu->regE = cpu->regD;}
void iop_5b(CPU* cpu) {cpu->regE = cpu->regE;}
void iop_5c(CPU* cpu) {cpu->regE = cpu->regH;}
void iop_5d(CPU* cpu) {cpu->regE = cpu->regL;}
void iop_5e(CPU* cpu) {cpu->t += 3; cpu->regE = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_5f(CPU* cpu) {cpu->regE = cpu->regA;}
// 60..67: mov regH,x
void iop_60(CPU* cpu) {cpu->regH = cpu->regB;}
void iop_61(CPU* cpu) {cpu->regH = cpu->regC;}
void iop_62(CPU* cpu) {cpu->regH = cpu->regD;}
void iop_63(CPU* cpu) {cpu->regH = cpu->regE;}
void iop_64(CPU* cpu) {cpu->regH = cpu->regH;}
void iop_65(CPU* cpu) {cpu->regH = cpu->regL;}
void iop_66(CPU* cpu) {cpu->t += 3; cpu->regH = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_67(CPU* cpu) {cpu->regH = cpu->regA;}
// 68..6f: mov regL,x
void iop_68(CPU* cpu) {cpu->regL = cpu->regB;}
void iop_69(CPU* cpu) {cpu->regL = cpu->regC;}
void iop_6a(CPU* cpu) {cpu->regL = cpu->regD;}
void iop_6b(CPU* cpu) {cpu->regL = cpu->regE;}
void iop_6c(CPU* cpu) {cpu->regL = cpu->regH;}
void iop_6d(CPU* cpu) {cpu->regL = cpu->regL;}
void iop_6e(CPU* cpu) {cpu->t += 3; cpu->regL = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_6f(CPU* cpu) {cpu->regL = cpu->regA;}
// 70..77: mov m,x
void iop_70(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regB, cpu->xptr);}
void iop_71(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regC, cpu->xptr);}
void iop_72(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regD, cpu->xptr);}
void iop_73(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regE, cpu->xptr);}
void iop_74(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regH, cpu->xptr);}
void iop_75(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regL, cpu->xptr);}
void iop_76(CPU* cpu) {cpu->regPC--; cpu->halt = 1;}
void iop_77(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->regHL, cpu->regA, cpu->xptr);}
// 78..7f: mov regA,x
void iop_78(CPU* cpu) {cpu->regA = cpu->regB;}
void iop_79(CPU* cpu) {cpu->regA = cpu->regC;}
void iop_7a(CPU* cpu) {cpu->regA = cpu->regD;}
void iop_7b(CPU* cpu) {cpu->regA = cpu->regE;}
void iop_7c(CPU* cpu) {cpu->regA = cpu->regH;}
void iop_7d(CPU* cpu) {cpu->regA = cpu->regL;}
void iop_7e(CPU* cpu) {cpu->t += 3; cpu->regA = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;}
void iop_7f(CPU* cpu) {cpu->regA = cpu->regA;}
// 80..87: add x
void iop_80(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regB);}
void iop_81(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regC);}
void iop_82(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regD);}
void iop_83(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regE);}
void iop_84(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regH);}
void iop_85(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regL);}
void iop_86(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_add(cpu, cpu->regA, cpu->tmpb);
}
void iop_87(CPU* cpu) {cpu->regA = iop_add(cpu, cpu->regA, cpu->regA);}
// 88..8f: adc x
void iop_88(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regB);}
void iop_89(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regC);}
void iop_8a(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regD);}
void iop_8b(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regE);}
void iop_8c(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regH);}
void iop_8d(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regL);}
void iop_8e(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_adc(cpu, cpu->regA, cpu->tmpb);
}
void iop_8f(CPU* cpu) {cpu->regA = iop_adc(cpu, cpu->regA, cpu->regA);}
// 90..97: sub x
void iop_90(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regB);}
void iop_91(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regC);}
void iop_92(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regD);}
void iop_93(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regE);}
void iop_94(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regH);}
void iop_95(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regL);}
void iop_96(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_sub(cpu, cpu->regA, cpu->tmpb);
}
void iop_97(CPU* cpu) {cpu->regA = iop_sub(cpu, cpu->regA, cpu->regA);}
// 98..9f: sbb x
void iop_98(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regB);}
void iop_99(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regC);}
void iop_9a(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regD);}
void iop_9b(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regE);}
void iop_9c(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regH);}
void iop_9d(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regL);}
void iop_9e(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_sbb(cpu, cpu->regA, cpu->tmpb);
}
void iop_9f(CPU* cpu) {cpu->regA = iop_sbb(cpu, cpu->regA, cpu->regA);}
// a0..a7: ana x
void iop_a0(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regB);}
void iop_a1(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regC);}
void iop_a2(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regD);}
void iop_a3(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regE);}
void iop_a4(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regH);}
void iop_a5(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regL);}
void iop_a6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_ana(cpu, cpu->regA, cpu->tmpb);
}
void iop_a7(CPU* cpu) {cpu->regA = iop_ana(cpu, cpu->regA, cpu->regA);}
// a8..af: xra x
void iop_a8(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regB);}
void iop_a9(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regC);}
void iop_aa(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regD);}
void iop_ab(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regE);}
void iop_ac(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regH);}
void iop_ad(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regL);}
void iop_ae(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_xra(cpu, cpu->regA, cpu->tmpb);
}
void iop_af(CPU* cpu) {cpu->regA = iop_xra(cpu, cpu->regA, cpu->regA);}
// b0..b7: ora x
void iop_b0(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regB);}
void iop_b1(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regC);}
void iop_b2(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regD);}
void iop_b3(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regE);}
void iop_b4(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regH);}
void iop_b5(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regL);}
void iop_b6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_ora(cpu, cpu->regA, cpu->tmpb);
}
void iop_b7(CPU* cpu) {cpu->regA = iop_ora(cpu, cpu->regA, cpu->regA);}
// b8..bf: cmp x
void iop_b8(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regB);}
void iop_b9(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regC);}
void iop_ba(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regD);}
void iop_bb(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regE);}
void iop_bc(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regH);}
void iop_bd(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regL);}
void iop_be(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regHL, 0, cpu->xptr) & 0xff;
	cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->tmpb);
}
void iop_bf(CPU* cpu) {cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->regA);}
// c0: rnz
void iop_c0(CPU* cpu) {if (!cpu->fi.z) cpu->regPC = iop_pop(cpu);}
// c1: pop b
void iop_c1(CPU* cpu) {cpu->regBC = iop_pop(cpu);}
// c2: jnz nn
void iop_c2(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.z) cpu->regPC = cpu->regWZ;
}
// c3: jmp nn
void iop_c3(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regPC = cpu->regWZ;
}
// c4: cnz nn
void iop_c4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.z) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// c5: push b
void iop_c5(CPU* cpu) {iop_push(cpu, cpu->regBC);}
// c6: adi n
void iop_c6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_add(cpu, cpu->regA, cpu->tmpb);
}
// c7/cf/d7/df/e7/ef/f7/ff: rst n
void iop_rst(CPU* cpu) {
	iop_push(cpu, cpu->regPC);
	cpu->regPC = cpu->com & 0x38;
}
// c8: rz
void iop_c8(CPU* cpu) {if (cpu->fi.z) cpu->regPC = iop_pop(cpu);}
// c9: ret
void iop_c9(CPU* cpu) {cpu->regPC = iop_pop(cpu);}
// ca: jz nn
void iop_ca(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.z) cpu->regPC = cpu->regWZ;
}
// cb = c3
// cc: cz nn
void iop_cc(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.z) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// cd: call nn
void iop_cd(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	iop_push(cpu, cpu->regPC);
	cpu->regPC = cpu->tmpw;
}
// ce: aci n
void iop_ce(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_adc(cpu, cpu->regA, cpu->tmpb);
}
// cf: rst 1
// d0: rnc
void iop_d0(CPU* cpu) {if (!cpu->fi.c) cpu->regPC = iop_pop(cpu);}
// d1: pop d
void iop_d1(CPU* cpu) {cpu->regDE = iop_pop(cpu);}
// d2: jnc nn
void iop_d2(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.c) cpu->regPC = cpu->regWZ;
}
// d3: out n
void iop_d3(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->htw = cpu->ltw;	// A8..15 = A0..7
	cpu->t += 3;
	cpu->iwr(cpu->tmpw, cpu->regA, cpu->xptr);
}
// d4: cnc nn
void iop_d4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.c) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// d5: push d
void iop_d5(CPU* cpu) {iop_push(cpu, cpu->regDE);}
// d6: sui n
void iop_d6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_sub(cpu, cpu->regA, cpu->tmpb);
}
// d7: rst 2
// d8: rc
void iop_d8(CPU* cpu) {if (cpu->fi.c) cpu->regPC = iop_pop(cpu);}
// d9: ret*
// da: jc nn
void iop_da(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.c) cpu->regPC = cpu->regWZ;
}
// db: in n
void iop_db(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->htw = cpu->regA;
	cpu->t += 3;
	cpu->regA = cpu->ird(cpu->tmpw, cpu->xptr) & 0xff;
}
// dc: cc nn
void iop_dc(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.c) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// dd: *call nn
// de: sbi n
void iop_de(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_sbb(cpu, cpu->regA, cpu->tmpb);
}
// df: rst 3
// e0: rpo
void iop_e0(CPU* cpu) {if (!cpu->fi.p) cpu->regPC = iop_pop(cpu);}
// e1: pop h
void iop_e1(CPU* cpu) {cpu->regHL = iop_pop(cpu);}
// e2: jpo nn
void iop_e2(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.p) cpu->regPC = cpu->regWZ;
}
// e3: xthl = ex (sp),regHL
void iop_e3(CPU* cpu) {
	cpu->tmpw = iop_pop(cpu);
	iop_push(cpu, cpu->regHL);
	cpu->t += 2;
	cpu->regHL = cpu->tmpw;
}
// e4: cpo nn
void iop_e4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.p) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// e5: push h
void iop_e5(CPU* cpu) {iop_push(cpu, cpu->regHL);}
// e6: ani n
void iop_e6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_ana(cpu, cpu->regA, cpu->tmpb);
}
// e7: rst 4
// e8: rpe
void iop_e8(CPU* cpu) {if (cpu->fi.p) cpu->regPC = iop_pop(cpu);}
// e9: pchl
void iop_e9(CPU* cpu) {cpu->regPC = cpu->regHL;}
// ea: jpe nn
void iop_ea(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.p) cpu->regPC = cpu->regWZ;
}
// eb: xchg
void iop_eb(CPU* cpu) {
	cpu->tmpw = cpu->regDE;
	cpu->regDE = cpu->regHL;
	cpu->regHL = cpu->tmpw;
}
// ec: cpe nn
void iop_ec(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.p) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// ed: *call nn
// ee: xri n
void iop_ee(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_xra(cpu, cpu->regA, cpu->tmpb);
}
// ef: rst 5
// f0: rp
void iop_f0(CPU* cpu) {if (!cpu->fi.s) cpu->regPC = iop_pop(cpu);}
// f1: pop psw
void iop_f1(CPU* cpu) {
	cpu->tmpw = iop_pop(cpu);
	cpu->regA = cpu->htw;
	cpu->f = cpu->ltw;
}
// f2: jp nn
void iop_f2(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.s) {
		cpu->regPC = cpu->tmpw;
	}
}
// f3: di
void iop_f3(CPU* cpu) {
	cpu->iff1 = 0;
	cpu->inten &= ~I8080_INT;
}
// f4: cp nn
void iop_f4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (!cpu->fi.s) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// f5: push psw
void iop_f5(CPU* cpu) {
	cpu->htw = cpu->regA;
	cpu->ltw = cpu->f & 0xff;;
	iop_push(cpu, cpu->tmpw);
}

// f6: ori n
void iop_f6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->regA = iop_ora(cpu, cpu->regA, cpu->tmpb);
}
// f7: rst 6
// f8: rm
void iop_f8(CPU* cpu) {if (cpu->fi.s) cpu->regPC = iop_pop(cpu);}
// f9: sphl
void iop_f9(CPU* cpu) {
	cpu->regSP = cpu->regHL;
}
// fa: jm nn
void iop_fa(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.s) cpu->regPC = cpu->regWZ;
}
// fb: ei
void iop_fb(CPU* cpu) {
	cpu->iff1 = 1;
	cpu->inten |= I8080_INT;
}
// fc: cm nn
void iop_fc(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->t += 3;
	cpu->htw = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	if (cpu->fi.s) {
		iop_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// fd: *call nn
// fe: cpi n
void iop_fe(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->regPC++, 0, cpu->xptr) & 0xff;
	cpu->tmpb = iop_sub(cpu, cpu->regA, cpu->tmpb);
}
// ff: rst 7

opCode i8080_tab[256] = {
	{0, 4, iop_00, 0, "nop"},
	{0, 4, iop_01, 0, "lxi b,:2"},
	{0, 4, iop_02, 0, "stax b"},
	{0, 5, iop_03, 0, "inx b"},
	{0, 5, iop_04, 0, "inr b"},
	{0, 5, iop_05, 0, "dcr b"},
	{0, 4, iop_06, 0, "mvi b,:1"},
	{0, 4, iop_07, 0, "rlc"},

	{0, 4, iop_00, 0, "nop"},
	{0, 10, iop_09, 0, "dad b"},
	{0, 4, iop_0a, 0, "ldax b"},
	{0, 5, iop_0b, 0, "dcx b"},
	{0, 5, iop_0c, 0, "inr c"},
	{0, 5, iop_0d, 0, "dcr c"},
	{0, 4, iop_0e, 0, "mvi c,:1"},
	{0, 4, iop_0f, 0, "rrc"},

	{0, 4, iop_00, 0, "nop"},
	{0, 4, iop_11, 0, "lxi d,:2"},
	{0, 4, iop_12, 0, "stax d"},
	{0, 5, iop_13, 0, "inx d"},
	{0, 5, iop_14, 0, "inr d"},
	{0, 5, iop_15, 0, "dcr d"},
	{0, 4, iop_16, 0, "mvi d,:1"},
	{0, 4, iop_17, 0, "ral"},

	{0, 4, iop_00, 0, "nop"},
	{0, 10, iop_19, 0, "dad d"},
	{0, 4, iop_1a, 0, "ldax d"},
	{0, 5, iop_1b, 0, "dcx d"},
	{0, 5, iop_1c, 0, "inr e"},
	{0, 5, iop_1d, 0, "dcr e"},
	{0, 4, iop_1e, 0, "mvi e,:1"},
	{0, 4, iop_1f, 0, "rar"},

	{0, 4, iop_00, 0, "nop"},
	{0, 4, iop_21, 0, "lxi h,:2"},
	{0, 4, iop_22, 0, "shld :2"},
	{0, 5, iop_23, 0, "inx h"},
	{0, 5, iop_24, 0, "inr h"},
	{0, 5, iop_25, 0, "dcr h"},
	{0, 4, iop_26, 0, "mvi h,:1"},
	{0, 4, iop_27, 0, "daa"},

	{0, 4, iop_00, 0, "nop"},
	{0, 10, iop_29, 0, "dad h"},
	{0, 4, iop_2a, 0, "lhld :2"},
	{0, 5, iop_2b, 0, "dcx h"},
	{0, 5, iop_2c, 0, "inr l"},
	{0, 5, iop_2d, 0, "dcr l"},
	{0, 4, iop_2e, 0, "mvi l,:1"},
	{0, 4, iop_2f, 0, "cma"},

	{0, 4, iop_00, 0, "nop"},
	{0, 4, iop_31, 0, "lxi sp,:2"},
	{0, 4, iop_32, 0, "sta :2"},
	{0, 5, iop_33, 0, "inx sp"},
	{0, 4, iop_34, 0, "inr m"},
	{0, 4, iop_35, 0, "dcr m"},
	{0, 4, iop_36, 0, "mvi m,:1"},
	{0, 4, iop_37, 0, "stc"},

	{0, 4, iop_00, 0, "nop"},
	{0, 10, iop_39, 0, "dad sp"},
	{0, 4, iop_3a, 0, "lda :2"},
	{0, 5, iop_3b, 0, "dcx sp"},
	{0, 5, iop_3c, 0, "inr a"},
	{0, 5, iop_3d, 0, "dcr a"},
	{0, 4, iop_3e, 0, "mvi a,:1"},
	{0, 4, iop_3f, 0, "cmc"},

	{0, 5, iop_40, 0, "mov b,b"},
	{0, 5, iop_41, 0, "mov b,c"},
	{0, 5, iop_42, 0, "mov b,d"},
	{0, 5, iop_43, 0, "mov b,e"},
	{0, 5, iop_44, 0, "mov b,h"},
	{0, 5, iop_45, 0, "mov b,l"},
	{0, 4, iop_46, 0, "mov b,m"},
	{0, 5, iop_47, 0, "mov b,a"},

	{0, 5, iop_48, 0, "mov c,b"},
	{0, 5, iop_49, 0, "mov c,c"},
	{0, 5, iop_4a, 0, "mov c,d"},
	{0, 5, iop_4b, 0, "mov c,e"},
	{0, 5, iop_4c, 0, "mov c,h"},
	{0, 5, iop_4d, 0, "mov c,l"},
	{0, 4, iop_4e, 0, "mov c,m"},
	{0, 5, iop_4f, 0, "mov c,a"},

	{0, 5, iop_50, 0, "mov d,b"},
	{0, 5, iop_51, 0, "mov d,c"},
	{0, 5, iop_52, 0, "mov d,d"},
	{0, 5, iop_53, 0, "mov d,e"},
	{0, 5, iop_54, 0, "mov d,h"},
	{0, 5, iop_55, 0, "mov d,l"},
	{0, 4, iop_56, 0, "mov d,m"},
	{0, 5, iop_57, 0, "mov d,a"},

	{0, 5, iop_58, 0, "mov e,b"},
	{0, 5, iop_59, 0, "mov e,c"},
	{0, 5, iop_5a, 0, "mov e,d"},
	{0, 5, iop_5b, 0, "mov e,e"},
	{0, 5, iop_5c, 0, "mov e,h"},
	{0, 5, iop_5d, 0, "mov e,l"},
	{0, 4, iop_5e, 0, "mov e,m"},
	{0, 5, iop_5f, 0, "mov e,a"},

	{0, 5, iop_60, 0, "mov h,b"},
	{0, 5, iop_61, 0, "mov h,c"},
	{0, 5, iop_62, 0, "mov h,d"},
	{0, 5, iop_63, 0, "mov h,e"},
	{0, 5, iop_64, 0, "mov h,h"},
	{0, 5, iop_65, 0, "mov h,l"},
	{0, 4, iop_66, 0, "mov h,m"},
	{0, 5, iop_67, 0, "mov h,a"},

	{0, 5, iop_68, 0, "mov l,b"},
	{0, 5, iop_69, 0, "mov l,c"},
	{0, 5, iop_6a, 0, "mov l,d"},
	{0, 5, iop_6b, 0, "mov l,e"},
	{0, 5, iop_6c, 0, "mov l,h"},
	{0, 5, iop_6d, 0, "mov l,l"},
	{0, 4, iop_6e, 0, "mov l,m"},
	{0, 5, iop_6f, 0, "mov l,a"},

	{0, 4, iop_70, 0, "mov m,b"},
	{0, 4, iop_71, 0, "mov m,c"},
	{0, 4, iop_72, 0, "mov m,d"},
	{0, 4, iop_73, 0, "mov m,e"},
	{0, 4, iop_74, 0, "mov m,h"},
	{0, 4, iop_75, 0, "mov m,l"},
	{0, 7, iop_76, 0, "hlt"},
	{0, 4, iop_77, 0, "mov m,a"},

	{0, 5, iop_78, 0, "mov a,b"},
	{0, 5, iop_79, 0, "mov a,c"},
	{0, 5, iop_7a, 0, "mov a,d"},
	{0, 5, iop_7b, 0, "mov a,e"},
	{0, 5, iop_7c, 0, "mov a,h"},
	{0, 5, iop_7d, 0, "mov a,l"},
	{0, 4, iop_7e, 0, "mov a,m"},
	{0, 5, iop_7f, 0, "mov a,a"},

	{0, 4, iop_80, 0, "add b"},
	{0, 4, iop_81, 0, "add c"},
	{0, 4, iop_82, 0, "add d"},
	{0, 4, iop_83, 0, "add e"},
	{0, 4, iop_84, 0, "add h"},
	{0, 4, iop_85, 0, "add l"},
	{0, 4, iop_86, 0, "add m"},
	{0, 4, iop_87, 0, "add a"},

	{0, 4, iop_88, 0, "adc b"},
	{0, 4, iop_89, 0, "adc c"},
	{0, 4, iop_8a, 0, "adc d"},
	{0, 4, iop_8b, 0, "adc e"},
	{0, 4, iop_8c, 0, "adc h"},
	{0, 4, iop_8d, 0, "adc l"},
	{0, 4, iop_8e, 0, "adc m"},
	{0, 4, iop_8f, 0, "adc a"},

	{0, 4, iop_90, 0, "sub b"},
	{0, 4, iop_91, 0, "sub c"},
	{0, 4, iop_92, 0, "sub d"},
	{0, 4, iop_93, 0, "sub e"},
	{0, 4, iop_94, 0, "sub h"},
	{0, 4, iop_95, 0, "sub l"},
	{0, 4, iop_96, 0, "sub m"},
	{0, 4, iop_97, 0, "sub a"},

	{0, 4, iop_98, 0, "sbb b"},
	{0, 4, iop_99, 0, "sbb c"},
	{0, 4, iop_9a, 0, "sbb d"},
	{0, 4, iop_9b, 0, "sbb e"},
	{0, 4, iop_9c, 0, "sbb h"},
	{0, 4, iop_9d, 0, "sbb l"},
	{0, 4, iop_9e, 0, "sbb m"},
	{0, 4, iop_9f, 0, "sbb a"},

	{0, 4, iop_a0, 0, "ana b"},
	{0, 4, iop_a1, 0, "ana c"},
	{0, 4, iop_a2, 0, "ana d"},
	{0, 4, iop_a3, 0, "ana e"},
	{0, 4, iop_a4, 0, "ana h"},
	{0, 4, iop_a5, 0, "ana l"},
	{0, 4, iop_a6, 0, "ana m"},
	{0, 4, iop_a7, 0, "ana a"},

	{0, 4, iop_a8, 0, "xra b"},
	{0, 4, iop_a9, 0, "xra c"},
	{0, 4, iop_aa, 0, "xra d"},
	{0, 4, iop_ab, 0, "xra e"},
	{0, 4, iop_ac, 0, "xra h"},
	{0, 4, iop_ad, 0, "xra l"},
	{0, 4, iop_ae, 0, "xra m"},
	{0, 4, iop_af, 0, "xra a"},

	{0, 4, iop_b0, 0, "ora b"},
	{0, 4, iop_b1, 0, "ora c"},
	{0, 4, iop_b2, 0, "ora d"},
	{0, 4, iop_b3, 0, "ora e"},
	{0, 4, iop_b4, 0, "ora h"},
	{0, 4, iop_b5, 0, "ora l"},
	{0, 4, iop_b6, 0, "ora m"},
	{0, 4, iop_b7, 0, "ora a"},

	{0, 4, iop_b8, 0, "cmp b"},
	{0, 4, iop_b9, 0, "cmp c"},
	{0, 4, iop_ba, 0, "cmp d"},
	{0, 4, iop_bb, 0, "cmp e"},
	{0, 4, iop_bc, 0, "cmp h"},
	{0, 4, iop_bd, 0, "cmp l"},
	{0, 4, iop_be, 0, "cmp m"},
	{0, 4, iop_bf, 0, "cmp a"},

	{0, 5, iop_c0, 0, "rnz"},
	{0, 4, iop_c1, 0, "pop b"},
	{0, 4, iop_c2, 0, "jnz :2"},
	{0, 4, iop_c3, 0, "jmp :2"},
	{OF_SKIPABLE, 5, iop_c4, 0, "cnz :2"},
	{0, 5, iop_c5, 0, "push b"},
	{0, 4, iop_c6, 0, "adi :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 0"},

	{0, 5, iop_c8, 0, "rz"},
	{0, 4, iop_c9, 0, "ret"},
	{0, 4, iop_ca, 0, "jz :2"},
	{0, 4, iop_c3, 0, "jmp :2"},
	{OF_SKIPABLE, 5, iop_cc, 0, "cz :2"},
	{OF_SKIPABLE, 5, iop_cd, 0, "call :2"},
	{0, 4, iop_ce, 0, "aci :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 1"},

	{0, 5, iop_d0, 0, "rnc"},
	{0, 4, iop_d1, 0, "pop d"},
	{0, 4, iop_d2, 0, "jnc :2"},
	{0, 4, iop_d3, 0, "out :1"},
	{OF_SKIPABLE, 5, iop_d4, 0, "cnc :2"},
	{0, 5, iop_d5, 0, "push d"},
	{0, 4, iop_d6, 0, "sui :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 2"},

	{0, 5, iop_d8, 0, "rc"},
	{0, 4, iop_c9, 0, "ret"},
	{0, 4, iop_da, 0, "jc :2"},
	{0, 4, iop_db, 0, "in :1"},
	{OF_SKIPABLE, 5, iop_dc, 0, "cc :2"},
	{OF_SKIPABLE, 5, iop_cd, 0, "call :2"},
	{0, 4, iop_de, 0, "sbi :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 3"},

	{0, 5, iop_e0, 0, "rpo"},
	{0, 4, iop_e1, 0, "pop h"},
	{0, 4, iop_e2, 0, "jpo :2"},
	{0, 4, iop_e3, 0, "xthl"},
	{OF_SKIPABLE, 5, iop_e4, 0, "cpo :2"},
	{0, 5, iop_e5, 0, "push h"},
	{0, 4, iop_e6, 0, "ani :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 4"},

	{0, 5, iop_e8, 0, "rpe"},
	{0, 5, iop_e9, 0, "pchl"},
	{0, 4, iop_ea, 0, "jpe :2"},
	{0, 5, iop_eb, 0, "xchg"},
	{OF_SKIPABLE, 5, iop_ec, 0, "cpe :2"},
	{OF_SKIPABLE, 5, iop_cd, 0, "call :2"},
	{0, 4, iop_ee, 0, "xri :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 5"},

	{0, 5, iop_f0, 0, "rp"},
	{0, 4, iop_f1, 0, "pop psw"},
	{0, 4, iop_f2, 0, "jp :2"},
	{0, 4, iop_f3, 0, "di"},
	{OF_SKIPABLE, 5, iop_f4, 0, "cp :2"},
	{0, 5, iop_f5, 0, "push psw"},
	{0, 4, iop_f6, 0, "ori :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 6"},

	{0, 5, iop_f8, 0, "rm"},
	{0, 5, iop_f9, 0, "sphl"},
	{0, 4, iop_fa, 0, "jm :2"},
	{0, 4, iop_fb, 0, "ei"},
	{OF_SKIPABLE, 5, iop_fc, 0, "cm :2"},
	{OF_SKIPABLE, 5, iop_cd, 0, "call :2"},
	{0, 4, iop_fe, 0, "cpi :1"},
	{OF_SKIPABLE, 5, iop_rst, 0, "rst 7"},
};
