// add ix,rp
void ixp09(ZXBase* p) {addIX(p,p->cpu->bc);}
void ixp19(ZXBase* p) {addIX(p,p->cpu->de);}
void ixp29(ZXBase* p) {addIX(p,p->cpu->ix);}
void ixp39(ZXBase* p) {addIX(p,p->cpu->sp);}
// inc/dec/ld ix
void ixp23(ZXBase* p) {p->cpu->ix++;}
void ixp2B(ZXBase* p) {p->cpu->ix--;}
void ixp21(ZXBase* p) {p->cpu->lx = p->mem->rd(p->cpu->pc++); p->cpu->hx = p->mem->rd(p->cpu->pc++);}
// ld (nn),ix | ld ix,(nn)	mptr = nn + 1
void ixp22(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->lx); p->mem->wr(p->cpu->mptr,p->cpu->hx);}
void ixp2A(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->lx = p->mem->rd(p->cpu->mptr++); p->cpu->hx = p->mem->rd(p->cpu->mptr);}
// inc/dec/ld lx
void ixp2C(ZXBase* p) {p->cpu->lx++; p->cpu->f = flag[p->cpu->lx].inc | (p->cpu->f & FC);}
void ixp2D(ZXBase* p) {p->cpu->lx--; p->cpu->f = flag[p->cpu->lx].dec | (p->cpu->f & FC);}
void ixp2E(ZXBase* p) {p->cpu->lx = p->mem->rd(p->cpu->pc++);}
// inc/dec/ld hx
void ixp24(ZXBase* p) {p->cpu->hx++; p->cpu->f = flag[p->cpu->hx].inc | (p->cpu->f & FC);}
void ixp25(ZXBase* p) {p->cpu->hx--; p->cpu->f = flag[p->cpu->hx].dec | (p->cpu->f & FC);}
void ixp26(ZXBase* p) {p->cpu->hx = p->mem->rd(p->cpu->pc++);}
// inc/dec/ld (ix+e)	mptr = ix + e
void ixp34(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr) + 1; p->mem->wr(p->cpu->mptr,p->cpu->x); p->cpu->f = flag[p->cpu->x].inc | (p->cpu->f & FC);}
void ixp35(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr) - 1; p->mem->wr(p->cpu->mptr,p->cpu->x); p->cpu->f = flag[p->cpu->x].dec | (p->cpu->f & FC);}
void ixp36(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->mem->rd(p->cpu->pc++));}
// ld b,x
void ixp44(ZXBase* p) {p->cpu->b = p->cpu->hx;}
void ixp45(ZXBase* p) {p->cpu->b = p->cpu->lx;}
void ixp46(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->b = p->mem->rd(p->cpu->mptr);}
// ld c,x
void ixp4C(ZXBase* p) {p->cpu->c = p->cpu->hx;}
void ixp4D(ZXBase* p) {p->cpu->c = p->cpu->lx;}
void ixp4E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->c = p->mem->rd(p->cpu->mptr);}
// ld d,x
void ixp54(ZXBase* p) {p->cpu->d = p->cpu->hx;}
void ixp55(ZXBase* p) {p->cpu->d = p->cpu->lx;}
void ixp56(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->d = p->mem->rd(p->cpu->mptr);}
// ld e,x
void ixp5C(ZXBase* p) {p->cpu->e = p->cpu->hx;}
void ixp5D(ZXBase* p) {p->cpu->e = p->cpu->lx;}
void ixp5E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->e = p->mem->rd(p->cpu->mptr);}
// ld hx,r
void ixp60(ZXBase* p) {p->cpu->hx = p->cpu->b;}
void ixp61(ZXBase* p) {p->cpu->hx = p->cpu->c;}
void ixp62(ZXBase* p) {p->cpu->hx = p->cpu->d;}
void ixp63(ZXBase* p) {p->cpu->hx = p->cpu->e;}
void ixp64(ZXBase*) {}
void ixp65(ZXBase* p) {p->cpu->hx = p->cpu->lx;}
void ixp66(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->h = p->mem->rd(p->cpu->mptr);}
void ixp67(ZXBase* p) {p->cpu->hx = p->cpu->a;}
// ld lx,r
void ixp68(ZXBase* p) {p->cpu->lx = p->cpu->b;}
void ixp69(ZXBase* p) {p->cpu->lx = p->cpu->c;}
void ixp6A(ZXBase* p) {p->cpu->lx = p->cpu->d;}
void ixp6B(ZXBase* p) {p->cpu->lx = p->cpu->e;}
void ixp6C(ZXBase* p) {p->cpu->lx = p->cpu->hx;}
void ixp6D(ZXBase*) {}
void ixp6E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->l = p->mem->rd(p->cpu->mptr);}
void ixp6F(ZXBase* p) {p->cpu->lx = p->cpu->a;}
// ld (ix+e),r
void ixp70(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->b);}
void ixp71(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->c);}
void ixp72(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->d);}
void ixp73(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->e);}
void ixp74(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->h);}
void ixp75(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->l);}
void ixp77(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->a);}
// ld a,x
void ixp7C(ZXBase* p) {p->cpu->a = p->cpu->hx;}
void ixp7D(ZXBase* p) {p->cpu->a = p->cpu->lx;}
void ixp7E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a = p->mem->rd(p->cpu->mptr);}
// add a,x
void ixp84(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->hx][0]; p->cpu->a += p->cpu->hx;}
void ixp85(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->lx][0]; p->cpu->a += p->cpu->lx;}
void ixp86(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].add[p->cpu->x][0]; p->cpu->a += p->cpu->x;}
// adc a,x
void ixp8C(ZXBase* p) {adcXX(p,p->cpu->hx);}
void ixp8D(ZXBase* p) {adcXX(p,p->cpu->lx);}
void ixp8E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); adcXX(p,p->mem->rd(p->cpu->mptr));}
// sub x
void ixp94(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->hx][0]; p->cpu->a -= p->cpu->hx;}
void ixp95(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->lx][0]; p->cpu->a -= p->cpu->lx;}
void ixp96(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].sub[p->cpu->x][0]; p->cpu->a -= p->cpu->x;}
// sbc a,x
void ixp9C(ZXBase* p) {sbcXX(p,p->cpu->hx);}
void ixp9D(ZXBase* p) {sbcXX(p,p->cpu->lx);}
void ixp9E(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); sbcXX(p,p->mem->rd(p->cpu->mptr));}
// and x
void ixpA4(ZXBase* p) {p->cpu->a &= p->cpu->hx; p->cpu->f = flag[p->cpu->a].andf;}
void ixpA5(ZXBase* p) {p->cpu->a &= p->cpu->lx; p->cpu->f = flag[p->cpu->a].andf;}
void ixpA6(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a &= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].andf;}
// xor x
void ixpAC(ZXBase* p) {p->cpu->a ^= p->cpu->hx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpAD(ZXBase* p) {p->cpu->a ^= p->cpu->lx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpAE(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a ^= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].orf;}
// or x
void ixpB4(ZXBase* p) {p->cpu->a |= p->cpu->hx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpB5(ZXBase* p) {p->cpu->a |= p->cpu->lx; p->cpu->f = flag[p->cpu->a].orf;}
void ixpB6(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a |= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].orf;}
// cp x
void ixpBC(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->hx][0];}
void ixpBD(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->lx][0];}
void ixpBE(ZXBase* p) {p->cpu->mptr = p->cpu->ix + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].sub[p->mem->rd(p->cpu->mptr)][0];}
// end - pop ix;push ix;ex (sp),ix;jp (ix);ld sp,ix
void ixpE1(ZXBase* p) {p->cpu->lx = p->mem->rd(p->cpu->sp++); p->cpu->hx = p->mem->rd(p->cpu->sp++);}	// pop
void ixpE5(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hx); p->mem->wr(--p->cpu->sp,p->cpu->lx);}		// push
void ixpE3(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->sp++); p->cpu->hptr = p->mem->rd(p->cpu->sp); p->mem->wr(p->cpu->sp--,p->cpu->hx); p->mem->wr(p->cpu->sp,p->cpu->lx); p->cpu->ix = p->cpu->mptr;}	// mptr = rp after operation
void ixpE9(ZXBase* p) {p->cpu->pc = p->cpu->ix;}
void ixpF9(ZXBase* p) {p->cpu->sp = p->cpu->ix;}

//==================

ZOp ixpref[256]={
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop"},
	{10,	CND_NONE,0,0,	0,	&npr01,	"ld bc,:2"},
	{7,	CND_NONE,0,0,	0,	&npr02,	"ld (bc),a"},
	{6,	CND_NONE,0,0,	0,	&npr03,	"inc bc"},
	{4,	CND_NONE,0,0,	0,	&npr04,	"inc b"},
	{4,	CND_NONE,0,0,	0,	&npr05,	"dec b"},
	{7,	CND_NONE,0,0,	0,	&npr06,	"ld b,:1"},
	{4,	CND_NONE,0,0,	0,	&npr07,	"rlca"},

	{4,	CND_NONE,0,0,	0,	&npr08,	"ex af,af'"},
	{11,	CND_NONE,0,0,	0,	&ixp09,	"add ix,bc"},
	{7,	CND_NONE,0,0,	0,	&npr0A,	"ld a,(bc)"},
	{6,	CND_NONE,0,0,	0,	&npr0B,	"dec bc"},
	{4,	CND_NONE,0,0,	0,	&npr0C,	"inc c"},
	{4,	CND_NONE,0,0,	0,	&npr0D,	"dec c"},
	{7,	CND_NONE,0,0,	0,	&npr0E,	"ld c,:1"},
	{4,	CND_NONE,0,0,	0,	&npr0F,	"rrca"},

	{8,	CND_DJNZ,5,0,	0,	&npr10,	"djnz :3"},
	{10,	CND_NONE,0,0,	0,	&npr11,	"ld de,:2"},
	{7,	CND_NONE,0,0,	0,	&npr12,	"ld (de),a"},
	{6,	CND_NONE,0,0,	0,	&npr13,	"inc de"},
	{4,	CND_NONE,0,0,	0,	&npr14,	"inc d"},
	{4,	CND_NONE,0,0,	0,	&npr15,	"dec d"},
	{7,	CND_NONE,0,0,	0,	&npr16,	"ld d,:1"},
	{4,	CND_NONE,0,0,	0,	&npr17,	"rla"},

	{12,	CND_NONE,0,0,	0,	&npr18,	"jr :3"},
	{11,	CND_NONE,0,0,	0,	&ixp19,	"add ix,de"},
	{7,	CND_NONE,0,0,	0,	&npr1A,	"ld a,(de)"},
	{6,	CND_NONE,0,0,	0,	&npr1B,	"dec de"},
	{4,	CND_NONE,0,0,	0,	&npr1C,	"inc e"},
	{4,	CND_NONE,0,0,	0,	&npr1D,	"dec e"},
	{7,	CND_NONE,0,0,	0,	&npr1E,	"ld e,:1"},
	{4,	CND_NONE,0,0,	0,	&npr1F,	"rra"},

	{7,	CND_Z,0,5,	0,	&npr20,	"jr nz,:3"},
	{10,	CND_NONE,0,0,	0,	&ixp21,	"ld ix,:2"},
	{16,	CND_NONE,0,0,	0,	&ixp22,	"ld (:2),ix"},
	{6,	CND_NONE,0,0,	0,	&ixp23,	"inc ix"},
	{4,	CND_NONE,0,0,	0,	&ixp24,	"inc hx"},
	{4,	CND_NONE,0,0,	0,	&ixp25,	"dec hx"},
	{7,	CND_NONE,0,0,	0,	&ixp26,	"ld hx,:1"},
	{4,	CND_NONE,0,0,	0,	&npr27,	"daa"},

	{7,	CND_Z,5,0,	0,	&npr28,	"jr z,:3"},
	{11,	CND_NONE,0,0,	0,	&ixp29,	"add ix,ix"},
	{16,	CND_NONE,0,0,	0,	&ixp2A,	"ld ix,(:2)"},
	{6,	CND_NONE,0,0,	0,	&ixp2B,	"dec ix"},
	{4,	CND_NONE,0,0,	0,	&ixp2C,	"inc lx"},
	{4,	CND_NONE,0,0,	0,	&ixp2D,	"dec lx"},
	{7,	CND_NONE,0,0,	0,	&ixp2E,	"ld lx,:1"},
	{4,	CND_NONE,0,0,	0,	&npr2F,	"cpl"},

	{7,	CND_C,0,5,	0,	&npr30,	"jr nc,:3"},
	{10,	CND_NONE,0,0,	0,	&npr31,	"ld sp,:2"},
	{13,	CND_NONE,0,0,	0,	&npr32,	"ld (:2),a"},
	{6,	CND_NONE,0,0,	0,	&npr33,	"inc sp"},
	{19,	CND_NONE,0,0,	0,	&ixp34,	"inc (ix:4)"},
	{19,	CND_NONE,0,0,	0,	&ixp35,	"dec (ix:4)"},
	{19,	CND_NONE,0,0,	0,	&ixp36,	"ld (ix:4),:1"},
	{4,	CND_NONE,0,0,	0,	&npr37,	"scf"},

	{7,	CND_C,5,0,	0,	&npr38,	"jr c,:3"},
	{11,	CND_NONE,0,0,	0,	&ixp39,	"add ix,sp"},
	{13,	CND_NONE,0,0,	0,	&npr3A,	"ld a,(:2)"},
	{6,	CND_NONE,0,0,	0,	&npr3B,	"dec sp"},
	{4,	CND_NONE,0,0,	0,	&npr3C,	"inc a"},
	{4,	CND_NONE,0,0,	0,	&npr3D,	"dec a"},
	{7,	CND_NONE,0,0,	0,	&npr3E,	"ld a,:1"},
	{4,	CND_NONE,0,0,	0,	&npr3F,	"ccf"},

	{4,	CND_NONE,0,0,	0,	&npr40,	"ld b,b"},
	{4,	CND_NONE,0,0,	0,	&npr41,	"ld b,c"},
	{4,	CND_NONE,0,0,	0,	&npr42,	"ld b,d"},
	{4,	CND_NONE,0,0,	0,	&npr43,	"ld b,e"},
	{4,	CND_NONE,0,0,	0,	&ixp44,	"ld b,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp45,	"ld b,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp46,	"ld b,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr47,	"ld b,a"},

	{4,	CND_NONE,0,0,	0,	&npr48,	"ld c,b"},
	{4,	CND_NONE,0,0,	0,	&npr49,	"ld c,c"},
	{4,	CND_NONE,0,0,	0,	&npr4A,	"ld c,d"},
	{4,	CND_NONE,0,0,	0,	&npr4B,	"ld c,e"},
	{4,	CND_NONE,0,0,	0,	&ixp4C,	"ld c,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp4D,	"ld c,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp4E,	"ld c,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr4F,	"ld c,a"},

	{4,	CND_NONE,0,0,	0,	&npr50,	"ld d,b"},
	{4,	CND_NONE,0,0,	0,	&npr51,	"ld d,c"},
	{4,	CND_NONE,0,0,	0,	&npr52,	"ld d,d"},
	{4,	CND_NONE,0,0,	0,	&npr53,	"ld d,e"},
	{4,	CND_NONE,0,0,	0,	&ixp54,	"ld d,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp55,	"ld d,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp56,	"ld d,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr57,	"ld d,a"},

	{4,	CND_NONE,0,0,	0,	&npr58,	"ld e,b"},
	{4,	CND_NONE,0,0,	0,	&npr59,	"ld e,c"},
	{4,	CND_NONE,0,0,	0,	&npr5A,	"ld e,d"},
	{4,	CND_NONE,0,0,	0,	&npr5B,	"ld e,e"},
	{4,	CND_NONE,0,0,	0,	&ixp5C,	"ld e,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp5D,	"ld e,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp5E,	"ld e,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr5F,	"ld e,a"},

	{4,	CND_NONE,0,0,	0,	&ixp60,	"ld hx,b"},
	{4,	CND_NONE,0,0,	0,	&ixp61,	"ld hx,c"},
	{4,	CND_NONE,0,0,	0,	&ixp62,	"ld hx,d"},
	{4,	CND_NONE,0,0,	0,	&ixp63,	"ld hx,e"},
	{4,	CND_NONE,0,0,	0,	&ixp64,	"ld hx,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp65,	"ld hx,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp66,	"ld h,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&ixp67,	"ld hx,a"},

	{4,	CND_NONE,0,0,	0,	&ixp68,	"ld lx,b"},
	{4,	CND_NONE,0,0,	0,	&ixp69,	"ld lx,c"},
	{4,	CND_NONE,0,0,	0,	&ixp6A,	"ld lx,d"},
	{4,	CND_NONE,0,0,	0,	&ixp6B,	"ld lx,e"},
	{4,	CND_NONE,0,0,	0,	&ixp6C,	"ld lx,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp6D,	"ld lx,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp6E,	"ld l,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&ixp6F,	"ld lx,a"},

	{15,	CND_NONE,0,0,	0,	&ixp70,	"ld (ix:4),b"},
	{15,	CND_NONE,0,0,	0,	&ixp71,	"ld (ix:4),c"},
	{15,	CND_NONE,0,0,	0,	&ixp72,	"ld (ix:4),d"},
	{15,	CND_NONE,0,0,	0,	&ixp73,	"ld (ix:4),e"},
	{15,	CND_NONE,0,0,	0,	&ixp74,	"ld (ix:4),h"},
	{15,	CND_NONE,0,0,	0,	&ixp75,	"ld (ix:4),l"},
	{4,	CND_NONE,0,0,	0,	&npr76,	"halt"},
	{15,	CND_NONE,0,0,	0,	&ixp77,	"ld (ix:4),a"},

	{4,	CND_NONE,0,0,	0,	&npr78,	"ld a,b"},
	{4,	CND_NONE,0,0,	0,	&npr79,	"ld a,c"},
	{4,	CND_NONE,0,0,	0,	&npr7A,	"ld a,d"},
	{4,	CND_NONE,0,0,	0,	&npr7B,	"ld a,e"},
	{4,	CND_NONE,0,0,	0,	&ixp7C,	"ld a,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp7D,	"ld a,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp7E,	"ld a,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr7F,	"ld a,a"},

	{4,	CND_NONE,0,0,	0,	&npr80,	"add a,b"},
	{4,	CND_NONE,0,0,	0,	&npr81,	"add a,c"},
	{4,	CND_NONE,0,0,	0,	&npr82,	"add a,d"},
	{4,	CND_NONE,0,0,	0,	&npr83,	"add a,e"},
	{4,	CND_NONE,0,0,	0,	&ixp84,	"add a,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp85,	"add a,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp86,	"add a,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr87,	"add a,a"},

	{4,	CND_NONE,0,0,	0,	&npr88,	"adc a,b"},
	{4,	CND_NONE,0,0,	0,	&npr89,	"adc a,c"},
	{4,	CND_NONE,0,0,	0,	&npr8A,	"adc a,d"},
	{4,	CND_NONE,0,0,	0,	&npr8B,	"adc a,e"},
	{4,	CND_NONE,0,0,	0,	&ixp8C,	"adc a,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp8D,	"adc a,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp8E,	"adc a,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr8F,	"adc a,a"},

	{4,	CND_NONE,0,0,	0,	&npr90,	"sub b"},
	{4,	CND_NONE,0,0,	0,	&npr91,	"sub c"},
	{4,	CND_NONE,0,0,	0,	&npr92,	"sub d"},
	{4,	CND_NONE,0,0,	0,	&npr93,	"sub e"},
	{4,	CND_NONE,0,0,	0,	&ixp94,	"sub hx"},
	{4,	CND_NONE,0,0,	0,	&ixp95,	"sub lx"},
	{15,	CND_NONE,0,0,	0,	&ixp96,	"sub (ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr97,	"sub a"},

	{4,	CND_NONE,0,0,	0,	&npr98,	"sbc a,b"},
	{4,	CND_NONE,0,0,	0,	&npr99,	"sbc a,c"},
	{4,	CND_NONE,0,0,	0,	&npr9A,	"sbc a,d"},
	{4,	CND_NONE,0,0,	0,	&npr9B,	"sbc a,e"},
	{4,	CND_NONE,0,0,	0,	&ixp9C,	"sbc a,hx"},
	{4,	CND_NONE,0,0,	0,	&ixp9D,	"sbc a,lx"},
	{15,	CND_NONE,0,0,	0,	&ixp9E,	"sbc a,(ix:4)"},
	{4,	CND_NONE,0,0,	0,	&npr9F,	"sbc a,a"},

	{4,	CND_NONE,0,0,	0,	&nprA0,	"and b"},
	{4,	CND_NONE,0,0,	0,	&nprA1,	"and c"},
	{4,	CND_NONE,0,0,	0,	&nprA2,	"and d"},
	{4,	CND_NONE,0,0,	0,	&nprA3,	"and e"},
	{4,	CND_NONE,0,0,	0,	&ixpA4,	"and hx"},
	{4,	CND_NONE,0,0,	0,	&ixpA5,	"and lx"},
	{15,	CND_NONE,0,0,	0,	&ixpA6,	"and (ix:4)"},
	{4,	CND_NONE,0,0,	0,	&nprA7,	"and a"},

	{4,	CND_NONE,0,0,	0,	&nprA8,	"xor b"},
	{4,	CND_NONE,0,0,	0,	&nprA9,	"xor c"},
	{4,	CND_NONE,0,0,	0,	&nprAA,	"xor d"},
	{4,	CND_NONE,0,0,	0,	&nprAB,	"xor e"},
	{4,	CND_NONE,0,0,	0,	&ixpAC,	"xor hx"},
	{4,	CND_NONE,0,0,	0,	&ixpAD,	"xor lx"},
	{15,	CND_NONE,0,0,	0,	&ixpAE,	"xor (ix:4)"},
	{4,	CND_NONE,0,0,	0,	&nprAF,	"xor a"},

	{4,	CND_NONE,0,0,	0,	&nprB0,	"or b"},
	{4,	CND_NONE,0,0,	0,	&nprB1,	"or c"},
	{4,	CND_NONE,0,0,	0,	&nprB2,	"or d"},
	{4,	CND_NONE,0,0,	0,	&nprB3,	"or e"},
	{4,	CND_NONE,0,0,	0,	&ixpB4,	"or hx"},
	{4,	CND_NONE,0,0,	0,	&ixpB5,	"or lx"},
	{15,	CND_NONE,0,0,	0,	&ixpB6,	"or (ix:4)"},
	{4,	CND_NONE,0,0,	0,	&nprB7,	"or a"},

	{4,	CND_NONE,0,0,	0,	&nprB8,	"cp b"},
	{4,	CND_NONE,0,0,	0,	&nprB9,	"cp c"},
	{4,	CND_NONE,0,0,	0,	&nprBA,	"cp d"},
	{4,	CND_NONE,0,0,	0,	&nprBB,	"cp e"},
	{4,	CND_NONE,0,0,	0,	&ixpBC,	"cp hx"},
	{4,	CND_NONE,0,0,	0,	&ixpBD,	"cp lx"},
	{15,	CND_NONE,0,0,	0,	&ixpBE,	"cp (ix:4)"},
	{4,	CND_NONE,0,0,	0,	&nprBF,	"cp a"},

	{5,	CND_Z,0,6,	0,	&nprC0,	"ret nz"},
	{10,	CND_NONE,0,0,	0,	&nprC1,	"pop bc"},
	{10,	CND_NONE,0,0,	0,	&nprC2,	"jp nz,:2"},
	{10,	CND_NONE,0,0,	0,	&nprC3,	"jp :2"},
	{10,	CND_Z,0,7,	0,	&nprC4,	"call nz,:2"},
	{11,	CND_NONE,0,0,	0,	&nprC5,	"push bc"},
	{7,	CND_NONE,0,0,	0,	&nprC6,	"add a,:1"},
	{11,	CND_NONE,0,0,	0,	&nprC7,	"rst #00"},

	{5,	CND_Z,6,0,	0,	&nprC8,	"ret z"},
	{10,	CND_NONE,0,0,	0,	&nprC9,	"ret"},
	{10,	CND_NONE,0,0,	0,	&nprCA,	"jp z,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprCB,	"#CB"},
	{10,	CND_Z,7,0,	0,	&nprCC,	"call z,:2"},
	{17,	CND_NONE,0,0,	0,	&nprCD,	"call :2"},
	{7,	CND_NONE,0,0,	0,	&nprCE,	"adc a,:1"},
	{11,	CND_NONE,0,0,	0,	&nprCF,	"rst #08"},

	{5,	CND_C,0,6,	0,	&nprD0,	"ret nc"},
	{10,	CND_NONE,0,0,	0,	&nprD1,	"pop de"},
	{10,	CND_NONE,0,0,	0,	&nprD2,	"jp nc,:2"},
	{11,	CND_NONE,0,0,	0,	&nprD3,	"out (:1),a"},
	{10,	CND_C,0,7,	0,	&nprD4,	"call nc,:2"},
	{11,	CND_NONE,0,0,	0,	&nprD5,	"push de"},
	{7,	CND_NONE,0,0,	0,	&nprD6,	"sub :1"},
	{11,	CND_NONE,0,0,	0,	&nprD7,	"rst #10"},

	{5,	CND_C,6,0,	0,	&nprD8,	"ret c"},
	{4,	CND_NONE,0,0,	0,	&nprD9,	"exx"},
	{10,	CND_NONE,0,0,	0,	&nprDA,	"jp c,:2"},
	{11,	CND_NONE,0,0,	0,	&nprDB,	"in a,(:1)"},
	{10,	CND_C,7,0,	0,	&nprDC,	"call c,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprDD,	"#DD"},
	{7,	CND_NONE,0,0,	0,	&nprDE,	"sbc a,:1"},
	{11,	CND_NONE,0,0,	0,	&nprDF,	"rst #18"},

	{5,	CND_P,0,6,	0,	&nprE0,	"ret po"},
	{10,	CND_NONE,0,0,	0,	&ixpE1,	"pop ix"},
	{10,	CND_NONE,0,0,	0,	&nprE2,	"jp po,:2"},
	{19,	CND_NONE,0,0,	0,	&ixpE3,	"ex (sp),ix"},
	{10,	CND_P,0,7,	0,	&nprE4,	"call po,:2"},
	{11,	CND_NONE,0,0,	0,	&ixpE5,	"push ix"},
	{7,	CND_NONE,0,0,	0,	&nprE6,	"and :1"},
	{11,	CND_NONE,0,0,	0,	&nprE7,	"rst #20"},

	{5,	CND_P,6,0,	0,	&nprE8,	"ret pe"},
	{4,	CND_NONE,0,0,	0,	&ixpE9,	"jp (ix)"},
	{10,	CND_NONE,0,0,	0,	&nprEA,	"jp pe,:2"},
	{4,	CND_NONE,0,0,	0,	&nprEB,	"ex de,hl"},
	{10,	CND_P,7,0,	0,	&nprEC,	"call pe,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprED,	"#ED"},
	{7,	CND_NONE,0,0,	0,	&nprEE,	"xor :1"},
	{11,	CND_NONE,0,0,	0,	&nprEF,	"rst #28"},

	{5,	CND_S,0,6,	0,	&nprF0,	"ret p"},
	{10,	CND_NONE,0,0,	0,	&nprF1,	"pop af"},
	{10,	CND_NONE,0,0,	0,	&nprF2,	"jp p,:2"},
	{4,	CND_NONE,0,0,	0,	&nprF3,	"di"},
	{10,	CND_S,0,7,	0,	&nprF4,	"call p,:2"},
	{11,	CND_NONE,0,0,	0,	&nprF5,	"push af"},
	{7,	CND_NONE,0,0,	0,	&nprF6,	"xor :1"},
	{11,	CND_NONE,0,0,	0,	&nprF7,	"rst #30"},

	{5,	CND_S,6,0,	0,	&nprF8,	"ret m"},
	{6,	CND_NONE,0,0,	0,	&ixpF9,	"ld sp,ix"},
	{10,	CND_NONE,0,0,	0,	&nprFA,	"jp m,:2"},
	{4,	CND_NONE,0,0,	0,	&nprFB,	"ei"},
	{10,	CND_Z,7,0,	0,	&nprFC,	"call m,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprFD,	"#FD"},
	{7,	CND_NONE,0,0,	0,	&nprFE,	"cp :1"},
	{11,	CND_NONE,0,0,	0,	&nprFF,	"rst #38"}
};
