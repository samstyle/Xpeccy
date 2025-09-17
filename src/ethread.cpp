// emulation thread (non-GUI)

#include <QWaitCondition>
#include <QTime>

#include "ethread.h"
#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "xcore/sound.h"
#include "xcore/vfilters.h"
#include "libxpeccy/cpu/Z80/z80.h"

#if USEMUTEX
QMutex emutex;
QWaitCondition qwc;
#else
int sleepy = 1;
#endif

#define LOG_OUTPUT 0
#if LOG_OUTPUT
static FILE* file = nullptr;
#endif

// unsigned char* blkData = NULL;

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

// TODO: do cpu_set_pc instead of comp->regPC=...

void xThread::tap_catch_load(Computer* comp) {
	int blk = comp->tape->block;
	if (blk >= comp->tape->blkCount) return;
	if (conf.tape.fast && comp->tape->blkData[blk].hasBytes) {
		unsigned short de = comp->cpu->regDE;
		unsigned short ix = comp->cpu->regIX;
		TapeBlockInfo inf = tapGetBlockInfo(comp->tape,blk,TFRM_ZX);
		unsigned char* blkData = (unsigned char*)malloc(inf.size + 2);
		tapGetBlockData(comp->tape,blk,blkData, inf.size + 2);
		if (inf.size >= de) {
			for (int i = 0; i < de; i++) {
				memWr(comp->mem,ix,blkData[i + 1]);
				ix++;
			}
			comp->cpu->regIX = ix;
			comp->cpu->regDE = 0x0000;
			comp->cpu->regHL = 0x0000;
		} else {
			comp->cpu->regHL = 0xff00;
		}
		tapNextBlock(comp->tape);
		cpu_set_pc(comp->cpu, 0x5df);
		// comp->cpu->regPC = 0x5df;
		free(blkData);
	} else if (conf.tape.autostart) {
		emit tapeSignal(TW_STATE,TWS_PLAY);
	}
}

void xThread::tap_catch_save(Computer* comp) {
	if (conf.tape.fast) {
		unsigned short de = comp->cpu->regDE;	// len
		unsigned short ix = comp->cpu->regIX;	// adr
		unsigned char crc = comp->cpu->regA;	// block type
		unsigned char* buf = (unsigned char*)malloc(de + 2);

		buf[0] = crc;
		for(int i = 0; i < de; i++) {
			buf[i + 1] = memRd(comp->mem, (ix + i) & 0xffff);
			crc ^= buf[i + 1];
		}
		buf[de + 1] = crc;

		TapeBlock blk = tapDataToBlock((char*)buf, de + 2, NULL);
		tap_add_block(comp->tape, blk);
		blkClear(&blk);
		free(buf);
		cpu_set_pc(comp->cpu, 0x53e);
		// comp->cpu->regPC = 0x053e;
		comp->cpu->regBC = 0x000e;
		comp->cpu->regDE = 0xffff;
		comp->cpu->regHL = 0x0000;
		comp->cpu->regA = 0x00;
		cpu_set_flag(comp->cpu, 0x51); // comp->cpu->f = 0x51;
	} else if (conf.tape.autostart) {
		emit tapeSignal(TW_STATE, TWS_REC);
	}
}

void xThread::emuCycle(Computer* comp) {
	int tm;
	int brkskip = 0;
	sndNs = 0;
	wavNs = 0;
	conf.snd.fill = 1;
	while (!comp->brk && conf.snd.fill && !finish && !conf.emu.pause) {
		// exec 1 opcode (or handle INT, NMI)
		if (conf.emu.pause) {
			sndNs += 1000;
		} else {
			if (brkskip) {
				brkskip = comp->debug;
				comp->debug = 1;		// block breakpoints checking
				tm = compExec(comp);
				comp->debug = brkskip;
				brkskip = 0;
			} else {
				tm = compExec(comp);			// TODO: it exits when fetch-brk is occured, pc doesn't changed
			}
			sndNs += tm;
			wavNs += tm;
			// tape trap	TODO: rework it as a system breakpoint
			int pc = cpu_get_pc(comp->cpu);
			if ((comp->hw->grp == HWG_ZX) && (comp->mem->map[0].type == MEM_ROM) && comp->rom && !comp->dos && !comp->ext) {
				if ((pc == 0x56c) || (pc == 0x5e7)) {	// load: ix:addr, de:len (0x580 ?) 56c/559
					tap_catch_load(comp);
				} else if (pc == 0x4d0) {				// save: ix:addr, de:len, a:block type(b7), hl:pilot len (1f80/0c98)?
					tap_catch_save(comp);
				}
				if (conf.tape.autostart && !conf.tape.fast && ((pc == 0x5df) || (pc == 0x53a))) {
					comp->tape->sigLen = 1e6;
					tapNextBlock(comp->tape);
					tapStop(comp->tape);
					emit tapeSignal(TW_STATE,TWS_STOP);
				}
			}
			// write wav sample
			if (wavNs > 22675) {		// ns per sample @ 44100Hz
				wavNs -= 22675;
				if (conf.snd.wavout)
					snd_wav_write();
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
// process noflic/scanlines (if !fast ???)
// buffers is already switches, bufimg - just painted (greyscale, if flag is set), scrimg - new
			if (!conf.emu.fast && (noflic > 0))
				scrMix(pscr, bufimg, bufSize, noflic / 100.0);

			// printf("s_frame\n");
			emit s_frame();
		}
#if LOG_OUTPUT
// ...
#endif
		if (comp->brk) {
			// printf("brkt = %i, brka = %X\n", comp->brkt, comp->brka);
			if (comp->brkt == -1) {			// irq or tmp
				conf.emu.pause |= PR_DEBUG;
				emit dbgRequest();
			} else {				// others
				xBrkPoint* ptr = brk_find(comp->brkt, comp->brka);
				if (ptr) {
					QString fnams;
					QFile file;
					// TODO: fetch break continues to repeat, comp->brk=0 is not enough?
					switch (ptr->action) {
						case BRK_ACT_COUNT:
							ptr->count++;
							comp->brk = 0;
							if (ptr->fetch) brkskip = 1;
							break;
						case BRK_ACT_SCR:
							fnams = QString(conf.scrShot.dir.c_str()).append(SLASH);
							fnams.append(QString("xpeccy_%0").arg(QTime::currentTime().toString("HHmmss_zzz")));	// TODO: counter-based name (1ms is not enough)
							tm = 0;
							do {
								file.setFileName(QString("%0_%1.scr").arg(fnams).arg(tm));
								tm++;
							} while (file.exists());
							file.open(QFile::WriteOnly);
							file.write((char*)(comp->mem->ramData + (5 << 14)), 0x1b00);
							file.close();
							comp->brk = 0;
							if (ptr->fetch) brkskip = 1;
							break;
						default:					// BRK_ACT_DBG
							conf.emu.pause |= PR_DEBUG;
							emit dbgRequest();
							break;
					}
				} else {				// breakpoint didn't found, but bit is set (?)
					comp->brk = 0;
				}
			}
		}
	};
	comp->brk = 0;
	comp->nmiRequest = 0;
}

void xThread::run() {
	Computer* comp;
#if LOG_OUTPUT
	file = fopen("/home/sam/cpu.log","wb");
#endif
	do {
#if !USEMUTEX
		sleepy = 1;
#endif
		comp = conf.prof.cur->zx;
#if HAVEZLIB
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
#if LOG_OUTPUT
	fclose(file);
#endif
	exit(0);
}
