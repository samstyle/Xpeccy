#include "v30regs.h"
#include "v30.h"

#include <stdio.h>

int v30_immb(CPU*);
void v30_push(CPU*,int);
int v30_pop(CPU*);
void v30_set_ps(CPU*,int);
void v30_int(CPU*,int);

// NOTE: use v30 regBP as 8080 regSP
// NOTE: PS segment for fetch operations, DS0 for all others

static int v30emu_add_h[8] = {0, 0, 1, 0, 1, 0, 1, 1};
static int v30emu_sub_h[8] = {1, 0, 0, 0, 1, 1, 1, 0};

// common

int v30emu_immb(CPU* cpu) {
	int d = cpu_mrd(cpu, (cpu->regPS << 4) + cpu->regPC);
	cpu->regPC++;
	return d;
}

int v30emu_mrd(CPU* cpu, int adr) {
	return cpu_mrd(cpu, (cpu->regDS0 << 4) + adr);
}

void v30emu_mwr(CPU* cpu, int adr, int dat) {
	cpu_mwr(cpu, (cpu->regDS0 << 4) + adr, dat);
}

void v30emu_setflag(CPU* cpu, int v) {
	cpu->flgC = v & 1;
	cpu->flgF1 = !!(v & 2);
	cpu->flgP = !!(v & 4);
	cpu->flgF3 = !!(v & 8);
	cpu->flgA = !!(v & 16);
	cpu->flgF5 = !!(v & 32);
	cpu->flgZ = !!(v & 64);
	cpu->flgS = !!(v & 128);
}

int v30emu_getflag(CPU* cpu) {
	int r = cpu->flgC | (cpu->flgF1 << 1) | (cpu->flgP << 2) | (cpu->flgF3 << 3);
	r |= (cpu->flgA << 4) | (cpu->flgF5 << 5) | (cpu->flgZ << 6) | (cpu->flgS << 7);
	return r;
}

unsigned char v30emu_inr(CPU* cpu, unsigned char val) {
	val++;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgA = !!((val & 0x0f) == 0);
	cpu->flgP = parity(val);
	return val;
}

unsigned char v30emu_dcr(CPU* cpu, unsigned char val) {
	val--;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgA = !!((val & 0x0f) != 0x0f);	// A flag is inverted (1:no b4-carry, 0:b4-carry)
	cpu->flgP = parity(val);
	return val;
}

unsigned char v30emu_add(CPU* cpu, unsigned char val, unsigned char add) {
	cpu->tmpw = val + add;
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgA = !!v30emu_add_h[((val & 8) >> 1) | ((add & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->flgP = parity(cpu->ltw);
	cpu->flgC = !!cpu->htw;
	cpu->flgF1 = 0;
	cpu->flgF3 = 0;
	cpu->flgF5 = 0;
	return cpu->ltw;
}

unsigned char v30emu_adc(CPU* cpu, unsigned char val, unsigned char add) {
	cpu->tmpw = val + add + cpu->flgC;
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgA = !!v30emu_add_h[((val & 8) >> 1) | ((add & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->flgP = parity(cpu->ltw);
	cpu->flgC = !!cpu->htw;
	return cpu->ltw;
}

unsigned char v30emu_sub(CPU* cpu, unsigned char val, unsigned char sub) {
	cpu->tmpw = val - sub;
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgA = !!v30emu_sub_h[((val & 8) >> 1) | ((sub & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->flgP = parity(cpu->ltw);
	cpu->flgC = !!cpu->htw;
	return cpu->ltw;
}

unsigned char v30emu_sbb(CPU* cpu, unsigned char val, unsigned char sub) {
	cpu->tmpw = val - sub - cpu->flgC;
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgA = !!v30emu_sub_h[((val & 8) >> 1) | ((sub & 8) >> 2) | ((cpu->ltw & 8) >> 3)];
	cpu->flgP = parity(cpu->ltw);
	cpu->flgC = !!cpu->htw;
	return cpu->ltw;
}

unsigned char v30emu_ana(CPU* cpu, unsigned char val, unsigned char arg) {
	cpu->flgA = !!((val | arg) & 0x08);	// from sPycialist
	val &= arg;
	cpu->flgC = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgP = parity(val);
	cpu->flgC = 0;
	return  val;
}

unsigned char v30emu_xra(CPU* cpu, unsigned char val, unsigned char arg) {
	val ^= arg;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgC = parity(val);
	cpu->flgA = 0;
	cpu->flgC = 0;
	return val;
}

unsigned char v30emu_ora(CPU* cpu, unsigned char val, unsigned char arg) {
	val |= arg;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgP = parity(val);
	cpu->flgA = 0;
	cpu->flgC = 0;
	return val;
}

unsigned short v30emu_pop(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_mrd(cpu, cpu->regBP++);
	cpu->t += 3;
	cpu->regWZh = v30emu_mrd(cpu, cpu->regBP++);
	return cpu->regWZ;
}

void v30emu_push(CPU* cpu, unsigned short val) {
	cpu->regWZ = val;
	cpu->t += 3;
	v30emu_mwr(cpu, --cpu->regBP, cpu->regWZh);
	cpu->t += 3;
	v30emu_mwr(cpu, --cpu->regBP, cpu->regWZl);
}

// 00, 08, 10, 18, 20, 28 : nop
void v30emu_00(CPU* cpu) {}

// 01:lxi b,nn
void v30emu_01(CPU* cpu) {
	cpu->t += 3;
	cpu->regC = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regB = v30emu_immb(cpu);
}

// 02:stax b
void v30emu_02(CPU* cpu) {
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regBC, cpu->regA);
}

// 03:inx b
void v30emu_03(CPU* cpu) {
	cpu->regBC++;
}

// 04:inr b
void v30emu_04(CPU* cpu) {
	cpu->regB = v30emu_inr(cpu, cpu->regB);
}

// 05:dcr b
void v30emu_05(CPU* cpu) {
	cpu->regB = v30emu_dcr(cpu, cpu->regB);
}

// 06:mvi b,n
void v30emu_06(CPU* cpu) {
	cpu->t += 3;
	cpu->regB = v30emu_immb(cpu);
}

// 07:rlc
void v30emu_07(CPU* cpu) {
	//cpu->f &= ~IFL_C;
	cpu->flgC = !!(cpu->regA & 0x80);
	cpu->regA <<= 1;
	if (cpu->flgC) cpu->regA |= 1;
}

// 08:nop*
// 09:dad b
void v30emu_09(CPU* cpu) {
	cpu->tmpi = cpu->regBC + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	//cpu->f &= ~IFL_C;
	cpu->flgC = !!(cpu->tmpi > 0xffff);
}

// 0a:ldax b
void v30emu_0a(CPU* cpu) {
	cpu->t += 3;
	cpu->regA = v30emu_mrd(cpu, cpu->regBC);
}

// 0b:dcx b
void v30emu_0b(CPU* cpu) {
	cpu->regBC--;
}

// 0c:inr c
void v30emu_0c(CPU* cpu) {
	cpu->regC = v30emu_inr(cpu, cpu->regC);
}

// 0d:dcr c
void v30emu_0d(CPU* cpu) {
	cpu->regC = v30emu_dcr(cpu, cpu->regC);
}

// 0e:mvi c,n
void v30emu_0e(CPU* cpu) {
	cpu->t += 3;
	cpu->regC =v30emu_immb(cpu);
}

// 0f:rrc
void v30emu_0f(CPU* cpu) {
	//cpu->f &= ~IFL_C;
	cpu->flgC = cpu->regA & 0x01;
	cpu->regA >>= 1;
	if (cpu->flgC) cpu->regA |= 0x80;
}

// 11:lxi d,nn
void v30emu_11(CPU* cpu) {
	cpu->t += 3;
	cpu->regE = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regD = v30emu_immb(cpu);
}

// 12:stax d
void v30emu_12(CPU* cpu) {
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regDE, cpu->regA);
}

// 13:inx d
void v30emu_13(CPU* cpu) {
	cpu->regDE++;
}

// 14:inr d
void v30emu_14(CPU* cpu) {
	cpu->regD = v30emu_inr(cpu, cpu->regD);
}

// 15:dcr d
void v30emu_15(CPU* cpu) {
	cpu->regD = v30emu_dcr(cpu, cpu->regD);
}

// 16:mvi d,n
void v30emu_16(CPU* cpu) {
	cpu->t += 3;
	cpu->regD = v30emu_immb(cpu);
}

// 17:ral
void v30emu_17(CPU* cpu) {
	cpu->ltw = cpu->regA;
	cpu->tmpw <<= 1;
	if (cpu->flgC) cpu->ltw |= 1;
	cpu->flgC = cpu->htw & 1;
	cpu->regA = cpu->ltw;
}

// 19:dad d
void v30emu_19(CPU* cpu) {
	cpu->tmpi = cpu->regDE + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	//cpu->f &= ~IFL_C;
	cpu->flgC = !!(cpu->tmpi > 0xffff);
}

// 1a:ldax d
void v30emu_1a(CPU* cpu) {
	cpu->t += 3;
	cpu->regA = v30emu_mrd(cpu, cpu->regDE);
}

// 1b:dcx d
void v30emu_1b(CPU* cpu) {
	cpu->regDE--;
}

// 1c:inr e
void v30emu_1c(CPU* cpu) {
	cpu->regE = v30emu_inr(cpu, cpu->regE);
}

// 1d:dcr e
void v30emu_1d(CPU* cpu) {
	cpu->regE = v30emu_dcr(cpu, cpu->regE);
}

// 1e:mvi e,n
void v30emu_1e(CPU* cpu) {
	cpu->t += 3;
	cpu->regE = v30emu_immb(cpu);
}

// 1f:rar
void v30emu_1f(CPU* cpu) {
	cpu->htw = cpu->regA;
	cpu->tmpw >>= 1;
	if (cpu->flgC) cpu->htw |= 0x80;
	cpu->flgC = !!(cpu->ltw & 0x80);
	cpu->regA = cpu->htw;
}

// 21:lxi h,nn
void v30emu_21(CPU* cpu) {
	cpu->t += 3;
	cpu->regL = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regH = v30emu_immb(cpu);
}

// 22:shld nn
void v30emu_22(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regWZ++, cpu->regL);
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regWZ, cpu->regH);
}


// 23:inx h
void v30emu_23(CPU* cpu) {
	cpu->regHL++;
}

// 24:inr h
void v30emu_24(CPU* cpu) {
	cpu->regH = v30emu_inr(cpu, cpu->regH);
}

// 25:dcr h
void v30emu_25(CPU* cpu) {
	cpu->regH = v30emu_dcr(cpu, cpu->regH);
}

// 26:mvi h,n
void v30emu_26(CPU* cpu) {
	cpu->t += 3;
	cpu->regH = v30emu_immb(cpu);
}

// 27:daa
void v30emu_27(CPU* cpu) {
	unsigned char add = 0;
	unsigned char cf = cpu->flgC;
	if ((cpu->flgA) || ((cpu->regA & 0x0f) > 0x09))
		add = 6;
	if ((cpu->flgC) || (cpu->regA > 0x9f) || ((cpu->regA > 0x8f) && ((cpu->regA & 0x0f) > 0x09)))
		add |= 0x60;
	if (cpu->regA > 0x99)
		cf = 1;
	cpu->regA = v30emu_add(cpu, cpu->regA, add);
	cpu->flgC = cf;
}

// 29:dad h
void v30emu_29(CPU* cpu) {
	cpu->tmpi = cpu->regHL + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	cpu->flgC = !!(cpu->tmpi > 0xffff);
}

// 2a:lhld nn
void v30emu_2a(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regL = v30emu_mrd(cpu, cpu->regWZ++);
	cpu->t += 3;
	cpu->regH = v30emu_mrd(cpu, cpu->regWZ);
}

// 2b:dcx h
void v30emu_2b(CPU* cpu) {
	cpu->regHL--;
}

// 2c:inr l
void v30emu_2c(CPU* cpu) {
	cpu->regL = v30emu_inr(cpu, cpu->regL);
}

// 2d:dcr l
void v30emu_2d(CPU* cpu) {
	cpu->regL = v30emu_dcr(cpu, cpu->regL);
}

// 2e:mvi l,n
void v30emu_2e(CPU* cpu) {
	cpu->t += 3;
	cpu->regL = v30emu_immb(cpu);
}

// 2f:cma
void v30emu_2f(CPU* cpu) {
	cpu->regA ^= 0xff;
}

// 31:lxi sp,nn
void v30emu_31(CPU* cpu) {
	cpu->t += 3;
	cpu->regBPl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regBPh = v30emu_immb(cpu);
}

// 32:sta nn
void v30emu_32(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regWZ, cpu->regA);
}

// 33:inx sp
void v30emu_33(CPU* cpu) {
	cpu->regBP++;
}

// 34:inr m
void v30emu_34(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->tmpb = v30emu_inr(cpu, cpu->tmpb);
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regHL, cpu->tmpb);
}

// 35:dcr m
void v30emu_35(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->tmpb = v30emu_dcr(cpu, cpu->tmpb);
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regHL, cpu->tmpb);
}

// 36:mvi m,n
void v30emu_36(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->t += 3;
	v30emu_mwr(cpu, cpu->regHL, cpu->tmpb);
}

// 37:stc
void v30emu_37(CPU* cpu) {
	cpu->flgC = 1;
}

// 39:dad sp
void v30emu_39(CPU* cpu) {
	cpu->tmpi = cpu->regBP + cpu->regHL;
	cpu->regHL = cpu->tmpi & 0xffff;
	// cpu->f &= ~IFL_C;
	cpu->flgC = !!(cpu->tmpi > 0xffff);
}

// 3a:lda nn
void v30emu_3a(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regA = v30emu_mrd(cpu, cpu->regWZ);
}

// 3b:dcx sp
void v30emu_3b(CPU* cpu) {
	cpu->regBP--;
}

// 3c:inr a
void v30emu_3c(CPU* cpu) {
	cpu->regA = v30emu_inr(cpu, cpu->regA);
}

// 3d:dcr a
void v30emu_3d(CPU* cpu) {
	cpu->regA = v30emu_dcr(cpu, cpu->regA);
}

// 3e:mvi a,n
void v30emu_3e(CPU* cpu) {
	cpu->t += 3;
	cpu->regA = v30emu_immb(cpu);
}

// 3f:cmc
void v30emu_3f(CPU* cpu) {
	cpu->flgC ^= 1;
}

// 40..47: mov b,x
void v30emu_40(CPU* cpu) {cpu->regB = cpu->regB;}
void v30emu_41(CPU* cpu) {cpu->regB = cpu->regC;}
void v30emu_42(CPU* cpu) {cpu->regB = cpu->regD;}
void v30emu_43(CPU* cpu) {cpu->regB = cpu->regE;}
void v30emu_44(CPU* cpu) {cpu->regB = cpu->regH;}
void v30emu_45(CPU* cpu) {cpu->regB = cpu->regL;}
void v30emu_46(CPU* cpu) {cpu->t += 3; cpu->regB = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_47(CPU* cpu) {cpu->regB = cpu->regA;}
// 48..4f: mov c,x
void v30emu_48(CPU* cpu) {cpu->regC = cpu->regB;}
void v30emu_49(CPU* cpu) {cpu->regC = cpu->regC;}
void v30emu_4a(CPU* cpu) {cpu->regC = cpu->regD;}
void v30emu_4b(CPU* cpu) {cpu->regC = cpu->regE;}
void v30emu_4c(CPU* cpu) {cpu->regC = cpu->regH;}
void v30emu_4d(CPU* cpu) {cpu->regC = cpu->regL;}
void v30emu_4e(CPU* cpu) {cpu->t += 3; cpu->regC = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_4f(CPU* cpu) {cpu->regC = cpu->regA;}
// 50..57: mov d,x
void v30emu_50(CPU* cpu) {cpu->regD = cpu->regB;}
void v30emu_51(CPU* cpu) {cpu->regD = cpu->regC;}
void v30emu_52(CPU* cpu) {cpu->regD = cpu->regD;}
void v30emu_53(CPU* cpu) {cpu->regD = cpu->regE;}
void v30emu_54(CPU* cpu) {cpu->regD = cpu->regH;}
void v30emu_55(CPU* cpu) {cpu->regD = cpu->regL;}
void v30emu_56(CPU* cpu) {cpu->t += 3; cpu->regD = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_57(CPU* cpu) {cpu->regD = cpu->regA;}
// 58..5f: mov e,x
void v30emu_58(CPU* cpu) {cpu->regE = cpu->regB;}
void v30emu_59(CPU* cpu) {cpu->regE = cpu->regC;}
void v30emu_5a(CPU* cpu) {cpu->regE = cpu->regD;}
void v30emu_5b(CPU* cpu) {cpu->regE = cpu->regE;}
void v30emu_5c(CPU* cpu) {cpu->regE = cpu->regH;}
void v30emu_5d(CPU* cpu) {cpu->regE = cpu->regL;}
void v30emu_5e(CPU* cpu) {cpu->t += 3; cpu->regE = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_5f(CPU* cpu) {cpu->regE = cpu->regA;}
// 60..67: mov h,x
void v30emu_60(CPU* cpu) {cpu->regH = cpu->regB;}
void v30emu_61(CPU* cpu) {cpu->regH = cpu->regC;}
void v30emu_62(CPU* cpu) {cpu->regH = cpu->regD;}
void v30emu_63(CPU* cpu) {cpu->regH = cpu->regE;}
void v30emu_64(CPU* cpu) {cpu->regH = cpu->regH;}
void v30emu_65(CPU* cpu) {cpu->regH = cpu->regL;}
void v30emu_66(CPU* cpu) {cpu->t += 3; cpu->regH = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_67(CPU* cpu) {cpu->regH = cpu->regA;}
// 68..6f: mov l,x
void v30emu_68(CPU* cpu) {cpu->regL = cpu->regB;}
void v30emu_69(CPU* cpu) {cpu->regL = cpu->regC;}
void v30emu_6a(CPU* cpu) {cpu->regL = cpu->regD;}
void v30emu_6b(CPU* cpu) {cpu->regL = cpu->regE;}
void v30emu_6c(CPU* cpu) {cpu->regL = cpu->regH;}
void v30emu_6d(CPU* cpu) {cpu->regL = cpu->regL;}
void v30emu_6e(CPU* cpu) {cpu->t += 3; cpu->regL = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_6f(CPU* cpu) {cpu->regL = cpu->regA;}
// 70..77: mov m,x
void v30emu_70(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regB);}
void v30emu_71(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regC);}
void v30emu_72(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regD);}
void v30emu_73(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regE);}
void v30emu_74(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regH);}
void v30emu_75(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regL);}
void v30emu_76(CPU* cpu) {cpu->regPC--; if (!cpu->flgHALT) {cpu->flgHALT = 1; cpu_irq(cpu, IRQ_CPU_HALT);}}
void v30emu_77(CPU* cpu) {cpu->t += 3; v30emu_mwr(cpu, cpu->regHL, cpu->regA);}
// 78..7f: mov a,x
void v30emu_78(CPU* cpu) {cpu->regA = cpu->regB;}
void v30emu_79(CPU* cpu) {cpu->regA = cpu->regC;}
void v30emu_7a(CPU* cpu) {cpu->regA = cpu->regD;}
void v30emu_7b(CPU* cpu) {cpu->regA = cpu->regE;}
void v30emu_7c(CPU* cpu) {cpu->regA = cpu->regH;}
void v30emu_7d(CPU* cpu) {cpu->regA = cpu->regL;}
void v30emu_7e(CPU* cpu) {cpu->t += 3; cpu->regA = v30emu_mrd(cpu, cpu->regHL);}
void v30emu_7f(CPU* cpu) {cpu->regA = cpu->regA;}
// 80..87: add x
void v30emu_80(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regB);}
void v30emu_81(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regC);}
void v30emu_82(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regD);}
void v30emu_83(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regE);}
void v30emu_84(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regH);}
void v30emu_85(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regL);}
void v30emu_86(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_add(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_87(CPU* cpu) {cpu->regA = v30emu_add(cpu, cpu->regA, cpu->regA);}
// 88..8f: adc x
void v30emu_88(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regB);}
void v30emu_89(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regC);}
void v30emu_8a(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regD);}
void v30emu_8b(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regE);}
void v30emu_8c(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regH);}
void v30emu_8d(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regL);}
void v30emu_8e(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_8f(CPU* cpu) {cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->regA);}
// 90..97: sub x
void v30emu_90(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regB);}
void v30emu_91(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regC);}
void v30emu_92(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regD);}
void v30emu_93(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regE);}
void v30emu_94(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regH);}
void v30emu_95(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regL);}
void v30emu_96(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_97(CPU* cpu) {cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->regA);}
// 98..9f: sbb x
void v30emu_98(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regB);}
void v30emu_99(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regC);}
void v30emu_9a(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regD);}
void v30emu_9b(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regE);}
void v30emu_9c(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regH);}
void v30emu_9d(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regL);}
void v30emu_9e(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_9f(CPU* cpu) {cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->regA);}
// a0..a7: ana x
void v30emu_a0(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regB);}
void v30emu_a1(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regC);}
void v30emu_a2(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regD);}
void v30emu_a3(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regE);}
void v30emu_a4(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regH);}
void v30emu_a5(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regL);}
void v30emu_a6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_a7(CPU* cpu) {cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->regA);}
// a8..af: xra x
void v30emu_a8(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regB);}
void v30emu_a9(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regC);}
void v30emu_aa(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regD);}
void v30emu_ab(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regE);}
void v30emu_ac(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regH);}
void v30emu_ad(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regL);}
void v30emu_ae(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_af(CPU* cpu) {cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->regA);}
// b0..b7: ora x
void v30emu_b0(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regB);}
void v30emu_b1(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regC);}
void v30emu_b2(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regD);}
void v30emu_b3(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regE);}
void v30emu_b4(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regH);}
void v30emu_b5(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regL);}
void v30emu_b6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_b7(CPU* cpu) {cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->regA);}
// b8..bf: cmp x
void v30emu_b8(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regB);}
void v30emu_b9(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regC);}
void v30emu_ba(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regD);}
void v30emu_bb(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regE);}
void v30emu_bc(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regH);}
void v30emu_bd(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regL);}
void v30emu_be(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_mrd(cpu, cpu->regHL);
	cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->tmpb);
}
void v30emu_bf(CPU* cpu) {cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->regA);}
// c0: rnz
void v30emu_c0(CPU* cpu) {if (!cpu->flgZ) cpu->regPC = v30emu_pop(cpu);}
// c1: pop b
void v30emu_c1(CPU* cpu) {cpu->regBC = v30emu_pop(cpu);}
// c2: jnz nn
void v30emu_c2(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (!cpu->flgZ) cpu->regPC = cpu->regWZ;
}
// c3: jmp nn
void v30emu_c3(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	cpu->regPC = cpu->regWZ;
}
// c4: cnz nn
void v30emu_c4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (!cpu->flgZ) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// c5: push b
void v30emu_c5(CPU* cpu) {v30emu_push(cpu, cpu->regBC);}
// c6: adi n
void v30emu_c6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_add(cpu, cpu->regA, cpu->tmpb);
}
// c7/cf/d7/df/e7/ef/f7/ff: rst n
void v30emu_rst(CPU* cpu) {
	v30emu_push(cpu, cpu->regPC);
	cpu->regPC = cpu->com & 0x38;
}
// c8: rz
void v30emu_c8(CPU* cpu) {if (cpu->flgZ) cpu->regPC = v30emu_pop(cpu);}
// c9: ret
void v30emu_c9(CPU* cpu) {cpu->regPC = v30emu_pop(cpu);}
// ca: jz nn
void v30emu_ca(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (cpu->flgZ) cpu->regPC = cpu->regWZ;
}
// cb = c3
// cc: cz nn
void v30emu_cc(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (cpu->flgZ) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// cd: call nn
void v30emu_cd(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	v30emu_push(cpu, cpu->regPC);
	cpu->regPC = cpu->tmpw;
}
// ce: aci n
void v30emu_ce(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_adc(cpu, cpu->regA, cpu->tmpb);
}
// cf: rst 1
// d0: rnc
void v30emu_d0(CPU* cpu) {if (!cpu->flgC) cpu->regPC = v30emu_pop(cpu);}
// d1: pop d
void v30emu_d1(CPU* cpu) {cpu->regDE = v30emu_pop(cpu);}
// d2: jnc nn
void v30emu_d2(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (!cpu->flgC) cpu->regPC = cpu->regWZ;
}
// d3: out n
void v30emu_d3(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->htw = cpu->ltw;	// A8..15 = A0..7
	cpu->t += 3;
	cpu->iwr(cpu->tmpw, cpu->regA, cpu->xptr);
}
// d4: cnc nn
void v30emu_d4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (!cpu->flgC) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// d5: push d
void v30emu_d5(CPU* cpu) {v30emu_push(cpu, cpu->regDE);}
// d6: sui n
void v30emu_d6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_sub(cpu, cpu->regA, cpu->tmpb);
}
// d7: rst 2
// d8: rc
void v30emu_d8(CPU* cpu) {if (cpu->flgC) cpu->regPC = v30emu_pop(cpu);}
// d9: ret*
// da: jc nn
void v30emu_da(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (cpu->flgC) cpu->regPC = cpu->regWZ;
}
// db: in n
void v30emu_db(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->htw = cpu->regA;
	cpu->t += 3;
	cpu->regA = cpu->ird(cpu->tmpw, cpu->xptr) & 0xff;
}
// dc: cc nn
void v30emu_dc(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (cpu->flgC) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// dd: *call nn
// de: sbi n
void v30emu_de(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_sbb(cpu, cpu->regA, cpu->tmpb);
}
// df: rst 3
// e0: rpo
void v30emu_e0(CPU* cpu) {if (!cpu->flgP) cpu->regPC = v30emu_pop(cpu);}
// e1: pop h
void v30emu_e1(CPU* cpu) {cpu->regHL = v30emu_pop(cpu);}
// e2: jpo nn
void v30emu_e2(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (!cpu->flgP) cpu->regPC = cpu->regWZ;
}
// e3: xthl = ex (sp),hl
void v30emu_e3(CPU* cpu) {
	cpu->tmpw = v30emu_pop(cpu);
	v30emu_push(cpu, cpu->regHL);
	cpu->t += 2;
	cpu->regHL = cpu->tmpw;
}
// e4: cpo nn
void v30emu_e4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (!cpu->flgP) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// e5: push h
void v30emu_e5(CPU* cpu) {v30emu_push(cpu, cpu->regHL);}
// e6: ani n
void v30emu_e6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_ana(cpu, cpu->regA, cpu->tmpb);
}
// e7: rst 4
// e8: rpe
void v30emu_e8(CPU* cpu) {if (cpu->flgP) cpu->regPC = v30emu_pop(cpu);}
// e9: pchl
void v30emu_e9(CPU* cpu) {cpu->regPC = cpu->regHL;}
// ea: jpe nn
void v30emu_ea(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (cpu->flgP) cpu->regPC = cpu->regWZ;
}
// eb: xchg
void v30emu_eb(CPU* cpu) {
	cpu->tmpw = cpu->regDE;
	cpu->regDE = cpu->regHL;
	cpu->regHL = cpu->tmpw;
}
// ec: cpe nn
void v30emu_ec(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (cpu->flgP) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// ed: emu prefix ?
// ed,ed,n = calln n = brk n in native mode
// ed,fd = retem = return from emulation mode
void v30emu_ed(CPU* cpu) {
	cpu->tmp = v30emu_immb(cpu);
	if (cpu->tmp == 0xed) {
		cpu->tmp = v30emu_immb(cpu);
		v30_push(cpu, v30_getflag(cpu));
		v30_push(cpu, cpu->regPS);
		v30_push(cpu, cpu->regPC);
		cpu->flgMD = 1;
		v30_int(cpu, cpu->tmp);
	} else if (cpu->tmp == 0xfd) {
		cpu->regPC = v30_pop(cpu);
		v30_set_ps(cpu, v30_pop(cpu));
		v30_setflag(cpu, v30_pop(cpu));
		cpu->flgBLKM = 1;
	}
}

// ee: xri n
void v30emu_ee(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_xra(cpu, cpu->regA, cpu->tmpb);
}
// ef: rst 5
// f0: rp
void v30emu_f0(CPU* cpu) {if (!cpu->flgS) cpu->regPC = v30emu_pop(cpu);}
// f1: pop psw
void v30emu_f1(CPU* cpu) {
	cpu->tmpw = v30emu_pop(cpu);
	cpu->regA = cpu->htw;
	v30emu_setflag(cpu, cpu->ltw);
}
// f2: jp nn
void v30emu_f2(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (!cpu->flgS) {
		cpu->regPC = cpu->tmpw;
	}
}
// f3: di
void v30emu_f3(CPU* cpu) {
	cpu->flgIFF1 = 0;
//	cpu->inten &= ~I8080_INT;
}
// f4: cp nn
void v30emu_f4(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (!cpu->flgS) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// f5: push psw
void v30emu_f5(CPU* cpu) {
	cpu->htw = cpu->regA;
	cpu->ltw = v30emu_getflag(cpu) & 0xff;;
	v30emu_push(cpu, cpu->tmpw);
}

// f6: ori n
void v30emu_f6(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->regA = v30emu_ora(cpu, cpu->regA, cpu->tmpb);
}
// f7: rst 6
// f8: rm
void v30emu_f8(CPU* cpu) {if (cpu->flgS) cpu->regPC = v30emu_pop(cpu);}
// f9: sphl
void v30emu_f9(CPU* cpu) {
	cpu->regBP = cpu->regHL;
}
// fa: jm nn
void v30emu_fa(CPU* cpu) {
	cpu->t += 3;
	cpu->regWZl = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->regWZh = v30emu_immb(cpu);
	if (cpu->flgS) cpu->regPC = cpu->regWZ;
}
// fb: ei
void v30emu_fb(CPU* cpu) {
	cpu->flgIFF1 = 1;
//	cpu->inten |= I8080_INT;
}
// fc: cm nn
void v30emu_fc(CPU* cpu) {
	cpu->t += 3;
	cpu->ltw = v30emu_immb(cpu);
	cpu->t += 3;
	cpu->htw = v30emu_immb(cpu);
	if (cpu->flgS) {
		v30emu_push(cpu, cpu->regPC);
		cpu->regPC = cpu->tmpw;
	}
}
// fd: *call nn
// fe: cpi n
void v30emu_fe(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = v30emu_immb(cpu);
	cpu->tmpb = v30emu_sub(cpu, cpu->regA, cpu->tmpb);
}
// ff: rst 7

opCode v30emu_tab[256] = {
	{0, 4, v30emu_00, 0, "nop"},
	{0, 4, v30emu_01, 0, "lxi b,:2"},
	{0, 4, v30emu_02, 0, "stax b"},
	{0, 5, v30emu_03, 0, "inx b"},
	{0, 5, v30emu_04, 0, "inr b"},
	{0, 5, v30emu_05, 0, "dcr b"},
	{0, 4, v30emu_06, 0, "mvi b,:1"},
	{0, 4, v30emu_07, 0, "rlc"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 10, v30emu_09, 0, "dad b"},
	{0, 4, v30emu_0a, 0, "ldax b"},
	{0, 5, v30emu_0b, 0, "dcx b"},
	{0, 5, v30emu_0c, 0, "inr c"},
	{0, 5, v30emu_0d, 0, "dcr c"},
	{0, 4, v30emu_0e, 0, "mvi c,:1"},
	{0, 4, v30emu_0f, 0, "rrc"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 4, v30emu_11, 0, "lxi d,:2"},
	{0, 4, v30emu_12, 0, "stax d"},
	{0, 5, v30emu_13, 0, "inx d"},
	{0, 5, v30emu_14, 0, "inr d"},
	{0, 5, v30emu_15, 0, "dcr d"},
	{0, 4, v30emu_16, 0, "mvi d,:1"},
	{0, 4, v30emu_17, 0, "ral"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 10, v30emu_19, 0, "dad d"},
	{0, 4, v30emu_1a, 0, "ldax d"},
	{0, 5, v30emu_1b, 0, "dcx d"},
	{0, 5, v30emu_1c, 0, "inr e"},
	{0, 5, v30emu_1d, 0, "dcr e"},
	{0, 4, v30emu_1e, 0, "mvi e,:1"},
	{0, 4, v30emu_1f, 0, "rar"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 4, v30emu_21, 0, "lxi h,:2"},
	{0, 4, v30emu_22, 0, "shld :2"},
	{0, 5, v30emu_23, 0, "inx h"},
	{0, 5, v30emu_24, 0, "inr h"},
	{0, 5, v30emu_25, 0, "dcr h"},
	{0, 4, v30emu_26, 0, "mvi h,:1"},
	{0, 4, v30emu_27, 0, "daa"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 10, v30emu_29, 0, "dad h"},
	{0, 4, v30emu_2a, 0, "lhld :2"},
	{0, 5, v30emu_2b, 0, "dcx h"},
	{0, 5, v30emu_2c, 0, "inr l"},
	{0, 5, v30emu_2d, 0, "dcr l"},
	{0, 4, v30emu_2e, 0, "mvi l,:1"},
	{0, 4, v30emu_2f, 0, "cma"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 4, v30emu_31, 0, "lxi sp,:2"},
	{0, 4, v30emu_32, 0, "sta :2"},
	{0, 5, v30emu_33, 0, "inx sp"},
	{0, 4, v30emu_34, 0, "inr m"},
	{0, 4, v30emu_35, 0, "dcr m"},
	{0, 4, v30emu_36, 0, "mvi m,:1"},
	{0, 4, v30emu_37, 0, "stc"},

	{0, 4, v30emu_00, 0, "nop"},
	{0, 10, v30emu_39, 0, "dad sp"},
	{0, 4, v30emu_3a, 0, "lda :2"},
	{0, 5, v30emu_3b, 0, "dcx sp"},
	{0, 5, v30emu_3c, 0, "inr a"},
	{0, 5, v30emu_3d, 0, "dcr a"},
	{0, 4, v30emu_3e, 0, "mvi a,:1"},
	{0, 4, v30emu_3f, 0, "cmc"},

	{0, 5, v30emu_40, 0, "mov b,b"},
	{0, 5, v30emu_41, 0, "mov b,c"},
	{0, 5, v30emu_42, 0, "mov b,d"},
	{0, 5, v30emu_43, 0, "mov b,e"},
	{0, 5, v30emu_44, 0, "mov b,h"},
	{0, 5, v30emu_45, 0, "mov b,l"},
	{0, 4, v30emu_46, 0, "mov b,m"},
	{0, 5, v30emu_47, 0, "mov b,a"},

	{0, 5, v30emu_48, 0, "mov c,b"},
	{0, 5, v30emu_49, 0, "mov c,c"},
	{0, 5, v30emu_4a, 0, "mov c,d"},
	{0, 5, v30emu_4b, 0, "mov c,e"},
	{0, 5, v30emu_4c, 0, "mov c,h"},
	{0, 5, v30emu_4d, 0, "mov c,l"},
	{0, 4, v30emu_4e, 0, "mov c,m"},
	{0, 5, v30emu_4f, 0, "mov c,a"},

	{0, 5, v30emu_50, 0, "mov d,b"},
	{0, 5, v30emu_51, 0, "mov d,c"},
	{0, 5, v30emu_52, 0, "mov d,d"},
	{0, 5, v30emu_53, 0, "mov d,e"},
	{0, 5, v30emu_54, 0, "mov d,h"},
	{0, 5, v30emu_55, 0, "mov d,l"},
	{0, 4, v30emu_56, 0, "mov d,m"},
	{0, 5, v30emu_57, 0, "mov d,a"},

	{0, 5, v30emu_58, 0, "mov e,b"},
	{0, 5, v30emu_59, 0, "mov e,c"},
	{0, 5, v30emu_5a, 0, "mov e,d"},
	{0, 5, v30emu_5b, 0, "mov e,e"},
	{0, 5, v30emu_5c, 0, "mov e,h"},
	{0, 5, v30emu_5d, 0, "mov e,l"},
	{0, 4, v30emu_5e, 0, "mov e,m"},
	{0, 5, v30emu_5f, 0, "mov e,a"},

	{0, 5, v30emu_60, 0, "mov h,b"},
	{0, 5, v30emu_61, 0, "mov h,c"},
	{0, 5, v30emu_62, 0, "mov h,d"},
	{0, 5, v30emu_63, 0, "mov h,e"},
	{0, 5, v30emu_64, 0, "mov h,h"},
	{0, 5, v30emu_65, 0, "mov h,l"},
	{0, 4, v30emu_66, 0, "mov h,m"},
	{0, 5, v30emu_67, 0, "mov h,a"},

	{0, 5, v30emu_68, 0, "mov l,b"},
	{0, 5, v30emu_69, 0, "mov l,c"},
	{0, 5, v30emu_6a, 0, "mov l,d"},
	{0, 5, v30emu_6b, 0, "mov l,e"},
	{0, 5, v30emu_6c, 0, "mov l,h"},
	{0, 5, v30emu_6d, 0, "mov l,l"},
	{0, 4, v30emu_6e, 0, "mov l,m"},
	{0, 5, v30emu_6f, 0, "mov l,a"},

	{0, 4, v30emu_70, 0, "mov m,b"},
	{0, 4, v30emu_71, 0, "mov m,c"},
	{0, 4, v30emu_72, 0, "mov m,d"},
	{0, 4, v30emu_73, 0, "mov m,e"},
	{0, 4, v30emu_74, 0, "mov m,h"},
	{0, 4, v30emu_75, 0, "mov m,l"},
	{0, 7, v30emu_76, 0, "hlt"},
	{0, 4, v30emu_77, 0, "mov m,a"},

	{0, 5, v30emu_78, 0, "mov a,b"},
	{0, 5, v30emu_79, 0, "mov a,c"},
	{0, 5, v30emu_7a, 0, "mov a,d"},
	{0, 5, v30emu_7b, 0, "mov a,e"},
	{0, 5, v30emu_7c, 0, "mov a,h"},
	{0, 5, v30emu_7d, 0, "mov a,l"},
	{0, 4, v30emu_7e, 0, "mov a,m"},
	{0, 5, v30emu_7f, 0, "mov a,a"},

	{0, 4, v30emu_80, 0, "add b"},
	{0, 4, v30emu_81, 0, "add c"},
	{0, 4, v30emu_82, 0, "add d"},
	{0, 4, v30emu_83, 0, "add e"},
	{0, 4, v30emu_84, 0, "add h"},
	{0, 4, v30emu_85, 0, "add l"},
	{0, 4, v30emu_86, 0, "add m"},
	{0, 4, v30emu_87, 0, "add a"},

	{0, 4, v30emu_88, 0, "adc b"},
	{0, 4, v30emu_89, 0, "adc c"},
	{0, 4, v30emu_8a, 0, "adc d"},
	{0, 4, v30emu_8b, 0, "adc e"},
	{0, 4, v30emu_8c, 0, "adc h"},
	{0, 4, v30emu_8d, 0, "adc l"},
	{0, 4, v30emu_8e, 0, "adc m"},
	{0, 4, v30emu_8f, 0, "adc a"},

	{0, 4, v30emu_90, 0, "sub b"},
	{0, 4, v30emu_91, 0, "sub c"},
	{0, 4, v30emu_92, 0, "sub d"},
	{0, 4, v30emu_93, 0, "sub e"},
	{0, 4, v30emu_94, 0, "sub h"},
	{0, 4, v30emu_95, 0, "sub l"},
	{0, 4, v30emu_96, 0, "sub m"},
	{0, 4, v30emu_97, 0, "sub a"},

	{0, 4, v30emu_98, 0, "sbb b"},
	{0, 4, v30emu_99, 0, "sbb c"},
	{0, 4, v30emu_9a, 0, "sbb d"},
	{0, 4, v30emu_9b, 0, "sbb e"},
	{0, 4, v30emu_9c, 0, "sbb h"},
	{0, 4, v30emu_9d, 0, "sbb l"},
	{0, 4, v30emu_9e, 0, "sbb m"},
	{0, 4, v30emu_9f, 0, "sbb a"},

	{0, 4, v30emu_a0, 0, "ana b"},
	{0, 4, v30emu_a1, 0, "ana c"},
	{0, 4, v30emu_a2, 0, "ana d"},
	{0, 4, v30emu_a3, 0, "ana e"},
	{0, 4, v30emu_a4, 0, "ana h"},
	{0, 4, v30emu_a5, 0, "ana l"},
	{0, 4, v30emu_a6, 0, "ana m"},
	{0, 4, v30emu_a7, 0, "ana a"},

	{0, 4, v30emu_a8, 0, "xra b"},
	{0, 4, v30emu_a9, 0, "xra c"},
	{0, 4, v30emu_aa, 0, "xra d"},
	{0, 4, v30emu_ab, 0, "xra e"},
	{0, 4, v30emu_ac, 0, "xra h"},
	{0, 4, v30emu_ad, 0, "xra l"},
	{0, 4, v30emu_ae, 0, "xra m"},
	{0, 4, v30emu_af, 0, "xra a"},

	{0, 4, v30emu_b0, 0, "ora b"},
	{0, 4, v30emu_b1, 0, "ora c"},
	{0, 4, v30emu_b2, 0, "ora d"},
	{0, 4, v30emu_b3, 0, "ora e"},
	{0, 4, v30emu_b4, 0, "ora h"},
	{0, 4, v30emu_b5, 0, "ora l"},
	{0, 4, v30emu_b6, 0, "ora m"},
	{0, 4, v30emu_b7, 0, "ora a"},

	{0, 4, v30emu_b8, 0, "cmp b"},
	{0, 4, v30emu_b9, 0, "cmp c"},
	{0, 4, v30emu_ba, 0, "cmp d"},
	{0, 4, v30emu_bb, 0, "cmp e"},
	{0, 4, v30emu_bc, 0, "cmp h"},
	{0, 4, v30emu_bd, 0, "cmp l"},
	{0, 4, v30emu_be, 0, "cmp m"},
	{0, 4, v30emu_bf, 0, "cmp a"},

	{0, 5, v30emu_c0, 0, "rnz"},
	{0, 4, v30emu_c1, 0, "pop b"},
	{0, 4, v30emu_c2, 0, "jnz :2"},
	{0, 4, v30emu_c3, 0, "jmp :2"},
	{OF_SKIPABLE, 5, v30emu_c4, 0, "cnz :2"},
	{0, 5, v30emu_c5, 0, "push b"},
	{0, 4, v30emu_c6, 0, "adi :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 0"},

	{0, 5, v30emu_c8, 0, "rz"},
	{0, 4, v30emu_c9, 0, "ret"},
	{0, 4, v30emu_ca, 0, "jz :2"},
	{0, 4, v30emu_c3, 0, "jmp :2"},
	{OF_SKIPABLE, 5, v30emu_cc, 0, "cz :2"},
	{OF_SKIPABLE, 5, v30emu_cd, 0, "call :2"},
	{0, 4, v30emu_ce, 0, "aci :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 1"},

	{0, 5, v30emu_d0, 0, "rnc"},
	{0, 4, v30emu_d1, 0, "pop d"},
	{0, 4, v30emu_d2, 0, "jnc :2"},
	{0, 4, v30emu_d3, 0, "out :1"},
	{OF_SKIPABLE, 5, v30emu_d4, 0, "cnc :2"},
	{0, 5, v30emu_d5, 0, "push d"},
	{0, 4, v30emu_d6, 0, "sui :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 2"},

	{0, 5, v30emu_d8, 0, "rc"},
	{0, 4, v30emu_c9, 0, "ret"},
	{0, 4, v30emu_da, 0, "jc :2"},
	{0, 4, v30emu_db, 0, "in :1"},
	{OF_SKIPABLE, 5, v30emu_dc, 0, "cc :2"},
	{OF_SKIPABLE, 5, v30emu_cd, 0, "call :2"},
	{0, 4, v30emu_de, 0, "sbi :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 3"},

	{0, 5, v30emu_e0, 0, "rpo"},
	{0, 4, v30emu_e1, 0, "pop h"},
	{0, 4, v30emu_e2, 0, "jpo :2"},
	{0, 4, v30emu_e3, 0, "xthl"},
	{OF_SKIPABLE, 5, v30emu_e4, 0, "cpo :2"},
	{0, 5, v30emu_e5, 0, "push h"},
	{0, 4, v30emu_e6, 0, "ani :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 4"},

	{0, 5, v30emu_e8, 0, "rpe"},
	{0, 5, v30emu_e9, 0, "pchl"},
	{0, 4, v30emu_ea, 0, "jpe :2"},
	{0, 5, v30emu_eb, 0, "xchg"},
	{OF_SKIPABLE, 5, v30emu_ec, 0, "cpe :2"},
	{OF_PREFIX, 5, v30emu_ed, 0, "#ED"},
	{0, 4, v30emu_ee, 0, "xri :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 5"},

	{0, 5, v30emu_f0, 0, "rp"},
	{0, 4, v30emu_f1, 0, "pop psw"},
	{0, 4, v30emu_f2, 0, "jp :2"},
	{0, 4, v30emu_f3, 0, "di"},
	{OF_SKIPABLE, 5, v30emu_f4, 0, "cp :2"},
	{0, 5, v30emu_f5, 0, "push psw"},
	{0, 4, v30emu_f6, 0, "ori :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 6"},

	{0, 5, v30emu_f8, 0, "rm"},
	{0, 5, v30emu_f9, 0, "sphl"},
	{0, 4, v30emu_fa, 0, "jm :2"},
	{0, 4, v30emu_fb, 0, "ei"},
	{OF_SKIPABLE, 5, v30emu_fc, 0, "cm :2"},
	{OF_SKIPABLE, 5, v30emu_cd, 0, "call :2"},
	{0, 4, v30emu_fe, 0, "cpi :1"},
	{OF_SKIPABLE, 5, v30emu_rst, 0, "rst 7"},
};
