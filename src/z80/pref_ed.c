// NOTE:ticks are NOT include ED prefix fetch. (-4)

// in
void edp40(Spec *p) {p->cpu->b = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->b].in | (p->cpu->f & FC);}
void edp48(Spec *p) {p->cpu->c = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->c].in | (p->cpu->f & FC);}
void edp50(Spec *p) {p->cpu->d = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->d].in | (p->cpu->f & FC);}
void edp58(Spec *p) {p->cpu->e = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->e].in | (p->cpu->f & FC);}
void edp60(Spec *p) {p->cpu->h = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->h].in | (p->cpu->f & FC);}
void edp68(Spec *p) {p->cpu->l = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->l].in | (p->cpu->f & FC);}
void edp70(Spec *p) {p->cpu->x = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->x].in | (p->cpu->f & FC);}
void edp78(Spec *p) {p->cpu->a = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->a].in | (p->cpu->f & FC); p->cpu->mptr = p->cpu->bc + 1;}	// in a,(c)	mptr = bc + 1
// out
void edp41(Spec *p) {p->io->out(p->cpu->bc,p->cpu->b);}
void edp49(Spec *p) {p->io->out(p->cpu->bc,p->cpu->c);}
void edp51(Spec *p) {p->io->out(p->cpu->bc,p->cpu->d);}
void edp59(Spec *p) {p->io->out(p->cpu->bc,p->cpu->e);}
void edp61(Spec *p) {p->io->out(p->cpu->bc,p->cpu->h);}
void edp69(Spec *p) {p->io->out(p->cpu->bc,p->cpu->l);}
void edp71(Spec *p) {p->io->out(p->cpu->bc,0);}
void edp79(Spec *p) {p->io->out(p->cpu->bc,p->cpu->a); p->cpu->mptr = p->cpu->bc + 1;}	// out (c),a	mptr = bc + 1
// sbc hl,rp
void edp42(Spec *p) {rpSBC(p,p->cpu->bc);}
void edp52(Spec *p) {rpSBC(p,p->cpu->de);}
void edp62(Spec *p) {rpSBC(p,p->cpu->hl);}
void edp72(Spec *p) {rpSBC(p,p->cpu->sp);}
// adc hl,rp
void edp4A(Spec *p) {rpADC(p,p->cpu->bc);}
void edp5A(Spec *p) {rpADC(p,p->cpu->de);}
void edp6A(Spec *p) {rpADC(p,p->cpu->hl);}
void edp7A(Spec *p) {rpADC(p,p->cpu->sp);}
// ld (nn).rp	mptr = nn + 1
void edp43(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->c); p->mem->wr(p->cpu->mptr,p->cpu->b);}
void edp53(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->e); p->mem->wr(p->cpu->mptr,p->cpu->d);}
void edp63(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->l); p->mem->wr(p->cpu->mptr,p->cpu->h);}
void edp73(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->lsp); p->mem->wr(p->cpu->mptr,p->cpu->hsp);}
// ld rp,(nn)	mptr = nn + 1
void edp4B(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->c = p->mem->rd(p->cpu->mptr++); p->cpu->b = p->mem->rd(p->cpu->mptr);}
void edp5B(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->e = p->mem->rd(p->cpu->mptr++); p->cpu->d = p->mem->rd(p->cpu->mptr);}
void edp6B(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->l = p->mem->rd(p->cpu->mptr++); p->cpu->h = p->mem->rd(p->cpu->mptr);}
void edp7B(Spec *p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->lsp = p->mem->rd(p->cpu->mptr++); p->cpu->hsp = p->mem->rd(p->cpu->mptr);}
// neg
void edp44(Spec *p) {p->cpu->a = -p->cpu->a; p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:FC) | ((p->cpu->a & 15)?FH:0) | ((p->cpu->a == 0x80)?FP:0) | FN;}
void edp4C(Spec *p) {edp44(p);}
void edp54(Spec *p) {edp44(p);}
void edp5C(Spec *p) {edp44(p);}
void edp64(Spec *p) {edp44(p);}
void edp6C(Spec *p) {edp44(p);}
void edp74(Spec *p) {edp44(p);}
void edp7C(Spec *p) {edp44(p);}
// retn / reti		mptr = ret.adr
void edp45(Spec *p) {p->cpu->iff1 = p->cpu->iff2; p->cpu->lpc = p->mem->rd(p->cpu->sp++); p->cpu->hpc = p->mem->rd(p->cpu->sp++); p->cpu->mptr = p->cpu->pc;}		// retn
void edp55(Spec *p) {edp45(p);}
void edp65(Spec *p) {edp45(p);}
void edp75(Spec *p) {edp45(p);}
void edp4D(Spec *p) {p->cpu->lpc = p->mem->rd(p->cpu->sp++); p->cpu->hpc = p->mem->rd(p->cpu->sp++); p->cpu->mptr = p->cpu->pc;}				// reti
void edp5D(Spec *p) {edp4D(p);}
void edp6D(Spec *p) {edp4D(p);}
void edp7D(Spec *p) {edp4D(p);}
// im
void edp46(Spec *p) {p->cpu->imode = 0;}
void edp4E(Spec *p) {edp46(p);}
void edp66(Spec *p) {edp46(p);}
void edp6E(Spec *p) {edp46(p);}
void edp56(Spec *p) {p->cpu->imode = 1;}
void edp76(Spec *p) {edp56(p);}
void edp5E(Spec *p) {p->cpu->imode = 2;}
void edp7E(Spec *p) {edp5E(p);}
// ld [i,r]
void edp47(Spec *p) {p->cpu->i = p->cpu->a;}
void edp4F(Spec *p) {p->cpu->r = p->cpu->a;}
void edp57(Spec *p) {p->cpu->a = p->cpu->i; p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a & 0xff)?0:FZ) | (p->cpu->iff2 ? FP:0) | (p->cpu->f & FC);}
void edp5F(Spec *p) {p->cpu->a = p->cpu->r; p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a & 0xff)?0:FZ) | (p->cpu->iff2 ? FP:0) | (p->cpu->f & FC);}
// rrd / rld	mptr = hl + 1
void edp67(Spec *p) {p->cpu->mptr = p->cpu->hl + 1;
		p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->dlt = p->cpu->x & 0x0f;
		p->cpu->x = ((p->cpu->x & 0xf0) >> 4) | ((p->cpu->a & 0x0f) << 4); p->cpu->a = (p->cpu->a & 0xf0) | p->cpu->dlt; p->mem->wr(p->cpu->hl,p->cpu->x);
		p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:0) | (parity(p->cpu->a)?FP:0) | (p->cpu->f & FC);
}
void edp6F(Spec *p) {p->cpu->mptr = p->cpu->hl + 1;
		p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->dlt = p->cpu->x & 0xf0;
		p->cpu->x = ((p->cpu->x & 0x0f) << 4) | (p->cpu->a & 0x0f); p->cpu->a = (p->cpu->a & 0xf0) | (p->cpu->dlt >> 4); p->mem->wr(p->cpu->hl,p->cpu->x);
		p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:0) | (parity(p->cpu->a)?FP:0) | (p->cpu->f & FC);
}
// [blk]i
void edpA0(Spec *p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl++); p->mem->wr(p->cpu->de++,p->cpu->x); p->cpu->x += p->cpu->a;
		p->cpu->f = (p->cpu->f & (FS | FZ | FC)) | (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0) | ((p->cpu->bc != 0)?FP:0);}
void edpA1(Spec *p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl++); p->cpu->f = (p->cpu->f & FC) | (flag[p->cpu->a].cp[p->cpu->x] & (FS | FZ | FH | FN)) | ((p->cpu->bc != 0)?FP:0);
		p->cpu->x = p->cpu->a - p->cpu->x - ((p->cpu->f & FH)?1:0); p->cpu->f |= (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0); p->cpu->mptr++;}		// cpi	mptr++
void edpA2(Spec *p) {p->cpu->mptr = p->cpu->bc + 1; p->mem->wr(p->cpu->hl++,p->io->in(p->cpu->bc)); p->cpu->b--; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// ini	mptr = bc.before + 1
void edpA3(Spec *p) {p->io->out(p->cpu->bc,p->mem->rd(p->cpu->hl++)); p->cpu->b--; p->cpu->mptr = p->cpu->bc + 1; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// outi	mptr = bc.after + 1
// [blk]d
void edpA8(Spec *p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl--); p->mem->wr(p->cpu->de--,p->cpu->x); p->cpu->x += p->cpu->a;
		p->cpu->f = (p->cpu->f & (FS | FZ | FC)) | (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0) | ((p->cpu->bc != 0)?FP:0);}
void edpA9(Spec *p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl--); p->cpu->f = (p->cpu->f & FC) | (flag[p->cpu->a].cp[p->cpu->x] & (FS | FZ | FH | FN)) | ((p->cpu->bc != 0)?FP:0);
		p->cpu->x = p->cpu->a - p->cpu->x - ((p->cpu->f & FH)?1:0); p->cpu->f |= (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0); p->cpu->mptr--;}		// cpd: mptr--
void edpAA(Spec *p) {p->cpu->mptr = p->cpu->bc - 1; p->mem->wr(p->cpu->hl--,p->io->in(p->cpu->bc)); p->cpu->b--; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// ind	mptr = bc.before - 1
void edpAB(Spec *p) {p->io->out(p->cpu->bc,p->mem->rd(p->cpu->hl--)); p->cpu->b--; p->cpu->mptr = p->cpu->bc - 1; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// outd	mptr = bc.after - 1
// [blk]ir
void edpB0(Spec *p) {edpA0(p); if (p->cpu->f & FP) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; p->cpu->t += 5;}}			// ldir: if (not over) mptr = instr.adr + 1
void edpB1(Spec *p) {edpA1(p); if ((!(p->cpu->f & FZ)) && (p->cpu->f & FP)) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; p->cpu->t += 5;}}	// cpir: if (not over) mptr = instr.adr + 1
void edpB2(Spec *p) {edpA2(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; p->cpu->t += 5;}}
void edpB3(Spec *p) {edpA3(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; p->cpu->t += 5;}}
// [blk]dr
void edpB8(Spec *p) {edpA8(p); if (p->cpu->f & FP) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; p->cpu->t += 5;}}			// lddr: if (not over) mptr = instr.adr + 1
void edpB9(Spec *p) {edpA9(p); if ((!(p->cpu->f & FZ)) && (p->cpu->f & FP)) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; p->cpu->t += 5;}}	// cpdr: if (not over) mptr = instr.adr + 1
void edpBA(Spec *p) {edpAA(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; p->cpu->t += 5;}}
void edpBB(Spec *p) {edpAB(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; p->cpu->t += 5;}}

ZOp edpref[256]={
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&edp40,8,"in b,(c)"),
	ZOp(&edp41,8,"out (c),b"),
	ZOp(&edp42,11,"sbc hl,bc"),
	ZOp(&edp43,16,"ld (:2),bc"),
	ZOp(&edp44,4,"neg"),
	ZOp(&edp45,11,"retn"),
	ZOp(&edp46,4,"im 0"),
	ZOp(&edp47,5,"ld i,a"),

	ZOp(&edp48,8,"in c,(c)"),
	ZOp(&edp49,8,"out (c),c"),
	ZOp(&edp4A,11,"adc hl,bc"),
	ZOp(&edp4B,16,"ld bc,(:2)"),
	ZOp(&edp4C,4,"neg *"),
	ZOp(&edp4D,11,"reti"),
	ZOp(&edp4E,4,"im 0 *"),
	ZOp(&edp4F,5,"ld r,a"),

	ZOp(&edp50,8,"in d,(c)"),
	ZOp(&edp51,8,"out (c),d"),
	ZOp(&edp52,11,"sbc hl,de"),
	ZOp(&edp53,16,"ld (:2),de"),
	ZOp(&edp54,4,"neg *"),
	ZOp(&edp55,11,"retn *"),
	ZOp(&edp56,4,"im 1"),
	ZOp(&edp57,5,"ld a,i"),

	ZOp(&edp58,8,"in e,(c)"),
	ZOp(&edp59,8,"out (c),e"),
	ZOp(&edp5A,11,"adc hl,de"),
	ZOp(&edp5B,16,"ld de,(:2)"),
	ZOp(&edp5C,4,"neg *"),
	ZOp(&edp5D,11,"reti *"),
	ZOp(&edp5E,4,"im 2"),
	ZOp(&edp5F,5,"ld a,r"),

	ZOp(&edp60,8,"in h,(c)"),
	ZOp(&edp61,8,"out (c),h"),
	ZOp(&edp62,11,"sbc hl,hl"),
	ZOp(&edp63,16,"ld (:2),hl"),
	ZOp(&edp64,4,"neg *"),
	ZOp(&edp65,11,"retn *"),
	ZOp(&edp66,4,"im 0 *"),
	ZOp(&edp67,14,"rrd"),

	ZOp(&edp68,8,"in l,(c)"),
	ZOp(&edp69,8,"out (c),l"),
	ZOp(&edp6A,11,"adc hl,hl"),
	ZOp(&edp6B,16,"ld hl,(:2)"),
	ZOp(&edp6C,4,"neg *"),
	ZOp(&edp6D,11,"reti *"),
	ZOp(&edp6E,4,"im 0 *"),
	ZOp(&edp6F,14,"rld"),

	ZOp(&edp70,8,"in (c)"),
	ZOp(&edp71,8,"out (c),0"),
	ZOp(&edp72,11,"sbc hl,sp"),
	ZOp(&edp73,16,"ld (:2),sp"),
	ZOp(&edp74,4,"neg *"),
	ZOp(&edp75,11,"retn *"),
	ZOp(&edp76,4,"im 1 *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&edp78,8,"in a,(c)"),
	ZOp(&edp79,8,"out (c),a"),
	ZOp(&edp7A,11,"adc hl,sp"),
	ZOp(&edp7B,16,"ld sp,(:2)"),
	ZOp(&edp7C,4,"neg *"),
	ZOp(&edp7D,11,"reti *"),
	ZOp(&edp7E,4,"im 2 *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&edpA0,12,"ldi"),
	ZOp(&edpA1,12,"cpi"),
	ZOp(&edpA2,12,"ini"),
	ZOp(&edpA3,12,"outi"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&edpA8,12,"ldd"),
	ZOp(&edpA9,12,"cpd"),
	ZOp(&edpAA,12,"ind"),
	ZOp(&edpAB,12,"outd"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&edpB0,12,"ldir"),
	ZOp(&edpB1,12,"cpir"),
	ZOp(&edpB2,12,"inir"),
	ZOp(&edpB3,12,"otir"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&edpB8,12,"lddr"),
	ZOp(&edpB9,12,"cpdr"),
	ZOp(&edpBA,12,"indr"),
	ZOp(&edpBB,12,"otdr"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),

	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
	ZOp(&npr00,4,"nop *"),
};
