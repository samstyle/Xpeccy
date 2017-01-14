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
	unsigned char val;
	long accum;			// ns accumulator
	int period;			// period value, ns
	int pcount;			// current period counter
} bitChan;

bitChan* bcCreate();
void bcDestroy(bitChan*);
void bcSync(bitChan*, int);

#endif
