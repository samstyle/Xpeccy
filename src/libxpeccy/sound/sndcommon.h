#ifndef _SNDCOMMON_H
#define _SNDCOMMON_H

typedef struct {
	unsigned char left;
	unsigned char right;
} sndPair;

extern int noizes[0x20000];

typedef struct {
	unsigned on:1;			// on/off sound output
	unsigned lev:1;			// 1/0 target level

	unsigned left:1;		// stereo output control
	unsigned right:1;

	unsigned char val;
	long accum;			// ns accumulator
	int perH;			// halfperiod for lev=1
	int perL;			// halfperiod for lev=0. gameboy has different halfperiod sizes
	int pcount;			// current halfperiod counter
} bitChan;

bitChan* bcCreate();
void bcDestroy(bitChan*);
void bcSync(bitChan*, int);

#endif
