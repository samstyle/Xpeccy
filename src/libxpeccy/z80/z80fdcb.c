// rlc reg,(iy+e)	[3rd] 4 5add 4rd 3wr
void fdcb00(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->b);}
void fdcb01(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->c);}
void fdcb02(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->d);}
void fdcb03(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->e);}
void fdcb04(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->h);}
void fdcb05(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->l);}
void fdcb06(Z80CPU* cpu) {XDCB(cpu->iy,RLC);}
void fdcb07(Z80CPU* cpu) {XDCBR(cpu->iy,RLC,cpu->a);}
// rrc reg,(iy+e)
void fdcb08(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->b);}
void fdcb09(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->c);}
void fdcb0A(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->d);}
void fdcb0B(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->e);}
void fdcb0C(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->h);}
void fdcb0D(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->l);}
void fdcb0E(Z80CPU* cpu) {XDCB(cpu->iy,RRC);}
void fdcb0F(Z80CPU* cpu) {XDCBR(cpu->iy,RRC,cpu->a);}
// rl reg,(iy+e)
void fdcb10(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->b);}
void fdcb11(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->c);}
void fdcb12(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->d);}
void fdcb13(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->e);}
void fdcb14(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->h);}
void fdcb15(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->l);}
void fdcb16(Z80CPU* cpu) {XDCB(cpu->iy,RL);}
void fdcb17(Z80CPU* cpu) {XDCBR(cpu->iy,RL,cpu->a);}
// rr reg,(iy+e)
void fdcb18(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->b);}
void fdcb19(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->c);}
void fdcb1A(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->d);}
void fdcb1B(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->e);}
void fdcb1C(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->h);}
void fdcb1D(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->l);}
void fdcb1E(Z80CPU* cpu) {XDCB(cpu->iy,RR);}
void fdcb1F(Z80CPU* cpu) {XDCBR(cpu->iy,RR,cpu->a);}
// sla reg,(iy+e)
void fdcb20(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->b);}
void fdcb21(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->c);}
void fdcb22(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->d);}
void fdcb23(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->e);}
void fdcb24(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->h);}
void fdcb25(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->l);}
void fdcb26(Z80CPU* cpu) {XDCB(cpu->iy,SLA);}
void fdcb27(Z80CPU* cpu) {XDCBR(cpu->iy,SLA,cpu->a);}
// sra reg,(iy+e)
void fdcb28(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->b);}
void fdcb29(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->c);}
void fdcb2A(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->d);}
void fdcb2B(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->e);}
void fdcb2C(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->h);}
void fdcb2D(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->l);}
void fdcb2E(Z80CPU* cpu) {XDCB(cpu->iy,SRA);}
void fdcb2F(Z80CPU* cpu) {XDCBR(cpu->iy,SRA,cpu->a);}
// sll reg,(iy+e)
void fdcb30(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->b);}
void fdcb31(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->c);}
void fdcb32(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->d);}
void fdcb33(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->e);}
void fdcb34(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->h);}
void fdcb35(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->l);}
void fdcb36(Z80CPU* cpu) {XDCB(cpu->iy,SLL);}
void fdcb37(Z80CPU* cpu) {XDCBR(cpu->iy,SLL,cpu->a);}
// srl reg,(iy+e)
void fdcb38(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->b);}
void fdcb39(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->c);}
void fdcb3A(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->d);}
void fdcb3B(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->e);}
void fdcb3C(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->h);}
void fdcb3D(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->l);}
void fdcb3E(Z80CPU* cpu) {XDCB(cpu->iy,SRL);}
void fdcb3F(Z80CPU* cpu) {XDCBR(cpu->iy,SRL,cpu->a);}

// bit n,(iy+e)
void fdcb46(Z80CPU* cpu) {BITX(cpu->iy,0);}
void fdcb4E(Z80CPU* cpu) {BITX(cpu->iy,1);}
void fdcb56(Z80CPU* cpu) {BITX(cpu->iy,2);}
void fdcb5E(Z80CPU* cpu) {BITX(cpu->iy,3);}
void fdcb66(Z80CPU* cpu) {BITX(cpu->iy,4);}
void fdcb6E(Z80CPU* cpu) {BITX(cpu->iy,5);}
void fdcb76(Z80CPU* cpu) {BITX(cpu->iy,6);}
void fdcb7E(Z80CPU* cpu) {BITX(cpu->iy,7);}

// res 0,reg,(iy+e)
void fdcb80(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->b);}
void fdcb81(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->c);}
void fdcb82(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->d);}
void fdcb83(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->e);}
void fdcb84(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->h);}
void fdcb85(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->l);}
void fdcb86(Z80CPU* cpu) {RESX(cpu->iy,0);}
void fdcb87(Z80CPU* cpu) {RESXR(cpu->iy,0,cpu->a);}
// res 1,reg,(iy+e)
void fdcb88(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->b);}
void fdcb89(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->c);}
void fdcb8A(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->d);}
void fdcb8B(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->e);}
void fdcb8C(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->h);}
void fdcb8D(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->l);}
void fdcb8E(Z80CPU* cpu) {RESX(cpu->iy,1);}
void fdcb8F(Z80CPU* cpu) {RESXR(cpu->iy,1,cpu->a);}
// res 2,reg,(iy+e)
void fdcb90(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->b);}
void fdcb91(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->c);}
void fdcb92(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->d);}
void fdcb93(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->e);}
void fdcb94(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->h);}
void fdcb95(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->l);}
void fdcb96(Z80CPU* cpu) {RESX(cpu->iy,2);}
void fdcb97(Z80CPU* cpu) {RESXR(cpu->iy,2,cpu->a);}
// res 3,reg,(iy+e)
void fdcb98(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->b);}
void fdcb99(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->c);}
void fdcb9A(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->d);}
void fdcb9B(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->e);}
void fdcb9C(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->h);}
void fdcb9D(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->l);}
void fdcb9E(Z80CPU* cpu) {RESX(cpu->iy,3);}
void fdcb9F(Z80CPU* cpu) {RESXR(cpu->iy,3,cpu->a);}
// res 4,reg,(iy+e)
void fdcbA0(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->b);}
void fdcbA1(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->c);}
void fdcbA2(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->d);}
void fdcbA3(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->e);}
void fdcbA4(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->h);}
void fdcbA5(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->l);}
void fdcbA6(Z80CPU* cpu) {RESX(cpu->iy,4);}
void fdcbA7(Z80CPU* cpu) {RESXR(cpu->iy,4,cpu->a);}
// res 5,reg,(iy+e)
void fdcbA8(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->b);}
void fdcbA9(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->c);}
void fdcbAA(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->d);}
void fdcbAB(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->e);}
void fdcbAC(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->h);}
void fdcbAD(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->l);}
void fdcbAE(Z80CPU* cpu) {RESX(cpu->iy,5);}
void fdcbAF(Z80CPU* cpu) {RESXR(cpu->iy,5,cpu->a);}
// res 6,reg,(iy+e)
void fdcbB0(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->b);}
void fdcbB1(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->c);}
void fdcbB2(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->d);}
void fdcbB3(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->e);}
void fdcbB4(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->h);}
void fdcbB5(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->l);}
void fdcbB6(Z80CPU* cpu) {RESX(cpu->iy,6);}
void fdcbB7(Z80CPU* cpu) {RESXR(cpu->iy,6,cpu->a);}
// res 7,reg,(iy+e)
void fdcbB8(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->b);}
void fdcbB9(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->c);}
void fdcbBA(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->d);}
void fdcbBB(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->e);}
void fdcbBC(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->h);}
void fdcbBD(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->l);}
void fdcbBE(Z80CPU* cpu) {RESX(cpu->iy,7);}
void fdcbBF(Z80CPU* cpu) {RESXR(cpu->iy,7,cpu->a);}

// set 0,reg,(iy+e)
void fdcbC0(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->b);}
void fdcbC1(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->c);}
void fdcbC2(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->d);}
void fdcbC3(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->e);}
void fdcbC4(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->h);}
void fdcbC5(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->l);}
void fdcbC6(Z80CPU* cpu) {SETX(cpu->iy,0);}
void fdcbC7(Z80CPU* cpu) {SETXR(cpu->iy,0,cpu->a);}
// set 1,reg,(iy+e)
void fdcbC8(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->b);}
void fdcbC9(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->c);}
void fdcbCA(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->d);}
void fdcbCB(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->e);}
void fdcbCC(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->h);}
void fdcbCD(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->l);}
void fdcbCE(Z80CPU* cpu) {SETX(cpu->iy,1);}
void fdcbCF(Z80CPU* cpu) {SETXR(cpu->iy,1,cpu->a);}
// set 2,reg,(iy+e)
void fdcbD0(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->b);}
void fdcbD1(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->c);}
void fdcbD2(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->d);}
void fdcbD3(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->e);}
void fdcbD4(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->h);}
void fdcbD5(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->l);}
void fdcbD6(Z80CPU* cpu) {SETX(cpu->iy,2);}
void fdcbD7(Z80CPU* cpu) {SETXR(cpu->iy,2,cpu->a);}
// set 3,reg,(iy+e)
void fdcbD8(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->b);}
void fdcbD9(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->c);}
void fdcbDA(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->d);}
void fdcbDB(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->e);}
void fdcbDC(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->h);}
void fdcbDD(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->l);}
void fdcbDE(Z80CPU* cpu) {SETX(cpu->iy,3);}
void fdcbDF(Z80CPU* cpu) {SETXR(cpu->iy,3,cpu->a);}
// set 4,reg,(iy+e)
void fdcbE0(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->b);}
void fdcbE1(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->c);}
void fdcbE2(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->d);}
void fdcbE3(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->e);}
void fdcbE4(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->h);}
void fdcbE5(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->l);}
void fdcbE6(Z80CPU* cpu) {SETX(cpu->iy,4);}
void fdcbE7(Z80CPU* cpu) {SETXR(cpu->iy,4,cpu->a);}
// set 5,reg,(iy+e)
void fdcbE8(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->b);}
void fdcbE9(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->c);}
void fdcbEA(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->d);}
void fdcbEB(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->e);}
void fdcbEC(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->h);}
void fdcbED(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->l);}
void fdcbEE(Z80CPU* cpu) {SETX(cpu->iy,5);}
void fdcbEF(Z80CPU* cpu) {SETXR(cpu->iy,5,cpu->a);}
// set 6,reg,(iy+e)
void fdcbF0(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->b);}
void fdcbF1(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->c);}
void fdcbF2(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->d);}
void fdcbF3(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->e);}
void fdcbF4(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->h);}
void fdcbF5(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->l);}
void fdcbF6(Z80CPU* cpu) {SETX(cpu->iy,6);}
void fdcbF7(Z80CPU* cpu) {SETXR(cpu->iy,6,cpu->a);}
// set 7,reg,(iy+e)
void fdcbF8(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->b);}
void fdcbF9(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->c);}
void fdcbFA(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->d);}
void fdcbFB(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->e);}
void fdcbFC(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->h);}
void fdcbFD(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->l);}
void fdcbFE(Z80CPU* cpu) {SETX(cpu->iy,7);}
void fdcbFF(Z80CPU* cpu) {SETXR(cpu->iy,7,cpu->a);}

//====
// opcode fetch doesn't eat 4T?

opCode fdcbTab[256]={
	{0,0,5,4,3,0,0,&fdcb00,NULL,"rlc b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb01,NULL,"rlc c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb02,NULL,"rlc d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb03,NULL,"rlc e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb04,NULL,"rlc h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb05,NULL,"rlc l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb06,NULL,"rlc (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb07,NULL,"rlc a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb08,NULL,"rrc b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb09,NULL,"rrc c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb0A,NULL,"rrc d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb0B,NULL,"rrc e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb0C,NULL,"rrc h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb0D,NULL,"rrc l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb0E,NULL,"rrc (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb0F,NULL,"rrc a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb10,NULL,"rl b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb11,NULL,"rl c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb12,NULL,"rl d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb13,NULL,"rl e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb14,NULL,"rl h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb15,NULL,"rl l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb16,NULL,"rl (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb17,NULL,"rl a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb18,NULL,"rr b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb19,NULL,"rr c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb1A,NULL,"rr d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb1B,NULL,"rr e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb1C,NULL,"rr h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb1D,NULL,"rr l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb1E,NULL,"rr (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb1F,NULL,"rr a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb20,NULL,"sla b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb21,NULL,"sla c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb22,NULL,"sla d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb23,NULL,"sla e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb24,NULL,"sla h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb25,NULL,"sla l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb26,NULL,"sla (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb27,NULL,"sla a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb28,NULL,"sra b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb29,NULL,"sra c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb2A,NULL,"sra d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb2B,NULL,"sra e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb2C,NULL,"sra h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb2D,NULL,"sra l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb2E,NULL,"sra (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb2F,NULL,"sra a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb30,NULL,"sll b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb31,NULL,"sll c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb32,NULL,"sll d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb33,NULL,"sll e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb34,NULL,"sll h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb35,NULL,"sll l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb36,NULL,"sll (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb37,NULL,"sll a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb38,NULL,"srl b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb39,NULL,"srl c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb3A,NULL,"srl d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb3B,NULL,"srl e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb3C,NULL,"srl h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb3D,NULL,"srl l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb3E,NULL,"srl (iy:5)"},
	{0,0,5,4,3,0,0,&fdcb3F,NULL,"srl a,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb46,NULL,"bit 0,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb4E,NULL,"bit 1,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb56,NULL,"bit 2,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb5E,NULL,"bit 3,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb66,NULL,"bit 4,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb6E,NULL,"bit 5,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb76,NULL,"bit 6,(iy:5)"},

	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},
	{0,0,5,4,0,0,0,&fdcb7E,NULL,"bit 7,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb80,NULL,"res 0,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb81,NULL,"res 0,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb82,NULL,"res 0,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb83,NULL,"res 0,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb84,NULL,"res 0,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb85,NULL,"res 0,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb86,NULL,"res 0,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb87,NULL,"res 0,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb88,NULL,"res 1,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb89,NULL,"res 1,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb8A,NULL,"res 1,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb8B,NULL,"res 1,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb8C,NULL,"res 1,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb8D,NULL,"res 1,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb8E,NULL,"res 1,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb8F,NULL,"res 1,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb90,NULL,"res 2,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb91,NULL,"res 2,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb92,NULL,"res 2,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb93,NULL,"res 2,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb94,NULL,"res 2,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb95,NULL,"res 2,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb96,NULL,"res 2,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb97,NULL,"res 2,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcb98,NULL,"res 3,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb99,NULL,"res 3,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb9A,NULL,"res 3,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb9B,NULL,"res 3,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb9C,NULL,"res 3,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb9D,NULL,"res 3,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb9E,NULL,"res 3,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcb9F,NULL,"res 3,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbA0,NULL,"res 4,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA1,NULL,"res 4,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA2,NULL,"res 4,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA3,NULL,"res 4,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA4,NULL,"res 4,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA5,NULL,"res 4,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA6,NULL,"res 4,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA7,NULL,"res 4,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbA8,NULL,"res 5,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbA9,NULL,"res 5,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbAA,NULL,"res 5,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbAB,NULL,"res 5,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbAC,NULL,"res 5,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbAD,NULL,"res 5,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbAE,NULL,"res 5,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbAF,NULL,"res 5,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbB0,NULL,"res 6,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB1,NULL,"res 6,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB2,NULL,"res 6,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB3,NULL,"res 6,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB4,NULL,"res 6,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB5,NULL,"res 6,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB6,NULL,"res 6,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB7,NULL,"res 6,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbB8,NULL,"res 7,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbB9,NULL,"res 7,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbBA,NULL,"res 7,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbBB,NULL,"res 7,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbBC,NULL,"res 7,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbBD,NULL,"res 7,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbBE,NULL,"res 7,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbBF,NULL,"res 7,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbC0,NULL,"set 0,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC1,NULL,"set 0,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC2,NULL,"set 0,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC3,NULL,"set 0,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC4,NULL,"set 0,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC5,NULL,"set 0,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC6,NULL,"set 0,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC7,NULL,"set 0,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbC8,NULL,"set 1,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbC9,NULL,"set 1,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbCA,NULL,"set 1,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbCB,NULL,"set 1,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbCC,NULL,"set 1,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbCD,NULL,"set 1,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbCE,NULL,"set 1,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbCF,NULL,"set 1,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbD0,NULL,"set 2,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD1,NULL,"set 2,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD2,NULL,"set 2,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD3,NULL,"set 2,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD4,NULL,"set 2,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD5,NULL,"set 2,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD6,NULL,"set 2,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD7,NULL,"set 2,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbD8,NULL,"set 3,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbD9,NULL,"set 3,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbDA,NULL,"set 3,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbDB,NULL,"set 3,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbDC,NULL,"set 3,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbDD,NULL,"set 3,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbDE,NULL,"set 3,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbDF,NULL,"set 3,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbE0,NULL,"set 4,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE1,NULL,"set 4,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE2,NULL,"set 4,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE3,NULL,"set 4,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE4,NULL,"set 4,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE5,NULL,"set 4,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE6,NULL,"set 4,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE7,NULL,"set 4,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbE8,NULL,"set 5,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbE9,NULL,"set 5,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbEA,NULL,"set 5,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbEB,NULL,"set 5,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbEC,NULL,"set 5,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbED,NULL,"set 5,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbEE,NULL,"set 5,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbEF,NULL,"set 5,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbF0,NULL,"set 6,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF1,NULL,"set 6,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF2,NULL,"set 6,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF3,NULL,"set 6,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF4,NULL,"set 6,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF5,NULL,"set 6,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF6,NULL,"set 6,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF7,NULL,"set 6,a,(iy:5)"},

	{0,0,5,4,3,0,0,&fdcbF8,NULL,"set 7,b,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbF9,NULL,"set 7,c,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbFA,NULL,"set 7,d,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbFB,NULL,"set 7,e,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbFC,NULL,"set 7,h,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbFD,NULL,"set 7,l,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbFE,NULL,"set 7,(iy:5)"},
	{0,0,5,4,3,0,0,&fdcbFF,NULL,"set 7,a,(iy:5)"},
};
