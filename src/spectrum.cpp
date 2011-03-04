#include "spectrum.h"
#include "video.h"
#include "settings.h"
#include "bdi.h"

#include "z80/tables.c"
#include "z80/instr.c"

Spec::Spec () {
	inst[0] = nopref;
	inst[1] = ixpref;
	inst[2] = iypref;
	inst[4] = cbpref;
	inst[5] = cxpref;
	inst[6] = cypref;
	inst[8] = edpref;
	nmi = false;
}

int Spec::interrupt() {
	if (!cpu->iff1) return 0;
	cpu->iff1 = cpu->iff2 = false;
	int res = 0;
	switch (cpu->imode) {
		case 0: mem->wr(--cpu->sp,cpu->hpc); mem->wr(--cpu->sp,cpu->lpc); cpu->pc = 0x38; cpu->mptr = cpu->pc; res = 12; break;
		case 1: mem->wr(--cpu->sp,cpu->hpc); mem->wr(--cpu->sp,cpu->lpc); cpu->pc = 0x38; cpu->mptr = cpu->pc; res = 13; break;
		case 2: mem->wr(--cpu->sp,cpu->hpc); mem->wr(--cpu->sp,cpu->lpc);
			cpu->adr = (cpu->i << 8) | 0xff; cpu->lpc = mem->rd(cpu->adr++); cpu->hpc = mem->rd(cpu->adr);
			cpu->mptr = cpu->pc; res = 19; break;
	}
	if ((io->flags & IO_WAIT) && sets->wait && (cpu->t & 1)) res++;		// TODO: is INT handle is WAIT'ing too?
	cpu->t += res;
	return res;
}

void Spec::nmihandle() {
	cpu->iff2 = cpu->iff1;
	cpu->iff1 = false;
	mem->wr(--cpu->sp,cpu->hpc);
	mem->wr(--cpu->sp,cpu->lpc);
	cpu->pc = 0x66;
	bdi->active = true;
	machine->setrom();
	cpu->t += 11;
}

int Spec::exec() {
	istrb = false;
	cpu->err = false;
	uint32_t fcnt = cpu->t;
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
		if (res->prf) res->func(this);
	} while (res->prf);
	if (vid != NULL) {vid->sync(cpu->t - cpu->ti, cpu->frq); fcnt = cpu->t;}
	res->func(this);
	if ((io->flags & IO_WAIT) && sets->wait && (cpu->t & 1)) cpu->t++;		// WAIT
	if ((cpu->hpc > 0x3f) && nmi) nmihandle();					// NMI
	if ((vid != NULL) && (cpu->t > fcnt)) vid->sync(cpu->t - fcnt, cpu->frq);
	return (cpu->t - cpu->ti);
}
