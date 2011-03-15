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
	ZOp(&cyp00,15,"rlc b,(iy:5)"),
	ZOp(&cyp01,15,"rlc c,(iy:5)"),
	ZOp(&cyp02,15,"rlc d,(iy:5)"),
	ZOp(&cyp03,15,"rlc e,(iy:5)"),
	ZOp(&cyp04,15,"rlc h,(iy:5)"),
	ZOp(&cyp05,15,"rlc l,(iy:5)"),
	ZOp(&cyp06,15,"rlc (iy:5)"),
	ZOp(&cyp07,15,"rlc a,(iy:5)"),

	ZOp(&cyp08,15,"rrc b,(iy:5)"),
	ZOp(&cyp09,15,"rrc c,(iy:5)"),
	ZOp(&cyp0A,15,"rrc d,(iy:5)"),
	ZOp(&cyp0B,15,"rrc e,(iy:5)"),
	ZOp(&cyp0C,15,"rrc h,(iy:5)"),
	ZOp(&cyp0D,15,"rrc l,(iy:5)"),
	ZOp(&cyp0E,15,"rrc (iy:5)"),
	ZOp(&cyp0F,15,"rrc a,(iy:5)"),

	ZOp(&cyp10,15,"rl b,(iy:5)"),
	ZOp(&cyp11,15,"rl c,(iy:5)"),
	ZOp(&cyp12,15,"rl d,(iy:5)"),
	ZOp(&cyp13,15,"rl e,(iy:5)"),
	ZOp(&cyp14,15,"rl h,(iy:5)"),
	ZOp(&cyp15,15,"rl l,(iy:5)"),
	ZOp(&cyp16,15,"rl (iy:5)"),
	ZOp(&cyp17,15,"rl a,(iy:5)"),

	ZOp(&cyp18,15,"rr b,(iy:5)"),
	ZOp(&cyp19,15,"rr c,(iy:5)"),
	ZOp(&cyp1A,15,"rr d,(iy:5)"),
	ZOp(&cyp1B,15,"rr e,(iy:5)"),
	ZOp(&cyp1C,15,"rr h,(iy:5)"),
	ZOp(&cyp1D,15,"rr l,(iy:5)"),
	ZOp(&cyp1E,15,"rr (iy:5)"),
	ZOp(&cyp1F,15,"rr a,(iy:5)"),

	ZOp(&cyp20,15,"sla b,(iy:5)"),
	ZOp(&cyp21,15,"sla c,(iy:5)"),
	ZOp(&cyp22,15,"sla d,(iy:5)"),
	ZOp(&cyp23,15,"sla e,(iy:5)"),
	ZOp(&cyp24,15,"sla h,(iy:5)"),
	ZOp(&cyp25,15,"sla l,(iy:5)"),
	ZOp(&cyp26,15,"sla (iy:5)"),
	ZOp(&cyp27,15,"sla a,(iy:5)"),

	ZOp(&cyp28,15,"sra b,(iy:5)"),
	ZOp(&cyp29,15,"sra c,(iy:5)"),
	ZOp(&cyp2A,15,"sra d,(iy:5)"),
	ZOp(&cyp2B,15,"sra e,(iy:5)"),
	ZOp(&cyp2C,15,"sra h,(iy:5)"),
	ZOp(&cyp2D,15,"sra l,(iy:5)"),
	ZOp(&cyp2E,15,"sra (iy:5)"),
	ZOp(&cyp2F,15,"sra a,(iy:5)"),

	ZOp(&cyp30,15,"sli b,(iy:5)"),
	ZOp(&cyp31,15,"sli c,(iy:5)"),
	ZOp(&cyp32,15,"sli d,(iy:5)"),
	ZOp(&cyp33,15,"sli e,(iy:5)"),
	ZOp(&cyp34,15,"sli h,(iy:5)"),
	ZOp(&cyp35,15,"sli l,(iy:5)"),
	ZOp(&cyp36,15,"sli (iy:5)"),
	ZOp(&cyp37,15,"sli a,(iy:5)"),

	ZOp(&cyp38,15,"srl b,(iy:5)"),
	ZOp(&cyp39,15,"srl c,(iy:5)"),
	ZOp(&cyp3A,15,"srl d,(iy:5)"),
	ZOp(&cyp3B,15,"srl e,(iy:5)"),
	ZOp(&cyp3C,15,"srl h,(iy:5)"),
	ZOp(&cyp3D,15,"srl l,(iy:5)"),
	ZOp(&cyp3E,15,"srl (iy:5)"),
	ZOp(&cyp3F,15,"srl a,(iy:5)"),

	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),
	ZOp(&cyp46,12,"bit 0,(iy:5)"),

	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),
	ZOp(&cyp4E,12,"bit 1,(iy:5)"),

	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),
	ZOp(&cyp56,12,"bit 2,(iy:5)"),

	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),
	ZOp(&cyp5E,12,"bit 3,(iy:5)"),

	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),
	ZOp(&cyp66,12,"bit 4,(iy:5)"),

	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),
	ZOp(&cyp6E,12,"bit 5,(iy:5)"),

	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),
	ZOp(&cyp76,12,"bit 6,(iy:5)"),

	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),
	ZOp(&cyp7E,12,"bit 7,(iy:5)"),

	ZOp(&cyp80,15,"res 0,b,(iy:5)"),
	ZOp(&cyp81,15,"res 0,c,(iy:5)"),
	ZOp(&cyp82,15,"res 0,d,(iy:5)"),
	ZOp(&cyp83,15,"res 0,e,(iy:5)"),
	ZOp(&cyp84,15,"res 0,h,(iy:5)"),
	ZOp(&cyp85,15,"res 0,b,(iy:5)"),
	ZOp(&cyp86,15,"res 0,(iy:5)"),
	ZOp(&cyp87,15,"res 0,a,(iy:5)"),

	ZOp(&cyp88,15,"res 1,b,(iy:5)"),
	ZOp(&cyp89,15,"res 1,c,(iy:5)"),
	ZOp(&cyp8A,15,"res 1,d,(iy:5)"),
	ZOp(&cyp8B,15,"res 1,e,(iy:5)"),
	ZOp(&cyp8C,15,"res 1,h,(iy:5)"),
	ZOp(&cyp8D,15,"res 1,l,(iy:5)"),
	ZOp(&cyp8E,15,"res 1,(iy:5)"),
	ZOp(&cyp8F,15,"res 1,a,(iy:5)"),

	ZOp(&cyp90,15,"res 2,b,(iy:5)"),
	ZOp(&cyp91,15,"res 2,c,(iy:5)"),
	ZOp(&cyp92,15,"res 2,d,(iy:5)"),
	ZOp(&cyp93,15,"res 2,e,(iy:5)"),
	ZOp(&cyp94,15,"res 2,h,(iy:5)"),
	ZOp(&cyp95,15,"res 2,l,(iy:5)"),
	ZOp(&cyp96,15,"res 2,(iy:5)"),
	ZOp(&cyp97,15,"res 2,a,(iy:5)"),

	ZOp(&cyp98,15,"res 3,b,(iy:5)"),
	ZOp(&cyp99,15,"res 3,c,(iy:5)"),
	ZOp(&cyp9A,15,"res 3,d,(iy:5)"),
	ZOp(&cyp9B,15,"res 3,e,(iy:5)"),
	ZOp(&cyp9C,15,"res 3,h,(iy:5)"),
	ZOp(&cyp9D,15,"res 3,l,(iy:5)"),
	ZOp(&cyp9E,15,"res 3,(iy:5)"),
	ZOp(&cyp9F,15,"res 3,a,(iy:5)"),

	ZOp(&cypA0,15,"res 4,b,(iy:5)"),
	ZOp(&cypA1,15,"res 4,c,(iy:5)"),
	ZOp(&cypA2,15,"res 4,d,(iy:5)"),
	ZOp(&cypA3,15,"res 4,e,(iy:5)"),
	ZOp(&cypA4,15,"res 4,h,(iy:5)"),
	ZOp(&cypA5,15,"res 4,l,(iy:5)"),
	ZOp(&cypA6,15,"res 4,(iy:5)"),
	ZOp(&cypA7,15,"res 4,a,(iy:5)"),

	ZOp(&cypA8,15,"res 5,b,(iy:5)"),
	ZOp(&cypA9,15,"res 5,c,(iy:5)"),
	ZOp(&cypAA,15,"res 5,d,(iy:5)"),
	ZOp(&cypAB,15,"res 5,e,(iy:5)"),
	ZOp(&cypAC,15,"res 5,h,(iy:5)"),
	ZOp(&cypAD,15,"res 5,l,(iy:5)"),
	ZOp(&cypAE,15,"res 5,(iy:5)"),
	ZOp(&cypAF,15,"res 5,a,(iy:5)"),

	ZOp(&cypB0,15,"res 6,b,(iy:5)"),
	ZOp(&cypB1,15,"res 6,c,(iy:5)"),
	ZOp(&cypB2,15,"res 6,d,(iy:5)"),
	ZOp(&cypB3,15,"res 6,e,(iy:5)"),
	ZOp(&cypB4,15,"res 6,h,(iy:5)"),
	ZOp(&cypB5,15,"res 6,l,(iy:5)"),
	ZOp(&cypB6,15,"res 6,(iy:5)"),
	ZOp(&cypB7,15,"res 6,a,(iy:5)"),

	ZOp(&cypB8,15,"res 7,b,(iy:5)"),
	ZOp(&cypB9,15,"res 7,c,(iy:5)"),
	ZOp(&cypBA,15,"res 7,d,(iy:5)"),
	ZOp(&cypBB,15,"res 7,e,(iy:5)"),
	ZOp(&cypBC,15,"res 7,h,(iy:5)"),
	ZOp(&cypBD,15,"res 7,l,(iy:5)"),
	ZOp(&cypBE,15,"res 7,(iy:5)"),
	ZOp(&cypBF,15,"res 7,a,(iy:5)"),

	ZOp(&cypC0,15,"set 0,b,(iy:5)"),
	ZOp(&cypC1,15,"set 0,c,(iy:5)"),
	ZOp(&cypC2,15,"set 0,d,(iy:5)"),
	ZOp(&cypC3,15,"set 0,e,(iy:5)"),
	ZOp(&cypC4,15,"set 0,h,(iy:5)"),
	ZOp(&cypC5,15,"set 0,l,(iy:5)"),
	ZOp(&cypC6,15,"set 0,(iy:5)"),
	ZOp(&cypC7,15,"set 0,a,(iy:5)"),

	ZOp(&cypC8,15,"set 1,b,(iy:5)"),
	ZOp(&cypC9,15,"set 1,c,(iy:5)"),
	ZOp(&cypCA,15,"set 1,d,(iy:5)"),
	ZOp(&cypCB,15,"set 1,e,(iy:5)"),
	ZOp(&cypCC,15,"set 1,h,(iy:5)"),
	ZOp(&cypCD,15,"set 1,l,(iy:5)"),
	ZOp(&cypCE,15,"set 1,(iy:5)"),
	ZOp(&cypCF,15,"set 1,a,(iy:5)"),

	ZOp(&cypD0,15,"set 2,b,(iy:5)"),
	ZOp(&cypD1,15,"set 2,c,(iy:5)"),
	ZOp(&cypD2,15,"set 2,d,(iy:5)"),
	ZOp(&cypD3,15,"set 2,e,(iy:5)"),
	ZOp(&cypD4,15,"set 2,h,(iy:5)"),
	ZOp(&cypD5,15,"set 2,l,(iy:5)"),
	ZOp(&cypD6,15,"set 2,(iy:5)"),
	ZOp(&cypD7,15,"set 2,a,(iy:5)"),

	ZOp(&cypD8,15,"set 3,b,(iy:5)"),
	ZOp(&cypD9,15,"set 3,c,(iy:5)"),
	ZOp(&cypDA,15,"set 3,d,(iy:5)"),
	ZOp(&cypDB,15,"set 3,e,(iy:5)"),
	ZOp(&cypDC,15,"set 3,h,(iy:5)"),
	ZOp(&cypDD,15,"set 3,l,(iy:5)"),
	ZOp(&cypDE,15,"set 3,(iy:5)"),
	ZOp(&cypDF,15,"set 3,a,(iy:5)"),

	ZOp(&cypE0,15,"set 4,b,(iy:5)"),
	ZOp(&cypE1,15,"set 4,c,(iy:5)"),
	ZOp(&cypE2,15,"set 4,d,(iy:5)"),
	ZOp(&cypE3,15,"set 4,e,(iy:5)"),
	ZOp(&cypE4,15,"set 4,h,(iy:5)"),
	ZOp(&cypE5,15,"set 4,l,(iy:5)"),
	ZOp(&cypE6,15,"set 4,(iy:5)"),
	ZOp(&cypE7,15,"set 4,a,(iy:5)"),

	ZOp(&cypE8,15,"set 5,b,(iy:5)"),
	ZOp(&cypE9,15,"set 5,c,(iy:5)"),
	ZOp(&cypEA,15,"set 5,d,(iy:5)"),
	ZOp(&cypEB,15,"set 5,e,(iy:5)"),
	ZOp(&cypEC,15,"set 5,h,(iy:5)"),
	ZOp(&cypED,15,"set 5,l,(iy:5)"),
	ZOp(&cypEE,15,"set 5,(iy:5)"),
	ZOp(&cypEF,15,"set 5,a,(iy:5)"),

	ZOp(&cypF0,15,"set 6,b,(iy:5)"),
	ZOp(&cypF1,15,"set 6,c,(iy:5)"),
	ZOp(&cypF2,15,"set 6,d,(iy:5)"),
	ZOp(&cypF3,15,"set 6,e,(iy:5)"),
	ZOp(&cypF4,15,"set 6,h,(iy:5)"),
	ZOp(&cypF5,15,"set 6,l,(iy:5)"),
	ZOp(&cypF6,15,"set 6,(iy:5)"),
	ZOp(&cypF7,15,"set 6,a,(iy:5)"),

	ZOp(&cypF8,15,"set 7,b,(iy:5)"),
	ZOp(&cypF9,15,"set 7,c,(iy:5)"),
	ZOp(&cypFA,15,"set 7,d,(iy:5)"),
	ZOp(&cypFB,15,"set 7,e,(iy:5)"),
	ZOp(&cypFC,15,"set 7,h,(iy:5)"),
	ZOp(&cypFD,15,"set 7,l,(iy:5)"),
	ZOp(&cypFE,15,"set 7,(iy:5)"),
	ZOp(&cypFF,15,"set 7,a,(iy:5)"),
};

