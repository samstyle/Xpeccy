#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"

extern opCode cbTab[256];
extern opCode edTab[256];
extern opCode ddTab[256];
extern opCode fdTab[256];

void z80_iowr(CPU* cpu, int adr, int data) {
	cpu->iwr(adr, data, cpu->xptr);
	cpu->t += 4;
}

int z80_iord(CPU* cpu, int adr) {
	int v = cpu->ird(adr, cpu->xptr);
	cpu->t += 4;
	return v & 0xff;
}

#if !Z80_NEW_RW_CYCLE
int z80_mrd(CPU* cpu, int a) {
	int v = cpu->mrd(a, 0, cpu->xptr);
	cpu->t += 3;
	return v & 0xff;
}

void z80_mwr(CPU* cpu, int a, int v) {
	cpu->mwr(a, v, cpu->xptr);
	cpu->t += 3;
}
#else
// TODO: use this (somehow...)
int z80_mrd(CPU* cpu, int adr) {
	cpu->adr = adr;
	cpu->t++;		// T1
	do {
		cpu->t++;	// T2 while wait
		cpu->xirq(IRQ_CPU_SYNC, cpu->xptr);
	} while (cpu->wait);
	cpu->t++;		// T3
	return cpu->mrd(adr, 0, cpu->xptr);
}

void z80_mwr(CPU *cpu, int adr, int data) {
	cpu->adr = adr;
	cpu->t++;		// T1
	do {
		cpu->t++;	// T2 while wait
		cpu->xirq(IRQ_CPU_SYNC, cpu->xptr);
	} while (cpu->wait);
	cpu->mwr(adr, data, cpu->xptr);
	cpu->t++;		// T3
}
#endif

// push/pop

unsigned short z80_pop(CPU* cpu) {
	xpair p;
	p.l = z80_mrd(cpu, cpu->sp++);
	p.h = z80_mrd(cpu, cpu->sp++);
	return p.w;
}

void z80_push(CPU* cpu, unsigned short w) {
	xpair p;
	p.w = w;
	z80_mwr(cpu, --cpu->sp, p.h);
	z80_mwr(cpu, --cpu->sp, p.l);
}

void z80_call(CPU* cpu, unsigned short a) {
	cpu->mptr = a;
	z80_push(cpu, cpu->pc);
	cpu->pc = cpu->mptr;
}

void z80_ret(CPU* cpu) {
	cpu->pc = z80_pop(cpu);
	cpu->mptr = cpu->pc;
}

// opcodes
// 00	nop		4
void npr00(CPU* cpu) {}

// 01	ld bc,nn	4 3rd 3rd
void npr01(CPU* cpu) {
	cpu->c = z80_mrd(cpu, cpu->pc++);
	cpu->b = z80_mrd(cpu, cpu->pc++);
}

// 02	ld (bc),a	4 3wr		mptr = (a << 8) | ((bc + 1) & 0xff)
void npr02(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_mwr(cpu, cpu->mptr++, cpu->a);
	cpu->hptr = cpu->a;
}

// 03	inc bc		6
void npr03(CPU* cpu) {
	cpu->bc++;
}

// 04	inc b		4
void npr04(CPU* cpu) {
	cpu->b = z80_inc8(cpu, cpu->b); //	INC(cpu->b);
}

// 05	dec b		4
void npr05(CPU* cpu) {
	cpu->b = z80_dec8(cpu, cpu->b); //	DEC(cpu->b);
}

// 06	ld b,n		4 3rd
void npr06(CPU* cpu) {
	cpu->b = z80_mrd(cpu, cpu->pc++);
}

// 07	rlca		4
void npr07(CPU* cpu) {
	cpu->a = (cpu->a << 1) | (cpu->a >> 7);
	//cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3 | Z80_FC));
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = (cpu->a & 1);
}

// 08	ex af,af'	4
void npr08(CPU* cpu) {
	cpu->ltw = cpu->a; cpu->a = cpu->a_; cpu->a_ = cpu->ltw;
	cpu->tmpi = cpu->f; cpu->f = cpu->f_; cpu->f_ = cpu->tmpi;
}

// 09	add hl,bc	11		mptr = hl+1 before adding
void npr09(CPU* cpu) {
	cpu->hl = z80_add16(cpu, cpu->hl, cpu->bc); //ADD16(cpu->hl, cpu->bc);
}

// 0A	ld a,(bc)	4 3rd		mptr = bc+1
void npr0A(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->a = z80_mrd(cpu, cpu->mptr++);
}

// 0B	dec bc		6
void npr0B(CPU* cpu) {
	cpu->bc--;
}

// 0C	inc c		4
void npr0C(CPU* cpu) {
	cpu->c = z80_inc8(cpu, cpu->c); //	INC(cpu->c);
}

// 0D	dec c		4
void npr0D(CPU* cpu) {
	cpu->c = z80_dec8(cpu, cpu->c);	//	DEC(cpu->c);
}

// 0E	ld c,n		4 3rd
void npr0E(CPU* cpu) {
	cpu->c = z80_mrd(cpu, cpu->pc++);
}

// 0F	rrca		4
void npr0F(CPU* cpu) {
	// cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & Z80_FC);
	cpu->fz.c = (cpu->a & 1);
	cpu->a = (cpu->a >> 1) | (cpu->a << 7);
	// cpu->f |= (cpu->a & (Z80_F5 | Z80_F3));
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 0;
}

// 10	djnz		5 3rd [5jr]
void npr10(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	cpu->b--;
	if (cpu->b) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 11	ld de,nn	4 3rd 3rd
void npr11(CPU* cpu) {
	cpu->e = z80_mrd(cpu, cpu->pc++);
	cpu->d = z80_mrd(cpu, cpu->pc++);
}

// 12	ld (de),a	4 3wr		mptr = (a << 8) | ((de + 1) & 0xff)
void npr12(CPU* cpu) {
	cpu->mptr = cpu->de;
	z80_mwr(cpu, cpu->mptr++, cpu->a);
	cpu->hptr = cpu->a;
}

// 13	inc de		6
void npr13(CPU* cpu) {
	cpu->de++;
}

// 14	inc d		4
void npr14(CPU* cpu) {
	cpu->d = z80_inc8(cpu, cpu->d);	// INC(cpu->d);
}

// 15	dec d		4
void npr15(CPU* cpu) {
	cpu->d = z80_dec8(cpu, cpu->d); // DEC(cpu->d);
}

// 16	ld d,n		4 3rd
void npr16(CPU* cpu) {
	cpu->d = z80_mrd(cpu, cpu->pc++);
}

// 17	rla		4
void npr17(CPU* cpu) {
	cpu->tmp = cpu->a;
	cpu->a = (cpu->a << 1) | cpu->fz.c;
	// cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3)) | (cpu->tmp >> 7);
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = !!(cpu->tmp & 0x80);
}

// 18	jr e		4 3rd 5jr
void npr18(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	cpu->pc += (signed char)cpu->tmp;
	cpu->mptr = cpu->pc;
	cpu->t += 5;
}

// 19	add hl,de	11	mptr = hl+1 before adding
void npr19(CPU* cpu) {
	cpu->hl = z80_add16(cpu, cpu->hl, cpu->de); //ADD16(cpu->hl,cpu->de);
}

// 1A	ld a,(de)	4 3rd	mptr = de + 1
void npr1A(CPU* cpu) {
	cpu->a = z80_mrd(cpu, cpu->de);
	cpu->mptr = cpu->de + 1;
}

// 1B	dec de		6
void npr1B(CPU* cpu) {
	cpu->de--;
}

// 1C	inc e		4
void npr1C(CPU* cpu) {
	cpu->e = z80_inc8(cpu, cpu->e); // INC(cpu->e);
}

// 1D	dec e		4
void npr1D(CPU* cpu) {
	cpu->e = z80_dec8(cpu, cpu->e); // DEC(cpu->e);
}

// 1E	ld e,n		4 3rd
void npr1E(CPU* cpu) {
	cpu->e = z80_mrd(cpu, cpu->pc++);
}

// 1F	rra		4
void npr1F(CPU* cpu) {
	cpu->tmp = cpu->a;
	cpu->a = (cpu->a >> 1) | (cpu->f << 7);
	// cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3)) | (cpu->tmp & Z80_FC);
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = cpu->tmp & 1;
}

// 20	jr nz,e		4 3rd [5jr]
void npr20(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.z) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 21	ld hl,nn	4 3rd 3rd
void npr21(CPU* cpu) {
	cpu->l = z80_mrd(cpu, cpu->pc++);
	cpu->h = z80_mrd(cpu, cpu->pc++);
}

// 22	ld (nn),hl	4 3rd 3rd 3wr 3wr	mptr = nn+1
void npr22(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	z80_mwr(cpu, cpu->mptr++, cpu->l);
	z80_mwr(cpu, cpu->mptr, cpu->h);
}

// 23	inc hl		6
void npr23(CPU* cpu) {
	cpu->hl++;
}

// 24	inc h		4
void npr24(CPU* cpu) {
	cpu->h = z80_inc8(cpu, cpu->h);	// INC(cpu->h);
}

// 25	dec h		4
void npr25(CPU* cpu) {
	cpu->h = z80_dec8(cpu, cpu->h); // DEC(cpu->h);
}

// 26	ld h,n		4 3rd
void npr26(CPU* cpu) {
	cpu->h = z80_mrd(cpu, cpu->pc++);
}

// 27	daa		4
void npr27(CPU* cpu) {
	const unsigned char* tdaa = daaTab + 2 * (cpu->a + 0x100 * ((cpu->f & 3) + ((cpu->f >> 2) & 4)));
	cpu->f = *tdaa;
	cpu->a = *(tdaa + 1);
}

// 28	jr z,e		4 3rd [5jr]
void npr28(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.z) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 29	add hl,hl	11
void npr29(CPU* cpu) {
	cpu->hl = z80_add16(cpu, cpu->hl, cpu->hl); //ADD16(cpu->hl,cpu->hl);
}

// 2A	ld hl,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void npr2A(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	cpu->l = z80_mrd(cpu, cpu->mptr++);
	cpu->h = z80_mrd(cpu, cpu->mptr);
}

// 2B	dec hl		6
void npr2B(CPU* cpu) {
	cpu->hl--;
}

// 2C	inc l		4
void npr2C(CPU* cpu) {
	cpu->l = z80_inc8(cpu, cpu->l); // INC(cpu->l);
}

// 2D	dec l		4
void npr2D(CPU* cpu) {
	cpu->l = z80_dec8(cpu, cpu->l); // DEC(cpu->l);
}

// 2E	ld l,n		4 3rd
void npr2E(CPU* cpu) {
	cpu->l = z80_mrd(cpu, cpu->pc++);
}

// 2F	cpl		4
void npr2F(CPU* cpu) {
	cpu->a ^= 0xff;
//	cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP | Z80_FC)) | (cpu->a & (Z80_F5 | Z80_F3)) | Z80_FH | Z80_FN;
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = 1;
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 1;
}

// 30	jr nc,e		4 3rd [5jr]
void npr30(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.c) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 31	ld sp,nn	4 3rd 3rd
void npr31(CPU* cpu) {
	cpu->lsp = z80_mrd(cpu, cpu->pc++);
	cpu->hsp = z80_mrd(cpu, cpu->pc++);
}

// 32	ld (nn),a	4 3rd 3rd 3wr		mptr = (a << 8) | ((nn + 1) & 0xff)
void npr32(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	z80_mwr(cpu, cpu->mptr++, cpu->a);
	cpu->hptr = cpu->a;
}

// 33	inc sp		6
void npr33(CPU* cpu) {
	cpu->sp++;
}

// 34	inc (hl)	4 3rd 4wr
void npr34(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->hl);
	cpu->tmpb = z80_inc8(cpu, cpu->tmpb); // INC(cpu->tmpb);
	cpu->t++;
	z80_mwr(cpu, cpu->hl, cpu->tmpb);
}

// 35	dec (hl)	4 3rd 4wr
void npr35(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->hl);
	cpu->tmpb = z80_dec8(cpu, cpu->tmpb); // DEC(cpu->tmpb);
	cpu->t++;
	z80_mwr(cpu, cpu->hl, cpu->tmpb);
}

// 36	ld (hl),n	4 3rd 3wr
void npr36(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	z80_mwr(cpu, cpu->hl, cpu->tmp);
}

// 37	scf		4
void npr37(CPU* cpu) {
	//cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | (cpu->a & (Z80_F5 | Z80_F3)) | Z80_FC;
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c = 1;
}

// 38	jr c,e		4 3rd [5jr]
void npr38(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.c) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 39	add hl,sp	11
void npr39(CPU* cpu) {
	cpu->hl = z80_add16(cpu, cpu->hl, cpu->sp); //ADD16(cpu->hl,cpu->sp);
}

// 3A	ld a,(nn)	4 3rd 3rd 3rd	mptr = nn+1
void npr3A(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	cpu->a = z80_mrd(cpu, cpu->mptr++);
}

// 3B	dec sp		6
void npr3B(CPU* cpu) {
	cpu->sp--;
}

// 3C	inc a		4
void npr3C(CPU* cpu) {
	cpu->a = z80_inc8(cpu, cpu->a); // INC(cpu->a);
}

// 3D	dec a		4
void npr3D(CPU* cpu) {
	cpu->a = z80_dec8(cpu, cpu->a); // DEC(cpu->a);
}

// 3E	ld a,n		4 3rd
void npr3E(CPU* cpu) {
	cpu->a = z80_mrd(cpu, cpu->pc++);
}

// 3F	ccf		4
void npr3F(CPU* cpu) {
	//cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FP)) | ((cpu->f & Z80_FC ) ? Z80_FH : Z80_FC) | (cpu->a & (Z80_F5 | Z80_F3));
	cpu->fz.f5 = !!(cpu->a & 0x20);
	cpu->fz.h = cpu->fz.c;		// old C in H
	cpu->fz.f3 = !!(cpu->a & 0x08);
	cpu->fz.n = 0;
	cpu->fz.c ^= 1;
}

// 40..47	ld b,r		4 [3rd]
void npr40(CPU* cpu) {}
void npr41(CPU* cpu) {cpu->b = cpu->c;}
void npr42(CPU* cpu) {cpu->b = cpu->d;}
void npr43(CPU* cpu) {cpu->b = cpu->e;}
void npr44(CPU* cpu) {cpu->b = cpu->h;}
void npr45(CPU* cpu) {cpu->b = cpu->l;}
void npr46(CPU* cpu) {cpu->b = z80_mrd(cpu, cpu->hl);}
void npr47(CPU* cpu) {cpu->b = cpu->a;}
// 48..4f	ld c,r		4 [3rd]
void npr48(CPU* cpu) {cpu->c = cpu->b;}
void npr49(CPU* cpu) {}
void npr4A(CPU* cpu) {cpu->c = cpu->d;}
void npr4B(CPU* cpu) {cpu->c = cpu->e;}
void npr4C(CPU* cpu) {cpu->c = cpu->h;}
void npr4D(CPU* cpu) {cpu->c = cpu->l;}
void npr4E(CPU* cpu) {cpu->c = z80_mrd(cpu, cpu->hl);}
void npr4F(CPU* cpu) {cpu->c = cpu->a;}
// 50..57	ld d,r		4 [3rd]
void npr50(CPU* cpu) {cpu->d = cpu->b;}
void npr51(CPU* cpu) {cpu->d = cpu->c;}
void npr52(CPU* cpu) {}
void npr53(CPU* cpu) {cpu->d = cpu->e;}
void npr54(CPU* cpu) {cpu->d = cpu->h;}
void npr55(CPU* cpu) {cpu->d = cpu->l;}
void npr56(CPU* cpu) {cpu->d = z80_mrd(cpu, cpu->hl);}
void npr57(CPU* cpu) {cpu->d = cpu->a;}
// 58..5f	ld e,r		4 [3rd]
void npr58(CPU* cpu) {cpu->e = cpu->b;}
void npr59(CPU* cpu) {cpu->e = cpu->c;}
void npr5A(CPU* cpu) {cpu->e = cpu->d;}
void npr5B(CPU* cpu) {}
void npr5C(CPU* cpu) {cpu->e = cpu->h;}
void npr5D(CPU* cpu) {cpu->e = cpu->l;}
void npr5E(CPU* cpu) {cpu->e = z80_mrd(cpu, cpu->hl);}
void npr5F(CPU* cpu) {cpu->e = cpu->a;}
// 60..67	ld h,r		4 [3rd]
void npr60(CPU* cpu) {cpu->h = cpu->b;}
void npr61(CPU* cpu) {cpu->h = cpu->c;}
void npr62(CPU* cpu) {cpu->h = cpu->d;}
void npr63(CPU* cpu) {cpu->h = cpu->e;}
void npr64(CPU* cpu) {}
void npr65(CPU* cpu) {cpu->h = cpu->l;}
void npr66(CPU* cpu) {cpu->h = z80_mrd(cpu, cpu->hl);}
void npr67(CPU* cpu) {cpu->h = cpu->a;}
// 68..6f	ld l,r		4 [3rd]
void npr68(CPU* cpu) {cpu->l = cpu->b;}
void npr69(CPU* cpu) {cpu->l = cpu->c;}
void npr6A(CPU* cpu) {cpu->l = cpu->d;}
void npr6B(CPU* cpu) {cpu->l = cpu->e;}
void npr6C(CPU* cpu) {cpu->l = cpu->h;}
void npr6D(CPU* cpu) {}
void npr6E(CPU* cpu) {cpu->l = z80_mrd(cpu, cpu->hl);}
void npr6F(CPU* cpu) {cpu->l = cpu->a;}
// 70..77	ld (hl),r	4 3wr
void npr70(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->b);}
void npr71(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->c);}
void npr72(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->d);}
void npr73(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->e);}
void npr74(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->h);}
void npr75(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->l);}
void npr76(CPU* cpu) {cpu->halt = 1; cpu->pc--;}
void npr77(CPU* cpu) {z80_mwr(cpu, cpu->hl, cpu->a);}
// 78..7f	ld a,r		4 [3rd]
void npr78(CPU* cpu) {cpu->a = cpu->b;}
void npr79(CPU* cpu) {cpu->a = cpu->c;}
void npr7A(CPU* cpu) {cpu->a = cpu->d;}
void npr7B(CPU* cpu) {cpu->a = cpu->e;}
void npr7C(CPU* cpu) {cpu->a = cpu->h;}
void npr7D(CPU* cpu) {cpu->a = cpu->l;}
void npr7E(CPU* cpu) {cpu->a = z80_mrd(cpu, cpu->hl);}
void npr7F(CPU* cpu) {}
// 80..87	add a,r		4 [3rd]
void npr80(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->b, 0);} //ADD(cpu->b);}
void npr81(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->c, 0);} //ADD(cpu->c);}
void npr82(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->d, 0);} //ADD(cpu->d);}
void npr83(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->e, 0);} //ADD(cpu->e);}
void npr84(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->h, 0);} //ADD(cpu->h);}
void npr85(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->l, 0);} //ADD(cpu->l);}
void npr86(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->hl); cpu->a = z80_add8(cpu, cpu->tmpb, 0);} //ADD(cpu->tmpb);}
void npr87(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->a, 0);} //ADD(cpu->a);}
// 88..8F	adc a,r		4 [3rd]
void npr88(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->b, cpu->fz.c);} //ADC(cpu->b);}
void npr89(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->c, cpu->fz.c);} //ADC(cpu->c);}
void npr8A(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->d, cpu->fz.c);} //ADC(cpu->d);}
void npr8B(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->e, cpu->fz.c);} //ADC(cpu->e);}
void npr8C(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->h, cpu->fz.c);} //ADC(cpu->h);}
void npr8D(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->l, cpu->fz.c);} //ADC(cpu->l);}
void npr8E(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->hl); cpu->a = z80_add8(cpu, cpu->tmpb, cpu->fz.c);} //ADC(cpu->tmpb);}
void npr8F(CPU* cpu) {cpu->a = z80_add8(cpu, cpu->a, cpu->fz.c);} //ADC(cpu->a);}
// 90..97	sub r		4 [3rd]
void npr90(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->b, 0);} //SUB(cpu->b);}
void npr91(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->c, 0);} //SUB(cpu->c);}
void npr92(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->d, 0);} //SUB(cpu->d);}
void npr93(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->e, 0);} //SUB(cpu->e);}
void npr94(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->h, 0);} //SUB(cpu->h);}
void npr95(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->l, 0);} //SUB(cpu->l);}
void npr96(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->hl); cpu->a = z80_sub8(cpu, cpu->tmpb, 0);} //SUB(cpu->tmpb);}
void npr97(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->a, 0);} //SUB(cpu->a);}
// 98..9F	sbc a,r		4 [3rd]
void npr98(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->b, cpu->fz.c);} //SBC(cpu->b);}
void npr99(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->c, cpu->fz.c);} //SBC(cpu->c);}
void npr9A(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->d, cpu->fz.c);} //SBC(cpu->d);}
void npr9B(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->e, cpu->fz.c);} //SBC(cpu->e);}
void npr9C(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->h, cpu->fz.c);} //SBC(cpu->h);}
void npr9D(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->l, cpu->fz.c);} //SBC(cpu->l);}
void npr9E(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->hl); cpu->a = z80_sub8(cpu, cpu->tmpb, cpu->fz.c);} //SBC(cpu->tmpb);}
void npr9F(CPU* cpu) {cpu->a = z80_sub8(cpu, cpu->a, cpu->fz.c);} //SBC(cpu->a);}
// a0..a7	and r		4 [3rd]
void nprA0(CPU* cpu) {z80_and8(cpu, cpu->b);}
void nprA1(CPU* cpu) {z80_and8(cpu, cpu->c);}
void nprA2(CPU* cpu) {z80_and8(cpu, cpu->d);}
void nprA3(CPU* cpu) {z80_and8(cpu, cpu->e);}
void nprA4(CPU* cpu) {z80_and8(cpu, cpu->h);}
void nprA5(CPU* cpu) {z80_and8(cpu, cpu->l);}
void nprA6(CPU* cpu) {cpu->tmp = z80_mrd(cpu, cpu->hl); z80_and8(cpu, cpu->tmp);}
void nprA7(CPU* cpu) {z80_and8(cpu, cpu->a);}
// a8..af	xor r		4 [3rd]
void nprA8(CPU* cpu) {z80_xor8(cpu, cpu->b);}
void nprA9(CPU* cpu) {z80_xor8(cpu, cpu->c);}
void nprAA(CPU* cpu) {z80_xor8(cpu, cpu->d);}
void nprAB(CPU* cpu) {z80_xor8(cpu, cpu->e);}
void nprAC(CPU* cpu) {z80_xor8(cpu, cpu->h);}
void nprAD(CPU* cpu) {z80_xor8(cpu, cpu->l);}
void nprAE(CPU* cpu) {cpu->tmp = z80_mrd(cpu, cpu->hl); z80_xor8(cpu, cpu->tmp);}
void nprAF(CPU* cpu) {z80_xor8(cpu, cpu->a);}
// b0..b8	or r		4 [3rd]
void nprB0(CPU* cpu) {z80_or8(cpu, cpu->b);}
void nprB1(CPU* cpu) {z80_or8(cpu, cpu->c);}
void nprB2(CPU* cpu) {z80_or8(cpu, cpu->d);}
void nprB3(CPU* cpu) {z80_or8(cpu, cpu->e);}
void nprB4(CPU* cpu) {z80_or8(cpu, cpu->h);}
void nprB5(CPU* cpu) {z80_or8(cpu, cpu->l);}
void nprB6(CPU* cpu) {cpu->tmp = z80_mrd(cpu, cpu->hl); z80_or8(cpu, cpu->tmp);}
void nprB7(CPU* cpu) {z80_or8(cpu, cpu->a);}
// b9..bf	cp r		4 [3rd]
void nprB8(CPU* cpu) {z80_cp8(cpu, cpu->b);} //CP(cpu->b);}
void nprB9(CPU* cpu) {z80_cp8(cpu, cpu->c);} //CP(cpu->c);}
void nprBA(CPU* cpu) {z80_cp8(cpu, cpu->d);} //CP(cpu->d);}
void nprBB(CPU* cpu) {z80_cp8(cpu, cpu->e);} //CP(cpu->e);}
void nprBC(CPU* cpu) {z80_cp8(cpu, cpu->h);} //CP(cpu->h);}
void nprBD(CPU* cpu) {z80_cp8(cpu, cpu->l);} //CP(cpu->l);}
void nprBE(CPU* cpu) {cpu->tmpb = z80_mrd(cpu, cpu->hl); z80_cp8(cpu, cpu->tmpb);} //CP(cpu->tmpb);}
void nprBF(CPU* cpu) {z80_cp8(cpu, cpu->a);} //CP(cpu->a);}

// c0	ret nz		5 [3rd 3rd]	mptr = ret.adr (if ret)
void nprC0(CPU* cpu) {
	if (!cpu->fz.z)
		z80_ret(cpu);
}

// c1	pop bc		4 3rd 3rd
void nprC1(CPU* cpu) {
	cpu->bc = z80_pop(cpu); // POP(cpu->b,cpu->c);
}

// c2	jp nz,nn	4 3rd 3rd	mptr = nn
void nprC2(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.z) cpu->pc = cpu->mptr;
}

// c3	jp nn		4 3rd 3rd	mptr = nn
void nprC3(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc);
	cpu->pc = cpu->mptr;
}

// c4	call nz,nn	4 3rd 3rd[+1] [3wr 3wr]	mptr = nn
void nprC4(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.z) {
		cpu->t++;
		z80_push(cpu, cpu->pc); //PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// c5	push bc		5 3wr 3wr
void nprC5(CPU* cpu) {
	z80_push(cpu, cpu->bc); // PUSH(cpu->b, cpu->c);
}

// c6	add a,n		4 3rd
void nprC6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	cpu->a = z80_add8(cpu, cpu->tmpb, 0); //ADD(cpu->tmpb);
}

// c7	rst00		5 3wr 3wr	mptr = 0
void nprC7(CPU* cpu) {
	z80_call(cpu, 0x00);
}

// c8	ret z		5 [3rd 3rd]	[mptr = ret.adr]
void nprC8(CPU* cpu) {
	if (cpu->fz.z) z80_ret(cpu);
}

// c9	ret		5 3rd 3rd	mptr = ret.adr
void nprC9(CPU* cpu) {
	z80_ret(cpu);
}

// ca	jp z,nn		4 3rd 3rd	mptr = nn
void nprCA(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.z) cpu->pc = cpu->mptr;
}

// cb	prefix		4
void nprCB(CPU* cpu) {
	cpu->opTab = cbTab;
}

// cc	call z,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprCC(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.z) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// cd	call nn		4 3rd 4rd 3wr 3wr		mptr = nn
void nprCD(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++); cpu->t++;
	z80_push(cpu, cpu->pc);
	cpu->pc = cpu->mptr;
}

// ce	adc a,n		4 3rd
void nprCE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	cpu->a = z80_add8(cpu, cpu->tmpb, cpu->fz.c); //ADC(cpu->tmpb);
}

// cf	rst08		5 3wr 3wr		mptr = 8
void nprCF(CPU* cpu) {
	z80_call(cpu, 0x08);
}

// d0	ret nc		5 [3rd 3rd]		[mptr = ret.adr]
void nprD0(CPU* cpu) {
	if (!cpu->fz.c) z80_ret(cpu);
}

// d1	pop de		4 3rd 3rd
void nprD1(CPU* cpu) {
	cpu->de = z80_pop(cpu);
}

// d2	jp nc,nn	4 3rd 3rd		mptr = nn
void nprD2(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.c) cpu->pc = cpu->mptr;
}

// d3	out(n),a	4 3rd 4out		mptr = (a<<8) | ((n + 1) & 0xff)
void nprD3(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = cpu->a;
	z80_iowr(cpu, cpu->mptr, cpu->a);
	cpu->lptr++;
}

// d4	call nc,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprD4(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.c) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// d5	push de		5 3wr 3wr
void nprD5(CPU* cpu) {
	z80_push(cpu, cpu->de);
}

// d6	sub n		4 3rd
void nprD6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	cpu->a = z80_sub8(cpu, cpu->tmpb, 0); // SUB(cpu->tmpb);
}

// d7	rst10		5 3wr 3wr	mptr = 0x10
void nprD7(CPU* cpu) {
	z80_call(cpu, 0x10);
}

// d8	ret c		5 [3rd 3rd]	[mptr = ret.adr]
void nprD8(CPU* cpu) {
	if (cpu->fz.c) z80_ret(cpu);
}

// d9	exx		4
void nprD9(CPU* cpu) {
	SWAP(cpu->bc,cpu->bc_);
	SWAP(cpu->de,cpu->de_);
	SWAP(cpu->hl,cpu->hl_);
}

// da	jp c,nn		4 3rd 3rd	memptr = nn
void nprDA(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.c) cpu->pc = cpu->mptr;
}

// db	in a,(n)	4 3rd 4in	memptr = ((a<<8) | n) + 1
void nprDB(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = cpu->a;
	cpu->a = z80_iord(cpu, cpu->mptr++);
}

// dc	call c,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprDC(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.c) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// dd	prefix IX	4
void nprDD(CPU* cpu) {
	cpu->opTab = ddTab;
}

// de	sbc a,n		4 3rd
void nprDE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	cpu->a = z80_sub8(cpu, cpu->tmpb, cpu->fz.c); // SBC(cpu->tmpb);
}

// df	rst18		5 3wr 3wr	mptr = 0x18;
void nprDF(CPU* cpu) {
	z80_call(cpu, 0x18);
}

// e0	ret po		5 [3rd 3rd]	[mptr = ret.adr]
void nprE0(CPU* cpu) {
	if (!cpu->fz.pv) z80_ret(cpu);
}

// e1	pop hl		4 3rd 3rd
void nprE1(CPU* cpu) {
	cpu->hl = z80_pop(cpu);
}

// e2	jp po,nn	4 3rd 3rd	mptr = nn
void nprE2(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.pv) cpu->pc = cpu->mptr;
}

// e3	ex (sp),hl	4 3rd 4rd 3wr 5wr	mptr = hl
void nprE3(CPU* cpu) {
	cpu->tmpw = z80_pop(cpu); cpu->t++;	// 3,3+1
	z80_push(cpu, cpu->hl); cpu->t += 2;	// 3,3+2
	cpu->hl = cpu->tmpw;
	cpu->mptr = cpu->hl;
}

// e4	call po,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprE4(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.pv) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// e5	push hl		5 3wr 3wr
void nprE5(CPU* cpu) {
	z80_push(cpu, cpu->hl);
}

// e6	and n		4 3rd
void nprE6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	z80_and8(cpu, cpu->tmpb);
}

// e7	rst20		5 3wr 3wr	mptr = 0x20
void nprE7(CPU* cpu) {
	z80_call(cpu, 0x20);
}

// e8	ret pe		5 [3rd 3rd]	[mptr = ret.adr]
void nprE8(CPU* cpu) {
	if (cpu->fz.pv) z80_ret(cpu);
}

// e9	jp (hl)		4
void nprE9(CPU* cpu) {
	cpu->pc = cpu->hl;
}

// ea	jp pe,nn	4 3rd 3rd	mptr = nn
void nprEA(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.pv) cpu->pc = cpu->mptr;
}

// eb	ex de,hl
void nprEB(CPU* cpu) {
	SWAP(cpu->hl,cpu->de);
}

// ec	call pe,nn	4 3rd 3rd[+1] 3wr 3wr	mptr = nn
void nprEC(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.pv) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// ed	prefix		4
void nprED(CPU* cpu) {
	cpu->opTab = edTab;
}

// ee	xor n		4 3rd
void nprEE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	z80_xor8(cpu, cpu->tmpb);
}

// ef	rst28		5 3wr 3wr	mptr = 0x28
void nprEF(CPU* cpu) {
	z80_call(cpu, 0x28);
}

// f0	ret p		5 [3rd 3rd]	[mptr = ret.adr]
void nprF0(CPU* cpu) {
	if (!cpu->fz.s) z80_ret(cpu);
}

// f1	pop af		4 3rd 3rd
void nprF1(CPU* cpu) {
	cpu->tmpw = z80_pop(cpu);
	cpu->f = cpu->ltw;
	cpu->a = cpu->htw;
}

// f2	jp p,nn		4 3rd 3rd	mptr = nn
void nprF2(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.s) cpu->pc = cpu->mptr;
}

// f3	di		4
void nprF3(CPU* cpu) {
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->inten &= ~Z80_INT;
}

// f4	call p,nn	4 3rd 3rd[+1] [3wr 3wr]		memptr = nn
void nprF4(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (!cpu->fz.s) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// f5	push af		5 3wr 3wr
void nprF5(CPU* cpu) {
	cpu->ltw = cpu->f;
	cpu->htw = cpu->a;
	z80_push(cpu, cpu->tmpw);
}

// f6	or n		4 3rd
void nprF6(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	z80_or8(cpu, cpu->tmpb);
}

// f7	rst30		5 3wr 3wr		mptr = 0x30
void nprF7(CPU* cpu) {
	z80_call(cpu, 0x30);
}

// f8	ret m		5 [3rd 3rd]		[mptr = ret.adr]
void nprF8(CPU* cpu) {
	if (cpu->fz.s) z80_ret(cpu);
}

// f9	ld sp,hl	6
void nprF9(CPU* cpu) {
	cpu->sp = cpu->hl;
}

// fa	jp m,nn		4 3rd 3rd		mptr = nn
void nprFA(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.s) cpu->pc = cpu->mptr;
}

// fb	ei		4
void nprFB(CPU* cpu) {
	cpu->iff1 = 1;
	cpu->iff2 = 1;
	cpu->noint = 1;
	cpu->inten |= Z80_INT;
}

// fc	call m,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprFC(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	if (cpu->fz.s) {
		cpu->t++;
		z80_push(cpu, cpu->pc);
		cpu->pc = cpu->mptr;
	}
}

// fd	prefix IY	4
void nprFD(CPU* cpu) {
	cpu->opTab = fdTab;
}

// fe	cp n		4 3rd
void nprFE(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->pc++);
	z80_cp8(cpu, cpu->tmpb); // CP(cpu->tmpb);
}

// ff	rst38		5 3rd 3rd	mptr = 0x38;
void nprFF(CPU* cpu) {
	z80_call(cpu, 0x38);
}

//==================

opCode npTab[256]={
	{0,4,npr00,NULL,"nop"},
	{0,4,npr01,NULL,"ld bc,:2"},
	{0,4,npr02,NULL,"ld (bc),a"},
	{0,6,npr03,NULL,"inc bc"},
	{0,4,npr04,NULL,"inc b"},
	{0,4,npr05,NULL,"dec b"},
	{0,4,npr06,NULL,"ld b,:1"},
	{0,4,npr07,NULL,"rlca"},

	{0,4,npr08,NULL,"ex af,af'"},
	{0,11,npr09,NULL,"add hl,bc"},
	{0,4,npr0A,NULL,"ld a,(bc)"},
	{0,6,npr0B,NULL,"dec bc"},
	{0,4,npr0C,NULL,"inc c"},
	{0,4,npr0D,NULL,"dec c"},
	{0,4,npr0E,NULL,"ld c,:1"},
	{0,4,npr0F,NULL,"rrca"},

	{OF_SKIPABLE | OF_RELJUMP,5,npr10,NULL,"djnz :3"},
	{0,4,npr11,NULL,"ld de,:2"},
	{0,4,npr12,NULL,"ld (de),a"},
	{0,6,npr13,NULL,"inc de"},
	{0,4,npr14,NULL,"inc d"},
	{0,4,npr15,NULL,"dec d"},
	{0,4,npr16,NULL,"ld d,:1"},
	{0,4,npr17,NULL,"rla"},

	{OF_RELJUMP,4,npr18,NULL,"jr :3"},
	{0,11,npr19,NULL,"add hl,de"},
	{0,4,npr1A,NULL,"ld a,(de)"},
	{0,6,npr1B,NULL,"dec de"},
	{0,4,npr1C,NULL,"inc e"},
	{0,4,npr1D,NULL,"dec e"},
	{0,4,npr1E,NULL,"ld e,:1"},
	{0,4,npr1F,NULL,"rra"},

	{OF_RELJUMP,4,npr20,NULL,"jr nz,:3"},
	{0,4,npr21,NULL,"ld hl,:2"},
	{OF_MWORD | OF_MEMADR,4,npr22,NULL,"ld (:2),hl"},		// 4,3rd,3rd,3wr,3wr
	{0,6,npr23,NULL,"inc hl"},
	{0,4,npr24,NULL,"inc h"},
	{0,4,npr25,NULL,"dec h"},
	{0,4,npr26,NULL,"ld h,:1"},
	{0,4,npr27,NULL,"daa"},

	{OF_RELJUMP,4,npr28,NULL,"jr z,:3"},
	{0,11,npr29,NULL,"add hl,hl"},
	{OF_MWORD | OF_MEMADR,4,npr2A,NULL,"ld hl,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,npr2B,NULL,"dec hl"},
	{0,4,npr2C,NULL,"inc l"},
	{0,4,npr2D,NULL,"dec l"},
	{0,4,npr2E,NULL,"ld l,:1"},
	{0,4,npr2F,NULL,"cpl"},

	{OF_RELJUMP,4,npr30,NULL,"jr nc,:3"},
	{0,4,npr31,NULL,"ld sp,:2"},
	{OF_MEMADR,4,npr32,NULL,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,npr33,NULL,"inc sp"},
	{0,4,npr34,NULL,"inc (hl)"},
	{0,4,npr35,NULL,"dec (hl)"},
	{0,4,npr36,NULL,"ld (hl),:1"},
	{0,4,npr37,NULL,"scf"},

	{OF_RELJUMP,4,npr38,NULL,"jr c,:3"},
	{0,11,npr39,NULL,"add hl,sp"},
	{OF_MEMADR,4,npr3A,NULL,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,npr3B,NULL,"dec sp"},
	{0,4,npr3C,NULL,"inc a"},
	{0,4,npr3D,NULL,"dec a"},
	{0,4,npr3E,NULL,"ld a,:1"},
	{0,4,npr3F,NULL,"ccf"},

	{0,4,npr40,NULL,"ld b,b"},
	{0,4,npr41,NULL,"ld b,c"},
	{0,4,npr42,NULL,"ld b,d"},
	{0,4,npr43,NULL,"ld b,e"},
	{0,4,npr44,NULL,"ld b,h"},
	{0,4,npr45,NULL,"ld b,l"},
	{0,4,npr46,NULL,"ld b,(hl)"},
	{0,4,npr47,NULL,"ld b,a"},

	{0,4,npr48,NULL,"ld c,b"},
	{0,4,npr49,NULL,"ld c,c"},
	{0,4,npr4A,NULL,"ld c,d"},
	{0,4,npr4B,NULL,"ld c,e"},
	{0,4,npr4C,NULL,"ld c,h"},
	{0,4,npr4D,NULL,"ld c,l"},
	{0,4,npr4E,NULL,"ld c,(hl)"},
	{0,4,npr4F,NULL,"ld c,a"},

	{0,4,npr50,NULL,"ld d,b"},
	{0,4,npr51,NULL,"ld d,c"},
	{0,4,npr52,NULL,"ld d,d"},
	{0,4,npr53,NULL,"ld d,e"},
	{0,4,npr54,NULL,"ld d,h"},
	{0,4,npr55,NULL,"ld d,l"},
	{0,4,npr56,NULL,"ld d,(hl)"},
	{0,4,npr57,NULL,"ld d,a"},

	{0,4,npr58,NULL,"ld e,b"},
	{0,4,npr59,NULL,"ld e,c"},
	{0,4,npr5A,NULL,"ld e,d"},
	{0,4,npr5B,NULL,"ld e,e"},
	{0,4,npr5C,NULL,"ld e,h"},
	{0,4,npr5D,NULL,"ld e,l"},
	{0,4,npr5E,NULL,"ld e,(hl)"},
	{0,4,npr5F,NULL,"ld e,a"},

	{0,4,npr60,NULL,"ld h,b"},
	{0,4,npr61,NULL,"ld h,c"},
	{0,4,npr62,NULL,"ld h,d"},
	{0,4,npr63,NULL,"ld h,e"},
	{0,4,npr64,NULL,"ld h,h"},
	{0,4,npr65,NULL,"ld h,l"},
	{0,4,npr66,NULL,"ld h,(hl)"},
	{0,4,npr67,NULL,"ld h,a"},

	{0,4,npr68,NULL,"ld l,b"},
	{0,4,npr69,NULL,"ld l,c"},
	{0,4,npr6A,NULL,"ld l,d"},
	{0,4,npr6B,NULL,"ld l,e"},
	{0,4,npr6C,NULL,"ld l,h"},
	{0,4,npr6D,NULL,"ld l,l"},
	{0,4,npr6E,NULL,"ld l,(hl)"},
	{0,4,npr6F,NULL,"ld l,a"},

	{0,4,npr70,NULL,"ld (hl),b"},
	{0,4,npr71,NULL,"ld (hl),c"},
	{0,4,npr72,NULL,"ld (hl),d"},
	{0,4,npr73,NULL,"ld (hl),e"},
	{0,4,npr74,NULL,"ld (hl),h"},
	{0,4,npr75,NULL,"ld (hl),l"},
	{OF_SKIPABLE,4,npr76,NULL,"halt"},
	{0,4,npr77,NULL,"ld (hl),a"},

	{0,4,npr78,NULL,"ld a,b"},
	{0,4,npr79,NULL,"ld a,c"},
	{0,4,npr7A,NULL,"ld a,d"},
	{0,4,npr7B,NULL,"ld a,e"},
	{0,4,npr7C,NULL,"ld a,h"},
	{0,4,npr7D,NULL,"ld a,l"},
	{0,4,npr7E,NULL,"ld a,(hl)"},
	{0,4,npr7F,NULL,"ld a,a"},

	{0,4,npr80,NULL,"add a,b"},
	{0,4,npr81,NULL,"add a,c"},
	{0,4,npr82,NULL,"add a,d"},
	{0,4,npr83,NULL,"add a,e"},
	{0,4,npr84,NULL,"add a,h"},
	{0,4,npr85,NULL,"add a,l"},
	{0,4,npr86,NULL,"add a,(hl)"},
	{0,4,npr87,NULL,"add a,a"},

	{0,4,npr88,NULL,"adc a,b"},
	{0,4,npr89,NULL,"adc a,c"},
	{0,4,npr8A,NULL,"adc a,d"},
	{0,4,npr8B,NULL,"adc a,e"},
	{0,4,npr8C,NULL,"adc a,h"},
	{0,4,npr8D,NULL,"adc a,l"},
	{0,4,npr8E,NULL,"adc a,(hl)"},
	{0,4,npr8F,NULL,"adc a,a"},

	{0,4,npr90,NULL,"sub b"},
	{0,4,npr91,NULL,"sub c"},
	{0,4,npr92,NULL,"sub d"},
	{0,4,npr93,NULL,"sub e"},
	{0,4,npr94,NULL,"sub h"},
	{0,4,npr95,NULL,"sub l"},
	{0,4,npr96,NULL,"sub (hl)"},
	{0,4,npr97,NULL,"sub a"},

	{0,4,npr98,NULL,"sbc a,b"},
	{0,4,npr99,NULL,"sbc a,c"},
	{0,4,npr9A,NULL,"sbc a,d"},
	{0,4,npr9B,NULL,"sbc a,e"},
	{0,4,npr9C,NULL,"sbc a,h"},
	{0,4,npr9D,NULL,"sbc a,l"},
	{0,4,npr9E,NULL,"sbc a,(hl)"},
	{0,4,npr9F,NULL,"sbc a,a"},

	{0,4,nprA0,NULL,"and b"},
	{0,4,nprA1,NULL,"and c"},
	{0,4,nprA2,NULL,"and d"},
	{0,4,nprA3,NULL,"and e"},
	{0,4,nprA4,NULL,"and h"},
	{0,4,nprA5,NULL,"and l"},
	{0,4,nprA6,NULL,"and (hl)"},
	{0,4,nprA7,NULL,"and a"},

	{0,4,nprA8,NULL,"xor b"},
	{0,4,nprA9,NULL,"xor c"},
	{0,4,nprAA,NULL,"xor d"},
	{0,4,nprAB,NULL,"xor e"},
	{0,4,nprAC,NULL,"xor h"},
	{0,4,nprAD,NULL,"xor l"},
	{0,4,nprAE,NULL,"xor (hl)"},
	{0,4,nprAF,NULL,"xor a"},

	{0,4,nprB0,NULL,"or b"},
	{0,4,nprB1,NULL,"or c"},
	{0,4,nprB2,NULL,"or d"},
	{0,4,nprB3,NULL,"or e"},
	{0,4,nprB4,NULL,"or h"},
	{0,4,nprB5,NULL,"or l"},
	{0,4,nprB6,NULL,"or (hl)"},
	{0,4,nprB7,NULL,"or a"},

	{0,4,nprB8,NULL,"cp b"},
	{0,4,nprB9,NULL,"cp c"},
	{0,4,nprBA,NULL,"cp d"},
	{0,4,nprBB,NULL,"cp e"},
	{0,4,nprBC,NULL,"cp h"},
	{0,4,nprBD,NULL,"cp l"},
	{0,4,nprBE,NULL,"cp (hl)"},
	{0,4,nprBF,NULL,"cp a"},

	{0,5,nprC0,NULL,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,nprC1,NULL,"pop bc"},
	{0,4,nprC2,NULL,"jp nz,:2"},
	{0,4,nprC3,NULL,"jp :2"},
	{OF_SKIPABLE,4,nprC4,NULL,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,nprC5,NULL,"push bc"},		// 5 3wr 3wr
	{0,4,nprC6,NULL,"add a,:1"},
	{OF_SKIPABLE,5,nprC7,NULL,"rst #00"},		// 5 3wr 3wr

	{0,5,nprC8,NULL,"ret z"},
	{0,4,nprC9,NULL,"ret"},
	{0,4,nprCA,NULL,"jp z,:2"},
	{OF_PREFIX,4,nprCB,cbTab,"#CB"},
	{OF_SKIPABLE,4,nprCC,NULL,"call z,:2"},
	{OF_SKIPABLE,4,nprCD,NULL,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,nprCE,NULL,"adc a,:1"},
	{OF_SKIPABLE,5,nprCF,NULL,"rst #08"},

	{0,5,nprD0,NULL,"ret nc"},
	{0,4,nprD1,NULL,"pop de"},
	{0,4,nprD2,NULL,"jp nc,:2"},
	{0,4,nprD3,NULL,"out (:1),a"},
	{OF_SKIPABLE,4,nprD4,NULL,"call nc,:2"},
	{0,5,nprD5,NULL,"push de"},
	{0,4,nprD6,NULL,"sub :1"},
	{OF_SKIPABLE,5,nprD7,NULL,"rst #10"},

	{0,5,nprD8,NULL,"ret c"},
	{0,4,nprD9,NULL,"exx"},
	{0,4,nprDA,NULL,"jp c,:2"},
	{0,4,nprDB,NULL,"in a,(:1)"},
	{OF_SKIPABLE,4,nprDC,NULL,"call c,:2"},
	{OF_PREFIX,4,nprDD,ddTab,"#DD"},
	{0,4,nprDE,NULL,"sbc a,:1"},
	{OF_SKIPABLE,5,nprDF,NULL,"rst #18"},

	{0,5,nprE0,NULL,"ret po"},
	{0,4,nprE1,NULL,"pop hl"},
	{0,4,nprE2,NULL,"jp po,:2"},
	{0,4,nprE3,NULL,"ex (sp),hl"},		// 4 3rd 4rd 3wr 5wr
	{OF_SKIPABLE,4,nprE4,NULL,"call po,:2"},
	{0,5,nprE5,NULL,"push hl"},
	{0,4,nprE6,NULL,"and :1"},
	{OF_SKIPABLE,5,nprE7,NULL,"rst #20"},

	{0,5,nprE8,NULL,"ret pe"},
	{0,4,nprE9,NULL,"jp (hl)"},
	{0,4,nprEA,NULL,"jp pe,:2"},
	{0,4,nprEB,NULL,"ex de,hl"},
	{OF_SKIPABLE,4,nprEC,NULL,"call pe,:2"},
	{OF_PREFIX,4,nprED,edTab,"#ED"},
	{0,4,nprEE,NULL,"xor :1"},
	{OF_SKIPABLE,5,nprEF,NULL,"rst #28"},

	{0,5,nprF0,NULL,"ret p"},
	{0,4,nprF1,NULL,"pop af"},
	{0,4,nprF2,NULL,"jp p,:2"},
	{0,4,nprF3,NULL,"di"},
	{OF_SKIPABLE,4,nprF4,NULL,"call p,:2"},
	{0,5,nprF5,NULL,"push af"},
	{0,4,nprF6,NULL,"or :1"},
	{OF_SKIPABLE,5,nprF7,NULL,"rst #30"},

	{0,5,nprF8,NULL,"ret m"},
	{0,6,nprF9,NULL,"ld sp,hl"},
	{0,4,nprFA,NULL,"jp m,:2"},
	{0,4,nprFB,NULL,"ei"},
	{OF_SKIPABLE,4,nprFC,NULL,"call m,:2"},
	{OF_PREFIX,4,nprFD,fdTab,"#FD"},
	{0,4,nprFE,NULL,"cp :1"},
	{OF_SKIPABLE,5,nprFF,NULL,"rst #38"}
};
