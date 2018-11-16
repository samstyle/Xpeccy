// emulation thread (non-GUI)

#include "ethread.h"
#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "xcore/sound.h"
#include "xcore/vfilters.h"

QMutex emutex;

unsigned char* blkData = NULL;
#if !VID_DIRECT_DRAW
extern unsigned char sbufa[1024 * 512 * 3];
extern unsigned char scrn[1024 * 512 * 3];
extern unsigned char prvScr[1024 * 512 * 3];

void processPicture(unsigned char* src, int size) {
	memcpy(sbufa, src, size);
	scrMix(prvScr, sbufa, size, noflic / 100.0);
}
#endif

xThread::xThread() {
	sndNs = 0;
	conf.emu.fast = 0;
	finish = 0;
	emutex.lock();
}

void xThread::tapeCatch() {
	int blk = comp->tape->block;
	if (blk >= comp->tape->blkCount) return;
	if (conf.tape.fast && comp->tape->blkData[blk].hasBytes) {
		unsigned short de = comp->cpu->de;
		unsigned short ix = comp->cpu->ix;
		TapeBlockInfo inf = tapGetBlockInfo(comp->tape,blk);
		blkData = (unsigned char*)realloc(blkData,inf.size + 2);
		tapGetBlockData(comp->tape,blk,blkData, inf.size + 2);
		if (inf.size == de) {
			for (int i = 0; i < de; i++) {
				memWr(comp->mem,ix,blkData[i + 1]);
				ix++;
			}
			comp->cpu->ix = ix;
			comp->cpu->de = 0;
			comp->cpu->hl = 0;
			tapNextBlock(comp->tape);
		} else {
			comp->cpu->hl = 0xff00;
		}
		comp->cpu->pc = 0x5df;
	} else {
		if (conf.tape.autostart)
			emit tapeSignal(TW_STATE,TWS_PLAY);
	}
}

void xThread::emuCycle() {
//	int endBuf = 0;
	comp->frmStrobe = 0;
	sndNs = 0;
	conf.snd.fill = 1;
	do {
		// exec 1 opcode (or handle INT, NMI)
		if (conf.emu.pause) {
			sndNs += 1000;
		} else {
			sndNs += compExec(comp);
#if !VID_DIRECT_DRAW
		// tape trap
			if (comp->frmStrobe && !conf.emu.fast) {
				comp->frmStrobe = 0;
				processPicture(comp->vid->scrimg, comp->vid->vBytes);
				emit picReady();
			}
#endif
			if ((comp->mem->map[0].type == MEM_ROM) && comp->rom && !comp->dos) {		// FIXME: shit
				if (comp->cpu->pc == 0x56b) tapeCatch();
				if ((comp->cpu->pc == 0x5e2) && conf.tape.autostart)
					emit tapeSignal(TW_STATE,TWS_STOP);
			}
		}
		// if need - request sound buffer update
		if (sndNs > nsPerSample) {
			sndSync(comp);
			sndNs -= nsPerSample;
		}
	} while (!comp->brk && conf.snd.fill);
	comp->nmiRequest = 0;
}

void xThread::run() {
	emutex.lock();
	do {
#ifdef HAVEZLIB
		if (comp->rzx.start) {
			comp->rzx.start = 0;
			comp->rzx.play = 1;
			comp->rzx.fCount = 0;
			comp->rzx.fCurrent = 0;
			rewind(comp->rzx.file);
			rzxGetFrame(comp);
		}
#endif
		if (!block && !comp->brk) {
			emuCycle();
			if (comp->brk) {
				conf.emu.pause |= PR_DEBUG;
				comp->brk = 0;
				emit dbgRequest();
			}
		}
		if (!conf.emu.fast) emutex.lock();		// wait until unlocked (MainWin::onTimer() or at exit)
	} while (!finish);
	emutex.unlock();
	exit(0);
}
