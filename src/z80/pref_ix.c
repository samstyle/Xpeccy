// add ix,rp
void ixp09(Spec *p) {addIX(p,p->cpu->bc);}
void ixp19(Spec *p) {addIX(p,p->cpu->de);}
void ixp29(Spec *p) {addIX(p,p->cpu->ix);}
void ixp39(Spec *p) {addIX(p,p->cpu->sp);}

// inc/dec/ld ix
void ixp23(Spec *p) {p->cpu->ix++;}
void ixp2B(Spec *p) {p->cpu->ix--;}
void ixp21(Spec *p) {p->cpu->lx = p->mem->rd(p->cpu->pc++); p->cpu->hx = p->mem->rd(p->cpu->pc++);}
// ld (nn),ix | ld ix,(nn)	mptr = nn + 1
void ixp22(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->lx); p->mem->wr(p->cpu->mptr,p->cpu->hx);}
void ixp2A(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->lx = p->mem->rd(p->cpu->mptr++); p->cpu->hx = p->mem->rd(p->cpu->mptr);}

// inc/dec/ld lx
void ixp2C(Spec *p) {p->cpu->lx++; p->cpu->f = flag[p->cpu->lx].inc | (p->cpu->f & FC);}
void ixp2D(Spec *p) {p->cpu->lx--; p->cpu->f = flag[p->cpu->lx].dec | (p->cpu->f & FC);}
void ixp2E(Spec *p) {p->cpu->lx = p->mem->rd(p->cpu->pc++);}
// inc/dec/ld hx
void ixp24(Spec *p) {p->cpu->hx++; p->cpu->f = flag[p->cpu->hx].inc | (p->cpu->f & FC);}
void ixp25(Spec *p) {p->cpu->hx--; p->cpu->f = flag[p->cpu->hx].dec | (p->cpu->f & FC);}
void ixp26(Spec *p) {p->cpu->hx = p->mem->rd(p->cpu->pc++);}
// inc/dec/ld (ix+e)	mptr = ix + e
void ixp34(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr) + 1; p->mem->wr(p->cpu->mptr,p->cpu->x); p->cpu->f = flag[p->cpu->x].inc | (p->cpu->f & FC);}
void ixp35(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr) - 1; p->mem->wr(p->cpu->mptr,p->cpu->x); p->cpu->f = flag[p->cpu->x].dec | (p->cpu->f & FC);}
void ixp36(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->mem->rd(p->cpu->pc++));}

// ld b,x
void ixp44(Spec *p) {p->cpu->b = p->cpu->hx;}
void ixp45(Spec *p) {p->cpu->b = p->cpu->lx;}
void ixp46(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->b = p->mem->rd(p->cpu->mptr);}
// ld c,x
void ixp4C(Spec *p) {p->cpu->c = p->cpu->hx;}
void ixp4D(Spec *p) {p->cpu->c = p->cpu->lx;}
void ixp4E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->c = p->mem->rd(p->cpu->mptr);}
// ld d,x
void ixp54(Spec *p) {p->cpu->d = p->cpu->hx;}
void ixp55(Spec *p) {p->cpu->d = p->cpu->lx;}
void ixp56(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->d = p->mem->rd(p->cpu->mptr);}
// ld e,x
void ixp5C(Spec *p) {p->cpu->e = p->cpu->hx;}
void ixp5D(Spec *p) {p->cpu->e = p->cpu->lx;}
void ixp5E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->e = p->mem->rd(p->cpu->mptr);}
// ld hx,r
void ixp60(Spec *p) {p->cpu->hx = p->cpu->b;}
void ixp61(Spec *p) {p->cpu->hx = p->cpu->c;}
void ixp62(Spec *p) {p->cpu->hx = p->cpu->d;}
void ixp63(Spec *p) {p->cpu->hx = p->cpu->e;}
void ixp64(Spec*) {}
void ixp65(Spec *p) {p->cpu->hx = p->cpu->lx;}
void ixp66(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->h = p->mem->rd(p->cpu->mptr);}
void ixp67(Spec *p) {p->cpu->hx = p->cpu->a;}
// ld lx,r
void ixp68(Spec *p) {p->cpu->lx = p->cpu->b;}
void ixp69(Spec *p) {p->cpu->lx = p->cpu->c;}
void ixp6A(Spec *p) {p->cpu->lx = p->cpu->d;}
void ixp6B(Spec *p) {p->cpu->lx = p->cpu->e;}
void ixp6C(Spec *p) {p->cpu->lx = p->cpu->hx;}
void ixp6D(Spec*) {}
void ixp6E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->l = p->mem->rd(p->cpu->mptr);}
void ixp6F(Spec *p) {p->cpu->lx = p->cpu->a;}
// ld (ix+e),r
void ixp70(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->b);}
void ixp71(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->c);}
void ixp72(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->d);}
void ixp73(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->e);}
void ixp74(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->h);}
void ixp75(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->l);}
void ixp77(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->a);}
// ld a,x
void ixp7C(Spec *p) {p->cpu->a = p->cpu->hx;}
void ixp7D(Spec *p) {p->cpu->a = p->cpu->lx;}
void ixp7E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a = p->mem->rd(p->cpu->mptr);}
// add a,x
void ixp84(Spec *p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->hx][0]; p->cpu->a += p->cpu->hx;}
void ixp85(Spec *p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->lx][0]; p->cpu->a += p->cpu->lx;}
void ixp86(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].add[p->cpu->x][0]; p->cpu->a += p->cpu->x;}
// adc a,x
void ixp8C(Spec *p) {adcXX(p,p->cpu->hx);}
void ixp8D(Spec *p) {adcXX(p,p->cpu->lx);}
void ixp8E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); adcXX(p,p->mem->rd(p->cpu->mptr));}
// sub x
void ixp94(Spec *p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->hx][0]; p->cpu->a -= p->cpu->hx;}
void ixp95(Spec *p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->lx][0]; p->cpu->a -= p->cpu->lx;}
void ixp96(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].sub[p->cpu->x][0]; p->cpu->a -= p->cpu->x;}
// sbc a,x
void ixp9C(Spec *p) {sbcXX(p,p->cpu->hx);}
void ixp9D(Spec *p) {sbcXX(p,p->cpu->lx);}
void ixp9E(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); sbcXX(p,p->mem->rd(p->cpu->mptr));}
// and x
void ixpA4(Spec *p) {p->cpu->a &= p->cpu->hx; p->cpu->f = flag[p->cpu->a].andf;}
void ixpA5(Spec *p) {p->cpu->a &= p->cpu->lx; p->cpu->f = flag[p->cpu->a].andf;}
void ixpA6(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a &= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].andf;}
// xor x
void ixpAC(Spec *p) {p->cpu->a ^= p->cpu->hx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpAD(Spec *p) {p->cpu->a ^= p->cpu->lx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpAE(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a ^= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].orf;}
// or x
void ixpB4(Spec *p) {p->cpu->a |= p->cpu->hx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpB5(Spec *p) {p->cpu->a |= p->cpu->lx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpB6(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a |= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].orf;}
// cp x
void ixpBC(Spec *p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->hx][0];}
void ixpBD(Spec *p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->lx][0];}
void ixpBE(Spec *p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].sub[p->mem->rd(p->cpu->mptr)][0];}
// end - pop ix;push ix;ex (sp),ix;jp (ix);ld sp,ix
void ixpE1(Spec *p) {p->cpu->lx = p->mem->rd(p->cpu->sp++); p->cpu->hx = p->mem->rd(p->cpu->sp++);}	// pop
void ixpE5(Spec *p) {p->mem->wr(--p->cpu->sp,p->cpu->hx); p->mem->wr(--p->cpu->sp,p->cpu->lx);}		// push
void ixpE3(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->sp++); p->cpu->hptr = p->mem->rd(p->cpu->sp); p->mem->wr(p->cpu->sp--,p->cpu->hx); p->mem->wr(p->cpu->sp,p->cpu->lx); p->cpu->ix = p->cpu->mptr;}	// mptr = rp after operation
void ixpE9(Spec *p) {p->cpu->pc = p->cpu->ix;}
void ixpF9(Spec *p) {p->cpu->sp = p->cpu->ix;}

//==================

ZOp ixpref[256]={
	ZOp(&npr00,4,"nop"),
	ZOp(&npr01,10,"ld bc,:2"),
	ZOp(&npr02,7,"ld (bc),a"),
	ZOp(&npr03,6,"inc bc"),
	ZOp(&npr04,4,"inc b"),
	ZOp(&npr05,4,"dec b"),
	ZOp(&npr06,7,"ld b,:1"),
	ZOp(&npr07,4,"rlca"),

	ZOp(&npr08,4,"ex af,af'"),
	ZOp(&ixp09,11,"add ix,bc"),
	ZOp(&npr0A,7,"ld a,(bc)"),
	ZOp(&npr0B,6,"dec bc"),
	ZOp(&npr0C,4,"inc c"),
	ZOp(&npr0D,4,"dec c"),
	ZOp(&npr0E,7,"ld c,:1"),
	ZOp(&npr0F,4,"rrca"),

	ZOp(&npr10,8,"djnz :3"),
	ZOp(&npr11,10,"ld de,:2"),
	ZOp(&npr12,7,"ld (de),a"),
	ZOp(&npr13,6,"inc de"),
	ZOp(&npr14,4,"inc d"),
	ZOp(&npr15,4,"dec d"),
	ZOp(&npr16,7,"ld d,:1"),
	ZOp(&npr17,4,"rla"),

	ZOp(&npr18,12,"jr :3"),
	ZOp(&ixp19,11,"add ix,de"),
	ZOp(&npr1A,7,"ld a,(de)"),
	ZOp(&npr1B,6,"dec de"),
	ZOp(&npr1C,4,"inc e"),
	ZOp(&npr1D,4,"dec e"),
	ZOp(&npr1E,7,"ld e,:1"),
	ZOp(&npr1F,4,"rra"),

	ZOp(&npr20,7,"jr nz,:3"),
	ZOp(&ixp21,10,"ld ix,:2"),
	ZOp(&ixp22,16,"ld (:2),ix"),
	ZOp(&ixp23,6,"inc ix"),
	ZOp(&ixp24,4,"inc hx"),
	ZOp(&ixp25,4,"dec hx"),
	ZOp(&ixp26,7,"ld hx,:1"),
	ZOp(&npr27,4,"daa"),

	ZOp(&npr28,7,"jr z,:3"),
	ZOp(&ixp29,11,"add ix,ix"),
	ZOp(&ixp2A,16,"ld ix,(:2)"),
	ZOp(&ixp2B,6,"dec ix"),
	ZOp(&ixp2C,4,"inc lx"),
	ZOp(&ixp2D,4,"dec lx"),
	ZOp(&ixp2E,7,"ld lx,:1"),
	ZOp(&npr2F,4,"cpl"),

	ZOp(&npr30,7,"jr nc,:3"),
	ZOp(&npr31,10,"ld sp,:2"),
	ZOp(&npr32,13,"ld (:2),a"),
	ZOp(&npr33,6,"inc sp"),
	ZOp(&ixp34,19,"inc (ix:4)"),
	ZOp(&ixp35,19,"dec (ix:4)"),
	ZOp(&ixp36,19,"ld (ix:4),:1"),
	ZOp(&npr37,4,"scf"),

	ZOp(&npr38,7,"jr c,:3"),
	ZOp(&ixp39,11,"add ix,sp"),
	ZOp(&npr3A,13,"ld a,(:2)"),
	ZOp(&npr3B,6,"dec sp"),
	ZOp(&npr3C,4,"inc a"),
	ZOp(&npr3D,4,"dec a"),
	ZOp(&npr3E,7,"ld a,n"),
	ZOp(&npr3F,4,"ccf"),

	ZOp(&npr40,4,"ld b,b"),
	ZOp(&npr41,4,"ld b,c"),
	ZOp(&npr42,4,"ld b,d"),
	ZOp(&npr43,4,"ld b,e"),
	ZOp(&ixp44,4,"ld b,hx"),
	ZOp(&ixp45,4,"ld b,lx"),
	ZOp(&ixp46,15,"ld b,(ix:4)"),
	ZOp(&npr47,4,"ld b,a"),

	ZOp(&npr48,4,"ld c,b"),
	ZOp(&npr49,4,"ld c,c"),
	ZOp(&npr4A,4,"ld c,d"),
	ZOp(&npr4B,4,"ld c,e"),
	ZOp(&ixp4C,4,"ld c,hx"),
	ZOp(&ixp4D,4,"ld c,lx"),
	ZOp(&ixp4E,15,"ld c,(ix:4)"),
	ZOp(&npr4F,4,"ld c,a"),

	ZOp(&npr50,4,"ld d,b"),
	ZOp(&npr51,4,"ld d,c"),
	ZOp(&npr52,4,"ld d,d"),
	ZOp(&npr53,4,"ld d,e"),
	ZOp(&ixp54,4,"ld d,hx"),
	ZOp(&ixp55,4,"ld d,lx"),
	ZOp(&ixp56,15,"ld d,(ix:4)"),
	ZOp(&npr57,4,"ld d,a"),

	ZOp(&npr58,4,"ld e,b"),
	ZOp(&npr59,4,"ld e,c"),
	ZOp(&npr5A,4,"ld e,d"),
	ZOp(&npr5B,4,"ld e,e"),
	ZOp(&ixp5C,4,"ld e,hx"),
	ZOp(&ixp5D,4,"ld e,lx"),
	ZOp(&ixp5E,15,"ld e,(ix:4)"),
	ZOp(&npr5F,4,"ld e,a"),

	ZOp(&ixp60,4,"ld hx,b"),
	ZOp(&ixp61,4,"ld hx,c"),
	ZOp(&ixp62,4,"ld hx,d"),
	ZOp(&ixp63,4,"ld hx,e"),
	ZOp(&ixp64,4,"ld hx,hx"),
	ZOp(&ixp65,4,"ld hx,lx"),
	ZOp(&ixp66,15,"ld h,(ix:4)"),
	ZOp(&ixp67,4,"ld hx,a"),

	ZOp(&ixp68,4,"ld lx,b"),
	ZOp(&ixp69,4,"ld lx,c"),
	ZOp(&ixp6A,4,"ld lx,d"),
	ZOp(&ixp6B,4,"ld lx,e"),
	ZOp(&ixp6C,4,"ld lx,hx"),
	ZOp(&ixp6D,4,"ld lx,lx"),
	ZOp(&ixp6E,15,"ld l,(ix:4)"),
	ZOp(&ixp6F,4,"ld lx,a"),

	ZOp(&ixp70,15,"ld (ix:4),b"),
	ZOp(&ixp71,15,"ld (ix:4),c"),
	ZOp(&ixp72,15,"ld (ix:4),d"),
	ZOp(&ixp73,15,"ld (ix:4),e"),
	ZOp(&ixp74,15,"ld (ix:4),h"),
	ZOp(&ixp75,15,"ld (ix:4),l"),
	ZOp(&npr76,4,"halt"),
	ZOp(&ixp77,15,"ld (ix:4),a"),

	ZOp(&npr78,4,"ld a,b"),
	ZOp(&npr79,4,"ld a,c"),
	ZOp(&npr7A,4,"ld a,d"),
	ZOp(&npr7B,4,"ld a,e"),
	ZOp(&ixp7C,4,"ld a,hx"),
	ZOp(&ixp7D,4,"ld a,lx"),
	ZOp(&ixp7E,15,"ld a,(ix:4)"),
	ZOp(&npr7F,4,"ld a,a"),

	ZOp(&npr80,4,"add a,b"),
	ZOp(&npr81,4,"add a,c"),
	ZOp(&npr82,4,"add a,d"),
	ZOp(&npr83,4,"add a,e"),
	ZOp(&ixp84,4,"add a,hx"),
	ZOp(&ixp85,4,"add a,lx"),
	ZOp(&ixp86,15,"add a,(ix:4)"),
	ZOp(&npr87,4,"add a,a"),

	ZOp(&npr88,4,"adc a,b"),
	ZOp(&npr89,4,"adc a,c"),
	ZOp(&npr8A,4,"adc a,d"),
	ZOp(&npr8B,4,"adc a,e"),
	ZOp(&ixp8C,4,"adc a,hx"),
	ZOp(&ixp8D,4,"adc a,lx"),
	ZOp(&ixp8E,15,"adc a,(ix:4)"),
	ZOp(&npr8F,4,"adc a,a"),

	ZOp(&npr90,4,"sub b"),
	ZOp(&npr91,4,"sub c"),
	ZOp(&npr92,4,"sub d"),
	ZOp(&npr93,4,"sub e"),
	ZOp(&ixp94,4,"sub hx"),
	ZOp(&ixp95,4,"sub lx"),
	ZOp(&ixp96,15,"sub (ix:4)"),
	ZOp(&npr97,4,"sub a"),

	ZOp(&npr98,4,"sbc a,b"),
	ZOp(&npr99,4,"sbc a,c"),
	ZOp(&npr9A,4,"sbc a,d"),
	ZOp(&npr9B,4,"sbc a,e"),
	ZOp(&ixp9C,4,"sbc a,hx"),
	ZOp(&ixp9D,4,"sbc a,lx"),
	ZOp(&ixp9E,15,"sbc a,(ix:4)"),
	ZOp(&npr9F,4,"sbc a,a"),

	ZOp(&nprA0,4,"and b"),
	ZOp(&nprA1,4,"and c"),
	ZOp(&nprA2,4,"and d"),
	ZOp(&nprA3,4,"and e"),
	ZOp(&ixpA4,4,"and hx"),
	ZOp(&ixpA5,4,"and lx"),
	ZOp(&ixpA6,15,"and (ix:4)"),
	ZOp(&nprA7,4,"and a"),

	ZOp(&nprA8,4,"xor b"),
	ZOp(&nprA9,4,"xor c"),
	ZOp(&nprAA,4,"xor d"),
	ZOp(&nprAB,4,"xor e"),
	ZOp(&ixpAC,4,"xor hx"),
	ZOp(&ixpAD,4,"xor lx"),
	ZOp(&ixpAE,15,"xor (ix:4)"),
	ZOp(&nprAF,4,"xor a"),

	ZOp(&nprB0,4,"or b"),
	ZOp(&nprB1,4,"or с"),
	ZOp(&nprB2,4,"or d"),
	ZOp(&nprB3,4,"or e"),
	ZOp(&ixpB4,4,"or hx"),
	ZOp(&ixpB5,4,"or lx"),
	ZOp(&ixpB6,15,"or (ix:4)"),
	ZOp(&nprB7,4,"or a"),

	ZOp(&nprB8,4,"cp b"),
	ZOp(&nprB9,4,"cp c"),
	ZOp(&nprBA,4,"cp d"),
	ZOp(&nprBB,4,"cp e"),
	ZOp(&ixpBC,4,"cp hx"),
	ZOp(&ixpBD,4,"cp lx"),
	ZOp(&ixpBE,15,"cp (ix:4)"),
	ZOp(&nprBF,4,"cp a"),

	ZOp(&nprC0,5,"ret nz"),
	ZOp(&nprC1,10,"pop bc"),
	ZOp(&nprC2,10,"jp nz,:2"),
	ZOp(&nprC3,10,"jp :2"),
	ZOp(&nprC4,10,"call nz,:2"),
	ZOp(&nprC5,11,"push bc"),
	ZOp(&nprC6,7,"add a,:1"),
	ZOp(&nprC7,11,"rst #00"),

	ZOp(&nprC8,5,"ret z"),
	ZOp(&nprC9,10,"ret"),
	ZOp(&nprCA,10,"jp z,:2"),
	ZOp(&nprCB,4,"#CB",true),
	ZOp(&nprCC,10,"call z,:2"),
	ZOp(&nprCD,10,"call :2"),	// [+4 +3]: acts like CALL true,nn
	ZOp(&nprCE,7,"adc a,:1"),
	ZOp(&nprCF,11,"rst #08"),

	ZOp(&nprD0,5,"ret nc"),
	ZOp(&nprD1,10,"pop de"),
	ZOp(&nprD2,10,"jp nc,:2"),
	ZOp(&nprD3,11,"out (:2),a"),
	ZOp(&nprD4,10,"call nc,:2"),
	ZOp(&nprD5,11,"push de"),
	ZOp(&nprD6,7,"sub :1"),
	ZOp(&nprD7,11,"rst #10"),

	ZOp(&nprD8,5,"ret c"),
	ZOp(&nprD9,4,"exx"),
	ZOp(&nprDA,10,"jp c,:2"),
	ZOp(&nprDB,11,"in a,(:1)"),
	ZOp(&nprDC,10,"call c,:2"),
	ZOp(&nprDD,4,"#DD",true),
	ZOp(&nprDE,7,"sbc a,:1"),
	ZOp(&nprDF,11,"rst #18"),

	ZOp(&nprE0,5,"ret po"),
	ZOp(&ixpE1,10,"pop ix"),
	ZOp(&nprE2,10,"jp po,:2"),
	ZOp(&ixpE3,19,"ex (sp),ix"),
	ZOp(&nprE4,10,"call c,:2"),
	ZOp(&ixpE5,11,"push ix"),
	ZOp(&nprE6,7,"and :1"),
	ZOp(&nprE7,11,"rst #20"),

	ZOp(&nprE8,5,"ret pe"),
	ZOp(&ixpE9,4,"jp (ix)"),
	ZOp(&nprEA,10,"jp pe,:2"),
	ZOp(&nprEB,4,"ex de,hl"),
	ZOp(&nprEC,10,"call pe,:2"),
	ZOp(&nprED,4,"#ED",true),
	ZOp(&nprEE,7,"xor :1"),
	ZOp(&nprEF,11,"rst #28"),

	ZOp(&nprF0,5,"ret p"),
	ZOp(&nprF1,10,"pop af"),
	ZOp(&nprF2,10,"jp p,:2"),
	ZOp(&nprF3,4,"di"),
	ZOp(&nprF4,10,"call p,:2"),
	ZOp(&nprF5,11,"push af"),
	ZOp(&nprF6,7,"xor :1"),
	ZOp(&nprF7,11,"rst #30"),

	ZOp(&nprF8,5,"ret m"),
	ZOp(&ixpF9,6,"ld sp,ix"),
	ZOp(&nprFA,10,"jp m,:2"),
	ZOp(&nprFB,4,"ei"),
	ZOp(&nprFC,10,"call m,:2"),
	ZOp(&nprFD,4,"#FD",true),
	ZOp(&nprFE,7,"cp :1"),
	ZOp(&nprFF,11,"rst #38")
};

