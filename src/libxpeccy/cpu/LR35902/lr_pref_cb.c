#include <stdlib.h>
#include "../cpu.h"
#include "lr_macro.h"

// 00..07	rlc	4 [4rd 3wr]
void lrcb00(CPU* cpu) {RLCX(cpu->regB);}
void lrcb01(CPU* cpu) {RLCX(cpu->regC);}
void lrcb02(CPU* cpu) {RLCX(cpu->regD);}
void lrcb03(CPU* cpu) {RLCX(cpu->regE);}
void lrcb04(CPU* cpu) {RLCX(cpu->regH);}
void lrcb05(CPU* cpu) {RLCX(cpu->regL);}
void lrcb06(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; RLCX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb07(CPU* cpu) {RLCX(cpu->regA);}
// 08..0f	rrc
void lrcb08(CPU* cpu) {RRCX(cpu->regB);}
void lrcb09(CPU* cpu) {RRCX(cpu->regC);}
void lrcb0A(CPU* cpu) {RRCX(cpu->regD);}
void lrcb0B(CPU* cpu) {RRCX(cpu->regE);}
void lrcb0C(CPU* cpu) {RRCX(cpu->regH);}
void lrcb0D(CPU* cpu) {RRCX(cpu->regL);}
void lrcb0E(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; RRCX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb0F(CPU* cpu) {RRCX(cpu->regA);}
// 10..17	rl
void lrcb10(CPU* cpu) {RLX(cpu->regB);}
void lrcb11(CPU* cpu) {RLX(cpu->regC);}
void lrcb12(CPU* cpu) {RLX(cpu->regD);}
void lrcb13(CPU* cpu) {RLX(cpu->regE);}
void lrcb14(CPU* cpu) {RLX(cpu->regH);}
void lrcb15(CPU* cpu) {RLX(cpu->regL);}
void lrcb16(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; RLX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb17(CPU* cpu) {RLX(cpu->regA);}
// 18..1f	rr
void lrcb18(CPU* cpu) {RRX(cpu->regB);}
void lrcb19(CPU* cpu) {RRX(cpu->regC);}
void lrcb1A(CPU* cpu) {RRX(cpu->regD);}
void lrcb1B(CPU* cpu) {RRX(cpu->regE);}
void lrcb1C(CPU* cpu) {RRX(cpu->regH);}
void lrcb1D(CPU* cpu) {RRX(cpu->regL);}
void lrcb1E(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; RRX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb1F(CPU* cpu) {RRX(cpu->regA);}
// 20..27	sla
void lrcb20(CPU* cpu) {SLAX(cpu->regB);}
void lrcb21(CPU* cpu) {SLAX(cpu->regC);}
void lrcb22(CPU* cpu) {SLAX(cpu->regD);}
void lrcb23(CPU* cpu) {SLAX(cpu->regE);}
void lrcb24(CPU* cpu) {SLAX(cpu->regH);}
void lrcb25(CPU* cpu) {SLAX(cpu->regL);}
void lrcb26(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; SLAX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb27(CPU* cpu) {SLAX(cpu->regA);}
// 28..2f	sra
void lrcb28(CPU* cpu) {SRAX(cpu->regB);}
void lrcb29(CPU* cpu) {SRAX(cpu->regC);}
void lrcb2A(CPU* cpu) {SRAX(cpu->regD);}
void lrcb2B(CPU* cpu) {SRAX(cpu->regE);}
void lrcb2C(CPU* cpu) {SRAX(cpu->regH);}
void lrcb2D(CPU* cpu) {SRAX(cpu->regL);}
void lrcb2E(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; SRAX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb2F(CPU* cpu) {SRAX(cpu->regA);}
// 30..37	swap
inline unsigned char lr_swaph(unsigned char v) {return ((v & 0x0f) << 4) | ((v & 0xf0) >> 4);}
void lrcb30(CPU* cpu) {cpu->regB = lr_swaph(cpu->regB);}
void lrcb31(CPU* cpu) {cpu->regC = lr_swaph(cpu->regC);}
void lrcb32(CPU* cpu) {cpu->regD = lr_swaph(cpu->regD);}
void lrcb33(CPU* cpu) {cpu->regE = lr_swaph(cpu->regE);}
void lrcb34(CPU* cpu) {cpu->regH = lr_swaph(cpu->regH);}
void lrcb35(CPU* cpu) {cpu->regL = lr_swaph(cpu->regL);}
void lrcb36(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; cpu->tmpb = lr_swaph(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb37(CPU* cpu) {cpu->regA = lr_swaph(cpu->regA);}
// 38..3f	srl
void lrcb38(CPU* cpu) {SRLX(cpu->regB);}
void lrcb39(CPU* cpu) {SRLX(cpu->regC);}
void lrcb3A(CPU* cpu) {SRLX(cpu->regD);}
void lrcb3B(CPU* cpu) {SRLX(cpu->regE);}
void lrcb3C(CPU* cpu) {SRLX(cpu->regH);}
void lrcb3D(CPU* cpu) {SRLX(cpu->regL);}
void lrcb3E(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->regHL); cpu->t++; SRLX(cpu->tmpb); lr_mwr(cpu, cpu->regHL, cpu->tmpb);}
void lrcb3F(CPU* cpu) {SRLX(cpu->regA);}

// 40..47	bit 0,r		4 [4rd]
void lrcb40(CPU* cpu) {BITL(0,cpu->regB);}
void lrcb41(CPU* cpu) {BITL(0,cpu->regC);}
void lrcb42(CPU* cpu) {BITL(0,cpu->regD);}
void lrcb43(CPU* cpu) {BITL(0,cpu->regE);}
void lrcb44(CPU* cpu) {BITL(0,cpu->regH);}
void lrcb45(CPU* cpu) {BITL(0,cpu->regL);}
void lrcb46(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(0,cpu->tmp);}
void lrcb47(CPU* cpu) {BITL(0,cpu->regA);}
// 48..4f	bit 1,r
void lrcb48(CPU* cpu) {BITL(1,cpu->regB);}
void lrcb49(CPU* cpu) {BITL(1,cpu->regC);}
void lrcb4A(CPU* cpu) {BITL(1,cpu->regD);}
void lrcb4B(CPU* cpu) {BITL(1,cpu->regE);}
void lrcb4C(CPU* cpu) {BITL(1,cpu->regH);}
void lrcb4D(CPU* cpu) {BITL(1,cpu->regL);}
void lrcb4E(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(1,cpu->tmp);}
void lrcb4F(CPU* cpu) {BITL(1,cpu->regA);}
// 50..57	bit 2,r
void lrcb50(CPU* cpu) {BITL(2,cpu->regB);}
void lrcb51(CPU* cpu) {BITL(2,cpu->regC);}
void lrcb52(CPU* cpu) {BITL(2,cpu->regD);}
void lrcb53(CPU* cpu) {BITL(2,cpu->regE);}
void lrcb54(CPU* cpu) {BITL(2,cpu->regH);}
void lrcb55(CPU* cpu) {BITL(2,cpu->regL);}
void lrcb56(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(2,cpu->tmp);}
void lrcb57(CPU* cpu) {BITL(2,cpu->regA);}
// 58..5f	bit 3,r
void lrcb58(CPU* cpu) {BITL(3,cpu->regB);}
void lrcb59(CPU* cpu) {BITL(3,cpu->regC);}
void lrcb5A(CPU* cpu) {BITL(3,cpu->regD);}
void lrcb5B(CPU* cpu) {BITL(3,cpu->regE);}
void lrcb5C(CPU* cpu) {BITL(3,cpu->regH);}
void lrcb5D(CPU* cpu) {BITL(3,cpu->regL);}
void lrcb5E(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(3,cpu->tmp);}
void lrcb5F(CPU* cpu) {BITL(3,cpu->regA);}
// 60..67	bit 4,r
void lrcb60(CPU* cpu) {BITL(4,cpu->regB);}
void lrcb61(CPU* cpu) {BITL(4,cpu->regC);}
void lrcb62(CPU* cpu) {BITL(4,cpu->regD);}
void lrcb63(CPU* cpu) {BITL(4,cpu->regE);}
void lrcb64(CPU* cpu) {BITL(4,cpu->regH);}
void lrcb65(CPU* cpu) {BITL(4,cpu->regL);}
void lrcb66(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(4,cpu->tmp);}
void lrcb67(CPU* cpu) {BITL(4,cpu->regA);}
// 68..6f	bit 5,r
void lrcb68(CPU* cpu) {BITL(5,cpu->regB);}
void lrcb69(CPU* cpu) {BITL(5,cpu->regC);}
void lrcb6A(CPU* cpu) {BITL(5,cpu->regD);}
void lrcb6B(CPU* cpu) {BITL(5,cpu->regE);}
void lrcb6C(CPU* cpu) {BITL(5,cpu->regH);}
void lrcb6D(CPU* cpu) {BITL(5,cpu->regL);}
void lrcb6E(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(5,cpu->tmp);}
void lrcb6F(CPU* cpu) {BITL(5,cpu->regA);}
// 70..77	bit 6,r
void lrcb70(CPU* cpu) {BITL(6,cpu->regB);}
void lrcb71(CPU* cpu) {BITL(6,cpu->regC);}
void lrcb72(CPU* cpu) {BITL(6,cpu->regD);}
void lrcb73(CPU* cpu) {BITL(6,cpu->regE);}
void lrcb74(CPU* cpu) {BITL(6,cpu->regH);}
void lrcb75(CPU* cpu) {BITL(6,cpu->regL);}
void lrcb76(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(6,cpu->tmp);}
void lrcb77(CPU* cpu) {BITL(6,cpu->regA);}
// 78..7f	bit 7,r
void lrcb78(CPU* cpu) {BITL(7,cpu->regB);}
void lrcb79(CPU* cpu) {BITL(7,cpu->regC);}
void lrcb7A(CPU* cpu) {BITL(7,cpu->regD);}
void lrcb7B(CPU* cpu) {BITL(7,cpu->regE);}
void lrcb7C(CPU* cpu) {BITL(7,cpu->regH);}
void lrcb7D(CPU* cpu) {BITL(7,cpu->regL);}
void lrcb7E(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; BITL(7,cpu->tmp);}
void lrcb7F(CPU* cpu) {BITL(7,cpu->regA);}

// 80..87	res 0,r		4 [4rd 3wr]
void lrcb80(CPU* cpu) {cpu->regB &= ~0x01;}
void lrcb81(CPU* cpu) {cpu->regC &= ~0x01;}
void lrcb82(CPU* cpu) {cpu->regD &= ~0x01;}
void lrcb83(CPU* cpu) {cpu->regE &= ~0x01;}
void lrcb84(CPU* cpu) {cpu->regH &= ~0x01;}
void lrcb85(CPU* cpu) {cpu->regL &= ~0x01;}
void lrcb86(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x01);}
void lrcb87(CPU* cpu) {cpu->regA &= ~0x01;}
// 88..8f	res 1,r
void lrcb88(CPU* cpu) {cpu->regB &= ~0x02;}
void lrcb89(CPU* cpu) {cpu->regC &= ~0x02;}
void lrcb8A(CPU* cpu) {cpu->regD &= ~0x02;}
void lrcb8B(CPU* cpu) {cpu->regE &= ~0x02;}
void lrcb8C(CPU* cpu) {cpu->regH &= ~0x02;}
void lrcb8D(CPU* cpu) {cpu->regL &= ~0x02;}
void lrcb8E(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x02);}
void lrcb8F(CPU* cpu) {cpu->regA &= ~0x02;}
// 90..97	res 2,r
void lrcb90(CPU* cpu) {cpu->regB &= ~0x04;}
void lrcb91(CPU* cpu) {cpu->regC &= ~0x04;}
void lrcb92(CPU* cpu) {cpu->regD &= ~0x04;}
void lrcb93(CPU* cpu) {cpu->regE &= ~0x04;}
void lrcb94(CPU* cpu) {cpu->regH &= ~0x04;}
void lrcb95(CPU* cpu) {cpu->regL &= ~0x04;}
void lrcb96(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x04);}
void lrcb97(CPU* cpu) {cpu->regA &= ~0x04;}
// 98..9f	res 3,r
void lrcb98(CPU* cpu) {cpu->regB &= ~0x08;}
void lrcb99(CPU* cpu) {cpu->regC &= ~0x08;}
void lrcb9A(CPU* cpu) {cpu->regD &= ~0x08;}
void lrcb9B(CPU* cpu) {cpu->regE &= ~0x08;}
void lrcb9C(CPU* cpu) {cpu->regH &= ~0x08;}
void lrcb9D(CPU* cpu) {cpu->regL &= ~0x08;}
void lrcb9E(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x08);}
void lrcb9F(CPU* cpu) {cpu->regA &= ~0x08;}
// a0..a7	res 4,r
void lrcbA0(CPU* cpu) {cpu->regB &= ~0x10;}
void lrcbA1(CPU* cpu) {cpu->regC &= ~0x10;}
void lrcbA2(CPU* cpu) {cpu->regD &= ~0x10;}
void lrcbA3(CPU* cpu) {cpu->regE &= ~0x10;}
void lrcbA4(CPU* cpu) {cpu->regH &= ~0x10;}
void lrcbA5(CPU* cpu) {cpu->regL &= ~0x10;}
void lrcbA6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x10);}
void lrcbA7(CPU* cpu) {cpu->regA &= ~0x10;}
// a8..af	res 5,r
void lrcbA8(CPU* cpu) {cpu->regB &= ~0x20;}
void lrcbA9(CPU* cpu) {cpu->regC &= ~0x20;}
void lrcbAA(CPU* cpu) {cpu->regD &= ~0x20;}
void lrcbAB(CPU* cpu) {cpu->regE &= ~0x20;}
void lrcbAC(CPU* cpu) {cpu->regH &= ~0x20;}
void lrcbAD(CPU* cpu) {cpu->regL &= ~0x20;}
void lrcbAE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x20);}
void lrcbAF(CPU* cpu) {cpu->regA &= ~0x20;}
// b0..b7	res 6,r
void lrcbB0(CPU* cpu) {cpu->regB &= ~0x40;}
void lrcbB1(CPU* cpu) {cpu->regC &= ~0x40;}
void lrcbB2(CPU* cpu) {cpu->regD &= ~0x40;}
void lrcbB3(CPU* cpu) {cpu->regE &= ~0x40;}
void lrcbB4(CPU* cpu) {cpu->regH &= ~0x40;}
void lrcbB5(CPU* cpu) {cpu->regL &= ~0x40;}
void lrcbB6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x40);}
void lrcbB7(CPU* cpu) {cpu->regA &= ~0x40;}
// b8..bf	res 7,r
void lrcbB8(CPU* cpu) {cpu->regB &= ~0x80;}
void lrcbB9(CPU* cpu) {cpu->regC &= ~0x80;}
void lrcbBA(CPU* cpu) {cpu->regD &= ~0x80;}
void lrcbBB(CPU* cpu) {cpu->regE &= ~0x80;}
void lrcbBC(CPU* cpu) {cpu->regH &= ~0x80;}
void lrcbBD(CPU* cpu) {cpu->regL &= ~0x80;}
void lrcbBE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp & ~0x80);}
void lrcbBF(CPU* cpu) {cpu->regA &= ~0x80;}

// c0..c7	set 0,r		4 [4rd 3wr]
void lrcbC0(CPU* cpu) {cpu->regB |= 0x01;}
void lrcbC1(CPU* cpu) {cpu->regC |= 0x01;}
void lrcbC2(CPU* cpu) {cpu->regD |= 0x01;}
void lrcbC3(CPU* cpu) {cpu->regE |= 0x01;}
void lrcbC4(CPU* cpu) {cpu->regH |= 0x01;}
void lrcbC5(CPU* cpu) {cpu->regL |= 0x01;}
void lrcbC6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x01);}
void lrcbC7(CPU* cpu) {cpu->regA |= 0x01;}
// c8..cf	set 1,r
void lrcbC8(CPU* cpu) {cpu->regB |= 0x02;}
void lrcbC9(CPU* cpu) {cpu->regC |= 0x02;}
void lrcbCA(CPU* cpu) {cpu->regD |= 0x02;}
void lrcbCB(CPU* cpu) {cpu->regE |= 0x02;}
void lrcbCC(CPU* cpu) {cpu->regH |= 0x02;}
void lrcbCD(CPU* cpu) {cpu->regL |= 0x02;}
void lrcbCE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x02);}
void lrcbCF(CPU* cpu) {cpu->regA |= 0x02;}
// d0..d7	set 2,r
void lrcbD0(CPU* cpu) {cpu->regB |= 0x04;}
void lrcbD1(CPU* cpu) {cpu->regC |= 0x04;}
void lrcbD2(CPU* cpu) {cpu->regD |= 0x04;}
void lrcbD3(CPU* cpu) {cpu->regE |= 0x04;}
void lrcbD4(CPU* cpu) {cpu->regH |= 0x04;}
void lrcbD5(CPU* cpu) {cpu->regL |= 0x04;}
void lrcbD6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x04);}
void lrcbD7(CPU* cpu) {cpu->regA |= 0x04;}
// d8..df	set 3,r
void lrcbD8(CPU* cpu) {cpu->regB |= 0x08;}
void lrcbD9(CPU* cpu) {cpu->regC |= 0x08;}
void lrcbDA(CPU* cpu) {cpu->regD |= 0x08;}
void lrcbDB(CPU* cpu) {cpu->regE |= 0x08;}
void lrcbDC(CPU* cpu) {cpu->regH |= 0x08;}
void lrcbDD(CPU* cpu) {cpu->regL |= 0x08;}
void lrcbDE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x08);}
void lrcbDF(CPU* cpu) {cpu->regA |= 0x08;}
// e0..e7	set 4,r
void lrcbE0(CPU* cpu) {cpu->regB |= 0x10;}
void lrcbE1(CPU* cpu) {cpu->regC |= 0x10;}
void lrcbE2(CPU* cpu) {cpu->regD |= 0x10;}
void lrcbE3(CPU* cpu) {cpu->regE |= 0x10;}
void lrcbE4(CPU* cpu) {cpu->regH |= 0x10;}
void lrcbE5(CPU* cpu) {cpu->regL |= 0x10;}
void lrcbE6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x10);}
void lrcbE7(CPU* cpu) {cpu->regA |= 0x10;}
// e8..ef	set 5,r
void lrcbE8(CPU* cpu) {cpu->regB |= 0x20;}
void lrcbE9(CPU* cpu) {cpu->regC |= 0x20;}
void lrcbEA(CPU* cpu) {cpu->regD |= 0x20;}
void lrcbEB(CPU* cpu) {cpu->regE |= 0x20;}
void lrcbEC(CPU* cpu) {cpu->regH |= 0x20;}
void lrcbED(CPU* cpu) {cpu->regL |= 0x20;}
void lrcbEE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x20);}
void lrcbEF(CPU* cpu) {cpu->regA |= 0x20;}
// f0..f7	set 6,r
void lrcbF0(CPU* cpu) {cpu->regB |= 0x40;}
void lrcbF1(CPU* cpu) {cpu->regC |= 0x40;}
void lrcbF2(CPU* cpu) {cpu->regD |= 0x40;}
void lrcbF3(CPU* cpu) {cpu->regE |= 0x40;}
void lrcbF4(CPU* cpu) {cpu->regH |= 0x40;}
void lrcbF5(CPU* cpu) {cpu->regL |= 0x40;}
void lrcbF6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x40);}
void lrcbF7(CPU* cpu) {cpu->regA |= 0x40;}
// f8..ff	set 7,r
void lrcbF8(CPU* cpu) {cpu->regB |= 0x80;}
void lrcbF9(CPU* cpu) {cpu->regC |= 0x80;}
void lrcbFA(CPU* cpu) {cpu->regD |= 0x80;}
void lrcbFB(CPU* cpu) {cpu->regE |= 0x80;}
void lrcbFC(CPU* cpu) {cpu->regH |= 0x80;}
void lrcbFD(CPU* cpu) {cpu->regL |= 0x80;}
void lrcbFE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->regHL); cpu->t++; lr_mwr(cpu, cpu->regHL, cpu->tmp | 0x80);}
void lrcbFF(CPU* cpu) {cpu->regA |= 0x80;}

// ===

opCode lrcbTab[256]={
	{0,4,lrcb00,NULL,"rlc b"},
	{0,4,lrcb01,NULL,"rlc c"},
	{0,4,lrcb02,NULL,"rlc d"},
	{0,4,lrcb03,NULL,"rlc e"},
	{0,4,lrcb04,NULL,"rlc h"},
	{0,4,lrcb05,NULL,"rlc l"},
	{0,4,lrcb06,NULL,"rlc (hl)"},
	{0,4,lrcb07,NULL,"rlc a"},

	{0,4,lrcb08,NULL,"rrc b"},
	{0,4,lrcb09,NULL,"rrc c"},
	{0,4,lrcb0A,NULL,"rrc d"},
	{0,4,lrcb0B,NULL,"rrc e"},
	{0,4,lrcb0C,NULL,"rrc h"},
	{0,4,lrcb0D,NULL,"rrc l"},
	{0,4,lrcb0E,NULL,"rrc (hl)"},
	{0,4,lrcb0F,NULL,"rrc a"},

	{0,4,lrcb10,NULL,"rl b"},
	{0,4,lrcb11,NULL,"rl c"},
	{0,4,lrcb12,NULL,"rl d"},
	{0,4,lrcb13,NULL,"rl e"},
	{0,4,lrcb14,NULL,"rl h"},
	{0,4,lrcb15,NULL,"rl l"},
	{0,4,lrcb16,NULL,"rl (hl)"},
	{0,4,lrcb17,NULL,"rl a"},

	{0,4,lrcb18,NULL,"rr b"},
	{0,4,lrcb19,NULL,"rr c"},
	{0,4,lrcb1A,NULL,"rr d"},
	{0,4,lrcb1B,NULL,"rr e"},
	{0,4,lrcb1C,NULL,"rr h"},
	{0,4,lrcb1D,NULL,"rr l"},
	{0,4,lrcb1E,NULL,"rr (hl)"},
	{0,4,lrcb1F,NULL,"rr a"},

	{0,4,lrcb20,NULL,"sla b"},
	{0,4,lrcb21,NULL,"sla c"},
	{0,4,lrcb22,NULL,"sla d"},
	{0,4,lrcb23,NULL,"sla e"},
	{0,4,lrcb24,NULL,"sla h"},
	{0,4,lrcb25,NULL,"sla l"},
	{0,4,lrcb26,NULL,"sla (hl)"},
	{0,4,lrcb27,NULL,"sla a"},

	{0,4,lrcb28,NULL,"sra b"},
	{0,4,lrcb29,NULL,"sra c"},
	{0,4,lrcb2A,NULL,"sra d"},
	{0,4,lrcb2B,NULL,"sra e"},
	{0,4,lrcb2C,NULL,"sra h"},
	{0,4,lrcb2D,NULL,"sra l"},
	{0,4,lrcb2E,NULL,"sra (hl)"},
	{0,4,lrcb2F,NULL,"sra a"},

	{0,4,lrcb30,NULL,"swap b"},
	{0,4,lrcb31,NULL,"swap c"},
	{0,4,lrcb32,NULL,"swap d"},
	{0,4,lrcb33,NULL,"swap e"},
	{0,4,lrcb34,NULL,"swap h"},
	{0,4,lrcb35,NULL,"swap l"},
	{0,4,lrcb36,NULL,"swap (hl)"},
	{0,4,lrcb37,NULL,"swap a"},

	{0,4,lrcb38,NULL,"srl b"},
	{0,4,lrcb39,NULL,"srl c"},
	{0,4,lrcb3A,NULL,"srl d"},
	{0,4,lrcb3B,NULL,"srl e"},
	{0,4,lrcb3C,NULL,"srl h"},
	{0,4,lrcb3D,NULL,"srl l"},
	{0,4,lrcb3E,NULL,"srl (hl)"},
	{0,4,lrcb3F,NULL,"srl a"},

	{0,4,lrcb40,NULL,"bit 0,b"},
	{0,4,lrcb41,NULL,"bit 0,c"},
	{0,4,lrcb42,NULL,"bit 0,d"},
	{0,4,lrcb43,NULL,"bit 0,e"},
	{0,4,lrcb44,NULL,"bit 0,h"},
	{0,4,lrcb45,NULL,"bit 0,l"},
	{0,4,lrcb46,NULL,"bit 0,(hl)"},
	{0,4,lrcb47,NULL,"bit 0,a"},

	{0,4,lrcb48,NULL,"bit 1,b"},
	{0,4,lrcb49,NULL,"bit 1,c"},
	{0,4,lrcb4A,NULL,"bit 1,d"},
	{0,4,lrcb4B,NULL,"bit 1,e"},
	{0,4,lrcb4C,NULL,"bit 1,h"},
	{0,4,lrcb4D,NULL,"bit 1,l"},
	{0,4,lrcb4E,NULL,"bit 1,(hl)"},
	{0,4,lrcb4F,NULL,"bit 1,a"},

	{0,4,lrcb50,NULL,"bit 2,b"},
	{0,4,lrcb51,NULL,"bit 2,c"},
	{0,4,lrcb52,NULL,"bit 2,d"},
	{0,4,lrcb53,NULL,"bit 2,e"},
	{0,4,lrcb54,NULL,"bit 2,h"},
	{0,4,lrcb55,NULL,"bit 2,l"},
	{0,4,lrcb56,NULL,"bit 2,(hl)"},
	{0,4,lrcb57,NULL,"bit 2,a"},

	{0,4,lrcb58,NULL,"bit 3,b"},
	{0,4,lrcb59,NULL,"bit 3,c"},
	{0,4,lrcb5A,NULL,"bit 3,d"},
	{0,4,lrcb5B,NULL,"bit 3,e"},
	{0,4,lrcb5C,NULL,"bit 3,h"},
	{0,4,lrcb5D,NULL,"bit 3,l"},
	{0,4,lrcb5E,NULL,"bit 3,(hl)"},
	{0,4,lrcb5F,NULL,"bit 3,a"},

	{0,4,lrcb60,NULL,"bit 4,b"},
	{0,4,lrcb61,NULL,"bit 4,c"},
	{0,4,lrcb62,NULL,"bit 4,d"},
	{0,4,lrcb63,NULL,"bit 4,e"},
	{0,4,lrcb64,NULL,"bit 4,h"},
	{0,4,lrcb65,NULL,"bit 4,l"},
	{0,4,lrcb66,NULL,"bit 4,(hl)"},
	{0,4,lrcb67,NULL,"bit 4,a"},

	{0,4,lrcb68,NULL,"bit 5,b"},
	{0,4,lrcb69,NULL,"bit 5,c"},
	{0,4,lrcb6A,NULL,"bit 5,d"},
	{0,4,lrcb6B,NULL,"bit 5,e"},
	{0,4,lrcb6C,NULL,"bit 5,h"},
	{0,4,lrcb6D,NULL,"bit 5,l"},
	{0,4,lrcb6E,NULL,"bit 5,(hl)"},
	{0,4,lrcb6F,NULL,"bit 5,a"},

	{0,4,lrcb70,NULL,"bit 6,b"},
	{0,4,lrcb71,NULL,"bit 6,c"},
	{0,4,lrcb72,NULL,"bit 6,d"},
	{0,4,lrcb73,NULL,"bit 6,e"},
	{0,4,lrcb74,NULL,"bit 6,h"},
	{0,4,lrcb75,NULL,"bit 6,l"},
	{0,4,lrcb76,NULL,"bit 6,(hl)"},
	{0,4,lrcb77,NULL,"bit 6,a"},

	{0,4,lrcb78,NULL,"bit 7,b"},
	{0,4,lrcb79,NULL,"bit 7,c"},
	{0,4,lrcb7A,NULL,"bit 7,d"},
	{0,4,lrcb7B,NULL,"bit 7,e"},
	{0,4,lrcb7C,NULL,"bit 7,h"},
	{0,4,lrcb7D,NULL,"bit 7,l"},
	{0,4,lrcb7E,NULL,"bit 7,(hl)"},
	{0,4,lrcb7F,NULL,"bit 7,a"},

	{0,4,lrcb80,NULL,"res 0,b"},
	{0,4,lrcb81,NULL,"res 0,c"},
	{0,4,lrcb82,NULL,"res 0,d"},
	{0,4,lrcb83,NULL,"res 0,e"},
	{0,4,lrcb84,NULL,"res 0,h"},
	{0,4,lrcb85,NULL,"res 0,l"},
	{0,4,lrcb86,NULL,"res 0,(hl)"},
	{0,4,lrcb87,NULL,"res 0,a"},

	{0,4,lrcb88,NULL,"res 1,b"},
	{0,4,lrcb89,NULL,"res 1,c"},
	{0,4,lrcb8A,NULL,"res 1,d"},
	{0,4,lrcb8B,NULL,"res 1,e"},
	{0,4,lrcb8C,NULL,"res 1,h"},
	{0,4,lrcb8D,NULL,"res 1,l"},
	{0,4,lrcb8E,NULL,"res 1,(hl)"},
	{0,4,lrcb8F,NULL,"res 1,a"},

	{0,4,lrcb90,NULL,"res 2,b"},
	{0,4,lrcb91,NULL,"res 2,c"},
	{0,4,lrcb92,NULL,"res 2,d"},
	{0,4,lrcb93,NULL,"res 2,e"},
	{0,4,lrcb94,NULL,"res 2,h"},
	{0,4,lrcb95,NULL,"res 2,l"},
	{0,4,lrcb96,NULL,"res 2,(hl)"},
	{0,4,lrcb97,NULL,"res 2,a"},

	{0,4,lrcb98,NULL,"res 3,b"},
	{0,4,lrcb99,NULL,"res 3,c"},
	{0,4,lrcb9A,NULL,"res 3,d"},
	{0,4,lrcb9B,NULL,"res 3,e"},
	{0,4,lrcb9C,NULL,"res 3,h"},
	{0,4,lrcb9D,NULL,"res 3,l"},
	{0,4,lrcb9E,NULL,"res 3,(hl)"},
	{0,4,lrcb9F,NULL,"res 3,a"},

	{0,4,lrcbA0,NULL,"res 4,b"},
	{0,4,lrcbA1,NULL,"res 4,c"},
	{0,4,lrcbA2,NULL,"res 4,d"},
	{0,4,lrcbA3,NULL,"res 4,e"},
	{0,4,lrcbA4,NULL,"res 4,h"},
	{0,4,lrcbA5,NULL,"res 4,l"},
	{0,4,lrcbA6,NULL,"res 4,(hl)"},
	{0,4,lrcbA7,NULL,"res 4,a"},

	{0,4,lrcbA8,NULL,"res 5,b"},
	{0,4,lrcbA9,NULL,"res 5,c"},
	{0,4,lrcbAA,NULL,"res 5,d"},
	{0,4,lrcbAB,NULL,"res 5,e"},
	{0,4,lrcbAC,NULL,"res 5,h"},
	{0,4,lrcbAD,NULL,"res 5,l"},
	{0,4,lrcbAE,NULL,"res 5,(hl)"},
	{0,4,lrcbAF,NULL,"res 5,a"},

	{0,4,lrcbB0,NULL,"res 6,b"},
	{0,4,lrcbB1,NULL,"res 6,c"},
	{0,4,lrcbB2,NULL,"res 6,d"},
	{0,4,lrcbB3,NULL,"res 6,e"},
	{0,4,lrcbB4,NULL,"res 6,h"},
	{0,4,lrcbB5,NULL,"res 6,l"},
	{0,4,lrcbB6,NULL,"res 6,(hl)"},
	{0,4,lrcbB7,NULL,"res 6,a"},

	{0,4,lrcbB8,NULL,"res 7,b"},
	{0,4,lrcbB9,NULL,"res 7,c"},
	{0,4,lrcbBA,NULL,"res 7,d"},
	{0,4,lrcbBB,NULL,"res 7,e"},
	{0,4,lrcbBC,NULL,"res 7,h"},
	{0,4,lrcbBD,NULL,"res 7,l"},
	{0,4,lrcbBE,NULL,"res 7,(hl)"},
	{0,4,lrcbBF,NULL,"res 7,a"},

	{0,4,lrcbC0,NULL,"set 0,b"},
	{0,4,lrcbC1,NULL,"set 0,c"},
	{0,4,lrcbC2,NULL,"set 0,d"},
	{0,4,lrcbC3,NULL,"set 0,e"},
	{0,4,lrcbC4,NULL,"set 0,h"},
	{0,4,lrcbC5,NULL,"set 0,l"},
	{0,4,lrcbC6,NULL,"set 0,(hl)"},
	{0,4,lrcbC7,NULL,"set 0,a"},

	{0,4,lrcbC8,NULL,"set 1,b"},
	{0,4,lrcbC9,NULL,"set 1,c"},
	{0,4,lrcbCA,NULL,"set 1,d"},
	{0,4,lrcbCB,NULL,"set 1,e"},
	{0,4,lrcbCC,NULL,"set 1,h"},
	{0,4,lrcbCD,NULL,"set 1,l"},
	{0,4,lrcbCE,NULL,"set 1,(hl)"},
	{0,4,lrcbCF,NULL,"set 1,a"},

	{0,4,lrcbD0,NULL,"set 2,b"},
	{0,4,lrcbD1,NULL,"set 2,c"},
	{0,4,lrcbD2,NULL,"set 2,d"},
	{0,4,lrcbD3,NULL,"set 2,e"},
	{0,4,lrcbD4,NULL,"set 2,h"},
	{0,4,lrcbD5,NULL,"set 2,l"},
	{0,4,lrcbD6,NULL,"set 2,(hl)"},
	{0,4,lrcbD7,NULL,"set 2,a"},

	{0,4,lrcbD8,NULL,"set 3,b"},
	{0,4,lrcbD9,NULL,"set 3,c"},
	{0,4,lrcbDA,NULL,"set 3,d"},
	{0,4,lrcbDB,NULL,"set 3,e"},
	{0,4,lrcbDC,NULL,"set 3,h"},
	{0,4,lrcbDD,NULL,"set 3,l"},
	{0,4,lrcbDE,NULL,"set 3,(hl)"},
	{0,4,lrcbDF,NULL,"set 3,a"},

	{0,4,lrcbE0,NULL,"set 4,b"},
	{0,4,lrcbE1,NULL,"set 4,c"},
	{0,4,lrcbE2,NULL,"set 4,d"},
	{0,4,lrcbE3,NULL,"set 4,e"},
	{0,4,lrcbE4,NULL,"set 4,h"},
	{0,4,lrcbE5,NULL,"set 4,l"},
	{0,4,lrcbE6,NULL,"set 4,(hl)"},
	{0,4,lrcbE7,NULL,"set 4,a"},

	{0,4,lrcbE8,NULL,"set 5,b"},
	{0,4,lrcbE9,NULL,"set 5,c"},
	{0,4,lrcbEA,NULL,"set 5,d"},
	{0,4,lrcbEB,NULL,"set 5,e"},
	{0,4,lrcbEC,NULL,"set 5,h"},
	{0,4,lrcbED,NULL,"set 5,l"},
	{0,4,lrcbEE,NULL,"set 5,(hl)"},
	{0,4,lrcbEF,NULL,"set 5,a"},

	{0,4,lrcbF0,NULL,"set 6,b"},
	{0,4,lrcbF1,NULL,"set 6,c"},
	{0,4,lrcbF2,NULL,"set 6,d"},
	{0,4,lrcbF3,NULL,"set 6,e"},
	{0,4,lrcbF4,NULL,"set 6,h"},
	{0,4,lrcbF5,NULL,"set 6,l"},
	{0,4,lrcbF6,NULL,"set 6,(hl)"},
	{0,4,lrcbF7,NULL,"set 6,a"},

	{0,4,lrcbF8,NULL,"set 7,b"},
	{0,4,lrcbF9,NULL,"set 7,c"},
	{0,4,lrcbFA,NULL,"set 7,d"},
	{0,4,lrcbFB,NULL,"set 7,e"},
	{0,4,lrcbFC,NULL,"set 7,h"},
	{0,4,lrcbFD,NULL,"set 7,l"},
	{0,4,lrcbFE,NULL,"set 7,(hl)"},
	{0,4,lrcbFF,NULL,"set 7,a"},
};
