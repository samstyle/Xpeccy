
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

// 36	ld (ix+e),n	4 3rd {5add 3rd} 3wr	mptr = ix+e
void dd36(Z80CPU* cpu) {
	RDSHIFT(cpu->ix);
	cpu->tmp = MEMRD(cpu->pc++,0);	// 0?
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
	cpu->tmpb = MEMRD(cpu->pc++,0);	// opcode. eat 0T? not m1
	cpu->op = &ddcbTab[cpu->tmpb];
	cpu->op->exec(cpu);
}

// e1	pop ix		4 3rd 3rd
void ddE1(Z80CPU* cpu) {
	POP(cpu->hx,cpu->lx);
}

// e3	ex (sp),ix	4 3rd 4rd 3wr 5wr	mptr = ix
void ddE3(Z80CPU* cpu) {
	POP(cpu->htw,cpu->ltw); cpu->t++;	// 3,3+1
	PUSH(cpu->hx, cpu->lx); cpu->t += 2;	// 3,3+2
	cpu->ix = cpu->tmpw;
	cpu->mptr = cpu->ix;
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
	{0,4,0,0,0,0,0,&npr00,NULL,"nop"},
	{0,4,3,3,0,0,0,&npr01,NULL,"ld bc,:2"},
	{0,4,3,0,0,0,0,&npr02,NULL,"ld (bc),a"},
	{0,6,0,0,0,0,0,&npr03,NULL,"inc bc"},
	{0,4,0,0,0,0,0,&npr04,NULL,"inc b"},
	{0,4,0,0,0,0,0,&npr05,NULL,"dec b"},
	{0,4,3,0,0,0,0,&npr06,NULL,"ld b,:1"},
	{0,4,0,0,0,0,0,&npr07,NULL,"rlca"},

	{0,4,0,0,0,0,0,&npr08,NULL,"ex af,af'"},
	{0,11,0,0,0,0,0,&dd09,NULL,"add ix,bc"},
	{0,4,3,0,0,0,0,&npr0A,NULL,"ld a,(bc)"},
	{0,6,0,0,0,0,0,&npr0B,NULL,"dec bc"},
	{0,4,0,0,0,0,0,&npr0C,NULL,"inc c"},
	{0,4,0,0,0,0,0,&npr0D,NULL,"dec c"},
	{0,4,3,0,0,0,0,&npr0E,NULL,"ld c,:1"},
	{0,4,0,0,0,0,0,&npr0F,NULL,"rrca"},

	{0,5,3,5,0,0,0,&npr10,NULL,"djnz :3"},
	{0,4,3,3,0,0,0,&npr11,NULL,"ld de,:2"},
	{0,4,3,0,0,0,0,&npr12,NULL,"ld (de),a"},
	{0,6,0,0,0,0,0,&npr13,NULL,"inc de"},
	{0,4,0,0,0,0,0,&npr14,NULL,"inc d"},
	{0,4,0,0,0,0,0,&npr15,NULL,"dec d"},
	{0,4,3,0,0,0,0,&npr16,NULL,"ld d,:1"},
	{0,4,0,0,0,0,0,&npr17,NULL,"rla"},

	{0,4,3,5,0,0,0,&npr18,NULL,"jr :3"},
	{0,11,0,0,0,0,0,&dd19,NULL,"add ix,de"},
	{0,4,3,0,0,0,0,&npr1A,NULL,"ld a,(de)"},
	{0,6,0,0,0,0,0,&npr1B,NULL,"dec de"},
	{0,4,0,0,0,0,0,&npr1C,NULL,"inc e"},
	{0,4,0,0,0,0,0,&npr1D,NULL,"dec e"},
	{0,4,3,0,0,0,0,&npr1E,NULL,"ld e,:1"},
	{0,4,0,0,0,0,0,&npr1F,NULL,"rra"},

	{0,4,3,5,0,0,0,&npr20,NULL,"jr nz,:3"},
	{0,4,3,3,0,0,0,&dd21,NULL,"ld ix,:2"},
	{0,4,3,3,3,3,0,&dd22,NULL,"ld (:2),ix"},		// 4,3rd,3rd,3wr,3wr
	{0,6,0,0,0,0,0,&dd23,NULL,"inc ix"},
	{0,4,0,0,0,0,0,&dd24,NULL,"inc hx"},
	{0,4,0,0,0,0,0,&dd25,NULL,"dec hx"},
	{0,4,3,0,0,0,0,&dd26,NULL,"ld hx,:1"},
	{0,4,0,0,0,0,0,&npr27,NULL,"daa"},

	{0,4,3,5,0,0,0,&npr28,NULL,"jr z,:3"},
	{0,11,0,0,0,0,0,&dd29,NULL,"add ix,ix"},
	{0,4,3,3,3,3,0,&dd2A,NULL,"ld ix,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,0,0,0,0,0,&dd2B,NULL,"dec ix"},
	{0,4,0,0,0,0,0,&dd2C,NULL,"inc lx"},
	{0,4,0,0,0,0,0,&dd2D,NULL,"dec lx"},
	{0,4,3,0,0,0,0,&dd2E,NULL,"ld lx,:1"},
	{0,4,0,0,0,0,0,&npr2F,NULL,"cpl"},

	{0,4,3,5,0,0,0,&npr30,NULL,"jr nc,:3"},
	{0,4,3,3,0,0,0,&npr31,NULL,"ld sp,:2"},
	{0,4,3,3,3,0,0,&npr32,NULL,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,0,0,0,0,0,&npr33,NULL,"inc sp"},
	{0,4,3,4,0,0,0,&dd34,NULL,"inc (ix:4)"},
	{0,4,3,4,0,0,0,&dd35,NULL,"dec (ix:4)"},
	{0,4,3,3,0,0,0,&dd36,NULL,"ld (ix:4),:1"},
	{0,4,0,0,0,0,0,&npr37,NULL,"scf"},

	{0,4,3,5,0,0,0,&npr38,NULL,"jr c,:3"},
	{0,11,0,0,0,0,0,&dd39,NULL,"add ix,sp"},
	{0,4,3,3,3,0,0,&npr3A,NULL,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,0,0,0,0,0,&npr3B,NULL,"dec sp"},
	{0,4,0,0,0,0,0,&npr3C,NULL,"inc a"},
	{0,4,0,0,0,0,0,&npr3D,NULL,"dec a"},
	{0,4,3,0,0,0,0,&npr3E,NULL,"ld a,:1"},
	{0,4,0,0,0,0,0,&npr3F,NULL,"ccf"},

	{0,4,0,0,0,0,0,&npr40,NULL,"ld b,b"},
	{0,4,0,0,0,0,0,&npr41,NULL,"ld b,c"},
	{0,4,0,0,0,0,0,&npr42,NULL,"ld b,d"},
	{0,4,0,0,0,0,0,&npr43,NULL,"ld b,e"},
	{0,4,0,0,0,0,0,&dd44,NULL,"ld b,hx"},
	{0,4,0,0,0,0,0,&dd45,NULL,"ld b,lx"},
	{0,4,3,0,0,0,0,&dd46,NULL,"ld b,(ix:4)"},
	{0,4,0,0,0,0,0,&npr47,NULL,"ld b,a"},

	{0,4,0,0,0,0,0,&npr48,NULL,"ld c,b"},
	{0,4,0,0,0,0,0,&npr49,NULL,"ld c,c"},
	{0,4,0,0,0,0,0,&npr4A,NULL,"ld c,d"},
	{0,4,0,0,0,0,0,&npr4B,NULL,"ld c,e"},
	{0,4,0,0,0,0,0,&dd4C,NULL,"ld c,hx"},
	{0,4,0,0,0,0,0,&dd4D,NULL,"ld c,lx"},
	{0,4,3,0,0,0,0,&dd4E,NULL,"ld c,(ix:4)"},
	{0,4,0,0,0,0,0,&npr4F,NULL,"ld c,a"},

	{0,4,0,0,0,0,0,&npr50,NULL,"ld d,b"},
	{0,4,0,0,0,0,0,&npr51,NULL,"ld d,c"},
	{0,4,0,0,0,0,0,&npr52,NULL,"ld d,d"},
	{0,4,0,0,0,0,0,&npr53,NULL,"ld d,e"},
	{0,4,0,0,0,0,0,&dd54,NULL,"ld d,hx"},
	{0,4,0,0,0,0,0,&dd55,NULL,"ld d,lx"},
	{0,4,3,0,0,0,0,&dd56,NULL,"ld d,(ix:4)"},
	{0,4,0,0,0,0,0,&npr57,NULL,"ld d,a"},

	{0,4,0,0,0,0,0,&npr58,NULL,"ld e,b"},
	{0,4,0,0,0,0,0,&npr59,NULL,"ld e,c"},
	{0,4,0,0,0,0,0,&npr5A,NULL,"ld e,d"},
	{0,4,0,0,0,0,0,&npr5B,NULL,"ld e,e"},
	{0,4,0,0,0,0,0,&dd5C,NULL,"ld e,hx"},
	{0,4,0,0,0,0,0,&dd5D,NULL,"ld e,lx"},
	{0,4,3,0,0,0,0,&dd5E,NULL,"ld e,(ix:4)"},
	{0,4,0,0,0,0,0,&npr5F,NULL,"ld e,a"},

	{0,4,0,0,0,0,0,&dd60,NULL,"ld hx,b"},
	{0,4,0,0,0,0,0,&dd61,NULL,"ld hx,c"},
	{0,4,0,0,0,0,0,&dd62,NULL,"ld hx,d"},
	{0,4,0,0,0,0,0,&dd63,NULL,"ld hx,e"},
	{0,4,0,0,0,0,0,&dd64,NULL,"ld hx,hx"},
	{0,4,0,0,0,0,0,&dd65,NULL,"ld hx,lx"},
	{0,4,3,0,0,0,0,&dd66,NULL,"ld h,(ix:4)"},
	{0,4,0,0,0,0,0,&dd67,NULL,"ld hx,a"},

	{0,4,0,0,0,0,0,&dd68,NULL,"ld lx,b"},
	{0,4,0,0,0,0,0,&dd69,NULL,"ld lx,c"},
	{0,4,0,0,0,0,0,&dd6A,NULL,"ld lx,d"},
	{0,4,0,0,0,0,0,&dd6B,NULL,"ld lx,e"},
	{0,4,0,0,0,0,0,&dd6C,NULL,"ld lx,hx"},
	{0,4,0,0,0,0,0,&dd6D,NULL,"ld lx,lx"},
	{0,4,3,0,0,0,0,&dd6E,NULL,"ld l,(ix:4)"},
	{0,4,0,0,0,0,0,&dd6F,NULL,"ld lx,a"},

	{0,4,3,0,0,0,0,&dd70,NULL,"ld (ix:4),b"},
	{0,4,3,0,0,0,0,&dd71,NULL,"ld (ix:4),c"},
	{0,4,3,0,0,0,0,&dd72,NULL,"ld (ix:4),d"},
	{0,4,3,0,0,0,0,&dd73,NULL,"ld (ix:4),e"},
	{0,4,3,0,0,0,0,&dd74,NULL,"ld (ix:4),h"},
	{0,4,3,0,0,0,0,&dd75,NULL,"ld (ix:4),l"},
	{0,4,0,0,0,0,0,&npr76,NULL,"halt"},
	{0,4,3,0,0,0,0,&dd77,NULL,"ld (ix:4),a"},

	{0,4,0,0,0,0,0,&npr78,NULL,"ld a,b"},
	{0,4,0,0,0,0,0,&npr79,NULL,"ld a,c"},
	{0,4,0,0,0,0,0,&npr7A,NULL,"ld a,d"},
	{0,4,0,0,0,0,0,&npr7B,NULL,"ld a,e"},
	{0,4,0,0,0,0,0,&dd7C,NULL,"ld a,hx"},
	{0,4,0,0,0,0,0,&dd7D,NULL,"ld a,lx"},
	{0,4,3,0,0,0,0,&dd7E,NULL,"ld a,(ix:4)"},
	{0,4,0,0,0,0,0,&npr7F,NULL,"ld a,a"},

	{0,4,0,0,0,0,0,&npr80,NULL,"add a,b"},
	{0,4,0,0,0,0,0,&npr81,NULL,"add a,c"},
	{0,4,0,0,0,0,0,&npr82,NULL,"add a,d"},
	{0,4,0,0,0,0,0,&npr83,NULL,"add a,e"},
	{0,4,0,0,0,0,0,&dd84,NULL,"add a,hx"},
	{0,4,0,0,0,0,0,&dd85,NULL,"add a,lx"},
	{0,4,3,0,0,0,0,&dd86,NULL,"add a,(ix:4)"},
	{0,4,0,0,0,0,0,&npr87,NULL,"add a,a"},

	{0,4,0,0,0,0,0,&npr88,NULL,"adc a,b"},
	{0,4,0,0,0,0,0,&npr89,NULL,"adc a,c"},
	{0,4,0,0,0,0,0,&npr8A,NULL,"adc a,d"},
	{0,4,0,0,0,0,0,&npr8B,NULL,"adc a,e"},
	{0,4,0,0,0,0,0,&dd8C,NULL,"adc a,hx"},
	{0,4,0,0,0,0,0,&dd8D,NULL,"adc a,lx"},
	{0,4,3,0,0,0,0,&dd8E,NULL,"adc a,(ix:4)"},
	{0,4,0,0,0,0,0,&npr8F,NULL,"adc a,a"},

	{0,4,0,0,0,0,0,&npr90,NULL,"sub b"},
	{0,4,0,0,0,0,0,&npr91,NULL,"sub c"},
	{0,4,0,0,0,0,0,&npr92,NULL,"sub d"},
	{0,4,0,0,0,0,0,&npr93,NULL,"sub e"},
	{0,4,0,0,0,0,0,&dd94,NULL,"sub hx"},
	{0,4,0,0,0,0,0,&dd95,NULL,"sub lx"},
	{0,4,3,0,0,0,0,&dd96,NULL,"sub (ix:4)"},
	{0,4,0,0,0,0,0,&npr97,NULL,"sub a"},

	{0,4,0,0,0,0,0,&npr98,NULL,"sbc a,b"},
	{0,4,0,0,0,0,0,&npr99,NULL,"sbc a,c"},
	{0,4,0,0,0,0,0,&npr9A,NULL,"sbc a,d"},
	{0,4,0,0,0,0,0,&npr9B,NULL,"sbc a,e"},
	{0,4,0,0,0,0,0,&dd9C,NULL,"sbc a,hx"},
	{0,4,0,0,0,0,0,&dd9D,NULL,"sbc a,lx"},
	{0,4,3,0,0,0,0,&dd9E,NULL,"sbc a,(ix:4)"},
	{0,4,0,0,0,0,0,&npr9F,NULL,"sbc a,a"},

	{0,4,0,0,0,0,0,&nprA0,NULL,"and b"},
	{0,4,0,0,0,0,0,&nprA1,NULL,"and c"},
	{0,4,0,0,0,0,0,&nprA2,NULL,"and d"},
	{0,4,0,0,0,0,0,&nprA3,NULL,"and e"},
	{0,4,0,0,0,0,0,&ddA4,NULL,"and hx"},
	{0,4,0,0,0,0,0,&ddA5,NULL,"and lx"},
	{0,4,3,0,0,0,0,&ddA6,NULL,"and (ix:4)"},
	{0,4,0,0,0,0,0,&nprA7,NULL,"and a"},

	{0,4,0,0,0,0,0,&nprA8,NULL,"xor b"},
	{0,4,0,0,0,0,0,&nprA9,NULL,"xor c"},
	{0,4,0,0,0,0,0,&nprAA,NULL,"xor d"},
	{0,4,0,0,0,0,0,&nprAB,NULL,"xor e"},
	{0,4,0,0,0,0,0,&ddAC,NULL,"xor hx"},
	{0,4,0,0,0,0,0,&ddAD,NULL,"xor lx"},
	{0,4,3,0,0,0,0,&ddAE,NULL,"xor (ix:4)"},
	{0,4,0,0,0,0,0,&nprAF,NULL,"xor a"},

	{0,4,0,0,0,0,0,&nprB0,NULL,"or b"},
	{0,4,0,0,0,0,0,&nprB1,NULL,"or c"},
	{0,4,0,0,0,0,0,&nprB2,NULL,"or d"},
	{0,4,0,0,0,0,0,&nprB3,NULL,"or e"},
	{0,4,0,0,0,0,0,&ddB4,NULL,"or hx"},
	{0,4,0,0,0,0,0,&ddB5,NULL,"or lx"},
	{0,4,3,0,0,0,0,&ddB6,NULL,"or (ix:4)"},
	{0,4,0,0,0,0,0,&nprB7,NULL,"or a"},

	{0,4,0,0,0,0,0,&nprB8,NULL,"cp b"},
	{0,4,0,0,0,0,0,&nprB9,NULL,"cp c"},
	{0,4,0,0,0,0,0,&nprBA,NULL,"cp d"},
	{0,4,0,0,0,0,0,&nprBB,NULL,"cp e"},
	{0,4,0,0,0,0,0,&ddBC,NULL,"cp hx"},
	{0,4,0,0,0,0,0,&ddBD,NULL,"cp lx"},
	{0,4,3,0,0,0,0,&ddBE,NULL,"cp (ix:4)"},
	{0,4,0,0,0,0,0,&nprBF,NULL,"cp a"},

	{0,5,3,3,0,0,0,&nprC0,NULL,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,3,3,0,0,0,&nprC1,NULL,"pop bc"},
	{0,4,3,3,0,0,0,&nprC2,NULL,"jp nz,:2"},
	{0,4,3,3,0,0,0,&nprC3,NULL,"jp :2"},
	{0,4,3,4,3,3,0,&nprC4,NULL,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,3,3,0,0,0,&nprC5,NULL,"push bc"},		// 5 3wr 3wr
	{0,4,3,0,0,0,0,&nprC6,NULL,"add a,:1"},
	{0,5,3,3,0,0,0,&nprC7,NULL,"rst #00"},		// 5 3wr 3wr

	{0,5,3,3,0,0,0,&nprC8,NULL,"ret z"},
	{0,4,3,3,0,0,0,&nprC9,NULL,"ret"},
	{0,4,3,3,0,0,0,&nprCA,NULL,"jp z,:2"},
	{1,4,0,0,0,0,0,&ddCB,ddcbTab,"#CB"},
	{0,4,3,4,3,3,0,&nprCC,NULL,"call z,:2"},
	{0,4,3,4,3,3,0,&nprCD,NULL,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,3,0,0,0,0,&nprCE,NULL,"adc a,:1"},
	{0,5,3,3,0,0,0,&nprCF,NULL,"rst #08"},

	{0,5,3,3,0,0,0,&nprD0,NULL,"ret nc"},
	{0,4,3,3,0,0,0,&nprD1,NULL,"pop de"},
	{0,4,3,3,0,0,0,&nprD2,NULL,"jp nc,:2"},
	{0,4,3,4,0,0,0,&nprD3,NULL,"out (:1),a"},
	{0,4,3,4,3,3,0,&nprD4,NULL,"call nc,:2"},
	{0,5,3,3,0,0,0,&nprD5,NULL,"push de"},
	{0,4,3,0,0,0,0,&nprD6,NULL,"sub :1"},
	{0,5,3,3,0,0,0,&nprD7,NULL,"rst #10"},

	{0,5,3,3,0,0,0,&nprD8,NULL,"ret c"},
	{0,4,0,0,0,0,0,&nprD9,NULL,"exx"},
	{0,4,3,3,0,0,0,&nprDA,NULL,"jp c,:2"},
	{0,4,3,4,0,0,0,&nprDB,NULL,"in a,(:1)"},
	{0,4,3,4,3,3,0,&nprDC,NULL,"call c,:2"},
	{1,4,0,0,0,0,0,&nprDD,ddTab,"#DD"},
	{0,4,3,0,0,0,0,&nprDE,NULL,"sbc a,:1"},
	{0,5,3,3,0,0,0,&nprDF,NULL,"rst #18"},

	{0,5,3,3,0,0,0,&nprE0,NULL,"ret po"},
	{0,4,3,3,0,0,0,&ddE1,NULL,"pop ix"},
	{0,4,3,3,0,0,0,&nprE2,NULL,"jp po,:2"},
	{0,4,3,4,3,5,0,&ddE3,NULL,"ex (sp),ix"},		// 4 3rd 4rd 3wr 5wr
	{0,4,6,7,0,0,0,&nprE4,NULL,"call po,:2"},
	{0,5,3,3,0,0,0,&ddE5,NULL,"push ix"},
	{0,4,3,0,0,0,0,&nprE6,NULL,"and :1"},
	{0,5,3,3,0,0,0,&nprE7,NULL,"rst #20"},

	{0,5,3,3,0,0,0,&nprE8,NULL,"ret pe"},
	{0,4,0,0,0,0,0,&ddE9,NULL,"jp (ix)"},
	{0,4,3,3,0,0,0,&nprEA,NULL,"jp pe,:2"},
	{0,4,0,0,0,0,0,&nprEB,NULL,"ex de,hl"},
	{0,4,3,4,3,3,0,&nprEC,NULL,"call pe,:2"},
	{1,4,0,0,0,0,0,&nprED,edTab,"#ED"},
	{0,4,3,0,0,0,0,&nprEE,NULL,"xor :1"},
	{0,5,3,3,0,0,0,&nprEF,NULL,"rst #28"},

	{0,5,3,3,0,0,0,&nprF0,NULL,"ret p"},
	{0,4,3,3,0,0,0,&nprF1,NULL,"pop af"},
	{0,4,3,3,0,0,0,&nprF2,NULL,"jp p,:2"},
	{0,4,0,0,0,0,0,&nprF3,NULL,"di"},
	{0,4,3,4,3,3,0,&nprF4,NULL,"call p,:2"},
	{0,5,3,3,0,0,0,&nprF5,NULL,"push af"},
	{0,4,3,0,0,0,0,&nprF6,NULL,"or :1"},
	{0,5,3,3,0,0,0,&nprF7,NULL,"rst #30"},

	{0,5,3,3,0,0,0,&nprF8,NULL,"ret m"},
	{0,6,0,0,0,0,0,&ddF9,NULL,"ld sp,ix"},
	{0,4,3,3,0,0,0,&nprFA,NULL,"jp m,:2"},
	{0,4,0,0,0,0,0,&nprFB,NULL,"ei"},
	{0,4,3,4,3,3,0,&nprFC,NULL,"call m,:2"},
	{1,4,0,0,0,0,0,&nprFD,fdTab,"#FD"},
	{0,4,3,3,0,0,0,&nprFE,NULL,"cp :1"},
	{0,5,3,3,0,0,0,&nprFF,NULL,"rst #38"}
};
