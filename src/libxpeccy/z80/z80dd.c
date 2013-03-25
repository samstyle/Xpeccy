
// 09	add ix,bc	11	mptr = ix+1 before adding
void dd09(Z80CPU* cpu) {
	ADD16(cpu->ix, cpu->bc);
}

// 19	add ix,de	11	mptr = ix+1 before adding
void dd19(Z80CPU* cpu) {
	ADD16(cpu->ix,cpu->de);
}

// 21	ld ix,nn	4 3rd 3rd
void dd21(Z80CPU* cpu) {
	cpu->lx = MEMRD(cpu->pc++,3);
	cpu->hx = MEMRD(cpu->pc++,3);
}

// 22	ld (nn),ix	4 3rd 3rd 3wr 3wr	mptr = nn+1
void dd22(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->lx,3);
	MEMWR(cpu->mptr,cpu->hx,3);
}

// 23	inc ix		6
void dd23(Z80CPU* cpu) {
	cpu->ix++;
}

// 24	inc hx		4
void dd24(Z80CPU* cpu) {
	INC(cpu->hx);
}

// 25	dec hx		4
void dd25(Z80CPU* cpu) {
	DEC(cpu->hx);
}

// 26	ld hx,n		4 3rd
void dd26(Z80CPU* cpu) {
	cpu->hx = MEMRD(cpu->pc++,3);
}

// 29	add ix,ix	11
void dd29(Z80CPU* cpu) {
	ADD16(cpu->ix,cpu->ix);
}

// 2A	ld ix,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void dd2A(Z80CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->lx = MEMRD(cpu->mptr++,3);
	cpu->hx = MEMRD(cpu->mptr,3);
}

// 2B	dec ix		6
void dd2B(Z80CPU* cpu) {
	cpu->ix--;
}

// 2C	inc lx		4
void dd2C(Z80CPU* cpu) {
	INC(cpu->lx);
}

// 2D	dec lx		4
void dd2D(Z80CPU* cpu) {
	DEC(cpu->lx);
}

// 2E	ld lx,n		4 3rd
void dd2E(Z80CPU* cpu) {
	cpu->lx = MEMRD(cpu->pc++,3);
}

// 34	inc (ix+e)	4 3rd 5add 4rd 3wr
void dd34(Z80CPU* cpu) {
	RDSHIFT(cpu->ix);
	cpu->tmp = MEMRD(cpu->mptr,4);
	INC(cpu->tmp);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 35	dec (ix+e)	4 3rd 5add 4rd 3wr	mptr = ix+e
void dd35(Z80CPU* cpu) {
	RDSHIFT(cpu->ix);
	cpu->tmp = MEMRD(cpu->mptr,4);
	DEC(cpu->tmp);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 36	ld (ix+e),n	4 3rd 5add 3rd 3wr	mptr = ix+e
void dd36(Z80CPU* cpu) {
	RDSHIFT(cpu->ix);
	cpu->tmp = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 39	add ix,sp	11
void dd39(Z80CPU* cpu) {
	ADD16(cpu->ix,cpu->sp);
}

// ld r,r		4 [3rd 5add 3rd]
void dd44(Z80CPU* cpu) {cpu->b = cpu->hx;}
void dd45(Z80CPU* cpu) {cpu->b = cpu->lx;}
void dd46(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->b = MEMRD(cpu->mptr,3);}
void dd4C(Z80CPU* cpu) {cpu->c = cpu->hx;}
void dd4D(Z80CPU* cpu) {cpu->c = cpu->lx;}
void dd4E(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->c = MEMRD(cpu->mptr,3);}
void dd54(Z80CPU* cpu) {cpu->d = cpu->hx;}
void dd55(Z80CPU* cpu) {cpu->d = cpu->lx;}
void dd56(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->d = MEMRD(cpu->mptr,3);}
void dd5C(Z80CPU* cpu) {cpu->e = cpu->hx;}
void dd5D(Z80CPU* cpu) {cpu->e = cpu->lx;}
void dd5E(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->e = MEMRD(cpu->mptr,3);}

void dd60(Z80CPU* cpu) {cpu->hx = cpu->b;}
void dd61(Z80CPU* cpu) {cpu->hx = cpu->c;}
void dd62(Z80CPU* cpu) {cpu->hx = cpu->d;}
void dd63(Z80CPU* cpu) {cpu->hx = cpu->e;}
void dd64(Z80CPU* cpu) {}
void dd65(Z80CPU* cpu) {cpu->hx = cpu->lx;}
void dd66(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->h = MEMRD(cpu->mptr,3);}
void dd67(Z80CPU* cpu) {cpu->hx = cpu->a;}

void dd68(Z80CPU* cpu) {cpu->lx = cpu->b;}
void dd69(Z80CPU* cpu) {cpu->lx = cpu->c;}
void dd6A(Z80CPU* cpu) {cpu->lx = cpu->d;}
void dd6B(Z80CPU* cpu) {cpu->lx = cpu->e;}
void dd6C(Z80CPU* cpu) {cpu->lx = cpu->hx;}
void dd6D(Z80CPU* cpu) {}
void dd6E(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->l = MEMRD(cpu->mptr,3);}
void dd6F(Z80CPU* cpu) {cpu->lx = cpu->a;}
// 70..77	ld (ix+e),r	4 3rd 5add 3wr
void dd70(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->b,3);}
void dd71(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->c,3);}
void dd72(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->d,3);}
void dd73(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->e,3);}
void dd74(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->h,3);}
void dd75(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->l,3);}
void dd77(Z80CPU* cpu) {RDSHIFT(cpu->ix); MEMWR(cpu->mptr,cpu->a,3);}

void dd7C(Z80CPU* cpu) {cpu->a = cpu->hx;}
void dd7D(Z80CPU* cpu) {cpu->a = cpu->lx;}
void dd7E(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->a = MEMRD(cpu->mptr,3);}

// add x
void dd84(Z80CPU* cpu) {ADD(cpu->hx);}
void dd85(Z80CPU* cpu) {ADD(cpu->lx);}
void dd86(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); ADD(cpu->tmpb);}
// adc x
void dd8C(Z80CPU* cpu) {ADC(cpu->hx);}
void dd8D(Z80CPU* cpu) {ADC(cpu->lx);}
void dd8E(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); ADC(cpu->tmpb);}
// sub x
void dd94(Z80CPU* cpu) {SUB(cpu->hx);}
void dd95(Z80CPU* cpu) {SUB(cpu->lx);}
void dd96(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); SUB(cpu->tmpb);}
// sbc x
void dd9C(Z80CPU* cpu) {SBC(cpu->hx);}
void dd9D(Z80CPU* cpu) {SBC(cpu->lx);}
void dd9E(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); SBC(cpu->tmpb);}
// and x
void ddA4(Z80CPU* cpu) {cpu->a &= cpu->hx; cpu->f = sz53pTab[cpu->a] | FH;}
void ddA5(Z80CPU* cpu) {cpu->a &= cpu->lx; cpu->f = sz53pTab[cpu->a] | FH;}
void ddA6(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a &= cpu->tmpb; cpu->f = sz53pTab[cpu->a] | FH;}
// xor x
void ddAC(Z80CPU* cpu) {cpu->a ^= cpu->hx; cpu->f = sz53pTab[cpu->a];}
void ddAD(Z80CPU* cpu) {cpu->a ^= cpu->lx; cpu->f = sz53pTab[cpu->a];}
void ddAE(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a ^= cpu->tmpb; cpu->f = sz53pTab[cpu->a];}
// or x
void ddB4(Z80CPU* cpu) {cpu->a |= cpu->hx; cpu->f = sz53pTab[cpu->a];}
void ddB5(Z80CPU* cpu) {cpu->a |= cpu->lx; cpu->f = sz53pTab[cpu->a];}
void ddB6(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a |= cpu->tmpb; cpu->f = sz53pTab[cpu->a];}
// cp x
void ddBC(Z80CPU* cpu) {CP(cpu->hx);}
void ddBD(Z80CPU* cpu) {CP(cpu->lx);}
void ddBE(Z80CPU* cpu) {RDSHIFT(cpu->ix); cpu->tmpb = MEMRD(cpu->mptr,3); CP(cpu->tmpb);}

// cb	ddcb prefix	4 3rd
void ddCB(Z80CPU* cpu) {
	cpu->opTab = ddcbTab;
	cpu->tmp = MEMRD(cpu->pc++,3);
}

// e1	pop ix		4 3rd 3rd
void ddE1(Z80CPU* cpu) {
	POP(cpu->hx,cpu->lx);
}

// e3	ex (sp),ix	4 3rd 4rd 3wr 5wr	mptr = ix
void ddE3(Z80CPU* cpu) {
	POP(cpu->htw,cpu->ltw); cpu->t++;	// 3,3+1
	PUSH(cpu->hx, cpu->lx); cpu->t += 2;	// 3,3+2
	cpu->mptr = cpu->ix;
	cpu->ix = cpu->tmpw;
}

// e5	push ix		5 3wr 3wr
void ddE5(Z80CPU* cpu) {
	PUSH(cpu->hx,cpu->lx);
}

// e9	jp (ix)		4
void ddE9(Z80CPU* cpu) {
	cpu->pc = cpu->ix;
}

// f9	ld sp,ix	6
void ddF9(Z80CPU* cpu) {
	cpu->sp = cpu->ix;
}

// ======

opCode ddTab[256]={
	{0,4,0,0,0,0,&npr00,"nop"},
	{0,4,3,3,0,0,&npr01,"ld bc,:2"},
	{0,4,3,0,0,0,&npr02,"ld (bc),a"},
	{0,6,0,0,0,0,&npr03,"inc bc"},
	{0,4,0,0,0,0,&npr04,"inc b"},
	{0,4,0,0,0,0,&npr05,"dec b"},
	{0,4,3,0,0,0,&npr06,"ld b,:1"},
	{0,4,0,0,0,0,&npr07,"rlca"},

	{0,4,0,0,0,0,&npr08,"ex af,af'"},
	{0,11,0,0,0,0,&dd09,"add ix,bc"},
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
	{0,11,0,0,0,0,&dd19,"add ix,de"},
	{0,4,3,0,0,0,&npr1A,"ld a,(de)"},
	{0,6,0,0,0,0,&npr1B,"dec de"},
	{0,4,0,0,0,0,&npr1C,"inc e"},
	{0,4,0,0,0,0,&npr1D,"dec e"},
	{0,4,3,0,0,0,&npr1E,"ld e,:1"},
	{0,4,0,0,0,0,&npr1F,"rra"},

	{0,4,3,5,0,0,&npr20,"jr nz,:3"},
	{0,4,3,3,0,0,&dd21,"ld ix,:2"},
	{0,4,3,3,3,3,&dd22,"ld (:2),ix"},		// 4,3rd,3rd,3wr,3wr
	{0,6,0,0,0,0,&dd23,"inc ix"},
	{0,4,0,0,0,0,&dd24,"inc hx"},
	{0,4,0,0,0,0,&dd25,"dec hx"},
	{0,4,3,0,0,0,&dd26,"ld hx,:1"},
	{0,4,0,0,0,0,&npr27,"daa"},

	{0,4,3,5,0,0,&npr28,"jr z,:3"},
	{0,11,0,0,0,0,&dd29,"add ix,ix"},
	{0,4,3,3,3,3,&dd2A,"ld ix,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,0,0,0,0,&dd2B,"dec ix"},
	{0,4,0,0,0,0,&dd2C,"inc lx"},
	{0,4,0,0,0,0,&dd2D,"dec lx"},
	{0,4,3,0,0,0,&dd2E,"ld lx,:1"},
	{0,4,0,0,0,0,&npr2F,"cpl"},

	{0,4,3,5,0,0,&npr30,"jr nc,:3"},
	{0,4,3,3,0,0,&npr31,"ld sp,:2"},
	{0,4,3,3,3,0,&npr32,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,0,0,0,0,&npr33,"inc sp"},
	{0,4,3,4,0,0,&dd34,"inc (ix:5)"},
	{0,4,3,4,0,0,&dd35,"dec (ix:5)"},
	{0,4,3,3,0,0,&dd36,"ld (ix:5),:1"},
	{0,4,0,0,0,0,&npr37,"scf"},

	{0,4,3,5,0,0,&npr38,"jr c,:3"},
	{0,11,0,0,0,0,&dd39,"add ix,sp"},
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
	{0,4,0,0,0,0,&dd44,"ld b,hx"},
	{0,4,0,0,0,0,&dd45,"ld b,lx"},
	{0,4,3,0,0,0,&dd46,"ld b,(ix:5)"},
	{0,4,0,0,0,0,&npr47,"ld b,a"},

	{0,4,0,0,0,0,&npr48,"ld c,b"},
	{0,4,0,0,0,0,&npr49,"ld c,c"},
	{0,4,0,0,0,0,&npr4A,"ld c,d"},
	{0,4,0,0,0,0,&npr4B,"ld c,e"},
	{0,4,0,0,0,0,&dd4C,"ld c,hx"},
	{0,4,0,0,0,0,&dd4D,"ld c,lx"},
	{0,4,3,0,0,0,&dd4E,"ld c,(ix:5)"},
	{0,4,0,0,0,0,&npr4F,"ld c,a"},

	{0,4,0,0,0,0,&npr50,"ld d,b"},
	{0,4,0,0,0,0,&npr51,"ld d,c"},
	{0,4,0,0,0,0,&npr52,"ld d,d"},
	{0,4,0,0,0,0,&npr53,"ld d,e"},
	{0,4,0,0,0,0,&dd54,"ld d,hx"},
	{0,4,0,0,0,0,&dd55,"ld d,lx"},
	{0,4,3,0,0,0,&dd56,"ld d,(ix:5)"},
	{0,4,0,0,0,0,&npr57,"ld d,a"},

	{0,4,0,0,0,0,&npr58,"ld e,b"},
	{0,4,0,0,0,0,&npr59,"ld e,c"},
	{0,4,0,0,0,0,&npr5A,"ld e,d"},
	{0,4,0,0,0,0,&npr5B,"ld e,e"},
	{0,4,0,0,0,0,&dd5C,"ld e,hx"},
	{0,4,0,0,0,0,&dd5D,"ld e,lx"},
	{0,4,3,0,0,0,&dd5E,"ld e,(ix:5)"},
	{0,4,0,0,0,0,&npr5F,"ld e,a"},

	{0,4,0,0,0,0,&dd60,"ld hx,b"},
	{0,4,0,0,0,0,&dd61,"ld hx,c"},
	{0,4,0,0,0,0,&dd62,"ld hx,d"},
	{0,4,0,0,0,0,&dd63,"ld hx,e"},
	{0,4,0,0,0,0,&dd64,"ld hx,hx"},
	{0,4,0,0,0,0,&dd65,"ld hx,lx"},
	{0,4,3,0,0,0,&dd66,"ld h,(ix:5)"},
	{0,4,0,0,0,0,&dd67,"ld hx,a"},

	{0,4,0,0,0,0,&dd68,"ld lx,b"},
	{0,4,0,0,0,0,&dd69,"ld lx,c"},
	{0,4,0,0,0,0,&dd6A,"ld lx,d"},
	{0,4,0,0,0,0,&dd6B,"ld lx,e"},
	{0,4,0,0,0,0,&dd6C,"ld lx,hx"},
	{0,4,0,0,0,0,&dd6D,"ld lx,lx"},
	{0,4,3,0,0,0,&dd6E,"ld l,(ix:5)"},
	{0,4,0,0,0,0,&dd6F,"ld lx,a"},

	{0,4,3,0,0,0,&dd70,"ld (ix:5),b"},
	{0,4,3,0,0,0,&dd71,"ld (ix:5),c"},
	{0,4,3,0,0,0,&dd72,"ld (ix:5),d"},
	{0,4,3,0,0,0,&dd73,"ld (ix:5),e"},
	{0,4,3,0,0,0,&dd74,"ld (ix:5),h"},
	{0,4,3,0,0,0,&dd75,"ld (ix:5),l"},
	{0,4,0,0,0,0,&npr76,"halt"},
	{0,4,3,0,0,0,&dd77,"ld (ix:5),a"},

	{0,4,0,0,0,0,&npr78,"ld a,b"},
	{0,4,0,0,0,0,&npr79,"ld a,c"},
	{0,4,0,0,0,0,&npr7A,"ld a,d"},
	{0,4,0,0,0,0,&npr7B,"ld a,e"},
	{0,4,0,0,0,0,&dd7C,"ld a,hx"},
	{0,4,0,0,0,0,&dd7D,"ld a,lx"},
	{0,4,3,0,0,0,&dd7E,"ld a,(ix:5)"},
	{0,4,0,0,0,0,&npr7F,"ld a,a"},

	{0,4,0,0,0,0,&npr80,"add a,b"},
	{0,4,0,0,0,0,&npr81,"add a,c"},
	{0,4,0,0,0,0,&npr82,"add a,d"},
	{0,4,0,0,0,0,&npr83,"add a,e"},
	{0,4,0,0,0,0,&dd84,"add a,hx"},
	{0,4,0,0,0,0,&dd85,"add a,lx"},
	{0,4,3,0,0,0,&dd86,"add a,(ix:5)"},
	{0,4,0,0,0,0,&npr87,"add a,a"},

	{0,4,0,0,0,0,&npr88,"adc a,b"},
	{0,4,0,0,0,0,&npr89,"adc a,c"},
	{0,4,0,0,0,0,&npr8A,"adc a,d"},
	{0,4,0,0,0,0,&npr8B,"adc a,e"},
	{0,4,0,0,0,0,&dd8C,"adc a,hx"},
	{0,4,0,0,0,0,&dd8D,"adc a,lx"},
	{0,4,3,0,0,0,&dd8E,"adc a,(ix:5)"},
	{0,4,0,0,0,0,&npr8F,"adc a,a"},

	{0,4,0,0,0,0,&npr90,"sub b"},
	{0,4,0,0,0,0,&npr91,"sub c"},
	{0,4,0,0,0,0,&npr92,"sub d"},
	{0,4,0,0,0,0,&npr93,"sub e"},
	{0,4,0,0,0,0,&dd94,"sub hx"},
	{0,4,0,0,0,0,&dd95,"sub lx"},
	{0,4,3,0,0,0,&dd96,"sub (ix:5)"},
	{0,4,0,0,0,0,&npr97,"sub a"},

	{0,4,0,0,0,0,&npr98,"sbc a,b"},
	{0,4,0,0,0,0,&npr99,"sbc a,c"},
	{0,4,0,0,0,0,&npr9A,"sbc a,d"},
	{0,4,0,0,0,0,&npr9B,"sbc a,e"},
	{0,4,0,0,0,0,&dd9C,"sbc a,hx"},
	{0,4,0,0,0,0,&dd9D,"sbc a,lx"},
	{0,4,3,0,0,0,&dd9E,"sbc a,(ix:5)"},
	{0,4,0,0,0,0,&npr9F,"sbc a,a"},

	{0,4,0,0,0,0,&nprA0,"and b"},
	{0,4,0,0,0,0,&nprA1,"and c"},
	{0,4,0,0,0,0,&nprA2,"and d"},
	{0,4,0,0,0,0,&nprA3,"and e"},
	{0,4,0,0,0,0,&ddA4,"and hx"},
	{0,4,0,0,0,0,&ddA5,"and lx"},
	{0,4,3,0,0,0,&ddA6,"and (ix:5)"},
	{0,4,0,0,0,0,&nprA7,"and a"},

	{0,4,0,0,0,0,&nprA8,"xor b"},
	{0,4,0,0,0,0,&nprA9,"xor c"},
	{0,4,0,0,0,0,&nprAA,"xor d"},
	{0,4,0,0,0,0,&nprAB,"xor e"},
	{0,4,0,0,0,0,&ddAC,"xor hx"},
	{0,4,0,0,0,0,&ddAD,"xor lx"},
	{0,4,3,0,0,0,&ddAE,"xor (ix:5)"},
	{0,4,0,0,0,0,&nprAF,"xor a"},

	{0,4,0,0,0,0,&nprB0,"or b"},
	{0,4,0,0,0,0,&nprB1,"or c"},
	{0,4,0,0,0,0,&nprB2,"or d"},
	{0,4,0,0,0,0,&nprB3,"or e"},
	{0,4,0,0,0,0,&ddB4,"or hx"},
	{0,4,0,0,0,0,&ddB5,"or lx"},
	{0,4,3,0,0,0,&ddB6,"or (ix:5)"},
	{0,4,0,0,0,0,&nprB7,"or a"},

	{0,4,0,0,0,0,&nprB8,"cp b"},
	{0,4,0,0,0,0,&nprB9,"cp c"},
	{0,4,0,0,0,0,&nprBA,"cp d"},
	{0,4,0,0,0,0,&nprBB,"cp e"},
	{0,4,0,0,0,0,&ddBC,"cp hx"},
	{0,4,0,0,0,0,&ddBD,"cp lx"},
	{0,4,3,0,0,0,&ddBE,"cp (ix:5)"},
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
	{1,4,0,0,0,0,&ddCB,"#CB"},
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
	{0,4,3,3,0,0,&ddE1,"pop ix"},
	{0,4,3,3,0,0,&nprE2,"jp po,:2"},
	{0,4,3,4,3,5,&ddE3,"ex (sp),ix"},		// 4 3rd 4rd 3wr 5wr
	{0,4,6,7,0,0,&nprE4,"call po,:2"},
	{0,5,3,3,0,0,&ddE5,"push ix"},
	{0,4,3,0,0,0,&nprE6,"and :1"},
	{0,5,3,3,0,0,&nprE7,"rst #20"},

	{0,5,3,3,0,0,&nprE8,"ret pe"},
	{0,4,0,0,0,0,&ddE9,"jp (ix)"},
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
	{0,6,0,0,0,0,&ddF9,"ld sp,ix"},
	{0,4,3,3,0,0,&nprFA,"jp m,:2"},
	{0,4,0,0,0,0,&nprFB,"ei"},
	{0,4,3,4,3,3,&nprFC,"call m,:2"},
	{1,4,0,0,0,0,&nprFD,"#FD"},
	{0,4,3,3,0,0,&nprFE,"cp :1"},
	{0,5,3,3,0,0,&nprFF,"rst #38"}
};
