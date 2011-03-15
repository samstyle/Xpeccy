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
	ZOp(&npr00,4,"nop"),
	ZOp(&npr01,10,"ld bc,:2"),
	ZOp(&npr02,7,"ld (bc),a"),
	ZOp(&npr03,6,"inc bc"),
	ZOp(&npr04,4,"inc b"),
	ZOp(&npr05,4,"dec b"),
	ZOp(&npr06,7,"ld b,:1"),
	ZOp(&npr07,4,"rlca"),

	ZOp(&npr08,4,"ex af,af'"),
	ZOp(&iyp09,11,"add iy,bc"),
	ZOp(&npr0A,7,"ld a,(bc)"),
	ZOp(&npr0B,6,"dec bc"),
	ZOp(&npr0C,4,"inc c"),
	ZOp(&npr0D,4,"dec c"),
	ZOp(&npr0E,7,"ld c,:1"),
	ZOp(&npr0F,4,"rrca"),

	ZOp(&npr10,8,CND_DJNZ,5,0,"djnz :3",0),
	ZOp(&npr11,10,"ld de,:2"),
	ZOp(&npr12,7,"ld (de),a"),
	ZOp(&npr13,6,"inc de"),
	ZOp(&npr14,4,"inc d"),
	ZOp(&npr15,4,"dec d"),
	ZOp(&npr16,7,"ld d,:1"),
	ZOp(&npr17,4,"rla"),

	ZOp(&npr18,12,"jr :3"),
	ZOp(&iyp19,11,"add iy,de"),
	ZOp(&npr1A,7,"ld a,(de)"),
	ZOp(&npr1B,6,"dec de"),
	ZOp(&npr1C,4,"inc e"),
	ZOp(&npr1D,4,"dec e"),
	ZOp(&npr1E,7,"ld e,:1"),
	ZOp(&npr1F,4,"rra"),

	ZOp(&npr20,7,CND_Z,0,5,"jr nz,:3",0),
	ZOp(&iyp21,10,"ld iy,:2"),
	ZOp(&iyp22,16,"ld (:2),iy"),
	ZOp(&iyp23,6,"inc iy"),
	ZOp(&iyp24,4,"inc hy"),
	ZOp(&iyp25,4,"dec hy"),
	ZOp(&iyp26,7,"ld hy,:1"),
	ZOp(&npr27,4,"daa"),

	ZOp(&npr28,7,CND_Z,5,0,"jr z,:3",0),
	ZOp(&iyp29,11,"add iy,iy"),
	ZOp(&iyp2A,16,"ld iy,(:2)"),
	ZOp(&iyp2B,6,"dec iy"),
	ZOp(&iyp2C,4,"inc ly"),
	ZOp(&iyp2D,4,"dec ly"),
	ZOp(&iyp2E,7,"ld ly,:1"),
	ZOp(&npr2F,4,"cpl"),

	ZOp(&npr30,7,CND_C,0,5,"jr nc,:3",0),
	ZOp(&npr31,10,"ld sp,:2"),
	ZOp(&npr32,13,"ld (:2),a"),
	ZOp(&npr33,6,"inc sp"),
	ZOp(&iyp34,19,"inc (iy:4)"),
	ZOp(&iyp35,19,"dec (iy:4)"),
	ZOp(&iyp36,19,"ld (iy:4),:1"),
	ZOp(&npr37,4,"scf"),

	ZOp(&npr38,7,CND_C,5,0,"jr c,:3",0),
	ZOp(&iyp39,11,"add iy,sp"),
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
	ZOp(&iyp44,4,"ld b,hy"),
	ZOp(&iyp45,4,"ld b,ly"),
	ZOp(&iyp46,15,"ld b,(iy:4)"),
	ZOp(&npr47,4,"ld b,a"),

	ZOp(&npr48,4,"ld c,b"),
	ZOp(&npr49,4,"ld c,c"),
	ZOp(&npr4A,4,"ld c,d"),
	ZOp(&npr4B,4,"ld c,e"),
	ZOp(&iyp4C,4,"ld c,hy"),
	ZOp(&iyp4D,4,"ld c,ly"),
	ZOp(&iyp4E,15,"ld c,(iy:4)"),
	ZOp(&npr4F,4,"ld c,a"),

	ZOp(&npr50,4,"ld d,b"),
	ZOp(&npr51,4,"ld d,c"),
	ZOp(&npr52,4,"ld d,d"),
	ZOp(&npr53,4,"ld d,e"),
	ZOp(&iyp54,4,"ld d,hy"),
	ZOp(&iyp55,4,"ld d,ly"),
	ZOp(&iyp56,15,"ld d,(iy:4)"),
	ZOp(&npr57,4,"ld d,a"),

	ZOp(&npr58,4,"ld e,b"),
	ZOp(&npr59,4,"ld e,c"),
	ZOp(&npr5A,4,"ld e,d"),
	ZOp(&npr5B,4,"ld e,e"),
	ZOp(&iyp5C,4,"ld e,hy"),
	ZOp(&iyp5D,4,"ld e,ly"),
	ZOp(&iyp5E,15,"ld e,(iy:4)"),
	ZOp(&npr5F,4,"ld e,a"),

	ZOp(&iyp60,4,"ld hy,b"),
	ZOp(&iyp61,4,"ld hy,c"),
	ZOp(&iyp62,4,"ld hy,d"),
	ZOp(&iyp63,4,"ld hy,e"),
	ZOp(&iyp64,4,"ld hy,hy"),
	ZOp(&iyp65,4,"ld hy,ly"),
	ZOp(&iyp66,15,"ld h,(iy:4)"),
	ZOp(&iyp67,4,"ld hy,a"),

	ZOp(&iyp68,4,"ld ly,b"),
	ZOp(&iyp69,4,"ld ly,c"),
	ZOp(&iyp6A,4,"ld ly,d"),
	ZOp(&iyp6B,4,"ld ly,e"),
	ZOp(&iyp6C,4,"ld ly,hy"),
	ZOp(&iyp6D,4,"ld ly,ly"),
	ZOp(&iyp6E,15,"ld l,(iy:4)"),
	ZOp(&iyp6F,4,"ld ly,a"),

	ZOp(&iyp70,15,"ld (iy:4),b"),
	ZOp(&iyp71,15,"ld (iy:4),c"),
	ZOp(&iyp72,15,"ld (iy:4),d"),
	ZOp(&iyp73,15,"ld (iy:4),e"),
	ZOp(&iyp74,15,"ld (iy:4),h"),
	ZOp(&iyp75,15,"ld (iy:4),l"),
	ZOp(&npr76,4,"halt"),
	ZOp(&iyp77,15,"ld (iy:4),a"),

	ZOp(&npr78,4,"ld a,b"),
	ZOp(&npr79,4,"ld a,c"),
	ZOp(&npr7A,4,"ld a,d"),
	ZOp(&npr7B,4,"ld a,e"),
	ZOp(&iyp7C,4,"ld a,hy"),
	ZOp(&iyp7D,4,"ld a,ly"),
	ZOp(&iyp7E,15,"ld a,(iy:4)"),
	ZOp(&npr7F,4,"ld a,a"),

	ZOp(&npr80,4,"add a,b"),
	ZOp(&npr81,4,"add a,c"),
	ZOp(&npr82,4,"add a,d"),
	ZOp(&npr83,4,"add a,e"),
	ZOp(&iyp84,4,"add a,hy"),
	ZOp(&iyp85,4,"add a,ly"),
	ZOp(&iyp86,15,"add a,(iy:4)"),
	ZOp(&npr87,4,"add a,a"),

	ZOp(&npr88,4,"adc a,b"),
	ZOp(&npr89,4,"adc a,c"),
	ZOp(&npr8A,4,"adc a,d"),
	ZOp(&npr8B,4,"adc a,e"),
	ZOp(&iyp8C,4,"adc a,hy"),
	ZOp(&iyp8D,4,"adc a,ly"),
	ZOp(&iyp8E,15,"adc a,(iy:4)"),
	ZOp(&npr8F,4,"adc a,a"),

	ZOp(&npr90,4,"sub b"),
	ZOp(&npr91,4,"sub c"),
	ZOp(&npr92,4,"sub d"),
	ZOp(&npr93,4,"sub e"),
	ZOp(&iyp94,4,"sub hy"),
	ZOp(&iyp95,4,"sub ly"),
	ZOp(&iyp96,7,"sub (iy:4)"),
	ZOp(&npr97,4,"sub a"),

	ZOp(&npr98,4,"sbc a,b"),
	ZOp(&npr99,4,"sbc a,c"),
	ZOp(&npr9A,4,"sbc a,d"),
	ZOp(&npr9B,4,"sbc a,e"),
	ZOp(&iyp9C,4,"sbc a,hy"),
	ZOp(&iyp9D,4,"sbc a,ly"),
	ZOp(&iyp9E,15,"sbc a,(iy:4)"),
	ZOp(&npr9F,4,"sbc a,a"),

	ZOp(&nprA0,4,"and b"),
	ZOp(&nprA1,4,"and c"),
	ZOp(&nprA2,4,"and d"),
	ZOp(&nprA3,4,"and e"),
	ZOp(&iypA4,4,"and hy"),
	ZOp(&iypA5,4,"and ly"),
	ZOp(&iypA6,15,"and (iy:4)"),
	ZOp(&nprA7,4,"and a"),

	ZOp(&nprA8,4,"xor b"),
	ZOp(&nprA9,4,"xor c"),
	ZOp(&nprAA,4,"xor d"),
	ZOp(&nprAB,4,"xor e"),
	ZOp(&iypAC,4,"xor hy"),
	ZOp(&iypAD,4,"xor ly"),
	ZOp(&iypAE,15,"xor (iy:4)"),
	ZOp(&nprAF,4,"xor a"),

	ZOp(&nprB0,4,"or b"),
	ZOp(&nprB1,4,"or с"),
	ZOp(&nprB2,4,"or d"),
	ZOp(&nprB3,4,"or e"),
	ZOp(&iypB4,4,"or hy"),
	ZOp(&iypB5,4,"or ly"),
	ZOp(&iypB6,15,"or (iy:4)"),
	ZOp(&nprB7,4,"or a"),

	ZOp(&nprB8,4,"cp b"),
	ZOp(&nprB9,4,"cp c"),
	ZOp(&nprBA,4,"cp d"),
	ZOp(&nprBB,4,"cp e"),
	ZOp(&iypBC,4,"cp hy"),
	ZOp(&iypBD,4,"cp ly"),
	ZOp(&iypBE,15,"cp (iy:4)"),
	ZOp(&nprBF,4,"cp a"),

	ZOp(&nprC0,5,CND_Z,0,6,"ret nz",0),		// [+6]
	ZOp(&nprC1,10,"pop bc"),
	ZOp(&nprC2,10,"jp nz,:2"),
	ZOp(&nprC3,10,"jp :2"),
	ZOp(&nprC4,10,CND_Z,0,7,"call nz,:2",0),	// [+7]
	ZOp(&nprC5,11,"push bc"),
	ZOp(&nprC6,7,"add a,:1"),
	ZOp(&nprC7,11,"rst #00"),

	ZOp(&nprC8,5,CND_Z,6,0,"ret z",0),		// [+7]
	ZOp(&nprC9,10,"ret"),
	ZOp(&nprCA,10,"jp z,:2"),
	ZOp(&nprCB,4,CND_NONE,0,0,"#CB",ZOP_PREFIX),
	ZOp(&nprCC,10,"call z,:2"),
	ZOp(&nprCD,10,"call :2"),	// [+4 +3]: it acts like CALL true,nn
	ZOp(&nprCE,7,"adc a,:1"),
	ZOp(&nprCF,11,"rst #08"),

	ZOp(&nprD0,5,CND_C,0,6,"ret nc",0),		// [+6]
	ZOp(&nprD1,10,"pop de"),
	ZOp(&nprD2,10,"jp nc,:2"),
	ZOp(&nprD3,11,"out (:1),a"),
	ZOp(&nprD4,10,CND_C,0,7,"call nc,:2",0),	// [+7]
	ZOp(&nprD5,11,"push de"),
	ZOp(&nprD6,7,"sub :1"),
	ZOp(&nprD7,11,"rst #10"),

	ZOp(&nprD8,5,CND_C,6,0,"ret c",0),		// [+6]
	ZOp(&nprD9,4,"exx"),
	ZOp(&nprDA,10,"jp c,:2"),
	ZOp(&nprDB,11,"in a,(:1)"),
	ZOp(&nprDC,10,CND_C,7,0,"call c,:2",0),	// [+7]
	ZOp(&nprDD,4,CND_NONE,0,0,"#DD",ZOP_PREFIX),
	ZOp(&nprDE,7,"sbc a,:1"),
	ZOp(&nprDF,11,"rst #18"),

	ZOp(&nprE0,5,CND_P,0,6,"ret po",0),		// [+6]
	ZOp(&iypE1,10,"pop iy"),
	ZOp(&nprE2,10,"jp po,:2"),
	ZOp(&iypE3,19,"ex (sp),iy"),
	ZOp(&nprE4,10,CND_P,0,7,"call po,:2",0),	// [+7]
	ZOp(&iypE5,11,"push iy"),
	ZOp(&nprE6,7,"and :1"),
	ZOp(&nprE7,11,"rst #20"),

	ZOp(&nprE8,5,CND_P,6,0,"ret pe",0),		// [+6]
	ZOp(&iypE9,4,"jp (iy)"),
	ZOp(&nprEA,10,"jp pe,:2"),
	ZOp(&nprEB,4,"ex de,hl"),
	ZOp(&nprEC,10,CND_P,7,0,"call pe,:2",0),	// [+7]
	ZOp(&nprEB,4,CND_NONE,0,0,"#EB",ZOP_PREFIX),
	ZOp(&nprEE,7,"xor :1"),
	ZOp(&nprEF,11,"rst #28"),

	ZOp(&nprF0,5,CND_S,0,6,"ret p",0),		// [+6]
	ZOp(&nprF1,10,"pop af"),
	ZOp(&nprF2,10,"jp p,:2"),
	ZOp(&nprF3,4,"di"),
	ZOp(&nprF4,10,CND_S,0,7,"call p,:2",0),	// [+7]
	ZOp(&nprF5,11,"push af"),
	ZOp(&nprF6,7,"xor :1"),
	ZOp(&nprF7,11,"rst #30"),

	ZOp(&nprF8,5,CND_S,6,0,"ret m",0),		// [+6]
	ZOp(&iypF9,6,"ld sp,iy"),
	ZOp(&nprFA,10,"jp m,:2"),
	ZOp(&nprFB,4,"ei"),
	ZOp(&nprFC,10,CND_S,7,0,"call m,:2",0),	// [+7]
	ZOp(&nprFD,4,CND_NONE,0,0,"#FD",ZOP_PREFIX),
	ZOp(&nprFE,7,"cp :1"),
	ZOp(&nprFF,11,"rst #38")
};

