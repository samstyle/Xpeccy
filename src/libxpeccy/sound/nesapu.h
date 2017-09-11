#ifndef X_NESAPU_H
#define X_NESAPU_H

#include "sndcommon.h"

typedef unsigned char(*aextmrd)(unsigned short, void*);

typedef struct {
	unsigned off:1;		// external on/off channel volume. sound generation doesn't stop
	unsigned en:1;		// channel enabled
	unsigned lev:1;		// square signal level
	unsigned env:1;		// envelope on
	unsigned mute:1;	// sweep unit has muted this channel
	unsigned elen:1;	// length counter enabled / envelope looped
	unsigned sweep:1;	// sweep working | TriChan linear counter reload flag
	unsigned sdir:1;	// 0:add to period, 1:sub from period
	unsigned dir:1;		// triangle wave direction (1:up 0:down)
	unsigned irq:1;		// dmc irq
	unsigned mode:1;	// noise long/short generator

	unsigned duty:2;
	unsigned nseed:15;	// noise seed

	unsigned char vol;	// current volume	(generated regardless of OFF & EN flags)
	unsigned char evol;	// envelope volume
	unsigned char out;	// output volume;	(respect OFF & EN flags and update from VOL if enabled)
	unsigned char buf;	// readed byte (digital)
	int len;		// length counter
	int lcnt;		// linear counter (triangle)
	int lval;		// linear counter reload value
	int hper;		// period div
	int pcnt;		// period div counter
	int pstp;		// waves counter
	int tper;		// target period (sweep)
	int eper;		// envelope period
	int ecnt;		// envelope counter;
	int sper;		// sweep period
	int scnt;		// sweep counter
	int sshi;		// sweep shift
	int sdiv;		// sweep divider
	unsigned short sadr;	// sample address (digital)
	unsigned short cadr;	// next fetch address (digital)
} apuChannel;

typedef struct {
	unsigned step5:1;
	unsigned irqen:1;	// enable frame irq
	unsigned firq:1;	// frame irq
	unsigned dirq:1;	// dmc irq

	int wper;		// wave tick = CPU/16
	int wcnt;
	int wstp;
	int wdiv;
	int tstp;

	apuChannel ch0;		// square 0
	apuChannel ch1;		// square 1
	apuChannel cht;		// triangle
	apuChannel chn;		// noise
	apuChannel chd;		// digital

	int time;

    aextmrd mrd;
	void* data;
} nesAPU;

nesAPU* apuCreate(aextmrd, void*);
void apuDestroy(nesAPU*);

void apuReset(nesAPU*);
void apuSync(nesAPU*, int);
sndPair apuVolume(nesAPU*);

void apuWrite(nesAPU*, int, unsigned char);

// void apuToneDuty(apuChannel*);

#endif
