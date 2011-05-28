void cyp00(ZXBase* p) {p->cpu->b = xRLC(p,p->cpu->iy);}
void cyp01(ZXBase* p) {p->cpu->c = xRLC(p,p->cpu->iy);}
void cyp02(ZXBase* p) {p->cpu->d = xRLC(p,p->cpu->iy);}
void cyp03(ZXBase* p) {p->cpu->e = xRLC(p,p->cpu->iy);}
void cyp04(ZXBase* p) {p->cpu->h = xRLC(p,p->cpu->iy);}
void cyp05(ZXBase* p) {p->cpu->l = xRLC(p,p->cpu->iy);}
void cyp06(ZXBase* p) {xRLC(p,p->cpu->iy);}
void cyp07(ZXBase* p) {p->cpu->a = xRLC(p,p->cpu->iy);}

void cyp08(ZXBase* p) {p->cpu->b = xRRC(p,p->cpu->iy);}
void cyp09(ZXBase* p) {p->cpu->c = xRRC(p,p->cpu->iy);}
void cyp0A(ZXBase* p) {p->cpu->d = xRRC(p,p->cpu->iy);}
void cyp0B(ZXBase* p) {p->cpu->e = xRRC(p,p->cpu->iy);}
void cyp0C(ZXBase* p) {p->cpu->h = xRRC(p,p->cpu->iy);}
void cyp0D(ZXBase* p) {p->cpu->l = xRRC(p,p->cpu->iy);}
void cyp0E(ZXBase* p) {xRRC(p,p->cpu->iy);}
void cyp0F(ZXBase* p) {p->cpu->a = xRRC(p,p->cpu->iy);}

void cyp10(ZXBase* p) {p->cpu->b = xRL(p,p->cpu->iy);}
void cyp11(ZXBase* p) {p->cpu->c = xRL(p,p->cpu->iy);}
void cyp12(ZXBase* p) {p->cpu->d = xRL(p,p->cpu->iy);}
void cyp13(ZXBase* p) {p->cpu->e = xRL(p,p->cpu->iy);}
void cyp14(ZXBase* p) {p->cpu->h = xRL(p,p->cpu->iy);}
void cyp15(ZXBase* p) {p->cpu->l = xRL(p,p->cpu->iy);}
void cyp16(ZXBase* p) {xRL(p,p->cpu->iy);}
void cyp17(ZXBase* p) {p->cpu->a = xRL(p,p->cpu->iy);}

void cyp18(ZXBase* p) {p->cpu->b = xRR(p,p->cpu->iy);}
void cyp19(ZXBase* p) {p->cpu->c = xRR(p,p->cpu->iy);}
void cyp1A(ZXBase* p) {p->cpu->d = xRR(p,p->cpu->iy);}
void cyp1B(ZXBase* p) {p->cpu->e = xRR(p,p->cpu->iy);}
void cyp1C(ZXBase* p) {p->cpu->h = xRR(p,p->cpu->iy);}
void cyp1D(ZXBase* p) {p->cpu->l = xRR(p,p->cpu->iy);}
void cyp1E(ZXBase* p) {xRR(p,p->cpu->iy);}
void cyp1F(ZXBase* p) {p->cpu->a = xRR(p,p->cpu->iy);}

void cyp20(ZXBase* p) {p->cpu->b = xSLA(p,p->cpu->iy);}
void cyp21(ZXBase* p) {p->cpu->c = xSLA(p,p->cpu->iy);}
void cyp22(ZXBase* p) {p->cpu->d = xSLA(p,p->cpu->iy);}
void cyp23(ZXBase* p) {p->cpu->e = xSLA(p,p->cpu->iy);}
void cyp24(ZXBase* p) {p->cpu->h = xSLA(p,p->cpu->iy);}
void cyp25(ZXBase* p) {p->cpu->l = xSLA(p,p->cpu->iy);}
void cyp26(ZXBase* p) {xSLA(p,p->cpu->iy);}
void cyp27(ZXBase* p) {p->cpu->a = xSLA(p,p->cpu->iy);}

void cyp28(ZXBase* p) {p->cpu->b = xSRA(p,p->cpu->iy);}
void cyp29(ZXBase* p) {p->cpu->c = xSRA(p,p->cpu->iy);}
void cyp2A(ZXBase* p) {p->cpu->d = xSRA(p,p->cpu->iy);}
void cyp2B(ZXBase* p) {p->cpu->e = xSRA(p,p->cpu->iy);}
void cyp2C(ZXBase* p) {p->cpu->h = xSRA(p,p->cpu->iy);}
void cyp2D(ZXBase* p) {p->cpu->l = xSRA(p,p->cpu->iy);}
void cyp2E(ZXBase* p) {xSRA(p,p->cpu->iy);}
void cyp2F(ZXBase* p) {p->cpu->a = xSRA(p,p->cpu->iy);}

void cyp30(ZXBase* p) {p->cpu->b = xSLI(p,p->cpu->iy);}
void cyp31(ZXBase* p) {p->cpu->c = xSLI(p,p->cpu->iy);}
void cyp32(ZXBase* p) {p->cpu->d = xSLI(p,p->cpu->iy);}
void cyp33(ZXBase* p) {p->cpu->e = xSLI(p,p->cpu->iy);}
void cyp34(ZXBase* p) {p->cpu->h = xSLI(p,p->cpu->iy);}
void cyp35(ZXBase* p) {p->cpu->l = xSLI(p,p->cpu->iy);}
void cyp36(ZXBase* p) {xSLI(p,p->cpu->iy);}
void cyp37(ZXBase* p) {p->cpu->a = xSLI(p,p->cpu->iy);}

void cyp38(ZXBase* p) {p->cpu->b = xSRL(p,p->cpu->iy);}
void cyp39(ZXBase* p) {p->cpu->c = xSRL(p,p->cpu->iy);}
void cyp3A(ZXBase* p) {p->cpu->d = xSRL(p,p->cpu->iy);}
void cyp3B(ZXBase* p) {p->cpu->e = xSRL(p,p->cpu->iy);}
void cyp3C(ZXBase* p) {p->cpu->h = xSRL(p,p->cpu->iy);}
void cyp3D(ZXBase* p) {p->cpu->l = xSRL(p,p->cpu->iy);}
void cyp3E(ZXBase* p) {xSRL(p,p->cpu->iy);}
void cyp3F(ZXBase* p) {p->cpu->a = xSRL(p,p->cpu->iy);}
// bit n,(iy+e)		flag b3,5 = memptr b3,5
void cyp46(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[0]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp4E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[1]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp56(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[2]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp5E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[3]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp66(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[4]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp6E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[5]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp76(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[6]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}
void cyp7E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->cpu->dlt; p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->mptr)].bit[7]) & (~(F5 | F3))) | (p->cpu->hptr & (F3 | F5));}

void cyp80(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x01);}
void cyp81(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x01);}
void cyp82(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x01);}
void cyp83(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x01);}
void cyp84(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x01);}
void cyp85(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x01);}
void cyp86(ZXBase* p) {xRES(p,p->cpu->iy,0x01);}
void cyp87(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x01);}

void cyp88(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x02);}
void cyp89(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x02);}
void cyp8A(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x02);}
void cyp8B(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x02);}
void cyp8C(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x02);}
void cyp8D(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x02);}
void cyp8E(ZXBase* p) {xRES(p,p->cpu->iy,0x02);}
void cyp8F(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x02);}

void cyp90(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x04);}
void cyp91(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x04);}
void cyp92(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x04);}
void cyp93(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x04);}
void cyp94(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x04);}
void cyp95(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x04);}
void cyp96(ZXBase* p) {xRES(p,p->cpu->iy,0x04);}
void cyp97(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x04);}

void cyp98(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x08);}
void cyp99(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x08);}
void cyp9A(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x08);}
void cyp9B(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x08);}
void cyp9C(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x08);}
void cyp9D(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x08);}
void cyp9E(ZXBase* p) {xRES(p,p->cpu->iy,0x08);}
void cyp9F(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x08);}

void cypA0(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x10);}
void cypA1(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x10);}
void cypA2(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x10);}
void cypA3(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x10);}
void cypA4(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x10);}
void cypA5(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x10);}
void cypA6(ZXBase* p) {xRES(p,p->cpu->iy,0x10);}
void cypA7(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x10);}

void cypA8(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x20);}
void cypA9(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x20);}
void cypAA(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x20);}
void cypAB(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x20);}
void cypAC(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x20);}
void cypAD(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x20);}
void cypAE(ZXBase* p) {xRES(p,p->cpu->iy,0x20);}
void cypAF(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x20);}

void cypB0(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x40);}
void cypB1(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x40);}
void cypB2(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x40);}
void cypB3(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x40);}
void cypB4(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x40);}
void cypB5(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x40);}
void cypB6(ZXBase* p) {xRES(p,p->cpu->iy,0x40);}
void cypB7(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x40);}

void cypB8(ZXBase* p) {p->cpu->b = xRES(p,p->cpu->iy,0x80);}
void cypB9(ZXBase* p) {p->cpu->c = xRES(p,p->cpu->iy,0x80);}
void cypBA(ZXBase* p) {p->cpu->d = xRES(p,p->cpu->iy,0x80);}
void cypBB(ZXBase* p) {p->cpu->e = xRES(p,p->cpu->iy,0x80);}
void cypBC(ZXBase* p) {p->cpu->h = xRES(p,p->cpu->iy,0x80);}
void cypBD(ZXBase* p) {p->cpu->l = xRES(p,p->cpu->iy,0x80);}
void cypBE(ZXBase* p) {xRES(p,p->cpu->iy,0x80);}
void cypBF(ZXBase* p) {p->cpu->a = xRES(p,p->cpu->iy,0x80);}

void cypC0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x01);}
void cypC1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x01);}
void cypC2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x01);}
void cypC3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x01);}
void cypC4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x01);}
void cypC5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x01);}
void cypC6(ZXBase* p) {xSET(p,p->cpu->iy,0x01);}
void cypC7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x01);}

void cypC8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x02);}
void cypC9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x02);}
void cypCA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x02);}
void cypCB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x02);}
void cypCC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x02);}
void cypCD(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x02);}
void cypCE(ZXBase* p) {xSET(p,p->cpu->iy,0x02);}
void cypCF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x02);}

void cypD0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x04);}
void cypD1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x04);}
void cypD2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x04);}
void cypD3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x04);}
void cypD4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x04);}
void cypD5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x04);}
void cypD6(ZXBase* p) {xSET(p,p->cpu->iy,0x04);}
void cypD7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x04);}

void cypD8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x08);}
void cypD9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x08);}
void cypDA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x08);}
void cypDB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x08);}
void cypDC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x08);}
void cypDD(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x08);}
void cypDE(ZXBase* p) {xSET(p,p->cpu->iy,0x08);}
void cypDF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x08);}

void cypE0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x10);}
void cypE1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x10);}
void cypE2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x10);}
void cypE3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x10);}
void cypE4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x10);}
void cypE5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x10);}
void cypE6(ZXBase* p) {xSET(p,p->cpu->iy,0x10);}
void cypE7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x10);}

void cypE8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x20);}
void cypE9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x20);}
void cypEA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x20);}
void cypEB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x20);}
void cypEC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x20);}
void cypED(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x20);}
void cypEE(ZXBase* p) {xSET(p,p->cpu->iy,0x20);}
void cypEF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x20);}

void cypF0(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x40);}
void cypF1(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x40);}
void cypF2(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x40);}
void cypF3(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x40);}
void cypF4(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x40);}
void cypF5(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x40);}
void cypF6(ZXBase* p) {xSET(p,p->cpu->iy,0x40);}
void cypF7(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x40);}

void cypF8(ZXBase* p) {p->cpu->b = xSET(p,p->cpu->iy,0x80);}
void cypF9(ZXBase* p) {p->cpu->c = xSET(p,p->cpu->iy,0x80);}
void cypFA(ZXBase* p) {p->cpu->d = xSET(p,p->cpu->iy,0x80);}
void cypFB(ZXBase* p) {p->cpu->e = xSET(p,p->cpu->iy,0x80);}
void cypFC(ZXBase* p) {p->cpu->h = xSET(p,p->cpu->iy,0x80);}
void cypFD(ZXBase* p) {p->cpu->l = xSET(p,p->cpu->iy,0x80);}
void cypFE(ZXBase* p) {xSET(p,p->cpu->iy,0x80);}
void cypFF(ZXBase* p) {p->cpu->a = xSET(p,p->cpu->iy,0x80);}

//==================

ZOp cypref[256]={
	{15,	CND_NONE,0,0,	0,	&cyp00,	"rlc b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp01,	"rlc c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp02,	"rlc d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp03,	"rlc e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp04,	"rlc h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp05,	"rlc l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp06,	"rlc (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp07,	"rlc a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp08,	"rrc b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp09,	"rrc c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp0A,	"rrc d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp0B,	"rrc e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp0C,	"rrc h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp0D,	"rrc l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp0E,	"rrc (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp0F,	"rrc a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp10,	"rl b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp11,	"rl c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp12,	"rl d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp13,	"rl e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp14,	"rl h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp15,	"rl l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp16,	"rl (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp17,	"rl a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp18,	"rr b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp19,	"rr c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp1A,	"rr d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp1B,	"rr e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp1C,	"rr h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp1D,	"rr l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp1E,	"rr (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp1F,	"rr a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp20,	"sla b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp21,	"sla c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp22,	"sla d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp23,	"sla e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp24,	"sla h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp25,	"sla l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp26,	"sla (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp27,	"sla a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp28,	"sra b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp29,	"sra c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp2A,	"sra d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp2B,	"sra e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp2C,	"sra h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp2D,	"sra l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp2E,	"sra (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp2F,	"sra a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp30,	"sli b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp31,	"sli c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp32,	"sli d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp33,	"sli e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp34,	"sli h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp35,	"sli l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp36,	"sli (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp37,	"sli a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp38,	"srl b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp39,	"srl c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp3A,	"srl d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp3B,	"srl e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp3C,	"srl h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp3D,	"srl l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp3E,	"srl (iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp3F,	"srl a,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp46,	"bit 0,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp4E,	"bit 1,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp56,	"bit 2,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp5E,	"bit 3,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp66,	"bit 4,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp6E,	"bit 5,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp76,	"bit 6,(iy:5)"},

	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},
	{12,	CND_NONE,0,0,	0,	&cyp7E,	"bit 7,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp80,	"res 0,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp81,	"res 0,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp82,	"res 0,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp83,	"res 0,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp84,	"res 0,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp85,	"res 0,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp86,	"res 0,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp87,	"res 0,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp88,	"res 1,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp89,	"res 1,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp8A,	"res 1,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp8B,	"res 1,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp8C,	"res 1,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp8D,	"res 1,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp8E,	"res 1,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp8F,	"res 1,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp90,	"res 2,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp91,	"res 2,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp92,	"res 2,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp93,	"res 2,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp94,	"res 2,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp95,	"res 2,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp96,	"res 2,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp97,	"res 2,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cyp98,	"res 3,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp99,	"res 3,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp9A,	"res 3,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp9B,	"res 3,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp9C,	"res 3,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp9D,	"res 3,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp9E,	"res 3,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cyp9F,	"res 3,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypA0,	"res 4,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA1,	"res 4,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA2,	"res 4,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA3,	"res 4,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA4,	"res 4,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA5,	"res 4,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA6,	"res 4,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA7,	"res 4,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypA8,	"res 5,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypA9,	"res 5,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypAA,	"res 5,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypAB,	"res 5,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypAC,	"res 5,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypAD,	"res 5,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypAE,	"res 5,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypAF,	"res 5,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypB0,	"res 6,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB1,	"res 6,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB2,	"res 6,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB3,	"res 6,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB4,	"res 6,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB5,	"res 6,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB6,	"res 6,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB7,	"res 6,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypB8,	"res 7,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypB9,	"res 7,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypBA,	"res 7,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypBB,	"res 7,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypBC,	"res 7,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypBD,	"res 7,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypBE,	"res 7,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypBF,	"res 7,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypC0,	"set 0,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC1,	"set 0,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC2,	"set 0,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC3,	"set 0,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC4,	"set 0,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC5,	"set 0,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC6,	"set 0,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC7,	"set 0,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypC8,	"set 1,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypC9,	"set 1,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypCA,	"set 1,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypCB,	"set 1,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypCC,	"set 1,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypCD,	"set 1,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypCE,	"set 1,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypCF,	"set 1,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypD0,	"set 2,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD1,	"set 2,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD2,	"set 2,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD3,	"set 2,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD4,	"set 2,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD5,	"set 2,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD6,	"set 2,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD7,	"set 2,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypD8,	"set 3,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypD9,	"set 3,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypDA,	"set 3,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypDB,	"set 3,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypDC,	"set 3,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypDD,	"set 3,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypDE,	"set 3,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypDF,	"set 3,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypE0,	"set 4,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE1,	"set 4,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE2,	"set 4,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE3,	"set 4,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE4,	"set 4,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE5,	"set 4,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE6,	"set 4,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE7,	"set 4,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypE8,	"set 5,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypE9,	"set 5,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypEA,	"set 5,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypEB,	"set 5,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypEC,	"set 5,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypED,	"set 5,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypEE,	"set 5,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypEF,	"set 5,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypF0,	"set 6,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF1,	"set 6,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF2,	"set 6,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF3,	"set 6,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF4,	"set 6,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF5,	"set 6,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF6,	"set 6,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF7,	"set 6,a,(iy:5)"},

	{15,	CND_NONE,0,0,	0,	&cypF8,	"set 7,b,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypF9,	"set 7,c,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypFA,	"set 7,d,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypFB,	"set 7,e,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypFC,	"set 7,h,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypFD,	"set 7,l,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypFE,	"set 7,(iy:5)"},
	{15,	CND_NONE,0,0,	0,	&cypFF,	"set 7,a,(iy:5)"},
};
