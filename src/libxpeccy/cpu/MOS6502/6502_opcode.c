#include <stdlib.h>

#include "6502.h"
#include "6502_macro.h"

int m6502_pop(CPU*);
void m6502_push(CPU*, int);

// there is no T increment for fetch/rd/wr operations cuz full opcode T is already added in 'exec', before execution

// cpu->regWZ = address on misc addressing types (mptr)
// cpu->tmp = operand
// IMM : next byte
#define mosGetImm(_op) _op = cpu->mrd(cpu->regPC++, 0, cpu->xptr);

// ZP : peek(n) +1T
void mosGetZPw(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZh = 0;
}

void mosGetZP(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// ZPX : peek((X + n) & 0xff)
void mosGetZPXw(CPU* cpu) {
	cpu->tmp = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZ = (cpu->regX + cpu->tmp) & 0xff;
}

void mosGetZPX(CPU* cpu) {
	mosGetZPXw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// ZPY : peek((Y + n) & 0xff)
void mosGetZPYw(CPU* cpu) {
	cpu->tmp = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZ = (cpu->regY + cpu->tmp) & 0xff;
}

void mosGetZPY(CPU* cpu) {
	mosGetZPYw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// ABS: peek(nn)
void mosGetABSw(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
}

void mosGetABS(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// ABSX: peek(nn + x) ! +1T if nn high byte changed
void mosGetABSXw(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->tmp = cpu->regWZh;
	cpu->regWZ += cpu->regX;
	if ((cpu->regWZh != cpu->tmp) && !cpu->sta)
		cpu->t++;
}

void mosGetABSX(CPU* cpu) {
	mosGetABSXw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// ABSY: peek(nn + y) ! +1T if nn high byte changed
void mosGetABSYw(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZh = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->tmp = cpu->regWZh;
	cpu->regWZ += cpu->regY;
	if ((cpu->regWZh != cpu->tmp) && !cpu->sta)
		cpu->t++;
}

void mosGetABSY(CPU* cpu) {
	mosGetABSYw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// INDX: 4T
// val = PEEK(PEEK((arg + X) & 0xff) + PEEK((arg + X + 1) & 0xff) * 256)
void mosGetINDXw(CPU* cpu) {
	cpu->tmp = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZ = (cpu->tmp + cpu->regX) & 0xff;
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);	// low adr byte
	cpu->regWZl++;					// hi byte didn't changed (00FF -> 0000)
	cpu->regWZh = cpu->mrd(cpu->regWZ, 0, cpu->xptr);	// hi adr byte
	cpu->regWZl = cpu->tmp;				// mptr = address
}

void mosGetINDX(CPU* cpu) {
	mosGetINDXw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// INDY: 4(5)T
// val = PEEK(PEEK(arg) + PEEK((arg + 1) & 0xff) * 256 + Y)
void mosGetINDYw(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);		// zp adr
	cpu->regWZh = 0;
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);		// pick abs.adr
	cpu->regWZl++;
	cpu->regWZh = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
	cpu->regWZl = cpu->tmp;
	cpu->tmp = cpu->regWZh;
	cpu->regWZ += cpu->regY;
	if ((cpu->regWZh != cpu->tmp) && !cpu->sta)
		cpu->t++;
}

void mosGetINDY(CPU* cpu) {
	mosGetINDYw(cpu);
	cpu->tmp = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// opcodes

// brk
void mosop00(CPU* cpu) {
	m6502_push(cpu, cpu->regPCh);
	m6502_push(cpu, cpu->regPCl);
	int f = mos_get_flag(cpu);
	m6502_push(cpu, f | 0x10 | 0x20);		// set B flag, F5
	// mos_set_flag(cpu, cpu->ltw);			// where from?
	cpu->regPCl = cpu->mrd(0xfffe, 0, cpu->xptr);
	cpu->regPCh = cpu->mrd(0xffff, 0, cpu->xptr);
}

// ora ind,x n : 6T
void mosop01(CPU* cpu) {
	mosGetINDX(cpu);
	MORA(cpu->tmp);
}

void mosop02(CPU* cpu) {
	cpu->lock = 1;
}

// slo ind,x n
void mosop03(CPU* cpu) {
	mosGetINDX(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MORA(cpu->tmp);
}

// *nop zp n
void mosop04(CPU* cpu) {
	mosGetZP(cpu);
}

// ora zp n : 3T
void mosop05(CPU* cpu) {
	mosGetZP(cpu);
	MORA(cpu->tmp);
}

// asl zp,n
void mosop06(CPU* cpu) {
	mosGetZP(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// slo zp,n
// = asl zp n + ora zp n
void mosop07(CPU* cpu) {
	mosop06(cpu);
	MORA(cpu->tmp);
}

// php = push f
void mosop08(CPU* cpu) {
	int f = mos_get_flag(cpu);
	m6502_push(cpu, f | 0x10);		// push B flag 1, but don't change it in F
}

// ora n
void mosop09(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MORA(cpu->tmp);
}

// asl regA
void mosop0A(CPU* cpu) {
	MASL(cpu->regA);
}

// *anc n = A & n : N,Z flags, C=N
void mosop0B(CPU* cpu) {
	mosGetImm(cpu->tmp);
	cpu->tmp &= cpu->regA;
	MFLAGZN(cpu->tmp);
	//cpu->f = (cpu->f & ~MFC) | ((cpu->tmp & 0x80) ? MFC : 0);
	cpu->flgC = !!(cpu->tmp & 0x80);
}

// nop abs nn
void mosop0C(CPU* cpu) {
	mosGetABS(cpu);
}

// ora abs nn
void mosop0D(CPU* cpu) {
	mosGetABS(cpu);
	MORA(cpu->tmp);
}

// asl abs nn
void mosop0E(CPU* cpu) {
	mosGetABS(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// slo abs nn
void mosop0F(CPU* cpu) {
	mosop0E(cpu);
	MORA(cpu->tmp);
}

// bpl e = jr p,e
void mosop10(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFN) return;
	if (!cpu->flgN) {
		MJR(cpu->tmp);
	}
}

// ora ind,y n
void mosop11(CPU* cpu) {
	mosGetINDY(cpu);
	MORA(cpu->tmp);
}

void mosop12(CPU* cpu) {
	cpu->lock = 1;
}

// slo ind,y n
void mosop13(CPU* cpu) {
	mosGetINDY(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MORA(cpu->tmp);
}

// nop zp,x n
void mosop14(CPU* cpu) {
	mosGetZPX(cpu);
}

// ora zp,x n
void mosop15(CPU* cpu) {
	mosGetZPX(cpu);
	MORA(cpu->tmp);
}

// asl zp,x n
void mosop16(CPU* cpu) {
	mosGetZPX(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// slo zp,x n
void mosop17(CPU* cpu) {
	mosop16(cpu);
	MORA(cpu->tmp);
}

// clc : C=0
void mosop18(CPU* cpu) {
	//cpu->f &= ~MFC;
	cpu->flgC = 0;
}

// ora abs,y nn
void mosop19(CPU* cpu) {
	mosGetABSY(cpu);
	MORA(cpu->tmp);
}

void mosop1A(CPU* cpu) {
}

// slo abs,y nn
void mosop1B(CPU* cpu) {
	mosGetABSY(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MORA(cpu->tmp);
}

// nop abs,x nn
void mosop1C(CPU* cpu) {
	mosGetABSX(cpu);
}

// ora abs,x nn
void mosop1D(CPU* cpu) {
	mosGetABSX(cpu);
	MORA(cpu->tmp);
}

// asl abs,x nn
void mosop1E(CPU* cpu) {
	mosGetABSX(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// slo abs,x nn
void mosop1F(CPU* cpu) {
	mosop1E(cpu);
	MORA(cpu->tmp);
}

// jsr nn = call nn
void mosop20(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);		// fetch low addr byte
	m6502_push(cpu, cpu->regPCh);			// push pch
	m6502_push(cpu, cpu->regPCl);			// push pcl
	cpu->regPCh = cpu->mrd(cpu->regPC, 0, cpu->xptr);		// fetch hi addr byte
	cpu->regPCl = cpu->regWZl;
}

// and ind,x n
void mosop21(CPU* cpu) {
	mosGetINDX(cpu);
	MAND(cpu->tmp);
}

void mosop22(CPU* cpu) {
	cpu->lock = 1;
}

// rla ind,x n
// = rol + and
void mosop23(CPU* cpu) {
	mosGetINDX(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MAND(cpu->tmp);
}

// bit zp n
void mosop24(CPU* cpu) {
	mosGetZP(cpu);
	MBIT(cpu->tmp);
}

// and zp n
void mosop25(CPU* cpu) {
	mosGetZP(cpu);
	MAND(cpu->tmp);
}

// rol zp n
void mosop26(CPU* cpu) {
	mosGetZP(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rla zp n
void mosop27(CPU* cpu) {
	mosop26(cpu);
	MAND(cpu->tmp);
}

// plp = pop f
void mosop28(CPU* cpu) {
	cpu->tmpb = cpu->mrd(cpu->regPC, 0, cpu->xptr);
	mos_set_flag(cpu, m6502_pop(cpu));
	cpu->flgB = 0;
	cpu->flgF5 = 1;
}

// and n
void mosop29(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MAND(cpu->tmp);
}

// rol regA
void mosop2A(CPU* cpu) {
	MROL(cpu->regA);
}

// 2b = 0b = anc n
void mosop2B(CPU* cpu) {
	mosop0B(cpu);
}

// bit abs nn
void mosop2C(CPU* cpu) {
	mosGetABS(cpu);
	MBIT(cpu->tmp);
}

// and abs nn
void mosop2D(CPU* cpu) {
	mosGetABS(cpu);
	MAND(cpu->tmp);
}

// rol abs nn
void mosop2E(CPU* cpu) {
	mosGetABS(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rla abs nn
void mosop2F(CPU* cpu) {
	mosop2E(cpu);
	MAND(cpu->tmp);
}

// bmi e = jr m,e
void mosop30(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFN) {
	if (cpu->flgN) {
		MJR(cpu->tmp);
	}
}

// and ind,y n
void mosop31(CPU* cpu) {
	mosGetINDY(cpu);
	MAND(cpu->tmp);
}

void mosop32(CPU* cpu) {
	cpu->lock = 1;
}

// rla ind,y n
void mosop33(CPU* cpu) {
	mosGetINDY(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MAND(cpu->tmp);
}

// = 14
void mosop34(CPU* cpu) {
	mosop14(cpu);
}

// and zp,x n
void mosop35(CPU* cpu) {
	mosGetZPX(cpu);
	MAND(cpu->tmp);
}

// rol zp,x n
void mosop36(CPU* cpu) {
	mosGetZPX(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rla zp,x n
void mosop37(CPU* cpu) {
	mosop36(cpu);
	MAND(cpu->tmp);
}

// sec
void mosop38(CPU* cpu) {
	//cpu->f |= MFC;
	cpu->flgC = 1;
}

// and abs,y nn
void mosop39(CPU* cpu) {
	mosGetABSY(cpu);
	MAND(cpu->tmp);
}

void mosop3A(CPU* cpu) {
}

// rla abs,y nn
void mosop3B(CPU* cpu) {
	mosGetABSY(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MAND(cpu->tmp);
}

// = 1c
void mosop3C(CPU* cpu) {
	mosop1C(cpu);
}

// and abs,x nn
void mosop3D(CPU* cpu) {
	mosGetABSX(cpu);
	MAND(cpu->tmp);
}

// rol abs,x nn
void mosop3E(CPU* cpu) {
	mosGetABSX(cpu);
	MROL(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rla abs,x nn
void mosop3F(CPU* cpu) {
	mosop3E(cpu);
	MAND(cpu->tmp);
}

// rti
void mosop40(CPU* cpu) {
	mos_set_flag(cpu, m6502_pop(cpu)); //  cpu->mrd(cpu->regSP, 0, cpu->xptr));
	cpu->flgF5 = 1;
	cpu->regPCl = m6502_pop(cpu);
	cpu->regPCh = m6502_pop(cpu);
}

// eor ind,x n
void mosop41(CPU* cpu) {
	mosGetINDX(cpu);
	MEOR(cpu->tmp);
}

void mosop42(CPU* cpu) {
	cpu->lock = 1;
}

// sre ind,x n
void mosop43(CPU* cpu) {
	mosGetINDX(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MEOR(cpu->tmp);
}

// *nop zp n
void mosop44(CPU* cpu) {
	mosop04(cpu);
}

// eor zp n
void mosop45(CPU* cpu) {
	mosGetZP(cpu);
	MEOR(cpu->tmp);
}

// lsr zp n
void mosop46(CPU* cpu) {
	mosGetZP(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sre zp n
// = lsr + eor
void mosop47(CPU* cpu) {
	mosop46(cpu);
	MEOR(cpu->tmp);
}

// pha : push regA
void mosop48(CPU* cpu) {
	m6502_push(cpu, cpu->regA);		// push a
}

// eor n
void mosop49(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MEOR(cpu->tmp);
}

// lsr regA
void mosop4A(CPU* cpu) {
	MLSR(cpu->regA);
}

// *asr n
// regA &= n. regA>>1. N.Z.C flags
void mosop4B(CPU* cpu) {
	mosGetImm(cpu->tmp);
	cpu->regA &= cpu->tmp;
	MLSR(cpu->regA);
}

// jmp nn
void mosop4C(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZh = cpu->mrd(cpu->regPC, 0, cpu->xptr);
	cpu->regPC = cpu->regWZ;
}

// eor abs nn
void mosop4D(CPU* cpu) {
	mosGetABS(cpu);
	MEOR(cpu->tmp);
}

// lsr abs nn
void mosop4E(CPU* cpu) {
	mosGetABS(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sre abs nn
void mosop4F(CPU* cpu) {
	mosop4E(cpu);
	MEOR(cpu->tmp);
}

// bvc e (v = 0)
void mosop50(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFV) return;
	if (!cpu->flgV) {
		MJR(cpu->tmp);
	}
}

// eor ind,y n
void mosop51(CPU* cpu) {
	mosGetINDY(cpu);
	MEOR(cpu->tmp);
}

void mosop52(CPU* cpu) {
	cpu->lock = 1;
}

// sre ind,y n
void mosop53(CPU* cpu) {
	mosGetINDY(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MEOR(cpu->tmp);
}

// = 14
void mosop54(CPU* cpu) {
	mosop14(cpu);
}

// eor zp,x n
void mosop55(CPU* cpu) {
	mosGetZPX(cpu);
	MEOR(cpu->tmp);
}

// lsr zp,x n
void mosop56(CPU* cpu) {
	mosGetZPX(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sre zp,x n
void mosop57(CPU* cpu) {
	mosop56(cpu);
	MEOR(cpu->tmp);
}

// cli
void mosop58(CPU* cpu) {
//	cpu->f &= ~MFI;
	cpu->flgI = 0;
//	cpu->inten |= MOS6502_INT_IRQ;
	cpu->noint = 1;
}

// eor abs,y nn
void mosop59(CPU* cpu) {
	mosGetABSY(cpu);
	MEOR(cpu->tmp);
}

void mosop5A(CPU* cpu) {
}

// sre abs,y nn
void mosop5B(CPU* cpu) {
	mosGetABSY(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MEOR(cpu->tmp);
}

// = 1c
void mosop5C(CPU* cpu) {
	mosop1C(cpu);
}

// eor abs,x nn
void mosop5D(CPU* cpu) {
	mosGetABSX(cpu);
	MEOR(cpu->tmp);
}

// lsr abs,x nn
void mosop5E(CPU* cpu) {
	mosGetABSX(cpu);
	MLSR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sre abs,x nn
void mosop5F(CPU* cpu) {
	mosop5E(cpu);
	MEOR(cpu->tmp);
}

// rts
void mosop60(CPU* cpu) {
	cpu->regPCl = m6502_pop(cpu);		// pop pcl
	cpu->regPCh = m6502_pop(cpu);		// pop pch
	cpu->regPC++;				// inc pc, cuz of jsr push algorithm
}

// adc ind,x n
void mosop61(CPU* cpu) {
	mosGetINDX(cpu);
	MADC(cpu->regA, cpu->tmp);
}

void mosop62(CPU* cpu) {
	cpu->lock = 1;
}

// rra ind,x n
void mosop63(CPU* cpu) {
	mosGetINDX(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MADC(cpu->regA, cpu->tmp);
}

// *nop zp n
void mosop64(CPU* cpu) {
	mosop04(cpu);
}

// adc zp n
void mosop65(CPU* cpu) {
	mosGetZP(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// ror zp n
void mosop66(CPU* cpu) {
	mosGetZP(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rra zp n
// = ror + adc
void mosop67(CPU* cpu) {
	mosop66(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// pla = pop regA
void mosop68(CPU* cpu) {
	cpu->tmpb = cpu->mrd(cpu->regPC, 0, cpu->xptr);
	cpu->regA = m6502_pop(cpu);
	MFLAGZN(cpu->regA);
}

// adc n
void mosop69(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MADC(cpu->regA, cpu->tmp);
}

// ror regA
void mosop6A(CPU* cpu) {
	MROR(cpu->regA);
}

// *arr n
// A &= n; A <<= 1;
// V = b5 ^ b6, C = b6
void mosop6B(CPU* cpu) {
	mosGetImm(cpu->tmp);
	cpu->regA &= cpu->tmp;
	cpu->regA = (cpu->regA << 1) | ((cpu->regA & 0x80) ? 1 : 0);
	MFLAGZN(cpu->regA);
	//cpu->f &= ~(MFV | MFC);
	//cpu->flgV = 0;
	//cpu->flgC = 0;
	cpu->flgV = (((cpu->regA >> 6) ^ (cpu->regA >> 5)) & 1); // cpu->f |= MFV;
	cpu->flgC = !!(cpu->regA & 0x40);
	//if (cpu->a & 0x40) cpu->f |= MFC;
}

// jmp (nn)
void mosop6C(CPU* cpu) {
	cpu->regWZl = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	cpu->regWZh = cpu->mrd(cpu->regPC, 0, cpu->xptr);
	cpu->regPCl = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
	cpu->regWZl++;		// do not change segment
	cpu->regPCh = cpu->mrd(cpu->regWZ, 0, cpu->xptr);
}

// adc abs nn
void mosop6D(CPU* cpu) {
	mosGetABS(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// ror abs nn
void mosop6E(CPU* cpu) {
	mosGetABS(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rra abs nn
void mosop6F(CPU* cpu) {
	mosop6E(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// bvs e (v = 1)
void mosop70(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFV) {
	if (cpu->flgV) {
		MJR(cpu->tmp);
	}
}

// 71: adc ind,y n
void mosop71(CPU* cpu) {
	mosGetINDY(cpu);
	MADC(cpu->regA, cpu->tmp);
}

void mosop72(CPU* cpu) {
	cpu->lock = 1;
}

// rra ind,y n
void mosop73(CPU* cpu) {
	mosGetINDY(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MADC(cpu->regA, cpu->tmp);
}

// = 14
void mosop74(CPU* cpu) {
	mosop14(cpu);
}

// adc zp,x n
void mosop75(CPU* cpu) {
	mosGetZPX(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// ror zp,x n
void mosop76(CPU* cpu) {
	mosGetZPX(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rra zp,x n
void mosop77(CPU* cpu) {
	mosop76(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// sei
void mosop78(CPU* cpu) {
//	cpu->f |= MFI;
	cpu->flgI = 1;
//	cpu->inten &= ~MOS6502_INT_IRQ;
}

// 79: adc abs,y nn
void mosop79(CPU* cpu) {
	mosGetABSY(cpu);
	MADC(cpu->regA, cpu->tmp);
}

void mosop7A(CPU* cpu) {
}

// rra abs,y nn
void mosop7B(CPU* cpu) {
	mosGetABSY(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MADC(cpu->regA, cpu->tmp);
}

// = 1c
void mosop7C(CPU* cpu) {
	mosop1C(cpu);
}

// adc abs,x nn
void mosop7D(CPU* cpu) {
	mosGetABSX(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// ror abs,x nn
void mosop7E(CPU* cpu) {
	mosGetABSX(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// rra abs,x nn
void mosop7F(CPU* cpu) {
	mosop7E(cpu);
	MADC(cpu->regA, cpu->tmp);
}

// nop n
void mosop80(CPU* cpu) {
	mosGetImm(cpu->tmp);
}

// sta ind,x n
void mosop81(CPU* cpu) {
	mosGetINDXw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

void mosop82(CPU* cpu) {
}

// *sax ind,x n
void mosop83(CPU* cpu) {
	mosGetINDXw(cpu);
	cpu->tmp = cpu->regA & cpu->regX;
//	MFLAGZN(cpu->tmp);				// ??? SAX doesn't affect flags?
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sty zp n
void mosop84(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regY, cpu->xptr);
}

// sta zp n
void mosop85(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

// stx zp n
void mosop86(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regX, cpu->xptr);
}

// *sax zp n
void mosop87(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->tmp = cpu->regA & cpu->regX;
//	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// dey
void mosop88(CPU* cpu) {
	cpu->regY--;
	MFLAGZN(cpu->regY);
}

void mosop89(CPU* cpu) {
}

// txa
void mosop8A(CPU* cpu) {
	cpu->regA = cpu->regX;
	MFLAGZN(cpu->regA);
}

void mosop8B(CPU* cpu) {
}

// sty abs nn
void mosop8C(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regY, cpu->xptr);
}

// sta abs nn
void mosop8D(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

// stx abs nn
void mosop8E(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regX, cpu->xptr);
}

// *sax abs nn
void mosop8F(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->tmp = cpu->regA & cpu->regX;
//	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// bcc e = jr nc,e
void mosop90(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFC) return;
	if (!cpu->flgC) {
		MJR(cpu->tmp);
	}
}

// sta ind,y n
void mosop91(CPU* cpu) {
	mosGetINDYw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

void mosop92(CPU* cpu) {
	cpu->lock = 1;
}

// axa ind,y n
void mosop93(CPU* cpu) {
	cpu->regX &= cpu->regA;
	mosGetINDYw(cpu);
	cpu->tmp = cpu->regX & 7;
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sty zp,x n
void mosop94(CPU* cpu) {
	mosGetZPXw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regY, cpu->xptr);
}

// sta zp,x n
void mosop95(CPU* cpu) {
	mosGetZPXw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

// stx zp,y n
void mosop96(CPU* cpu) {
	mosGetZPYw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regX, cpu->xptr);
}

// *sax zp,y n
void mosop97(CPU* cpu) {
	mosGetZPYw(cpu);
	cpu->tmp = cpu->regA & cpu->regX;
//	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// tya
void mosop98(CPU* cpu) {
	cpu->regA = cpu->regY;
	MFLAGZN(cpu->regA);
}

// sta abs,y nn
void mosop99(CPU* cpu) {
	mosGetABSYw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

// txs (not affect flags)
void mosop9A(CPU* cpu) {
	cpu->regS = cpu->regX;
}

void mosop9B(CPU* cpu) {
}

// sya abs,x nn
// ld (abs,x), (Y & (high(abs,x) + 1))
void mosop9C(CPU* cpu) {
	mosGetABSXw(cpu);
	cpu->tmp = cpu->regY & (cpu->regWZh + 1);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// sta abs,x nn
void mosop9D(CPU* cpu) {
	mosGetABSXw(cpu);
	cpu->mwr(cpu->regWZ, cpu->regA, cpu->xptr);
}

// sxa abs,y nn
// ld (abs,y), (X & (high (abs,y) + 1))
void mosop9E(CPU* cpu) {
	mosGetABSYw(cpu);
	cpu->tmp = cpu->regX & (cpu->regWZh + 1);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// axa abs,y nn
// X &= A, mwr abs,y nn,(X & 7)
void mosop9F(CPU* cpu) {
	cpu->regX &= cpu->regA;
	mosGetABSYw(cpu);
	cpu->tmp = cpu->regX & 7;
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// ldy n
void mosopA0(CPU* cpu) {
	cpu->regY = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	MFLAGZN(cpu->regY);
}

// lda ind,x n
void mosopA1(CPU* cpu) {
	mosGetINDX(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

// ldx n
void mosopA2(CPU* cpu) {
	cpu->regX = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	MFLAGZN(cpu->regX);
}

void mosopA3(CPU* cpu) {
	mosGetINDX(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// ldy zp n
void mosopA4(CPU* cpu) {
	mosGetZP(cpu);
	cpu->regY = cpu->tmp;
	MFLAGZN(cpu->regY);
}

// lda zp n
void mosopA5(CPU* cpu) {
	mosGetZP(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

// ldx zp n
void mosopA6(CPU* cpu) {
	mosGetZP(cpu);
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->regX);
}

// lax zp n
void mosopA7(CPU* cpu) {
	mosGetZP(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

//tay
void mosopA8(CPU* cpu) {
	cpu->regY = cpu->regA;
	MFLAGZN(cpu->regY);
}

// lda n
void mosopA9(CPU* cpu) {
	cpu->regA = cpu->mrd(cpu->regPC++, 0, cpu->xptr);
	MFLAGZN(cpu->regA);
}

// tax
void mosopAA(CPU* cpu) {
	cpu->regX = cpu->regA;
	MFLAGZN(cpu->regX);
}

// *atx n: A &= n, X = A, N.Z
void mosopAB(CPU* cpu) {
	mosGetImm(cpu->tmp);
	cpu->tmp &= cpu->regA;
	cpu->regX = cpu->regA;
	MFLAGZN(cpu->tmp);
}

// ldy abs nn
void mosopAC(CPU* cpu) {
	mosGetABS(cpu);
	cpu->regY = cpu->tmp;
	MFLAGZN(cpu->regY);
}

// lda abs nn
void mosopAD(CPU* cpu) {
	mosGetABS(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

// ldx abs nn
void mosopAE(CPU* cpu) {
	mosGetABS(cpu);
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->regX);
}

void mosopAF(CPU* cpu) {
	mosGetABS(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// bcs e = jr c,e
void mosopB0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFC) {
	if (cpu->flgC) {
		MJR(cpu->tmp);
	}
}

// lda ind,y n
void mosopB1(CPU* cpu) {
	mosGetINDY(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

void mosopB2(CPU* cpu) {
	cpu->lock = 1;
}

void mosopB3(CPU* cpu) {
	mosGetINDY(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// ldy zp,x n
void mosopB4(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->regY = cpu->tmp;
	MFLAGZN(cpu->regY);
}

// lda zp,x n
void mosopB5(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

// ldx zp,y n
void mosopB6(CPU* cpu) {
	mosGetZPY(cpu);
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->regX);
}

void mosopB7(CPU* cpu) {
	mosGetZPY(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// clv
void mosopB8(CPU* cpu) {
	// cpu->f &= ~MFV;
	cpu->flgV = 0;
}

// lda abs,y nn
void mosopB9(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

// tsx
void mosopBA(CPU* cpu) {
	cpu->regX = cpu->regS;
	MFLAGZN(cpu->regX);
}

// lar abs,y nn
// = lda + ldx
void mosopBB(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// ldy abs,x nn
void mosopBC(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->regY = cpu->tmp;
	MFLAGZN(cpu->regY);
}

// lda abs,x nn
void mosopBD(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->regA = cpu->tmp;
	MFLAGZN(cpu->regA);
}

// ldx abs,y nn
void mosopBE(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->regX);
}

// lax abs,y nn
void mosopBF(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->regA = cpu->tmp;
	cpu->regX = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// cpy n
void mosopC0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MCMP(cpu->regY, cpu->tmp);
}

// cmp ind,x n
void mosopC1(CPU* cpu) {
	mosGetINDX(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

void mosopC2(CPU* cpu) {
}

// dcp ind,x n
void mosopC3(CPU* cpu) {
	mosGetINDX(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MCMP(cpu->regA, cpu->tmp);
}

// cpy zp n
void mosopC4(CPU* cpu) {
	mosGetZP(cpu);
	MCMP(cpu->regY, cpu->tmp);
}

// cmp zp n
void mosopC5(CPU* cpu) {
	mosGetZP(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// dec zp n
void mosopC6(CPU* cpu) {
	mosGetZP(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// *dcp zp n
// dec (mem) + cp (mem)
void mosopC7(CPU* cpu) {
	mosopC6(cpu);	// dec zp n
	MCMP(cpu->regA, cpu->tmp);
}

// iny
void mosopC8(CPU* cpu) {
	cpu->regY++;
	MFLAGZN(cpu->regY);
}

// cmp n
void mosopC9(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MCMP(cpu->regA, cpu->tmp);
}

// dex
void mosopCA(CPU* cpu) {
	cpu->regX--;
	MFLAGZN(cpu->regX);
}

// axs n
// X &= A, X -= n
void mosopCB(CPU* cpu) {
	mosGetImm(cpu->tmp);
	cpu->regX &= cpu->regA;
	MFLAGZN(cpu->regX);
	MSBC(cpu->regX, cpu->tmp);
}

// cpy abs nn
void mosopCC(CPU* cpu) {
	mosGetABS(cpu);
	MCMP(cpu->regY, cpu->tmp);
}

// cmp abs nn
void mosopCD(CPU* cpu) {
	mosGetABS(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// dec abs nn
void mosopCE(CPU* cpu) {
	mosGetABS(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// dcp abs nn
void mosopCF(CPU* cpu) {
	mosopCE(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// bne e = jr nz,e
void mosopD0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFZ) return;
	if (!cpu->flgZ) {
		MJR(cpu->tmp);
	}
}

// cmp ind,y n
void mosopD1(CPU* cpu) {
	mosGetINDY(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

void mosopD2(CPU* cpu) {
	cpu->lock = 1;
}

// dcp ind,y nn
void mosopD3(CPU* cpu) {
	mosGetINDY(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MCMP(cpu->regA, cpu->tmp);
}

// = 14
void mosopD4(CPU* cpu) {
	mosop14(cpu);
}

// cmp zp,x n
void mosopD5(CPU* cpu) {
	mosGetZPX(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// dec zp,x n
void mosopD6(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// dcp zp,x n
void mosopD7(CPU* cpu) {
	mosopD6(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// cld
void mosopD8(CPU* cpu) {
//	cpu->f &= ~MFD;
	cpu->flgD = 0;
}

// cmp abs,y nn
void mosopD9(CPU* cpu) {
	mosGetABSY(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

void mosopDA(CPU* cpu) {
}

// dcp abs,y nn
void mosopDB(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MCMP(cpu->regA, cpu->tmp);
}

// = 1c
void mosopDC(CPU* cpu) {
	mosop1C(cpu);
}

// cmp abs,x nn
void mosopDD(CPU* cpu) {
	mosGetABSX(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// dec abs,x nn
void mosopDE(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->tmp--;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// dcp abs,x nn
void mosopDF(CPU* cpu) {
	mosopDE(cpu);
	MCMP(cpu->regA, cpu->tmp);
}

// cpx n
void mosopE0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MCMP(cpu->regX, cpu->tmp);
}

// sbc ind,x n
void mosopE1(CPU* cpu) {
	mosGetINDX(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

void mosopE2(CPU* cpu) {
}

// isb ind,x n
void mosopE3(CPU* cpu) {
	mosGetINDX(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MSBC(cpu->regA, cpu->tmp);
}

// cpx zp n
void mosopE4(CPU* cpu) {
	mosGetZP(cpu);
	MCMP(cpu->regX, cpu->tmp);
}

// sbc zp n
void mosopE5(CPU* cpu) {
	mosGetZP(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// inc zp n
void mosopE6(CPU* cpu) {
	mosGetZP(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// isb zp n
// = inc zp n + sbc zp n
void mosopE7(CPU* cpu) {
	mosopE6(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// inx
void mosopE8(CPU* cpu) {
	cpu->regX++;
	MFLAGZN(cpu->regX);
}

// sbc n
void mosopE9(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MSBC(cpu->regA, cpu->tmp);
}

// nop
void mosopEA(CPU* cpu) {
}

// = e9
void mosopEB(CPU* cpu) {
	mosopE9(cpu);
}

// cpx abs nn
void mosopEC(CPU* cpu) {
	mosGetABS(cpu);
	MCMP(cpu->regX, cpu->tmp);
}

// sbc abs nn
void mosopED(CPU* cpu) {
	mosGetABS(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// inc abs nn
void mosopEE(CPU* cpu) {
	mosGetABS(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// isb abs nn
void mosopEF(CPU* cpu) {
	mosopEE(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// beq e = jr z,e
void mosopF0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	//if (cpu->f & MFZ) {
	if (cpu->flgZ) {
		MJR(cpu->tmp);
	}
}

// sbc ind,y n
void mosopF1(CPU* cpu) {
	mosGetINDY(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

void mosopF2(CPU* cpu) {
	cpu->lock = 1;
}

// isb ind,y n
void mosopF3(CPU* cpu) {
	mosGetINDY(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MSBC(cpu->regA, cpu->tmp);
}

// = 14
void mosopF4(CPU* cpu) {
	mosop14(cpu);
}

// sbc zp,x n
void mosopF5(CPU* cpu) {
	mosGetZPX(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// inc zp,x n
void mosopF6(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// isb zp,x n
void mosopF7(CPU* cpu) {
	mosopF6(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// sed
void mosopF8(CPU* cpu) {
	//cpu->f |= MFD;
	cpu->flgD = 1;
}

// sbc abs,y nn
void mosopF9(CPU* cpu) {
	mosGetABSY(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

void mosopFA(CPU* cpu) {
}

// isb abs,y nn
void mosopFB(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
	MSBC(cpu->regA, cpu->tmp);
}

// = 1c
void mosopFC(CPU* cpu) {
	mosop1C(cpu);
}

// sbc abs,x nn
void mosopFD(CPU* cpu) {
	mosGetABSX(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

// inc abs,x nn
void mosopFE(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->regWZ, cpu->tmp, cpu->xptr);
}

// isb abs,x nn
void mosopFF(CPU* cpu) {
	mosopFE(cpu);
	MSBC(cpu->regA, cpu->tmp);
}

opCode mosTab[256] = {
	{0,7,mosop00,NULL,"brk"},			// 7T!
	{0,6,mosop01,NULL,"ora ind,x :1"},
	{0,2,mosop02,NULL,"kil"},
	{0,8,mosop03,NULL,"slo ind,x :1"},
	{0,3,mosop04,NULL,"nop zp :1"},
	{0,3,mosop05,NULL,"ora zp :1"},
	{0,5,mosop06,NULL,"asl zp :1"},
	{0,5,mosop07,NULL,"slo zp :1"},

	{0,3,mosop08,NULL,"php"},
	{0,2,mosop09,NULL,"ora :1"},
	{0,2,mosop0A,NULL,"asl a"},
	{0,2,mosop0B,NULL,"anc :1"},
	{0,4,mosop0C,NULL,"nop abs :2"},
	{0,4,mosop0D,NULL,"ora abs :2"},
	{0,6,mosop0E,NULL,"asl abs :2"},
	{0,6,mosop0F,NULL,"slo abs :2"},

	{OF_RELJUMP,2,mosop10,NULL,"bpl :3"},
	{OF_EXT,5,mosop11,NULL,"ora ind,y :1"},
	{0,2,mosop12,NULL,"kil"},
	{OF_EXT,8,mosop13,NULL,"slo ind,y :1"},
	{0,4,mosop14,NULL,"nop zp,x :1"},
	{0,4,mosop15,NULL,"ora zp,x :1"},
	{0,6,mosop16,NULL,"asl zp,x :1"},
	{0,6,mosop17,NULL,"slo zp,x :1"},

	{0,2,mosop18,NULL,"clc"},
	{0,4,mosop19,NULL,"ora abs,y :2"},
	{0,2,mosop1A,NULL,"nop"},
	{OF_EXT,7,mosop1B,NULL,"slo abs,y :2"},
	{0,4,mosop1C,NULL,"nop abs,x :2"},
	{0,4,mosop1D,NULL,"ora abs,x :2"},
	{OF_EXT,7,mosop1E,NULL,"asl abs,x :2"},		// don't add 1T
	{OF_EXT,7,mosop1F,NULL,"slo abs,x :2"},

	{OF_SKIPABLE,6,mosop20,NULL,"jsr :2"},
	{0,6,mosop21,NULL,"and ind,x :1"},
	{0,2,mosop22,NULL,"kil"},
	{0,8,mosop23,NULL,"rla ind,x :1"},
	{0,3,mosop24,NULL,"bit zp :1"},
	{0,3,mosop25,NULL,"and zp :1"},
	{0,5,mosop26,NULL,"rol zp :1"},
	{0,5,mosop27,NULL,"rla zp :1"},

	{0,4,mosop28,NULL,"plp"},
	{0,2,mosop29,NULL,"and :1"},
	{0,2,mosop2A,NULL,"rol a"},
	{0,2,mosop2B,NULL,"anc :1"},
	{0,4,mosop2C,NULL,"bit abs :2"},
	{0,4,mosop2D,NULL,"and abs :2"},
	{0,6,mosop2E,NULL,"rol abs :2"},
	{0,6,mosop2F,NULL,"rla abs :2"},

	{OF_RELJUMP,2,mosop30,NULL,"bmi :3"},
	{OF_EXT,5,mosop31,NULL,"and ind,y :1"},		// don't add 1T on page boundary cross
	{0,2,mosop32,NULL,"kil"},
	{OF_EXT,8,mosop33,NULL,"rla ind,y :1"},
	{0,4,mosop34,NULL,"nop zp,x :1"},
	{0,4,mosop35,NULL,"and zp,x :1"},
	{0,6,mosop36,NULL,"rol zp,x :1"},
	{0,6,mosop37,NULL,"rla zp,x :1"},

	{0,2,mosop38,NULL,"sec"},
	{0,4,mosop39,NULL,"and abs,y :2"},
	{0,2,mosop3A,NULL,"nop"},
	{OF_EXT,7,mosop3B,NULL,"rla abs,y :2"},
	{0,4,mosop3C,NULL,"nop abs,x :2"},
	{0,4,mosop3D,NULL,"and abs,x :2"},
	{OF_EXT,7,mosop3E,NULL,"rol abs,x :2"},
	{OF_EXT,7,mosop3F,NULL,"rla abs,x :2"},

	{0,6,mosop40,NULL,"rti"},
	{0,6,mosop41,NULL,"eor ind,x :1"},
	{0,2,mosop42,NULL,"kil"},
	{0,8,mosop43,NULL,"sre ind,x :1"},
	{0,3,mosop44,NULL,"nop zp :1"},
	{0,3,mosop45,NULL,"eor zp :1"},
	{0,5,mosop46,NULL,"lsr zp :1"},
	{0,5,mosop47,NULL,"sre zp :1"},

	{0,3,mosop48,NULL,"pha"},
	{0,2,mosop49,NULL,"eor :1"},
	{0,2,mosop4A,NULL,"lsr a"},
	{0,2,mosop4B,NULL,"asr :1"},
	{0,3,mosop4C,NULL,"jmp :2"},
	{0,4,mosop4D,NULL,"eor abs :2"},
	{0,6,mosop4E,NULL,"lsr abs :2"},
	{0,6,mosop4F,NULL,"sre abs :2"},

	{OF_RELJUMP,2,mosop50,NULL,"bvc :3"},
	{0,5,mosop51,NULL,"eor ind,y :1"},
	{0,2,mosop52,NULL,"kil"},
	{OF_EXT,8,mosop53,NULL,"sre ind,y :1"},
	{0,4,mosop54,NULL,"nop zp,x :1"},
	{0,4,mosop55,NULL,"eor zp,x :1"},
	{0,6,mosop56,NULL,"lsr zp,x :1"},
	{0,6,mosop57,NULL,"sre zp,x :1"},

	{0,2,mosop58,NULL,"cli"},
	{0,4,mosop59,NULL,"eor abs,y :2"},
	{0,2,mosop5A,NULL,"nop"},
	{OF_EXT,7,mosop5B,NULL,"sre abs,y :2"},
	{0,4,mosop5C,NULL,"nop abs,x :2"},
	{0,4,mosop5D,NULL,"eor abs,x :2"},
	{OF_EXT,7,mosop5E,NULL,"lsr abs,x :2"},
	{OF_EXT,7,mosop5F,NULL,"sre abs,x :2"},

	{0,6,mosop60,NULL,"rts"},
	{0,6,mosop61,NULL,"adc ind,x :1"},
	{0,2,mosop62,NULL,"kil"},
	{0,8,mosop63,NULL,"rra ind,x :1"},
	{0,3,mosop64,NULL,"nop zp :1"},
	{0,3,mosop65,NULL,"adc zp :1"},
	{0,5,mosop66,NULL,"ror zp :1"},
	{0,5,mosop67,NULL,"rra zp :1"},

	{0,4,mosop68,NULL,"pla"},
	{0,2,mosop69,NULL,"adc :1"},
	{0,2,mosop6A,NULL,"ror a"},
	{0,2,mosop6B,NULL,"arr :1"},
	{0,5,mosop6C,NULL,"jmp (:2)"},
	{0,4,mosop6D,NULL,"adc abs :2"},
	{0,6,mosop6E,NULL,"ror abs :2"},
	{0,6,mosop6F,NULL,"rra abs :2"},

	{OF_RELJUMP,2,mosop70,NULL,"bvs :3"},
	{0,5,mosop71,NULL,"adc ind,y :1"},
	{0,2,mosop72,NULL,"kil"},
	{OF_EXT,8,mosop73,NULL,"rra ind,y :1"},
	{0,4,mosop74,NULL,"nop zp,x :1"},
	{0,4,mosop75,NULL,"adc zp,x :1"},
	{0,6,mosop76,NULL,"ror zp,x :1"},
	{0,6,mosop77,NULL,"rra zp,x :1"},

	{0,2,mosop78,NULL,"sei"},
	{0,4,mosop79,NULL,"adc abs,y :2"},
	{0,2,mosop7A,NULL,"nop"},
	{OF_EXT,7,mosop7B,NULL,"rra abs,y :2"},
	{0,4,mosop7C,NULL,"nop abs,x :2"},
	{0,4,mosop7D,NULL,"adc abs,x :2"},
	{OF_EXT,7,mosop7E,NULL,"ror abs,x :2"},
	{OF_EXT,7,mosop7F,NULL,"rra abs,x :2"},

	{0,2,mosop80,NULL,"nop :1"},
	{0,6,mosop81,NULL,"sta ind,x :1"},
	{0,2,mosop82,NULL,"unknown"},
	{0,6,mosop83,NULL,"sax ind,x :1"},
	{0,3,mosop84,NULL,"sty zp :1"},
	{0,3,mosop85,NULL,"sta zp :1"},
	{0,3,mosop86,NULL,"stx zp :1"},
	{0,3,mosop87,NULL,"sax zp :1"},

	{0,2,mosop88,NULL,"dey"},
	{0,2,mosop89,NULL,"unknown"},
	{0,2,mosop8A,NULL,"txa"},
	{0,2,mosop8B,NULL,"unknown"},
	{0,4,mosop8C,NULL,"sty abs :2"},
	{0,4,mosop8D,NULL,"sta abs :2"},
	{0,4,mosop8E,NULL,"stx abs :2"},
	{0,4,mosop8F,NULL,"sax abs :2"},

	{OF_RELJUMP,2,mosop90,NULL,"bcc :3"},
	{OF_EXT,6,mosop91,NULL,"sta ind,y :1"},
	{0,2,mosop92,NULL,"kil"},
	{0,6,mosop93,NULL,"axa ind,y :1"},
	{0,4,mosop94,NULL,"sty zp,x :1"},
	{0,4,mosop95,NULL,"sta zp,x :1"},
	{0,4,mosop96,NULL,"stx zp,y :1"},
	{0,4,mosop97,NULL,"sax zp,y :1"},

	{0,2,mosop98,NULL,"tya"},
	{OF_EXT,5,mosop99,NULL,"sta abs,y :2"},
	{0,2,mosop9A,NULL,"txs"},
	{0,2,mosop9B,NULL,"unknown"},
	{OF_EXT,5,mosop9C,NULL,"sya abs,x :2"},
	{OF_EXT,5,mosop9D,NULL,"sta abs,x :2"},
	{OF_EXT,5,mosop9E,NULL,"sxa abs,y :2"},
	{0,5,mosop9F,NULL,"axa abs,y :2"},

	{0,2,mosopA0,NULL,"ldy :1"},
	{0,6,mosopA1,NULL,"lda ind,x :1"},
	{0,2,mosopA2,NULL,"ldx :1"},
	{0,6,mosopA3,NULL,"lax ind,x :1"},
	{0,3,mosopA4,NULL,"ldy zp :1"},
	{0,3,mosopA5,NULL,"lda zp :1"},
	{0,3,mosopA6,NULL,"ldx zp :1"},
	{0,3,mosopA7,NULL,"lax zp :1"},

	{0,2,mosopA8,NULL,"tay"},
	{0,2,mosopA9,NULL,"lda :1"},
	{0,2,mosopAA,NULL,"tax"},
	{0,2,mosopAB,NULL,"atx :1"},
	{0,4,mosopAC,NULL,"ldy abs :2"},
	{0,4,mosopAD,NULL,"lda abs :2"},
	{0,4,mosopAE,NULL,"ldx abs :2"},
	{0,4,mosopAF,NULL,"lax abs :2"},

	{OF_RELJUMP,2,mosopB0,NULL,"bcs :3"},
	{0,5,mosopB1,NULL,"lda ind,y :1"},
	{0,2,mosopB2,NULL,"kil"},
	{0,5,mosopB3,NULL,"lax ind,y :1"},
	{0,4,mosopB4,NULL,"ldy zp,x :1"},
	{0,4,mosopB5,NULL,"lda zp,x :1"},
	{0,4,mosopB6,NULL,"ldx zp,y :1"},
	{0,4,mosopB7,NULL,"lax zp,y :1"},

	{0,2,mosopB8,NULL,"clv"},
	{0,4,mosopB9,NULL,"lda abs,y :2"},
	{0,2,mosopBA,NULL,"tsx"},
	{0,4,mosopBB,NULL,"lar abs,y :2"},
	{0,4,mosopBC,NULL,"ldy abs,x :2"},
	{0,4,mosopBD,NULL,"lda abs,x :2"},
	{0,4,mosopBE,NULL,"ldx abs,y :2"},
	{0,4,mosopBF,NULL,"lax abs,y :2"},

	{0,2,mosopC0,NULL,"cpy :1"},
	{0,6,mosopC1,NULL,"cmp ind,x :1"},
	{0,2,mosopC2,NULL,"unknown"},
	{0,8,mosopC3,NULL,"dcp ind,x :1"},
	{0,3,mosopC4,NULL,"cpy zp :1"},
	{0,3,mosopC5,NULL,"cmp zp :1"},
	{0,5,mosopC6,NULL,"dec zp :1"},
	{0,5,mosopC7,NULL,"dcp zp :1"},

	{0,2,mosopC8,NULL,"iny"},
	{0,2,mosopC9,NULL,"cmp :1"},
	{0,2,mosopCA,NULL,"dex"},
	{0,2,mosopCB,NULL,"axs :1"},
	{0,4,mosopCC,NULL,"cpy abs :2"},
	{0,4,mosopCD,NULL,"cmp abs :2"},
	{0,6,mosopCE,NULL,"dec abs :2"},
	{0,6,mosopCF,NULL,"dcp abs :2"},

	{OF_RELJUMP,2,mosopD0,NULL,"bne :3"},
	{0,5,mosopD1,NULL,"cmp ind,y :1"},
	{0,2,mosopD2,NULL,"kil"},
	{OF_EXT,8,mosopD3,NULL,"dcp ind,y :1"},
	{0,4,mosopD4,NULL,"nop zp,x :1"},
	{0,4,mosopD5,NULL,"cmp zp,x :1"},
	{0,6,mosopD6,NULL,"dec zp,x :1"},
	{0,6,mosopD7,NULL,"dcp zp,x :1"},

	{0,2,mosopD8,NULL,"cld"},
	{0,4,mosopD9,NULL,"cmp abs,y :2"},
	{0,2,mosopDA,NULL,"nop"},
	{OF_EXT,7,mosopDB,NULL,"dcp abs,y :2"},
	{0,4,mosopDC,NULL,"nop abs,x :2"},
	{0,4,mosopDD,NULL,"cmp abs,x :2"},
	{OF_EXT,7,mosopDE,NULL,"dec abs,x :2"},
	{OF_EXT,7,mosopDF,NULL,"dcp abs,x :2"},

	{0,2,mosopE0,NULL,"cpx :1"},
	{0,6,mosopE1,NULL,"sbc ind,x :1"},
	{0,2,mosopE2,NULL,"unknown"},
	{0,8,mosopE3,NULL,"isb ind,x :1"},
	{0,3,mosopE4,NULL,"cpx zp :1"},
	{0,3,mosopE5,NULL,"sbc zp :1"},
	{0,5,mosopE6,NULL,"inc zp :1"},
	{0,5,mosopE7,NULL,"isb zp :1"},

	{0,2,mosopE8,NULL,"inx"},
	{0,2,mosopE9,NULL,"sbc :1"},
	{0,2,mosopEA,NULL,"nop"},
	{0,2,mosopEB,NULL,"sbc :1"},
	{0,4,mosopEC,NULL,"cpx abs :2"},
	{0,4,mosopED,NULL,"sbc abs :2"},
	{0,6,mosopEE,NULL,"inc abs :2"},
	{0,6,mosopEF,NULL,"isb abs :2"},

	{OF_RELJUMP,2,mosopF0,NULL,"beq :3"},
	{OF_EXT,5,mosopF1,NULL,"sbc ind,y :1"},
	{0,2,mosopF2,NULL,"kil"},
	{OF_EXT,8,mosopF3,NULL,"isb ind,y :1"},
	{0,4,mosopF4,NULL,"nop zp,x :1"},
	{0,4,mosopF5,NULL,"sbc zp,x :1"},
	{0,6,mosopF6,NULL,"inc zp,x :1"},
	{0,6,mosopF7,NULL,"isb zp,x :1"},

	{0,2,mosopF8,NULL,"sed"},
	{0,4,mosopF9,NULL,"sbc abs,y :2"},
	{0,2,mosopFA,NULL,"nop"},
	{OF_EXT,7,mosopFB,NULL,"isb abs,y :2"},
	{0,4,mosopFC,NULL,"nop abs,x :2"},
	{0,4,mosopFD,NULL,"sbc abs,x :2"},
	{OF_EXT,7,mosopFE,NULL,"inc abs,x :2"},
	{OF_EXT,7,mosopFF,NULL,"isb abs,x :2"}
};
