#ifndef X_SNDCOMMON_H
#define X_SNDCOMMON_H

typedef  struct {
	int master;
	int beep;
	int tape;
	int ay;
	int gs;
	int sdrv;
	int saa;
} sndVolume;

typedef struct {
	unsigned int left;
	unsigned int right;
} sndPair;

extern char noizes[0x20000];

typedef struct {
	unsigned lev:1;			// 1/0 target level
	int val;			// current sound level (0..255)
//	long accum;			// ns accumulator
	int step;			// halfperiod counter
	unsigned int perH;		// halfperiod for lev=1
	unsigned int perL;		// halfperiod for lev=0. gameboy has different halfperiod lenght
	int pcount;			// current halfperiod counter
} bitChan;

bitChan* bcCreate();
void bcDestroy(bitChan*);
void bcSync(bitChan*, int);

// sndPair mixer(sndPair, int, int, int);

#endif
