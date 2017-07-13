#ifndef _NESAPU_H
#define _NESAPU_H

typedef struct {
	unsigned lev:1;
	unsigned char vol;
	int per0;		// @192Hz ticks for level 0
	int per1;		// same for level 1
	unsigned sweep:1;	// sweep enabled
	unsigned sweepDir:1;	// 0:dec 1:inc
	int sweepPer;		// sweep period @192Hz ticks
	int sweepTick;
	int sweepDeg;		// sweep shift amount (1-7)
	int counter;		// counter @192Hz ticks to for invert level;
} nesPulseChan;

typedef struct {
	nesPulseChan ch0;
	nesPulseChan ch1;
	int tick;		// PAL 192Hz ticks = 1e9/192 ns per tick; NOTE: 240Hz @ NTSC
} nesAPU;

nesAPU* apuCreate();
void apuDestroy(nesAPU*);
void apuSync(nesAPU*, int);

#endif
