// NOTE:ticks are NOT include ED prefix fetch. (-4)

// in
void edp40(ZXBase* p) {p->cpu->b = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->b].in | (p->cpu->f & FC);}
void edp48(ZXBase* p) {p->cpu->c = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->c].in | (p->cpu->f & FC);}
void edp50(ZXBase* p) {p->cpu->d = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->d].in | (p->cpu->f & FC);}
void edp58(ZXBase* p) {p->cpu->e = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->e].in | (p->cpu->f & FC);}
void edp60(ZXBase* p) {p->cpu->h = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->h].in | (p->cpu->f & FC);}
void edp68(ZXBase* p) {p->cpu->l = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->l].in | (p->cpu->f & FC);}
void edp70(ZXBase* p) {p->cpu->x = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->x].in | (p->cpu->f & FC);}
void edp78(ZXBase* p) {p->cpu->a = p->io->in(p->cpu->bc); p->cpu->f = flag[p->cpu->a].in | (p->cpu->f & FC); p->cpu->mptr = p->cpu->bc + 1;}	// in a,(c)	mptr = bc + 1
// out
void edp41(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->b);}
void edp49(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->c);}
void edp51(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->d);}
void edp59(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->e);}
void edp61(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->h);}
void edp69(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->l);}
void edp71(ZXBase* p) {p->io->out(p->cpu->bc,0);}
void edp79(ZXBase* p) {p->io->out(p->cpu->bc,p->cpu->a); p->cpu->mptr = p->cpu->bc + 1;}	// out (c),a	mptr = bc + 1
// sbc hl,rp
void edp42(ZXBase* p) {rpSBC(p,p->cpu->bc);}
void edp52(ZXBase* p) {rpSBC(p,p->cpu->de);}
void edp62(ZXBase* p) {rpSBC(p,p->cpu->hl);}
void edp72(ZXBase* p) {rpSBC(p,p->cpu->sp);}
// adc hl,rp
void edp4A(ZXBase* p) {rpADC(p,p->cpu->bc);}
void edp5A(ZXBase* p) {rpADC(p,p->cpu->de);}
void edp6A(ZXBase* p) {rpADC(p,p->cpu->hl);}
void edp7A(ZXBase* p) {rpADC(p,p->cpu->sp);}
// ld (nn).rp	mptr = nn + 1
void edp43(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->c); p->mem->wr(p->cpu->mptr,p->cpu->b);}
void edp53(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->e); p->mem->wr(p->cpu->mptr,p->cpu->d);}
void edp63(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->l); p->mem->wr(p->cpu->mptr,p->cpu->h);}
void edp73(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->lsp); p->mem->wr(p->cpu->mptr,p->cpu->hsp);}
// ld rp,(nn)	mptr = nn + 1
void edp4B(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->c = p->mem->rd(p->cpu->mptr++); p->cpu->b = p->mem->rd(p->cpu->mptr);}
void edp5B(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->e = p->mem->rd(p->cpu->mptr++); p->cpu->d = p->mem->rd(p->cpu->mptr);}
void edp6B(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->l = p->mem->rd(p->cpu->mptr++); p->cpu->h = p->mem->rd(p->cpu->mptr);}
void edp7B(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->lsp = p->mem->rd(p->cpu->mptr++); p->cpu->hsp = p->mem->rd(p->cpu->mptr);}
// neg
void edp44(ZXBase* p) {p->cpu->a = -p->cpu->a; p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:FC) | ((p->cpu->a & 15)?FH:0) | ((p->cpu->a == 0x80)?FP:0) | FN;}
void edp4C(ZXBase* p) {edp44(p);}
void edp54(ZXBase* p) {edp44(p);}
void edp5C(ZXBase* p) {edp44(p);}
void edp64(ZXBase* p) {edp44(p);}
void edp6C(ZXBase* p) {edp44(p);}
void edp74(ZXBase* p) {edp44(p);}
void edp7C(ZXBase* p) {edp44(p);}
// retn / reti		mptr = ret.adr
void edp45(ZXBase* p) {p->cpu->iff1 = p->cpu->iff2; p->cpu->lpc = p->mem->rd(p->cpu->sp++); p->cpu->hpc = p->mem->rd(p->cpu->sp++); p->cpu->mptr = p->cpu->pc;}		// retn
void edp55(ZXBase* p) {edp45(p);}
void edp65(ZXBase* p) {edp45(p);}
void edp75(ZXBase* p) {edp45(p);}
void edp4D(ZXBase* p) {p->cpu->lpc = p->mem->rd(p->cpu->sp++); p->cpu->hpc = p->mem->rd(p->cpu->sp++); p->cpu->mptr = p->cpu->pc;}				// reti
void edp5D(ZXBase* p) {edp4D(p);}
void edp6D(ZXBase* p) {edp4D(p);}
void edp7D(ZXBase* p) {edp4D(p);}
// im
void edp46(ZXBase* p) {p->cpu->imode = 0;}
void edp4E(ZXBase* p) {edp46(p);}
void edp66(ZXBase* p) {edp46(p);}
void edp6E(ZXBase* p) {edp46(p);}
void edp56(ZXBase* p) {p->cpu->imode = 1;}
void edp76(ZXBase* p) {edp56(p);}
void edp5E(ZXBase* p) {p->cpu->imode = 2;}
void edp7E(ZXBase* p) {edp5E(p);}
// ld [i,r]
void edp47(ZXBase* p) {p->cpu->i = p->cpu->a;}
void edp4F(ZXBase* p) {p->cpu->r = p->cpu->a;}
void edp57(ZXBase* p) {p->cpu->a = p->cpu->i; p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a & 0xff)?0:FZ) | (p->cpu->iff2 ? FP:0) | (p->cpu->f & FC);}
void edp5F(ZXBase* p) {p->cpu->a = p->cpu->r; p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a & 0xff)?0:FZ) | (p->cpu->iff2 ? FP:0) | (p->cpu->f & FC);}
// rrd / rld	mptr = hl + 1
void edp67(ZXBase* p) {p->cpu->mptr = p->cpu->hl + 1;
		p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->dlt = p->cpu->x & 0x0f;
		p->cpu->x = ((p->cpu->x & 0xf0) >> 4) | ((p->cpu->a & 0x0f) << 4); p->cpu->a = (p->cpu->a & 0xf0) | p->cpu->dlt; p->mem->wr(p->cpu->hl,p->cpu->x);
		p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:0) | (parity(p->cpu->a)?FP:0) | (p->cpu->f & FC);
}
void edp6F(ZXBase* p) {p->cpu->mptr = p->cpu->hl + 1;
		p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->dlt = p->cpu->x & 0xf0;
		p->cpu->x = ((p->cpu->x & 0x0f) << 4) | (p->cpu->a & 0x0f); p->cpu->a = (p->cpu->a & 0xf0) | (p->cpu->dlt >> 4); p->mem->wr(p->cpu->hl,p->cpu->x);
		p->cpu->f = (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:0) | (parity(p->cpu->a)?FP:0) | (p->cpu->f & FC);
}
// [blk]i
void edpA0(ZXBase* p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl++); p->mem->wr(p->cpu->de++,p->cpu->x); p->cpu->x += p->cpu->a;
		p->cpu->f = (p->cpu->f & (FS | FZ | FC)) | (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0) | ((p->cpu->bc != 0)?FP:0);}
void edpA1(ZXBase* p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl++); p->cpu->f = (p->cpu->f & FC) | (flag[p->cpu->a].cp[p->cpu->x] & (FS | FZ | FH | FN)) | ((p->cpu->bc != 0)?FP:0);
		p->cpu->x = p->cpu->a - p->cpu->x - ((p->cpu->f & FH)?1:0); p->cpu->f |= (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0); p->cpu->mptr++;}		// cpi	mptr++
void edpA2(ZXBase* p) {p->cpu->mptr = p->cpu->bc + 1; p->mem->wr(p->cpu->hl++,p->io->in(p->cpu->bc)); p->cpu->b--; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// ini	mptr = bc.before + 1
void edpA3(ZXBase* p) {p->io->out(p->cpu->bc,p->mem->rd(p->cpu->hl++)); p->cpu->b--; p->cpu->mptr = p->cpu->bc + 1; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// outi	mptr = bc.after + 1
// [blk]d
void edpA8(ZXBase* p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl--); p->mem->wr(p->cpu->de--,p->cpu->x); p->cpu->x += p->cpu->a;
		p->cpu->f = (p->cpu->f & (FS | FZ | FC)) | (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0) | ((p->cpu->bc != 0)?FP:0);}
void edpA9(ZXBase* p) {p->cpu->bc--; p->cpu->x = p->mem->rd(p->cpu->hl--); p->cpu->f = (p->cpu->f & FC) | (flag[p->cpu->a].cp[p->cpu->x] & (FS | FZ | FH | FN)) | ((p->cpu->bc != 0)?FP:0);
		p->cpu->x = p->cpu->a - p->cpu->x - ((p->cpu->f & FH)?1:0); p->cpu->f |= (p->cpu->x & F3) | ((p->cpu->x & 2)?F5:0); p->cpu->mptr--;}		// cpd: mptr--
void edpAA(ZXBase* p) {p->cpu->mptr = p->cpu->bc - 1; p->mem->wr(p->cpu->hl--,p->io->in(p->cpu->bc)); p->cpu->b--; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// ind	mptr = bc.before - 1
void edpAB(ZXBase* p) {p->io->out(p->cpu->bc,p->mem->rd(p->cpu->hl--)); p->cpu->b--; p->cpu->mptr = p->cpu->bc - 1; p->cpu->f = (p->cpu->f & ~(FZ | FN)) | ((p->cpu->b == 0)?FZ:0) | FN;}	// outd	mptr = bc.after - 1
// [blk]ir
void edpB0(ZXBase* p) {edpA0(p); if (p->cpu->f & FP) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}			// ldir: if (not over) mptr = instr.adr + 1
void edpB1(ZXBase* p) {edpA1(p); if ((!(p->cpu->f & FZ)) && (p->cpu->f & FP)) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}	// cpir: if (not over) mptr = instr.adr + 1
void edpB2(ZXBase* p) {edpA2(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}
void edpB3(ZXBase* p) {edpA3(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}
// [blk]dr
void edpB8(ZXBase* p) {edpA8(p); if (p->cpu->f & FP) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}			// lddr: if (not over) mptr = instr.adr + 1
void edpB9(ZXBase* p) {edpA9(p); if ((!(p->cpu->f & FZ)) && (p->cpu->f & FP)) {p->cpu->mptr = p->cpu->pc - 1; p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}	// cpdr: if (not over) mptr = instr.adr + 1
void edpBA(ZXBase* p) {edpAA(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}
void edpBB(ZXBase* p) {edpAB(p); if (!(p->cpu->f & FZ)) {p->cpu->pc -= 2; /*p->cpu->t += 5;*/}}

ZOp edpref[256]={
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{8,	CND_NONE,0,0,	0,	&edp40,	"in b,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp41,	"out (c),b"},
	{11,	CND_NONE,0,0,	0,	&edp42,	"sbc hl,bc"},
	{16,	CND_NONE,0,0,	0,	&edp43,	"ld (:2),bc"},
	{4,	CND_NONE,0,0,	0,	&edp44,	"neg"},
	{11,	CND_NONE,0,0,	0,	&edp45,	"retn"},
	{4,	CND_NONE,0,0,	0,	&edp46,	"im 0"},
	{5,	CND_NONE,0,0,	0,	&edp47,	"ld i,a"},

	{8,	CND_NONE,0,0,	0,	&edp48,	"in c,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp49,	"out (c),c"},
	{11,	CND_NONE,0,0,	0,	&edp4A,	"adc hl,bc"},
	{16,	CND_NONE,0,0,	0,	&edp4B,	"ld bc,(:2)"},
	{4,	CND_NONE,0,0,	0,	&edp4C,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp4D,	"reti"},
	{4,	CND_NONE,0,0,	0,	&edp4E,	"im 0 *"},
	{5,	CND_NONE,0,0,	0,	&edp4F,	"ld r,a"},

	{8,	CND_NONE,0,0,	0,	&edp50,	"in d,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp51,	"out (c),d"},
	{11,	CND_NONE,0,0,	0,	&edp52,	"sbc hl,de"},
	{16,	CND_NONE,0,0,	0,	&edp53,	"ld (:2),de"},
	{4,	CND_NONE,0,0,	0,	&edp54,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp55,	"retn *"},
	{4,	CND_NONE,0,0,	0,	&edp56,	"im 1"},
	{5,	CND_NONE,0,0,	0,	&edp57,	"ld a,i"},

	{8,	CND_NONE,0,0,	0,	&edp58,	"in e,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp59,	"out (c),e"},
	{11,	CND_NONE,0,0,	0,	&edp5A,	"adc hl,de"},
	{16,	CND_NONE,0,0,	0,	&edp5B,	"ld de,(:2)"},
	{4,	CND_NONE,0,0,	0,	&edp5C,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp5D,	"reti *"},
	{4,	CND_NONE,0,0,	0,	&edp5E,	"im 2"},
	{5,	CND_NONE,0,0,	0,	&edp5F,	"ld a,r"},

	{8,	CND_NONE,0,0,	0,	&edp60,	"in h,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp61,	"out (c),h"},
	{11,	CND_NONE,0,0,	0,	&edp62,	"sbc hl,hl"},
	{16,	CND_NONE,0,0,	0,	&edp63,	"ld (:2),hl"},
	{4,	CND_NONE,0,0,	0,	&edp64,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp65,	"retn *"},
	{4,	CND_NONE,0,0,	0,	&edp66,	"im 0 *"},
	{14,	CND_NONE,0,0,	0,	&edp67,	"rrd"},

	{8,	CND_NONE,0,0,	0,	&edp68,	"in l,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp69,	"out (c),l"},
	{11,	CND_NONE,0,0,	0,	&edp6A,	"adc hl,hl"},
	{16,	CND_NONE,0,0,	0,	&edp6B,	"ld hl,(:2)"},
	{4,	CND_NONE,0,0,	0,	&edp6C,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp6D,	"reti *"},
	{4,	CND_NONE,0,0,	0,	&edp6E,	"im 0 *"},
	{14,	CND_NONE,0,0,	0,	&edp6F,	"rld"},

	{8,	CND_NONE,0,0,	0,	&edp70,	"in (c)"},
	{8,	CND_NONE,0,0,	0,	&edp71,	"out (c),0"},
	{11,	CND_NONE,0,0,	0,	&edp72,	"sbc hl,sp"},
	{16,	CND_NONE,0,0,	0,	&edp73,	"ld (:2),sp"},
	{4,	CND_NONE,0,0,	0,	&edp74,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp75,	"retn *"},
	{4,	CND_NONE,0,0,	0,	&edp76,	"im 1 *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{8,	CND_NONE,0,0,	0,	&edp78,	"in a,(c)"},
	{8,	CND_NONE,0,0,	0,	&edp79,	"out (c),a"},
	{11,	CND_NONE,0,0,	0,	&edp7A,	"adc hl,sp"},
	{16,	CND_NONE,0,0,	0,	&edp7B,	"ld sp,(:2)"},
	{4,	CND_NONE,0,0,	0,	&edp7C,	"neg *"},
	{11,	CND_NONE,0,0,	0,	&edp7D,	"reti *"},
	{4,	CND_NONE,0,0,	0,	&edp7E,	"im 2 *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{12,	CND_NONE,0,0,	0,	&edpA0,	"ldi"},
	{12,	CND_NONE,0,0,	0,	&edpA1,	"cpi"},
	{12,	CND_NONE,0,0,	0,	&edpA2,	"ini"},
	{12,	CND_NONE,0,0,	0,	&edpA3,	"outi"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{12,	CND_NONE,0,0,	0,	&edpA8,	"ldd"},
	{12,	CND_NONE,0,0,	0,	&edpA9,	"cpd"},
	{12,	CND_NONE,0,0,	0,	&edpAA,	"ind"},
	{12,	CND_NONE,0,0,	0,	&edpAB,	"outd"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{12,	CND_LDIR,5,0,	0,	&edpB0,	"ldir"},
	{12,	CND_CPIR,5,0,	0,	&edpB1,	"cpir"},
	{12,	CND_LDIR,5,0,	0,	&edpB2,	"inir"},
	{12,	CND_LDIR,5,0,	0,	&edpB3,	"otir"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{12,	CND_LDIR,5,0,	0,	&edpB8,	"lddr"},
	{12,	CND_CPIR,5,0,	0,	&edpB9,	"cpdr"},
	{12,	CND_LDIR,5,0,	0,	&edpBA,	"indr"},
	{12,	CND_LDIR,5,0,	0,	&edpBB,	"otdr"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},

	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop *"},
};
