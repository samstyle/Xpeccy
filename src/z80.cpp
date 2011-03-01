//#include "z80.h"
#include "debuger.h"
//#include "emulwin.h"
#include "spectrum.h"
#include "video.h"
#include "sound.h"
#include "tape.h"
//#include "memory.h"
//#include "iosys.h"
#include "bdi.h"

//#include "z80/tables.c"

Z80::Z80(float fr) {
//	filltabs();
	t = tb = 0;
	frq = fr;
}

void Z80::reset() {
	err = block = false;
	pc = 0;
	iff1 = iff2 = false;
}

//void Z80::sync(int ts) {
//	vid->sync(ts);
//	t += ts;
//}

/*
void Z80::exec() {
	istrb = false;
	err = false;
	if (block & !dbg->active) {sync(4); return;}

	if (nextei) {iff1 = iff2 = true; nextei = false;}
	ZOp *res = NULL;
	mod = 0;
	ti = t;
	do {
		r = ((r + 1) & 0x7f) | (r & 0x80);
		cod = mem->rd(pc++);
		res = &inst[mod][cod];
		t += res->t;
		if (res->prf) res->func(this);
	} while (res->prf);
	vid->sync(t - ti);
//	unsigned int fcnt = t;
	res->func(this);
//	if (fcnt < t) vid->sync(t - fcnt);

	if ((snd->enabled) && (t > snd->t)) snd->sync();
	if (bdi->enable) {
		bdi->sync(t - ti);
		if (bdi->active && (hpc > 0x3f)) {bdi->active = false; machine->setrom();}
		if (!bdi->active && (hpc == 0x3d) && (mem->prt0 & 0x10)) {bdi->active = true; machine->setrom();}
	}
	if (!dbg->active) {
		// somehow catch CPoint
		if (dbg->findbp(BPoint((pc < 0x4000)?mem->crom:mem->cram,pc)) != -1) {
			dbg->start();
			err = true;
		}
		if (!err && istrb) interrupt();
	}
}
*/
