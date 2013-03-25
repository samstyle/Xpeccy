// 40	in b,(c)	4 4in		mptr = bc+1
void ed40(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->b = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->b];
}

// 41	out (c),b	4 4out		mptr = (a<<8) | ((port + 1) & 0xff)
void ed41(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->b,4);
	cpu->hptr = cpu->a;
}

// 42	sbc hl,bc	11
void ed42(Z80CPU* cpu) {
	SBC16(cpu->bc);
}

// 43	ld (nn),bc	4 3rd 3rd 3wr 3wr	mptr = nn + 1
void ed43(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->c,3);
	MEMWR(cpu->mptr,cpu->b,3);
}

// 44	neg	4
void ed44(Z80CPU* cpu) {
	cpu->tmpb = cpu->a;
	cpu->a = 0;
	SUB(cpu->tmpb);
}

// 45	retn	4 3rd 3rd
void ed45(Z80CPU* cpu) {
	cpu->iff1 = cpu->iff2;
	RET;
}

// 46	im0	4
void ed46(Z80CPU* cpu) {
	cpu->imode = 0;
}

// 47	ld i,a	5
void ed47(Z80CPU* cpu) {
	cpu->i = cpu->a;
}

// 48	in c,(c)	4 4in		mptr = port + 1
void ed48(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->c = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->c];
}

// 49	out (c),c	4 4out		mptr = (a<<8) | ((port + 1) & 0xff)
void ed49(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->c,4);
	cpu->hptr = cpu->a;
}

// 4a	adc hl,bc	11
void ed4A(Z80CPU* cpu) {
	ADC16(cpu->bc);
}

// 4b	ld bc,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void ed4B(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->c = MEMRD(cpu->mptr++,3);
	cpu->b = MEMRD(cpu->mptr,3);
}

// 4d	reti	4 3rd 3rd
void ed4D(Z80CPU* cpu) {
	cpu->iff1 = cpu->iff2;
	RET;
}

// 4f	ld r,a	5
void ed4F(Z80CPU* cpu) {
	cpu->r = cpu->a;
	cpu->r7 = cpu->a;
}

// 50	in d,(c)	4 4in	mptr = port + 1
void ed50(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->d = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->d];
}

// 51	out (c),d	4 4out	mptr = (a<<8) | ((port+1) & ff)
void ed51(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->d,4);
	cpu->hptr = cpu->a;
}

// 52	sbc hl,de	11
void ed52(Z80CPU* cpu) {
	SBC16(cpu->de);
}

// 53	ld (nn),de	4 3rd 3rd 3wr 3wr	mptr = nn + 1
void ed53(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->e,3);
	MEMWR(cpu->mptr,cpu->d,3);
}

// 56	im1		4
void ed56(Z80CPU* cpu) {
	cpu->imode = 1;
}

// 57	ld a,i		5
void ed57(Z80CPU* cpu) {
	cpu->a = cpu->i;
	cpu->f = (cpu->f & FC) | (sz53pTab[cpu->a] & ~FP) | (cpu->iff2 ? FV : 0);
}

// 58	in e,(c)	4 4in		mptr = port + 1
void ed58(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->e = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->e];
}

// 59	out (c),e	4 4out		mptr = ((port+1) & ff) | (a << 8)
void ed59(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->e,4);
	cpu->hptr = cpu->a;
}

// 5a	adc hl,de	11
void ed5A(Z80CPU* cpu) {
	ADC16(cpu->de);
}

// 5b	ld de,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn + 1
void ed5B(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->e = MEMRD(cpu->mptr++,3);
	cpu->d = MEMRD(cpu->mptr,3);
}

// 5e	im2		4
void ed5E(Z80CPU* cpu) {
	cpu->imode = 2;
}

// 5f	ld a,r		5
void ed5F(Z80CPU* cpu) {
	cpu->a = (cpu->r & 0x7f) | (cpu->r7 & 0x80);
	cpu->f = (cpu->f & FC) | (sz53pTab[cpu->a] & ~FP) | (cpu->iff2 ? FV : 0);
	cpu->resPV = 1;
}

// 60	in h,(c)	4 4in		mptr = port + 1
void ed60(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->h = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->h];
}

// 61	out (c),h	4 4out		mptr = ((port + 1) & FF) | (a << 8)
void ed61(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->h,4);
}

// 62	sbc hl,hl	11
void ed62(Z80CPU* cpu) {
	SBC16(cpu->hl);
}

// 67	rrd		4 3rd 4 3wr	mptr = hl + 1
void ed67(Z80CPU* cpu) {
	cpu->mptr = cpu->hl;
	cpu->tmpb = MEMRD(cpu->mptr,3);
	cpu->t += 4;
	MEMWR(cpu->mptr++,(cpu->a << 4) | (cpu->tmpb >> 4),3);
	cpu->a = (cpu->a & 0xf0) | (cpu->tmpb & 0x0f);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->a];
}

// 68	in l,(c)	4 4in		mptr = port + 1
void ed68(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->l = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->l];
}

// 69	out (c),l	4 4out		mptr = ((port+1)&FF)|(a<<8)
void ed69(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->l,4);
	cpu->hptr = cpu->a;
}

// 6a	adc hl,hl	11
void ed6A(Z80CPU* cpu) {
	ADC16(cpu->hl);
}

// 6f	rld		4 3rd 4 3wr	mptr = hl+1
void ed6F(Z80CPU* cpu) {
	cpu->mptr = cpu->hl;
	cpu->tmpb = MEMRD(cpu->mptr,3);
	cpu->t += 4;
	MEMWR(cpu->mptr++, (cpu->tmpb << 4 ) | (cpu->a & 0x0f), 3);
	cpu->a = (cpu->a & 0xf0) | (cpu->tmpb >> 4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->a];
}

// 70	in (c)		4 4in		mptr = port + 1
void ed70(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->tmp = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->tmp];
}

// 71	out (c),0	4 4out		mptr = ((port+1)&FF)|(a<<8)
void ed71(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,0,4);
	cpu->hptr = cpu->a;
}

// 72	sbc hl,sp	11
void ed72(Z80CPU* cpu) {
	SBC16(cpu->sp);
}

// 73	ld (nn),sp	4 3rd 3rd 3wr 3wr	mptr = nn + 1
void ed73(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->lsp,3);
	MEMWR(cpu->mptr,cpu->hsp,3);
}

// 78	in a,(c)	4 4in		mptr = port + 1
void ed78(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->a = IORD(cpu->mptr++,4);
	cpu->f = (cpu->f & FC) | sz53pTab[cpu->a];
}

// 79	out (c),a	4 4out		mptr = ((port+1)&FF)|(a<<8)
void ed79(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	IOWR(cpu->mptr++,cpu->a,4);
	cpu->hptr = cpu->a;
}

// 7a	adc hl,sp	11
void ed7A(Z80CPU* cpu) {
	ADC16(cpu->sp);
}

// 7b	ld sp,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn + 1
void ed7B(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->lsp = MEMRD(cpu->mptr++,3);
	cpu->hsp = MEMRD(cpu->mptr,3);
}

// a0	ldi	4 3rd 5wr
void edA0(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->hl++,3);
	MEMWR(cpu->de++,cpu->tmp,5);
	cpu->bc--;
	cpu->tmp += cpu->a;
	cpu->f = (cpu->f & (FC | FZ | FS)) | (cpu->bc ? FV : 0 ) | (cpu->tmp & F3) | ((cpu->tmp & 0x02) ? F5 : 0);
}

// a1	cpi	4 3rd 5?	mptr++
void edA1(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->hl,3);
	cpu->tmpw = cpu->a - cpu->tmpb;
	cpu->tmp = ((cpu->a & 0x08) >> 3) | ((cpu->tmpb & 0x08) >> 2) | ((cpu->tmpw & 0x08 ) >> 1);
	cpu->hl++;
	cpu->bc--;
	cpu->f = (cpu->f & FC) | (cpu->bc ? FV : 0) | FN | FHsubTab[cpu->tmp] | (cpu->tmpw ? 0 : FZ) | (cpu->tmpw & FS);
	if (cpu->f & FH) cpu->tmpw--;
	cpu->f |= (cpu->tmpw & F3) | ((cpu->tmpw & 0x02) ? F5 : 0);
	cpu->mptr++;
	cpu->t += 5;
}

// a2	ini	5 4in 3wr	mptr = bc + 1 (before dec)
void edA2(Z80CPU* cpu) {
	cpu->mptr = cpu->bc + 1;
	cpu->tmp = IORD(cpu->bc,4);
	MEMWR(cpu->hl++,cpu->tmp,3);
	cpu->b--;
	cpu->f = (cpu->tmp & 0x80 ? FN : 0) | (cpu->b & (FS | F5 | F3)) | (cpu->b ? 0 : FZ);
	
	cpu->tmpw = cpu->tmp + ((cpu->c + 1) & 0xff);
	if (cpu->tmpw > 255) cpu->f |= (FC | FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & FP);
}

// a3	outi	5 3rd 4wr	mptr = bc + 1 (after dec)
void edA3(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->hl,3);
	cpu->b--;
	cpu->mptr = cpu->bc + 1;
	IOWR(cpu->bc,cpu->tmp,4);
	cpu->hl++;
	cpu->f = (cpu->tmp & 0x80 ? FN : 0 ) | (cpu->b & (FS | F5 | F3)) | (cpu->b ? 0 : FZ);

	cpu->tmpw = cpu->tmp + cpu->l;
	if (cpu->tmpw > 255) cpu->f |= (FC | FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & FP);
}

// a8	ldd	4 3rd 5wr
void edA8(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->hl--,3);
	MEMWR(cpu->de--,cpu->tmp,5);
	cpu->bc--;
	cpu->tmp += cpu->a;
	cpu->f = (cpu->f & (FC | FZ | FS)) | (cpu->bc ? FV : 0 ) | (cpu->tmp & F3) | ((cpu->tmp & 0x02) ? F5 : 0);
}

// a9	cpd	4 3rd 5?	mptr--
void edA9(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->hl,3);
	cpu->tmpw = cpu->a - cpu->tmpb;
	cpu->tmp = ((cpu->a & 0x08) >> 3) | ((cpu->tmpb & 0x08) >> 2) | ((cpu->tmpw & 0x08 ) >> 1);
	cpu->hl--;
	cpu->bc--;
	cpu->f = (cpu->f & FC) | (cpu->bc ? FV : 0) | FN | FHsubTab[cpu->tmp] | (cpu->tmpw ? 0 : FZ) | (cpu->tmpw & FS);
	if (cpu->f & FH) cpu->tmpw--;
	cpu->f |= (cpu->tmpw & F3) | ((cpu->tmpw & 0x02) ? F5 : 0);
	cpu->mptr--;
	cpu->t += 5;
}

// aa	ind	5 4in 3wr	mptr = bc - 1 (before dec)
void edAA(Z80CPU* cpu) {
	cpu->mptr = cpu->bc - 1;
	cpu->tmp = IORD(cpu->bc,4);
	MEMWR(cpu->hl--,cpu->tmp,3);
	cpu->b--;
	cpu->f = ((cpu->tmp & 0x80) ? FN : 0) | (cpu->b & (FS | F5 | F3)) | (cpu->b ? 0 : FZ);
	
	cpu->tmpw = cpu->tmp + ((cpu->c - 1) & 0xff);
	if (cpu->tmpw > 255) cpu->f |= (FC | FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & FP);
}

// ab	outd	5 3rd 4wr	mptr = bc - 1 (after dec)
void edAB(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->hl,3);
	cpu->b--;
	cpu->mptr = cpu->bc - 1;
	IOWR(cpu->bc,cpu->tmp,4);
	cpu->hl--;
	cpu->f = (cpu->tmp & 0x80 ? FN : 0 ) | (cpu->b & (FS | F5 | F3)) | (cpu->b ? 0 : FZ);

	cpu->tmpw = cpu->tmp + cpu->l;
	if (cpu->tmpw > 255) cpu->f |= (FC | FH);
	cpu->f |= (sz53pTab[(cpu->tmpw & 7) ^ cpu->b] & FP);
}

// b0	ldir	= ldi until bc!=0	[+5T, mptr = pc+1]
void edB0(Z80CPU* cpu) {
	edA0(cpu);
	if (cpu->bc) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// b1	cpir	= cpi until (FV & !FZ)
void edB1(Z80CPU* cpu) {
	edA1(cpu);
	if ((cpu->f & (FV | FZ)) == FV) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// b2	inir	= ini until b!=0
void edB2(Z80CPU* cpu) {
	edA2(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// b3	otir	= outi until b!=0
void edB3(Z80CPU* cpu) {
	edA3(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// b8	lddr	= ldd until bc!=0
void edB8(Z80CPU* cpu) {
	edA8(cpu);
	if (cpu->bc) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// b9	cpdr	= cpd until (FV & !FZ)
void edB9(Z80CPU* cpu) {
	edA9(cpu);
	if ((cpu->f & (FV | FZ)) == FV) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// ba	indr	= ind until b!=0
void edBA(Z80CPU* cpu) {
	edAA(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

// bb	otdr	= outd until b!=0
void edBB(Z80CPU* cpu) {
	edAB(cpu);
	if (cpu->b) {
		cpu->pc -= 2;
		cpu->t += 5;
		cpu->mptr = cpu->pc + 1;
	}
}

opCode edTab[256]={
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,4,0,0,0,&ed40,"in b,(c)"},
	{0,4,4,0,0,0,&ed41,"out (c),b"},
	{0,11,0,0,0,0,&ed42,"sbc hl,bc"},
	{0,4,3,3,3,3,&ed43,"ld (:2),bc"},
	{0,4,0,0,0,0,&ed44,"neg"},
	{0,4,3,3,0,0,&ed45,"retn"},
	{0,4,0,0,0,0,&ed46,"im 0"},
	{0,5,0,0,0,0,&ed47,"ld i,a"},

	{0,4,4,0,0,0,&ed48,"in c,(c)"},
	{0,4,4,0,0,0,&ed49,"out (c),c"},
	{0,11,0,0,0,0,&ed4A,"adc hl,bc"},
	{0,4,3,3,3,3,&ed4B,"ld bc,(:2)"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed4D,"reti"},
	{0,4,0,0,0,0,&ed46,"im 0 *"},
	{0,5,0,0,0,0,&ed4F,"ld r,a"},

	{0,4,4,0,0,0,&ed50,"in d,(c)"},
	{0,4,4,0,0,0,&ed51,"out (c),d"},
	{0,11,0,0,0,0,&ed52,"sbc hl,de"},
	{0,4,3,3,3,3,&ed53,"ld (:2),de"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed45,"retn *"},
	{0,4,0,0,0,0,&ed56,"im 1"},
	{0,5,0,0,0,0,&ed57,"ld a,i"},

	{0,4,4,0,0,0,&ed58,"in e,(c)"},
	{0,4,4,0,0,0,&ed59,"out (c),e"},
	{0,11,0,0,0,0,&ed5A,"adc hl,de"},
	{0,4,3,3,3,3,&ed5B,"ld de,(:2)"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed4D,"reti *"},
	{0,4,0,0,0,0,&ed5E,"im 2"},
	{0,5,0,0,0,0,&ed5F,"ld a,r"},

	{0,4,4,0,0,0,&ed60,"in h,(c)"},
	{0,4,4,0,0,0,&ed61,"out (c),h"},
	{0,11,0,0,0,0,&ed62,"sbc hl,hl"},
	{0,4,3,3,3,3,&npr22,"ld (:2),hl"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed45,"retn *"},
	{0,4,0,0,0,0,&ed46,"im 0 *"},
	{0,4,3,4,3,0,&ed67,"rrd"},

	{0,4,4,0,0,0,&ed68,"in l,(c)"},
	{0,4,4,0,0,0,&ed69,"out (c),l"},
	{0,11,0,0,0,0,&ed6A,"adc hl,hl"},
	{0,4,3,3,3,3,&npr2A,"ld hl,(:2)"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed4D,"reti *"},
	{0,4,0,0,0,0,&ed46,"im 0 *"},
	{0,4,3,4,3,0,&ed6F,"rld"},

	{0,4,4,0,0,0,&ed70,"in (c)"},
	{0,4,4,0,0,0,&ed71,"out (c),0"},
	{0,11,0,0,0,0,&ed72,"sbc hl,sp"},
	{0,4,3,3,3,3,&ed73,"ld (:2),sp"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed45,"retn *"},
	{0,4,0,0,0,0,&ed56,"im 1 *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,4,0,0,0,&ed78,"in a,(c)"},
	{0,4,4,0,0,0,&ed79,"out (c),a"},
	{0,11,0,0,0,0,&ed7A,"adc hl,sp"},
	{0,4,3,3,3,3,&ed7B,"ld sp,(:2)"},
	{0,4,0,0,0,0,&ed44,"neg *"},
	{0,4,3,3,0,0,&ed4D,"reti *"},
	{0,4,0,0,0,0,&ed5E,"im 2 *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,3,5,0,0,&edA0,"ldi"},
	{0,4,3,5,0,0,&edA1,"cpi"},
	{0,5,4,3,0,0,&edA2,"ini"},
	{0,5,3,4,0,0,&edA3,"outi"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,3,5,0,0,&edA8,"ldd"},
	{0,4,3,5,0,0,&edA9,"cpd"},
	{0,5,4,3,0,0,&edAA,"ind"},
	{0,5,3,4,0,0,&edAB,"outd"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,3,5,5,0,&edB0,"ldir"},
	{0,4,3,5,5,0,&edB1,"cpir"},
	{0,5,3,3,5,0,&edB2,"inir"},
	{0,5,3,4,5,0,&edB3,"otir"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,3,5,5,0,&edB8,"lddr"},
	{0,4,3,5,5,0,&edB9,"cpdr"},
	{0,5,4,3,5,0,&edBA,"indr"},
	{0,5,3,4,5,0,&edBB,"otdr"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},

	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
	{0,4,0,0,0,0,&npr00,"nop *"},
};
