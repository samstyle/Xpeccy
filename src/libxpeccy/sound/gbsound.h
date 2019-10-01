#ifndef _GBSOUND_H
#define _GBSOUND_H

#include "sndcommon.h"

typedef struct {
	unsigned on:1;
	unsigned cont:1;
	unsigned so1:1;
	unsigned so2:1;

	unsigned lev:1;
	int perH;		// 128KHz ticks to change level
	int perL;
	int per;
	int cnt;		// countdown
	int step;

	int dur;			// sound duration (cont = 0), 256Hz ticks

	struct {
		unsigned on:1;
		unsigned dir:1;		// 0:down, 1:up
		unsigned char vol;
		int per;		// 64Hz ticks to step
		int cnt;		// countdown
	} env;
	struct {
		unsigned dir:1;		// 0:frq down, 1:frq up
		int step;		// 1..7 = change period for ( 1/(2^n) of current )
		int per;		// 128Hz ticks to step
		int cnt;		// countdown
	} sweep;
} gbsChan;

typedef struct {
	long period;
	long count;
	int tick;
} fSeq;

typedef struct {
	unsigned on:1;
	fSeq wav;	// waveform generator @ 128KHz	len:/512 sweep:/1024 env:/2048
	gbsChan ch1;
	gbsChan ch2;
	gbsChan ch3;
	gbsChan ch4;
	unsigned ch3on:1;	// master on/off ch3
	int ch3vol;		// ch3 output level (0,1,2,3) = (0,100%,50%,25%)
	unsigned char wave[32];	// ch3 wave data
} gbSound;

gbSound* gbsCreate();
void gbsDestroy(gbSound*);
void gbsSync(gbSound*, int);
sndPair gbsVolume(gbSound*);

#endif
