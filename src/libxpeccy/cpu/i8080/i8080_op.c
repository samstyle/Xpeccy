#include "i8080.h"

extern const unsigned char sz53pTab[0x100];

// common

unsigned char iop_inr(CPU* cpu, unsigned char val) {
	val++;
	cpu->f &= (IFL_S | IFL_Z | IFL_A | IFL_P);
	cpu->f |= sz53pTab[val] & (IFL_S | IFL_Z | IFL_P);
	if (!(val & 0x0f)) cpu->f |= IFL_A;
	return val;
}

unsigned char iop_dcr(CPU* cpu, unsigned char val) {
	val--;
	cpu->f &= (IFL_S | IFL_Z | IFL_A | IFL_P);
	cpu->f |= sz53pTab[val] & (IFL_S | IFL_Z | IFL_P);
	if ((val & 0x0f) == 0x0f) cpu->f |= IFL_A;
	return val;
}

unsigned char iop_add(CPU* cpu, unsigned char val, unsigned char add) {
	cpu->tmpw = val + add;
	cpu->f = 2 | (sz53pTab[val] & (IFL_S | IFL_Z | IFL_P));
	if (cpu->htw) cpu->f |= IFL_C;
	if ((cpu->ltw ^ val) & 0xf0) cpu->f |= IFL_A;
	return cpu->ltw;
}

// 00, 08, 10, 18, 20, 28 : nop
void iop_00(CPU* cpu) {}

// 01:lxi b,nn
void iop_01(CPU* cpu) {
	cpu->t += 3;
	cpu->c = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->b = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 02:stax b
void iop_02(CPU* cpu) {
	cpu->t += 3;
	cpu->mwr(cpu->bc, cpu->a, cpu->data);
}

// 03:inx b
void iop_03(CPU* cpu) {
	cpu->bc++;
}

// 04:inr b
void iop_04(CPU* cpu) {
	cpu->b = iop_inr(cpu, cpu->b);
}

// 05:dcr b
void iop_05(CPU* cpu) {
	cpu->b = iop_dcr(cpu, cpu->b);
}

// 06:mvi b,n
void iop_06(CPU* cpu) {
	cpu->t += 3;
	cpu->b = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 07:rlc
void iop_07(CPU* cpu) {
	cpu->f &= ~IFL_C;
	if (cpu->a & 0x80) cpu->f |= IFL_C;
	cpu->a = (cpu->a << 1) | (cpu->a >> 7);
}

// 09:dad b
void iop_09(CPU* cpu) {
	cpu->tmpi = cpu->bc + cpu->hl;
	cpu->hl = cpu->tmpi & 0xffff;
	cpu->f &= ~IFL_C;
	if (cpu->tmpi & 0x10000) cpu->f |= IFL_C;
}

// 0a:ldax b
void iop_0a(CPU* cpu) {
	cpu->t += 3;
	cpu->a = cpu->mrd(cpu->bc, 0, cpu->data);
}

// 0b:dcx b
void iop_0b(CPU* cpu) {
	cpu->bc--;
}

// 0c:inr c
void iop_0c(CPU* cpu) {
	cpu->c = iop_inr(cpu, cpu->c);
}

// 0d:dcr c
void iop_0d(CPU* cpu) {
	cpu->c = iop_dcr(cpu, cpu->c);
}

// 0e:mvi c,n
void iop_0e(CPU* cpu) {
	cpu->t += 3;
	cpu->c = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 0f:rrc
void iop_0f(CPU* cpu) {
	cpu->f &= ~IFL_C;
	if (cpu->a & 0x01) cpu->f |= IFL_C;
	cpu->a = (cpu->a >> 1) | (cpu->a << 7);
}

// 11:lxi d,nn
void iop_11(CPU* cpu) {
	cpu->t += 3;
	cpu->e = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->d = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 12:stax d
void iop_12(CPU* cpu) {
	cpu->t += 3;
	cpu->mwr(cpu->de, cpu->a, cpu->data);
}

// 13:inx d
void iop_13(CPU* cpu) {
	cpu->de++;
}

// 14:inr d
void iop_14(CPU* cpu) {
	cpu->d = iop_inr(cpu, cpu->d);
}

// 15:dcr d
void iop_15(CPU* cpu) {
	cpu->d = iop_dcr(cpu, cpu->d);
}

// 16:mvi d,n
void iop_16(CPU* cpu) {
	cpu->t += 3;
	cpu->d = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 17:ral
void iop_17(CPU* cpu) {
	cpu->htw = cpu->a;
	cpu->ltw = (cpu->f & IFL_C) ? 0xff : 0x00;
	cpu->f = (cpu->f & ~IFL_C) | ((cpu->a & 0x80) ? IFL_C : 0);
	cpu->tmpw <<= 1;
	cpu->a = cpu->htw;
}

// 19:dad d
void iop_19(CPU* cpu) {
	cpu->tmpi = cpu->de + cpu->hl;
	cpu->hl = cpu->tmpi & 0xffff;
	cpu->f &= ~IFL_C;
	if (cpu->tmpi & 0x10000) cpu->f |= IFL_C;
}

// 1a:ldax d
void iop_1a(CPU* cpu) {
	cpu->t += 3;
	cpu->a = cpu->mrd(cpu->de, 0, cpu->data);
}

// 1b:dcx d
void iop_1b(CPU* cpu) {
	cpu->de--;
}

// 1c:inr e
void iop_1c(CPU* cpu) {
	cpu->e = iop_inr(cpu, cpu->e);
}

// 1d:dcr e
void iop_1d(CPU* cpu) {
	cpu->e = iop_dcr(cpu, cpu->e);
}

// 1e:mvi e,n
void iop_1e(CPU* cpu) {
	cpu->t += 3;
	cpu->e = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 1f:rar
void iop_1f(CPU* cpu) {
	cpu->htw = (cpu->f & IFL_C) ? 0xff : 0x00;
	cpu->ltw = cpu->a;
	cpu->f = (cpu->f & ~IFL_C) | ((cpu->a & 0x01) ? IFL_C : 0);
	cpu->tmpw >>= 1;
	cpu->a = cpu->ltw;
}

// 21:lxi h,nn
void iop_21(CPU* cpu) {
	cpu->t += 3;
	cpu->l = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->h = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 22:shld nn
void iop_22(CPU* cpu) {
	cpu->t += 3;
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->mwr(cpu->mptr++, cpu->l, cpu->data);
	cpu->t += 3;
	cpu->mwr(cpu->mptr, cpu->h, cpu->data);
}


// 23:inx h
void iop_23(CPU* cpu) {
	cpu->hl++;
}

// 24:inr h
void iop_24(CPU* cpu) {
	cpu->h = iop_inr(cpu, cpu->h);
}

// 25:dcr h
void iop_25(CPU* cpu) {
	cpu->h = iop_dcr(cpu, cpu->h);
}

// 26:mvi h,n
void iop_26(CPU* cpu) {
	cpu->t += 3;
	cpu->h = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 27:daa
void iop_27(CPU* cpu) {
	// TODO: daa
}

// 29:dad h
void iop_29(CPU* cpu) {
	cpu->tmpi = cpu->hl + cpu->hl;
	cpu->hl = cpu->tmpi & 0xffff;
	cpu->f &= ~IFL_C;
	if (cpu->tmpi & 0x10000) cpu->f |= IFL_C;
}

// 2a:lhld nn
void iop_2a(CPU* cpu) {
	cpu->t += 3;
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->l = cpu->mrd(cpu->mptr++, 0, cpu->data);
	cpu->t += 3;
	cpu->h = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// 2b:dcx h
void iop_2b(CPU* cpu) {
	cpu->hl--;
}

// 2c:inr l
void iop_2c(CPU* cpu) {
	cpu->l = iop_inr(cpu, cpu->e);
}

// 2d:dcr l
void iop_2d(CPU* cpu) {
	cpu->l = iop_dcr(cpu, cpu->l);
}

// 2e:mvi l,n
void iop_2e(CPU* cpu) {
	cpu->t += 3;
	cpu->l = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 2f:cma
void iop_2f(CPU* cpu) {
	cpu->a ^= 0xff;
}

// 31:lxi sp,nn
void iop_31(CPU* cpu) {
	cpu->t += 3;
	cpu->lsp = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->hsp = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 32:sta nn
void iop_32(CPU* cpu) {
	cpu->t += 3;
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->mwr(cpu->mptr, cpu->a, cpu->data);
}

// 33:inx sp
void iop_33(CPU* cpu) {
	cpu->sp++;
}

// 34:inr m
void iop_34(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->hl, 0, cpu->data);
	cpu->tmpb = iop_inr(cpu, cpu->tmpb);
	cpu->t += 3;
	cpu->mwr(cpu->hl, cpu->tmpb, cpu->data);
}

// 35:dcr m
void iop_35(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->hl, 0, cpu->data);
	cpu->tmpb = iop_dcr(cpu, cpu->tmpb);
	cpu->t += 3;
	cpu->mwr(cpu->hl, cpu->tmpb, cpu->data);
}

// 36:mvi m,n
void iop_36(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->mwr(cpu->hl, cpu->tmpb, cpu->data);
}

// 37:stc
void iop_37(CPU* cpu) {
	cpu->f |= IFL_C;
}

// 39:dad sp
void iop_39(CPU* cpu) {
	cpu->tmpi = cpu->sp + cpu->hl;
	cpu->hl = cpu->tmpi & 0xffff;
	cpu->f &= ~IFL_C;
	if (cpu->tmpi & 0x10000) cpu->f |= IFL_C;
}

// 3a:lda nn
void iop_3a(CPU* cpu) {
	cpu->t += 3;
	cpu->lptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->hptr = cpu->mrd(cpu->pc++, 0, cpu->data);
	cpu->t += 3;
	cpu->a = cpu->mrd(cpu->mptr, 0, cpu->data);
}

// 3b:dcx sp
void iop_3b(CPU* cpu) {
	cpu->sp--;
}

// 3c:inr a
void iop_3c(CPU* cpu) {
	cpu->a = iop_inr(cpu, cpu->a);
}

// 3d:dcr a
void iop_3d(CPU* cpu) {
	cpu->a = iop_dcr(cpu, cpu->a);
}

// 3e:mvi a,n
void iop_3e(CPU* cpu) {
	cpu->t += 3;
	cpu->a = cpu->mrd(cpu->pc++, 0, cpu->data);
}

// 3f:cmc
void iop_3f(CPU* cpu) {
	cpu->f ^= IFL_C;
}

// 40..47: mov b,x
void iop_40(CPU* cpu) {cpu->b = cpu->b;}
void iop_41(CPU* cpu) {cpu->b = cpu->c;}
void iop_42(CPU* cpu) {cpu->b = cpu->d;}
void iop_43(CPU* cpu) {cpu->b = cpu->e;}
void iop_44(CPU* cpu) {cpu->b = cpu->h;}
void iop_45(CPU* cpu) {cpu->b = cpu->l;}
void iop_46(CPU* cpu) {cpu->t += 3; cpu->b = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_47(CPU* cpu) {cpu->b = cpu->a;}
// 48..4f: mov c,x
void iop_48(CPU* cpu) {cpu->c = cpu->b;}
void iop_49(CPU* cpu) {cpu->c = cpu->c;}
void iop_4a(CPU* cpu) {cpu->c = cpu->d;}
void iop_4b(CPU* cpu) {cpu->c = cpu->e;}
void iop_4c(CPU* cpu) {cpu->c = cpu->h;}
void iop_4d(CPU* cpu) {cpu->c = cpu->l;}
void iop_4e(CPU* cpu) {cpu->t += 3; cpu->c = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_4f(CPU* cpu) {cpu->c = cpu->a;}
// 50..57: mov d,x
void iop_50(CPU* cpu) {cpu->d = cpu->b;}
void iop_51(CPU* cpu) {cpu->d = cpu->c;}
void iop_52(CPU* cpu) {cpu->d = cpu->d;}
void iop_53(CPU* cpu) {cpu->d = cpu->e;}
void iop_54(CPU* cpu) {cpu->d = cpu->h;}
void iop_55(CPU* cpu) {cpu->d = cpu->l;}
void iop_56(CPU* cpu) {cpu->t += 3; cpu->d = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_57(CPU* cpu) {cpu->d = cpu->a;}
// 58..5f: mov e,x
void iop_58(CPU* cpu) {cpu->e = cpu->b;}
void iop_59(CPU* cpu) {cpu->e = cpu->c;}
void iop_5a(CPU* cpu) {cpu->e = cpu->d;}
void iop_5b(CPU* cpu) {cpu->e = cpu->e;}
void iop_5c(CPU* cpu) {cpu->e = cpu->h;}
void iop_5d(CPU* cpu) {cpu->e = cpu->l;}
void iop_5e(CPU* cpu) {cpu->t += 3; cpu->e = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_5f(CPU* cpu) {cpu->e = cpu->a;}
// 60..67: mov h,x
void iop_60(CPU* cpu) {cpu->h = cpu->b;}
void iop_61(CPU* cpu) {cpu->h = cpu->c;}
void iop_62(CPU* cpu) {cpu->h = cpu->d;}
void iop_63(CPU* cpu) {cpu->h = cpu->e;}
void iop_64(CPU* cpu) {cpu->h = cpu->h;}
void iop_65(CPU* cpu) {cpu->h = cpu->l;}
void iop_66(CPU* cpu) {cpu->t += 3; cpu->h = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_67(CPU* cpu) {cpu->h = cpu->a;}
// 68..6f: mov l,x
void iop_68(CPU* cpu) {cpu->l = cpu->b;}
void iop_69(CPU* cpu) {cpu->l = cpu->c;}
void iop_6a(CPU* cpu) {cpu->l = cpu->d;}
void iop_6b(CPU* cpu) {cpu->l = cpu->e;}
void iop_6c(CPU* cpu) {cpu->l = cpu->h;}
void iop_6d(CPU* cpu) {cpu->l = cpu->l;}
void iop_6e(CPU* cpu) {cpu->t += 3; cpu->l = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_6f(CPU* cpu) {cpu->l = cpu->a;}
// 70..77: mov m,x
void iop_70(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->b, cpu->data);}
void iop_71(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->c, cpu->data);}
void iop_72(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->d, cpu->data);}
void iop_73(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->e, cpu->data);}
void iop_74(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->h, cpu->data);}
void iop_75(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->l, cpu->data);}
void iop_76(CPU* cpu) {cpu->pc--; cpu->halt = 1;}
void iop_77(CPU* cpu) {cpu->t += 3; cpu->mwr(cpu->hl, cpu->a, cpu->data);}
// 78..7f: mov a,x
void iop_78(CPU* cpu) {cpu->a = cpu->b;}
void iop_79(CPU* cpu) {cpu->a = cpu->c;}
void iop_7a(CPU* cpu) {cpu->a = cpu->d;}
void iop_7b(CPU* cpu) {cpu->a = cpu->e;}
void iop_7c(CPU* cpu) {cpu->a = cpu->h;}
void iop_7d(CPU* cpu) {cpu->a = cpu->l;}
void iop_7e(CPU* cpu) {cpu->t += 3; cpu->a = cpu->mrd(cpu->hl, 0, cpu->data);}
void iop_7f(CPU* cpu) {cpu->a = cpu->a;}
// 80..87: add x
void iop_80(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->b);}
void iop_81(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->c);}
void iop_82(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->d);}
void iop_83(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->e);}
void iop_84(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->h);}
void iop_85(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->l);}
void iop_86(CPU* cpu) {
	cpu->t += 3;
	cpu->tmpb = cpu->mrd(cpu->hl, 0, cpu->data);
	cpu->a = iop_add(cpu, cpu->a, cpu->tmpb);
}
void iop_87(CPU* cpu) {cpu->a = iop_add(cpu, cpu->a, cpu->a);}
