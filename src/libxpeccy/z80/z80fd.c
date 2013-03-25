
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

// 36	ld (iy+e),n	4 3rd 5add 3rd 3wr	mptr = iy+e
void fd36(Z80CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->pc++,3);
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
}

// e1	pop iy		4 3rd 3rd
void fdE1(Z80CPU* cpu) {
	POP(cpu->hy,cpu->ly);
}

// e3	ex (sp),iy	4 3rd 4rd 3wr 5wr	mptr = iy
void fdE3(Z80CPU* cpu) {
	POP(cpu->htw,cpu->ltw); cpu->t++;	// 3,3+1
	PUSH(cpu->hy, cpu->ly); cpu->t += 2;	// 3,3+2
	cpu->mptr = cpu->iy;
	cpu->iy = cpu->tmpw;
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
	{0,4,0,0,0,0,&npr00,"nop"},
	{0,4,3,3,0,0,&npr01,"ld bc,:2"},
	{0,4,3,0,0,0,&npr02,"ld (bc),a"},
	{0,6,0,0,0,0,&npr03,"inc bc"},
	{0,4,0,0,0,0,&npr04,"inc b"},
	{0,4,0,0,0,0,&npr05,"dec b"},
	{0,4,3,0,0,0,&npr06,"ld b,:1"},
	{0,4,0,0,0,0,&npr07,"rlca"},

	{0,4,0,0,0,0,&npr08,"ex af,af'"},
	{0,11,0,0,0,0,&fd09,"add iy,bc"},
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
	{0,11,0,0,0,0,&fd19,"add iy,de"},
	{0,4,3,0,0,0,&npr1A,"ld a,(de)"},
	{0,6,0,0,0,0,&npr1B,"dec de"},
	{0,4,0,0,0,0,&npr1C,"inc e"},
	{0,4,0,0,0,0,&npr1D,"dec e"},
	{0,4,3,0,0,0,&npr1E,"ld e,:1"},
	{0,4,0,0,0,0,&npr1F,"rra"},

	{0,4,3,5,0,0,&npr20,"jr nz,:3"},
	{0,4,3,3,0,0,&fd21,"ld iy,:2"},
	{0,4,3,3,3,3,&fd22,"ld (:2),iy"},		// 4,3rd,3rd,3wr,3wr
	{0,6,0,0,0,0,&fd23,"inc iy"},
	{0,4,0,0,0,0,&fd24,"inc hy"},
	{0,4,0,0,0,0,&fd25,"dec hy"},
	{0,4,3,0,0,0,&fd26,"ld hy,:1"},
	{0,4,0,0,0,0,&npr27,"daa"},

	{0,4,3,5,0,0,&npr28,"jr z,:3"},
	{0,11,0,0,0,0,&fd29,"add iy,iy"},
	{0,4,3,3,3,3,&fd2A,"ld iy,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,0,0,0,0,&fd2B,"dec iy"},
	{0,4,0,0,0,0,&fd2C,"inc ly"},
	{0,4,0,0,0,0,&fd2D,"dec ly"},
	{0,4,3,0,0,0,&fd2E,"ld ly,:1"},
	{0,4,0,0,0,0,&npr2F,"cpl"},

	{0,4,3,5,0,0,&npr30,"jr nc,:3"},
	{0,4,3,3,0,0,&npr31,"ld sp,:2"},
	{0,4,3,3,3,0,&npr32,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,0,0,0,0,&npr33,"inc sp"},
	{0,4,3,4,0,0,&fd34,"inc (iy:5)"},
	{0,4,3,4,0,0,&fd35,"dec (iy:5)"},
	{0,4,3,3,0,0,&fd36,"ld (iy:5),:1"},
	{0,4,0,0,0,0,&npr37,"scf"},

	{0,4,3,5,0,0,&npr38,"jr c,:3"},
	{0,11,0,0,0,0,&fd39,"add iy,sp"},
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
	{0,4,0,0,0,0,&fd44,"ld b,hy"},
	{0,4,0,0,0,0,&fd45,"ld b,ly"},
	{0,4,3,0,0,0,&fd46,"ld b,(iy:5)"},
	{0,4,0,0,0,0,&npr47,"ld b,a"},

	{0,4,0,0,0,0,&npr48,"ld c,b"},
	{0,4,0,0,0,0,&npr49,"ld c,c"},
	{0,4,0,0,0,0,&npr4A,"ld c,d"},
	{0,4,0,0,0,0,&npr4B,"ld c,e"},
	{0,4,0,0,0,0,&fd4C,"ld c,hy"},
	{0,4,0,0,0,0,&fd4D,"ld c,ly"},
	{0,4,3,0,0,0,&fd4E,"ld c,(iy:5)"},
	{0,4,0,0,0,0,&npr4F,"ld c,a"},

	{0,4,0,0,0,0,&npr50,"ld d,b"},
	{0,4,0,0,0,0,&npr51,"ld d,c"},
	{0,4,0,0,0,0,&npr52,"ld d,d"},
	{0,4,0,0,0,0,&npr53,"ld d,e"},
	{0,4,0,0,0,0,&fd54,"ld d,hy"},
	{0,4,0,0,0,0,&fd55,"ld d,ly"},
	{0,4,3,0,0,0,&fd56,"ld d,(iy:5)"},
	{0,4,0,0,0,0,&npr57,"ld d,a"},

	{0,4,0,0,0,0,&npr58,"ld e,b"},
	{0,4,0,0,0,0,&npr59,"ld e,c"},
	{0,4,0,0,0,0,&npr5A,"ld e,d"},
	{0,4,0,0,0,0,&npr5B,"ld e,e"},
	{0,4,0,0,0,0,&fd5C,"ld e,hy"},
	{0,4,0,0,0,0,&fd5D,"ld e,ly"},
	{0,4,3,0,0,0,&fd5E,"ld e,(iy:5)"},
	{0,4,0,0,0,0,&npr5F,"ld e,a"},

	{0,4,0,0,0,0,&fd60,"ld hy,b"},
	{0,4,0,0,0,0,&fd61,"ld hy,c"},
	{0,4,0,0,0,0,&fd62,"ld hy,d"},
	{0,4,0,0,0,0,&fd63,"ld hy,e"},
	{0,4,0,0,0,0,&fd64,"ld hy,hy"},
	{0,4,0,0,0,0,&fd65,"ld hy,ly"},
	{0,4,3,0,0,0,&fd66,"ld h,(iy:5)"},
	{0,4,0,0,0,0,&fd67,"ld hy,a"},

	{0,4,0,0,0,0,&fd68,"ld ly,b"},
	{0,4,0,0,0,0,&fd69,"ld ly,c"},
	{0,4,0,0,0,0,&fd6A,"ld ly,d"},
	{0,4,0,0,0,0,&fd6B,"ld ly,e"},
	{0,4,0,0,0,0,&fd6C,"ld ly,hy"},
	{0,4,0,0,0,0,&fd6D,"ld ly,ly"},
	{0,4,3,0,0,0,&fd6E,"ld l,(iy:5)"},
	{0,4,0,0,0,0,&fd6F,"ld ly,a"},

	{0,4,3,0,0,0,&fd70,"ld (iy:5),b"},
	{0,4,3,0,0,0,&fd71,"ld (iy:5),c"},
	{0,4,3,0,0,0,&fd72,"ld (iy:5),d"},
	{0,4,3,0,0,0,&fd73,"ld (iy:5),e"},
	{0,4,3,0,0,0,&fd74,"ld (iy:5),h"},
	{0,4,3,0,0,0,&fd75,"ld (iy:5),l"},
	{0,4,0,0,0,0,&npr76,"halt"},
	{0,4,3,0,0,0,&fd77,"ld (iy:5),a"},

	{0,4,0,0,0,0,&npr78,"ld a,b"},
	{0,4,0,0,0,0,&npr79,"ld a,c"},
	{0,4,0,0,0,0,&npr7A,"ld a,d"},
	{0,4,0,0,0,0,&npr7B,"ld a,e"},
	{0,4,0,0,0,0,&fd7C,"ld a,hy"},
	{0,4,0,0,0,0,&fd7D,"ld a,ly"},
	{0,4,3,0,0,0,&fd7E,"ld a,(iy:5)"},
	{0,4,0,0,0,0,&npr7F,"ld a,a"},

	{0,4,0,0,0,0,&npr80,"add a,b"},
	{0,4,0,0,0,0,&npr81,"add a,c"},
	{0,4,0,0,0,0,&npr82,"add a,d"},
	{0,4,0,0,0,0,&npr83,"add a,e"},
	{0,4,0,0,0,0,&fd84,"add a,hy"},
	{0,4,0,0,0,0,&fd85,"add a,ly"},
	{0,4,3,0,0,0,&fd86,"add a,(iy:5)"},
	{0,4,0,0,0,0,&npr87,"add a,a"},

	{0,4,0,0,0,0,&npr88,"adc a,b"},
	{0,4,0,0,0,0,&npr89,"adc a,c"},
	{0,4,0,0,0,0,&npr8A,"adc a,d"},
	{0,4,0,0,0,0,&npr8B,"adc a,e"},
	{0,4,0,0,0,0,&fd8C,"adc a,hy"},
	{0,4,0,0,0,0,&fd8D,"adc a,ly"},
	{0,4,3,0,0,0,&fd8E,"adc a,(iy:5)"},
	{0,4,0,0,0,0,&npr8F,"adc a,a"},

	{0,4,0,0,0,0,&npr90,"sub b"},
	{0,4,0,0,0,0,&npr91,"sub c"},
	{0,4,0,0,0,0,&npr92,"sub d"},
	{0,4,0,0,0,0,&npr93,"sub e"},
	{0,4,0,0,0,0,&fd94,"sub hy"},
	{0,4,0,0,0,0,&fd95,"sub ly"},
	{0,4,3,0,0,0,&fd96,"sub (iy:5)"},
	{0,4,0,0,0,0,&npr97,"sub a"},

	{0,4,0,0,0,0,&npr98,"sbc a,b"},
	{0,4,0,0,0,0,&npr99,"sbc a,c"},
	{0,4,0,0,0,0,&npr9A,"sbc a,d"},
	{0,4,0,0,0,0,&npr9B,"sbc a,e"},
	{0,4,0,0,0,0,&fd9C,"sbc a,hy"},
	{0,4,0,0,0,0,&fd9D,"sbc a,ly"},
	{0,4,3,0,0,0,&fd9E,"sbc a,(iy:5)"},
	{0,4,0,0,0,0,&npr9F,"sbc a,a"},

	{0,4,0,0,0,0,&nprA0,"and b"},
	{0,4,0,0,0,0,&nprA1,"and c"},
	{0,4,0,0,0,0,&nprA2,"and d"},
	{0,4,0,0,0,0,&nprA3,"and e"},
	{0,4,0,0,0,0,&fdA4,"and hy"},
	{0,4,0,0,0,0,&fdA5,"and ly"},
	{0,4,3,0,0,0,&fdA6,"and (iy:5)"},
	{0,4,0,0,0,0,&nprA7,"and a"},

	{0,4,0,0,0,0,&nprA8,"xor b"},
	{0,4,0,0,0,0,&nprA9,"xor c"},
	{0,4,0,0,0,0,&nprAA,"xor d"},
	{0,4,0,0,0,0,&nprAB,"xor e"},
	{0,4,0,0,0,0,&fdAC,"xor hy"},
	{0,4,0,0,0,0,&fdAD,"xor ly"},
	{0,4,3,0,0,0,&fdAE,"xor (iy:5)"},
	{0,4,0,0,0,0,&nprAF,"xor a"},

	{0,4,0,0,0,0,&nprB0,"or b"},
	{0,4,0,0,0,0,&nprB1,"or c"},
	{0,4,0,0,0,0,&nprB2,"or d"},
	{0,4,0,0,0,0,&nprB3,"or e"},
	{0,4,0,0,0,0,&fdB4,"or hy"},
	{0,4,0,0,0,0,&fdB5,"or ly"},
	{0,4,3,0,0,0,&fdB6,"or (iy:5)"},
	{0,4,0,0,0,0,&nprB7,"or a"},

	{0,4,0,0,0,0,&nprB8,"cp b"},
	{0,4,0,0,0,0,&nprB9,"cp c"},
	{0,4,0,0,0,0,&nprBA,"cp d"},
	{0,4,0,0,0,0,&nprBB,"cp e"},
	{0,4,0,0,0,0,&fdBC,"cp hy"},
	{0,4,0,0,0,0,&fdBD,"cp ly"},
	{0,4,3,0,0,0,&fdBE,"cp (iy:5)"},
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
	{1,4,0,0,0,0,&fdCB,"#CB"},
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
	{0,4,3,3,0,0,&fdE1,"pop iy"},
	{0,4,3,3,0,0,&nprE2,"jp po,:2"},
	{0,4,3,4,3,5,&fdE3,"ex (sp),iy"},		// 4 3rd 4rd 3wr 5wr
	{0,4,6,7,0,0,&nprE4,"call po,:2"},
	{0,5,3,3,0,0,&fdE5,"push iy"},
	{0,4,3,0,0,0,&nprE6,"and :1"},
	{0,5,3,3,0,0,&nprE7,"rst #20"},

	{0,5,3,3,0,0,&nprE8,"ret pe"},
	{0,4,0,0,0,0,&fdE9,"jp (iy)"},
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
	{0,6,0,0,0,0,&fdF9,"ld sp,iy"},
	{0,4,3,3,0,0,&nprFA,"jp m,:2"},
	{0,4,0,0,0,0,&nprFB,"ei"},
	{0,4,3,4,3,3,&nprFC,"call m,:2"},
	{1,4,0,0,0,0,&nprFD,"#FD"},
	{0,4,3,3,0,0,&nprFE,"cp :1"},
	{0,5,3,3,0,0,&nprFF,"rst #38"}
};
