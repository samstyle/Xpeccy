#ifndef X_NESAPU_H
#define X_NESAPU_H

#include "sndcommon.h"

typedef unsigned char(*extmrd)(unsigned short, void*);

typedef struct {
	unsigned off:1;		// external on/off channel volume. sound generation doesn't stop
	unsigned en:1;		// channel enabled
	unsigned lev:1;		// square signal level
	unsigned env:1;		// envelope on
	unsigned elen:1;	// length counter enabled / envelope looped
	unsigned duty:2;
	unsigned sweep:1;	// sweep working
	unsigned sdir:1;	// 0:add to period, 1:sub from period
	unsigned dir:1;		// triangle wave direction (1:up 0:down)
	unsigned irq:1;		// dmc irq

	unsigned char vol;	// current volume;
	unsigned char buf;	// readed byte (digital)
	int len;		// length counter
	int lcnt;		// linear counter (triangle)
	int hper;		// 50/50 period
	int per0;		// duty period for 0
	int per1;		// duty period for 1
	int pcnt;		// period counter
	int pstp;		// waves counter
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

	int tper;		// 240Hz period
	int tcnt;
	int tstp;

	apuChannel ch0;		// square 0
	apuChannel ch1;		// square 1
	apuChannel cht;		// triangle
	apuChannel chn;		// noise
	apuChannel chd;		// digital

	int time;

	extmrd mrd;
	void* data;
} nesAPU;

nesAPU* apuCreate(extmrd, void*);
void apuDestroy(nesAPU*);
void apuSync(nesAPU*, int);
sndPair apuVolume(nesAPU*);

void apuToneDuty(apuChannel*);

#endif
