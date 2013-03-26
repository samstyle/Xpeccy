// rlc reg,(ix+e)	[3rd] 4 5add 4rd 3wr
void ddcb00(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->b);}
void ddcb01(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->c);}
void ddcb02(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->d);}
void ddcb03(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->e);}
void ddcb04(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->h);}
void ddcb05(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->l);}
void ddcb06(Z80CPU* cpu) {XDCB(cpu->ix,RLC);}
void ddcb07(Z80CPU* cpu) {XDCBR(cpu->ix,RLC,cpu->a);}
// rrc reg,(ix+e)
void ddcb08(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->b);}
void ddcb09(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->c);}
void ddcb0A(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->d);}
void ddcb0B(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->e);}
void ddcb0C(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->h);}
void ddcb0D(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->l);}
void ddcb0E(Z80CPU* cpu) {XDCB(cpu->ix,RRC);}
void ddcb0F(Z80CPU* cpu) {XDCBR(cpu->ix,RRC,cpu->a);}
// rl reg,(ix+e)
void ddcb10(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->b);}
void ddcb11(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->c);}
void ddcb12(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->d);}
void ddcb13(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->e);}
void ddcb14(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->h);}
void ddcb15(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->l);}
void ddcb16(Z80CPU* cpu) {XDCB(cpu->ix,RL);}
void ddcb17(Z80CPU* cpu) {XDCBR(cpu->ix,RL,cpu->a);}
// rr reg,(ix+e)
void ddcb18(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->b);}
void ddcb19(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->c);}
void ddcb1A(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->d);}
void ddcb1B(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->e);}
void ddcb1C(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->h);}
void ddcb1D(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->l);}
void ddcb1E(Z80CPU* cpu) {XDCB(cpu->ix,RR);}
void ddcb1F(Z80CPU* cpu) {XDCBR(cpu->ix,RR,cpu->a);}
// sla reg,(ix+e)
void ddcb20(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->b);}
void ddcb21(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->c);}
void ddcb22(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->d);}
void ddcb23(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->e);}
void ddcb24(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->h);}
void ddcb25(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->l);}
void ddcb26(Z80CPU* cpu) {XDCB(cpu->ix,SLA);}
void ddcb27(Z80CPU* cpu) {XDCBR(cpu->ix,SLA,cpu->a);}
// sra reg,(ix+e)
void ddcb28(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->b);}
void ddcb29(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->c);}
void ddcb2A(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->d);}
void ddcb2B(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->e);}
void ddcb2C(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->h);}
void ddcb2D(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->l);}
void ddcb2E(Z80CPU* cpu) {XDCB(cpu->ix,SRA);}
void ddcb2F(Z80CPU* cpu) {XDCBR(cpu->ix,SRA,cpu->a);}
// sll reg,(ix+e)
void ddcb30(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->b);}
void ddcb31(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->c);}
void ddcb32(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->d);}
void ddcb33(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->e);}
void ddcb34(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->h);}
void ddcb35(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->l);}
void ddcb36(Z80CPU* cpu) {XDCB(cpu->ix,SLL);}
void ddcb37(Z80CPU* cpu) {XDCBR(cpu->ix,SLL,cpu->a);}
// srl reg,(ix+e)
void ddcb38(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->b);}
void ddcb39(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->c);}
void ddcb3A(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->d);}
void ddcb3B(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->e);}
void ddcb3C(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->h);}
void ddcb3D(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->l);}
void ddcb3E(Z80CPU* cpu) {XDCB(cpu->ix,SRL);}
void ddcb3F(Z80CPU* cpu) {XDCBR(cpu->ix,SRL,cpu->a);}

// bit n,(ix+e)
void ddcb46(Z80CPU* cpu) {BITX(cpu->ix,0);}
void ddcb4E(Z80CPU* cpu) {BITX(cpu->ix,1);}
void ddcb56(Z80CPU* cpu) {BITX(cpu->ix,2);}
void ddcb5E(Z80CPU* cpu) {BITX(cpu->ix,3);}
void ddcb66(Z80CPU* cpu) {BITX(cpu->ix,4);}
void ddcb6E(Z80CPU* cpu) {BITX(cpu->ix,5);}
void ddcb76(Z80CPU* cpu) {BITX(cpu->ix,6);}
void ddcb7E(Z80CPU* cpu) {BITX(cpu->ix,7);}

// res 0,reg,(ix+e)
void ddcb80(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->b);}
void ddcb81(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->c);}
void ddcb82(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->d);}
void ddcb83(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->e);}
void ddcb84(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->h);}
void ddcb85(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->l);}
void ddcb86(Z80CPU* cpu) {RESX(cpu->ix,0);}
void ddcb87(Z80CPU* cpu) {RESXR(cpu->ix,0,cpu->a);}
// res 1,reg,(ix+e)
void ddcb88(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->b);}
void ddcb89(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->c);}
void ddcb8A(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->d);}
void ddcb8B(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->e);}
void ddcb8C(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->h);}
void ddcb8D(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->l);}
void ddcb8E(Z80CPU* cpu) {RESX(cpu->ix,1);}
void ddcb8F(Z80CPU* cpu) {RESXR(cpu->ix,1,cpu->a);}
// res 2,reg,(ix+e)
void ddcb90(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->b);}
void ddcb91(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->c);}
void ddcb92(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->d);}
void ddcb93(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->e);}
void ddcb94(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->h);}
void ddcb95(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->l);}
void ddcb96(Z80CPU* cpu) {RESX(cpu->ix,2);}
void ddcb97(Z80CPU* cpu) {RESXR(cpu->ix,2,cpu->a);}
// res 3,reg,(ix+e)
void ddcb98(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->b);}
void ddcb99(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->c);}
void ddcb9A(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->d);}
void ddcb9B(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->e);}
void ddcb9C(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->h);}
void ddcb9D(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->l);}
void ddcb9E(Z80CPU* cpu) {RESX(cpu->ix,3);}
void ddcb9F(Z80CPU* cpu) {RESXR(cpu->ix,3,cpu->a);}
// res 4,reg,(ix+e)
void ddcbA0(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->b);}
void ddcbA1(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->c);}
void ddcbA2(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->d);}
void ddcbA3(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->e);}
void ddcbA4(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->h);}
void ddcbA5(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->l);}
void ddcbA6(Z80CPU* cpu) {RESX(cpu->ix,4);}
void ddcbA7(Z80CPU* cpu) {RESXR(cpu->ix,4,cpu->a);}
// res 5,reg,(ix+e)
void ddcbA8(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->b);}
void ddcbA9(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->c);}
void ddcbAA(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->d);}
void ddcbAB(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->e);}
void ddcbAC(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->h);}
void ddcbAD(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->l);}
void ddcbAE(Z80CPU* cpu) {RESX(cpu->ix,5);}
void ddcbAF(Z80CPU* cpu) {RESXR(cpu->ix,5,cpu->a);}
// res 6,reg,(ix+e)
void ddcbB0(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->b);}
void ddcbB1(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->c);}
void ddcbB2(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->d);}
void ddcbB3(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->e);}
void ddcbB4(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->h);}
void ddcbB5(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->l);}
void ddcbB6(Z80CPU* cpu) {RESX(cpu->ix,6);}
void ddcbB7(Z80CPU* cpu) {RESXR(cpu->ix,6,cpu->a);}
// res 7,reg,(ix+e)
void ddcbB8(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->b);}
void ddcbB9(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->c);}
void ddcbBA(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->d);}
void ddcbBB(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->e);}
void ddcbBC(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->h);}
void ddcbBD(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->l);}
void ddcbBE(Z80CPU* cpu) {RESX(cpu->ix,7);}
void ddcbBF(Z80CPU* cpu) {RESXR(cpu->ix,7,cpu->a);}

// set 0,reg,(ix+e)
void ddcbC0(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->b);}
void ddcbC1(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->c);}
void ddcbC2(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->d);}
void ddcbC3(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->e);}
void ddcbC4(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->h);}
void ddcbC5(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->l);}
void ddcbC6(Z80CPU* cpu) {SETX(cpu->ix,0);}
void ddcbC7(Z80CPU* cpu) {SETXR(cpu->ix,0,cpu->a);}
// set 1,reg,(ix+e)
void ddcbC8(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->b);}
void ddcbC9(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->c);}
void ddcbCA(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->d);}
void ddcbCB(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->e);}
void ddcbCC(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->h);}
void ddcbCD(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->l);}
void ddcbCE(Z80CPU* cpu) {SETX(cpu->ix,1);}
void ddcbCF(Z80CPU* cpu) {SETXR(cpu->ix,1,cpu->a);}
// set 2,reg,(ix+e)
void ddcbD0(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->b);}
void ddcbD1(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->c);}
void ddcbD2(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->d);}
void ddcbD3(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->e);}
void ddcbD4(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->h);}
void ddcbD5(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->l);}
void ddcbD6(Z80CPU* cpu) {SETX(cpu->ix,2);}
void ddcbD7(Z80CPU* cpu) {SETXR(cpu->ix,2,cpu->a);}
// set 3,reg,(ix+e)
void ddcbD8(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->b);}
void ddcbD9(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->c);}
void ddcbDA(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->d);}
void ddcbDB(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->e);}
void ddcbDC(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->h);}
void ddcbDD(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->l);}
void ddcbDE(Z80CPU* cpu) {SETX(cpu->ix,3);}
void ddcbDF(Z80CPU* cpu) {SETXR(cpu->ix,3,cpu->a);}
// set 4,reg,(ix+e)
void ddcbE0(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->b);}
void ddcbE1(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->c);}
void ddcbE2(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->d);}
void ddcbE3(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->e);}
void ddcbE4(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->h);}
void ddcbE5(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->l);}
void ddcbE6(Z80CPU* cpu) {SETX(cpu->ix,4);}
void ddcbE7(Z80CPU* cpu) {SETXR(cpu->ix,4,cpu->a);}
// set 5,reg,(ix+e)
void ddcbE8(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->b);}
void ddcbE9(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->c);}
void ddcbEA(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->d);}
void ddcbEB(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->e);}
void ddcbEC(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->h);}
void ddcbED(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->l);}
void ddcbEE(Z80CPU* cpu) {SETX(cpu->ix,5);}
void ddcbEF(Z80CPU* cpu) {SETXR(cpu->ix,5,cpu->a);}
// set 6,reg,(ix+e)
void ddcbF0(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->b);}
void ddcbF1(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->c);}
void ddcbF2(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->d);}
void ddcbF3(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->e);}
void ddcbF4(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->h);}
void ddcbF5(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->l);}
void ddcbF6(Z80CPU* cpu) {SETX(cpu->ix,6);}
void ddcbF7(Z80CPU* cpu) {SETXR(cpu->ix,6,cpu->a);}
// set 7,reg,(ix+e)
void ddcbF8(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->b);}
void ddcbF9(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->c);}
void ddcbFA(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->d);}
void ddcbFB(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->e);}
void ddcbFC(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->h);}
void ddcbFD(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->l);}
void ddcbFE(Z80CPU* cpu) {SETX(cpu->ix,7);}
void ddcbFF(Z80CPU* cpu) {SETXR(cpu->ix,7,cpu->a);}

//====
// why opcode fetching doesn't eat 4T?

opCode ddcbTab[256]={
	{0,0,5,4,3,0,&ddcb00,NULL,"rlc b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb01,NULL,"rlc c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb02,NULL,"rlc d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb03,NULL,"rlc e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb04,NULL,"rlc h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb05,NULL,"rlc l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb06,NULL,"rlc (ix:5)"},
	{0,0,5,4,3,0,&ddcb07,NULL,"rlc a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb08,NULL,"rrc b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb09,NULL,"rrc c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb0A,NULL,"rrc d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb0B,NULL,"rrc e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb0C,NULL,"rrc h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb0D,NULL,"rrc l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb0E,NULL,"rrc (ix:5)"},
	{0,0,5,4,3,0,&ddcb0F,NULL,"rrc a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb10,NULL,"rl b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb11,NULL,"rl c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb12,NULL,"rl d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb13,NULL,"rl e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb14,NULL,"rl h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb15,NULL,"rl l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb16,NULL,"rl (ix:5)"},
	{0,0,5,4,3,0,&ddcb17,NULL,"rl a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb18,NULL,"rr b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb19,NULL,"rr c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb1A,NULL,"rr d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb1B,NULL,"rr e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb1C,NULL,"rr h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb1D,NULL,"rr l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb1E,NULL,"rr (ix:5)"},
	{0,0,5,4,3,0,&ddcb1F,NULL,"rr a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb20,NULL,"sla b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb21,NULL,"sla c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb22,NULL,"sla d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb23,NULL,"sla e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb24,NULL,"sla h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb25,NULL,"sla l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb26,NULL,"sla (ix:5)"},
	{0,0,5,4,3,0,&ddcb27,NULL,"sla a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb28,NULL,"sra b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb29,NULL,"sra c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb2A,NULL,"sra d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb2B,NULL,"sra e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb2C,NULL,"sra h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb2D,NULL,"sra l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb2E,NULL,"sra (ix:5)"},
	{0,0,5,4,3,0,&ddcb2F,NULL,"sra a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb30,NULL,"sll b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb31,NULL,"sll c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb32,NULL,"sll d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb33,NULL,"sll e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb34,NULL,"sll h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb35,NULL,"sll l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb36,NULL,"sll (ix:5)"},
	{0,0,5,4,3,0,&ddcb37,NULL,"sll a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb38,NULL,"srl b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb39,NULL,"srl c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb3A,NULL,"srl d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb3B,NULL,"srl e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb3C,NULL,"srl h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb3D,NULL,"srl l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb3E,NULL,"srl (ix:5)"},
	{0,0,5,4,3,0,&ddcb3F,NULL,"srl a,(ix:5)"},

	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},
	{0,0,5,4,0,0,&ddcb46,NULL,"bit 0,(ix:5)"},

	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},
	{0,0,5,4,0,0,&ddcb4E,NULL,"bit 1,(ix:5)"},

	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},
	{0,0,5,4,0,0,&ddcb56,NULL,"bit 2,(ix:5)"},

	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},
	{0,0,5,4,0,0,&ddcb5E,NULL,"bit 3,(ix:5)"},

	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},
	{0,0,5,4,0,0,&ddcb66,NULL,"bit 4,(ix:5)"},

	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},
	{0,0,5,4,0,0,&ddcb6E,NULL,"bit 5,(ix:5)"},

	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},
	{0,0,5,4,0,0,&ddcb76,NULL,"bit 6,(ix:5)"},

	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},
	{0,0,5,4,0,0,&ddcb7E,NULL,"bit 7,(ix:5)"},

	{0,0,5,4,3,0,&ddcb80,NULL,"res 0,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb81,NULL,"res 0,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb82,NULL,"res 0,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb83,NULL,"res 0,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb84,NULL,"res 0,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb85,NULL,"res 0,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb86,NULL,"res 0,(ix:5)"},
	{0,0,5,4,3,0,&ddcb87,NULL,"res 0,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb88,NULL,"res 1,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb89,NULL,"res 1,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb8A,NULL,"res 1,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb8B,NULL,"res 1,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb8C,NULL,"res 1,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb8D,NULL,"res 1,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb8E,NULL,"res 1,(ix:5)"},
	{0,0,5,4,3,0,&ddcb8F,NULL,"res 1,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb90,NULL,"res 2,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb91,NULL,"res 2,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb92,NULL,"res 2,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb93,NULL,"res 2,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb94,NULL,"res 2,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb95,NULL,"res 2,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb96,NULL,"res 2,(ix:5)"},
	{0,0,5,4,3,0,&ddcb97,NULL,"res 2,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcb98,NULL,"res 3,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcb99,NULL,"res 3,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcb9A,NULL,"res 3,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcb9B,NULL,"res 3,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcb9C,NULL,"res 3,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcb9D,NULL,"res 3,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcb9E,NULL,"res 3,(ix:5)"},
	{0,0,5,4,3,0,&ddcb9F,NULL,"res 3,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbA0,NULL,"res 4,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA1,NULL,"res 4,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA2,NULL,"res 4,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA3,NULL,"res 4,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA4,NULL,"res 4,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA5,NULL,"res 4,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA6,NULL,"res 4,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA7,NULL,"res 4,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbA8,NULL,"res 5,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbA9,NULL,"res 5,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbAA,NULL,"res 5,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbAB,NULL,"res 5,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbAC,NULL,"res 5,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbAD,NULL,"res 5,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbAE,NULL,"res 5,(ix:5)"},
	{0,0,5,4,3,0,&ddcbAF,NULL,"res 5,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbB0,NULL,"res 6,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB1,NULL,"res 6,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB2,NULL,"res 6,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB3,NULL,"res 6,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB4,NULL,"res 6,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB5,NULL,"res 6,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB6,NULL,"res 6,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB7,NULL,"res 6,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbB8,NULL,"res 7,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbB9,NULL,"res 7,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbBA,NULL,"res 7,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbBB,NULL,"res 7,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbBC,NULL,"res 7,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbBD,NULL,"res 7,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbBE,NULL,"res 7,(ix:5)"},
	{0,0,5,4,3,0,&ddcbBF,NULL,"res 7,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbC0,NULL,"set 0,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC1,NULL,"set 0,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC2,NULL,"set 0,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC3,NULL,"set 0,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC4,NULL,"set 0,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC5,NULL,"set 0,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC6,NULL,"set 0,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC7,NULL,"set 0,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbC8,NULL,"set 1,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbC9,NULL,"set 1,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbCA,NULL,"set 1,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbCB,NULL,"set 1,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbCC,NULL,"set 1,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbCD,NULL,"set 1,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbCE,NULL,"set 1,(ix:5)"},
	{0,0,5,4,3,0,&ddcbCF,NULL,"set 1,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbD0,NULL,"set 2,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD1,NULL,"set 2,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD2,NULL,"set 2,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD3,NULL,"set 2,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD4,NULL,"set 2,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD5,NULL,"set 2,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD6,NULL,"set 2,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD7,NULL,"set 2,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbD8,NULL,"set 3,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbD9,NULL,"set 3,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbDA,NULL,"set 3,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbDB,NULL,"set 3,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbDC,NULL,"set 3,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbDD,NULL,"set 3,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbDE,NULL,"set 3,(ix:5)"},
	{0,0,5,4,3,0,&ddcbDF,NULL,"set 3,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbE0,NULL,"set 4,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE1,NULL,"set 4,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE2,NULL,"set 4,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE3,NULL,"set 4,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE4,NULL,"set 4,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE5,NULL,"set 4,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE6,NULL,"set 4,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE7,NULL,"set 4,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbE8,NULL,"set 5,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbE9,NULL,"set 5,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbEA,NULL,"set 5,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbEB,NULL,"set 5,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbEC,NULL,"set 5,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbED,NULL,"set 5,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbEE,NULL,"set 5,(ix:5)"},
	{0,0,5,4,3,0,&ddcbEF,NULL,"set 5,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbF0,NULL,"set 6,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF1,NULL,"set 6,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF2,NULL,"set 6,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF3,NULL,"set 6,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF4,NULL,"set 6,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF5,NULL,"set 6,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF6,NULL,"set 6,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF7,NULL,"set 6,a,(ix:5)"},

	{0,0,5,4,3,0,&ddcbF8,NULL,"set 7,b,(ix:5)"},
	{0,0,5,4,3,0,&ddcbF9,NULL,"set 7,c,(ix:5)"},
	{0,0,5,4,3,0,&ddcbFA,NULL,"set 7,d,(ix:5)"},
	{0,0,5,4,3,0,&ddcbFB,NULL,"set 7,e,(ix:5)"},
	{0,0,5,4,3,0,&ddcbFC,NULL,"set 7,h,(ix:5)"},
	{0,0,5,4,3,0,&ddcbFD,NULL,"set 7,l,(ix:5)"},
	{0,0,5,4,3,0,&ddcbFE,NULL,"set 7,(ix:5)"},
	{0,0,5,4,3,0,&ddcbFF,NULL,"set 7,a,(ix:5)"},
};
