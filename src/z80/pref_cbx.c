void cxp00(Spec *p) {p->cpu->b = xRLC(p,p->cpu->ix);}
void cxp01(Spec *p) {p->cpu->c = xRLC(p,p->cpu->ix);}
void cxp02(Spec *p) {p->cpu->d = xRLC(p,p->cpu->ix);}
void cxp03(Spec *p) {p->cpu->e = xRLC(p,p->cpu->ix);}
void cxp04(Spec *p) {p->cpu->h = xRLC(p,p->cpu->ix);}
void cxp05(Spec *p) {p->cpu->l = xRLC(p,p->cpu->ix);}
void cxp06(Spec *p) {xRLC(p,p->cpu->ix);}
void cxp07(Spec *p) {p->cpu->a = xRLC(p,p->cpu->ix);}

void cxp08(Spec *p) {p->cpu->b = xRRC(p,p->cpu->ix);}
void cxp09(Spec *p) {p->cpu->c = xRRC(p,p->cpu->ix);}
void cxp0A(Spec *p) {p->cpu->d = xRRC(p,p->cpu->ix);}
void cxp0B(Spec *p) {p->cpu->e = xRRC(p,p->cpu->ix);}
void cxp0C(Spec *p) {p->cpu->h = xRRC(p,p->cpu->ix);}
void cxp0D(Spec *p) {p->cpu->l = xRRC(p,p->cpu->ix);}
void cxp0E(Spec *p) {xRRC(p,p->cpu->ix);}
void cxp0F(Spec *p) {p->cpu->a = xRRC(p,p->cpu->ix);}

void cxp10(Spec *p) {p->cpu->b = xRL(p,p->cpu->ix);}
void cxp11(Spec *p) {p->cpu->c = xRL(p,p->cpu->ix);}
void cxp12(Spec *p) {p->cpu->d = xRL(p,p->cpu->ix);}
void cxp13(Spec *p) {p->cpu->e = xRL(p,p->cpu->ix);}
void cxp14(Spec *p) {p->cpu->h = xRL(p,p->cpu->ix);}
void cxp15(Spec *p) {p->cpu->l = xRL(p,p->cpu->ix);}
void cxp16(Spec *p) {xRL(p,p->cpu->ix);}
void cxp17(Spec *p) {p->cpu->a = xRL(p,p->cpu->ix);}

void cxp18(Spec *p) {p->cpu->b = xRR(p,p->cpu->ix);}
void cxp19(Spec *p) {p->cpu->c = xRR(p,p->cpu->ix);}
void cxp1A(Spec *p) {p->cpu->d = xRR(p,p->cpu->ix);}
void cxp1B(Spec *p) {p->cpu->e = xRR(p,p->cpu->ix);}
void cxp1C(Spec *p) {p->cpu->h = xRR(p,p->cpu->ix);}
void cxp1D(Spec *p) {p->cpu->l = xRR(p,p->cpu->ix);}
void cxp1E(Spec *p) {xRR(p,p->cpu->ix);}
void cxp1F(Spec *p) {p->cpu->a = xRR(p,p->cpu->ix);}

void cxp20(Spec *p) {p->cpu->b = xSLA(p,p->cpu->ix);}
void cxp21(Spec *p) {p->cpu->c = xSLA(p,p->cpu->ix);}
void cxp22(Spec *p) {p->cpu->d = xSLA(p,p->cpu->ix);}
void cxp23(Spec *p) {p->cpu->e = xSLA(p,p->cpu->ix);}
void cxp24(Spec *p) {p->cpu->h = xSLA(p,p->cpu->ix);}
void cxp25(Spec *p) {p->cpu->l = xSLA(p,p->cpu->ix);}
void cxp26(Spec *p) {xSLA(p,p->cpu->ix);}
void cxp27(Spec *p) {p->cpu->a = xSLA(p,p->cpu->ix);}

void cxp28(Spec *p) {p->cpu->b = xSRA(p,p->cpu->ix);}
void cxp29(Spec *p) {p->cpu->c = xSRA(p,p->cpu->ix);}
void cxp2A(Spec *p) {p->cpu->d = xSRA(p,p->cpu->ix);}
void cxp2B(Spec *p) {p->cpu->e = xSRA(p,p->cpu->ix);}
void cxp2C(Spec *p) {p->cpu->h = xSRA(p,p->cpu->ix);}
void cxp2D(Spec *p) {p->cpu->l = xSRA(p,p->cpu->ix);}
void cxp2E(Spec *p) {xSRA(p,p->cpu->ix);}
void cxp2F(Spec *p) {p->cpu->a = xSRA(p,p->cpu->ix);}

void cxp30(Spec *p) {p->cpu->b = xSLI(p,p->cpu->ix);}
void cxp31(Spec *p) {p->cpu->c = xSLI(p,p->cpu->ix);}
void cxp32(Spec *p) {p->cpu->d = xSLI(p,p->cpu->ix);}
void cxp33(Spec *p) {p->cpu->e = xSLI(p,p->cpu->ix);}
void cxp34(Spec *p) {p->cpu->h = xSLI(p,p->cpu->ix);}
void cxp35(Spec *p) {p->cpu->l = xSLI(p,p->cpu->ix);}
void cxp36(Spec *p) {xSLI(p,p->cpu->ix);}
void cxp37(Spec *p) {p->cpu->a = xSLI(p,p->cpu->ix);}

void cxp38(Spec *p) {p->cpu->b = xSRL(p,p->cpu->ix);}
void cxp39(Spec *p) {p->cpu->c = xSRL(p,p->cpu->ix);}
void cxp3A(Spec *p) {p->cpu->d = xSRL(p,p->cpu->ix);}
void cxp3B(Spec *p) {p->cpu->e = xSRL(p,p->cpu->ix);}
void cxp3C(Spec *p) {p->cpu->h = xSRL(p,p->cpu->ix);}
void cxp3D(Spec *p) {p->cpu->l = xSRL(p,p->cpu->ix);}
void cxp3E(Spec *p) {xSRL(p,p->cpu->ix);}
void cxp3F(Spec *p) {p->cpu->a = xSRL(p,p->cpu->ix);}
// bit n,(ix+d)	flag b3,5 = mptr.hi b3,5
void cxp46(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[0]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp4E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[1]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp56(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[2]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp5E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[3]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp66(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[4]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp6E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[5]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp76(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[6]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp7E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[7]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}

void cxp80(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x01);}
void cxp81(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x01);}
void cxp82(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x01);}
void cxp83(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x01);}
void cxp84(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x01);}
void cxp85(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x01);}
void cxp86(Spec *p) {xRES(p,p->cpu->ix,0x01);}
void cxp87(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x01);}

void cxp88(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x02);}
void cxp89(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x02);}
void cxp8A(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x02);}
void cxp8B(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x02);}
void cxp8C(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x02);}
void cxp8D(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x02);}
void cxp8E(Spec *p) {xRES(p,p->cpu->ix,0x02);}
void cxp8F(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x02);}

void cxp90(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x04);}
void cxp91(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x04);}
void cxp92(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x04);}
void cxp93(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x04);}
void cxp94(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x04);}
void cxp95(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x04);}
void cxp96(Spec *p) {xRES(p,p->cpu->ix,0x04);}
void cxp97(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x04);}

void cxp98(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x08);}
void cxp99(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x08);}
void cxp9A(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x08);}
void cxp9B(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x08);}
void cxp9C(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x08);}
void cxp9D(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x08);}
void cxp9E(Spec *p) {xRES(p,p->cpu->ix,0x08);}
void cxp9F(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x08);}

void cxpA0(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x10);}
void cxpA1(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x10);}
void cxpA2(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x10);}
void cxpA3(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x10);}
void cxpA4(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x10);}
void cxpA5(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x10);}
void cxpA6(Spec *p) {xRES(p,p->cpu->ix,0x10);}
void cxpA7(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x10);}

void cxpA8(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x20);}
void cxpA9(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x20);}
void cxpAA(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x20);}
void cxpAB(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x20);}
void cxpAC(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x20);}
void cxpAD(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x20);}
void cxpAE(Spec *p) {xRES(p,p->cpu->ix,0x20);}
void cxpAF(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x20);}

void cxpB0(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x40);}
void cxpB1(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x40);}
void cxpB2(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x40);}
void cxpB3(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x40);}
void cxpB4(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x40);}
void cxpB5(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x40);}
void cxpB6(Spec *p) {xRES(p,p->cpu->ix,0x40);}
void cxpB7(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x40);}

void cxpB8(Spec *p) {p->cpu->b = xRES(p,p->cpu->ix,0x80);}
void cxpB9(Spec *p) {p->cpu->c = xRES(p,p->cpu->ix,0x80);}
void cxpBA(Spec *p) {p->cpu->d = xRES(p,p->cpu->ix,0x80);}
void cxpBB(Spec *p) {p->cpu->e = xRES(p,p->cpu->ix,0x80);}
void cxpBC(Spec *p) {p->cpu->h = xRES(p,p->cpu->ix,0x80);}
void cxpBD(Spec *p) {p->cpu->l = xRES(p,p->cpu->ix,0x80);}
void cxpBE(Spec *p) {xRES(p,p->cpu->ix,0x80);}
void cxpBF(Spec *p) {p->cpu->a = xRES(p,p->cpu->ix,0x80);}

void cxpC0(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x01);}
void cxpC1(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x01);}
void cxpC2(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x01);}
void cxpC3(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x01);}
void cxpC4(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x01);}
void cxpC5(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x01);}
void cxpC6(Spec *p) {xSET(p,p->cpu->ix,0x01);}
void cxpC7(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x01);}

void cxpC8(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x02);}
void cxpC9(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x02);}
void cxpCA(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x02);}
void cxpCB(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x02);}
void cxpCC(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x02);}
void cxpCD(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x02);}
void cxpCE(Spec *p) {xSET(p,p->cpu->ix,0x02);}
void cxpCF(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x02);}

void cxpD0(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x04);}
void cxpD1(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x04);}
void cxpD2(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x04);}
void cxpD3(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x04);}
void cxpD4(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x04);}
void cxpD5(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x04);}
void cxpD6(Spec *p) {xSET(p,p->cpu->ix,0x04);}
void cxpD7(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x04);}

void cxpD8(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x08);}
void cxpD9(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x08);}
void cxpDA(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x08);}
void cxpDB(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x08);}
void cxpDC(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x08);}
void cxpDD(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x08);}
void cxpDE(Spec *p) {xSET(p,p->cpu->ix,0x08);}
void cxpDF(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x08);}

void cxpE0(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x10);}
void cxpE1(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x10);}
void cxpE2(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x10);}
void cxpE3(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x10);}
void cxpE4(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x10);}
void cxpE5(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x10);}
void cxpE6(Spec *p) {xSET(p,p->cpu->ix,0x10);}
void cxpE7(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x10);}

void cxpE8(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x20);}
void cxpE9(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x20);}
void cxpEA(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x20);}
void cxpEB(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x20);}
void cxpEC(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x20);}
void cxpED(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x20);}
void cxpEE(Spec *p) {xSET(p,p->cpu->ix,0x20);}
void cxpEF(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x20);}

void cxpF0(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x40);}
void cxpF1(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x40);}
void cxpF2(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x40);}
void cxpF3(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x40);}
void cxpF4(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x40);}
void cxpF5(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x40);}
void cxpF6(Spec *p) {xSET(p,p->cpu->ix,0x40);}
void cxpF7(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x40);}

void cxpF8(Spec *p) {p->cpu->b = xSET(p,p->cpu->ix,0x80);}
void cxpF9(Spec *p) {p->cpu->c = xSET(p,p->cpu->ix,0x80);}
void cxpFA(Spec *p) {p->cpu->d = xSET(p,p->cpu->ix,0x80);}
void cxpFB(Spec *p) {p->cpu->e = xSET(p,p->cpu->ix,0x80);}
void cxpFC(Spec *p) {p->cpu->h = xSET(p,p->cpu->ix,0x80);}
void cxpFD(Spec *p) {p->cpu->l = xSET(p,p->cpu->ix,0x80);}
void cxpFE(Spec *p) {xSET(p,p->cpu->ix,0x80);}
void cxpFF(Spec *p) {p->cpu->a = xSET(p,p->cpu->ix,0x80);}

//==================

ZOp cxpref[256]={
	ZOp(&cxp00,15,"rlc b,(ix:5)"),
	ZOp(&cxp01,15,"rlc c,(ix:5)"),
	ZOp(&cxp02,15,"rlc d,(ix:5)"),
	ZOp(&cxp03,15,"rlc e,(ix:5)"),
	ZOp(&cxp04,15,"rlc h,(ix:5)"),
	ZOp(&cxp05,15,"rlc l,(ix:5)"),
	ZOp(&cxp06,15,"rlc (ix:5)"),
	ZOp(&cxp07,15,"rlc a,(ix:5)"),

	ZOp(&cxp08,15,"rrc b,(ix:5)"),
	ZOp(&cxp09,15,"rrc c,(ix:5)"),
	ZOp(&cxp0A,15,"rrc d,(ix:5)"),
	ZOp(&cxp0B,15,"rrc e,(ix:5)"),
	ZOp(&cxp0C,15,"rrc h,(ix:5)"),
	ZOp(&cxp0D,15,"rrc l,(ix:5)"),
	ZOp(&cxp0E,15,"rrc (ix:5)"),
	ZOp(&cxp0F,15,"rrc a,(ix:5)"),

	ZOp(&cxp10,15,"rl b,(ix:5)"),
	ZOp(&cxp11,15,"rl c,(ix:5)"),
	ZOp(&cxp12,15,"rl d,(ix:5)"),
	ZOp(&cxp13,15,"rl e,(ix:5)"),
	ZOp(&cxp14,15,"rl h,(ix:5)"),
	ZOp(&cxp15,15,"rl l,(ix:5)"),
	ZOp(&cxp16,15,"rl (ix:5)"),
	ZOp(&cxp17,15,"rl a,(ix:5)"),

	ZOp(&cxp18,15,"rr b,(ix:5)"),
	ZOp(&cxp19,15,"rr c,(ix:5)"),
	ZOp(&cxp1A,15,"rr d,(ix:5)"),
	ZOp(&cxp1B,15,"rr e,(ix:5)"),
	ZOp(&cxp1C,15,"rr h,(ix:5)"),
	ZOp(&cxp1D,15,"rr l,(ix:5)"),
	ZOp(&cxp1E,15,"rr (ix:5)"),
	ZOp(&cxp1F,15,"rr a,(ix:5)"),

	ZOp(&cxp20,15,"sla b,(ix:5)"),
	ZOp(&cxp21,15,"sla c,(ix:5)"),
	ZOp(&cxp22,15,"sla d,(ix:5)"),
	ZOp(&cxp23,15,"sla e,(ix:5)"),
	ZOp(&cxp24,15,"sla h,(ix:5)"),
	ZOp(&cxp25,15,"sla l,(ix:5)"),
	ZOp(&cxp26,15,"sla (ix:5)"),
	ZOp(&cxp27,15,"sla a,(ix:5)"),

	ZOp(&cxp28,15,"sra b,(ix:5)"),
	ZOp(&cxp29,15,"sra c,(ix:5)"),
	ZOp(&cxp2A,15,"sra d,(ix:5)"),
	ZOp(&cxp2B,15,"sra e,(ix:5)"),
	ZOp(&cxp2C,15,"sra h,(ix:5)"),
	ZOp(&cxp2D,15,"sra l,(ix:5)"),
	ZOp(&cxp2E,15,"sra (ix:5)"),
	ZOp(&cxp2F,15,"sra a,(ix:5)"),

	ZOp(&cxp30,15,"sli b,(ix:5)"),
	ZOp(&cxp31,15,"sli c,(ix:5)"),
	ZOp(&cxp32,15,"sli d,(ix:5)"),
	ZOp(&cxp33,15,"sli e,(ix:5)"),
	ZOp(&cxp34,15,"sli h,(ix:5)"),
	ZOp(&cxp35,15,"sli l,(ix:5)"),
	ZOp(&cxp36,15,"sli (ix:5)"),
	ZOp(&cxp37,15,"sli a,(ix:5)"),

	ZOp(&cxp38,15,"srl b,(ix:5)"),
	ZOp(&cxp39,15,"srl c,(ix:5)"),
	ZOp(&cxp3A,15,"srl d,(ix:5)"),
	ZOp(&cxp3B,15,"srl e,(ix:5)"),
	ZOp(&cxp3C,15,"srl h,(ix:5)"),
	ZOp(&cxp3D,15,"srl l,(ix:5)"),
	ZOp(&cxp3E,15,"srl (ix:5)"),
	ZOp(&cxp3F,15,"srl a,(ix:5)"),

	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),
	ZOp(&cxp46,12,"bit 0,(ix:5)"),

	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),
	ZOp(&cxp4E,12,"bit 1,(ix:5)"),

	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),
	ZOp(&cxp56,12,"bit 2,(ix:5)"),

	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),
	ZOp(&cxp5E,12,"bit 3,(ix:5)"),

	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),
	ZOp(&cxp66,12,"bit 4,(ix:5)"),

	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),
	ZOp(&cxp6E,12,"bit 5,(ix:5)"),

	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),
	ZOp(&cxp76,12,"bit 6,(ix:5)"),

	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),
	ZOp(&cxp7E,12,"bit 7,(ix:5)"),

	ZOp(&cxp80,15,"res 0,b,(ix:5)"),
	ZOp(&cxp81,15,"res 0,c,(ix:5)"),
	ZOp(&cxp82,15,"res 0,d,(ix:5)"),
	ZOp(&cxp83,15,"res 0,e,(ix:5)"),
	ZOp(&cxp84,15,"res 0,h,(ix:5)"),
	ZOp(&cxp85,15,"res 0,b,(ix:5)"),
	ZOp(&cxp86,15,"res 0,(ix:5)"),
	ZOp(&cxp87,15,"res 0,a,(ix:5)"),

	ZOp(&cxp88,15,"res 1,b,(ix:5)"),
	ZOp(&cxp89,15,"res 1,c,(ix:5)"),
	ZOp(&cxp8A,15,"res 1,d,(ix:5)"),
	ZOp(&cxp8B,15,"res 1,e,(ix:5)"),
	ZOp(&cxp8C,15,"res 1,h,(ix:5)"),
	ZOp(&cxp8D,15,"res 1,l,(ix:5)"),
	ZOp(&cxp8E,15,"res 1,(ix:5)"),
	ZOp(&cxp8F,15,"res 1,a,(ix:5)"),

	ZOp(&cxp90,15,"res 2,b,(ix:5)"),
	ZOp(&cxp91,15,"res 2,c,(ix:5)"),
	ZOp(&cxp92,15,"res 2,d,(ix:5)"),
	ZOp(&cxp93,15,"res 2,e,(ix:5)"),
	ZOp(&cxp94,15,"res 2,h,(ix:5)"),
	ZOp(&cxp95,15,"res 2,l,(ix:5)"),
	ZOp(&cxp96,15,"res 2,(ix:5)"),
	ZOp(&cxp97,15,"res 2,a,(ix:5)"),

	ZOp(&cxp98,15,"res 3,b,(ix:5)"),
	ZOp(&cxp99,15,"res 3,c,(ix:5)"),
	ZOp(&cxp9A,15,"res 3,d,(ix:5)"),
	ZOp(&cxp9B,15,"res 3,e,(ix:5)"),
	ZOp(&cxp9C,15,"res 3,h,(ix:5)"),
	ZOp(&cxp9D,15,"res 3,l,(ix:5)"),
	ZOp(&cxp9E,15,"res 3,(ix:5)"),
	ZOp(&cxp9F,15,"res 3,a,(ix:5)"),

	ZOp(&cxpA0,15,"res 4,b,(ix:5)"),
	ZOp(&cxpA1,15,"res 4,c,(ix:5)"),
	ZOp(&cxpA2,15,"res 4,d,(ix:5)"),
	ZOp(&cxpA3,15,"res 4,e,(ix:5)"),
	ZOp(&cxpA4,15,"res 4,h,(ix:5)"),
	ZOp(&cxpA5,15,"res 4,l,(ix:5)"),
	ZOp(&cxpA6,15,"res 4,(ix:5)"),
	ZOp(&cxpA7,15,"res 4,a,(ix:5)"),

	ZOp(&cxpA8,15,"res 5,b,(ix:5)"),
	ZOp(&cxpA9,15,"res 5,c,(ix:5)"),
	ZOp(&cxpAA,15,"res 5,d,(ix:5)"),
	ZOp(&cxpAB,15,"res 5,e,(ix:5)"),
	ZOp(&cxpAC,15,"res 5,h,(ix:5)"),
	ZOp(&cxpAD,15,"res 5,l,(ix:5)"),
	ZOp(&cxpAE,15,"res 5,(ix:5)"),
	ZOp(&cxpAF,15,"res 5,a,(ix:5)"),

	ZOp(&cxpB0,15,"res 6,b,(ix:5)"),
	ZOp(&cxpB1,15,"res 6,c,(ix:5)"),
	ZOp(&cxpB2,15,"res 6,d,(ix:5)"),
	ZOp(&cxpB3,15,"res 6,e,(ix:5)"),
	ZOp(&cxpB4,15,"res 6,h,(ix:5)"),
	ZOp(&cxpB5,15,"res 6,l,(ix:5)"),
	ZOp(&cxpB6,15,"res 6,(ix:5)"),
	ZOp(&cxpB7,15,"res 6,a,(ix:5)"),

	ZOp(&cxpB8,15,"res 7,b,(ix:5)"),
	ZOp(&cxpB9,15,"res 7,c,(ix:5)"),
	ZOp(&cxpBA,15,"res 7,d,(ix:5)"),
	ZOp(&cxpBB,15,"res 7,e,(ix:5)"),
	ZOp(&cxpBC,15,"res 7,h,(ix:5)"),
	ZOp(&cxpBD,15,"res 7,l,(ix:5)"),
	ZOp(&cxpBE,15,"res 7,(ix:5)"),
	ZOp(&cxpBF,15,"res 7,a,(ix:5)"),

	ZOp(&cxpC0,15,"set 0,b,(ix:5)"),
	ZOp(&cxpC1,15,"set 0,c,(ix:5)"),
	ZOp(&cxpC2,15,"set 0,d,(ix:5)"),
	ZOp(&cxpC3,15,"set 0,e,(ix:5)"),
	ZOp(&cxpC4,15,"set 0,h,(ix:5)"),
	ZOp(&cxpC5,15,"set 0,l,(ix:5)"),
	ZOp(&cxpC6,15,"set 0,(ix:5)"),
	ZOp(&cxpC7,15,"set 0,a,(ix:5)"),

	ZOp(&cxpC8,15,"set 1,b,(ix:5)"),
	ZOp(&cxpC9,15,"set 1,c,(ix:5)"),
	ZOp(&cxpCA,15,"set 1,d,(ix:5)"),
	ZOp(&cxpCB,15,"set 1,e,(ix:5)"),
	ZOp(&cxpCC,15,"set 1,h,(ix:5)"),
	ZOp(&cxpCD,15,"set 1,l,(ix:5)"),
	ZOp(&cxpCE,15,"set 1,(ix:5)"),
	ZOp(&cxpCF,15,"set 1,a,(ix:5)"),

	ZOp(&cxpD0,15,"set 2,b,(ix:5)"),
	ZOp(&cxpD1,15,"set 2,c,(ix:5)"),
	ZOp(&cxpD2,15,"set 2,d,(ix:5)"),
	ZOp(&cxpD3,15,"set 2,e,(ix:5)"),
	ZOp(&cxpD4,15,"set 2,h,(ix:5)"),
	ZOp(&cxpD5,15,"set 2,l,(ix:5)"),
	ZOp(&cxpD6,15,"set 2,(ix:5)"),
	ZOp(&cxpD7,15,"set 2,a,(ix:5)"),

	ZOp(&cxpD8,15,"set 3,b,(ix:5)"),
	ZOp(&cxpD9,15,"set 3,c,(ix:5)"),
	ZOp(&cxpDA,15,"set 3,d,(ix:5)"),
	ZOp(&cxpDB,15,"set 3,e,(ix:5)"),
	ZOp(&cxpDC,15,"set 3,h,(ix:5)"),
	ZOp(&cxpDD,15,"set 3,l,(ix:5)"),
	ZOp(&cxpDE,15,"set 3,(ix:5)"),
	ZOp(&cxpDF,15,"set 3,a,(ix:5)"),

	ZOp(&cxpE0,15,"set 4,b,(ix:5)"),
	ZOp(&cxpE1,15,"set 4,c,(ix:5)"),
	ZOp(&cxpE2,15,"set 4,d,(ix:5)"),
	ZOp(&cxpE3,15,"set 4,e,(ix:5)"),
	ZOp(&cxpE4,15,"set 4,h,(ix:5)"),
	ZOp(&cxpE5,15,"set 4,l,(ix:5)"),
	ZOp(&cxpE6,15,"set 4,(ix:5)"),
	ZOp(&cxpE7,15,"set 4,a,(ix:5)"),

	ZOp(&cxpE8,15,"set 5,b,(ix:5)"),
	ZOp(&cxpE9,15,"set 5,c,(ix:5)"),
	ZOp(&cxpEA,15,"set 5,d,(ix:5)"),
	ZOp(&cxpEB,15,"set 5,e,(ix:5)"),
	ZOp(&cxpEC,15,"set 5,h,(ix:5)"),
	ZOp(&cxpED,15,"set 5,l,(ix:5)"),
	ZOp(&cxpEE,15,"set 5,(ix:5)"),
	ZOp(&cxpEF,15,"set 5,a,(ix:5)"),

	ZOp(&cxpF0,15,"set 6,b,(ix:5)"),
	ZOp(&cxpF1,15,"set 6,c,(ix:5)"),
	ZOp(&cxpF2,15,"set 6,d,(ix:5)"),
	ZOp(&cxpF3,15,"set 6,e,(ix:5)"),
	ZOp(&cxpF4,15,"set 6,h,(ix:5)"),
	ZOp(&cxpF5,15,"set 6,l,(ix:5)"),
	ZOp(&cxpF6,15,"set 6,(ix:5)"),
	ZOp(&cxpF7,15,"set 6,a,(ix:5)"),

	ZOp(&cxpF8,15,"set 7,b,(ix:5)"),
	ZOp(&cxpF9,15,"set 7,c,(ix:5)"),
	ZOp(&cxpFA,15,"set 7,d,(ix:5)"),
	ZOp(&cxpFB,15,"set 7,e,(ix:5)"),
	ZOp(&cxpFC,15,"set 7,h,(ix:5)"),
	ZOp(&cxpFD,15,"set 7,l,(ix:5)"),
	ZOp(&cxpFE,15,"set 7,(ix:5)"),
	ZOp(&cxpFF,15,"set 7,a,(ix:5)"),
};

