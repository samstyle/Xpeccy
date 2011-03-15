void cxp00(ZXBase* p) {p->cpu->b = xRLC(p,p->cpu->ix);}
void cxp01(ZXBase* p) {p->cpu->c = xRLC(p,p->cpu->ix);}
void cxp02(ZXBase* p) {p->cpu->d = xRLC(p,p->cpu->ix);}
void cxp03(ZXBase* p) {p->cpu->e = xRLC(p,p->cpu->ix);}
void cxp04(ZXBase* p) {p->cpu->h = xRLC(p,p->cpu->ix);}
void cxp05(ZXBase* p) {p->cpu->l = xRLC(p,p->cpu->ix);}
void cxp06(ZXBase* p) {xRLC(p,p->cpu->ix);}
void cxp07(ZXBase* p) {p->cpu->a = xRLC(p,p->cpu->ix);}

void cxp08(ZXBase* p) {p->cpu->b = xRRC(p,p->cpu->ix);}
void cxp09(ZXBase* p) {p->cpu->c = xRRC(p,p->cpu->ix);}
void cxp0A(ZXBase* p) {p->cpu->d = xRRC(p,p->cpu->ix);}
void cxp0B(ZXBase* p) {p->cpu->e = xRRC(p,p->cpu->ix);}
void cxp0C(ZXBase* p) {p->cpu->h = xRRC(p,p->cpu->ix);}
void cxp0D(ZXBase* p) {p->cpu->l = xRRC(p,p->cpu->ix);}
void cxp0E(ZXBase* p) {xRRC(p,p->cpu->ix);}
void cxp0F(ZXBase* p) {p->cpu->a = xRRC(p,p->cpu->ix);}

void cxp10(ZXBase* p) {p->cpu->b = xRL(p,p->cpu->ix);}
void cxp11(ZXBase* p) {p->cpu->c = xRL(p,p->cpu->ix);}
void cxp12(ZXBase* p) {p->cpu->d = xRL(p,p->cpu->ix);}
void cxp13(ZXBase* p) {p->cpu->e = xRL(p,p->cpu->ix);}
void cxp14(ZXBase* p) {p->cpu->h = xRL(p,p->cpu->ix);}
void cxp15(ZXBase* p) {p->cpu->l = xRL(p,p->cpu->ix);}
void cxp16(ZXBase* p) {xRL(p,p->cpu->ix);}
void cxp17(ZXBase* p) {p->cpu->a = xRL(p,p->cpu->ix);}

void cxp18(ZXBase* p) {p->cpu->b = xRR(p,p->cpu->ix);}
void cxp19(ZXBase* p) {p->cpu->c = xRR(p,p->cpu->ix);}
void cxp1A(ZXBase* p) {p->cpu->d = xRR(p,p->cpu->ix);}
void cxp1B(ZXBase* p) {p->cpu->e = xRR(p,p->cpu->ix);}
void cxp1C(ZXBase* p) {p->cpu->h = xRR(p,p->cpu->ix);}
void cxp1D(ZXBase* p) {p->cpu->l = xRR(p,p->cpu->ix);}
void cxp1E(ZXBase* p) {xRR(p,p->cpu->ix);}
void cxp1F(ZXBase* p) {p->cpu->a = xRR(p,p->cpu->ix);}

void cxp20(ZXBase* p) {p->cpu->b = xSLA(p,p->cpu->ix);}
void cxp21(ZXBase* p) {p->cpu->c = xSLA(p,p->cpu->ix);}
void cxp22(ZXBase* p) {p->cpu->d = xSLA(p,p->cpu->ix);}
void cxp23(ZXBase* p) {p->cpu->e = xSLA(p,p->cpu->ix);}
void cxp24(ZXBase* p) {p->cpu->h = xSLA(p,p->cpu->ix);}
void cxp25(ZXBase* p) {p->cpu->l = xSLA(p,p->cpu->ix);}
void cxp26(ZXBase* p) {xSLA(p,p->cpu->ix);}
void cxp27(ZXBase* p) {p->cpu->a = xSLA(p,p->cpu->ix);}

void cxp28(ZXBase* p) {p->cpu->b = xSRA(p,p->cpu->ix);}
void cxp29(ZXBase* p) {p->cpu->c = xSRA(p,p->cpu->ix);}
void cxp2A(ZXBase* p) {p->cpu->d = xSRA(p,p->cpu->ix);}
void cxp2B(ZXBase* p) {p->cpu->e = xSRA(p,p->cpu->ix);}
void cxp2C(ZXBase* p) {p->cpu->h = xSRA(p,p->cpu->ix);}
void cxp2D(ZXBase* p) {p->cpu->l = xSRA(p,p->cpu->ix);}
void cxp2E(ZXBase* p) {xSRA(p,p->cpu->ix);}
void cxp2F(ZXBase* p) {p->cpu->a = xSRA(p,p->cpu->ix);}

void cxp30(ZXBase* p) {p->cpu->b = xSLI(p,p->cpu->ix);}
void cxp31(ZXBase* p) {p->cpu->c = xSLI(p,p->cpu->ix);}
void cxp32(ZXBase* p) {p->cpu->d = xSLI(p,p->cpu->ix);}
void cxp33(ZXBase* p) {p->cpu->e = xSLI(p,p->cpu->ix);}
void cxp34(ZXBase* p) {p->cpu->h = xSLI(p,p->cpu->ix);}
void cxp35(ZXBase* p) {p->cpu->l = xSLI(p,p->cpu->ix);}
void cxp36(ZXBase* p) {xSLI(p,p->cpu->ix);}
void cxp37(ZXBase* p) {p->cpu->a = xSLI(p,p->cpu->ix);}

void cxp38(ZXBase* p) {p->cpu->b = xSRL(p,p->cpu->ix);}
void cxp39(ZXBase* p) {p->cpu->c = xSRL(p,p->cpu->ix);}
void cxp3A(ZXBase* p) {p->cpu->d = xSRL(p,p->cpu->ix);}
void cxp3B(ZXBase* p) {p->cpu->e = xSRL(p,p->cpu->ix);}
void cxp3C(ZXBase* p) {p->cpu->h = xSRL(p,p->cpu->ix);}
void cxp3D(ZXBase* p) {p->cpu->l = xSRL(p,p->cpu->ix);}
void cxp3E(ZXBase* p) {xSRL(p,p->cpu->ix);}
void cxp3F(ZXBase* p) {p->cpu->a = xSRL(p,p->cpu->ix);}
// bit n,(ix+d)	flag b3,5 = mptr.hi b3,5
void cxp46(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[0]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp4E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[1]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp56(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[2]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp5E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[3]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp66(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[4]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp6E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[5]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp76(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[6]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cxp7E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[7]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}

void cxp80(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x01);}
void cxp81(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x01);}
void cxp82(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x01);}
void cxp83(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x01);}
void cxp84(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x01);}
void cxp85(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x01);}
void cxp86(ZXBase* p) {xRES(p,p->cpu->ix,0x01);}
void cxp87(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x01);}

void cxp88(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x02);}
void cxp89(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x02);}
void cxp8A(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x02);}
void cxp8B(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x02);}
void cxp8C(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x02);}
void cxp8D(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x02);}
void cxp8E(ZXBase* p) {xRES(p,p->cpu->ix,0x02);}
void cxp8F(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x02);}

void cxp90(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x04);}
void cxp91(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x04);}
void cxp92(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x04);}
void cxp93(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x04);}
void cxp94(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x04);}
void cxp95(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x04);}
void cxp96(ZXBase* p) {xRES(p,p->cpu->ix,0x04);}
void cxp97(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x04);}

void cxp98(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x08);}
void cxp99(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x08);}
void cxp9A(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x08);}
void cxp9B(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x08);}
void cxp9C(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x08);}
void cxp9D(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x08);}
void cxp9E(ZXBase* p) {xRES(p,p->cpu->ix,0x08);}
void cxp9F(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x08);}

void cxpA0(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x10);}
void cxpA1(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x10);}
void cxpA2(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x10);}
void cxpA3(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x10);}
void cxpA4(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x10);}
void cxpA5(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x10);}
void cxpA6(ZXBase* p) {xRES(p,p->cpu->ix,0x10);}
void cxpA7(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x10);}

void cxpA8(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x20);}
void cxpA9(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x20);}
void cxpAA(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x20);}
void cxpAB(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x20);}
void cxpAC(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x20);}
void cxpAD(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x20);}
void cxpAE(ZXBase* p) {xRES(p,p->cpu->ix,0x20);}
void cxpAF(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x20);}

void cxpB0(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x40);}
void cxpB1(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x40);}
void cxpB2(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x40);}
void cxpB3(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x40);}
void cxpB4(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x40);}
void cxpB5(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x40);}
void cxpB6(ZXBase* p) {xRES(p,p->cpu->ix,0x40);}
void cxpB7(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x40);}

void cxpB8(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->ix,0x80);}
void cxpB9(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->ix,0x80);}
void cxpBA(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->ix,0x80);}
void cxpBB(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->ix,0x80);}
void cxpBC(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->ix,0x80);}
void cxpBD(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->ix,0x80);}
void cxpBE(ZXBase* p) {xRES(p,p->cpu->ix,0x80);}
void cxpBF(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->ix,0x80);}

void cxpC0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x01);}
void cxpC1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x01);}
void cxpC2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x01);}
void cxpC3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x01);}
void cxpC4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x01);}
void cxpC5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x01);}
void cxpC6(ZXBase* p) {xSET(p,p->cpu->ix,0x01);}
void cxpC7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x01);}

void cxpC8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x02);}
void cxpC9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x02);}
void cxpCA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x02);}
void cxpCB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x02);}
void cxpCC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x02);}
void cxpCD(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x02);}
void cxpCE(ZXBase* p) {xSET(p,p->cpu->ix,0x02);}
void cxpCF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x02);}

void cxpD0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x04);}
void cxpD1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x04);}
void cxpD2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x04);}
void cxpD3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x04);}
void cxpD4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x04);}
void cxpD5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x04);}
void cxpD6(ZXBase* p) {xSET(p,p->cpu->ix,0x04);}
void cxpD7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x04);}

void cxpD8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x08);}
void cxpD9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x08);}
void cxpDA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x08);}
void cxpDB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x08);}
void cxpDC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x08);}
void cxpDD(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x08);}
void cxpDE(ZXBase* p) {xSET(p,p->cpu->ix,0x08);}
void cxpDF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x08);}

void cxpE0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x10);}
void cxpE1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x10);}
void cxpE2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x10);}
void cxpE3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x10);}
void cxpE4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x10);}
void cxpE5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x10);}
void cxpE6(ZXBase* p) {xSET(p,p->cpu->ix,0x10);}
void cxpE7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x10);}

void cxpE8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x20);}
void cxpE9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x20);}
void cxpEA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x20);}
void cxpEB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x20);}
void cxpEC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x20);}
void cxpED(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x20);}
void cxpEE(ZXBase* p) {xSET(p,p->cpu->ix,0x20);}
void cxpEF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x20);}

void cxpF0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x40);}
void cxpF1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x40);}
void cxpF2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x40);}
void cxpF3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x40);}
void cxpF4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x40);}
void cxpF5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x40);}
void cxpF6(ZXBase* p) {xSET(p,p->cpu->ix,0x40);}
void cxpF7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x40);}

void cxpF8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->ix,0x80);}
void cxpF9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->ix,0x80);}
void cxpFA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->ix,0x80);}
void cxpFB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->ix,0x80);}
void cxpFC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->ix,0x80);}
void cxpFD(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->ix,0x80);}
void cxpFE(ZXBase* p) {xSET(p,p->cpu->ix,0x80);}
void cxpFF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->ix,0x80);}

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

