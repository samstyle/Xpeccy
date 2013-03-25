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

opCode fdcbTab[256]={
	{0,4,5,4,3,0,&fdcb00,"rlc b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb01,"rlc c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb02,"rlc d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb03,"rlc e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb04,"rlc h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb05,"rlc l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb06,"rlc (iy:5)"},
	{0,4,5,4,3,0,&fdcb07,"rlc a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb08,"rrc b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb09,"rrc c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb0A,"rrc d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb0B,"rrc e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb0C,"rrc h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb0D,"rrc l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb0E,"rrc (iy:5)"},
	{0,4,5,4,3,0,&fdcb0F,"rrc a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb10,"rl b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb11,"rl c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb12,"rl d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb13,"rl e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb14,"rl h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb15,"rl l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb16,"rl (iy:5)"},
	{0,4,5,4,3,0,&fdcb17,"rl a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb18,"rr b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb19,"rr c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb1A,"rr d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb1B,"rr e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb1C,"rr h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb1D,"rr l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb1E,"rr (iy:5)"},
	{0,4,5,4,3,0,&fdcb1F,"rr a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb20,"sla b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb21,"sla c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb22,"sla d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb23,"sla e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb24,"sla h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb25,"sla l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb26,"sla (iy:5)"},
	{0,4,5,4,3,0,&fdcb27,"sla a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb28,"sra b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb29,"sra c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb2A,"sra d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb2B,"sra e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb2C,"sra h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb2D,"sra l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb2E,"sra (iy:5)"},
	{0,4,5,4,3,0,&fdcb2F,"sra a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb30,"sll b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb31,"sll c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb32,"sll d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb33,"sll e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb34,"sll h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb35,"sll l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb36,"sll (iy:5)"},
	{0,4,5,4,3,0,&fdcb37,"sll a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb38,"srl b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb39,"srl c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb3A,"srl d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb3B,"srl e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb3C,"srl h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb3D,"srl l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb3E,"srl (iy:5)"},
	{0,4,5,4,3,0,&fdcb3F,"srl a,(iy:5)"},

	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},
	{0,4,5,4,0,0,&fdcb46,"bit 0,(iy:5)"},

	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},
	{0,4,5,4,0,0,&fdcb4E,"bit 1,(iy:5)"},

	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},
	{0,4,5,4,0,0,&fdcb56,"bit 2,(iy:5)"},

	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},
	{0,4,5,4,0,0,&fdcb5E,"bit 3,(iy:5)"},

	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},
	{0,4,5,4,0,0,&fdcb66,"bit 4,(iy:5)"},

	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},
	{0,4,5,4,0,0,&fdcb6E,"bit 5,(iy:5)"},

	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},
	{0,4,5,4,0,0,&fdcb76,"bit 6,(iy:5)"},

	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},
	{0,4,5,4,0,0,&fdcb7E,"bit 7,(iy:5)"},

	{0,4,5,4,3,0,&fdcb80,"res 0,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb81,"res 0,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb82,"res 0,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb83,"res 0,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb84,"res 0,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb85,"res 0,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb86,"res 0,(iy:5)"},
	{0,4,5,4,3,0,&fdcb87,"res 0,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb88,"res 1,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb89,"res 1,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb8A,"res 1,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb8B,"res 1,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb8C,"res 1,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb8D,"res 1,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb8E,"res 1,(iy:5)"},
	{0,4,5,4,3,0,&fdcb8F,"res 1,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb90,"res 2,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb91,"res 2,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb92,"res 2,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb93,"res 2,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb94,"res 2,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb95,"res 2,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb96,"res 2,(iy:5)"},
	{0,4,5,4,3,0,&fdcb97,"res 2,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcb98,"res 3,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcb99,"res 3,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcb9A,"res 3,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcb9B,"res 3,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcb9C,"res 3,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcb9D,"res 3,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcb9E,"res 3,(iy:5)"},
	{0,4,5,4,3,0,&fdcb9F,"res 3,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbA0,"res 4,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA1,"res 4,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA2,"res 4,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA3,"res 4,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA4,"res 4,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA5,"res 4,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA6,"res 4,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA7,"res 4,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbA8,"res 5,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbA9,"res 5,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbAA,"res 5,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbAB,"res 5,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbAC,"res 5,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbAD,"res 5,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbAE,"res 5,(iy:5)"},
	{0,4,5,4,3,0,&fdcbAF,"res 5,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbB0,"res 6,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB1,"res 6,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB2,"res 6,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB3,"res 6,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB4,"res 6,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB5,"res 6,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB6,"res 6,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB7,"res 6,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbB8,"res 7,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbB9,"res 7,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbBA,"res 7,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbBB,"res 7,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbBC,"res 7,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbBD,"res 7,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbBE,"res 7,(iy:5)"},
	{0,4,5,4,3,0,&fdcbBF,"res 7,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbC0,"set 0,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC1,"set 0,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC2,"set 0,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC3,"set 0,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC4,"set 0,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC5,"set 0,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC6,"set 0,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC7,"set 0,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbC8,"set 1,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbC9,"set 1,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbCA,"set 1,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbCB,"set 1,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbCC,"set 1,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbCD,"set 1,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbCE,"set 1,(iy:5)"},
	{0,4,5,4,3,0,&fdcbCF,"set 1,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbD0,"set 2,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD1,"set 2,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD2,"set 2,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD3,"set 2,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD4,"set 2,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD5,"set 2,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD6,"set 2,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD7,"set 2,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbD8,"set 3,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbD9,"set 3,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbDA,"set 3,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbDB,"set 3,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbDC,"set 3,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbDD,"set 3,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbDE,"set 3,(iy:5)"},
	{0,4,5,4,3,0,&fdcbDF,"set 3,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbE0,"set 4,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE1,"set 4,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE2,"set 4,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE3,"set 4,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE4,"set 4,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE5,"set 4,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE6,"set 4,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE7,"set 4,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbE8,"set 5,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbE9,"set 5,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbEA,"set 5,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbEB,"set 5,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbEC,"set 5,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbED,"set 5,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbEE,"set 5,(iy:5)"},
	{0,4,5,4,3,0,&fdcbEF,"set 5,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbF0,"set 6,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF1,"set 6,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF2,"set 6,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF3,"set 6,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF4,"set 6,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF5,"set 6,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF6,"set 6,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF7,"set 6,a,(iy:5)"},

	{0,4,5,4,3,0,&fdcbF8,"set 7,b,(iy:5)"},
	{0,4,5,4,3,0,&fdcbF9,"set 7,c,(iy:5)"},
	{0,4,5,4,3,0,&fdcbFA,"set 7,d,(iy:5)"},
	{0,4,5,4,3,0,&fdcbFB,"set 7,e,(iy:5)"},
	{0,4,5,4,3,0,&fdcbFC,"set 7,h,(iy:5)"},
	{0,4,5,4,3,0,&fdcbFD,"set 7,l,(iy:5)"},
	{0,4,5,4,3,0,&fdcbFE,"set 7,(iy:5)"},
	{0,4,5,4,3,0,&fdcbFF,"set 7,a,(iy:5)"},
};
