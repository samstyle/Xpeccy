#include <stdlib.h>
#include <stdio.h>
#include "../cpu.h"
#include "lr_macro.h"

extern opCode lrcbTab[256];
extern const unsigned char daaTab[0x1000];

void lr_mwr(CPU* cpu, int a, int v) {
	cpu->mwr(a, v, cpu->xptr);
	cpu->t += 3;
}

int lr_mrd(CPU* cpu, int a) {
	int v = cpu->mrd(a, 0, cpu->xptr);
	cpu->t += 3;
	return v & 0xff;
}

unsigned short lr_pop(CPU* cpu) {
	xpair p;
	p.l = lr_mrd(cpu, cpu->sp++); // MEMRD(cpu->sp++, 3);
	p.h = lr_mrd(cpu, cpu->sp++); // MEMRD(cpu->sp++, 3);
	return p.w;
}

void lr_push(CPU* cpu, unsigned short w) {
	xpair p;
	p.w = w;
	lr_mwr(cpu, --cpu->sp, p.h); // MEMWR(--cpu->sp, p.h, 3);
	lr_mwr(cpu, --cpu->sp, p.l); // MEMWR(--cpu->sp, p.l, 3);
}

// mptr = ret adr
void lr_ret(CPU* cpu) {
	cpu->pc = lr_pop(cpu);
	cpu->mptr = cpu->pc;
}

// mptr = call adr
void lr_call(CPU* cpu, unsigned short a) {
	lr_push(cpu, cpu->pc);
	cpu->mptr = a;
	cpu->pc = a;
}

// alu

unsigned char lr_add_fh[8] = {0, 1, 1, 1, 0, 0, 0, 1};
unsigned char lr_sub_fh[8] = {0, 0, 1, 0, 1, 0, 1, 1};

unsigned char lr_inc8(CPU* cpu, unsigned char v) {
	v++;
	cpu->fl.n = 0;
	cpu->fl.z = !v;
	cpu->fl.h = !(v & 0x0f);
	return v;
}

unsigned char lr_dec8(CPU* cpu, unsigned char v) {
	cpu->fl.h = !(v & 0x0f);
	v--;
	cpu->fl.n = 1;
	cpu->fl.z = !v;
	return v;
}

unsigned char lr_add8(CPU* cpu, unsigned char a, unsigned char v, unsigned char c) {
	c &= cpu->fl.c;
	cpu->tmpw = a + v + c;
	cpu->tmp = ((a & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->fl.c = !!cpu->htw;
	cpu->fl.h = lr_add_fh[cpu->tmp & 7];
	cpu->fl.z = !cpu->ltw;
	cpu->fl.n = 0;
	return cpu->ltw;
}

unsigned char lr_sub8(CPU* cpu, unsigned char a, unsigned char v, unsigned char c) {
	c &= cpu->fl.c;
	cpu->tmpw = a - v - c;
	cpu->tmp = ((a & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->fl.c = !!cpu->htw;
	cpu->fl.h = lr_sub_fh[cpu->tmp & 7];
	cpu->fl.z = !cpu->ltw;
	cpu->fl.n = 1;
	return cpu->ltw;
}

unsigned short lr_add16(CPU* cpu, unsigned short a, unsigned short v) {
	cpu->tmpi = a + v;
	cpu->tmp = ((a & 0x800) >> 11) | ((v & 0x800) >> 10) | ((cpu->tmpi & 0x800) >> 9);\
	cpu->mptr = a + 1;
	cpu->fl.c = (cpu->tmpi > 0xffff);
	cpu->fl.h = lr_add_fh[cpu->tmp & 7];
	cpu->fl.n = 0;
	return cpu->tmpi & 0xffff;
}

void lr_and8(CPU* cpu, unsigned char v) {
	cpu->a &= v;
	cpu->fl.c = 0;
	cpu->fl.n = 0;
	cpu->fl.h = 1;
	cpu->fl.z = !cpu->a;
}

void lr_or8(CPU* cpu, unsigned char v) {
	cpu->a |= v;
	cpu->fl.c = 0;
	cpu->fl.n = 0;
	cpu->fl.h = 0;
	cpu->fl.z = !cpu->a;
}

void lr_xor8(CPU* cpu, unsigned char v) {
	cpu->a ^= v;
	cpu->fl.c = 0;
	cpu->fl.n = 0;
	cpu->fl.h = 0;
	cpu->fl.z = !cpu->a;
}

// opcodes
// 00	nop		4
void lrnop00(CPU* cpu) {}

// 01	ld bc,nn	4 3rd 3rd
void lrnop01(CPU* cpu) {
	cpu->c = lr_mrd(cpu, cpu->pc++);
	cpu->b = lr_mrd(cpu, cpu->pc++);
}

// 02	ld (bc),a	4 3wr		mptr = (a << 8) | ((bc + 1) & 0xff)
void lrnop02(CPU* cpu) {
	cpu->mptr = cpu->bc;
	lr_mwr(cpu, cpu->mptr++, cpu->a);
	cpu->hptr = cpu->a;
}

// 03	inc bc		6
void lrnop03(CPU* cpu) {
	cpu->bc++;
}

// 04	inc b		4
void lrnop04(CPU* cpu) {
	cpu->b = lr_inc8(cpu, cpu->b); //INCL(cpu->b);
}

// 05	dec b		4
void lrnop05(CPU* cpu) {
	cpu->b = lr_dec8(cpu, cpu->b); //DECL(cpu->b);
}

// 06	ld b,n		4 3rd
void lrnop06(CPU* cpu) {
	cpu->b = lr_mrd(cpu, cpu->pc++);
}

// 07	rlca		4
void lrnop07(CPU* cpu) {
	cpu->a = (cpu->a << 1) | (cpu->a >> 7);
	cpu->fl.c = cpu->a & 1;
	cpu->fl.h = 0;
	cpu->fl.n = 0;
	//cpu->f = (cpu->f & FLZ) | ((cpu->a & 1) ? FLC : 0);
}

// 08	ld (nn),sp		4 4rd 4rd 4wr 4wr
void lrnop08(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++); cpu->t++;
	cpu->hptr = lr_mrd(cpu, cpu->pc++); cpu->t++;
	lr_mwr(cpu, cpu->mptr++, cpu->lsp); cpu->t++;
	lr_mwr(cpu, cpu->mptr++, cpu->hsp); cpu->t++;
}

// 09	add hl,bc	11		mptr = hl+1 before adding
void lrnop09(CPU* cpu) {
	cpu->hl = lr_add16(cpu, cpu->hl, cpu->bc); //ADDL16(cpu->hl, cpu->bc);
}

// 0A	ld a,(bc)	4 3rd		mptr = bc+1
void lrnop0A(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->a = lr_mrd(cpu, cpu->mptr++);
}

// 0B	dec bc		6
void lrnop0B(CPU* cpu) {
	cpu->bc--;
}

// 0C	inc c		4
void lrnop0C(CPU* cpu) {
	cpu->c = lr_inc8(cpu, cpu->c); //INCL(cpu->c);
}

// 0D	dec c		4
void lrnop0D(CPU* cpu) {
	cpu->c = lr_dec8(cpu, cpu->c); //DECL(cpu->c);
}

// 0E	ld c,n		4 3rd
void lrnop0E(CPU* cpu) {
	cpu->c = lr_mrd(cpu, cpu->pc++);
}

// 0F	rrca		4
void lrnop0F(CPU* cpu) {
	// cpu->f = (cpu->f & FLZ) | ((cpu->a & 1) ? FLC : 0);
	cpu->fl.n = 0;
	cpu->fl.h = 0;
	cpu->fl.c = cpu->a & 1;
	cpu->a = (cpu->a >> 1) | (cpu->a << 7);
	// cpu->f |= (cpu->a & (F5 | F3));
}

// 10	stop		0
void lrnop10(CPU* cpu) {
	cpu->stop = 1;
	cpu->pc--;
}

// 11	ld de,nn	4 3rd 3rd
void lrnop11(CPU* cpu) {
	cpu->e = lr_mrd(cpu, cpu->pc++);
	cpu->d = lr_mrd(cpu, cpu->pc++);
}

// 12	ld (de),a	4 3wr		mptr = (a << 8) | ((de + 1) & 0xff)
void lrnop12(CPU* cpu) {
	cpu->mptr = cpu->de;
	lr_mwr(cpu, cpu->mptr++, cpu->a);
	cpu->hptr = cpu->a;
}

// 13	inc de		6
void lrnop13(CPU* cpu) {
	cpu->de++;
}

// 14	inc d		4
void lrnop14(CPU* cpu) {
	cpu->d = lr_inc8(cpu, cpu->d); //INCL(cpu->d);
}

// 15	dec d		4
void lrnop15(CPU* cpu) {
	cpu->d = lr_dec8(cpu, cpu->d); //DECL(cpu->d);
}

// 16	ld d,n		4 3rd
void lrnop16(CPU* cpu) {
	cpu->d = lr_mrd(cpu, cpu->pc++);
}

// 17	rla		4
void lrnop17(CPU* cpu) {
	cpu->tmp = cpu->a;
	cpu->a = (cpu->a << 1) | cpu->fl.c;
	//cpu->f = (cpu->f & FLZ) | ((cpu->tmp & 0x80) ? FLC : 0);
	cpu->fl.n = 0;
	cpu->fl.h = 0;
	cpu->fl.c = !!(cpu->tmp & 0x80);
}

// 18	jr e		4 3rd 5jr
void lrnop18(CPU* cpu) {
	cpu->tmp = lr_mrd(cpu, cpu->pc++);
	cpu->pc += (signed char)cpu->tmp;
	cpu->mptr = cpu->pc;
	cpu->t += 5;
}

// 19	add hl,de	11	mptr = hl+1 before adding
void lrnop19(CPU* cpu) {
	cpu->hl = lr_add16(cpu, cpu->hl, cpu->de); //ADDL16(cpu->hl,cpu->de);
}

// 1A	ld a,(de)	4 3rd	mptr = de + 1
void lrnop1A(CPU* cpu) {
	cpu->a = lr_mrd(cpu, cpu->de);
	cpu->mptr = cpu->de + 1;
}

// 1B	dec de		6
void lrnop1B(CPU* cpu) {
	cpu->de--;
}

// 1C	inc e		4
void lrnop1C(CPU* cpu) {
	cpu->e = lr_inc8(cpu, cpu->e); //INCL(cpu->e);
}

// 1D	dec e		4
void lrnop1D(CPU* cpu) {
	cpu->e = lr_dec8(cpu, cpu->e); //DECL(cpu->e);
}

// 1E	ld e,n		4 3rd
void lrnop1E(CPU* cpu) {
	cpu->e = lr_mrd(cpu, cpu->pc++);
}

// 1F	rra		4
void lrnop1F(CPU* cpu) {
	cpu->tmp = cpu->a;
	cpu->a = (cpu->a >> 1) | ((cpu->fl.c) ? 0x80 : 0);
	//cpu->f = (cpu->f & FLZ) | ((cpu->tmp & 1) ? FLC : 0);
	cpu->fl.n = 0;
	cpu->fl.h = 0;
	cpu->fl.c = cpu->tmp & 1;
}

// 20	jr nz,e		4 3rd [5jr]
void lrnop20(CPU* cpu) {
	cpu->tmp = lr_mrd(cpu, cpu->pc++);
	if (!cpu->fl.z) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 21	ld hl,nn	4 3rd 3rd
void lrnop21(CPU* cpu) {
	cpu->l = lr_mrd(cpu, cpu->pc++);
	cpu->h = lr_mrd(cpu, cpu->pc++);
}

// 22	ldi (hl),a		4 4wr
void lrnop22(CPU* cpu) {
	lr_mwr(cpu, cpu->hl++, cpu->a); cpu->t++;
}

// 23	inc hl		6
void lrnop23(CPU* cpu) {
	cpu->hl++;
}

// 24	inc h		4
void lrnop24(CPU* cpu) {
	cpu->h = lr_inc8(cpu, cpu->h); //INCL(cpu->h);
}

// 25	dec h		4
void lrnop25(CPU* cpu) {
	cpu->h = lr_dec8(cpu, cpu->h); //DECL(cpu->h);
}

// 26	ld h,n		4 3rd
void lrnop26(CPU* cpu) {
	cpu->h = lr_mrd(cpu, cpu->pc++);
}

// 27	daa		4
void lrnop27(CPU* cpu) {
	const unsigned char* tdaa = daaTab + 2 * (cpu->a + 0x100 * ((cpu->fl.c ? 1 : 0) | (cpu->fl.n ? 2 : 0) | (cpu->fl.h ? 4 : 0)));
	cpu->tmp = *tdaa;			// this is z80 flag
	cpu->a = *(tdaa + 1);
	cpu->fl.z = !!(cpu->tmp & 0x40);	// convert z80 flag to lr35902 flag
	cpu->fl.n = !!(cpu->tmp & 0x02);
	cpu->fl.h = !!(cpu->tmp & 0x10);
	cpu->fl.c = !!(cpu->tmp & 0x01);
}

// 28	jr z,e		4 3rd [5jr]
void lrnop28(CPU* cpu) {
	cpu->tmp = lr_mrd(cpu, cpu->pc++);
	if (cpu->fl.z) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 29	add hl,hl	11
void lrnop29(CPU* cpu) {
	cpu->hl = lr_add16(cpu, cpu->hl, cpu->hl); //ADDL16(cpu->hl,cpu->hl);
}

// 2A	ldi a,(hl)	4 4rd
void lrnop2A(CPU* cpu) {
	cpu->a = lr_mrd(cpu, cpu->hl++); cpu->t++;
}

// 2B	dec hl		6
void lrnop2B(CPU* cpu) {
	cpu->hl--;
}

// 2C	inc l		4
void lrnop2C(CPU* cpu) {
	cpu->l = lr_inc8(cpu, cpu->l); //INCL(cpu->l);
}

// 2D	dec l		4
void lrnop2D(CPU* cpu) {
	cpu->l = lr_dec8(cpu, cpu->l); //DECL(cpu->l);
}

// 2E	ld l,n		4 3rd
void lrnop2E(CPU* cpu) {
	cpu->l = lr_mrd(cpu, cpu->pc++);
}

// 2F	cpl		4
void lrnop2F(CPU* cpu) {
	cpu->a ^= 0xff;
	//cpu->f = (cpu->f & (FLZ | FLC)) | FLH | FLN;
	cpu->fl.h = 1;
	cpu->fl.n = 1;
}

// 30	jr nc,e		4 3rd [5jr]
void lrnop30(CPU* cpu) {
	cpu->tmp = lr_mrd(cpu, cpu->pc++);
	if (!cpu->fl.c) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 31	ld sp,nn	4 3rd 3rd
void lrnop31(CPU* cpu) {
	cpu->lsp = lr_mrd(cpu, cpu->pc++);
	cpu->hsp = lr_mrd(cpu, cpu->pc++);
}

// 32	ldd (hl),a		4 4wr
void lrnop32(CPU* cpu) {
	lr_mwr(cpu, cpu->hl--, cpu->a); cpu->t++;
}

// 33	inc sp		6
void lrnop33(CPU* cpu) {
	cpu->sp++;
}

// 34	inc (hl)	4 3rd 4wr
void lrnop34(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->hl);
	cpu->tmpb = lr_inc8(cpu, cpu->tmpb); //INCL(cpu->tmpb);
	cpu->t++;
	lr_mwr(cpu, cpu->hl, cpu->tmpb);
}

// 35	dec (hl)	4 3rd 4wr
void lrnop35(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->hl);
	cpu->tmpb = lr_dec8(cpu, cpu->tmpb); //DECL(cpu->tmpb);
	cpu->t++;
	lr_mwr(cpu, cpu->hl, cpu->tmpb);
}

// 36	ld (hl),n	4 3rd 3wr
void lrnop36(CPU* cpu) {
	cpu->tmp = lr_mrd(cpu, cpu->pc++);
	lr_mwr(cpu, cpu->hl, cpu->tmp);
}

// 37	scf		4
void lrnop37(CPU* cpu) {
	cpu->fl.c = 1;
}

// 38	jr c,e		4 3rd [5jr]
void lrnop38(CPU* cpu) {
	cpu->tmp = lr_mrd(cpu, cpu->pc++);
	if (cpu->fl.c) {
		cpu->pc += (signed char)cpu->tmp;
		cpu->mptr = cpu->pc;
		cpu->t += 5;
	}
}

// 39	add hl,sp	11
void lrnop39(CPU* cpu) {
	cpu->hl = lr_add16(cpu, cpu->hl, cpu->sp); //ADDL16(cpu->hl,cpu->sp);
}

// 3A	ldd a,(hl)	4 4rd
void lrnop3A(CPU* cpu) {
	cpu->a = lr_mrd(cpu, cpu->hl--);
	cpu->t++;
}

// 3B	dec sp		6
void lrnop3B(CPU* cpu) {
	cpu->sp--;
}

// 3C	inc a		4
void lrnop3C(CPU* cpu) {
	cpu->a = lr_inc8(cpu, cpu->a); //INCL(cpu->a);
}

// 3D	dec a		4
void lrnop3D(CPU* cpu) {
	cpu->a = lr_dec8(cpu, cpu->a); //DECL(cpu->a);
}

// 3E	ld a,n		4 3rd
void lrnop3E(CPU* cpu) {
	cpu->a = lr_mrd(cpu, cpu->pc++);
}

// 3F	ccf		4
void lrnop3F(CPU* cpu) {
	//cpu->f = (cpu->f & FLZ) | ((cpu->f & FLC ) ? FLH : FLC);
	cpu->fl.h = cpu->fl.c;
	cpu->fl.c ^= 1;
}

// 40..47	ld b,r		4 [3rd]
void lrnop40(CPU* cpu) {}
void lrnop41(CPU* cpu) {cpu->b = cpu->c;}
void lrnop42(CPU* cpu) {cpu->b = cpu->d;}
void lrnop43(CPU* cpu) {cpu->b = cpu->e;}
void lrnop44(CPU* cpu) {cpu->b = cpu->h;}
void lrnop45(CPU* cpu) {cpu->b = cpu->l;}
void lrnop46(CPU* cpu) {cpu->b = lr_mrd(cpu,cpu->hl);}
void lrnop47(CPU* cpu) {cpu->b = cpu->a;}
// 48..4f	ld c,r		4 [3rd]
void lrnop48(CPU* cpu) {cpu->c = cpu->b;}
void lrnop49(CPU* cpu) {}
void lrnop4A(CPU* cpu) {cpu->c = cpu->d;}
void lrnop4B(CPU* cpu) {cpu->c = cpu->e;}
void lrnop4C(CPU* cpu) {cpu->c = cpu->h;}
void lrnop4D(CPU* cpu) {cpu->c = cpu->l;}
void lrnop4E(CPU* cpu) {cpu->c = lr_mrd(cpu,cpu->hl);}
void lrnop4F(CPU* cpu) {cpu->c = cpu->a;}
// 50..57	ld d,r		4 [3rd]
void lrnop50(CPU* cpu) {cpu->d = cpu->b;}
void lrnop51(CPU* cpu) {cpu->d = cpu->c;}
void lrnop52(CPU* cpu) {}
void lrnop53(CPU* cpu) {cpu->d = cpu->e;}
void lrnop54(CPU* cpu) {cpu->d = cpu->h;}
void lrnop55(CPU* cpu) {cpu->d = cpu->l;}
void lrnop56(CPU* cpu) {cpu->d = lr_mrd(cpu,cpu->hl);}
void lrnop57(CPU* cpu) {cpu->d = cpu->a;}
// 58..5f	ld e,r		4 [3rd]
void lrnop58(CPU* cpu) {cpu->e = cpu->b;}
void lrnop59(CPU* cpu) {cpu->e = cpu->c;}
void lrnop5A(CPU* cpu) {cpu->e = cpu->d;}
void lrnop5B(CPU* cpu) {}
void lrnop5C(CPU* cpu) {cpu->e = cpu->h;}
void lrnop5D(CPU* cpu) {cpu->e = cpu->l;}
void lrnop5E(CPU* cpu) {cpu->e = lr_mrd(cpu,cpu->hl);}
void lrnop5F(CPU* cpu) {cpu->e = cpu->a;}
// 60..67	ld h,r		4 [3rd]
void lrnop60(CPU* cpu) {cpu->h = cpu->b;}
void lrnop61(CPU* cpu) {cpu->h = cpu->c;}
void lrnop62(CPU* cpu) {cpu->h = cpu->d;}
void lrnop63(CPU* cpu) {cpu->h = cpu->e;}
void lrnop64(CPU* cpu) {}
void lrnop65(CPU* cpu) {cpu->h = cpu->l;}
void lrnop66(CPU* cpu) {cpu->h = lr_mrd(cpu,cpu->hl);}
void lrnop67(CPU* cpu) {cpu->h = cpu->a;}
// 68..6f	ld l,r		4 [3rd]
void lrnop68(CPU* cpu) {cpu->l = cpu->b;}
void lrnop69(CPU* cpu) {cpu->l = cpu->c;}
void lrnop6A(CPU* cpu) {cpu->l = cpu->d;}
void lrnop6B(CPU* cpu) {cpu->l = cpu->e;}
void lrnop6C(CPU* cpu) {cpu->l = cpu->h;}
void lrnop6D(CPU* cpu) {}
void lrnop6E(CPU* cpu) {cpu->l = lr_mrd(cpu,cpu->hl);}
void lrnop6F(CPU* cpu) {cpu->l = cpu->a;}
// 70..77	ld (hl),r	4 3wr
void lrnop70(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->b);}
void lrnop71(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->c);}
void lrnop72(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->d);}
void lrnop73(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->e);}
void lrnop74(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->h);}
void lrnop75(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->l);}
void lrnop76(CPU* cpu) {cpu->halt = 1; cpu->pc--;}
void lrnop77(CPU* cpu) {lr_mwr(cpu, cpu->hl,cpu->a);}
// 78..7f	ld a,r		4 [3rd]
void lrnop78(CPU* cpu) {cpu->a = cpu->b;}
void lrnop79(CPU* cpu) {cpu->a = cpu->c;}
void lrnop7A(CPU* cpu) {cpu->a = cpu->d;}
void lrnop7B(CPU* cpu) {cpu->a = cpu->e;}
void lrnop7C(CPU* cpu) {cpu->a = cpu->h;}
void lrnop7D(CPU* cpu) {cpu->a = cpu->l;}
void lrnop7E(CPU* cpu) {cpu->a = lr_mrd(cpu, cpu->hl);}
void lrnop7F(CPU* cpu) {}
// 80..87	add a,r		4 [3rd]
void lrnop80(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->b, 0);} //ADDL(cpu->b);}
void lrnop81(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->c, 0);} //ADDL(cpu->c);}
void lrnop82(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->d, 0);} //ADDL(cpu->d);}
void lrnop83(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->e, 0);} //ADDL(cpu->e);}
void lrnop84(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->h, 0);} //ADDL(cpu->h);}
void lrnop85(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->l, 0);} //ADDL(cpu->l);}
void lrnop86(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->hl); cpu->a = lr_add8(cpu, cpu->a, cpu->tmpb, 0);} //ADDL(cpu->tmpb);}
void lrnop87(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->a, 0);} //ADDL(cpu->a);}
// 88..8F	adc a,r		4 [3rd]
void lrnop88(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->b, 1);} //ADCL(cpu->b);}
void lrnop89(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->c, 1);} //ADCL(cpu->c);}
void lrnop8A(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->d, 1);} //ADCL(cpu->d);}
void lrnop8B(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->e, 1);} //ADCL(cpu->e);}
void lrnop8C(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->h, 1);} //ADCL(cpu->h);}
void lrnop8D(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->l, 1);} //ADCL(cpu->l);}
void lrnop8E(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->hl); cpu->a = lr_add8(cpu, cpu->a, cpu->tmpb, 1);} //ADCL(cpu->tmpb);}
void lrnop8F(CPU* cpu) {cpu->a = lr_add8(cpu, cpu->a, cpu->a, 1);} //ADCL(cpu->a);}
// 90..97	sub r		4 [3rd]
void lrnop90(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->b, 0);} //SUBL(cpu->b);}
void lrnop91(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->c, 0);} //SUBL(cpu->c);}
void lrnop92(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->d, 0);} //SUBL(cpu->d);}
void lrnop93(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->e, 0);} //SUBL(cpu->e);}
void lrnop94(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->h, 0);} //SUBL(cpu->h);}
void lrnop95(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->l, 0);} //SUBL(cpu->l);}
void lrnop96(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->hl); cpu->a = lr_sub8(cpu, cpu->a, cpu->tmpb, 0);} //SUBL(cpu->tmpb);}
void lrnop97(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->a, 0);} //SUBL(cpu->a);}
// 98..9F	sbc a,r		4 [3rd]
void lrnop98(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->b, 1);} //SBCL(cpu->b);}
void lrnop99(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->c, 1);} //SBCL(cpu->c);}
void lrnop9A(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->d, 1);} //SBCL(cpu->d);}
void lrnop9B(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->e, 1);} //SBCL(cpu->e);}
void lrnop9C(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->h, 1);} //SBCL(cpu->h);}
void lrnop9D(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->l, 1);} //SBCL(cpu->l);}
void lrnop9E(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->hl); cpu->a = lr_sub8(cpu, cpu->a, cpu->tmpb, 1);} //SBCL(cpu->tmpb);}
void lrnop9F(CPU* cpu) {cpu->a = lr_sub8(cpu, cpu->a, cpu->a, 1);} //SBCL(cpu->a);}
// a0..a7	and r		4 [3rd]
void lrnopA0(CPU* cpu) {lr_and8(cpu, cpu->b);}
void lrnopA1(CPU* cpu) {lr_and8(cpu, cpu->c);}
void lrnopA2(CPU* cpu) {lr_and8(cpu, cpu->d);}
void lrnopA3(CPU* cpu) {lr_and8(cpu, cpu->e);}
void lrnopA4(CPU* cpu) {lr_and8(cpu, cpu->h);}
void lrnopA5(CPU* cpu) {lr_and8(cpu, cpu->l);}
void lrnopA6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->hl); lr_and8(cpu, cpu->tmp);}
void lrnopA7(CPU* cpu) {lr_and8(cpu, cpu->a);}
// a8..af	xor r		4 [3rd]
void lrnopA8(CPU* cpu) {lr_xor8(cpu, cpu->b);}
void lrnopA9(CPU* cpu) {lr_xor8(cpu, cpu->c);}
void lrnopAA(CPU* cpu) {lr_xor8(cpu, cpu->d);}
void lrnopAB(CPU* cpu) {lr_xor8(cpu, cpu->e);}
void lrnopAC(CPU* cpu) {lr_xor8(cpu, cpu->h);}
void lrnopAD(CPU* cpu) {lr_xor8(cpu, cpu->l);}
void lrnopAE(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->hl); lr_xor8(cpu, cpu->tmp);}
void lrnopAF(CPU* cpu) {lr_xor8(cpu, cpu->a);}
// b0..b8	or r		4 [3rd]
void lrnopB0(CPU* cpu) {lr_or8(cpu, cpu->b);}
void lrnopB1(CPU* cpu) {lr_or8(cpu, cpu->c);}
void lrnopB2(CPU* cpu) {lr_or8(cpu, cpu->d);}
void lrnopB3(CPU* cpu) {lr_or8(cpu, cpu->e);}
void lrnopB4(CPU* cpu) {lr_or8(cpu, cpu->h);}
void lrnopB5(CPU* cpu) {lr_or8(cpu, cpu->l);}
void lrnopB6(CPU* cpu) {cpu->tmp = lr_mrd(cpu, cpu->hl); lr_or8(cpu, cpu->tmp);}
void lrnopB7(CPU* cpu) {lr_or8(cpu, cpu->a);}
// b9..bf	cp r		4 [3rd]
void lrnopB8(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->b, 0);} //CMP(cpu->b);}
void lrnopB9(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->c, 0);} //CMP(cpu->c);}
void lrnopBA(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->d, 0);} //CMP(cpu->d);}
void lrnopBB(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->e, 0);} //CMP(cpu->e);}
void lrnopBC(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->h, 0);} //CMP(cpu->h);}
void lrnopBD(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->l, 0);} //CMP(cpu->l);}
void lrnopBE(CPU* cpu) {cpu->tmpb = lr_mrd(cpu, cpu->hl); cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->tmpb, 0);} //CMP(cpu->tmpb);}
void lrnopBF(CPU* cpu) {cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->a, 0);} //CMP(cpu->a);}

// c0	ret nz		5 [3rd 3rd]	mptr = ret.adr (if ret)
void lrnopC0(CPU* cpu) {
	if (!cpu->fl.z) lr_ret(cpu);
}

// c1	pop bc		4 3rd 3rd
void lrnopC1(CPU* cpu) {
	cpu->bc = lr_pop(cpu); // POP(cpu->b,cpu->c);
}

// c2	jp nz,nn	4 3rd 3rd	mptr = nn
void lrnopC2(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (!cpu->fl.z) cpu->pc = cpu->mptr;
}

// c3	jp nn		4 3rd 3rd	mptr = nn
void lrnopC3(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu,cpu->pc++);
	cpu->hptr = lr_mrd(cpu,cpu->pc);
	cpu->pc = cpu->mptr;
}

// c4	call nz,nn	4 3rd 3rd[+1] [3wr 3wr]	mptr = nn
void lrnopC4(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu,cpu->pc++);
	cpu->hptr = lr_mrd(cpu,cpu->pc++);
	if (!cpu->fl.z) {
		cpu->t++;
		lr_push(cpu, cpu->pc); // PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// c5	push bc		5 3wr 3wr
void lrnopC5(CPU* cpu) {
	lr_push(cpu, cpu->bc); //PUSH(cpu->b, cpu->c);
}

// c6	add a,n		4 3rd
void lrnopC6(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->a = lr_add8(cpu, cpu->a, cpu->tmpb, 0); //ADDL(cpu->tmpb);
}

// c7	rst00		5 3wr 3wr	mptr = 0
void lrnopC7(CPU* cpu) {
	lr_call(cpu, 0x00);
}

// c8	ret z		5 [3rd 3rd]	[mptr = ret.adr]
void lrnopC8(CPU* cpu) {
	if (cpu->fl.z) lr_ret(cpu);
}

// c9	ret		5 3rd 3rd	mptr = ret.adr
void lrnopC9(CPU* cpu) {
	lr_ret(cpu);
}

// ca	jp z,nn		4 3rd 3rd	mptr = nn
void lrnopCA(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (cpu->fl.z) cpu->pc = cpu->mptr;
}

// cb	prefix		4
void lrnopCB(CPU* cpu) {
	cpu->opTab = lrcbTab;
}

// cc	call z,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void lrnopCC(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (cpu->fl.z) {
		cpu->t++;
		lr_push(cpu, cpu->pc); // PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// cd	call nn		4 3rd 4rd 3wr 3wr		mptr = nn
void lrnopCD(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	lr_push(cpu, cpu->pc); //PUSH(cpu->hpc,cpu->lpc);
	cpu->pc = cpu->mptr;
}

// ce	adc a,n		4 3rd
void lrnopCE(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->a = lr_add8(cpu, cpu->a, cpu->tmpb, 1); //ADCL(cpu->tmpb);
}

// cf	rst08		5 3wr 3wr		mptr = 8
void lrnopCF(CPU* cpu) {
	lr_call(cpu, 0x08);
}

// d0	ret nc		5 [3rd 3rd]		[mptr = ret.adr]
void lrnopD0(CPU* cpu) {
	if (!cpu->fl.c) lr_ret(cpu);
}

// d1	pop de		4 3rd 3rd
void lrnopD1(CPU* cpu) {
	cpu->de = lr_pop(cpu); // POP(cpu->d,cpu->e);
}

// d2	jp nc,nn	4 3rd 3rd		mptr = nn
void lrnopD2(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (!cpu->fl.c) cpu->pc = cpu->mptr;
}

// d4	call nc,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void lrnopD4(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (!cpu->fl.c) {
		cpu->t++;
		lr_push(cpu, cpu->pc); //PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// d5	push de		5 3wr 3wr
void lrnopD5(CPU* cpu) {
	lr_push(cpu, cpu->de); // PUSH(cpu->d,cpu->e);
}

// d6	sub n		4 3rd
void lrnopD6(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->a = lr_sub8(cpu, cpu->a, cpu->tmpb, 0); //SUBL(cpu->tmpb);
}

// d7	rst10		5 3wr 3wr	mptr = 0x10
void lrnopD7(CPU* cpu) {
	lr_call(cpu, 0x10);
}

// d8	ret c		5 [3rd 3rd]	[mptr = ret.adr]
void lrnopD8(CPU* cpu) {
	if (cpu->fl.c) lr_ret(cpu);
}

// d9	reti		10? 3rd 3rd
void lrnopD9(CPU* cpu) {
	cpu->iff1 = 1;
	lr_ret(cpu);
}

// da	jp c,nn		4 3rd 3rd	memptr = nn
void lrnopDA(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (cpu->fl.c) cpu->pc = cpu->mptr;
}

// dc	call c,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void lrnopDC(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	if (cpu->fl.c) {
		cpu->t++;
		lr_push(cpu, cpu->pc); // PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// de	sbc a,n		4 3rd
void lrnopDE(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->a = lr_sub8(cpu, cpu->a, cpu->tmpb, 1); //SBCL(cpu->tmpb);
}

// df	rst18		5 3wr 3wr	mptr = 0x18;
void lrnopDF(CPU* cpu) {
	lr_call(cpu, 0x18);
}

// e0	ld (FF00 + n), a
void lrnopE0(CPU* cpu) {
	cpu->ltw = lr_mrd(cpu, cpu->pc++);
	cpu->htw = 0xff;
	cpu->t++;
	lr_mwr(cpu, cpu->tmpw, cpu->a);
	cpu->t++;
}

// e1	pop hl		4 3rd 3rd
void lrnopE1(CPU* cpu) {
	cpu->hl = lr_pop(cpu); // POP(cpu->h, cpu->l);
}

// e2	ld (FF00 + C), a
void lrnopE2(CPU* cpu) {
	cpu->ltw = cpu->c;
	cpu->htw = 0xff;
	lr_mwr(cpu, cpu->tmpw, cpu->a);
	cpu->t++;
}

// e5	push hl		5 3wr 3wr
void lrnopE5(CPU* cpu) {
	lr_push(cpu, cpu->hl); // PUSH(cpu->h, cpu->l);
}

// e6	and n		4 3rd
void lrnopE6(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	lr_and8(cpu, cpu->tmpb);
}

// e7	rst20		5 3wr 3wr	mptr = 0x20
void lrnopE7(CPU* cpu) {
	lr_call(cpu, 0x20);
}

// e8	add sp,e	???
void lrnopE8(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	cpu->sp += (signed char)cpu->tmpb;
}

// e9	jp (hl)		4
void lrnopE9(CPU* cpu) {
	cpu->pc = cpu->hl;
}

// ea	ld (nn),a	4 4rd 4rd 4wr
void lrnopEA(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	lr_mwr(cpu, cpu->mptr, cpu->a);
	cpu->t++;
}

// ee	xor n		4 3rd
void lrnopEE(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	lr_xor8(cpu, cpu->tmpb);
}

// ef	rst28		5 3wr 3wr	mptr = 0x28
void lrnopEF(CPU* cpu) {
	lr_call(cpu, 0x28);
}

// f0	ld a,(FF00 + n)
void lrnopF0(CPU* cpu) {
	cpu->ltw = lr_mrd(cpu, cpu->pc++);
	cpu->htw = 0xff;
	cpu->t++;
	cpu->a = lr_mrd(cpu, cpu->tmpw);
	cpu->t++;
}

// f1	pop af		4 3rd 3rd
void lrnopF1(CPU* cpu) {
	cpu->tmpw = lr_pop(cpu); //POP(cpu->a,cpu->f);
	cpu->a = cpu->htw;
	cpu->f = cpu->ltw;
}

// f2	ld a,(FF00 + C)
void lrnopF2(CPU* cpu) {
	cpu->ltw = cpu->c;
	cpu->htw = 0xff;
	cpu->a = lr_mrd(cpu, cpu->tmpw);
	cpu->t++;
}

// f3	di		4
void lrnopF3(CPU* cpu) {
	cpu->iff1 = 0;
}

// f5	push af		5 3wr 3wr
void lrnopF5(CPU* cpu) {
	// PUSH(cpu->a,cpu->f);
	cpu->ltw = cpu->f;
	cpu->htw = cpu->a;
	lr_push(cpu, cpu->tmpw);
}

// f6	or n		4 3rd
void lrnopF6(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	lr_or8(cpu, cpu->tmpb);
}

// f7	rst30		5 3wr 3wr		mptr = 0x30
void lrnopF7(CPU* cpu) {
	lr_call(cpu, 0x30);
}

// f8	ld hl, sp + e	???
void lrnopF8(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	cpu->hl = cpu->sp + (signed char)cpu->tmpb;
}

// f9	ld sp,hl	6
void lrnopF9(CPU* cpu) {
	cpu->sp = cpu->hl;
}

// fa	ld a,(nn)	4 4rd 4rd 4rd
void lrnopFA(CPU* cpu) {
	cpu->lptr = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	cpu->hptr = lr_mrd(cpu, cpu->pc++);
	cpu->t++;
	cpu->a = lr_mrd(cpu, cpu->mptr);
	cpu->t++;
}

// fb	ei		4
void lrnopFB(CPU* cpu) {
	cpu->iff1 = 1;
//	cpu->noint = 1;
}

// fe	cp n		4 3rd
void lrnopFE(CPU* cpu) {
	cpu->tmpb = lr_mrd(cpu, cpu->pc++);
	cpu->tmpb = lr_sub8(cpu, cpu->a, cpu->tmpb, 0); // CMP(cpu->tmpb);
}

// ff	rst38		5 3rd 3rd	mptr = 0x38;
void lrnopFF(CPU* cpu) {
	lr_call(cpu, 0x38);
}

// any missing opcode : lock-up CPU
void lrnLock(CPU* cpu) {
	cpu->lock = 1;
	cpu->pc--;
}

//==================

opCode lrTab[256]={
	{0,4,lrnop00,NULL,"nop"},
	{0,4,lrnop01,NULL,"ld bc,:2"},
	{0,4,lrnop02,NULL,"ld (bc),a"},
	{0,6,lrnop03,NULL,"inc bc"},
	{0,4,lrnop04,NULL,"inc b"},
	{0,4,lrnop05,NULL,"dec b"},
	{0,4,lrnop06,NULL,"ld b,:1"},
	{0,4,lrnop07,NULL,"rlca"},

	{0,4,lrnop08,NULL,"ld (:2),sp"},
	{0,11,lrnop09,NULL,"add hl,bc"},
	{0,4,lrnop0A,NULL,"ld a,(bc)"},
	{0,6,lrnop0B,NULL,"dec bc"},
	{0,4,lrnop0C,NULL,"inc c"},
	{0,4,lrnop0D,NULL,"dec c"},
	{0,4,lrnop0E,NULL,"ld c,:1"},
	{0,4,lrnop0F,NULL,"rrca"},

	{0,4,lrnop10,NULL,"stop"},
	{0,4,lrnop11,NULL,"ld de,:2"},
	{0,4,lrnop12,NULL,"ld (de),a"},
	{0,6,lrnop13,NULL,"inc de"},
	{0,4,lrnop14,NULL,"inc d"},
	{0,4,lrnop15,NULL,"dec d"},
	{0,4,lrnop16,NULL,"ld d,:1"},
	{0,4,lrnop17,NULL,"rla"},

	{OF_RELJUMP,4,lrnop18,NULL,"jr :3"},
	{0,11,lrnop19,NULL,"add hl,de"},
	{0,4,lrnop1A,NULL,"ld a,(de)"},
	{0,6,lrnop1B,NULL,"dec de"},
	{0,4,lrnop1C,NULL,"inc e"},
	{0,4,lrnop1D,NULL,"dec e"},
	{0,4,lrnop1E,NULL,"ld e,:1"},
	{0,4,lrnop1F,NULL,"rra"},

	{OF_RELJUMP,4,lrnop20,NULL,"jr nz,:3"},
	{0,4,lrnop21,NULL,"ld hl,:2"},
	{0,4,lrnop22,NULL,"ldi (hl),a"},
	{0,6,lrnop23,NULL,"inc hl"},
	{0,4,lrnop24,NULL,"inc h"},
	{0,4,lrnop25,NULL,"dec h"},
	{0,4,lrnop26,NULL,"ld h,:1"},
	{0,4,lrnop27,NULL,"daa"},

	{OF_RELJUMP,4,lrnop28,NULL,"jr z,:3"},
	{0,11,lrnop29,NULL,"add hl,hl"},
	{0,4,lrnop2A,NULL,"ldi a,(hl)"},
	{0,6,lrnop2B,NULL,"dec hl"},
	{0,4,lrnop2C,NULL,"inc l"},
	{0,4,lrnop2D,NULL,"dec l"},
	{0,4,lrnop2E,NULL,"ld l,:1"},
	{0,4,lrnop2F,NULL,"cpl"},

	{OF_RELJUMP,4,lrnop30,NULL,"jr nc,:3"},
	{0,4,lrnop31,NULL,"ld sp,:2"},
	{0,4,lrnop32,NULL,"ldd (hl),a"},
	{0,6,lrnop33,NULL,"inc sp"},
	{0,4,lrnop34,NULL,"inc (hl)"},
	{0,4,lrnop35,NULL,"dec (hl)"},
	{0,4,lrnop36,NULL,"ld (hl),:1"},
	{0,4,lrnop37,NULL,"scf"},

	{OF_RELJUMP,4,lrnop38,NULL,"jr c,:3"},
	{0,11,lrnop39,NULL,"add hl,sp"},
	{0,4,lrnop3A,NULL,"ldd a,(hl)"},
	{0,6,lrnop3B,NULL,"dec sp"},
	{0,4,lrnop3C,NULL,"inc a"},
	{0,4,lrnop3D,NULL,"dec a"},
	{0,4,lrnop3E,NULL,"ld a,:1"},
	{0,4,lrnop3F,NULL,"ccf"},

	{0,4,lrnop40,NULL,"ld b,b"},
	{0,4,lrnop41,NULL,"ld b,c"},
	{0,4,lrnop42,NULL,"ld b,d"},
	{0,4,lrnop43,NULL,"ld b,e"},
	{0,4,lrnop44,NULL,"ld b,h"},
	{0,4,lrnop45,NULL,"ld b,l"},
	{0,4,lrnop46,NULL,"ld b,(hl)"},
	{0,4,lrnop47,NULL,"ld b,a"},

	{0,4,lrnop48,NULL,"ld c,b"},
	{0,4,lrnop49,NULL,"ld c,c"},
	{0,4,lrnop4A,NULL,"ld c,d"},
	{0,4,lrnop4B,NULL,"ld c,e"},
	{0,4,lrnop4C,NULL,"ld c,h"},
	{0,4,lrnop4D,NULL,"ld c,l"},
	{0,4,lrnop4E,NULL,"ld c,(hl)"},
	{0,4,lrnop4F,NULL,"ld c,a"},

	{0,4,lrnop50,NULL,"ld d,b"},
	{0,4,lrnop51,NULL,"ld d,c"},
	{0,4,lrnop52,NULL,"ld d,d"},
	{0,4,lrnop53,NULL,"ld d,e"},
	{0,4,lrnop54,NULL,"ld d,h"},
	{0,4,lrnop55,NULL,"ld d,l"},
	{0,4,lrnop56,NULL,"ld d,(hl)"},
	{0,4,lrnop57,NULL,"ld d,a"},

	{0,4,lrnop58,NULL,"ld e,b"},
	{0,4,lrnop59,NULL,"ld e,c"},
	{0,4,lrnop5A,NULL,"ld e,d"},
	{0,4,lrnop5B,NULL,"ld e,e"},
	{0,4,lrnop5C,NULL,"ld e,h"},
	{0,4,lrnop5D,NULL,"ld e,l"},
	{0,4,lrnop5E,NULL,"ld e,(hl)"},
	{0,4,lrnop5F,NULL,"ld e,a"},

	{0,4,lrnop60,NULL,"ld h,b"},
	{0,4,lrnop61,NULL,"ld h,c"},
	{0,4,lrnop62,NULL,"ld h,d"},
	{0,4,lrnop63,NULL,"ld h,e"},
	{0,4,lrnop64,NULL,"ld h,h"},
	{0,4,lrnop65,NULL,"ld h,l"},
	{0,4,lrnop66,NULL,"ld h,(hl)"},
	{0,4,lrnop67,NULL,"ld h,a"},

	{0,4,lrnop68,NULL,"ld l,b"},
	{0,4,lrnop69,NULL,"ld l,c"},
	{0,4,lrnop6A,NULL,"ld l,d"},
	{0,4,lrnop6B,NULL,"ld l,e"},
	{0,4,lrnop6C,NULL,"ld l,h"},
	{0,4,lrnop6D,NULL,"ld l,l"},
	{0,4,lrnop6E,NULL,"ld l,(hl)"},
	{0,4,lrnop6F,NULL,"ld l,a"},

	{0,4,lrnop70,NULL,"ld (hl),b"},
	{0,4,lrnop71,NULL,"ld (hl),c"},
	{0,4,lrnop72,NULL,"ld (hl),d"},
	{0,4,lrnop73,NULL,"ld (hl),e"},
	{0,4,lrnop74,NULL,"ld (hl),h"},
	{0,4,lrnop75,NULL,"ld (hl),l"},
	{OF_SKIPABLE,4,lrnop76,NULL,"halt"},
	{0,4,lrnop77,NULL,"ld (hl),a"},

	{0,4,lrnop78,NULL,"ld a,b"},
	{0,4,lrnop79,NULL,"ld a,c"},
	{0,4,lrnop7A,NULL,"ld a,d"},
	{0,4,lrnop7B,NULL,"ld a,e"},
	{0,4,lrnop7C,NULL,"ld a,h"},
	{0,4,lrnop7D,NULL,"ld a,l"},
	{0,4,lrnop7E,NULL,"ld a,(hl)"},
	{0,4,lrnop7F,NULL,"ld a,a"},

	{0,4,lrnop80,NULL,"add a,b"},
	{0,4,lrnop81,NULL,"add a,c"},
	{0,4,lrnop82,NULL,"add a,d"},
	{0,4,lrnop83,NULL,"add a,e"},
	{0,4,lrnop84,NULL,"add a,h"},
	{0,4,lrnop85,NULL,"add a,l"},
	{0,4,lrnop86,NULL,"add a,(hl)"},
	{0,4,lrnop87,NULL,"add a,a"},

	{0,4,lrnop88,NULL,"adc a,b"},
	{0,4,lrnop89,NULL,"adc a,c"},
	{0,4,lrnop8A,NULL,"adc a,d"},
	{0,4,lrnop8B,NULL,"adc a,e"},
	{0,4,lrnop8C,NULL,"adc a,h"},
	{0,4,lrnop8D,NULL,"adc a,l"},
	{0,4,lrnop8E,NULL,"adc a,(hl)"},
	{0,4,lrnop8F,NULL,"adc a,a"},

	{0,4,lrnop90,NULL,"sub b"},
	{0,4,lrnop91,NULL,"sub c"},
	{0,4,lrnop92,NULL,"sub d"},
	{0,4,lrnop93,NULL,"sub e"},
	{0,4,lrnop94,NULL,"sub h"},
	{0,4,lrnop95,NULL,"sub l"},
	{0,4,lrnop96,NULL,"sub (hl)"},
	{0,4,lrnop97,NULL,"sub a"},

	{0,4,lrnop98,NULL,"sbc a,b"},
	{0,4,lrnop99,NULL,"sbc a,c"},
	{0,4,lrnop9A,NULL,"sbc a,d"},
	{0,4,lrnop9B,NULL,"sbc a,e"},
	{0,4,lrnop9C,NULL,"sbc a,h"},
	{0,4,lrnop9D,NULL,"sbc a,l"},
	{0,4,lrnop9E,NULL,"sbc a,(hl)"},
	{0,4,lrnop9F,NULL,"sbc a,a"},

	{0,4,lrnopA0,NULL,"and b"},
	{0,4,lrnopA1,NULL,"and c"},
	{0,4,lrnopA2,NULL,"and d"},
	{0,4,lrnopA3,NULL,"and e"},
	{0,4,lrnopA4,NULL,"and h"},
	{0,4,lrnopA5,NULL,"and l"},
	{0,4,lrnopA6,NULL,"and (hl)"},
	{0,4,lrnopA7,NULL,"and a"},

	{0,4,lrnopA8,NULL,"xor b"},
	{0,4,lrnopA9,NULL,"xor c"},
	{0,4,lrnopAA,NULL,"xor d"},
	{0,4,lrnopAB,NULL,"xor e"},
	{0,4,lrnopAC,NULL,"xor h"},
	{0,4,lrnopAD,NULL,"xor l"},
	{0,4,lrnopAE,NULL,"xor (hl)"},
	{0,4,lrnopAF,NULL,"xor a"},

	{0,4,lrnopB0,NULL,"or b"},
	{0,4,lrnopB1,NULL,"or c"},
	{0,4,lrnopB2,NULL,"or d"},
	{0,4,lrnopB3,NULL,"or e"},
	{0,4,lrnopB4,NULL,"or h"},
	{0,4,lrnopB5,NULL,"or l"},
	{0,4,lrnopB6,NULL,"or (hl)"},
	{0,4,lrnopB7,NULL,"or a"},

	{0,4,lrnopB8,NULL,"cp b"},
	{0,4,lrnopB9,NULL,"cp c"},
	{0,4,lrnopBA,NULL,"cp d"},
	{0,4,lrnopBB,NULL,"cp e"},
	{0,4,lrnopBC,NULL,"cp h"},
	{0,4,lrnopBD,NULL,"cp l"},
	{0,4,lrnopBE,NULL,"cp (hl)"},
	{0,4,lrnopBF,NULL,"cp a"},

	{0,5,lrnopC0,NULL,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,lrnopC1,NULL,"pop bc"},
	{0,4,lrnopC2,NULL,"jp nz,:2"},
	{0,4,lrnopC3,NULL,"jp :2"},
	{OF_SKIPABLE,4,lrnopC4,NULL,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,lrnopC5,NULL,"push bc"},		// 5 3wr 3wr
	{0,4,lrnopC6,NULL,"add a,:1"},
	{OF_SKIPABLE,5,lrnopC7,NULL,"rst #00"},		// 5 3wr 3wr

	{0,5,lrnopC8,NULL,"ret z"},
	{0,4,lrnopC9,NULL,"ret"},
	{0,4,lrnopCA,NULL,"jp z,:2"},
	{OF_PREFIX,4,lrnopCB,lrcbTab,"#CB"},
	{OF_SKIPABLE,4,lrnopCC,NULL,"call z,:2"},
	{OF_SKIPABLE,4,lrnopCD,NULL,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,lrnopCE,NULL,"adc a,:1"},
	{OF_SKIPABLE,5,lrnopCF,NULL,"rst #08"},

	{0,5,lrnopD0,NULL,"ret nc"},
	{0,4,lrnopD1,NULL,"pop de"},
	{0,4,lrnopD2,NULL,"jp nc,:2"},
	{0,4,lrnLock,NULL,"(lock)"},
	{OF_SKIPABLE,4,lrnopD4,NULL,"call nc,:2"},
	{0,5,lrnopD5,NULL,"push de"},
	{0,4,lrnopD6,NULL,"sub :1"},
	{OF_SKIPABLE,5,lrnopD7,NULL,"rst #10"},

	{0,5,lrnopD8,NULL,"ret c"},
	{0,10,lrnopD9,NULL,"reti"},
	{0,4,lrnopDA,NULL,"jp c,:2"},
	{0,4,lrnLock,NULL,"(lock)"},
	{OF_SKIPABLE,4,lrnopDC,NULL,"call c,:2"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnopDE,NULL,"sbc a,:1"},
	{OF_SKIPABLE,5,lrnopDF,NULL,"rst #18"},

	{0,4,lrnopE0,NULL,"ldh (:1),a"},
	{0,4,lrnopE1,NULL,"pop hl"},
	{0,4,lrnopE2,NULL,"ldh (c),a"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,5,lrnopE5,NULL,"push hl"},
	{0,4,lrnopE6,NULL,"and :1"},
	{OF_SKIPABLE,5,lrnopE7,NULL,"rst #20"},

	{0,4,lrnopE8,NULL,"add sp,:4"},
	{0,4,lrnopE9,NULL,"jp (hl)"},
	{0,4,lrnopEA,NULL,"ld (:2),a"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnopEE,NULL,"xor :1"},
	{OF_SKIPABLE,5,lrnopEF,NULL,"rst #28"},

	{0,4,lrnopF0,NULL,"ldh a,(:1)"},
	{0,4,lrnopF1,NULL,"pop af"},
	{0,4,lrnopF2,NULL,"ldh a,(c)"},
	{0,4,lrnopF3,NULL,"di"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,5,lrnopF5,NULL,"push af"},
	{0,4,lrnopF6,NULL,"or :1"},
	{OF_SKIPABLE,5,lrnopF7,NULL,"rst #30"},

	{0,4,lrnopF8,NULL,"ld hl,sp:4"},
	{0,6,lrnopF9,NULL,"ld sp,hl"},
	{0,4,lrnopFA,NULL,"ld a,(:2)"},
	{0,4,lrnopFB,NULL,"ei"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnLock,NULL,"(lock)"},
	{0,4,lrnopFE,NULL,"cp :1"},
	{OF_SKIPABLE,5,lrnopFF,NULL,"rst #38"}
};
