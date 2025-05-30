#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"

// rlc reg,(regIY+e)	[3rd] 4 5add 4rd 3wr
void fdcb00(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regB);}
void fdcb01(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regC);}
void fdcb02(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regD);}
void fdcb03(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regE);}
void fdcb04(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regH);}
void fdcb05(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regL);}
void fdcb06(CPU* cpu) {XDCB(cpu->regIY,z80_rlc);}
void fdcb07(CPU* cpu) {XDCBR(cpu->regIY,z80_rlc,cpu->regA);}
// rrc reg,(regIY+e)
void fdcb08(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regB);}
void fdcb09(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regC);}
void fdcb0A(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regD);}
void fdcb0B(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regE);}
void fdcb0C(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regH);}
void fdcb0D(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regL);}
void fdcb0E(CPU* cpu) {XDCB(cpu->regIY,z80_rrc);}
void fdcb0F(CPU* cpu) {XDCBR(cpu->regIY,z80_rrc,cpu->regA);}
// rl reg,(regIY+e)
void fdcb10(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regB);}
void fdcb11(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regC);}
void fdcb12(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regD);}
void fdcb13(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regE);}
void fdcb14(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regH);}
void fdcb15(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regL);}
void fdcb16(CPU* cpu) {XDCB(cpu->regIY,z80_rl);}
void fdcb17(CPU* cpu) {XDCBR(cpu->regIY,z80_rl,cpu->regA);}
// rr reg,(regIY+e)
void fdcb18(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regB);}
void fdcb19(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regC);}
void fdcb1A(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regD);}
void fdcb1B(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regE);}
void fdcb1C(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regH);}
void fdcb1D(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regL);}
void fdcb1E(CPU* cpu) {XDCB(cpu->regIY,z80_rr);}
void fdcb1F(CPU* cpu) {XDCBR(cpu->regIY,z80_rr,cpu->regA);}
// sla reg,(regIY+e)
void fdcb20(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regB);}
void fdcb21(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regC);}
void fdcb22(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regD);}
void fdcb23(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regE);}
void fdcb24(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regH);}
void fdcb25(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regL);}
void fdcb26(CPU* cpu) {XDCB(cpu->regIY,z80_sla);}
void fdcb27(CPU* cpu) {XDCBR(cpu->regIY,z80_sla,cpu->regA);}
// sra reg,(regIY+e)
void fdcb28(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regB);}
void fdcb29(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regC);}
void fdcb2A(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regD);}
void fdcb2B(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regE);}
void fdcb2C(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regH);}
void fdcb2D(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regL);}
void fdcb2E(CPU* cpu) {XDCB(cpu->regIY,z80_sra);}
void fdcb2F(CPU* cpu) {XDCBR(cpu->regIY,z80_sra,cpu->regA);}
// sll reg,(regIY+e)
void fdcb30(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regB);}
void fdcb31(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regC);}
void fdcb32(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regD);}
void fdcb33(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regE);}
void fdcb34(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regH);}
void fdcb35(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regL);}
void fdcb36(CPU* cpu) {XDCB(cpu->regIY,z80_sll);}
void fdcb37(CPU* cpu) {XDCBR(cpu->regIY,z80_sll,cpu->regA);}
// srl reg,(regIY+e)
void fdcb38(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regB);}
void fdcb39(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regC);}
void fdcb3A(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regD);}
void fdcb3B(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regE);}
void fdcb3C(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regH);}
void fdcb3D(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regL);}
void fdcb3E(CPU* cpu) {XDCB(cpu->regIY,z80_srl);}
void fdcb3F(CPU* cpu) {XDCBR(cpu->regIY,z80_srl,cpu->regA);}

// bit n,(regIY+e)
void fdcb46(CPU* cpu) {BITX(cpu->regIY,0);}
void fdcb4E(CPU* cpu) {BITX(cpu->regIY,1);}
void fdcb56(CPU* cpu) {BITX(cpu->regIY,2);}
void fdcb5E(CPU* cpu) {BITX(cpu->regIY,3);}
void fdcb66(CPU* cpu) {BITX(cpu->regIY,4);}
void fdcb6E(CPU* cpu) {BITX(cpu->regIY,5);}
void fdcb76(CPU* cpu) {BITX(cpu->regIY,6);}
void fdcb7E(CPU* cpu) {BITX(cpu->regIY,7);}

// res 0,reg,(regIY+e)
void fdcb80(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regB);}
void fdcb81(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regC);}
void fdcb82(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regD);}
void fdcb83(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regE);}
void fdcb84(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regH);}
void fdcb85(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regL);}
void fdcb86(CPU* cpu) {RESX(cpu->regIY,0);}
void fdcb87(CPU* cpu) {RESXR(cpu->regIY,0,cpu->regA);}
// res 1,reg,(regIY+e)
void fdcb88(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regB);}
void fdcb89(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regC);}
void fdcb8A(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regD);}
void fdcb8B(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regE);}
void fdcb8C(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regH);}
void fdcb8D(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regL);}
void fdcb8E(CPU* cpu) {RESX(cpu->regIY,1);}
void fdcb8F(CPU* cpu) {RESXR(cpu->regIY,1,cpu->regA);}
// res 2,reg,(regIY+e)
void fdcb90(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regB);}
void fdcb91(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regC);}
void fdcb92(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regD);}
void fdcb93(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regE);}
void fdcb94(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regH);}
void fdcb95(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regL);}
void fdcb96(CPU* cpu) {RESX(cpu->regIY,2);}
void fdcb97(CPU* cpu) {RESXR(cpu->regIY,2,cpu->regA);}
// res 3,reg,(regIY+e)
void fdcb98(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regB);}
void fdcb99(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regC);}
void fdcb9A(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regD);}
void fdcb9B(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regE);}
void fdcb9C(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regH);}
void fdcb9D(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regL);}
void fdcb9E(CPU* cpu) {RESX(cpu->regIY,3);}
void fdcb9F(CPU* cpu) {RESXR(cpu->regIY,3,cpu->regA);}
// res 4,reg,(regIY+e)
void fdcbA0(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regB);}
void fdcbA1(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regC);}
void fdcbA2(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regD);}
void fdcbA3(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regE);}
void fdcbA4(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regH);}
void fdcbA5(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regL);}
void fdcbA6(CPU* cpu) {RESX(cpu->regIY,4);}
void fdcbA7(CPU* cpu) {RESXR(cpu->regIY,4,cpu->regA);}
// res 5,reg,(regIY+e)
void fdcbA8(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regB);}
void fdcbA9(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regC);}
void fdcbAA(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regD);}
void fdcbAB(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regE);}
void fdcbAC(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regH);}
void fdcbAD(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regL);}
void fdcbAE(CPU* cpu) {RESX(cpu->regIY,5);}
void fdcbAF(CPU* cpu) {RESXR(cpu->regIY,5,cpu->regA);}
// res 6,reg,(regIY+e)
void fdcbB0(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regB);}
void fdcbB1(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regC);}
void fdcbB2(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regD);}
void fdcbB3(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regE);}
void fdcbB4(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regH);}
void fdcbB5(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regL);}
void fdcbB6(CPU* cpu) {RESX(cpu->regIY,6);}
void fdcbB7(CPU* cpu) {RESXR(cpu->regIY,6,cpu->regA);}
// res 7,reg,(regIY+e)
void fdcbB8(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regB);}
void fdcbB9(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regC);}
void fdcbBA(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regD);}
void fdcbBB(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regE);}
void fdcbBC(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regH);}
void fdcbBD(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regL);}
void fdcbBE(CPU* cpu) {RESX(cpu->regIY,7);}
void fdcbBF(CPU* cpu) {RESXR(cpu->regIY,7,cpu->regA);}

// set 0,reg,(regIY+e)
void fdcbC0(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regB);}
void fdcbC1(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regC);}
void fdcbC2(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regD);}
void fdcbC3(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regE);}
void fdcbC4(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regH);}
void fdcbC5(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regL);}
void fdcbC6(CPU* cpu) {SETX(cpu->regIY,0);}
void fdcbC7(CPU* cpu) {SETXR(cpu->regIY,0,cpu->regA);}
// set 1,reg,(regIY+e)
void fdcbC8(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regB);}
void fdcbC9(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regC);}
void fdcbCA(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regD);}
void fdcbCB(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regE);}
void fdcbCC(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regH);}
void fdcbCD(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regL);}
void fdcbCE(CPU* cpu) {SETX(cpu->regIY,1);}
void fdcbCF(CPU* cpu) {SETXR(cpu->regIY,1,cpu->regA);}
// set 2,reg,(regIY+e)
void fdcbD0(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regB);}
void fdcbD1(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regC);}
void fdcbD2(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regD);}
void fdcbD3(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regE);}
void fdcbD4(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regH);}
void fdcbD5(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regL);}
void fdcbD6(CPU* cpu) {SETX(cpu->regIY,2);}
void fdcbD7(CPU* cpu) {SETXR(cpu->regIY,2,cpu->regA);}
// set 3,reg,(regIY+e)
void fdcbD8(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regB);}
void fdcbD9(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regC);}
void fdcbDA(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regD);}
void fdcbDB(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regE);}
void fdcbDC(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regH);}
void fdcbDD(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regL);}
void fdcbDE(CPU* cpu) {SETX(cpu->regIY,3);}
void fdcbDF(CPU* cpu) {SETXR(cpu->regIY,3,cpu->regA);}
// set 4,reg,(regIY+e)
void fdcbE0(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regB);}
void fdcbE1(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regC);}
void fdcbE2(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regD);}
void fdcbE3(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regE);}
void fdcbE4(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regH);}
void fdcbE5(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regL);}
void fdcbE6(CPU* cpu) {SETX(cpu->regIY,4);}
void fdcbE7(CPU* cpu) {SETXR(cpu->regIY,4,cpu->regA);}
// set 5,reg,(regIY+e)
void fdcbE8(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regB);}
void fdcbE9(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regC);}
void fdcbEA(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regD);}
void fdcbEB(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regE);}
void fdcbEC(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regH);}
void fdcbED(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regL);}
void fdcbEE(CPU* cpu) {SETX(cpu->regIY,5);}
void fdcbEF(CPU* cpu) {SETXR(cpu->regIY,5,cpu->regA);}
// set 6,reg,(regIY+e)
void fdcbF0(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regB);}
void fdcbF1(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regC);}
void fdcbF2(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regD);}
void fdcbF3(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regE);}
void fdcbF4(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regH);}
void fdcbF5(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regL);}
void fdcbF6(CPU* cpu) {SETX(cpu->regIY,6);}
void fdcbF7(CPU* cpu) {SETXR(cpu->regIY,6,cpu->regA);}
// set 7,reg,(regIY+e)
void fdcbF8(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regB);}
void fdcbF9(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regC);}
void fdcbFA(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regD);}
void fdcbFB(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regE);}
void fdcbFC(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regH);}
void fdcbFD(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regL);}
void fdcbFE(CPU* cpu) {SETX(cpu->regIY,7);}
void fdcbFF(CPU* cpu) {SETXR(cpu->regIY,7,cpu->regA);}

//====
// opcode fetch doesn't eat 4T?

opCode fdcbTab[256]={
	{0,0,fdcb00,NULL,"rlc b,(iy:5)"},
	{0,0,fdcb01,NULL,"rlc c,(iy:5)"},
	{0,0,fdcb02,NULL,"rlc d,(iy:5)"},
	{0,0,fdcb03,NULL,"rlc e,(iy:5)"},
	{0,0,fdcb04,NULL,"rlc h,(iy:5)"},
	{0,0,fdcb05,NULL,"rlc l,(iy:5)"},
	{0,0,fdcb06,NULL,"rlc (iy:5)"},
	{0,0,fdcb07,NULL,"rlc a,(iy:5)"},

	{0,0,fdcb08,NULL,"rrc b,(iy:5)"},
	{0,0,fdcb09,NULL,"rrc c,(iy:5)"},
	{0,0,fdcb0A,NULL,"rrc d,(iy:5)"},
	{0,0,fdcb0B,NULL,"rrc e,(iy:5)"},
	{0,0,fdcb0C,NULL,"rrc h,(iy:5)"},
	{0,0,fdcb0D,NULL,"rrc l,(iy:5)"},
	{0,0,fdcb0E,NULL,"rrc (iy:5)"},
	{0,0,fdcb0F,NULL,"rrc a,(iy:5)"},

	{0,0,fdcb10,NULL,"rl b,(iy:5)"},
	{0,0,fdcb11,NULL,"rl c,(iy:5)"},
	{0,0,fdcb12,NULL,"rl d,(iy:5)"},
	{0,0,fdcb13,NULL,"rl e,(iy:5)"},
	{0,0,fdcb14,NULL,"rl h,(iy:5)"},
	{0,0,fdcb15,NULL,"rl l,(iy:5)"},
	{0,0,fdcb16,NULL,"rl (iy:5)"},
	{0,0,fdcb17,NULL,"rl a,(iy:5)"},

	{0,0,fdcb18,NULL,"rr b,(iy:5)"},
	{0,0,fdcb19,NULL,"rr c,(iy:5)"},
	{0,0,fdcb1A,NULL,"rr d,(iy:5)"},
	{0,0,fdcb1B,NULL,"rr e,(iy:5)"},
	{0,0,fdcb1C,NULL,"rr h,(iy:5)"},
	{0,0,fdcb1D,NULL,"rr l,(iy:5)"},
	{0,0,fdcb1E,NULL,"rr (iy:5)"},
	{0,0,fdcb1F,NULL,"rr a,(iy:5)"},

	{0,0,fdcb20,NULL,"sla b,(iy:5)"},
	{0,0,fdcb21,NULL,"sla c,(iy:5)"},
	{0,0,fdcb22,NULL,"sla d,(iy:5)"},
	{0,0,fdcb23,NULL,"sla e,(iy:5)"},
	{0,0,fdcb24,NULL,"sla h,(iy:5)"},
	{0,0,fdcb25,NULL,"sla l,(iy:5)"},
	{0,0,fdcb26,NULL,"sla (iy:5)"},
	{0,0,fdcb27,NULL,"sla a,(iy:5)"},

	{0,0,fdcb28,NULL,"sra b,(iy:5)"},
	{0,0,fdcb29,NULL,"sra c,(iy:5)"},
	{0,0,fdcb2A,NULL,"sra d,(iy:5)"},
	{0,0,fdcb2B,NULL,"sra e,(iy:5)"},
	{0,0,fdcb2C,NULL,"sra h,(iy:5)"},
	{0,0,fdcb2D,NULL,"sra l,(iy:5)"},
	{0,0,fdcb2E,NULL,"sra (iy:5)"},
	{0,0,fdcb2F,NULL,"sra a,(iy:5)"},

	{0,0,fdcb30,NULL,"sll b,(iy:5)"},
	{0,0,fdcb31,NULL,"sll c,(iy:5)"},
	{0,0,fdcb32,NULL,"sll d,(iy:5)"},
	{0,0,fdcb33,NULL,"sll e,(iy:5)"},
	{0,0,fdcb34,NULL,"sll h,(iy:5)"},
	{0,0,fdcb35,NULL,"sll l,(iy:5)"},
	{0,0,fdcb36,NULL,"sll (iy:5)"},
	{0,0,fdcb37,NULL,"sll a,(iy:5)"},

	{0,0,fdcb38,NULL,"srl b,(iy:5)"},
	{0,0,fdcb39,NULL,"srl c,(iy:5)"},
	{0,0,fdcb3A,NULL,"srl d,(iy:5)"},
	{0,0,fdcb3B,NULL,"srl e,(iy:5)"},
	{0,0,fdcb3C,NULL,"srl h,(iy:5)"},
	{0,0,fdcb3D,NULL,"srl l,(iy:5)"},
	{0,0,fdcb3E,NULL,"srl (iy:5)"},
	{0,0,fdcb3F,NULL,"srl a,(iy:5)"},

	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,fdcb46,NULL,"bit 0,(iy:5)"},

	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,fdcb4E,NULL,"bit 1,(iy:5)"},

	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,fdcb56,NULL,"bit 2,(iy:5)"},

	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,fdcb5E,NULL,"bit 3,(iy:5)"},

	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,fdcb66,NULL,"bit 4,(iy:5)"},

	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,fdcb6E,NULL,"bit 5,(iy:5)"},

	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,fdcb76,NULL,"bit 6,(iy:5)"},

	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,fdcb7E,NULL,"bit 7,(iy:5)"},

	{0,0,fdcb80,NULL,"res 0,b,(iy:5)"},
	{0,0,fdcb81,NULL,"res 0,c,(iy:5)"},
	{0,0,fdcb82,NULL,"res 0,d,(iy:5)"},
	{0,0,fdcb83,NULL,"res 0,e,(iy:5)"},
	{0,0,fdcb84,NULL,"res 0,h,(iy:5)"},
	{0,0,fdcb85,NULL,"res 0,l,(iy:5)"},
	{0,0,fdcb86,NULL,"res 0,(iy:5)"},
	{0,0,fdcb87,NULL,"res 0,a,(iy:5)"},

	{0,0,fdcb88,NULL,"res 1,b,(iy:5)"},
	{0,0,fdcb89,NULL,"res 1,c,(iy:5)"},
	{0,0,fdcb8A,NULL,"res 1,d,(iy:5)"},
	{0,0,fdcb8B,NULL,"res 1,e,(iy:5)"},
	{0,0,fdcb8C,NULL,"res 1,h,(iy:5)"},
	{0,0,fdcb8D,NULL,"res 1,l,(iy:5)"},
	{0,0,fdcb8E,NULL,"res 1,(iy:5)"},
	{0,0,fdcb8F,NULL,"res 1,a,(iy:5)"},

	{0,0,fdcb90,NULL,"res 2,b,(iy:5)"},
	{0,0,fdcb91,NULL,"res 2,c,(iy:5)"},
	{0,0,fdcb92,NULL,"res 2,d,(iy:5)"},
	{0,0,fdcb93,NULL,"res 2,e,(iy:5)"},
	{0,0,fdcb94,NULL,"res 2,h,(iy:5)"},
	{0,0,fdcb95,NULL,"res 2,l,(iy:5)"},
	{0,0,fdcb96,NULL,"res 2,(iy:5)"},
	{0,0,fdcb97,NULL,"res 2,a,(iy:5)"},

	{0,0,fdcb98,NULL,"res 3,b,(iy:5)"},
	{0,0,fdcb99,NULL,"res 3,c,(iy:5)"},
	{0,0,fdcb9A,NULL,"res 3,d,(iy:5)"},
	{0,0,fdcb9B,NULL,"res 3,e,(iy:5)"},
	{0,0,fdcb9C,NULL,"res 3,h,(iy:5)"},
	{0,0,fdcb9D,NULL,"res 3,l,(iy:5)"},
	{0,0,fdcb9E,NULL,"res 3,(iy:5)"},
	{0,0,fdcb9F,NULL,"res 3,a,(iy:5)"},

	{0,0,fdcbA0,NULL,"res 4,b,(iy:5)"},
	{0,0,fdcbA1,NULL,"res 4,c,(iy:5)"},
	{0,0,fdcbA2,NULL,"res 4,d,(iy:5)"},
	{0,0,fdcbA3,NULL,"res 4,e,(iy:5)"},
	{0,0,fdcbA4,NULL,"res 4,h,(iy:5)"},
	{0,0,fdcbA5,NULL,"res 4,l,(iy:5)"},
	{0,0,fdcbA6,NULL,"res 4,(iy:5)"},
	{0,0,fdcbA7,NULL,"res 4,a,(iy:5)"},

	{0,0,fdcbA8,NULL,"res 5,b,(iy:5)"},
	{0,0,fdcbA9,NULL,"res 5,c,(iy:5)"},
	{0,0,fdcbAA,NULL,"res 5,d,(iy:5)"},
	{0,0,fdcbAB,NULL,"res 5,e,(iy:5)"},
	{0,0,fdcbAC,NULL,"res 5,h,(iy:5)"},
	{0,0,fdcbAD,NULL,"res 5,l,(iy:5)"},
	{0,0,fdcbAE,NULL,"res 5,(iy:5)"},
	{0,0,fdcbAF,NULL,"res 5,a,(iy:5)"},

	{0,0,fdcbB0,NULL,"res 6,b,(iy:5)"},
	{0,0,fdcbB1,NULL,"res 6,c,(iy:5)"},
	{0,0,fdcbB2,NULL,"res 6,d,(iy:5)"},
	{0,0,fdcbB3,NULL,"res 6,e,(iy:5)"},
	{0,0,fdcbB4,NULL,"res 6,h,(iy:5)"},
	{0,0,fdcbB5,NULL,"res 6,l,(iy:5)"},
	{0,0,fdcbB6,NULL,"res 6,(iy:5)"},
	{0,0,fdcbB7,NULL,"res 6,a,(iy:5)"},

	{0,0,fdcbB8,NULL,"res 7,b,(iy:5)"},
	{0,0,fdcbB9,NULL,"res 7,c,(iy:5)"},
	{0,0,fdcbBA,NULL,"res 7,d,(iy:5)"},
	{0,0,fdcbBB,NULL,"res 7,e,(iy:5)"},
	{0,0,fdcbBC,NULL,"res 7,h,(iy:5)"},
	{0,0,fdcbBD,NULL,"res 7,l,(iy:5)"},
	{0,0,fdcbBE,NULL,"res 7,(iy:5)"},
	{0,0,fdcbBF,NULL,"res 7,a,(iy:5)"},

	{0,0,fdcbC0,NULL,"set 0,b,(iy:5)"},
	{0,0,fdcbC1,NULL,"set 0,c,(iy:5)"},
	{0,0,fdcbC2,NULL,"set 0,d,(iy:5)"},
	{0,0,fdcbC3,NULL,"set 0,e,(iy:5)"},
	{0,0,fdcbC4,NULL,"set 0,h,(iy:5)"},
	{0,0,fdcbC5,NULL,"set 0,l,(iy:5)"},
	{0,0,fdcbC6,NULL,"set 0,(iy:5)"},
	{0,0,fdcbC7,NULL,"set 0,a,(iy:5)"},

	{0,0,fdcbC8,NULL,"set 1,b,(iy:5)"},
	{0,0,fdcbC9,NULL,"set 1,c,(iy:5)"},
	{0,0,fdcbCA,NULL,"set 1,d,(iy:5)"},
	{0,0,fdcbCB,NULL,"set 1,e,(iy:5)"},
	{0,0,fdcbCC,NULL,"set 1,h,(iy:5)"},
	{0,0,fdcbCD,NULL,"set 1,l,(iy:5)"},
	{0,0,fdcbCE,NULL,"set 1,(iy:5)"},
	{0,0,fdcbCF,NULL,"set 1,a,(iy:5)"},

	{0,0,fdcbD0,NULL,"set 2,b,(iy:5)"},
	{0,0,fdcbD1,NULL,"set 2,c,(iy:5)"},
	{0,0,fdcbD2,NULL,"set 2,d,(iy:5)"},
	{0,0,fdcbD3,NULL,"set 2,e,(iy:5)"},
	{0,0,fdcbD4,NULL,"set 2,h,(iy:5)"},
	{0,0,fdcbD5,NULL,"set 2,l,(iy:5)"},
	{0,0,fdcbD6,NULL,"set 2,(iy:5)"},
	{0,0,fdcbD7,NULL,"set 2,a,(iy:5)"},

	{0,0,fdcbD8,NULL,"set 3,b,(iy:5)"},
	{0,0,fdcbD9,NULL,"set 3,c,(iy:5)"},
	{0,0,fdcbDA,NULL,"set 3,d,(iy:5)"},
	{0,0,fdcbDB,NULL,"set 3,e,(iy:5)"},
	{0,0,fdcbDC,NULL,"set 3,h,(iy:5)"},
	{0,0,fdcbDD,NULL,"set 3,l,(iy:5)"},
	{0,0,fdcbDE,NULL,"set 3,(iy:5)"},
	{0,0,fdcbDF,NULL,"set 3,a,(iy:5)"},

	{0,0,fdcbE0,NULL,"set 4,b,(iy:5)"},
	{0,0,fdcbE1,NULL,"set 4,c,(iy:5)"},
	{0,0,fdcbE2,NULL,"set 4,d,(iy:5)"},
	{0,0,fdcbE3,NULL,"set 4,e,(iy:5)"},
	{0,0,fdcbE4,NULL,"set 4,h,(iy:5)"},
	{0,0,fdcbE5,NULL,"set 4,l,(iy:5)"},
	{0,0,fdcbE6,NULL,"set 4,(iy:5)"},
	{0,0,fdcbE7,NULL,"set 4,a,(iy:5)"},

	{0,0,fdcbE8,NULL,"set 5,b,(iy:5)"},
	{0,0,fdcbE9,NULL,"set 5,c,(iy:5)"},
	{0,0,fdcbEA,NULL,"set 5,d,(iy:5)"},
	{0,0,fdcbEB,NULL,"set 5,e,(iy:5)"},
	{0,0,fdcbEC,NULL,"set 5,h,(iy:5)"},
	{0,0,fdcbED,NULL,"set 5,l,(iy:5)"},
	{0,0,fdcbEE,NULL,"set 5,(iy:5)"},
	{0,0,fdcbEF,NULL,"set 5,a,(iy:5)"},

	{0,0,fdcbF0,NULL,"set 6,b,(iy:5)"},
	{0,0,fdcbF1,NULL,"set 6,c,(iy:5)"},
	{0,0,fdcbF2,NULL,"set 6,d,(iy:5)"},
	{0,0,fdcbF3,NULL,"set 6,e,(iy:5)"},
	{0,0,fdcbF4,NULL,"set 6,h,(iy:5)"},
	{0,0,fdcbF5,NULL,"set 6,l,(iy:5)"},
	{0,0,fdcbF6,NULL,"set 6,(iy:5)"},
	{0,0,fdcbF7,NULL,"set 6,a,(iy:5)"},

	{0,0,fdcbF8,NULL,"set 7,b,(iy:5)"},
	{0,0,fdcbF9,NULL,"set 7,c,(iy:5)"},
	{0,0,fdcbFA,NULL,"set 7,d,(iy:5)"},
	{0,0,fdcbFB,NULL,"set 7,e,(iy:5)"},
	{0,0,fdcbFC,NULL,"set 7,h,(iy:5)"},
	{0,0,fdcbFD,NULL,"set 7,l,(iy:5)"},
	{0,0,fdcbFE,NULL,"set 7,(iy:5)"},
	{0,0,fdcbFF,NULL,"set 7,a,(iy:5)"},
};
