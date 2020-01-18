// emulation thread (non-GUI)

#include <QWaitCondition>

#include "ethread.h"
#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "xcore/sound.h"
#include "xcore/vfilters.h"

#if USEMUTEX
QMutex emutex;
QWaitCondition qwc;
#else
int sleepy = 1;
#endif

unsigned char* blkData = NULL;

xThread::xThread() {
	sndNs = 0;
	conf.emu.fast = 0;
	finish = 0;
}

void xThread::stop() {
	finish = 1;
#if USEMUTEX
	qwc.wakeAll();
#else
	sleepy = 0;
#endif
}

void xThread::tap_catch_load(Computer* comp) {
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
		} else {
			comp->cpu->hl = 0xff00;
		}
		tapNextBlock(comp->tape);
		comp->cpu->pc = 0x5df;
	} else if (conf.tape.autostart) {
		emit tapeSignal(TW_STATE,TWS_PLAY);
	}
}

void xThread::tap_catch_save(Computer* comp) {
	if (conf.tape.fast) {
		unsigned short de = comp->cpu->de;	// len
		unsigned short ix = comp->cpu->ix;	// adr
		unsigned char crc = comp->cpu->a;	// block type
		unsigned char* buf = (unsigned char*)malloc(de + 2);

		buf[0] = crc;
		for(int i = 0; i < de; i++) {
			buf[i + 1] = memRd(comp->mem, (ix + i) & 0xffff);
			crc ^= buf[i + 1];
		}
		buf[de + 1] = crc;

		TapeBlock blk = tapDataToBlock((char*)buf, de + 2, NULL);
		tapAddBlock(comp->tape, blk);
		blkClear(&blk);
		free(buf);
		comp->cpu->pc = 0x053e;
		comp->cpu->bc = 0x000e;
		comp->cpu->de = 0xffff;
		comp->cpu->hl = 0x0000;
		comp->cpu->af = 0x0051;
	} else if (conf.tape.autostart) {
		emit tapeSignal(TW_STATE, TWS_REC);
	}
}

void xThread::emuCycle(Computer* comp) {
	sndNs = 0;
	conf.snd.fill = 1;
	do {
		// exec 1 opcode (or handle INT, NMI)
		if (conf.emu.pause) {
			sndNs += 1000;
		} else {
			sndNs += compExec(comp);
			// tape trap
			if ((comp->hw->grp == HWG_ZX) && (comp->mem->map[0].type == MEM_ROM) && comp->rom && !comp->dos && !comp->ext) {
				if (comp->cpu->pc == 0x562) {			// load: ix:addr, de:len (0x580 ?)
					tap_catch_load(comp);
				} else if (comp->cpu->pc == 0x4d0) {		// save: ix:addr, de:len, a:block type(b7), hl:pilot len (1f80/0c98)?
					tap_catch_save(comp);
				}
				if (conf.tape.autostart && !conf.tape.fast && ((comp->cpu->pc == 0x5df) || (comp->cpu->pc == 0x53a))) {
					comp->tape->sigLen += 1e6;
					tapNextBlock(comp->tape);
					emit tapeSignal(TW_STATE,TWS_STOP);
				}
			}
		}
		// sound buffer update
		while (sndNs > nsPerSample) {
			sndSync(comp);
			sndNs -= nsPerSample;
		}
		if (comp->frmStrobe) {
			comp->frmStrobe = 0;
			conf.vid.fcount++;
			emit s_frame();
		}
	} while (!comp->brk && conf.snd.fill && !finish && !conf.emu.pause);
	comp->nmiRequest = 0;
}

void xThread::run() {
	Computer* comp;
	do {
#if !USEMUTEX
		sleepy = 1;
#endif
		comp = conf.prof.cur->zx;
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
		if (!conf.emu.pause) {
			emuCycle(comp);
			if (comp->brk) {
				conf.emu.pause |= PR_DEBUG;
				comp->brk = 0;
				emit dbgRequest();
			}
		}
#if USEMUTEX
		if (!conf.emu.fast && !finish) {
			emutex.lock();
			qwc.wait(&emutex);
			emutex.unlock();
		}
#else
		while (!conf.emu.fast && sleepy && !finish)
			usleep(10);
#endif
	} while (!finish);
	exit(0);
}
