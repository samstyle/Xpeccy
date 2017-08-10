#include <stdlib.h>

#include "6502.h"
#include "6502_macro.h"

// cpu->mptr = address on misc addressing types
// cpu->tmp = operand
// IMM : next byte
#define mosGetImm(_op) _op = cpu->mrd(cpu->pc++, 0, cpu->data);

// ZP : peek(n) +1T
void mosGetZPw(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->hptr = 0;
}

void mosGetZP(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// ZPX : peek((X + n) & 0xff)
void mosGetZPXw(CPU* cpu) {
	cpu->tmp = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->mptr = (cpu->lx + cpu->tmp) & 0xff;
}

void mosGetZPX(CPU* cpu) {
	mosGetZPXw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// ZPY : peek((Y + n) & 0xff)
void mosGetZPYw(CPU* cpu) {
	cpu->tmp = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->mptr = (cpu->ly + cpu->tmp) & 0xff;
}

void mosGetZPY(CPU* cpu) {
	mosGetZPYw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// ABS: peek(nn)
void mosGetABSw(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
}

void mosGetABS(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// ABSX: peek(nn + x) ! +1T if nn high byte changed
void mosGetABSXw(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->tmp = cpu->hptr;
	cpu->mptr += cpu->lx;
	if (cpu->hptr != cpu->tmp)
		cpu->t++;
}

void mosGetABSX(CPU* cpu) {
	mosGetABSXw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// ABSY: peek(nn + y) ! +1T if nn high byte changed
void mosGetABSYw(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->tmp = cpu->hptr;
	cpu->mptr += cpu->ly;
	if (cpu->hptr != cpu->tmp)
		cpu->t++;
}

void mosGetABSY(CPU* cpu) {
	mosGetABSYw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// INDX: 4T
// val = PEEK(PEEK((arg + X) & 0xff) + PEEK((arg + X + 1) & 0xff) * 256)
void mosGetINDXw(CPU* cpu) {
	cpu->tmp = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->mptr = (cpu->tmp + cpu->lx) & 0xff;
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);	// low adr byte
	cpu->lptr++;					// hi byte didn't changed (00FF -> 0000)
	cpu->hptr = cpu->mrd(cpu->mptr, 0, cpu->data);	// hi adr byte
	cpu->lptr = cpu->tmp;				// mptr = address
}

void mosGetINDX(CPU* cpu) {
	mosGetINDXw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// INDY: 4(5)T
// val = PEEK(PEEK(arg) + PEEK((arg + 1) & 0xff) * 256 + Y)
void mosGetINDYw(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);		// zp adr
	cpu->hptr = 0;
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);		// pick abs.adr
	cpu->lptr++;
	cpu->hptr = cpu->mrd(cpu->mptr, 0, cpu->data);
	cpu->lptr = cpu->tmp;
	cpu->tmp = cpu->hptr;
	cpu->mptr += cpu->ly;
	if (cpu->tmp != cpu->hptr)
		cpu->t++;
}

void mosGetINDY(CPU* cpu) {
	mosGetINDYw(cpu);
	cpu->tmp = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// opcodes

// brk : 7T
void mosop00(CPU* cpu) {
	cpu->pc++;						// increment pc???
	cpu->f |= (MFB | 0x20);
	cpu->mwr(cpu->sp, cpu->hpc, cpu->data);
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->lpc, cpu->data);
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->f, cpu->data);
	cpu->lsp--;
	cpu->lpc = cpu->mrd(0xfffe, 0, cpu->data);
	cpu->hpc = cpu->mrd(0xffff, 0, cpu->data);
}

// 01:ora indx n : 6T
void mosop01(CPU* cpu) {
	mosGetINDX(cpu);
	MORA(cpu->tmp);
}

void mosop02(CPU* cpu) {
}

void mosop03(CPU* cpu) {
}

void mosop04(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop07(CPU* cpu) {
}

// php = push f
void mosop08(CPU* cpu) {
	cpu->f |= (MFB | 0x20);
	cpu->mwr(cpu->sp, cpu->f, cpu->data);
	cpu->lsp--;
}

// ora n
void mosop09(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MORA(cpu->tmp);
}

// asl a
void mosop0A(CPU* cpu) {
	MASL(cpu->a);
}

void mosop0B(CPU* cpu) {
}

void mosop0C(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop0F(CPU* cpu) {
}

// bpl e = jr p,e
void mosop10(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFN) return;
	MJR(cpu->tmp);
}

// ora ind,y n
void mosop11(CPU* cpu) {
	mosGetINDY(cpu);
	MORA(cpu->tmp);
}

void mosop12(CPU* cpu) {
}

void mosop13(CPU* cpu) {
}

void mosop14(CPU* cpu) {
}

// ora zp,x n
void mosop15(CPU* cpu) {
	mosGetZPX(cpu);
	MORA(cpu->tmp);
}

// asl zpx n
void mosop16(CPU* cpu) {
	mosGetZPX(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop17(CPU* cpu) {
}

// clc : C=0
void mosop18(CPU* cpu) {
	cpu->f &= ~MFC;
}

// ora abs,y nn
void mosop19(CPU* cpu) {
	mosGetABSY(cpu);
	MORA(cpu->tmp);
}

void mosop1A(CPU* cpu) {
}

void mosop1B(CPU* cpu) {
}

void mosop1C(CPU* cpu) {
}

// ora abs,x nn
void mosop1D(CPU* cpu) {
	mosGetABSX(cpu);
	MORA(cpu->tmp);
}

// asl absx nn
void mosop1E(CPU* cpu) {
	mosGetABSX(cpu);
	MASL(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop1F(CPU* cpu) {
}

// jsr nn = call nn
void mosop20(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);		// fetch low addr byte
	cpu->mwr(cpu->sp, cpu->hpc, cpu->data);			// push pch
	cpu->lsp--;
	cpu->mwr(cpu->sp, cpu->lpc, cpu->data);			// push pcl
	cpu->lsp--;
	cpu->hpc = cpu->mrd(cpu->pc, 0, cpu->data);		// fetch hi addr byte
	cpu->lpc = cpu->lptr;
}

// and ind,x n
void mosop21(CPU* cpu) {
	mosGetINDX(cpu);
	MAND(cpu->tmp);
}

void mosop22(CPU* cpu) {
}

void mosop23(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop27(CPU* cpu) {
}

// plp = pop f
void mosop28(CPU* cpu) {
	cpu->tmpb = cpu->mrd(cpu->pc, 0, cpu->data);
	cpu->lsp++;
	cpu->f = cpu->mrd(cpu->sp, 0 ,cpu->data);
}

// and n
void mosop29(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MAND(cpu->tmp);
}

// rol a
void mosop2A(CPU* cpu) {
	MROL(cpu->a);
}

void mosop2B(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop2F(CPU* cpu) {
}

// bmi e = jr m,e
void mosop30(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFN) {
		MJR(cpu->tmp);
	}
}

// and ind,y n
void mosop31(CPU* cpu) {
	mosGetINDY(cpu);
	MAND(cpu->tmp);
}

void mosop32(CPU* cpu) {
}

void mosop33(CPU* cpu) {
}

void mosop34(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop37(CPU* cpu) {
}

// sec
void mosop38(CPU* cpu) {
	cpu->f |= MFC;
}

// and abs,y nn
void mosop39(CPU* cpu) {
	mosGetABSY(cpu);
	MAND(cpu->tmp);
}

void mosop3A(CPU* cpu) {
}

void mosop3B(CPU* cpu) {
}

void mosop3C(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop3F(CPU* cpu) {
}

// rti
void mosop40(CPU* cpu) {
	cpu->lsp++;
	cpu->f = cpu->mrd(cpu->sp, 0, cpu->data);
	cpu->lsp++;
	cpu->lpc = cpu->mrd(cpu->sp, 0, cpu->data);
	cpu->lsp++;
	cpu->hpc = cpu->mrd(cpu->sp, 0, cpu->data);
}

// eor ind,x n
void mosop41(CPU* cpu) {
	mosGetINDX(cpu);
	MEOR(cpu->tmp);
}

void mosop42(CPU* cpu) {
}

void mosop43(CPU* cpu) {
}

void mosop44(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop47(CPU* cpu) {
}

// pha : push a
void mosop48(CPU* cpu) {
	cpu->mwr(cpu->sp, cpu->a, cpu->data);		// push a
	cpu->lsp--;
}

// eor n
void mosop49(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MEOR(cpu->tmp);
}

// lsr a
void mosop4A(CPU* cpu) {
	MLSR(cpu->a);
}

void mosop4B(CPU* cpu) {
}

// jmp nn
void mosop4C(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->hptr = cpu->mrd(cpu->pc, 0, cpu->data);
	cpu->pc = cpu->mptr;
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop4F(CPU* cpu) {
}

// bvc e (v = 0)
void mosop50(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFV) return;
	MJR(cpu->tmp);
}

// eor ind,y n
void mosop51(CPU* cpu) {
	mosGetINDY(cpu);
	MEOR(cpu->tmp);
}

void mosop52(CPU* cpu) {
}

void mosop53(CPU* cpu) {
}

void mosop54(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop57(CPU* cpu) {
}

// cli
void mosop58(CPU* cpu) {
	cpu->f &= ~MFI;
	cpu->noint = 1;
}

// eor abs,y nn
void mosop59(CPU* cpu) {
	mosGetABSY(cpu);
	MEOR(cpu->tmp);
}

void mosop5A(CPU* cpu) {
}

void mosop5B(CPU* cpu) {
}

void mosop5C(CPU* cpu) {
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
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop5F(CPU* cpu) {
}

// rts
void mosop60(CPU* cpu) {
	cpu->lsp++;
	cpu->lpc = cpu->mrd(cpu->sp, 0, cpu->data);		// pop pcl
	cpu->lsp++;
	cpu->hpc = cpu->mrd(cpu->sp, 0, cpu->data);		// pop pch
	cpu->pc++;						// inc pc, cuz of jsr push algorithm
}

// adc indx
void mosop61(CPU* cpu) {
	mosGetINDX(cpu);
	MADC(cpu->tmp);
}

void mosop62(CPU* cpu) {
}

void mosop63(CPU* cpu) {
}

void mosop64(CPU* cpu) {
}

// adc zp n
void mosop65(CPU* cpu) {
	mosGetZP(cpu);
	MADC(cpu->tmp);
}

// ror zp n
void mosop66(CPU* cpu) {
	mosGetZP(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop67(CPU* cpu) {
}

// pla = pop a
void mosop68(CPU* cpu) {
	cpu->tmpb = cpu->mrd(cpu->pc, 0, cpu->data);
	cpu->lsp++;
	cpu->a = cpu->mrd(cpu->sp, 0, cpu->data);
	MFLAGZN(cpu->a);
}

// adc n
void mosop69(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MADC(cpu->tmp);
}

// ror a
void mosop6A(CPU* cpu) {
	MROR(cpu->a);
}

void mosop6B(CPU* cpu) {
}

// jmp (nn)
void mosop6C(CPU* cpu) {
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->hptr = cpu->mrd(cpu->pc, 0, cpu->data);
	cpu->lpc = cpu->mrd(cpu->mptr, 0, cpu->data);
	cpu->lptr++;		// do not change segment
	cpu->hpc = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// adc abs nn
void mosop6D(CPU* cpu) {
	mosGetABS(cpu);
	MADC(cpu->tmp);
}

// ror abs nn
void mosop6E(CPU* cpu) {
	mosGetABS(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop6F(CPU* cpu) {
}

// bvs e (v = 1)
void mosop70(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFV) {
		MJR(cpu->tmp);
	}
}

// 71: adc ind,y n
void mosop71(CPU* cpu) {
	mosGetINDY(cpu);
	MADC(cpu->tmp);
}

void mosop72(CPU* cpu) {
}

void mosop73(CPU* cpu) {
}

void mosop74(CPU* cpu) {
}

// adc zp,x n
void mosop75(CPU* cpu) {
	mosGetZPX(cpu);
	MADC(cpu->tmp);
}

// ror zp,x n
void mosop76(CPU* cpu) {
	mosGetZPX(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop77(CPU* cpu) {
}

// sei
void mosop78(CPU* cpu) {
	cpu->f |= MFI;
}

// 79: adc abs,y nn
void mosop79(CPU* cpu) {
	mosGetABSY(cpu);
	MADC(cpu->tmp);
}

void mosop7A(CPU* cpu) {
}

void mosop7B(CPU* cpu) {
}

void mosop7C(CPU* cpu) {
}

// adc abs,x nn
void mosop7D(CPU* cpu) {
	mosGetABSX(cpu);
	MADC(cpu->tmp);
}

// ror abs,x nn
void mosop7E(CPU* cpu) {
	mosGetABSX(cpu);
	MROR(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosop7F(CPU* cpu) {
}

void mosop80(CPU* cpu) {
}

// sta ind,x n
void mosop81(CPU* cpu) {
	mosGetINDXw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

void mosop82(CPU* cpu) {
}

void mosop83(CPU* cpu) {
	mosGetINDXw(cpu);
	cpu->mwr(cpu->mptr, cpu->a & cpu->lx, cpu->data);
}

// sty zp n
void mosop84(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->mptr, cpu->ly, cpu->data);
}

// sta zp n
void mosop85(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

// stx zp n
void mosop86(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->mptr, cpu->lx, cpu->data);
}

void mosop87(CPU* cpu) {
	mosGetZPw(cpu);
	cpu->mwr(cpu->mptr, cpu->a & cpu->lx, cpu->data);
}

// dey
void mosop88(CPU* cpu) {
	cpu->ly--;
	MFLAGZN(cpu->ly);
//	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((cpu->ly & 0x80) ? MFN : 0) | (cpu->ly ? 0 : MFZ);
}

void mosop89(CPU* cpu) {
}

// txa
void mosop8A(CPU* cpu) {
	cpu->a = cpu->lx;
	MFLAGZN(cpu->a);
}

void mosop8B(CPU* cpu) {
}

// sty abs nn
void mosop8C(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->mptr, cpu->ly, cpu->data);
}

// sta abs nn
void mosop8D(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

// stx abs nn
void mosop8E(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->mptr, cpu->lx, cpu->data);
}

void mosop8F(CPU* cpu) {
	mosGetABSw(cpu);
	cpu->mwr(cpu->mptr, cpu->a & cpu->lx, cpu->data);
}

// bcc e = jr nc,e
void mosop90(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFC) return;
	MJR(cpu->tmp);
}

// sta ind,y n
void mosop91(CPU* cpu) {
	mosGetINDYw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

void mosop92(CPU* cpu) {
}

void mosop93(CPU* cpu) {
}

// sty zp,x n
void mosop94(CPU* cpu) {
	mosGetZPXw(cpu);
	cpu->mwr(cpu->mptr, cpu->ly, cpu->data);
}

// sta zp,x n
void mosop95(CPU* cpu) {
	mosGetZPXw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

// stx zp,y n
void mosop96(CPU* cpu) {
	mosGetZPYw(cpu);
	cpu->mwr(cpu->mptr, cpu->lx, cpu->data);
}

void mosop97(CPU* cpu) {
	mosGetZPYw(cpu);
	cpu->mwr(cpu->mptr, cpu->a & cpu->lx, cpu->data);
}

// tya
void mosop98(CPU* cpu) {
	cpu->a = cpu->ly;
	MFLAGZN(cpu->a);
}

// sta abs,y nn
void mosop99(CPU* cpu) {
	mosGetABSYw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

// txs
void mosop9A(CPU* cpu) {
	cpu->lsp = cpu->lx;
}

void mosop9B(CPU* cpu) {
}

void mosop9C(CPU* cpu) {
}

// sta abs,x nn
void mosop9D(CPU* cpu) {
	mosGetABSXw(cpu);
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

void mosop9E(CPU* cpu) {
}

void mosop9F(CPU* cpu) {
}

// ldy n
void mosopA0(CPU* cpu) {
	cpu->ly = cpu->mrd(cpu->pc++, 0, cpu->data);
	MFLAGZN(cpu->ly);
}

// lda ind,x n
void mosopA1(CPU* cpu) {
	mosGetINDX(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

// ldx n
void mosopA2(CPU* cpu) {
	cpu->lx = cpu->mrd(cpu->pc++, 0, cpu->data);
	MFLAGZN(cpu->lx);
}

void mosopA3(CPU* cpu) {
	mosGetINDX(cpu);
	cpu->a = cpu->tmp;
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// ldy zp n
void mosopA4(CPU* cpu) {
	mosGetZP(cpu);
	cpu->ly = cpu->tmp;
	MFLAGZN(cpu->ly);
}

// lda zp n
void mosopA5(CPU* cpu) {
	mosGetZP(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

// ldx zp n
void mosopA6(CPU* cpu) {
	mosGetZP(cpu);
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->lx);
}

void mosopA7(CPU* cpu) {
	mosGetZP(cpu);
	cpu->a = cpu->tmp;
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

//tay
void mosopA8(CPU* cpu) {
	cpu->ly = cpu->a;
	MFLAGZN(cpu->ly);
}

// lda n
void mosopA9(CPU* cpu) {
	cpu->a = cpu->mrd(cpu->pc++, 0, cpu->data);
	MFLAGZN(cpu->a);
}

// tax
void mosopAA(CPU* cpu) {
	cpu->lx = cpu->a;
	MFLAGZN(cpu->lx);
}

void mosopAB(CPU* cpu) {
}

// ldy abs nn
void mosopAC(CPU* cpu) {
	mosGetABS(cpu);
	cpu->ly = cpu->tmp;
	MFLAGZN(cpu->ly);
}

// lda abs nn
void mosopAD(CPU* cpu) {
	mosGetABS(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

// ldx abs nn
void mosopAE(CPU* cpu) {
	mosGetABS(cpu);
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->lx);
}

void mosopAF(CPU* cpu) {
	mosGetABS(cpu);
	cpu->a = cpu->tmp;
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// bcs e = jr c,e
void mosopB0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFC) {
		MJR(cpu->tmp);
	}
}

// lda ind,y n
void mosopB1(CPU* cpu) {
	mosGetINDY(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

void mosopB2(CPU* cpu) {
}

void mosopB3(CPU* cpu) {
	mosGetINDY(cpu);
	cpu->a = cpu->tmp;
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// ldy zp,x n
void mosopB4(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->ly = cpu->tmp;
	MFLAGZN(cpu->ly);
}

// lda zp,x n
void mosopB5(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

// ldx zp,y n
void mosopB6(CPU* cpu) {
	mosGetZPY(cpu);
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->lx);
}

void mosopB7(CPU* cpu) {
	mosGetZPY(cpu);
	cpu->a = cpu->tmp;
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->tmp);
}

// clv
void mosopB8(CPU* cpu) {
	cpu->f &= ~MFV;
}

// lda abs,y nn
void mosopB9(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

// tsx
void mosopBA(CPU* cpu) {
	cpu->lx = cpu->lsp;
	MFLAGZN(cpu->lx);
}

void mosopBB(CPU* cpu) {
}

// ldy abs,x nn
void mosopBC(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->ly = cpu->tmp;
	MFLAGZN(cpu->ly);
}

// lda abs,x nn
void mosopBD(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->a = cpu->tmp;
	MFLAGZN(cpu->a);
}

// ldx abs,y nn
void mosopBE(CPU* cpu) {
	mosGetABSY(cpu);
	cpu->lx = cpu->tmp;
	MFLAGZN(cpu->lx);
}

void mosopBF(CPU* cpu) {
}

// cpy n
void mosopC0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MCMP(cpu->ly, cpu->tmp);
}

// cmp ind,x n
void mosopC1(CPU* cpu) {
	mosGetINDX(cpu);
	MCMP(cpu->a, cpu->tmp);
}

void mosopC2(CPU* cpu) {
}

void mosopC3(CPU* cpu) {
}

// cpy zp n
void mosopC4(CPU* cpu) {
	mosGetZP(cpu);
	MCMP(cpu->ly, cpu->tmp);
}

// cmp zp n
void mosopC5(CPU* cpu) {
	mosGetZP(cpu);
	MCMP(cpu->a, cpu->tmp);
}

// dec zp n
void mosopC6(CPU* cpu) {
	mosGetZP(cpu);
	cpu->tmp--;
	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((cpu->tmp & 0x80) ? MFN : 0) | (cpu->tmp ? 0 : MFZ);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopC7(CPU* cpu) {
}

// iny
void mosopC8(CPU* cpu) {
	cpu->ly++;
	MFLAGZN(cpu->ly);
}

// cmp n
void mosopC9(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MCMP(cpu->a, cpu->tmp);
}

// dex
void mosopCA(CPU* cpu) {
	cpu->lx--;
	MFLAGZN(cpu->lx);
//	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((cpu->lx & 0x80) ? MFN : 0) | (cpu->lx ? 0 : MFZ);
}

void mosopCB(CPU* cpu) {
}

// cpy abs nn
void mosopCC(CPU* cpu) {
	mosGetABS(cpu);
	MCMP(cpu->ly, cpu->tmp);
}

// cmp abs nn
void mosopCD(CPU* cpu) {
	mosGetABS(cpu);
	MCMP(cpu->a, cpu->tmp);
}

// dec abs nn
void mosopCE(CPU* cpu) {
	mosGetABS(cpu);
	cpu->tmp--;
	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((cpu->tmp & 0x80) ? MFN : 0) | (cpu->tmp ? 0 : MFZ);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopCF(CPU* cpu) {
}

// bne e = jr nz,e
void mosopD0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFZ) return;
	MJR(cpu->tmp);
}

// cmp ind,y n
void mosopD1(CPU* cpu) {
	mosGetINDY(cpu);
	MCMP(cpu->a, cpu->tmp);
}

void mosopD2(CPU* cpu) {
}

void mosopD3(CPU* cpu) {
}

void mosopD4(CPU* cpu) {
}

// cmp zp,x n
void mosopD5(CPU* cpu) {
	mosGetZPX(cpu);
	MCMP(cpu->a, cpu->tmp);
}

// dec zp,x n
void mosopD6(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->tmp--;
	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((cpu->tmp & 0x80) ? MFN : 0) | (cpu->tmp ? 0 : MFZ);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopD7(CPU* cpu) {
}

// cld
void mosopD8(CPU* cpu) {
	cpu->f &= ~MFD;
}

// cmp abs,y nn
void mosopD9(CPU* cpu) {
	mosGetABSY(cpu);
	MCMP(cpu->a, cpu->tmp);
}

void mosopDA(CPU* cpu) {
}

void mosopDB(CPU* cpu) {
}

void mosopDC(CPU* cpu) {
}

// cmp abs,x nn
void mosopDD(CPU* cpu) {
	mosGetABSX(cpu);
	MCMP(cpu->a, cpu->tmp);
}

// dec abs,x nn
void mosopDE(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->tmp--;
	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((cpu->tmp & 0x80) ? MFN : 0) | (cpu->tmp ? 0 : MFZ);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopDF(CPU* cpu) {
}

// cpx n
void mosopE0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MCMP(cpu->lx, cpu->tmp);
}

// sbc ind,x n
void mosopE1(CPU* cpu) {
	mosGetINDX(cpu);
	MSBC(cpu->tmp);
}

void mosopE2(CPU* cpu) {
}

void mosopE3(CPU* cpu) {
}

// cpx zp n
void mosopE4(CPU* cpu) {
	mosGetZP(cpu);
	MCMP(cpu->lx, cpu->tmp);
}

// sbc zp n
void mosopE5(CPU* cpu) {
	mosGetZP(cpu);
	MSBC(cpu->tmp);
}

// inc zp n
void mosopE6(CPU* cpu) {
	mosGetZP(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopE7(CPU* cpu) {
}

// inx
void mosopE8(CPU* cpu) {
	cpu->lx++;
	MFLAGZN(cpu->lx);
}

// sbc n
void mosopE9(CPU* cpu) {
	mosGetImm(cpu->tmp);
	MSBC(cpu->tmp);
}

// nop
void mosopEA(CPU* cpu) {
}

void mosopEB(CPU* cpu) {
}

// cpx abs nn
void mosopEC(CPU* cpu) {
	mosGetABS(cpu);
	MCMP(cpu->lx, cpu->tmp);
}

// sbc abs nn
void mosopED(CPU* cpu) {
	mosGetABS(cpu);
	MSBC(cpu->tmp);
}

// inc abs nn
void mosopEE(CPU* cpu) {
	mosGetABS(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopEF(CPU* cpu) {
}

// beq e = jr z,e
void mosopF0(CPU* cpu) {
	mosGetImm(cpu->tmp);
	if (cpu->f & MFZ) {
		MJR(cpu->tmp);
	}
}

// sbc ind,y n
void mosopF1(CPU* cpu) {
	mosGetINDY(cpu);
	MSBC(cpu->tmp);
}

void mosopF2(CPU* cpu) {
}

void mosopF3(CPU* cpu) {
}

void mosopF4(CPU* cpu) {
}

// sbc zp,x n
void mosopF5(CPU* cpu) {
	mosGetZPX(cpu);
	MSBC(cpu->tmp);
}

// inc zp,x n
void mosopF6(CPU* cpu) {
	mosGetZPX(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopF7(CPU* cpu) {
}

// sed
void mosopF8(CPU* cpu) {
	cpu->f |= MFD;
}

// sbc abs,y nn
void mosopF9(CPU* cpu) {
	mosGetABSY(cpu);
	MSBC(cpu->tmp);
}

void mosopFA(CPU* cpu) {
}

void mosopFB(CPU* cpu) {
}

void mosopFC(CPU* cpu) {
}

// sbc abs,x nn
void mosopFD(CPU* cpu) {
	mosGetABSX(cpu);
	MSBC(cpu->tmp);
}

// inc abs,x nn
void mosopFE(CPU* cpu) {
	mosGetABSX(cpu);
	cpu->tmp++;
	MFLAGZN(cpu->tmp);
	cpu->mwr(cpu->mptr, cpu->tmp, cpu->data);
}

void mosopFF(CPU* cpu) {
}

opCode mosTab[256] = {
	{0,2,mosop00,NULL,"brk"},
	{0,6,mosop01,NULL,"ora ind,x :1"},
	{0,2,mosop02,NULL,"unknown"},
	{0,2,mosop03,NULL,"unknown"},
	{0,2,mosop04,NULL,"unknown"},
	{0,3,mosop05,NULL,"ora zp :1"},
	{0,5,mosop06,NULL,"asl zp :1"},
	{0,2,mosop07,NULL,"unknown"},

	{0,3,mosop08,NULL,"php"},
	{0,2,mosop09,NULL,"ora :1"},
	{0,2,mosop0A,NULL,"asl a"},
	{0,2,mosop0B,NULL,"unknown"},
	{0,2,mosop0C,NULL,"unknown"},
	{0,4,mosop0D,NULL,"ora abs :2"},
	{0,6,mosop0E,NULL,"asl abs :2"},
	{0,2,mosop0F,NULL,"unknown"},

	{0,2,mosop10,NULL,"bpl :3"},
	{0,5,mosop11,NULL,"ora ind,y :1"},
	{0,2,mosop12,NULL,"unknown"},
	{0,2,mosop13,NULL,"unknown"},
	{0,2,mosop14,NULL,"unknown"},
	{0,4,mosop15,NULL,"ora zp,x :1"},
	{0,6,mosop16,NULL,"asl zp,x :1"},
	{0,2,mosop17,NULL,"unknown"},

	{0,2,mosop18,NULL,"clc"},
	{0,4,mosop19,NULL,"ora abs,y :2"},
	{0,2,mosop1A,NULL,"unknown"},
	{0,2,mosop1B,NULL,"unknown"},
	{0,2,mosop1C,NULL,"unknown"},
	{0,4,mosop1D,NULL,"ora abs,x :2"},
	{0,7,mosop1E,NULL,"asl abs,x :2"},
	{0,2,mosop1F,NULL,"unknown"},

	{0,6,mosop20,NULL,"jsr :2"},
	{0,6,mosop21,NULL,"and ind,x :1"},
	{0,2,mosop22,NULL,"unknown"},
	{0,2,mosop23,NULL,"unknown"},
	{0,3,mosop24,NULL,"bit zp :1"},
	{0,3,mosop25,NULL,"and zp :1"},
	{0,5,mosop26,NULL,"rol zp :1"},
	{0,2,mosop27,NULL,"unknown"},

	{0,4,mosop28,NULL,"plp"},
	{0,2,mosop29,NULL,"and :1"},
	{0,2,mosop2A,NULL,"rol a"},
	{0,2,mosop2B,NULL,"unknown"},
	{0,4,mosop2C,NULL,"bit abs :2"},
	{0,4,mosop2D,NULL,"and abs :2"},
	{0,6,mosop2E,NULL,"rol abs :2"},
	{0,2,mosop2F,NULL,"unknown"},

	{0,2,mosop30,NULL,"bmi :3"},
	{0,5,mosop31,NULL,"and ind,y :1"},
	{0,2,mosop32,NULL,"unknown"},
	{0,2,mosop33,NULL,"unknown"},
	{0,2,mosop34,NULL,"unknown"},
	{0,4,mosop35,NULL,"and zp,x :1"},
	{0,6,mosop36,NULL,"rol zp,x :1"},
	{0,2,mosop37,NULL,"unknown"},

	{0,2,mosop38,NULL,"sec"},
	{0,4,mosop39,NULL,"and abs,y :2"},
	{0,2,mosop3A,NULL,"unknown"},
	{0,2,mosop3B,NULL,"unknown"},
	{0,2,mosop3C,NULL,"unknown"},
	{0,4,mosop3D,NULL,"and abs,x :2"},
	{0,7,mosop3E,NULL,"rol abs,x :2"},
	{0,2,mosop3F,NULL,"unknown"},

	{0,6,mosop40,NULL,"rti"},
	{0,6,mosop41,NULL,"eor ind,x :1"},
	{0,2,mosop42,NULL,"unknown"},
	{0,2,mosop43,NULL,"unknown"},
	{0,2,mosop44,NULL,"unknown"},
	{0,3,mosop45,NULL,"eor zp :1"},
	{0,5,mosop46,NULL,"lsr zp :1"},
	{0,2,mosop47,NULL,"unknown"},

	{0,3,mosop48,NULL,"pha"},
	{0,2,mosop49,NULL,"eor :1"},
	{0,2,mosop4A,NULL,"lsr a"},
	{0,2,mosop4B,NULL,"unknown"},
	{0,3,mosop4C,NULL,"jmp :2"},
	{0,4,mosop4D,NULL,"eor abs :2"},
	{0,6,mosop4E,NULL,"lsr abs :2"},
	{0,2,mosop4F,NULL,"unknown"},

	{0,2,mosop50,NULL,"bvc :3"},
	{0,5,mosop51,NULL,"eor ind,y :1"},
	{0,2,mosop52,NULL,"unknown"},
	{0,2,mosop53,NULL,"unknown"},
	{0,2,mosop54,NULL,"unknown"},
	{0,4,mosop55,NULL,"eor zp,x :1"},
	{0,6,mosop56,NULL,"lsr zp,x :1"},
	{0,2,mosop57,NULL,"unknown"},

	{0,2,mosop58,NULL,"cli"},
	{0,4,mosop59,NULL,"eor abs,y :2"},
	{0,2,mosop5A,NULL,"unknown"},
	{0,2,mosop5B,NULL,"unknown"},
	{0,2,mosop5C,NULL,"unknown"},
	{0,4,mosop5D,NULL,"eor abs,x :2"},
	{0,7,mosop5E,NULL,"lsr abs,x :2"},
	{0,2,mosop5F,NULL,"unknown"},

	{0,6,mosop60,NULL,"rts"},
	{0,6,mosop61,NULL,"adc ind,x :1"},
	{0,2,mosop62,NULL,"unknown"},
	{0,2,mosop63,NULL,"unknown"},
	{0,2,mosop64,NULL,"unknown"},
	{0,3,mosop65,NULL,"adc zp :1"},
	{0,5,mosop66,NULL,"ror zp :1"},
	{0,2,mosop67,NULL,"unknown"},

	{0,4,mosop68,NULL,"pla"},
	{0,2,mosop69,NULL,"adc :1"},
	{0,2,mosop6A,NULL,"ror a"},
	{0,2,mosop6B,NULL,"unknown"},
	{0,5,mosop6C,NULL,"jmp (:2)"},
	{0,4,mosop6D,NULL,"adc abs :2"},
	{0,6,mosop6E,NULL,"ror abs :2"},
	{0,2,mosop6F,NULL,"unknown"},

	{0,2,mosop70,NULL,"bvs :3"},
	{0,5,mosop71,NULL,"adc ind,y :1"},
	{0,2,mosop72,NULL,"unknown"},
	{0,2,mosop73,NULL,"unknown"},
	{0,2,mosop74,NULL,"unknown"},
	{0,4,mosop75,NULL,"adc zp,x :1"},
	{0,6,mosop76,NULL,"ror zp,x :1"},
	{0,2,mosop77,NULL,"unknown"},

	{0,2,mosop78,NULL,"sei"},
	{0,4,mosop79,NULL,"adc abs,y :2"},
	{0,2,mosop7A,NULL,"unknown"},
	{0,2,mosop7B,NULL,"unknown"},
	{0,2,mosop7C,NULL,"unknown"},
	{0,4,mosop7D,NULL,"adc abs,x :2"},
	{0,7,mosop7E,NULL,"ror abs,x :2"},
	{0,2,mosop7F,NULL,"unknown"},

	{0,2,mosop80,NULL,"unknown"},
	{0,6,mosop81,NULL,"sta ind,x :1"},
	{0,2,mosop82,NULL,"unknown"},
	{0,2,mosop83,NULL,"sax ind,x :1"},
	{0,3,mosop84,NULL,"sty zp :1"},
	{0,3,mosop85,NULL,"sta zp :1"},
	{0,3,mosop86,NULL,"stx zp :1"},
	{0,3,mosop87,NULL,"sax :1"},

	{0,2,mosop88,NULL,"dey"},
	{0,2,mosop89,NULL,"unknown"},
	{0,2,mosop8A,NULL,"txa"},
	{0,2,mosop8B,NULL,"unknown"},
	{0,4,mosop8C,NULL,"sty abs :2"},
	{0,4,mosop8D,NULL,"sta abs :2"},
	{0,4,mosop8E,NULL,"stx abs :2"},
	{0,4,mosop8F,NULL,"sax abs :2"},

	{0,2,mosop90,NULL,"bcc :3"},
	{0,6,mosop91,NULL,"sta ind,y :1"},
	{0,2,mosop92,NULL,"unknown"},
	{0,2,mosop93,NULL,"unknown"},
	{0,4,mosop94,NULL,"sty zp,x :1"},
	{0,4,mosop95,NULL,"sta zp,x :1"},
	{0,4,mosop96,NULL,"stx zp,y :1"},
	{0,4,mosop97,NULL,"sax zp,y :1"},

	{0,2,mosop98,NULL,"tya"},
	{0,5,mosop99,NULL,"sta abs,y :2"},
	{0,2,mosop9A,NULL,"txs"},
	{0,2,mosop9B,NULL,"unknown"},
	{0,2,mosop9C,NULL,"unknown"},
	{0,5,mosop9D,NULL,"sta abs,x :2"},
	{0,2,mosop9E,NULL,"unknown"},
	{0,2,mosop9F,NULL,"unknown"},

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
	{0,2,mosopAB,NULL,"unknown"},
	{0,4,mosopAC,NULL,"ldy abs :2"},
	{0,4,mosopAD,NULL,"lda abs :2"},
	{0,4,mosopAE,NULL,"ldx abs :2"},
	{0,4,mosopAF,NULL,"lax abs :2"},

	{0,2,mosopB0,NULL,"bcs :3"},
	{0,5,mosopB1,NULL,"lda ind,y :1"},
	{0,2,mosopB2,NULL,"unknown"},
	{0,5,mosopB3,NULL,"lax ind,y :1"},
	{0,4,mosopB4,NULL,"ldy zp,x :1"},
	{0,4,mosopB5,NULL,"lda zp,x :1"},
	{0,4,mosopB6,NULL,"ldx zp,y :1"},
	{0,4,mosopB7,NULL,"lax zp,y :1"},

	{0,2,mosopB8,NULL,"clv"},
	{0,4,mosopB9,NULL,"lda abs,y :2"},
	{0,2,mosopBA,NULL,"tsx"},
	{0,2,mosopBB,NULL,"unknown"},
	{0,4,mosopBC,NULL,"ldy abs,x :2"},
	{0,4,mosopBD,NULL,"lda abs,x :2"},
	{0,4,mosopBE,NULL,"ldx abs,y :2"},
	{0,2,mosopBF,NULL,"unknown"},

	{0,2,mosopC0,NULL,"cpy :1"},
	{0,6,mosopC1,NULL,"cmp ind,x :1"},
	{0,2,mosopC2,NULL,"unknown"},
	{0,2,mosopC3,NULL,"unknown"},
	{0,3,mosopC4,NULL,"cpy zp :1"},
	{0,3,mosopC5,NULL,"cmp zp :1"},
	{0,5,mosopC6,NULL,"dec zp n"},
	{0,2,mosopC7,NULL,"unknown"},

	{0,2,mosopC8,NULL,"iny"},
	{0,2,mosopC9,NULL,"cmp :1"},
	{0,2,mosopCA,NULL,"dex"},
	{0,2,mosopCB,NULL,"unknown"},
	{0,4,mosopCC,NULL,"cpy abs :2"},
	{0,4,mosopCD,NULL,"cmp abs :2"},
	{0,6,mosopCE,NULL,"dec abs :2"},
	{0,2,mosopCF,NULL,"unknown"},

	{0,2,mosopD0,NULL,"bne :3"},
	{0,5,mosopD1,NULL,"cmp ind,y :1"},
	{0,2,mosopD2,NULL,"unknown"},
	{0,2,mosopD3,NULL,"unknown"},
	{0,2,mosopD4,NULL,"unknown"},
	{0,4,mosopD5,NULL,"cmp zp,x :1"},
	{0,6,mosopD6,NULL,"dec zp,x :1"},
	{0,2,mosopD7,NULL,"unknown"},

	{0,2,mosopD8,NULL,"cld"},
	{0,4,mosopD9,NULL,"cmp abs,y :2"},
	{0,2,mosopDA,NULL,"unknown"},
	{0,2,mosopDB,NULL,"unknown"},
	{0,2,mosopDC,NULL,"unknown"},
	{0,4,mosopDD,NULL,"cmp abs,x :2"},
	{0,7,mosopDE,NULL,"dec abs,x :2"},
	{0,2,mosopDF,NULL,"unknown"},

	{0,2,mosopE0,NULL,"cpx :1"},
	{0,6,mosopE1,NULL,"sbc ind,x :1"},
	{0,2,mosopE2,NULL,"unknown"},
	{0,2,mosopE3,NULL,"unknown"},
	{0,3,mosopE4,NULL,"cpx zp :1"},
	{0,3,mosopE5,NULL,"sbc zp :1"},
	{0,5,mosopE6,NULL,"inc zp :1"},
	{0,2,mosopE7,NULL,"unknown"},

	{0,2,mosopE8,NULL,"inx"},
	{0,2,mosopE9,NULL,"sbc :1"},
	{0,2,mosopEA,NULL,"nop"},
	{0,2,mosopEB,NULL,"unknown"},
	{0,4,mosopEC,NULL,"cpx abs :2"},
	{0,4,mosopED,NULL,"sbc abs :2"},
	{0,6,mosopEE,NULL,"inc abs :2"},
	{0,2,mosopEF,NULL,"unknown"},

	{0,2,mosopF0,NULL,"beq :3"},
	{0,5,mosopF1,NULL,"sbc ind,y :1"},
	{0,2,mosopF2,NULL,"unknown"},
	{0,2,mosopF3,NULL,"unknown"},
	{0,2,mosopF4,NULL,"unknown"},
	{0,4,mosopF5,NULL,"sbc zp,x :1"},
	{0,6,mosopF6,NULL,"inc zp,x :1"},
	{0,2,mosopF7,NULL,"unknown"},

	{0,2,mosopF8,NULL,"sed"},
	{0,4,mosopF9,NULL,"sbc abs,y :2"},
	{0,2,mosopFA,NULL,"unknown"},
	{0,2,mosopFB,NULL,"unknown"},
	{0,2,mosopFC,NULL,"unknown"},
	{0,4,mosopFD,NULL,"sbc abs,x :2"},
	{0,7,mosopFE,NULL,"inc abs,x :2"},
	{0,2,mosopFF,NULL,"unknown"}
};
