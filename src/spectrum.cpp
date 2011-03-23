#include "spectrum.h"
#include "video.h"
#include "settings.h"
#include "bdi.h"

#include "z80/tables.c"
#include "z80/instr.c"

extern Settings* sets;
extern BDI* bdi;
extern HardWare* hw;

uint8_t zx_in(int);
void zx_out(int,uint8_t);

ZOp::ZOp(void(*fn)(ZXBase*),int32_t tks,int32_t cnd,int32_t t1,int32_t t0,const char* nm,int32_t fl) {
	name = nm;
	func = fn;
	t = tks;
	cond = cnd;
	tcn0 = t0;
	tcn1 = t1;
	flags = fl;
}

ZOp::ZOp(void(*fn)(ZXBase*),int32_t tks,const char* nm) {
	name = nm;
	func = fn;
	t = tks;
	cond = CND_NONE;
	tcn0 = tcn1 = 0;
	flags = 0;
}

ZXComp::ZXComp() {
	sys = new ZXBase;
	sys->cpu = new Z80(3.5);
	sys->mem = new Memory(MEM_ZX);
	sys->io = new IOSys(&zx_in,&zx_out);
//	vid = new Video;
	tape = new Tape;
}

void ZXComp::reset() {
	sys->io->block7ffd=false;
	sys->mem->prt1 = 0;
	sys->mem->prt0 = ((sys->mem->res==1)<<4);
	sys->mem->setrom(sys->mem->res);
	sys->mem->setram(0);
	sys->cpu->reset();
	vid->curscr = false;
	vid->mode = VID_NORMAL;
}

void ZXComp::exec() {
	ZOpResult res = sys->exec();
	vid->sync(res.ticks,sys->cpu->frq);
	res.exec(sys);
	if ((sys->cpu->hpc > 0x3f) && sys->nmi) {
		NMIHandle();
	}
}

void ZXComp::INTHandle() {
	int32_t res = sys->interrupt();
//printf("%i\n",res);
	vid->sync(res,sys->cpu->frq);
}

void ZXComp::NMIHandle() {
	sys->cpu->iff2 = sys->cpu->iff1;
	sys->cpu->iff1 = false;
	sys->mem->wr(--sys->cpu->sp,sys->cpu->hpc);
	sys->mem->wr(--sys->cpu->sp,sys->cpu->lpc);
	sys->cpu->pc = 0x66;
	bdi->active = true;
	hw->setrom();
	sys->cpu->t += 11;
	vid->sync(11,sys->cpu->frq);
}

ZXBase::ZXBase () {
	inst[0] = nopref;
	inst[1] = ixpref;
	inst[2] = iypref;
	inst[4] = cbpref;
	inst[5] = cxpref;
	inst[6] = cypref;
	inst[8] = edpref;
	nmi = false;
}

ZOpResult ZXBase::exec() {
	ZOpResult zres;
	istrb = false;
	cpu->err = false;
//	uint32_t fcnt = cpu->t;
//	if (cpu->block & !dbg->active) {sync(4); return;}
	if (cpu->nextei) {
		cpu->iff1 = cpu->iff2 = true;
		cpu->nextei = false;
	}
	ZOp *res = NULL;
	cpu->mod = 0;
	cpu->ti = cpu->t;
	do {
		cpu->r = ((cpu->r + 1) & 0x7f) | (cpu->r & 0x80);
		cpu->cod = mem->rd(cpu->pc++);
		res = &inst[cpu->mod][cpu->cod];
		cpu->t += res->t;
		if (res->flags & ZOP_PREFIX) res->func(this);
	} while (res->flags & ZOP_PREFIX);
	switch (res->cond) {
		case CND_NONE: break;
		case CND_Z: cpu->t += (cpu->f & FZ) ? res->tcn1 : res->tcn0; break;
		case CND_C: cpu->t += (cpu->f & FC) ? res->tcn1 : res->tcn0; break;
		case CND_P: cpu->t += (cpu->f & FP) ? res->tcn1 : res->tcn0; break;
		case CND_S: cpu->t += (cpu->f & FS) ? res->tcn1 : res->tcn0; break;
		case CND_DJNZ: cpu->t += (cpu->b != 1) ? res->tcn1 : res->tcn0; break;
		case CND_LDIR: cpu->t += (cpu->bc != 1) ? res->tcn1 : res->tcn0; break;
		case CND_CPIR: cpu->t += ((cpu->bc != 1) && (cpu->a != mem->rd(cpu->hl))) ? res->tcn1 : res->tcn0; break;
		default: printf("undefined condition\n"); throw(0);
	}
//	if (vid != NULL) {vid->sync(cpu->t - cpu->ti, cpu->frq); fcnt = cpu->t;}
	zres.exec = res->func;
//	res->func(this);
	if ((io->flags & IO_WAIT) && sets->wait && (cpu->t & 1)) cpu->t++;		// WAIT
//	if ((cpu->hpc > 0x3f) && nmi) nmihandle();					// NMI
//	if ((vid != NULL) && (cpu->t > fcnt)) vid->sync(cpu->t - fcnt, cpu->frq);
//	return (cpu->t - cpu->ti);
	zres.ticks = cpu->t - cpu->ti;
	return zres;
}

int32_t ZXBase::interrupt() {
	if (!cpu->iff1) return 0;
	cpu->iff1 = cpu->iff2 = false;
	int32_t res = 0;
	switch (cpu->imode) {
		case 0: mem->wr(--cpu->sp,cpu->hpc);
			mem->wr(--cpu->sp,cpu->lpc);
			cpu->pc = 0x38;
			cpu->mptr = cpu->pc;
			res = 12;
			break;
		case 1: mem->wr(--cpu->sp,cpu->hpc);
			mem->wr(--cpu->sp,cpu->lpc);
			cpu->pc = 0x38;
			cpu->mptr = cpu->pc;
			res = 13;
			break;
		case 2: mem->wr(--cpu->sp,cpu->hpc);
			mem->wr(--cpu->sp,cpu->lpc);
			cpu->adr = (cpu->i << 8) | 0xff;
			cpu->lpc = mem->rd(cpu->adr);
			cpu->hpc = mem->rd(cpu->adr+1);
			cpu->mptr = cpu->pc;
			res = 19;
			break;
	}
	if ((io->flags & IO_WAIT) && sets->wait && (cpu->t & 1)) res++;		// TODO: is INT handle is WAIT'ing too?
	cpu->t += res;
	return res;
}
