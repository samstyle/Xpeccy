#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"

// rlc reg,(ix+e)	[3rd] 4 5add 4rd 3wr
void ddcb00(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regB);}
void ddcb01(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regC);}
void ddcb02(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regD);}
void ddcb03(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regE);}
void ddcb04(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regH);}
void ddcb05(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regL);}
void ddcb06(CPU* cpu) {XDCB(cpu->regIX,z80_rlc);}
void ddcb07(CPU* cpu) {XDCBR(cpu->regIX,z80_rlc,cpu->regA);}
// rrc reg,(ix+e)
void ddcb08(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regB);}
void ddcb09(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regC);}
void ddcb0A(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regD);}
void ddcb0B(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regE);}
void ddcb0C(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regH);}
void ddcb0D(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regL);}
void ddcb0E(CPU* cpu) {XDCB(cpu->regIX,z80_rrc);}
void ddcb0F(CPU* cpu) {XDCBR(cpu->regIX,z80_rrc,cpu->regA);}
// rl reg,(ix+e)
void ddcb10(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regB);}
void ddcb11(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regC);}
void ddcb12(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regD);}
void ddcb13(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regE);}
void ddcb14(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regH);}
void ddcb15(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regL);}
void ddcb16(CPU* cpu) {XDCB(cpu->regIX,z80_rl);}
void ddcb17(CPU* cpu) {XDCBR(cpu->regIX,z80_rl,cpu->regA);}
// rr reg,(ix+e)
void ddcb18(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regB);}
void ddcb19(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regC);}
void ddcb1A(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regD);}
void ddcb1B(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regE);}
void ddcb1C(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regH);}
void ddcb1D(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regL);}
void ddcb1E(CPU* cpu) {XDCB(cpu->regIX,z80_rr);}
void ddcb1F(CPU* cpu) {XDCBR(cpu->regIX,z80_rr,cpu->regA);}
// sla reg,(ix+e)
void ddcb20(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regB);}
void ddcb21(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regC);}
void ddcb22(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regD);}
void ddcb23(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regE);}
void ddcb24(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regH);}
void ddcb25(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regL);}
void ddcb26(CPU* cpu) {XDCB(cpu->regIX,z80_sla);}
void ddcb27(CPU* cpu) {XDCBR(cpu->regIX,z80_sla,cpu->regA);}
// sra reg,(ix+e)
void ddcb28(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regB);}
void ddcb29(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regC);}
void ddcb2A(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regD);}
void ddcb2B(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regE);}
void ddcb2C(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regH);}
void ddcb2D(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regL);}
void ddcb2E(CPU* cpu) {XDCB(cpu->regIX,z80_sra);}
void ddcb2F(CPU* cpu) {XDCBR(cpu->regIX,z80_sra,cpu->regA);}
// sll reg,(ix+e)
void ddcb30(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regB);}
void ddcb31(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regC);}
void ddcb32(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regD);}
void ddcb33(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regE);}
void ddcb34(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regH);}
void ddcb35(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regL);}
void ddcb36(CPU* cpu) {XDCB(cpu->regIX,z80_sll);}
void ddcb37(CPU* cpu) {XDCBR(cpu->regIX,z80_sll,cpu->regA);}
// srl reg,(ix+e)
void ddcb38(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regB);}
void ddcb39(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regC);}
void ddcb3A(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regD);}
void ddcb3B(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regE);}
void ddcb3C(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regH);}
void ddcb3D(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regL);}
void ddcb3E(CPU* cpu) {XDCB(cpu->regIX,z80_srl);}
void ddcb3F(CPU* cpu) {XDCBR(cpu->regIX,z80_srl,cpu->regA);}

// bit n,(ix+e)
void ddcb46(CPU* cpu) {BITX(cpu->regIX,0);}
void ddcb4E(CPU* cpu) {BITX(cpu->regIX,1);}
void ddcb56(CPU* cpu) {BITX(cpu->regIX,2);}
void ddcb5E(CPU* cpu) {BITX(cpu->regIX,3);}
void ddcb66(CPU* cpu) {BITX(cpu->regIX,4);}
void ddcb6E(CPU* cpu) {BITX(cpu->regIX,5);}
void ddcb76(CPU* cpu) {BITX(cpu->regIX,6);}
void ddcb7E(CPU* cpu) {BITX(cpu->regIX,7);}

// res 0,reg,(ix+e)
void ddcb80(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regB);}
void ddcb81(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regC);}
void ddcb82(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regD);}
void ddcb83(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regE);}
void ddcb84(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regH);}
void ddcb85(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regL);}
void ddcb86(CPU* cpu) {RESX(cpu->regIX,0);}
void ddcb87(CPU* cpu) {RESXR(cpu->regIX,0,cpu->regA);}
// res 1,reg,(ix+e)
void ddcb88(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regB);}
void ddcb89(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regC);}
void ddcb8A(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regD);}
void ddcb8B(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regE);}
void ddcb8C(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regH);}
void ddcb8D(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regL);}
void ddcb8E(CPU* cpu) {RESX(cpu->regIX,1);}
void ddcb8F(CPU* cpu) {RESXR(cpu->regIX,1,cpu->regA);}
// res 2,reg,(ix+e)
void ddcb90(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regB);}
void ddcb91(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regC);}
void ddcb92(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regD);}
void ddcb93(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regE);}
void ddcb94(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regH);}
void ddcb95(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regL);}
void ddcb96(CPU* cpu) {RESX(cpu->regIX,2);}
void ddcb97(CPU* cpu) {RESXR(cpu->regIX,2,cpu->regA);}
// res 3,reg,(ix+e)
void ddcb98(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regB);}
void ddcb99(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regC);}
void ddcb9A(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regD);}
void ddcb9B(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regE);}
void ddcb9C(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regH);}
void ddcb9D(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regL);}
void ddcb9E(CPU* cpu) {RESX(cpu->regIX,3);}
void ddcb9F(CPU* cpu) {RESXR(cpu->regIX,3,cpu->regA);}
// res 4,reg,(ix+e)
void ddcbA0(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regB);}
void ddcbA1(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regC);}
void ddcbA2(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regD);}
void ddcbA3(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regE);}
void ddcbA4(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regH);}
void ddcbA5(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regL);}
void ddcbA6(CPU* cpu) {RESX(cpu->regIX,4);}
void ddcbA7(CPU* cpu) {RESXR(cpu->regIX,4,cpu->regA);}
// res 5,reg,(ix+e)
void ddcbA8(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regB);}
void ddcbA9(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regC);}
void ddcbAA(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regD);}
void ddcbAB(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regE);}
void ddcbAC(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regH);}
void ddcbAD(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regL);}
void ddcbAE(CPU* cpu) {RESX(cpu->regIX,5);}
void ddcbAF(CPU* cpu) {RESXR(cpu->regIX,5,cpu->regA);}
// res 6,reg,(ix+e)
void ddcbB0(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regB);}
void ddcbB1(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regC);}
void ddcbB2(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regD);}
void ddcbB3(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regE);}
void ddcbB4(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regH);}
void ddcbB5(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regL);}
void ddcbB6(CPU* cpu) {RESX(cpu->regIX,6);}
void ddcbB7(CPU* cpu) {RESXR(cpu->regIX,6,cpu->regA);}
// res 7,reg,(ix+e)
void ddcbB8(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regB);}
void ddcbB9(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regC);}
void ddcbBA(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regD);}
void ddcbBB(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regE);}
void ddcbBC(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regH);}
void ddcbBD(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regL);}
void ddcbBE(CPU* cpu) {RESX(cpu->regIX,7);}
void ddcbBF(CPU* cpu) {RESXR(cpu->regIX,7,cpu->regA);}

// set 0,reg,(ix+e)
void ddcbC0(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regB);}
void ddcbC1(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regC);}
void ddcbC2(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regD);}
void ddcbC3(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regE);}
void ddcbC4(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regH);}
void ddcbC5(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regL);}
void ddcbC6(CPU* cpu) {SETX(cpu->regIX,0);}
void ddcbC7(CPU* cpu) {SETXR(cpu->regIX,0,cpu->regA);}
// set 1,reg,(ix+e)
void ddcbC8(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regB);}
void ddcbC9(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regC);}
void ddcbCA(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regD);}
void ddcbCB(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regE);}
void ddcbCC(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regH);}
void ddcbCD(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regL);}
void ddcbCE(CPU* cpu) {SETX(cpu->regIX,1);}
void ddcbCF(CPU* cpu) {SETXR(cpu->regIX,1,cpu->regA);}
// set 2,reg,(ix+e)
void ddcbD0(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regB);}
void ddcbD1(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regC);}
void ddcbD2(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regD);}
void ddcbD3(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regE);}
void ddcbD4(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regH);}
void ddcbD5(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regL);}
void ddcbD6(CPU* cpu) {SETX(cpu->regIX,2);}
void ddcbD7(CPU* cpu) {SETXR(cpu->regIX,2,cpu->regA);}
// set 3,reg,(ix+e)
void ddcbD8(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regB);}
void ddcbD9(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regC);}
void ddcbDA(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regD);}
void ddcbDB(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regE);}
void ddcbDC(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regH);}
void ddcbDD(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regL);}
void ddcbDE(CPU* cpu) {SETX(cpu->regIX,3);}
void ddcbDF(CPU* cpu) {SETXR(cpu->regIX,3,cpu->regA);}
// set 4,reg,(ix+e)
void ddcbE0(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regB);}
void ddcbE1(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regC);}
void ddcbE2(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regD);}
void ddcbE3(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regE);}
void ddcbE4(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regH);}
void ddcbE5(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regL);}
void ddcbE6(CPU* cpu) {SETX(cpu->regIX,4);}
void ddcbE7(CPU* cpu) {SETXR(cpu->regIX,4,cpu->regA);}
// set 5,reg,(ix+e)
void ddcbE8(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regB);}
void ddcbE9(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regC);}
void ddcbEA(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regD);}
void ddcbEB(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regE);}
void ddcbEC(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regH);}
void ddcbED(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regL);}
void ddcbEE(CPU* cpu) {SETX(cpu->regIX,5);}
void ddcbEF(CPU* cpu) {SETXR(cpu->regIX,5,cpu->regA);}
// set 6,reg,(ix+e)
void ddcbF0(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regB);}
void ddcbF1(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regC);}
void ddcbF2(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regD);}
void ddcbF3(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regE);}
void ddcbF4(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regH);}
void ddcbF5(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regL);}
void ddcbF6(CPU* cpu) {SETX(cpu->regIX,6);}
void ddcbF7(CPU* cpu) {SETXR(cpu->regIX,6,cpu->regA);}
// set 7,reg,(ix+e)
void ddcbF8(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regB);}
void ddcbF9(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regC);}
void ddcbFA(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regD);}
void ddcbFB(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regE);}
void ddcbFC(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regH);}
void ddcbFD(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regL);}
void ddcbFE(CPU* cpu) {SETX(cpu->regIX,7);}
void ddcbFF(CPU* cpu) {SETXR(cpu->regIX,7,cpu->regA);}

//====
// why opcode fetching doesn't eat 4T?

opCode ddcbTab[256]={
	{0,0,ddcb00,NULL,"rlc b,(ix:5)"},
	{0,0,ddcb01,NULL,"rlc c,(ix:5)"},
	{0,0,ddcb02,NULL,"rlc d,(ix:5)"},
	{0,0,ddcb03,NULL,"rlc e,(ix:5)"},
	{0,0,ddcb04,NULL,"rlc h,(ix:5)"},
	{0,0,ddcb05,NULL,"rlc l,(ix:5)"},
	{0,0,ddcb06,NULL,"rlc (ix:5)"},
	{0,0,ddcb07,NULL,"rlc a,(ix:5)"},

	{0,0,ddcb08,NULL,"rrc b,(ix:5)"},
	{0,0,ddcb09,NULL,"rrc c,(ix:5)"},
	{0,0,ddcb0A,NULL,"rrc d,(ix:5)"},
	{0,0,ddcb0B,NULL,"rrc e,(ix:5)"},
	{0,0,ddcb0C,NULL,"rrc h,(ix:5)"},
	{0,0,ddcb0D,NULL,"rrc l,(ix:5)"},
	{0,0,ddcb0E,NULL,"rrc (ix:5)"},
	{0,0,ddcb0F,NULL,"rrc a,(ix:5)"},

	{0,0,ddcb10,NULL,"rl b,(ix:5)"},
	{0,0,ddcb11,NULL,"rl c,(ix:5)"},
	{0,0,ddcb12,NULL,"rl d,(ix:5)"},
	{0,0,ddcb13,NULL,"rl e,(ix:5)"},
	{0,0,ddcb14,NULL,"rl h,(ix:5)"},
	{0,0,ddcb15,NULL,"rl l,(ix:5)"},
	{0,0,ddcb16,NULL,"rl (ix:5)"},
	{0,0,ddcb17,NULL,"rl a,(ix:5)"},

	{0,0,ddcb18,NULL,"rr b,(ix:5)"},
	{0,0,ddcb19,NULL,"rr c,(ix:5)"},
	{0,0,ddcb1A,NULL,"rr d,(ix:5)"},
	{0,0,ddcb1B,NULL,"rr e,(ix:5)"},
	{0,0,ddcb1C,NULL,"rr h,(ix:5)"},
	{0,0,ddcb1D,NULL,"rr l,(ix:5)"},
	{0,0,ddcb1E,NULL,"rr (ix:5)"},
	{0,0,ddcb1F,NULL,"rr a,(ix:5)"},

	{0,0,ddcb20,NULL,"sla b,(ix:5)"},
	{0,0,ddcb21,NULL,"sla c,(ix:5)"},
	{0,0,ddcb22,NULL,"sla d,(ix:5)"},
	{0,0,ddcb23,NULL,"sla e,(ix:5)"},
	{0,0,ddcb24,NULL,"sla h,(ix:5)"},
	{0,0,ddcb25,NULL,"sla l,(ix:5)"},
	{0,0,ddcb26,NULL,"sla (ix:5)"},
	{0,0,ddcb27,NULL,"sla a,(ix:5)"},

	{0,0,ddcb28,NULL,"sra b,(ix:5)"},
	{0,0,ddcb29,NULL,"sra c,(ix:5)"},
	{0,0,ddcb2A,NULL,"sra d,(ix:5)"},
	{0,0,ddcb2B,NULL,"sra e,(ix:5)"},
	{0,0,ddcb2C,NULL,"sra h,(ix:5)"},
	{0,0,ddcb2D,NULL,"sra l,(ix:5)"},
	{0,0,ddcb2E,NULL,"sra (ix:5)"},
	{0,0,ddcb2F,NULL,"sra a,(ix:5)"},

	{0,0,ddcb30,NULL,"sll b,(ix:5)"},
	{0,0,ddcb31,NULL,"sll c,(ix:5)"},
	{0,0,ddcb32,NULL,"sll d,(ix:5)"},
	{0,0,ddcb33,NULL,"sll e,(ix:5)"},
	{0,0,ddcb34,NULL,"sll h,(ix:5)"},
	{0,0,ddcb35,NULL,"sll l,(ix:5)"},
	{0,0,ddcb36,NULL,"sll (ix:5)"},
	{0,0,ddcb37,NULL,"sll a,(ix:5)"},

	{0,0,ddcb38,NULL,"srl b,(ix:5)"},
	{0,0,ddcb39,NULL,"srl c,(ix:5)"},
	{0,0,ddcb3A,NULL,"srl d,(ix:5)"},
	{0,0,ddcb3B,NULL,"srl e,(ix:5)"},
	{0,0,ddcb3C,NULL,"srl h,(ix:5)"},
	{0,0,ddcb3D,NULL,"srl l,(ix:5)"},
	{0,0,ddcb3E,NULL,"srl (ix:5)"},
	{0,0,ddcb3F,NULL,"srl a,(ix:5)"},

	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,ddcb46,NULL,"bit 0,(ix:5)"},

	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,ddcb4E,NULL,"bit 1,(ix:5)"},

	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,ddcb56,NULL,"bit 2,(ix:5)"},

	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,ddcb5E,NULL,"bit 3,(ix:5)"},

	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,ddcb66,NULL,"bit 4,(ix:5)"},

	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,ddcb6E,NULL,"bit 5,(ix:5)"},

	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,ddcb76,NULL,"bit 6,(ix:5)"},

	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,ddcb7E,NULL,"bit 7,(ix:5)"},

	{0,0,ddcb80,NULL,"res 0,b,(ix:5)"},
	{0,0,ddcb81,NULL,"res 0,c,(ix:5)"},
	{0,0,ddcb82,NULL,"res 0,d,(ix:5)"},
	{0,0,ddcb83,NULL,"res 0,e,(ix:5)"},
	{0,0,ddcb84,NULL,"res 0,h,(ix:5)"},
	{0,0,ddcb85,NULL,"res 0,l,(ix:5)"},
	{0,0,ddcb86,NULL,"res 0,(ix:5)"},
	{0,0,ddcb87,NULL,"res 0,a,(ix:5)"},

	{0,0,ddcb88,NULL,"res 1,b,(ix:5)"},
	{0,0,ddcb89,NULL,"res 1,c,(ix:5)"},
	{0,0,ddcb8A,NULL,"res 1,d,(ix:5)"},
	{0,0,ddcb8B,NULL,"res 1,e,(ix:5)"},
	{0,0,ddcb8C,NULL,"res 1,h,(ix:5)"},
	{0,0,ddcb8D,NULL,"res 1,l,(ix:5)"},
	{0,0,ddcb8E,NULL,"res 1,(ix:5)"},
	{0,0,ddcb8F,NULL,"res 1,a,(ix:5)"},

	{0,0,ddcb90,NULL,"res 2,b,(ix:5)"},
	{0,0,ddcb91,NULL,"res 2,c,(ix:5)"},
	{0,0,ddcb92,NULL,"res 2,d,(ix:5)"},
	{0,0,ddcb93,NULL,"res 2,e,(ix:5)"},
	{0,0,ddcb94,NULL,"res 2,h,(ix:5)"},
	{0,0,ddcb95,NULL,"res 2,l,(ix:5)"},
	{0,0,ddcb96,NULL,"res 2,(ix:5)"},
	{0,0,ddcb97,NULL,"res 2,a,(ix:5)"},

	{0,0,ddcb98,NULL,"res 3,b,(ix:5)"},
	{0,0,ddcb99,NULL,"res 3,c,(ix:5)"},
	{0,0,ddcb9A,NULL,"res 3,d,(ix:5)"},
	{0,0,ddcb9B,NULL,"res 3,e,(ix:5)"},
	{0,0,ddcb9C,NULL,"res 3,h,(ix:5)"},
	{0,0,ddcb9D,NULL,"res 3,l,(ix:5)"},
	{0,0,ddcb9E,NULL,"res 3,(ix:5)"},
	{0,0,ddcb9F,NULL,"res 3,a,(ix:5)"},

	{0,0,ddcbA0,NULL,"res 4,b,(ix:5)"},
	{0,0,ddcbA1,NULL,"res 4,c,(ix:5)"},
	{0,0,ddcbA2,NULL,"res 4,d,(ix:5)"},
	{0,0,ddcbA3,NULL,"res 4,e,(ix:5)"},
	{0,0,ddcbA4,NULL,"res 4,h,(ix:5)"},
	{0,0,ddcbA5,NULL,"res 4,l,(ix:5)"},
	{0,0,ddcbA6,NULL,"res 4,(ix:5)"},
	{0,0,ddcbA7,NULL,"res 4,a,(ix:5)"},

	{0,0,ddcbA8,NULL,"res 5,b,(ix:5)"},
	{0,0,ddcbA9,NULL,"res 5,c,(ix:5)"},
	{0,0,ddcbAA,NULL,"res 5,d,(ix:5)"},
	{0,0,ddcbAB,NULL,"res 5,e,(ix:5)"},
	{0,0,ddcbAC,NULL,"res 5,h,(ix:5)"},
	{0,0,ddcbAD,NULL,"res 5,l,(ix:5)"},
	{0,0,ddcbAE,NULL,"res 5,(ix:5)"},
	{0,0,ddcbAF,NULL,"res 5,a,(ix:5)"},

	{0,0,ddcbB0,NULL,"res 6,b,(ix:5)"},
	{0,0,ddcbB1,NULL,"res 6,c,(ix:5)"},
	{0,0,ddcbB2,NULL,"res 6,d,(ix:5)"},
	{0,0,ddcbB3,NULL,"res 6,e,(ix:5)"},
	{0,0,ddcbB4,NULL,"res 6,h,(ix:5)"},
	{0,0,ddcbB5,NULL,"res 6,l,(ix:5)"},
	{0,0,ddcbB6,NULL,"res 6,(ix:5)"},
	{0,0,ddcbB7,NULL,"res 6,a,(ix:5)"},

	{0,0,ddcbB8,NULL,"res 7,b,(ix:5)"},
	{0,0,ddcbB9,NULL,"res 7,c,(ix:5)"},
	{0,0,ddcbBA,NULL,"res 7,d,(ix:5)"},
	{0,0,ddcbBB,NULL,"res 7,e,(ix:5)"},
	{0,0,ddcbBC,NULL,"res 7,h,(ix:5)"},
	{0,0,ddcbBD,NULL,"res 7,l,(ix:5)"},
	{0,0,ddcbBE,NULL,"res 7,(ix:5)"},
	{0,0,ddcbBF,NULL,"res 7,a,(ix:5)"},

	{0,0,ddcbC0,NULL,"set 0,b,(ix:5)"},
	{0,0,ddcbC1,NULL,"set 0,c,(ix:5)"},
	{0,0,ddcbC2,NULL,"set 0,d,(ix:5)"},
	{0,0,ddcbC3,NULL,"set 0,e,(ix:5)"},
	{0,0,ddcbC4,NULL,"set 0,h,(ix:5)"},
	{0,0,ddcbC5,NULL,"set 0,l,(ix:5)"},
	{0,0,ddcbC6,NULL,"set 0,(ix:5)"},
	{0,0,ddcbC7,NULL,"set 0,a,(ix:5)"},

	{0,0,ddcbC8,NULL,"set 1,b,(ix:5)"},
	{0,0,ddcbC9,NULL,"set 1,c,(ix:5)"},
	{0,0,ddcbCA,NULL,"set 1,d,(ix:5)"},
	{0,0,ddcbCB,NULL,"set 1,e,(ix:5)"},
	{0,0,ddcbCC,NULL,"set 1,h,(ix:5)"},
	{0,0,ddcbCD,NULL,"set 1,l,(ix:5)"},
	{0,0,ddcbCE,NULL,"set 1,(ix:5)"},
	{0,0,ddcbCF,NULL,"set 1,a,(ix:5)"},

	{0,0,ddcbD0,NULL,"set 2,b,(ix:5)"},
	{0,0,ddcbD1,NULL,"set 2,c,(ix:5)"},
	{0,0,ddcbD2,NULL,"set 2,d,(ix:5)"},
	{0,0,ddcbD3,NULL,"set 2,e,(ix:5)"},
	{0,0,ddcbD4,NULL,"set 2,h,(ix:5)"},
	{0,0,ddcbD5,NULL,"set 2,l,(ix:5)"},
	{0,0,ddcbD6,NULL,"set 2,(ix:5)"},
	{0,0,ddcbD7,NULL,"set 2,a,(ix:5)"},

	{0,0,ddcbD8,NULL,"set 3,b,(ix:5)"},
	{0,0,ddcbD9,NULL,"set 3,c,(ix:5)"},
	{0,0,ddcbDA,NULL,"set 3,d,(ix:5)"},
	{0,0,ddcbDB,NULL,"set 3,e,(ix:5)"},
	{0,0,ddcbDC,NULL,"set 3,h,(ix:5)"},
	{0,0,ddcbDD,NULL,"set 3,l,(ix:5)"},
	{0,0,ddcbDE,NULL,"set 3,(ix:5)"},
	{0,0,ddcbDF,NULL,"set 3,a,(ix:5)"},

	{0,0,ddcbE0,NULL,"set 4,b,(ix:5)"},
	{0,0,ddcbE1,NULL,"set 4,c,(ix:5)"},
	{0,0,ddcbE2,NULL,"set 4,d,(ix:5)"},
	{0,0,ddcbE3,NULL,"set 4,e,(ix:5)"},
	{0,0,ddcbE4,NULL,"set 4,h,(ix:5)"},
	{0,0,ddcbE5,NULL,"set 4,l,(ix:5)"},
	{0,0,ddcbE6,NULL,"set 4,(ix:5)"},
	{0,0,ddcbE7,NULL,"set 4,a,(ix:5)"},

	{0,0,ddcbE8,NULL,"set 5,b,(ix:5)"},
	{0,0,ddcbE9,NULL,"set 5,c,(ix:5)"},
	{0,0,ddcbEA,NULL,"set 5,d,(ix:5)"},
	{0,0,ddcbEB,NULL,"set 5,e,(ix:5)"},
	{0,0,ddcbEC,NULL,"set 5,h,(ix:5)"},
	{0,0,ddcbED,NULL,"set 5,l,(ix:5)"},
	{0,0,ddcbEE,NULL,"set 5,(ix:5)"},
	{0,0,ddcbEF,NULL,"set 5,a,(ix:5)"},

	{0,0,ddcbF0,NULL,"set 6,b,(ix:5)"},
	{0,0,ddcbF1,NULL,"set 6,c,(ix:5)"},
	{0,0,ddcbF2,NULL,"set 6,d,(ix:5)"},
	{0,0,ddcbF3,NULL,"set 6,e,(ix:5)"},
	{0,0,ddcbF4,NULL,"set 6,h,(ix:5)"},
	{0,0,ddcbF5,NULL,"set 6,l,(ix:5)"},
	{0,0,ddcbF6,NULL,"set 6,(ix:5)"},
	{0,0,ddcbF7,NULL,"set 6,a,(ix:5)"},

	{0,0,ddcbF8,NULL,"set 7,b,(ix:5)"},
	{0,0,ddcbF9,NULL,"set 7,c,(ix:5)"},
	{0,0,ddcbFA,NULL,"set 7,d,(ix:5)"},
	{0,0,ddcbFB,NULL,"set 7,e,(ix:5)"},
	{0,0,ddcbFC,NULL,"set 7,h,(ix:5)"},
	{0,0,ddcbFD,NULL,"set 7,l,(ix:5)"},
	{0,0,ddcbFE,NULL,"set 7,(ix:5)"},
	{0,0,ddcbFF,NULL,"set 7,a,(ix:5)"},
};
