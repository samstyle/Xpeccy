#include <SDL.h>
#include <QElapsedTimer>

#include "pacing.h"
#include "sound.h"
#include "xcore.h"

#if USEMUTEX
#include <QMutex>
#include <QWaitCondition>
extern QMutex emutex;
extern QWaitCondition qwc;
#else
extern int sleepy;
#endif

// wake period in ms, independent of the audio buffer size
#define PACE_TICK_MS 2

// if the audio ring buffer is fuller than this (in bytes), do not add
// samples this tick, so we do not run ahead of real playback
#define RING_HIGH_WATER (0x3fff * 3 / 4)

static SDL_TimerID paceTimerId = 0;
static QElapsedTimer paceClock;
static qint64 paceLastNs = 0;
static qint64 paceRemainderNs = 0;

static Uint32 pace_tick(Uint32 interval, void*) {
	qint64 nowNs = paceClock.nsecsElapsed();
	qint64 dtNs = nowNs - paceLastNs + paceRemainderNs;
	paceLastNs = nowNs;

	if (!conf.emu.fast && !conf.emu.pause) {
		// samples for the elapsed time; keep the remainder to avoid drift
		qint64 samples = (dtNs * conf.snd.rate) / 1000000000LL;
		paceRemainderNs = dtNs - (samples * 1000000000LL) / conf.snd.rate;
		if ((samples > 0) && (sndGetRingDistance() < RING_HIGH_WATER)) {
			conf.snd.need += (int)samples;
		}
	} else {
		conf.snd.need = 0;
		paceRemainderNs = 0;
	}

#if USEMUTEX
	qwc.wakeAll();
#else
	sleepy = 0;
#endif
	return interval;
}

void pacingInit() {
	paceClock.start();
	paceLastNs = paceClock.nsecsElapsed();
	paceRemainderNs = 0;
	paceTimerId = SDL_AddTimer(PACE_TICK_MS, pace_tick, NULL);
}

void pacingClose() {
	if (paceTimerId) {
		SDL_RemoveTimer(paceTimerId);
		paceTimerId = 0;
	}
}
