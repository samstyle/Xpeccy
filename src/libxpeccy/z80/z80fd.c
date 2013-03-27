
// 09	add iy,bc	11	mptr = iy+1 before adding
void fd09(Z80CPU* cpu) {
	ADD16(cpu->iy, cpu->bc);
}

// 19	add iy,de	11	mptr = iy+1 before adding
void fd19(Z80CPU* cpu) {
	ADD16(cpu->iy,cpu->de);
}

// 21	ld iy,nn	4 3rd 3rd
void fd21(Z80CPU* cpu) {
	cpu->ly = MEMRD(cpu->pc++,3);
	cpu->hy = MEMRD(cpu->pc++,3);
}

// 22	ld (nn),iy	4 3rd 3rd 3wr 3wr	mptr = nn+1
void fd22(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->ly,3);
	MEMWR(cpu->mptr,cpu->hy,3);
}

// 23	inc iy		6
void fd23(Z80CPU* cpu) {
	cpu->iy++;
}

// 24	inc hy		4
void fd24(Z80CPU* cpu) {
	INC(cpu->hy);
}

// 25	dec hy		4
void fd25(Z80CPU* cpu) {
	DEC(cpu->hy);
}

// 26	ld hy,n		4 3rd
void fd26(Z80CPU* cpu) {
	cpu->hy = MEMRD(cpu->pc++,3);
}

// 29	add iy,iy	11
void fd29(Z80CPU* cpu) {
	ADD16(cpu->iy,cpu->iy);
}

// 2A	ld iy,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void fd2A(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->ly = MEMRD(cpu->mptr++,3);
	cpu->hy = MEMRD(cpu->mptr,3);
}

// 2B	dec iy		6
void fd2B(Z80CPU* cpu) {
	cpu->iy--;
}

// 2C	inc ly		4
void fd2C(Z80CPU* cpu) {
	INC(cpu->ly);
}

// 2D	dec ly		4
void fd2D(Z80CPU* cpu) {
	DEC(cpu->ly);
}

// 2E	ld ly,n		4 3rd
void fd2E(Z80CPU* cpu) {
	cpu->ly = MEMRD(cpu->pc++,3);
}

// 34	inc (iy+e)	4 3rd 5add 4rd 3wr
void fd34(Z80CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->mptr,4);
	INC(cpu->tmp);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 35	dec (iy+e)	4 3rd 5add 4rd 3wr	mptr = iy+e
void fd35(Z80CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->mptr,4);
	DEC(cpu->tmp);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 36	ld (iy+e),n	4 3rd {5add 3rd} 3wr	mptr = iy+e
void fd36(Z80CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->pc++,0);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 39	add iy,sp	11
void fd39(Z80CPU* cpu) {
	ADD16(cpu->iy,cpu->sp);
}

// ld r,r		4 [3rd 5add 3rd]
void fd44(Z80CPU* cpu) {cpu->b = cpu->hy;}
void fd45(Z80CPU* cpu) {cpu->b = cpu->ly;}
void fd46(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->b = MEMRD(cpu->mptr,3);}
void fd4C(Z80CPU* cpu) {cpu->c = cpu->hy;}
void fd4D(Z80CPU* cpu) {cpu->c = cpu->ly;}
void fd4E(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->c = MEMRD(cpu->mptr,3);}
void fd54(Z80CPU* cpu) {cpu->d = cpu->hy;}
void fd55(Z80CPU* cpu) {cpu->d = cpu->ly;}
void fd56(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->d = MEMRD(cpu->mptr,3);}
void fd5C(Z80CPU* cpu) {cpu->e = cpu->hy;}
void fd5D(Z80CPU* cpu) {cpu->e = cpu->ly;}
void fd5E(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->e = MEMRD(cpu->mptr,3);}

void fd60(Z80CPU* cpu) {cpu->hy = cpu->b;}
void fd61(Z80CPU* cpu) {cpu->hy = cpu->c;}
void fd62(Z80CPU* cpu) {cpu->hy = cpu->d;}
void fd63(Z80CPU* cpu) {cpu->hy = cpu->e;}
void fd64(Z80CPU* cpu) {}
void fd65(Z80CPU* cpu) {cpu->hy = cpu->ly;}
void fd66(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->h = MEMRD(cpu->mptr,3);}
void fd67(Z80CPU* cpu) {cpu->hy = cpu->a;}

void fd68(Z80CPU* cpu) {cpu->ly = cpu->b;}
void fd69(Z80CPU* cpu) {cpu->ly = cpu->c;}
void fd6A(Z80CPU* cpu) {cpu->ly = cpu->d;}
void fd6B(Z80CPU* cpu) {cpu->ly = cpu->e;}
void fd6C(Z80CPU* cpu) {cpu->ly = cpu->hy;}
void fd6D(Z80CPU* cpu) {}
void fd6E(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->l = MEMRD(cpu->mptr,3);}
void fd6F(Z80CPU* cpu) {cpu->ly = cpu->a;}
// 70..77	ld (iy+e),r	4 3rd 5add 3wr
void fd70(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->b,3);}
void fd71(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->c,3);}
void fd72(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->d,3);}
void fd73(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->e,3);}
void fd74(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->h,3);}
void fd75(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->l,3);}
void fd77(Z80CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->a,3);}

void fd7C(Z80CPU* cpu) {cpu->a = cpu->hy;}
void fd7D(Z80CPU* cpu) {cpu->a = cpu->ly;}
void fd7E(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->a = MEMRD(cpu->mptr,3);}

// add x
void fd84(Z80CPU* cpu) {ADD(cpu->hy);}
void fd85(Z80CPU* cpu) {ADD(cpu->ly);}
void fd86(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); ADD(cpu->tmpb);}
// adc x
void fd8C(Z80CPU* cpu) {ADC(cpu->hy);}
void fd8D(Z80CPU* cpu) {ADC(cpu->ly);}
void fd8E(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); ADC(cpu->tmpb);}
// sub x
void fd94(Z80CPU* cpu) {SUB(cpu->hy);}
void fd95(Z80CPU* cpu) {SUB(cpu->ly);}
void fd96(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); SUB(cpu->tmpb);}
// sbc x
void fd9C(Z80CPU* cpu) {SBC(cpu->hy);}
void fd9D(Z80CPU* cpu) {SBC(cpu->ly);}
void fd9E(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); SBC(cpu->tmpb);}
// and x
void fdA4(Z80CPU* cpu) {cpu->a &= cpu->hy; cpu->f = sz53pTab[cpu->a] | FH;}
void fdA5(Z80CPU* cpu) {cpu->a &= cpu->ly; cpu->f = sz53pTab[cpu->a] | FH;}
void fdA6(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a &= cpu->tmpb; cpu->f = sz53pTab[cpu->a] | FH;}
// xor x
void fdAC(Z80CPU* cpu) {cpu->a ^= cpu->hy; cpu->f = sz53pTab[cpu->a];}
void fdAD(Z80CPU* cpu) {cpu->a ^= cpu->ly; cpu->f = sz53pTab[cpu->a];}
void fdAE(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a ^= cpu->tmpb; cpu->f = sz53pTab[cpu->a];}
// or x
void fdB4(Z80CPU* cpu) {cpu->a |= cpu->hy; cpu->f = sz53pTab[cpu->a];}
void fdB5(Z80CPU* cpu) {cpu->a |= cpu->ly; cpu->f = sz53pTab[cpu->a];}
void fdB6(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a |= cpu->tmpb; cpu->f = sz53pTab[cpu->a];}
// cp x
void fdBC(Z80CPU* cpu) {CP(cpu->hy);}
void fdBD(Z80CPU* cpu) {CP(cpu->ly);}
void fdBE(Z80CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); CP(cpu->tmpb);}

// cb	fdcb prefix	4 3rd
void fdCB(Z80CPU* cpu) {
	cpu->opTab = fdcbTab;
	cpu->tmp = MEMRD(cpu->pc++,3);
	cpu->tmpb = MEMRD(cpu->pc++,0);	// opcode. eat 0T? not m1
	cpu->op = &fdcbTab[cpu->tmpb];
	cpu->op->exec(cpu);
}

// e1	pop iy		4 3rd 3rd
void fdE1(Z80CPU* cpu) {
	POP(cpu->hy,cpu->ly);
}

// e3	ex (sp),iy	4 3rd 4rd 3wr 5wr	mptr = iy
void fdE3(Z80CPU* cpu) {
	POP(cpu->htw,cpu->ltw); cpu->t++;	// 3,3+1
	PUSH(cpu->hy, cpu->ly); cpu->t += 2;	// 3,3+2
	cpu->iy = cpu->tmpw;
	cpu->mptr = cpu->iy;
}

// e5	push iy		5 3wr 3wr
void fdE5(Z80CPU* cpu) {
	PUSH(cpu->hy,cpu->ly);
}

// e9	jp (iy)		4
void fdE9(Z80CPU* cpu) {
	cpu->pc = cpu->iy;
}

// f9	ld sp,iy	6
void fdF9(Z80CPU* cpu) {
	cpu->sp = cpu->iy;
}

// ======

opCode fdTab[256]={
	{0,4,0,0,0,0,&npr00,NULL,"nop"},
	{0,4,3,3,0,0,&npr01,NULL,"ld bc,:2"},
	{0,4,3,0,0,0,&npr02,NULL,"ld (bc),a"},
	{0,6,0,0,0,0,&npr03,NULL,"inc bc"},
	{0,4,0,0,0,0,&npr04,NULL,"inc b"},
	{0,4,0,0,0,0,&npr05,NULL,"dec b"},
	{0,4,3,0,0,0,&npr06,NULL,"ld b,:1"},
	{0,4,0,0,0,0,&npr07,NULL,"rlca"},

	{0,4,0,0,0,0,&npr08,NULL,"ex af,af'"},
	{0,11,0,0,0,0,&fd09,NULL,"add iy,bc"},
	{0,4,3,0,0,0,&npr0A,NULL,"ld a,(bc)"},
	{0,6,0,0,0,0,&npr0B,NULL,"dec bc"},
	{0,4,0,0,0,0,&npr0C,NULL,"inc c"},
	{0,4,0,0,0,0,&npr0D,NULL,"dec c"},
	{0,4,3,0,0,0,&npr0E,NULL,"ld c,:1"},
	{0,4,0,0,0,0,&npr0F,NULL,"rrca"},

	{0,5,3,5,0,0,&npr10,NULL,"djnz :3"},
	{0,4,3,3,0,0,&npr11,NULL,"ld de,:2"},
	{0,4,3,0,0,0,&npr12,NULL,"ld (de),a"},
	{0,6,0,0,0,0,&npr13,NULL,"inc de"},
	{0,4,0,0,0,0,&npr14,NULL,"inc d"},
	{0,4,0,0,0,0,&npr15,NULL,"dec d"},
	{0,4,3,0,0,0,&npr16,NULL,"ld d,:1"},
	{0,4,0,0,0,0,&npr17,NULL,"rla"},

	{0,4,3,5,0,0,&npr18,NULL,"jr :3"},
	{0,11,0,0,0,0,&fd19,NULL,"add iy,de"},
	{0,4,3,0,0,0,&npr1A,NULL,"ld a,(de)"},
	{0,6,0,0,0,0,&npr1B,NULL,"dec de"},
	{0,4,0,0,0,0,&npr1C,NULL,"inc e"},
	{0,4,0,0,0,0,&npr1D,NULL,"dec e"},
	{0,4,3,0,0,0,&npr1E,NULL,"ld e,:1"},
	{0,4,0,0,0,0,&npr1F,NULL,"rra"},

	{0,4,3,5,0,0,&npr20,NULL,"jr nz,:3"},
	{0,4,3,3,0,0,&fd21,NULL,"ld iy,:2"},
	{0,4,3,3,3,3,&fd22,NULL,"ld (:2),iy"},		// 4,3rd,3rd,3wr,3wr
	{0,6,0,0,0,0,&fd23,NULL,"inc iy"},
	{0,4,0,0,0,0,&fd24,NULL,"inc hy"},
	{0,4,0,0,0,0,&fd25,NULL,"dec hy"},
	{0,4,3,0,0,0,&fd26,NULL,"ld hy,:1"},
	{0,4,0,0,0,0,&npr27,NULL,"daa"},

	{0,4,3,5,0,0,&npr28,NULL,"jr z,:3"},
	{0,11,0,0,0,0,&fd29,NULL,"add iy,iy"},
	{0,4,3,3,3,3,&fd2A,NULL,"ld iy,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,0,0,0,0,&fd2B,NULL,"dec iy"},
	{0,4,0,0,0,0,&fd2C,NULL,"inc ly"},
	{0,4,0,0,0,0,&fd2D,NULL,"dec ly"},
	{0,4,3,0,0,0,&fd2E,NULL,"ld ly,:1"},
	{0,4,0,0,0,0,&npr2F,NULL,"cpl"},

	{0,4,3,5,0,0,&npr30,NULL,"jr nc,:3"},
	{0,4,3,3,0,0,&npr31,NULL,"ld sp,:2"},
	{0,4,3,3,3,0,&npr32,NULL,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,0,0,0,0,&npr33,NULL,"inc sp"},
	{0,4,3,4,0,0,&fd34,NULL,"inc (iy:4)"},
	{0,4,3,4,0,0,&fd35,NULL,"dec (iy:4)"},
	{0,4,3,3,0,0,&fd36,NULL,"ld (iy:4),:1"},
	{0,4,0,0,0,0,&npr37,NULL,"scf"},

	{0,4,3,5,0,0,&npr38,NULL,"jr c,:3"},
	{0,11,0,0,0,0,&fd39,NULL,"add iy,sp"},
	{0,4,3,3,3,0,&npr3A,NULL,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,0,0,0,0,&npr3B,NULL,"dec sp"},
	{0,4,0,0,0,0,&npr3C,NULL,"inc a"},
	{0,4,0,0,0,0,&npr3D,NULL,"dec a"},
	{0,4,3,0,0,0,&npr3E,NULL,"ld a,:1"},
	{0,4,0,0,0,0,&npr3F,NULL,"ccf"},

	{0,4,0,0,0,0,&npr40,NULL,"ld b,b"},
	{0,4,0,0,0,0,&npr41,NULL,"ld b,c"},
	{0,4,0,0,0,0,&npr42,NULL,"ld b,d"},
	{0,4,0,0,0,0,&npr43,NULL,"ld b,e"},
	{0,4,0,0,0,0,&fd44,NULL,"ld b,hy"},
	{0,4,0,0,0,0,&fd45,NULL,"ld b,ly"},
	{0,4,3,0,0,0,&fd46,NULL,"ld b,(iy:4)"},
	{0,4,0,0,0,0,&npr47,NULL,"ld b,a"},

	{0,4,0,0,0,0,&npr48,NULL,"ld c,b"},
	{0,4,0,0,0,0,&npr49,NULL,"ld c,c"},
	{0,4,0,0,0,0,&npr4A,NULL,"ld c,d"},
	{0,4,0,0,0,0,&npr4B,NULL,"ld c,e"},
	{0,4,0,0,0,0,&fd4C,NULL,"ld c,hy"},
	{0,4,0,0,0,0,&fd4D,NULL,"ld c,ly"},
	{0,4,3,0,0,0,&fd4E,NULL,"ld c,(iy:4)"},
	{0,4,0,0,0,0,&npr4F,NULL,"ld c,a"},

	{0,4,0,0,0,0,&npr50,NULL,"ld d,b"},
	{0,4,0,0,0,0,&npr51,NULL,"ld d,c"},
	{0,4,0,0,0,0,&npr52,NULL,"ld d,d"},
	{0,4,0,0,0,0,&npr53,NULL,"ld d,e"},
	{0,4,0,0,0,0,&fd54,NULL,"ld d,hy"},
	{0,4,0,0,0,0,&fd55,NULL,"ld d,ly"},
	{0,4,3,0,0,0,&fd56,NULL,"ld d,(iy:4)"},
	{0,4,0,0,0,0,&npr57,NULL,"ld d,a"},

	{0,4,0,0,0,0,&npr58,NULL,"ld e,b"},
	{0,4,0,0,0,0,&npr59,NULL,"ld e,c"},
	{0,4,0,0,0,0,&npr5A,NULL,"ld e,d"},
	{0,4,0,0,0,0,&npr5B,NULL,"ld e,e"},
	{0,4,0,0,0,0,&fd5C,NULL,"ld e,hy"},
	{0,4,0,0,0,0,&fd5D,NULL,"ld e,ly"},
	{0,4,3,0,0,0,&fd5E,NULL,"ld e,(iy:4)"},
	{0,4,0,0,0,0,&npr5F,NULL,"ld e,a"},

	{0,4,0,0,0,0,&fd60,NULL,"ld hy,b"},
	{0,4,0,0,0,0,&fd61,NULL,"ld hy,c"},
	{0,4,0,0,0,0,&fd62,NULL,"ld hy,d"},
	{0,4,0,0,0,0,&fd63,NULL,"ld hy,e"},
	{0,4,0,0,0,0,&fd64,NULL,"ld hy,hy"},
	{0,4,0,0,0,0,&fd65,NULL,"ld hy,ly"},
	{0,4,3,0,0,0,&fd66,NULL,"ld h,(iy:4)"},
	{0,4,0,0,0,0,&fd67,NULL,"ld hy,a"},

	{0,4,0,0,0,0,&fd68,NULL,"ld ly,b"},
	{0,4,0,0,0,0,&fd69,NULL,"ld ly,c"},
	{0,4,0,0,0,0,&fd6A,NULL,"ld ly,d"},
	{0,4,0,0,0,0,&fd6B,NULL,"ld ly,e"},
	{0,4,0,0,0,0,&fd6C,NULL,"ld ly,hy"},
	{0,4,0,0,0,0,&fd6D,NULL,"ld ly,ly"},
	{0,4,3,0,0,0,&fd6E,NULL,"ld l,(iy:4)"},
	{0,4,0,0,0,0,&fd6F,NULL,"ld ly,a"},

	{0,4,3,0,0,0,&fd70,NULL,"ld (iy:4),b"},
	{0,4,3,0,0,0,&fd71,NULL,"ld (iy:4),c"},
	{0,4,3,0,0,0,&fd72,NULL,"ld (iy:4),d"},
	{0,4,3,0,0,0,&fd73,NULL,"ld (iy:4),e"},
	{0,4,3,0,0,0,&fd74,NULL,"ld (iy:4),h"},
	{0,4,3,0,0,0,&fd75,NULL,"ld (iy:4),l"},
	{0,4,0,0,0,0,&npr76,NULL,"halt"},
	{0,4,3,0,0,0,&fd77,NULL,"ld (iy:4),a"},

	{0,4,0,0,0,0,&npr78,NULL,"ld a,b"},
	{0,4,0,0,0,0,&npr79,NULL,"ld a,c"},
	{0,4,0,0,0,0,&npr7A,NULL,"ld a,d"},
	{0,4,0,0,0,0,&npr7B,NULL,"ld a,e"},
	{0,4,0,0,0,0,&fd7C,NULL,"ld a,hy"},
	{0,4,0,0,0,0,&fd7D,NULL,"ld a,ly"},
	{0,4,3,0,0,0,&fd7E,NULL,"ld a,(iy:4)"},
	{0,4,0,0,0,0,&npr7F,NULL,"ld a,a"},

	{0,4,0,0,0,0,&npr80,NULL,"add a,b"},
	{0,4,0,0,0,0,&npr81,NULL,"add a,c"},
	{0,4,0,0,0,0,&npr82,NULL,"add a,d"},
	{0,4,0,0,0,0,&npr83,NULL,"add a,e"},
	{0,4,0,0,0,0,&fd84,NULL,"add a,hy"},
	{0,4,0,0,0,0,&fd85,NULL,"add a,ly"},
	{0,4,3,0,0,0,&fd86,NULL,"add a,(iy:4)"},
	{0,4,0,0,0,0,&npr87,NULL,"add a,a"},

	{0,4,0,0,0,0,&npr88,NULL,"adc a,b"},
	{0,4,0,0,0,0,&npr89,NULL,"adc a,c"},
	{0,4,0,0,0,0,&npr8A,NULL,"adc a,d"},
	{0,4,0,0,0,0,&npr8B,NULL,"adc a,e"},
	{0,4,0,0,0,0,&fd8C,NULL,"adc a,hy"},
	{0,4,0,0,0,0,&fd8D,NULL,"adc a,ly"},
	{0,4,3,0,0,0,&fd8E,NULL,"adc a,(iy:4)"},
	{0,4,0,0,0,0,&npr8F,NULL,"adc a,a"},

	{0,4,0,0,0,0,&npr90,NULL,"sub b"},
	{0,4,0,0,0,0,&npr91,NULL,"sub c"},
	{0,4,0,0,0,0,&npr92,NULL,"sub d"},
	{0,4,0,0,0,0,&npr93,NULL,"sub e"},
	{0,4,0,0,0,0,&fd94,NULL,"sub hy"},
	{0,4,0,0,0,0,&fd95,NULL,"sub ly"},
	{0,4,3,0,0,0,&fd96,NULL,"sub (iy:4)"},
	{0,4,0,0,0,0,&npr97,NULL,"sub a"},

	{0,4,0,0,0,0,&npr98,NULL,"sbc a,b"},
	{0,4,0,0,0,0,&npr99,NULL,"sbc a,c"},
	{0,4,0,0,0,0,&npr9A,NULL,"sbc a,d"},
	{0,4,0,0,0,0,&npr9B,NULL,"sbc a,e"},
	{0,4,0,0,0,0,&fd9C,NULL,"sbc a,hy"},
	{0,4,0,0,0,0,&fd9D,NULL,"sbc a,ly"},
	{0,4,3,0,0,0,&fd9E,NULL,"sbc a,(iy:4)"},
	{0,4,0,0,0,0,&npr9F,NULL,"sbc a,a"},

	{0,4,0,0,0,0,&nprA0,NULL,"and b"},
	{0,4,0,0,0,0,&nprA1,NULL,"and c"},
	{0,4,0,0,0,0,&nprA2,NULL,"and d"},
	{0,4,0,0,0,0,&nprA3,NULL,"and e"},
	{0,4,0,0,0,0,&fdA4,NULL,"and hy"},
	{0,4,0,0,0,0,&fdA5,NULL,"and ly"},
	{0,4,3,0,0,0,&fdA6,NULL,"and (iy:4)"},
	{0,4,0,0,0,0,&nprA7,NULL,"and a"},

	{0,4,0,0,0,0,&nprA8,NULL,"xor b"},
	{0,4,0,0,0,0,&nprA9,NULL,"xor c"},
	{0,4,0,0,0,0,&nprAA,NULL,"xor d"},
	{0,4,0,0,0,0,&nprAB,NULL,"xor e"},
	{0,4,0,0,0,0,&fdAC,NULL,"xor hy"},
	{0,4,0,0,0,0,&fdAD,NULL,"xor ly"},
	{0,4,3,0,0,0,&fdAE,NULL,"xor (iy:4)"},
	{0,4,0,0,0,0,&nprAF,NULL,"xor a"},

	{0,4,0,0,0,0,&nprB0,NULL,"or b"},
	{0,4,0,0,0,0,&nprB1,NULL,"or c"},
	{0,4,0,0,0,0,&nprB2,NULL,"or d"},
	{0,4,0,0,0,0,&nprB3,NULL,"or e"},
	{0,4,0,0,0,0,&fdB4,NULL,"or hy"},
	{0,4,0,0,0,0,&fdB5,NULL,"or ly"},
	{0,4,3,0,0,0,&fdB6,NULL,"or (iy:4)"},
	{0,4,0,0,0,0,&nprB7,NULL,"or a"},

	{0,4,0,0,0,0,&nprB8,NULL,"cp b"},
	{0,4,0,0,0,0,&nprB9,NULL,"cp c"},
	{0,4,0,0,0,0,&nprBA,NULL,"cp d"},
	{0,4,0,0,0,0,&nprBB,NULL,"cp e"},
	{0,4,0,0,0,0,&fdBC,NULL,"cp hy"},
	{0,4,0,0,0,0,&fdBD,NULL,"cp ly"},
	{0,4,3,0,0,0,&fdBE,NULL,"cp (iy:4)"},
	{0,4,0,0,0,0,&nprBF,NULL,"cp a"},

	{0,5,3,3,0,0,&nprC0,NULL,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,3,3,0,0,&nprC1,NULL,"pop bc"},
	{0,4,3,3,0,0,&nprC2,NULL,"jp nz,:2"},
	{0,4,3,3,0,0,&nprC3,NULL,"jp :2"},
	{0,4,3,4,3,3,&nprC4,NULL,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,3,3,0,0,&nprC5,NULL,"push bc"},		// 5 3wr 3wr
	{0,4,3,0,0,0,&nprC6,NULL,"add a,:1"},
	{0,5,3,3,0,0,&nprC7,NULL,"rst #00"},		// 5 3wr 3wr

	{0,5,3,3,0,0,&nprC8,NULL,"ret z"},
	{0,4,3,3,0,0,&nprC9,NULL,"ret"},
	{0,4,3,3,0,0,&nprCA,NULL,"jp z,:2"},
	{1,4,0,0,0,0,&fdCB,fdcbTab,"#CB"},
	{0,4,3,4,3,3,&nprCC,NULL,"call z,:2"},
	{0,4,3,4,3,3,&nprCD,NULL,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,3,0,0,0,&nprCE,NULL,"adc a,:1"},
	{0,5,3,3,0,0,&nprCF,NULL,"rst #08"},

	{0,5,3,3,0,0,&nprD0,NULL,"ret nc"},
	{0,4,3,3,0,0,&nprD1,NULL,"pop de"},
	{0,4,3,3,0,0,&nprD2,NULL,"jp nc,:2"},
	{0,4,3,4,0,0,&nprD3,NULL,"out (:1),a"},
	{0,4,3,4,3,3,&nprD4,NULL,"call nc,:2"},
	{0,5,3,3,0,0,&nprD5,NULL,"push de"},
	{0,4,3,0,0,0,&nprD6,NULL,"sub :1"},
	{0,5,3,3,0,0,&nprD7,NULL,"rst #10"},

	{0,5,3,3,0,0,&nprD8,NULL,"ret c"},
	{0,4,0,0,0,0,&nprD9,NULL,"exx"},
	{0,4,3,3,0,0,&nprDA,NULL,"jp c,:2"},
	{0,4,3,4,0,0,&nprDB,NULL,"in a,(:1)"},
	{0,4,3,4,3,3,&nprDC,NULL,"call c,:2"},
	{1,4,0,0,0,0,&nprDD,ddTab,"#DD"},
	{0,4,3,0,0,0,&nprDE,NULL,"sbc a,:1"},
	{0,5,3,3,0,0,&nprDF,NULL,"rst #18"},

	{0,5,3,3,0,0,&nprE0,NULL,"ret po"},
	{0,4,3,3,0,0,&fdE1,NULL,"pop iy"},
	{0,4,3,3,0,0,&nprE2,NULL,"jp po,:2"},
	{0,4,3,4,3,5,&fdE3,NULL,"ex (sp),iy"},		// 4 3rd 4rd 3wr 5wr
	{0,4,6,7,0,0,&nprE4,NULL,"call po,:2"},
	{0,5,3,3,0,0,&fdE5,NULL,"push iy"},
	{0,4,3,0,0,0,&nprE6,NULL,"and :1"},
	{0,5,3,3,0,0,&nprE7,NULL,"rst #20"},

	{0,5,3,3,0,0,&nprE8,NULL,"ret pe"},
	{0,4,0,0,0,0,&fdE9,NULL,"jp (iy)"},
	{0,4,3,3,0,0,&nprEA,NULL,"jp pe,:2"},
	{0,4,0,0,0,0,&nprEB,NULL,"ex de,hl"},
	{0,4,3,4,3,3,&nprEC,NULL,"call pe,:2"},
	{1,4,0,0,0,0,&nprED,edTab,"#ED"},
	{0,4,3,0,0,0,&nprEE,NULL,"xor :1"},
	{0,5,3,3,0,0,&nprEF,NULL,"rst #28"},

	{0,5,3,3,0,0,&nprF0,NULL,"ret p"},
	{0,4,3,3,0,0,&nprF1,NULL,"pop af"},
	{0,4,3,3,0,0,&nprF2,NULL,"jp p,:2"},
	{0,4,0,0,0,0,&nprF3,NULL,"di"},
	{0,4,3,4,3,3,&nprF4,NULL,"call p,:2"},
	{0,5,3,3,0,0,&nprF5,NULL,"push af"},
	{0,4,3,0,0,0,&nprF6,NULL,"or :1"},
	{0,5,3,3,0,0,&nprF7,NULL,"rst #30"},

	{0,5,3,3,0,0,&nprF8,NULL,"ret m"},
	{0,6,0,0,0,0,&fdF9,NULL,"ld sp,iy"},
	{0,4,3,3,0,0,&nprFA,NULL,"jp m,:2"},
	{0,4,0,0,0,0,&nprFB,NULL,"ei"},
	{0,4,3,4,3,3,&nprFC,NULL,"call m,:2"},
	{1,4,0,0,0,0,&nprFD,fdTab,"#FD"},
	{0,4,3,3,0,0,&nprFE,NULL,"cp :1"},
	{0,5,3,3,0,0,&nprFF,NULL,"rst #38"}
};
