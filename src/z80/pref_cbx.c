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
	{15,	CND_NONE,0,0,	0,	&cxp00,	"rlc b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp01,	"rlc c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp02,	"rlc d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp03,	"rlc e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp04,	"rlc h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp05,	"rlc l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp06,	"rlc (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp07,	"rlc a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp08,	"rrc b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp09,	"rrc c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp0A,	"rrc d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp0B,	"rrc e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp0C,	"rrc h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp0D,	"rrc l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp0E,	"rrc (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp0F,	"rrc a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp10,	"rl b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp11,	"rl c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp12,	"rl d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp13,	"rl e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp14,	"rl h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp15,	"rl l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp16,	"rl (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp17,	"rl a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp18,	"rr b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp19,	"rr c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp1A,	"rr d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp1B,	"rr e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp1C,	"rr h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp1D,	"rr l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp1E,	"rr (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp1F,	"rr a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp20,	"sla b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp21,	"sla c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp22,	"sla d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp23,	"sla e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp24,	"sla h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp25,	"sla l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp26,	"sla (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp27,	"sla a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp28,	"sra b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp29,	"sra c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp2A,	"sra d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp2B,	"sra e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp2C,	"sra h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp2D,	"sra l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp2E,	"sra (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp2F,	"sra a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp30,	"sli b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp31,	"sli c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp32,	"sli d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp33,	"sli e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp34,	"sli h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp35,	"sli l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp36,	"sli (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp37,	"sli a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp38,	"srl b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp39,	"srl c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp3A,	"srl d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp3B,	"srl e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp3C,	"srl h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp3D,	"srl l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp3E,	"srl (ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp3F,	"srl a,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp46,	"bit 0,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp4E,	"bit 1,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp56,	"bit 2,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp5E,	"bit 3,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp66,	"bit 4,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp6E,	"bit 5,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp76,	"bit 6,(ix:5)"},

	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},
	{12,	CND_NONE,0,0,	0,	&cxp7E,	"bit 7,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp80,	"res 0,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp81,	"res 0,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp82,	"res 0,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp83,	"res 0,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp84,	"res 0,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp85,	"res 0,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp86,	"res 0,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp87,	"res 0,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp88,	"res 1,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp89,	"res 1,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp8A,	"res 1,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp8B,	"res 1,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp8C,	"res 1,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp8D,	"res 1,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp8E,	"res 1,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp8F,	"res 1,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp90,	"res 2,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp91,	"res 2,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp92,	"res 2,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp93,	"res 2,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp94,	"res 2,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp95,	"res 2,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp96,	"res 2,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp97,	"res 2,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxp98,	"res 3,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp99,	"res 3,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp9A,	"res 3,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp9B,	"res 3,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp9C,	"res 3,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp9D,	"res 3,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp9E,	"res 3,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxp9F,	"res 3,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpA0,	"res 4,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA1,	"res 4,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA2,	"res 4,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA3,	"res 4,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA4,	"res 4,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA5,	"res 4,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA6,	"res 4,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA7,	"res 4,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpA8,	"res 5,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpA9,	"res 5,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpAA,	"res 5,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpAB,	"res 5,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpAC,	"res 5,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpAD,	"res 5,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpAE,	"res 5,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpAF,	"res 5,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpB0,	"res 6,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB1,	"res 6,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB2,	"res 6,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB3,	"res 6,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB4,	"res 6,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB5,	"res 6,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB6,	"res 6,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB7,	"res 6,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpB8,	"res 7,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpB9,	"res 7,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpBA,	"res 7,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpBB,	"res 7,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpBC,	"res 7,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpBD,	"res 7,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpBE,	"res 7,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpBF,	"res 7,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpC0,	"set 0,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC1,	"set 0,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC2,	"set 0,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC3,	"set 0,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC4,	"set 0,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC5,	"set 0,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC6,	"set 0,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC7,	"set 0,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpC8,	"set 1,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpC9,	"set 1,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpCA,	"set 1,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpCB,	"set 1,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpCC,	"set 1,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpCD,	"set 1,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpCE,	"set 1,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpCF,	"set 1,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpD0,	"set 2,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD1,	"set 2,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD2,	"set 2,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD3,	"set 2,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD4,	"set 2,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD5,	"set 2,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD6,	"set 2,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD7,	"set 2,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpD8,	"set 3,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpD9,	"set 3,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpDA,	"set 3,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpDB,	"set 3,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpDC,	"set 3,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpDD,	"set 3,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpDE,	"set 3,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpDF,	"set 3,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpE0,	"set 4,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE1,	"set 4,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE2,	"set 4,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE3,	"set 4,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE4,	"set 4,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE5,	"set 4,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE6,	"set 4,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE7,	"set 4,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpE8,	"set 5,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpE9,	"set 5,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpEA,	"set 5,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpEB,	"set 5,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpEC,	"set 5,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpED,	"set 5,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpEE,	"set 5,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpEF,	"set 5,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpF0,	"set 6,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF1,	"set 6,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF2,	"set 6,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF3,	"set 6,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF4,	"set 6,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF5,	"set 6,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF6,	"set 6,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF7,	"set 6,a,(ix:5)"},

	{15,	CND_NONE,0,0,	0,	&cxpF8,	"set 7,b,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpF9,	"set 7,c,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpFA,	"set 7,d,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpFB,	"set 7,e,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpFC,	"set 7,h,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpFD,	"set 7,l,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpFE,	"set 7,(ix:5)"},
	{15,	CND_NONE,0,0,	0,	&cxpFF,	"set 7,a,(ix:5)"},
};
