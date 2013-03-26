// 00	nop		4
void npr00(Z80CPU* cpu) {}

// 01	ld bc,nn	4 3rd 3rd
void npr01(Z80CPU* cpu) {
	cpu->c = MEMRD(cpu->pc++,3);
	cpu->b = MEMRD(cpu->pc++,3);
}

// 02	ld (bc),a	4 3wr		mptr = (a << 8) | ((bc + 1) & 0xff)
void npr02(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	MEMWR(cpu->mptr++,cpu->a,3);
	cpu->hptr = cpu->a;
}

// 03	inc bc		6
void npr03(Z80CPU* cpu) {
	cpu->bc++;
}

// 04	inc b		4
void npr04(Z80CPU* cpu) {
	INC(cpu->b);
}

// 05	dec b		4
void npr05(Z80CPU* cpu) {
	DEC(cpu->b);
}

// 06	ld b,n		4 3rd
void npr06(Z80CPU* cpu) {
	cpu->b = MEMRD(cpu->pc++,3);
}

// 07	rlca		4
void npr07(Z80CPU* cpu) {
	cpu->a = (cpu->a << 1) | (cpu->a >> 7);
	cpu->f = (cpu->f & (FS | FZ | FP)) | (cpu->a & (F5 | F3 | FC));
}

// 08	ex af,af'	4
void npr08(Z80CPU* cpu) {
	SWAP(cpu->af,cpu->af_);
}

// 09	add hl,bc	11		mptr = hl+1 before adding
void npr09(Z80CPU* cpu) {
	ADD16(cpu->hl, cpu->bc);
}

// 0A	ld a,(bc)	4 3rd		mptr = bc+1
void npr0A(Z80CPU* cpu) {
	cpu->mptr = cpu->bc;
	cpu->a = MEMRD(cpu->mptr++,3);
}

// 0B	dec bc		6
void npr0B(Z80CPU* cpu) {
	cpu->bc--;
}

// 0C	inc c		4
void npr0C(Z80CPU* cpu) {
	INC(cpu->c);
}

// 0D	dec c		4
void npr0D(Z80CPU* cpu) {
	DEC(cpu->c);
}

// 0E	ld c,n		4 3rd
void npr0E(Z80CPU* cpu) {
	cpu->c = MEMRD(cpu->pc++,3);
}

// 0F	rrca		4
void npr0F(Z80CPU* cpu) {
	cpu->f = (cpu->f & (FS | FZ | FP)) | (cpu->a & FC);
	cpu->a = (cpu->a >> 1) | (cpu->a << 7);
	cpu->f |= (cpu->a & (F5 | F3));
}

// 10	djnz		5 3rd [5jr]
void npr10(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	cpu->b--;
	if (cpu->b) JR(cpu->tmp);
}

// 11	ld de,nn	4 3rd 3rd
void npr11(Z80CPU* cpu) {
	cpu->e = MEMRD(cpu->pc++,3);
	cpu->d = MEMRD(cpu->pc++,3);
}

// 12	ld (de),a	4 3wr		mptr = (a << 8) | ((de + 1) & 0xff)
void npr12(Z80CPU* cpu) {
	cpu->mptr = cpu->de;
	MEMWR(cpu->mptr++,cpu->a,3);
	cpu->hptr = cpu->a;
}

// 13	inc de		6
void npr13(Z80CPU* cpu) {
	cpu->de++;
}

// 14	inc d		4
void npr14(Z80CPU* cpu) {
	INC(cpu->d);
}

// 15	dec d		4
void npr15(Z80CPU* cpu) {
	DEC(cpu->d);
}

// 16	ld d,n		4 3rd
void npr16(Z80CPU* cpu) {
	cpu->d = MEMRD(cpu->pc++,3);
}

// 17	rla		4
void npr17(Z80CPU* cpu) {
	cpu->tmp = cpu->a;
	cpu->a = (cpu->a << 1) | (cpu->f & FC);
	cpu->f = (cpu->f & (FS | FZ | FP)) | (cpu->a & (F5 | F3)) | (cpu->tmp >> 7);
}

// 18	jr e		4 3rd 5jr
void npr18(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	JR(cpu->tmp);
}

// 19	add hl,de	11	mptr = hl+1 before adding
void npr19(Z80CPU* cpu) {
	ADD16(cpu->hl,cpu->de);
}

// 1A	ld a,(de)	4 3rd	mptr = de + 1
void npr1A(Z80CPU* cpu) {
	cpu->a = MEMRD(cpu->de,3);
	cpu->mptr = cpu->de + 1;
}

// 1B	dec de		6
void npr1B(Z80CPU* cpu) {
	cpu->de--;
}

// 1C	inc e		4
void npr1C(Z80CPU* cpu) {
	INC(cpu->e);
}

// 1D	dec e		4
void npr1D(Z80CPU* cpu) {
	DEC(cpu->e);
}

// 1E	ld e,n		4 3rd
void npr1E(Z80CPU* cpu) {
	cpu->e = MEMRD(cpu->pc++,3);
}

// 1F	rra		4
void npr1F(Z80CPU* cpu) {
	cpu->tmp = cpu->a;
	cpu->a = (cpu->a >> 1) | (cpu->f << 7);
	cpu->f = (cpu->f & (FS | FZ | FP)) | (cpu->a & (F5 | F3)) | (cpu->tmp & FC);
}

// 20	jr nz,e		4 3rd [5jr]
void npr20(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FZ)) JR(cpu->tmp);
}

// 21	ld hl,nn	4 3rd 3rd
void npr21(Z80CPU* cpu) {
	cpu->l = MEMRD(cpu->pc++,3);
	cpu->h = MEMRD(cpu->pc++,3);
}

// 22	ld (nn),hl	4 3rd 3rd 3wr 3wr	mptr = nn+1
void npr22(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->l,3);
	MEMWR(cpu->mptr,cpu->h,3);
}

// 23	inc hl		6
void npr23(Z80CPU* cpu) {
	cpu->hl++;
}

// 24	inc h		4
void npr24(Z80CPU* cpu) {
	INC(cpu->h);
}

// 25	dec h		4
void npr25(Z80CPU* cpu) {
	DEC(cpu->h);
}

// 26	ld h,n		4 3rd
void npr26(Z80CPU* cpu) {
	cpu->h = MEMRD(cpu->pc++,3);
}

// 27	daa		4
void npr27(Z80CPU* cpu) {
	const unsigned char* tdaa = daaTab + 2 * (cpu->a + 0x100 * ((cpu->f & 3) + ((cpu->f >> 2) & 4)));
	cpu->f = *tdaa;
	cpu->a = *(tdaa + 1);
}

// 28	jr z,e		4 3rd [5jr]
void npr28(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	if (cpu->f & FZ) JR(cpu->tmp);
}

// 29	add hl,hl	11
void npr29(Z80CPU* cpu) {
	ADD16(cpu->hl,cpu->hl);
}

// 2A	ld hl,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void npr2A(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->l = MEMRD(cpu->mptr++,3);
	cpu->h = MEMRD(cpu->mptr,3);
}

// 2B	dec hl		6
void npr2B(Z80CPU* cpu) {
	cpu->hl--;
}

// 2C	inc l		4
void npr2C(Z80CPU* cpu) {
	INC(cpu->l);
}

// 2D	dec l		4
void npr2D(Z80CPU* cpu) {
	DEC(cpu->l);
}

// 2E	ld l,n		4 3rd
void npr2E(Z80CPU* cpu) {
	cpu->l = MEMRD(cpu->pc++,3);
}

// 2F	cpl		4
void npr2F(Z80CPU* cpu) {
	cpu->a ^= 0xff;
	cpu->f = (cpu->f & (FS | FZ | FP | FC)) | (cpu->a & (F5 | F3)) | FH | FN;
}

// 30	jr nc,e		4 3rd [5jr]
void npr30(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FC)) JR(cpu->tmp);
}

// 31	ld sp,nn	4 3rd 3rd
void npr31(Z80CPU* cpu) {
	cpu->lsp = MEMRD(cpu->pc++,3);
	cpu->hsp = MEMRD(cpu->pc++,3);
}

// 32	ld (nn),a	4 3rd 3rd 3wr		mptr = (a << 8) | ((nn + 1) & 0xff)
void npr32(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++, cpu->a, 3);
	cpu->hptr = cpu->a;
}

// 33	inc sp		6
void npr33(Z80CPU* cpu) {
	cpu->sp++;
}

// 34	inc (hl)	4 3rd 4wr
void npr34(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->hl,3);
	INC(cpu->tmpb);
	MEMWR(cpu->hl,cpu->tmpb,4);
}

// 35	dec (hl)	4 3rd 4wr
void npr35(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->hl,3);
	DEC(cpu->tmpb);
	MEMWR(cpu->hl,cpu->tmpb,4);
}

// 36	ld (hl),n	4 3rd 3wr
void npr36(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	MEMWR(cpu->hl,cpu->tmp,3);
}

// 37	scf		4
void npr37(Z80CPU* cpu) {
	cpu->f = (cpu->f & (FS | FZ | FP)) | (cpu->a & (F5 | F3)) | FC;
}

// 38	jr c,e		4 3rd [5jr]
void npr38(Z80CPU* cpu) {
	cpu->tmp = MEMRD(cpu->pc++,3);
	if (cpu->f & FC) JR(cpu->tmp);
}

// 39	add hl,sp	11
void npr39(Z80CPU* cpu) {
	ADD16(cpu->hl,cpu->sp);
}

// 3A	ld a,(nn)	4 3rd 3rd 3rd	mptr = nn+1
void npr3A(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->a = MEMRD(cpu->mptr++,3);
}

// 3B	dec sp		6
void npr3B(Z80CPU* cpu) {
	cpu->sp--;
}

// 3C	inc a		4
void npr3C(Z80CPU* cpu) {
	INC(cpu->a);
}

// 3D	dec a		4
void npr3D(Z80CPU* cpu) {
	DEC(cpu->a);
}

// 3E	ld a,n		4 3rd
void npr3E(Z80CPU* cpu) {
	cpu->a = MEMRD(cpu->pc++,3);
}

// 3F	ccf		4
void npr3F(Z80CPU* cpu) {
	cpu->f = (cpu->f & (FS | FZ | FP)) | ((cpu->f & FC ) ? FH : FC) | (cpu->a & (F5 | F3));
}

// 40..47	ld b,r		4 [3rd]
void npr40(Z80CPU* cpu) {}
void npr41(Z80CPU* cpu) {cpu->b = cpu->c;}
void npr42(Z80CPU* cpu) {cpu->b = cpu->d;}
void npr43(Z80CPU* cpu) {cpu->b = cpu->e;}
void npr44(Z80CPU* cpu) {cpu->b = cpu->h;}
void npr45(Z80CPU* cpu) {cpu->b = cpu->l;}
void npr46(Z80CPU* cpu) {cpu->b = MEMRD(cpu->hl,3);}
void npr47(Z80CPU* cpu) {cpu->b = cpu->a;}
// 48..4f	ld c,r		4 [3rd]
void npr48(Z80CPU* cpu) {cpu->c = cpu->b;}
void npr49(Z80CPU* cpu) {}
void npr4A(Z80CPU* cpu) {cpu->c = cpu->d;}
void npr4B(Z80CPU* cpu) {cpu->c = cpu->e;}
void npr4C(Z80CPU* cpu) {cpu->c = cpu->h;}
void npr4D(Z80CPU* cpu) {cpu->c = cpu->l;}
void npr4E(Z80CPU* cpu) {cpu->c = MEMRD(cpu->hl,3);}
void npr4F(Z80CPU* cpu) {cpu->c = cpu->a;}
// 50..57	ld d,r		4 [3rd]
void npr50(Z80CPU* cpu) {cpu->d = cpu->b;}
void npr51(Z80CPU* cpu) {cpu->d = cpu->c;}
void npr52(Z80CPU* cpu) {}
void npr53(Z80CPU* cpu) {cpu->d = cpu->e;}
void npr54(Z80CPU* cpu) {cpu->d = cpu->h;}
void npr55(Z80CPU* cpu) {cpu->d = cpu->l;}
void npr56(Z80CPU* cpu) {cpu->d = MEMRD(cpu->hl,3);}
void npr57(Z80CPU* cpu) {cpu->d = cpu->a;}
// 58..5f	ld e,r		4 [3rd]
void npr58(Z80CPU* cpu) {cpu->e = cpu->b;}
void npr59(Z80CPU* cpu) {cpu->e = cpu->c;}
void npr5A(Z80CPU* cpu) {cpu->e = cpu->d;}
void npr5B(Z80CPU* cpu) {}
void npr5C(Z80CPU* cpu) {cpu->e = cpu->h;}
void npr5D(Z80CPU* cpu) {cpu->e = cpu->l;}
void npr5E(Z80CPU* cpu) {cpu->e = MEMRD(cpu->hl,3);}
void npr5F(Z80CPU* cpu) {cpu->e = cpu->a;}
// 60..67	ld h,r		4 [3rd]
void npr60(Z80CPU* cpu) {cpu->h = cpu->b;}
void npr61(Z80CPU* cpu) {cpu->h = cpu->c;}
void npr62(Z80CPU* cpu) {cpu->h = cpu->d;}
void npr63(Z80CPU* cpu) {cpu->h = cpu->e;}
void npr64(Z80CPU* cpu) {}
void npr65(Z80CPU* cpu) {cpu->h = cpu->l;}
void npr66(Z80CPU* cpu) {cpu->h = MEMRD(cpu->hl,3);}
void npr67(Z80CPU* cpu) {cpu->h = cpu->a;}
// 68..6f	ld l,r		4 [3rd]
void npr68(Z80CPU* cpu) {cpu->l = cpu->b;}
void npr69(Z80CPU* cpu) {cpu->l = cpu->c;}
void npr6A(Z80CPU* cpu) {cpu->l = cpu->d;}
void npr6B(Z80CPU* cpu) {cpu->l = cpu->e;}
void npr6C(Z80CPU* cpu) {cpu->l = cpu->h;}
void npr6D(Z80CPU* cpu) {}
void npr6E(Z80CPU* cpu) {cpu->l = MEMRD(cpu->hl,3);}
void npr6F(Z80CPU* cpu) {cpu->l = cpu->a;}
// 70..77	ld (hl),r	4 3wr
void npr70(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->b,3);}
void npr71(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->c,3);}
void npr72(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->d,3);}
void npr73(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->e,3);}
void npr74(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->h,3);}
void npr75(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->l,3);}
void npr76(Z80CPU* cpu) {cpu->halt = 1; cpu->pc--;}
void npr77(Z80CPU* cpu) {MEMWR(cpu->hl,cpu->a,3);}
// 78..7f	ld a,r		4 [3rd]
void npr78(Z80CPU* cpu) {cpu->a = cpu->b;}
void npr79(Z80CPU* cpu) {cpu->a = cpu->c;}
void npr7A(Z80CPU* cpu) {cpu->a = cpu->d;}
void npr7B(Z80CPU* cpu) {cpu->a = cpu->e;}
void npr7C(Z80CPU* cpu) {cpu->a = cpu->h;}
void npr7D(Z80CPU* cpu) {cpu->a = cpu->l;}
void npr7E(Z80CPU* cpu) {cpu->a = MEMRD(cpu->hl,3);}
void npr7F(Z80CPU* cpu) {}
// 80..87	add a,r		4 [3rd]
void npr80(Z80CPU* cpu) {ADD(cpu->b);}
void npr81(Z80CPU* cpu) {ADD(cpu->c);}
void npr82(Z80CPU* cpu) {ADD(cpu->d);}
void npr83(Z80CPU* cpu) {ADD(cpu->e);}
void npr84(Z80CPU* cpu) {ADD(cpu->h);}
void npr85(Z80CPU* cpu) {ADD(cpu->l);}
void npr86(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,3); ADD(cpu->tmpb);}
void npr87(Z80CPU* cpu) {ADD(cpu->a);}
// 88..8F	adc a,r		4 [3rd]
void npr88(Z80CPU* cpu) {ADC(cpu->b);}
void npr89(Z80CPU* cpu) {ADC(cpu->c);}
void npr8A(Z80CPU* cpu) {ADC(cpu->d);}
void npr8B(Z80CPU* cpu) {ADC(cpu->e);}
void npr8C(Z80CPU* cpu) {ADC(cpu->h);}
void npr8D(Z80CPU* cpu) {ADC(cpu->l);}
void npr8E(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,3); ADC(cpu->tmpb);}
void npr8F(Z80CPU* cpu) {ADC(cpu->a);}
// 90..97	sub r		4 [3rd]
void npr90(Z80CPU* cpu) {SUB(cpu->b);}
void npr91(Z80CPU* cpu) {SUB(cpu->c);}
void npr92(Z80CPU* cpu) {SUB(cpu->d);}
void npr93(Z80CPU* cpu) {SUB(cpu->e);}
void npr94(Z80CPU* cpu) {SUB(cpu->h);}
void npr95(Z80CPU* cpu) {SUB(cpu->l);}
void npr96(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,3); SUB(cpu->tmpb);}
void npr97(Z80CPU* cpu) {SUB(cpu->a);}
// 98..9F	sbc a,r		4 [3rd]
void npr98(Z80CPU* cpu) {SBC(cpu->b);}
void npr99(Z80CPU* cpu) {SBC(cpu->c);}
void npr9A(Z80CPU* cpu) {SBC(cpu->d);}
void npr9B(Z80CPU* cpu) {SBC(cpu->e);}
void npr9C(Z80CPU* cpu) {SBC(cpu->h);}
void npr9D(Z80CPU* cpu) {SBC(cpu->l);}
void npr9E(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,3); SBC(cpu->tmpb);}
void npr9F(Z80CPU* cpu) {SBC(cpu->a);}
// a0..a7	and r		4 [3rd]
void nprA0(Z80CPU* cpu) {cpu->a &= cpu->b; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA1(Z80CPU* cpu) {cpu->a &= cpu->c; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA2(Z80CPU* cpu) {cpu->a &= cpu->d; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA3(Z80CPU* cpu) {cpu->a &= cpu->e; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA4(Z80CPU* cpu) {cpu->a &= cpu->h; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA5(Z80CPU* cpu) {cpu->a &= cpu->l; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,3); cpu->a &= cpu->tmp; cpu->f = sz53pTab[cpu->a] | FH;}
void nprA7(Z80CPU* cpu) {cpu->f = sz53pTab[cpu->a] | FH;}
// a8..af	xor r		4 [3rd]
void nprA8(Z80CPU* cpu) {cpu->a ^= cpu->b; cpu->f = sz53pTab[cpu->a];}
void nprA9(Z80CPU* cpu) {cpu->a ^= cpu->c; cpu->f = sz53pTab[cpu->a];}
void nprAA(Z80CPU* cpu) {cpu->a ^= cpu->d; cpu->f = sz53pTab[cpu->a];}
void nprAB(Z80CPU* cpu) {cpu->a ^= cpu->e; cpu->f = sz53pTab[cpu->a];}
void nprAC(Z80CPU* cpu) {cpu->a ^= cpu->h; cpu->f = sz53pTab[cpu->a];}
void nprAD(Z80CPU* cpu) {cpu->a ^= cpu->l; cpu->f = sz53pTab[cpu->a];}
void nprAE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,3); cpu->a ^= cpu->tmp; cpu->f = sz53pTab[cpu->a];}
void nprAF(Z80CPU* cpu) {cpu->a = 0; cpu->f = FZ | FP;}
// b0..b8	or r		4 [3rd]
void nprB0(Z80CPU* cpu) {cpu->a |= cpu->b; cpu->f = sz53pTab[cpu->a];}
void nprB1(Z80CPU* cpu) {cpu->a |= cpu->c; cpu->f = sz53pTab[cpu->a];}
void nprB2(Z80CPU* cpu) {cpu->a |= cpu->d; cpu->f = sz53pTab[cpu->a];}
void nprB3(Z80CPU* cpu) {cpu->a |= cpu->e; cpu->f = sz53pTab[cpu->a];}
void nprB4(Z80CPU* cpu) {cpu->a |= cpu->h; cpu->f = sz53pTab[cpu->a];}
void nprB5(Z80CPU* cpu) {cpu->a |= cpu->l; cpu->f = sz53pTab[cpu->a];}
void nprB6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,3); cpu->a |= cpu->tmp; cpu->f = sz53pTab[cpu->a];}
void nprB7(Z80CPU* cpu) {cpu->f = sz53pTab[cpu->a];}
// b9..bf	cp r		4 [3rd]
void nprB8(Z80CPU* cpu) {CP(cpu->b);}
void nprB9(Z80CPU* cpu) {CP(cpu->c);}
void nprBA(Z80CPU* cpu) {CP(cpu->d);}
void nprBB(Z80CPU* cpu) {CP(cpu->e);}
void nprBC(Z80CPU* cpu) {CP(cpu->h);}
void nprBD(Z80CPU* cpu) {CP(cpu->l);}
void nprBE(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,3); CP(cpu->tmpb);}
void nprBF(Z80CPU* cpu) {CP(cpu->a);}

// c0	ret nz		5 [3rd 3rd]	mptr = ret.adr (if ret)
void nprC0(Z80CPU* cpu) {
	if (!(cpu->f & FZ)) RET
}

// c1	pop bc		4 3rd 3rd
void nprC1(Z80CPU* cpu) {
	POP(cpu->b,cpu->c);
}

// c2	jp nz,nn	4 3rd 3rd	mptr = nn
void nprC2(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FZ)) cpu->pc = cpu->mptr;
}

// c3	jp nn		4 3rd 3rd	mptr = nn
void nprC3(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc,3);
	cpu->pc = cpu->mptr;
}

// c4	call nz,nn	4 3rd 3rd[+1] [3wr 3wr]	mptr = nn
void nprC4(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FZ)) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// c5	push bc		5 3wr 3wr
void nprC5(Z80CPU* cpu) {
	PUSH(cpu->b, cpu->c);
}

// c6	add a,n		4 3rd
void nprC6(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	ADD(cpu->tmpb);
}

// c7	rst00		5 3wr 3wr	mptr = 0
void nprC7(Z80CPU* cpu) {
	RST(0x00);
}

// c8	ret z		5 [3rd 3rd]	[mptr = ret.adr]
void nprC8(Z80CPU* cpu) {
	if (cpu->f & FZ) RET;
}

// c9	ret		5 3rd 3rd	mptr = ret.adr
void nprC9(Z80CPU* cpu) {
	RET;
}

// ca	jp z,nn		4 3rd 3rd	mptr = nn
void nprCA(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FZ) cpu->pc = cpu->mptr;
}

// cb	prefix		4
void nprCB(Z80CPU* cpu) {
	cpu->opTab = cbTab;
}

// cc	call z,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprCC(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FZ) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// cd	call nn		4 3rd 4rd 3wr 3wr		mptr = nn
void nprCD(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,4);
	PUSH(cpu->hpc,cpu->lpc);
	cpu->pc = cpu->mptr;
}

// ce	adc a,n		4 3rd
void nprCE(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	ADC(cpu->tmpb);
}

// cf	rst08		5 3wr 3wr		mptr = 8
void nprCF(Z80CPU* cpu) {
	RST(0x08);
}

// d0	ret nc		5 [3rd 3rd]		[mptr = ret.adr]
void nprD0(Z80CPU* cpu) {
	if (!(cpu->f & FC)) RET;
}

// d1	pop de		4 3rd 3rd
void nprD1(Z80CPU* cpu) {
	POP(cpu->d,cpu->e);
}

// d2	jp nc,nn	4 3rd 3rd		mptr = nn
void nprD2(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FC)) cpu->pc = cpu->mptr;
}

// d3	out(n),a	4 3rd 4out		mptr = (a<<8) | ((n + 1) & 0xff)
void nprD3(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = cpu->a;
	IOWR(cpu->mptr,cpu->a,4);
	cpu->lptr++;
}

// d4	call nc,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprD4(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FC)) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// d5	push de		5 3wr 3wr
void nprD5(Z80CPU* cpu) {
	PUSH(cpu->d,cpu->e);
}

// d6	sub n		4 3rd
void nprD6(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	SUB(cpu->tmpb);
}

// d7	rst10		5 3wr 3wr	mptr = 0x10
void nprD7(Z80CPU* cpu) {
	RST(0x10);
}

// d8	ret c		5 [3rd 3rd]	[mptr = ret.adr]
void nprD8(Z80CPU* cpu) {
	if (cpu->f & FC) RET;
}

// d9	exx		4
void nprD9(Z80CPU* cpu) {
	SWAP(cpu->bc,cpu->bc_);
	SWAP(cpu->de,cpu->de_);
	SWAP(cpu->hl,cpu->hl_);
}

// da	jp c,nn		4 3rd 3rd	memptr = nn
void nprDA(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FC) cpu->pc = cpu->mptr;
}

// db	in a,(n)	4 3rd 4in	memptr = ((a<<8) | n) + 1
void nprDB(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = cpu->a;
	cpu->a = IORD(cpu->mptr++,4);
}

// dc	call c,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprDC(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FC) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// dd	prefix IX	4
void nprDD(Z80CPU* cpu) {
	cpu->opTab = ddTab;
}

// de	sbc a,n		4 3rd
void nprDE(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	SBC(cpu->tmpb);
}

// df	rst18		5 3wr 3wr	mptr = 0x18;
void nprDF(Z80CPU* cpu) {
	RST(0x18);
}

// e0	ret po		5 [3rd 3rd]	[mptr = ret.adr]
void nprE0(Z80CPU* cpu) {
	if (!(cpu->f & FP)) RET
}

// e1	pop hl		4 3rd 3rd
void nprE1(Z80CPU* cpu) {
	POP(cpu->h, cpu->l);
}

// e2	jp po,nn	4 3rd 3rd	mptr = nn
void nprE2(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FP)) cpu->pc = cpu->mptr;
}

// e3	ex (sp),hl	4 3rd 4rd 3wr 5wr	mptr = hl
void nprE3(Z80CPU* cpu) {
	POP(cpu->htw,cpu->ltw); cpu->t++;	// 3,3+1
	PUSH(cpu->h, cpu->l); cpu->t += 2;	// 3,3+2
	cpu->mptr = cpu->hl;
	cpu->hl = cpu->tmpw;
}

// e4	call po,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprE4(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FP)) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// e5	push hl		5 3wr 3wr
void nprE5(Z80CPU* cpu) {
	PUSH(cpu->h, cpu->l);
}

// e6	and n		4 3rd
void nprE6(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	cpu->a &= cpu->tmpb;
	cpu->f = sz53pTab[cpu->a] | FH;
}

// e7	rst20		5 3wr 3wr	mptr = 0x20
void nprE7(Z80CPU* cpu) {
	RST(0x20);
}

// e8	ret pe		5 [3rd 3rd]	[mptr = ret.adr]
void nprE8(Z80CPU* cpu) {
	if (cpu->f & FP) RET;
}

// e9	jp (hl)		4
void nprE9(Z80CPU* cpu) {
	cpu->pc = cpu->hl;
}

// ea	jp pe,nn	4 3rd 3rd	mptr = nn
void nprEA(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FP) cpu->pc = cpu->mptr;
}

// eb	ex de,hl
void nprEB(Z80CPU* cpu) {
	SWAP(cpu->hl,cpu->de);
}

// ec	call pe,nn	4 3rd 3rd[+1] 3wr 3wr	mptr = nn
void nprEC(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FP) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// ed	prefix		4
void nprED(Z80CPU* cpu) {
	cpu->opTab = edTab;
}

// ee	xor n		4 3rd
void nprEE(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	cpu->a ^= cpu->tmpb;
	cpu->f = sz53pTab[cpu->a];
}

// ef	rst28		5 3wr 3wr	mptr = 0x28
void nprEF(Z80CPU* cpu) {
	RST(0x28);
}

// f0	ret p		5 [3rd 3rd]	[mptr = ret.adr]
void nprF0(Z80CPU* cpu) {
	if (!(cpu->f & FS)) RET;
}

// f1	pop af		4 3rd 3rd
void nprF1(Z80CPU* cpu) {
	POP(cpu->a,cpu->f);
}

// f2	jp p,nn		4 3rd 3rd	mptr = nn
void nprF2(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FS)) cpu->pc = cpu->mptr;
}

// f3	di		4
void nprF3(Z80CPU* cpu) {
	cpu->iff1 = 0;
	cpu->iff2 = 0;
}

// f4	call p,nn	4 3rd 3rd[+1] [3wr 3wr]		memptr = nn
void nprF4(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (!(cpu->f & FS)) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// f5	push af		5 3wr 3wr
void nprF5(Z80CPU* cpu) {
	PUSH(cpu->a,cpu->f);
}

// f6	or n		4 3rd
void nprF6(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	cpu->a |= cpu->tmpb;
	cpu->f = sz53pTab[cpu->a];
}

// f7	rst30		5 3wr 3wr		mptr = 0x30
void nprF7(Z80CPU* cpu) {
	RST(0x30);
}

// f8	ret m		5 [3rd 3rd]		[mptr = ret.adr]
void nprF8(Z80CPU* cpu) {
	if (cpu->f & FS) RET;
}

// f9	ld sp,hl	6
void nprF9(Z80CPU* cpu) {
	cpu->sp = cpu->hl;
}

// fa	jp m,nn		4 3rd 3rd		mptr = nn
void nprFA(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FS) cpu->pc = cpu->mptr;
}

// fb	ei		4
void nprFB(Z80CPU* cpu) {
	cpu->iff1 = 1;
	cpu->iff2 = 1;
	cpu->noint = 1;
}

// fc	call m,nn	4 3rd 3rd[+1] [3wr 3wr]		mptr = nn
void nprFC(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	if (cpu->f & FS) {
		cpu->t++;
		PUSH(cpu->hpc,cpu->lpc);
		cpu->pc = cpu->mptr;
	}
}

// fd	prefix IY	4
void nprFD(Z80CPU* cpu) {
	cpu->opTab = fdTab;
}

// fe	cp n		4 3rd
void nprFE(Z80CPU* cpu) {
	cpu->tmpb = MEMRD(cpu->pc++,3);
	CP(cpu->tmpb);
}

// ff	rst38		5 3rd 3rd	mptr = 0x38;
void nprFF(Z80CPU* cpu) {
	RST(0x38);
}

//==================

opCode npTab[256]={
	{0,4,0,0,0,0,&npr00,"nop"},
	{0,4,3,3,0,0,&npr01,"ld bc,:2"},
	{0,4,3,0,0,0,&npr02,"ld (bc),a"},
	{0,6,0,0,0,0,&npr03,"inc bc"},
	{0,4,0,0,0,0,&npr04,"inc b"},
	{0,4,0,0,0,0,&npr05,"dec b"},
	{0,4,3,0,0,0,&npr06,"ld b,:1"},
	{0,4,0,0,0,0,&npr07,"rlca"},

	{0,4,0,0,0,0,&npr08,"ex af,af'"},
	{0,11,0,0,0,0,&npr09,"add hl,bc"},
	{0,4,3,0,0,0,&npr0A,"ld a,(bc)"},
	{0,6,0,0,0,0,&npr0B,"dec bc"},
	{0,4,0,0,0,0,&npr0C,"inc c"},
	{0,4,0,0,0,0,&npr0D,"dec c"},
	{0,4,3,0,0,0,&npr0E,"ld c,:1"},
	{0,4,0,0,0,0,&npr0F,"rrca"},

	{0,5,3,5,0,0,&npr10,"djnz :3"},
	{0,4,3,3,0,0,&npr11,"ld de,:2"},
	{0,4,3,0,0,0,&npr12,"ld (de),a"},
	{0,6,0,0,0,0,&npr13,"inc de"},
	{0,4,0,0,0,0,&npr14,"inc d"},
	{0,4,0,0,0,0,&npr15,"dec d"},
	{0,4,3,0,0,0,&npr16,"ld d,:1"},
	{0,4,0,0,0,0,&npr17,"rla"},

	{0,4,3,5,0,0,&npr18,"jr :3"},
	{0,11,0,0,0,0,&npr19,"add hl,de"},
	{0,4,3,0,0,0,&npr1A,"ld a,(de)"},
	{0,6,0,0,0,0,&npr1B,"dec de"},
	{0,4,0,0,0,0,&npr1C,"inc e"},
	{0,4,0,0,0,0,&npr1D,"dec e"},
	{0,4,3,0,0,0,&npr1E,"ld e,:1"},
	{0,4,0,0,0,0,&npr1F,"rra"},

	{0,4,3,5,0,0,&npr20,"jr nz,:3"},
	{0,4,3,3,0,0,&npr21,"ld hl,:2"},
	{0,4,3,3,3,3,&npr22,"ld (:2),hl"},		// 4,3rd,3rd,3wr,3wr
	{0,6,0,0,0,0,&npr23,"inc hl"},
	{0,4,0,0,0,0,&npr24,"inc h"},
	{0,4,0,0,0,0,&npr25,"dec h"},
	{0,4,3,0,0,0,&npr26,"ld h,:1"},
	{0,4,0,0,0,0,&npr27,"daa"},

	{0,4,3,5,0,0,&npr28,"jr z,:3"},
	{0,11,0,0,0,0,&npr29,"add hl,hl"},
	{0,4,3,3,3,3,&npr2A,"ld hl,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,0,0,0,0,&npr2B,"dec hl"},
	{0,4,0,0,0,0,&npr2C,"inc l"},
	{0,4,0,0,0,0,&npr2D,"dec l"},
	{0,4,3,0,0,0,&npr2E,"ld l,:1"},
	{0,4,0,0,0,0,&npr2F,"cpl"},

	{0,4,3,5,0,0,&npr30,"jr nc,:3"},
	{0,4,3,3,0,0,&npr31,"ld sp,:2"},
	{0,4,3,3,3,0,&npr32,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,0,0,0,0,&npr33,"inc sp"},
	{0,4,3,4,0,0,&npr34,"inc (hl)"},
	{0,4,3,4,0,0,&npr35,"dec (hl)"},
	{0,4,3,3,0,0,&npr36,"ld (hl),:1"},
	{0,4,0,0,0,0,&npr37,"scf"},

	{0,4,3,5,0,0,&npr38,"jr c,:3"},
	{0,11,0,0,0,0,&npr39,"add hl,sp"},
	{0,4,3,3,3,0,&npr3A,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,0,0,0,0,&npr3B,"dec sp"},
	{0,4,0,0,0,0,&npr3C,"inc a"},
	{0,4,0,0,0,0,&npr3D,"dec a"},
	{0,4,3,0,0,0,&npr3E,"ld a,:1"},
	{0,4,0,0,0,0,&npr3F,"ccf"},

	{0,4,0,0,0,0,&npr40,"ld b,b"},
	{0,4,0,0,0,0,&npr41,"ld b,c"},
	{0,4,0,0,0,0,&npr42,"ld b,d"},
	{0,4,0,0,0,0,&npr43,"ld b,e"},
	{0,4,0,0,0,0,&npr44,"ld b,h"},
	{0,4,0,0,0,0,&npr45,"ld b,l"},
	{0,4,3,0,0,0,&npr46,"ld b,(hl)"},
	{0,4,0,0,0,0,&npr47,"ld b,a"},

	{0,4,0,0,0,0,&npr48,"ld c,b"},
	{0,4,0,0,0,0,&npr49,"ld c,c"},
	{0,4,0,0,0,0,&npr4A,"ld c,d"},
	{0,4,0,0,0,0,&npr4B,"ld c,e"},
	{0,4,0,0,0,0,&npr4C,"ld c,h"},
	{0,4,0,0,0,0,&npr4D,"ld c,l"},
	{0,4,3,0,0,0,&npr4E,"ld c,(hl)"},
	{0,4,0,0,0,0,&npr4F,"ld c,a"},

	{0,4,0,0,0,0,&npr50,"ld d,b"},
	{0,4,0,0,0,0,&npr51,"ld d,c"},
	{0,4,0,0,0,0,&npr52,"ld d,d"},
	{0,4,0,0,0,0,&npr53,"ld d,e"},
	{0,4,0,0,0,0,&npr54,"ld d,h"},
	{0,4,0,0,0,0,&npr55,"ld d,l"},
	{0,4,3,0,0,0,&npr56,"ld d,(hl)"},
	{0,4,0,0,0,0,&npr57,"ld d,a"},

	{0,4,0,0,0,0,&npr58,"ld e,b"},
	{0,4,0,0,0,0,&npr59,"ld e,c"},
	{0,4,0,0,0,0,&npr5A,"ld e,d"},
	{0,4,0,0,0,0,&npr5B,"ld e,e"},
	{0,4,0,0,0,0,&npr5C,"ld e,h"},
	{0,4,0,0,0,0,&npr5D,"ld e,l"},
	{0,4,3,0,0,0,&npr5E,"ld e,(hl)"},
	{0,4,0,0,0,0,&npr5F,"ld e,a"},

	{0,4,0,0,0,0,&npr60,"ld h,b"},
	{0,4,0,0,0,0,&npr61,"ld h,c"},
	{0,4,0,0,0,0,&npr62,"ld h,d"},
	{0,4,0,0,0,0,&npr63,"ld h,e"},
	{0,4,0,0,0,0,&npr64,"ld h,h"},
	{0,4,0,0,0,0,&npr65,"ld h,l"},
	{0,4,3,0,0,0,&npr66,"ld h,(hl)"},
	{0,4,0,0,0,0,&npr67,"ld h,a"},

	{0,4,0,0,0,0,&npr68,"ld l,b"},
	{0,4,0,0,0,0,&npr69,"ld l,c"},
	{0,4,0,0,0,0,&npr6A,"ld l,d"},
	{0,4,0,0,0,0,&npr6B,"ld l,e"},
	{0,4,0,0,0,0,&npr6C,"ld l,h"},
	{0,4,0,0,0,0,&npr6D,"ld l,l"},
	{0,4,3,0,0,0,&npr6E,"ld l,(hl)"},
	{0,4,0,0,0,0,&npr6F,"ld l,a"},

	{0,4,3,0,0,0,&npr70,"ld (hl),b"},
	{0,4,3,0,0,0,&npr71,"ld (hl),c"},
	{0,4,3,0,0,0,&npr72,"ld (hl),d"},
	{0,4,3,0,0,0,&npr73,"ld (hl),e"},
	{0,4,3,0,0,0,&npr74,"ld (hl),h"},
	{0,4,3,0,0,0,&npr75,"ld (hl),l"},
	{0,4,0,0,0,0,&npr76,"halt"},
	{0,4,3,0,0,0,&npr77,"ld (hl),a"},

	{0,4,0,0,0,0,&npr78,"ld a,b"},
	{0,4,0,0,0,0,&npr79,"ld a,c"},
	{0,4,0,0,0,0,&npr7A,"ld a,d"},
	{0,4,0,0,0,0,&npr7B,"ld a,e"},
	{0,4,0,0,0,0,&npr7C,"ld a,h"},
	{0,4,0,0,0,0,&npr7D,"ld a,l"},
	{0,4,3,0,0,0,&npr7E,"ld a,(hl)"},
	{0,4,0,0,0,0,&npr7F,"ld a,a"},

	{0,4,0,0,0,0,&npr80,"add a,b"},
	{0,4,0,0,0,0,&npr81,"add a,c"},
	{0,4,0,0,0,0,&npr82,"add a,d"},
	{0,4,0,0,0,0,&npr83,"add a,e"},
	{0,4,0,0,0,0,&npr84,"add a,h"},
	{0,4,0,0,0,0,&npr85,"add a,l"},
	{0,4,3,0,0,0,&npr86,"add a,(hl)"},
	{0,4,0,0,0,0,&npr87,"add a,a"},

	{0,4,0,0,0,0,&npr88,"adc a,b"},
	{0,4,0,0,0,0,&npr89,"adc a,c"},
	{0,4,0,0,0,0,&npr8A,"adc a,d"},
	{0,4,0,0,0,0,&npr8B,"adc a,e"},
	{0,4,0,0,0,0,&npr8C,"adc a,h"},
	{0,4,0,0,0,0,&npr8D,"adc a,l"},
	{0,4,3,0,0,0,&npr8E,"adc a,(hl)"},
	{0,4,0,0,0,0,&npr8F,"adc a,a"},

	{0,4,0,0,0,0,&npr90,"sub b"},
	{0,4,0,0,0,0,&npr91,"sub c"},
	{0,4,0,0,0,0,&npr92,"sub d"},
	{0,4,0,0,0,0,&npr93,"sub e"},
	{0,4,0,0,0,0,&npr94,"sub h"},
	{0,4,0,0,0,0,&npr95,"sub l"},
	{0,4,3,0,0,0,&npr96,"sub (hl)"},
	{0,4,0,0,0,0,&npr97,"sub a"},

	{0,4,0,0,0,0,&npr98,"sbc a,b"},
	{0,4,0,0,0,0,&npr99,"sbc a,c"},
	{0,4,0,0,0,0,&npr9A,"sbc a,d"},
	{0,4,0,0,0,0,&npr9B,"sbc a,e"},
	{0,4,0,0,0,0,&npr9C,"sbc a,h"},
	{0,4,0,0,0,0,&npr9D,"sbc a,l"},
	{0,4,3,0,0,0,&npr9E,"sbc a,(hl)"},
	{0,4,0,0,0,0,&npr9F,"sbc a,a"},

	{0,4,0,0,0,0,&nprA0,"and b"},
	{0,4,0,0,0,0,&nprA1,"and c"},
	{0,4,0,0,0,0,&nprA2,"and d"},
	{0,4,0,0,0,0,&nprA3,"and e"},
	{0,4,0,0,0,0,&nprA4,"and h"},
	{0,4,0,0,0,0,&nprA5,"and l"},
	{0,4,3,0,0,0,&nprA6,"and (hl)"},
	{0,4,0,0,0,0,&nprA7,"and a"},

	{0,4,0,0,0,0,&nprA8,"xor b"},
	{0,4,0,0,0,0,&nprA9,"xor c"},
	{0,4,0,0,0,0,&nprAA,"xor d"},
	{0,4,0,0,0,0,&nprAB,"xor e"},
	{0,4,0,0,0,0,&nprAC,"xor h"},
	{0,4,0,0,0,0,&nprAD,"xor l"},
	{0,4,3,0,0,0,&nprAE,"xor (hl)"},
	{0,4,0,0,0,0,&nprAF,"xor a"},

	{0,4,0,0,0,0,&nprB0,"or b"},
	{0,4,0,0,0,0,&nprB1,"or c"},
	{0,4,0,0,0,0,&nprB2,"or d"},
	{0,4,0,0,0,0,&nprB3,"or e"},
	{0,4,0,0,0,0,&nprB4,"or h"},
	{0,4,0,0,0,0,&nprB5,"or l"},
	{0,4,3,0,0,0,&nprB6,"or (hl)"},
	{0,4,0,0,0,0,&nprB7,"or a"},

	{0,4,0,0,0,0,&nprB8,"cp b"},
	{0,4,0,0,0,0,&nprB9,"cp c"},
	{0,4,0,0,0,0,&nprBA,"cp d"},
	{0,4,0,0,0,0,&nprBB,"cp e"},
	{0,4,0,0,0,0,&nprBC,"cp h"},
	{0,4,0,0,0,0,&nprBD,"cp l"},
	{0,4,3,0,0,0,&nprBE,"cp (hl)"},
	{0,4,0,0,0,0,&nprBF,"cp a"},

	{0,5,3,3,0,0,&nprC0,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,3,3,0,0,&nprC1,"pop bc"},
	{0,4,3,3,0,0,&nprC2,"jp nz,:2"},
	{0,4,3,3,0,0,&nprC3,"jp :2"},
	{0,4,3,4,3,3,&nprC4,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,3,3,0,0,&nprC5,"push bc"},		// 5 3wr 3wr
	{0,4,3,0,0,0,&nprC6,"add a,:1"},
	{0,5,3,3,0,0,&nprC7,"rst #00"},		// 5 3wr 3wr

	{0,5,3,3,0,0,&nprC8,"ret z"},
	{0,4,3,3,0,0,&nprC9,"ret"},
	{0,4,3,3,0,0,&nprCA,"jp z,:2"},
	{1,4,0,0,0,0,&nprCB,"#CB"},
	{0,4,3,4,3,3,&nprCC,"call z,:2"},
	{0,4,3,4,3,3,&nprCD,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,3,0,0,0,&nprCE,"adc a,:1"},
	{0,5,3,3,0,0,&nprCF,"rst #08"},

	{0,5,3,3,0,0,&nprD0,"ret nc"},
	{0,4,3,3,0,0,&nprD1,"pop de"},
	{0,4,3,3,0,0,&nprD2,"jp nc,:2"},
	{0,4,3,4,0,0,&nprD3,"out (:1),a"},
	{0,4,3,4,3,3,&nprD4,"call nc,:2"},
	{0,5,3,3,0,0,&nprD5,"push de"},
	{0,4,3,0,0,0,&nprD6,"sub :1"},
	{0,5,3,3,0,0,&nprD7,"rst #10"},

	{0,5,3,3,0,0,&nprD8,"ret c"},
	{0,4,0,0,0,0,&nprD9,"exx"},
	{0,4,3,3,0,0,&nprDA,"jp c,:2"},
	{0,4,3,4,0,0,&nprDB,"in a,(:1)"},
	{0,4,3,4,3,3,&nprDC,"call c,:2"},
	{1,4,0,0,0,0,&nprDD,"#DD"},
	{0,4,3,0,0,0,&nprDE,"sbc a,:1"},
	{0,5,3,3,0,0,&nprDF,"rst #18"},

	{0,5,3,3,0,0,&nprE0,"ret po"},
	{0,4,3,3,0,0,&nprE1,"pop hl"},
	{0,4,3,3,0,0,&nprE2,"jp po,:2"},
	{0,4,3,4,3,5,&nprE3,"ex (sp),hl"},		// 4 3rd 4rd 3wr 5wr
	{0,4,6,7,0,0,&nprE4,"call po,:2"},
	{0,5,3,3,0,0,&nprE5,"push hl"},
	{0,4,3,0,0,0,&nprE6,"and :1"},
	{0,5,3,3,0,0,&nprE7,"rst #20"},

	{0,5,3,3,0,0,&nprE8,"ret pe"},
	{0,4,0,0,0,0,&nprE9,"jp (hl)"},
	{0,4,3,3,0,0,&nprEA,"jp pe,:2"},
	{0,4,0,0,0,0,&nprEB,"ex de,hl"},
	{0,4,3,4,3,3,&nprEC,"call pe,:2"},
	{1,4,0,0,0,0,&nprED,"#ED"},
	{0,4,3,0,0,0,&nprEE,"xor :1"},
	{0,5,3,3,0,0,&nprEF,"rst #28"},

	{0,5,3,3,0,0,&nprF0,"ret p"},
	{0,4,3,3,0,0,&nprF1,"pop af"},
	{0,4,3,3,0,0,&nprF2,"jp p,:2"},
	{0,4,0,0,0,0,&nprF3,"di"},
	{0,4,3,4,3,3,&nprF4,"call p,:2"},
	{0,5,3,3,0,0,&nprF5,"push af"},
	{0,4,3,0,0,0,&nprF6,"or :1"},
	{0,5,3,3,0,0,&nprF7,"rst #30"},

	{0,5,3,3,0,0,&nprF8,"ret m"},
	{0,6,0,0,0,0,&nprF9,"ld sp,hl"},
	{0,4,3,3,0,0,&nprFA,"jp m,:2"},
	{0,4,0,0,0,0,&nprFB,"ei"},
	{0,4,3,4,3,3,&nprFC,"call m,:2"},
	{1,4,0,0,0,0,&nprFD,"#FD"},
	{0,4,3,3,0,0,&nprFE,"cp :1"},
	{0,5,3,3,0,0,&nprFF,"rst #38"}
};