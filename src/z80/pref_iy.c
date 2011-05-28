// add iy,rp
void iyp09(ZXBase* p) {addIY(p,p->cpu->bc);}
void iyp19(ZXBase* p) {addIY(p,p->cpu->de);}
void iyp29(ZXBase* p) {addIY(p,p->cpu->iy);}
void iyp39(ZXBase* p) {addIY(p,p->cpu->sp);}
// inc/dec/ld iy
void iyp23(ZXBase* p) {p->cpu->iy++;}
void iyp2B(ZXBase* p) {p->cpu->iy--;}
void iyp21(ZXBase* p) {p->cpu->ly = p->mem->rd(p->cpu->pc++); p->cpu->hy = p->mem->rd(p->cpu->pc++);}
// ld (nn),iy | ld iy,(nn)	mptr = nn + 1
void iyp22(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->ly); p->mem->wr(p->cpu->mptr,p->cpu->hy);}
void iyp2A(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->ly = p->mem->rd(p->cpu->mptr++); p->cpu->hy = p->mem->rd(p->cpu->mptr);}
// inc/dec/ld ly
void iyp2C(ZXBase* p) {p->cpu->ly++; p->cpu->f = flag[p->cpu->ly].inc | (p->cpu->f & FC);}
void iyp2D(ZXBase* p) {p->cpu->ly--; p->cpu->f = flag[p->cpu->ly].dec | (p->cpu->f & FC);}
void iyp2E(ZXBase* p) {p->cpu->ly = p->mem->rd(p->cpu->pc++);}
// inc/dec/ld hy
void iyp24(ZXBase* p) {p->cpu->hy++; p->cpu->f = flag[p->cpu->hy].inc | (p->cpu->f & FC);}
void iyp25(ZXBase* p) {p->cpu->hy--; p->cpu->f = flag[p->cpu->hy].dec | (p->cpu->f & FC);}
void iyp26(ZXBase* p) {p->cpu->hy = p->mem->rd(p->cpu->pc++);}
// inc/dec/ld (iy+e)
void iyp34(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr) + 1; p->mem->wr(p->cpu->mptr,p->cpu->x); p->cpu->f = flag[p->cpu->x].inc | (p->cpu->f & FC);}
void iyp35(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr) - 1; p->mem->wr(p->cpu->mptr,p->cpu->x); p->cpu->f = flag[p->cpu->x].dec | (p->cpu->f & FC);}
void iyp36(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->mem->rd(p->cpu->pc++));}
// ld b,y
void iyp44(ZXBase* p) {p->cpu->b = p->cpu->hy;}
void iyp45(ZXBase* p) {p->cpu->b = p->cpu->ly;}
void iyp46(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->b = p->mem->rd(p->cpu->mptr);}
// ld c,y
void iyp4C(ZXBase* p) {p->cpu->c = p->cpu->hy;}
void iyp4D(ZXBase* p) {p->cpu->c = p->cpu->ly;}
void iyp4E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->c = p->mem->rd(p->cpu->mptr);}
// ld d,y
void iyp54(ZXBase* p) {p->cpu->d = p->cpu->hy;}
void iyp55(ZXBase* p) {p->cpu->d = p->cpu->ly;}
void iyp56(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->d = p->mem->rd(p->cpu->mptr);}
// ld e,y
void iyp5C(ZXBase* p) {p->cpu->e = p->cpu->hy;}
void iyp5D(ZXBase* p) {p->cpu->e = p->cpu->ly;}
void iyp5E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->e = p->mem->rd(p->cpu->mptr);}
// ld hy,r
void iyp60(ZXBase* p) {p->cpu->hy = p->cpu->b;}
void iyp61(ZXBase* p) {p->cpu->hy = p->cpu->c;}
void iyp62(ZXBase* p) {p->cpu->hy = p->cpu->d;}
void iyp63(ZXBase* p) {p->cpu->hy = p->cpu->e;}
void iyp64(ZXBase*) {}
void iyp65(ZXBase* p) {p->cpu->hy = p->cpu->ly;}
void iyp66(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->h = p->mem->rd(p->cpu->mptr);}
void iyp67(ZXBase* p) {p->cpu->hy = p->cpu->a;}
// ld ly,r
void iyp68(ZXBase* p) {p->cpu->ly = p->cpu->b;}
void iyp69(ZXBase* p) {p->cpu->ly = p->cpu->c;}
void iyp6A(ZXBase* p) {p->cpu->ly = p->cpu->d;}
void iyp6B(ZXBase* p) {p->cpu->ly = p->cpu->e;}
void iyp6C(ZXBase* p) {p->cpu->ly = p->cpu->hy;}
void iyp6D(ZXBase*) {}
void iyp6E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->l = p->mem->rd(p->cpu->mptr);}
void iyp6F(ZXBase* p) {p->cpu->ly = p->cpu->a;}
// ld (iy+e),r
void iyp70(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->b);}
void iyp71(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->c);}
void iyp72(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->d);}
void iyp73(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->e);}
void iyp74(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->h);}
void iyp75(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->l);}
void iyp77(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr,p->cpu->a);}
// ld a,y
void iyp7C(ZXBase* p) {p->cpu->a = p->cpu->hy;}
void iyp7D(ZXBase* p) {p->cpu->a = p->cpu->ly;}
void iyp7E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a = p->mem->rd(p->cpu->mptr);}
// add y
void iyp84(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->hy][0]; p->cpu->a += p->cpu->hy;}
void iyp85(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->ly][0]; p->cpu->a += p->cpu->ly;}
void iyp86(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].add[p->cpu->x][0]; p->cpu->a += p->cpu->x;}
// adc y
void iyp8C(ZXBase* p) {adcXX(p,p->cpu->hy);}
void iyp8D(ZXBase* p) {adcXX(p,p->cpu->ly);}
void iyp8E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); adcXX(p,p->mem->rd(p->cpu->mptr));}
// sub y
void iyp94(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->hy][0]; p->cpu->a -= p->cpu->hy;}
void iyp95(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->ly][0]; p->cpu->a -= p->cpu->ly;}
void iyp96(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].sub[p->cpu->x][0]; p->cpu->a -= p->cpu->x;}
// sbc y
void iyp9C(ZXBase* p) {sbcXX(p,p->cpu->hy);}
void iyp9D(ZXBase* p) {sbcXX(p,p->cpu->ly);}
void iyp9E(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); sbcXX(p,p->mem->rd(p->cpu->mptr));}
// and y
void iypA4(ZXBase* p) {p->cpu->a &= p->cpu->hy; p->cpu->f = flag[p->cpu->a].andf;}
void iypA5(ZXBase* p) {p->cpu->a &= p->cpu->ly; p->cpu->f = flag[p->cpu->a].andf;}
void iypA6(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a &= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].andf;}
// xor y
void iypAC(ZXBase* p) {p->cpu->a ^= p->cpu->hy; p->cpu->f = flag[p->cpu->a].orf;}
void iypAD(ZXBase* p) {p->cpu->a ^= p->cpu->ly; p->cpu->f = flag[p->cpu->a].orf;}
void iypAE(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a ^= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].orf;}
// or y
void iypB4(ZXBase* p) {p->cpu->a |= p->cpu->hy; p->cpu->f = flag[p->cpu->a].orf;}
void iypB5(ZXBase* p) {p->cpu->a |= p->cpu->ly; p->cpu->f = flag[p->cpu->a].orf;}
void iypB6(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->a |= p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->a].orf;}
// cp y
void iypBC(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->hy][0];}
void iypBD(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->ly][0];}
void iypBE(ZXBase* p) {p->cpu->mptr = p->cpu->iy + (signed char)p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].sub[p->mem->rd(p->cpu->mptr)][0];}
// end
void iypE1(ZXBase* p) {p->cpu->ly = p->mem->rd(p->cpu->sp++); p->cpu->hy = p->mem->rd(p->cpu->sp++);}
void iypE5(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hy); p->mem->wr(--p->cpu->sp,p->cpu->ly);}
void iypE3(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->sp++); p->cpu->hptr = p->mem->rd(p->cpu->sp); p->mem->wr(p->cpu->sp--,p->cpu->hy); p->mem->wr(p->cpu->sp,p->cpu->ly); p->cpu->iy = p->cpu->mptr;}	// mptr = rp after operation
void iypE9(ZXBase* p) {p->cpu->pc = p->cpu->iy;}
void iypF9(ZXBase* p) {p->cpu->sp = p->cpu->iy;}

//==================

ZOp iypref[256]={
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop"},
	{10,	CND_NONE,0,0,	0,	&npr01,	"ld bc,:2"},
	{7,	CND_NONE,0,0,	0,	&npr02,	"ld (bc),a"},
	{6,	CND_NONE,0,0,	0,	&npr03,	"inc bc"},
	{4,	CND_NONE,0,0,	0,	&npr04,	"inc b"},
	{4,	CND_NONE,0,0,	0,	&npr05,	"dec b"},
	{7,	CND_NONE,0,0,	0,	&npr06,	"ld b,:1"},
	{4,	CND_NONE,0,0,	0,	&npr07,	"rlca"},

	{4,	CND_NONE,0,0,	0,	&npr08,	"ex af,af'"},
	{11,	CND_NONE,0,0,	0,	&iyp09,	"add iy,bc"},
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
	{11,	CND_NONE,0,0,	0,	&iyp19,	"add iy,de"},
	{7,	CND_NONE,0,0,	0,	&npr1A,	"ld a,(de)"},
	{6,	CND_NONE,0,0,	0,	&npr1B,	"dec de"},
	{4,	CND_NONE,0,0,	0,	&npr1C,	"inc e"},
	{4,	CND_NONE,0,0,	0,	&npr1D,	"dec e"},
	{7,	CND_NONE,0,0,	0,	&npr1E,	"ld e,:1"},
	{4,	CND_NONE,0,0,	0,	&npr1F,	"rra"},

	{7,	CND_Z,0,5,	0,	&npr20,	"jr nz,:3"},
	{10,	CND_NONE,0,0,	0,	&iyp21,	"ld iy,:2"},
	{16,	CND_NONE,0,0,	0,	&iyp22,	"ld (:2),iy"},
	{6,	CND_NONE,0,0,	0,	&iyp23,	"inc iy"},
	{4,	CND_NONE,0,0,	0,	&iyp24,	"inc hy"},
	{4,	CND_NONE,0,0,	0,	&iyp25,	"dec hy"},
	{7,	CND_NONE,0,0,	0,	&iyp26,	"ld hy,:1"},
	{4,	CND_NONE,0,0,	0,	&npr27,	"daa"},

	{7,	CND_Z,5,0,	0,	&npr28,	"jr z,:3"},
	{11,	CND_NONE,0,0,	0,	&iyp29,	"add iy,iy"},
	{16,	CND_NONE,0,0,	0,	&iyp2A,	"ld iy,(:2)"},
	{6,	CND_NONE,0,0,	0,	&iyp2B,	"dec iy"},
	{4,	CND_NONE,0,0,	0,	&iyp2C,	"inc ly"},
	{4,	CND_NONE,0,0,	0,	&iyp2D,	"dec ly"},
	{7,	CND_NONE,0,0,	0,	&iyp2E,	"ld ly,:1"},
	{4,	CND_NONE,0,0,	0,	&npr2F,	"cpl"},

	{7,	CND_C,0,5,	0,	&npr30,	"jr nc,:3"},
	{10,	CND_NONE,0,0,	0,	&npr31,	"ld sp,:2"},
	{13,	CND_NONE,0,0,	0,	&npr32,	"ld (:2),a"},
	{6,	CND_NONE,0,0,	0,	&npr33,	"inc sp"},
	{19,	CND_NONE,0,0,	0,	&iyp34,	"inc (iy:4)"},
	{19,	CND_NONE,0,0,	0,	&iyp35,	"dec (iy:4)"},
	{19,	CND_NONE,0,0,	0,	&iyp36,	"ld (iy:4),:1"},
	{4,	CND_NONE,0,0,	0,	&npr37,	"scf"},

	{7,	CND_C,5,0,	0,	&npr38,	"jr c,:3"},
	{11,	CND_NONE,0,0,	0,	&iyp39,	"add iy,sp"},
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
	{4,	CND_NONE,0,0,	0,	&iyp44,	"ld b,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp45,	"ld b,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp46,	"ld b,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr47,	"ld b,a"},

	{4,	CND_NONE,0,0,	0,	&npr48,	"ld c,b"},
	{4,	CND_NONE,0,0,	0,	&npr49,	"ld c,c"},
	{4,	CND_NONE,0,0,	0,	&npr4A,	"ld c,d"},
	{4,	CND_NONE,0,0,	0,	&npr4B,	"ld c,e"},
	{4,	CND_NONE,0,0,	0,	&iyp4C,	"ld c,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp4D,	"ld c,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp4E,	"ld c,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr4F,	"ld c,a"},

	{4,	CND_NONE,0,0,	0,	&npr50,	"ld d,b"},
	{4,	CND_NONE,0,0,	0,	&npr51,	"ld d,c"},
	{4,	CND_NONE,0,0,	0,	&npr52,	"ld d,d"},
	{4,	CND_NONE,0,0,	0,	&npr53,	"ld d,e"},
	{4,	CND_NONE,0,0,	0,	&iyp54,	"ld d,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp55,	"ld d,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp56,	"ld d,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr57,	"ld d,a"},

	{4,	CND_NONE,0,0,	0,	&npr58,	"ld e,b"},
	{4,	CND_NONE,0,0,	0,	&npr59,	"ld e,c"},
	{4,	CND_NONE,0,0,	0,	&npr5A,	"ld e,d"},
	{4,	CND_NONE,0,0,	0,	&npr5B,	"ld e,e"},
	{4,	CND_NONE,0,0,	0,	&iyp5C,	"ld e,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp5D,	"ld e,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp5E,	"ld e,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr5F,	"ld e,a"},

	{4,	CND_NONE,0,0,	0,	&iyp60,	"ld hy,b"},
	{4,	CND_NONE,0,0,	0,	&iyp61,	"ld hy,c"},
	{4,	CND_NONE,0,0,	0,	&iyp62,	"ld hy,d"},
	{4,	CND_NONE,0,0,	0,	&iyp63,	"ld hy,e"},
	{4,	CND_NONE,0,0,	0,	&iyp64,	"ld hy,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp65,	"ld hy,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp66,	"ld h,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&iyp67,	"ld hy,a"},

	{4,	CND_NONE,0,0,	0,	&iyp68,	"ld ly,b"},
	{4,	CND_NONE,0,0,	0,	&iyp69,	"ld ly,c"},
	{4,	CND_NONE,0,0,	0,	&iyp6A,	"ld ly,d"},
	{4,	CND_NONE,0,0,	0,	&iyp6B,	"ld ly,e"},
	{4,	CND_NONE,0,0,	0,	&iyp6C,	"ld ly,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp6D,	"ld ly,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp6E,	"ld l,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&iyp6F,	"ld ly,a"},

	{15,	CND_NONE,0,0,	0,	&iyp70,	"ld (iy:4),b"},
	{15,	CND_NONE,0,0,	0,	&iyp71,	"ld (iy:4),c"},
	{15,	CND_NONE,0,0,	0,	&iyp72,	"ld (iy:4),d"},
	{15,	CND_NONE,0,0,	0,	&iyp73,	"ld (iy:4),e"},
	{15,	CND_NONE,0,0,	0,	&iyp74,	"ld (iy:4),h"},
	{15,	CND_NONE,0,0,	0,	&iyp75,	"ld (iy:4),l"},
	{4,	CND_NONE,0,0,	0,	&npr76,	"halt"},
	{15,	CND_NONE,0,0,	0,	&iyp77,	"ld (iy:4),a"},

	{4,	CND_NONE,0,0,	0,	&npr78,	"ld a,b"},
	{4,	CND_NONE,0,0,	0,	&npr79,	"ld a,c"},
	{4,	CND_NONE,0,0,	0,	&npr7A,	"ld a,d"},
	{4,	CND_NONE,0,0,	0,	&npr7B,	"ld a,e"},
	{4,	CND_NONE,0,0,	0,	&iyp7C,	"ld a,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp7D,	"ld a,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp7E,	"ld a,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr7F,	"ld a,a"},

	{4,	CND_NONE,0,0,	0,	&npr80,	"add a,b"},
	{4,	CND_NONE,0,0,	0,	&npr81,	"add a,c"},
	{4,	CND_NONE,0,0,	0,	&npr82,	"add a,d"},
	{4,	CND_NONE,0,0,	0,	&npr83,	"add a,e"},
	{4,	CND_NONE,0,0,	0,	&iyp84,	"add a,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp85,	"add a,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp86,	"add a,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr87,	"add a,a"},

	{4,	CND_NONE,0,0,	0,	&npr88,	"adc a,b"},
	{4,	CND_NONE,0,0,	0,	&npr89,	"adc a,c"},
	{4,	CND_NONE,0,0,	0,	&npr8A,	"adc a,d"},
	{4,	CND_NONE,0,0,	0,	&npr8B,	"adc a,e"},
	{4,	CND_NONE,0,0,	0,	&iyp8C,	"adc a,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp8D,	"adc a,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp8E,	"adc a,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr8F,	"adc a,a"},

	{4,	CND_NONE,0,0,	0,	&npr90,	"sub b"},
	{4,	CND_NONE,0,0,	0,	&npr91,	"sub c"},
	{4,	CND_NONE,0,0,	0,	&npr92,	"sub d"},
	{4,	CND_NONE,0,0,	0,	&npr93,	"sub e"},
	{4,	CND_NONE,0,0,	0,	&iyp94,	"sub hy"},
	{4,	CND_NONE,0,0,	0,	&iyp95,	"sub ly"},
	{15,	CND_NONE,0,0,	0,	&iyp96,	"sub (iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr97,	"sub a"},

	{4,	CND_NONE,0,0,	0,	&npr98,	"sbc a,b"},
	{4,	CND_NONE,0,0,	0,	&npr99,	"sbc a,c"},
	{4,	CND_NONE,0,0,	0,	&npr9A,	"sbc a,d"},
	{4,	CND_NONE,0,0,	0,	&npr9B,	"sbc a,e"},
	{4,	CND_NONE,0,0,	0,	&iyp9C,	"sbc a,hy"},
	{4,	CND_NONE,0,0,	0,	&iyp9D,	"sbc a,ly"},
	{15,	CND_NONE,0,0,	0,	&iyp9E,	"sbc a,(iy:4)"},
	{4,	CND_NONE,0,0,	0,	&npr9F,	"sbc a,a"},

	{4,	CND_NONE,0,0,	0,	&nprA0,	"and b"},
	{4,	CND_NONE,0,0,	0,	&nprA1,	"and c"},
	{4,	CND_NONE,0,0,	0,	&nprA2,	"and d"},
	{4,	CND_NONE,0,0,	0,	&nprA3,	"and e"},
	{4,	CND_NONE,0,0,	0,	&iypA4,	"and hy"},
	{4,	CND_NONE,0,0,	0,	&iypA5,	"and ly"},
	{15,	CND_NONE,0,0,	0,	&iypA6,	"and (iy:4)"},
	{4,	CND_NONE,0,0,	0,	&nprA7,	"and a"},

	{4,	CND_NONE,0,0,	0,	&nprA8,	"xor b"},
	{4,	CND_NONE,0,0,	0,	&nprA9,	"xor c"},
	{4,	CND_NONE,0,0,	0,	&nprAA,	"xor d"},
	{4,	CND_NONE,0,0,	0,	&nprAB,	"xor e"},
	{4,	CND_NONE,0,0,	0,	&iypAC,	"xor hy"},
	{4,	CND_NONE,0,0,	0,	&iypAD,	"xor ly"},
	{15,	CND_NONE,0,0,	0,	&iypAE,	"xor (iy:4)"},
	{4,	CND_NONE,0,0,	0,	&nprAF,	"xor a"},

	{4,	CND_NONE,0,0,	0,	&nprB0,	"or b"},
	{4,	CND_NONE,0,0,	0,	&nprB1,	"or c"},
	{4,	CND_NONE,0,0,	0,	&nprB2,	"or d"},
	{4,	CND_NONE,0,0,	0,	&nprB3,	"or e"},
	{4,	CND_NONE,0,0,	0,	&iypB4,	"or hy"},
	{4,	CND_NONE,0,0,	0,	&iypB5,	"or ly"},
	{15,	CND_NONE,0,0,	0,	&iypB6,	"or (iy:4)"},
	{4,	CND_NONE,0,0,	0,	&nprB7,	"or a"},

	{4,	CND_NONE,0,0,	0,	&nprB8,	"cp b"},
	{4,	CND_NONE,0,0,	0,	&nprB9,	"cp c"},
	{4,	CND_NONE,0,0,	0,	&nprBA,	"cp d"},
	{4,	CND_NONE,0,0,	0,	&nprBB,	"cp e"},
	{4,	CND_NONE,0,0,	0,	&iypBC,	"cp hy"},
	{4,	CND_NONE,0,0,	0,	&iypBD,	"cp ly"},
	{15,	CND_NONE,0,0,	0,	&iypBE,	"cp (iy:4)"},
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
	{10,	CND_NONE,0,0,	0,	&iypE1,	"pop iy"},
	{10,	CND_NONE,0,0,	0,	&nprE2,	"jp po,:2"},
	{19,	CND_NONE,0,0,	0,	&iypE3,	"ex (sp),iy"},
	{10,	CND_P,0,7,	0,	&nprE4,	"call po,:2"},
	{11,	CND_NONE,0,0,	0,	&iypE5,	"push iy"},
	{7,	CND_NONE,0,0,	0,	&nprE6,	"and :1"},
	{11,	CND_NONE,0,0,	0,	&nprE7,	"rst #20"},

	{5,	CND_P,6,0,	0,	&nprE8,	"ret pe"},
	{4,	CND_NONE,0,0,	0,	&iypE9,	"jp (iy)"},
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
	{6,	CND_NONE,0,0,	0,	&iypF9,	"ld sp,iy"},
	{10,	CND_NONE,0,0,	0,	&nprFA,	"jp m,:2"},
	{4,	CND_NONE,0,0,	0,	&nprFB,	"ei"},
	{10,	CND_Z,7,0,	0,	&nprFC,	"call m,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprFD,	"#FD"},
	{7,	CND_NONE,0,0,	0,	&nprFE,	"cp :1"},
	{11,	CND_NONE,0,0,	0,	&nprFF,	"rst #38"}
};
