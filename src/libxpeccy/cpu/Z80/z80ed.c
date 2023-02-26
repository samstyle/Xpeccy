#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"
#include "z80_nop.h"

// 40	in b,(c)	4 4in		mptr = bc+1
void ed40(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->b = z80_iord(cpu, cpu->mptr++); // IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->b];
}

// 41	out (c),b	4 4out		mptr = (a<<8) | ((port + 1) & 0xff)
void ed41(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->b);
//	cpu->hptr = cpu->a;
}

// 42	sbc hl,bc	11
void ed42(CPU* cpu) {
	cpu->hl = z80_sub16(cpu, cpu->hl, cpu->bc, cpu->fz.c); //SBC16(cpu->bc);
}

// 43	ld (nn),bc	4 3rd 3rd 3wr 3wr	mptr = nn + 1
void ed43(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	z80_mwr(cpu, cpu->mptr++, cpu->c);
	z80_mwr(cpu, cpu->mptr, cpu->b);
}

// 44	neg	4
void ed44(CPU* cpu) {
	cpu->tmpb = cpu->a;
	cpu->a = 0;
	cpu->a = z80_sub8(cpu, cpu->tmpb, 0); //SUB(cpu->tmpb);
}

// 45	retn	4 3rd 3rd
void ed45(CPU* cpu) {
	cpu->iff1 = cpu->iff2;
	z80_ret(cpu);
}

// 46	im0	4
void ed46(CPU* cpu) {
	cpu->imode = 0;
}

// 47	ld i,a	5
void ed47(CPU* cpu) {
	cpu->i = cpu->a;
}

// 48	in c,(c)	4 4in		mptr = port + 1
void ed48(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->c = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->c];
}

// 49	out (c),c	4 4out		mptr = (a<<8) | ((port + 1) & 0xff)
void ed49(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->c);
//	cpu->hptr = cpu->a;
}

// 4a	adc hl,bc	11
void ed4A(CPU* cpu) {
	cpu->hl = z80_adc16(cpu, cpu->hl, cpu->bc, cpu->fz.c); // ADC16(cpu->bc);
}

// 4b	ld bc,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void ed4B(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	cpu->c = z80_mrd(cpu, cpu->mptr++);
	cpu->b = z80_mrd(cpu, cpu->mptr);
}

// 4d	reti	4 3rd 3rd
void ed4D(CPU* cpu) {
	// cpu->iff1 = cpu->iff2;
	z80_ret(cpu);
}

// 4f	ld r,a	5
void ed4F(CPU* cpu) {
	cpu->r = cpu->a;
	cpu->r7 = cpu->a & 0x80;
}

// 50	in d,(c)	4 4in	mptr = port + 1
void ed50(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->d = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->d];
}

// 51	out (c),d	4 4out	mptr = (a<<8) | ((port+1) & ff)
void ed51(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->d);
//	cpu->hptr = cpu->a;
}

// 52	sbc hl,de	11
void ed52(CPU* cpu) {
	cpu->hl = z80_sub16(cpu, cpu->hl, cpu->de, cpu->fz.c); //SBC16(cpu->de);
}

// 53	ld (nn),de	4 3rd 3rd 3wr 3wr	mptr = nn + 1
void ed53(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	z80_mwr(cpu, cpu->mptr++,cpu->e);
	z80_mwr(cpu, cpu->mptr,cpu->d);
}

// 56	im1		4
void ed56(CPU* cpu) {
	cpu->imode = 1;
}

// 57	ld a,i		5
void ed57(CPU* cpu) {
	cpu->a = cpu->i;
	cpu->f = (cpu->f & Z80_FC) | (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | (cpu->iff2 ? Z80_FV : 0);
	cpu->resPV = 1;
}

// 58	in e,(c)	4 4in		mptr = port + 1
void ed58(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->e = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->e];
}

// 59	out (c),e	4 4out		mptr = ((port+1) & ff) | (a << 8)
void ed59(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->e);
//	cpu->hptr = cpu->a;
}

// 5a	adc hl,de	11
void ed5A(CPU* cpu) {
	cpu->hl = z80_adc16(cpu, cpu->hl, cpu->de, cpu->fz.c); //ADC16(cpu->de);
}

// 5b	ld de,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn + 1
void ed5B(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	cpu->e = z80_mrd(cpu, cpu->mptr++);
	cpu->d = z80_mrd(cpu, cpu->mptr);
}

// 5e	im2		4
void ed5E(CPU* cpu) {
	cpu->imode = 2;
}

// 5f	ld a,r		5
void ed5F(CPU* cpu) {
	cpu->a = (cpu->r & 0x7f) | (cpu->r7 & 0x80);
	cpu->f = (cpu->f & Z80_FC) | (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | (cpu->iff2 ? Z80_FV : 0);
	cpu->resPV = 1;
}

// 60	in h,(c)	4 4in		mptr = port + 1
void ed60(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->h = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->h];
}

// 61	out (c),h	4 4out		mptr = ((port + 1) & FF) | (a << 8)
void ed61(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->h);
//	cpu->hptr = a;
}

// 62	sbc hl,hl	11
void ed62(CPU* cpu) {
	cpu->hl = z80_sub16(cpu, cpu->hl, cpu->hl, cpu->fz.c); //SBC16(cpu->hl);
}

// 67	rrd		4 3rd 4 3wr	mptr = hl + 1
void ed67(CPU* cpu) {
	cpu->mptr = cpu->hl;
	cpu->tmpb = z80_mrd(cpu, cpu->mptr);
	cpu->t += 4;
	z80_mwr(cpu, cpu->mptr++, (cpu->a << 4) | (cpu->tmpb >> 4));
	cpu->a = (cpu->a & 0xf0) | (cpu->tmpb & 0x0f);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->a];
}

// 68	in l,(c)	4 4in		mptr = port + 1
void ed68(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->l = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->l];
}

// 69	out (c),l	4 4out		mptr = ((port+1)&FF)|(a<<8)
void ed69(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->l);
//	cpu->hptr = cpu->a;
}

// 6a	adc hl,hl	11
void ed6A(CPU* cpu) {
	cpu->hl = z80_adc16(cpu, cpu->hl, cpu->hl, cpu->fz.c); //ADC16(cpu->hl);
}

// 6f	rld		4 3rd 4 3wr	mptr = hl+1
void ed6F(CPU* cpu) {
	cpu->mptr = cpu->hl;
	cpu->tmpb = z80_mrd(cpu, cpu->mptr);
	cpu->t += 4;
	z80_mwr(cpu, cpu->mptr++, (cpu->tmpb << 4 ) | (cpu->a & 0x0f));
	cpu->a = (cpu->a & 0xf0) | (cpu->tmpb >> 4);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->a];
}

// 70	in (c)		4 4in		mptr = port + 1
void ed70(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->tmp = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->tmp];
}

// 71	out (c),0	4 4out		mptr = ((port+1)&FF)|(a<<8)
void ed71(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, 0);
//	cpu->hptr = cpu->a;
}

// 72	sbc hl,sp	11
void ed72(CPU* cpu) {
	cpu->hl = z80_sub16(cpu, cpu->hl, cpu->sp, cpu->fz.c); //SBC16(cpu->sp);
}

// 73	ld (nn),sp	4 3rd 3rd 3wr 3wr	mptr = nn + 1
void ed73(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	z80_mwr(cpu, cpu->mptr++, cpu->lsp);
	z80_mwr(cpu, cpu->mptr,cpu->hsp);
}

// 78	in a,(c)	4 4in		mptr = port + 1
void ed78(CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->a = z80_iord(cpu, cpu->mptr++);
	cpu->f = (cpu->f & Z80_FC) | sz53pTab[cpu->a];
}

// 79	out (c),a	4 4out		mptr = ((port+1)&FF)|(a<<8)
void ed79(CPU* cpu) {
	cpu->mptr = cpu->bc;
	z80_iowr(cpu, cpu->mptr++, cpu->a);
//	cpu->hptr = cpu->a;
}

// 7a	adc hl,sp	11
void ed7A(CPU* cpu) {
	cpu->hl = z80_adc16(cpu, cpu->hl, cpu->sp, cpu->fz.c); //ADC16(cpu->sp);
}

// 7b	ld sp,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn + 1
void ed7B(CPU* cpu) {
	cpu->lptr = z80_mrd(cpu, cpu->pc++);
	cpu->hptr = z80_mrd(cpu, cpu->pc++);
	cpu->lsp = z80_mrd(cpu, cpu->mptr++);
	cpu->hsp = z80_mrd(cpu, cpu->mptr);
}

// a0	ldi	4 3rd 5wr
void edA0(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->hl++);
	z80_mwr(cpu, cpu->de++, cpu->tmp);
	cpu->t += 2;
	cpu->bc--;
	cpu->tmp += cpu->a;
	cpu->f = (cpu->f & (Z80_FC | Z80_FZ | Z80_FS)) | (cpu->bc ? Z80_FV : 0 ) | (cpu->tmp & Z80_F3) | ((cpu->tmp & 0x02) ? Z80_F5 : 0);
}

// a1	cpi	4 3rd 5?	mptr++
void edA1(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->hl);
	cpu->tmpw = cpu->a - cpu->tmpb;
	cpu->tmp = ((cpu->a & 0x08) >> 3) | ((cpu->tmpb & 0x08) >> 2) | ((cpu->tmpw & 0x08 ) >> 1);
	cpu->hl++;
	cpu->bc--;
	cpu->f = (cpu->f & Z80_FC) | (cpu->bc ? Z80_FV : 0) | Z80_FN | FHsubTab[cpu->tmp] | (cpu->tmpw ? 0 : Z80_FZ) | (cpu->tmpw & Z80_FS);
	if (cpu->f & Z80_FH) cpu->tmpw--;
	cpu->f |= (cpu->tmpw & Z80_F3) | ((cpu->tmpw & 0x02) ? Z80_F5 : 0);
	cpu->mptr++;
	cpu->t += 5;
}

// a2	ini	5 4in 3wr	mptr = bc + 1 (before dec)
void edA2(CPU* cpu) {
	cpu->mptr = cpu->bc + 1;
	cpu->tmp = z80_iord(cpu, cpu->bc);
	z80_mwr(cpu, cpu->hl++, cpu->tmp);
	cpu->b--;
	cpu->f = (cpu->tmp & 0x80 ? Z80_FN : 0) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);

	cpu->tmpw = cpu->tmp + ((cpu->c + 1) & 0xff);
	if (cpu->tmpw > 255) cpu->f |= (Z80_FC | Z80_FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
}

// a3	outi	5 3rd 4wr	mptr = bc + 1 (after dec)
void edA3(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->hl);
	cpu->b--;
	cpu->mptr = cpu->bc + 1;
	z80_iowr(cpu, cpu->bc, cpu->tmp);
	cpu->hl++;
	cpu->f = (cpu->tmp & 0x80 ? Z80_FN : 0 ) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);

	cpu->tmpw = cpu->tmp + cpu->l;
	if (cpu->tmpw > 255) cpu->f |= (Z80_FC | Z80_FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
}

// a8	ldd	4 3rd 5wr
void edA8(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->hl--);
	z80_mwr(cpu, cpu->de--,cpu->tmp);
	cpu->t += 2;
	cpu->bc--;
	cpu->tmp += cpu->a;
	cpu->f = (cpu->f & (Z80_FC | Z80_FZ | Z80_FS)) | (cpu->bc ? Z80_FV : 0 ) | (cpu->tmp & Z80_F3) | ((cpu->tmp & 0x02) ? Z80_F5 : 0);
}

// a9	cpd	4 3rd 5?	mptr--
void edA9(CPU* cpu) {
	cpu->tmpb = z80_mrd(cpu, cpu->hl);
	cpu->tmpw = cpu->a - cpu->tmpb;
	cpu->tmp = ((cpu->a & 0x08) >> 3) | ((cpu->tmpb & 0x08) >> 2) | ((cpu->tmpw & 0x08 ) >> 1);
	cpu->hl--;
	cpu->bc--;
	cpu->f = (cpu->f & Z80_FC) | (cpu->bc ? Z80_FV : 0) | Z80_FN | FHsubTab[cpu->tmp] | (cpu->tmpw ? 0 : Z80_FZ) | (cpu->tmpw & Z80_FS);
	if (cpu->f & Z80_FH) cpu->tmpw--;
	cpu->f |= (cpu->tmpw & Z80_F3) | ((cpu->tmpw & 0x02) ? Z80_F5 : 0);
	cpu->mptr--;
	cpu->t += 5;
}

// aa	ind	5 4in 3wr	mptr = bc - 1 (before dec)
void edAA(CPU* cpu) {
	cpu->mptr = cpu->bc - 1;
	cpu->tmp = z80_iord(cpu, cpu->bc);
	z80_mwr(cpu, cpu->hl--, cpu->tmp);
	cpu->b--;
	cpu->f = ((cpu->tmp & 0x80) ? Z80_FN : 0) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);

	cpu->tmpw = cpu->tmp + ((cpu->c - 1) & 0xff);
	if (cpu->tmpw > 255) cpu->f |= (Z80_FC | Z80_FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
}

// ab	outd	5 3rd 4wr	mptr = bc - 1 (after dec)
void edAB(CPU* cpu) {
	cpu->tmp = z80_mrd(cpu, cpu->hl);
	cpu->b--;
	cpu->mptr = cpu->bc - 1;
	z80_iowr(cpu, cpu->bc, cpu->tmp);
	cpu->hl--;
	cpu->f = (cpu->tmp & 0x80 ? Z80_FN : 0 ) | (cpu->b & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->b ? 0 : Z80_FZ);

	cpu->tmpw = cpu->tmp + cpu->l;
	if (cpu->tmpw > 255) cpu->f |= (Z80_FC | Z80_FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & Z80_FP);
}

// b0	ldir	= ldi until bc!=0	[+5T, mptr = pc+1]
void edB0(CPU* cpu) {
	edA0(cpu);
	if (cpu->bc) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blk = 1;
	} else {
		cpu->blk = 0;
	}
}

// b1	cpir	= cpi until (FV & !FZ)
void edB1(CPU* cpu) {
	edA1(cpu);
	if ((cpu->f & (Z80_FV | Z80_FZ)) == Z80_FV) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blk = 1;
	} else {
		cpu->blk = 0;
	}
}

// b2	inir	= ini until b!=0
void edB2(CPU* cpu) {
	edA2(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blkio = 1;
	} else {
		cpu->blkio = 0;
	}
}

// b3	otir	= outi until b!=0
void edB3(CPU* cpu) {
	edA3(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blkio = 1;
	} else {
		cpu->blkio = 0;
	}
}

// b8	lddr	= ldd until bc!=0
void edB8(CPU* cpu) {
	edA8(cpu);
	if (cpu->bc) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blk = 1;
	} else {
		cpu->blk = 0;
	}
}

// b9	cpdr	= cpd until (FV & !FZ)
void edB9(CPU* cpu) {
	edA9(cpu);
	if ((cpu->f & (Z80_FV | Z80_FZ)) == Z80_FV) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blk = 1;
	} else {
		cpu->blk = 0;
	}
}

// ba	indr	= ind until b!=0
void edBA(CPU* cpu) {
	edAA(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blkio = 1;
	} else {
		cpu->blkio = 0;
	}
}

// bb	otdr	= outd until b!=0
void edBB(CPU* cpu) {
	edAB(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
		cpu->blkio = 1;
	} else {
		cpu->blkio = 0;
	}
}

opCode edTab[256]={
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,ed40,NULL,"in b,(c)"},
	{0,4,ed41,NULL,"out (c),b"},
	{0,11,ed42,NULL,"sbc hl,bc"},
	{OF_MWORD | OF_MEMADR,4,ed43,NULL,"ld (:2),bc"},
	{0,4,ed44,NULL,"neg"},
	{0,4,ed45,NULL,"retn"},
	{0,4,ed46,NULL,"im 0"},
	{0,5,ed47,NULL,"ld i,a"},

	{0,4,ed48,NULL,"in c,(c)"},
	{0,4,ed49,NULL,"out (c),c"},
	{0,11,ed4A,NULL,"adc hl,bc"},
	{OF_MWORD | OF_MEMADR,4,ed4B,NULL,"ld bc,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti"},
	{0,4,ed46,NULL,"im 0 *"},
	{0,5,ed4F,NULL,"ld r,a"},

	{0,4,ed50,NULL,"in d,(c)"},
	{0,4,ed51,NULL,"out (c),d"},
	{0,11,ed52,NULL,"sbc hl,de"},
	{OF_MWORD | OF_MEMADR,4,ed53,NULL,"ld (:2),de"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed45,NULL,"retn *"},
	{0,4,ed56,NULL,"im 1"},
	{0,5,ed57,NULL,"ld a,i"},

	{0,4,ed58,NULL,"in e,(c)"},
	{0,4,ed59,NULL,"out (c),e"},
	{0,11,ed5A,NULL,"adc hl,de"},
	{OF_MWORD | OF_MEMADR,4,ed5B,NULL,"ld de,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti *"},
	{0,4,ed5E,NULL,"im 2"},
	{0,5,ed5F,NULL,"ld a,r"},

	{0,4,ed60,NULL,"in h,(c)"},
	{0,4,ed61,NULL,"out (c),h"},
	{0,11,ed62,NULL,"sbc hl,hl"},
	{OF_MWORD | OF_MEMADR,4,npr22,NULL,"ld (:2),hl"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed45,NULL,"retn *"},
	{0,4,ed46,NULL,"im 0 *"},
	{0,4,ed67,NULL,"rrd"},

	{0,4,ed68,NULL,"in l,(c)"},
	{0,4,ed69,NULL,"out (c),l"},
	{0,11,ed6A,NULL,"adc hl,hl"},
	{OF_MWORD | OF_MEMADR,4,npr2A,NULL,"ld hl,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti *"},
	{0,4,ed46,NULL,"im 0 *"},
	{0,4,ed6F,NULL,"rld"},

	{0,4,ed70,NULL,"in (c)"},
	{0,4,ed71,NULL,"out (c),0"},
	{0,11,ed72,NULL,"sbc hl,sp"},
	{OF_MWORD | OF_MEMADR,4,ed73,NULL,"ld (:2),sp"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed45,NULL,"retn *"},
	{0,4,ed56,NULL,"im 1 *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,ed78,NULL,"in a,(c)"},
	{0,4,ed79,NULL,"out (c),a"},
	{0,11,ed7A,NULL,"adc hl,sp"},
	{OF_MWORD | OF_MEMADR,4,ed7B,NULL,"ld sp,(:2)"},
	{0,4,ed44,NULL,"neg *"},
	{0,4,ed4D,NULL,"reti *"},
	{0,4,ed5E,NULL,"im 2 *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,edA0,NULL,"ldi"},
	{0,4,edA1,NULL,"cpi"},
	{0,5,edA2,NULL,"ini"},
	{0,5,edA3,NULL,"outi"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,edA8,NULL,"ldd"},
	{0,4,edA9,NULL,"cpd"},
	{0,5,edAA,NULL,"ind"},
	{0,5,edAB,NULL,"outd"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{OF_SKIPABLE,4,edB0,NULL,"ldir"},
	{OF_SKIPABLE,4,edB1,NULL,"cpir"},
	{OF_SKIPABLE,5,edB2,NULL,"inir"},
	{OF_SKIPABLE,5,edB3,NULL,"otir"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{OF_SKIPABLE,4,edB8,NULL,"lddr"},
	{OF_SKIPABLE,4,edB9,NULL,"cpdr"},
	{OF_SKIPABLE,5,edBA,NULL,"indr"},
	{OF_SKIPABLE,5,edBB,NULL,"otdr"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},

	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
	{0,4,npr00,NULL,"nop *"},
};
