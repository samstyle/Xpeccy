#include "nesapu.h"

#include <stdlib.h>
#include <string.h>

extern char noizes[0x20000];

nesAPU* apuCreate() {
	nesAPU* apu = (nesAPU*)malloc(sizeof(nesAPU));
	memset(apu, 0x00, sizeof(nesAPU));
	return apu;
}

void apuDestroy(nesAPU* apu) {
	if (apu) free(apu);
}

// pulse channels (0,1)
void apuPulseTick(nesapuChan* ch) {
	if (!ch->en) return;
	ch->tcount++;
	if (!ch->len) return;
	if (ch->pcount > 0) {
		ch->pcount--;
		if (ch->pcount == 0) {
			ch->lev ^= 1;
			ch->pcount = ch->lev ? ch->per1 : ch->per0;
		}
	}
	if (!(ch->tcount & 3) && ch->lenen) {		// decrease len counter tick:4
		ch->len--;
	}
}

// triangle channel (2)

static int triVolume[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};

void apuTriangleTick(nesapuChan* ch) {
	if (!ch->en) return;
	ch->tcount++;
	if (ch->len) {
		if (ch->pcount > 0) {
			ch->pcount--;
			if (ch->pcount == 0) {
				ch->pcount = ch->per;		// triangle:no decay
				ch->step++;
				ch->vol = triVolume[ch->step & 0x1f];
			}
		}
		// TODO: triangle channel have linear counter
		if (!(ch->tcount & 3) && ch->lenen) {		// length counter
			ch->len--;
		}
	} else {
		ch->vol = 0;
	}
}

// noise channel (3)

void apuNoiseTick(nesapuChan* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	ch->tcount++;
	if (ch->pcount > 0) {
		ch->pcount--;
		if (ch->pcount == 0) {
			ch->pcount = ch->per;		// noise:no decay
			ch->step++;
			ch->lev = noizes[ch->step & 0x1fff] ? 1 : 0;
		}
	}
	if (!(ch->tcount & 3) && ch->lenen) {		// length counter
		ch->len--;
	}
}

void apuSync(nesAPU* apu, int ns) {
	apu->tick -= ns;
	if (apu->tick > 0) return;
	apu->tick += apu->period;					// add period
	apu->tcount++;
	if (apu->tcount > 4) apu->tcount = 0;				// 0 to 4 in cycle
	if ((apu->tcount == 3) && apu->inten) apu->frm = 1;		// frame
	if ((apu->tcount == 4) && apu->pal) return;			// PAL: skip 1 of 5 ticks
	apuPulseTick(&apu->ch0);
	apuPulseTick(&apu->ch1);
	apuTriangleTick(&apu->ch2);
	apuNoiseTick(&apu->ch3);
}

void apuChReset(nesapuChan* ch) {
	ch->en = 1;
	ch->len = 0;
	ch->per = 0;
	ch->pcount = 0;
}

void apuReset(nesAPU* apu) {
	apuChReset(&apu->ch0);
	apuChReset(&apu->ch1);
	apuChReset(&apu->ch2);
	apuChReset(&apu->ch3);
	apuChReset(&apu->ch4);
}

sndPair apuVolume(nesAPU* apu) {
	sndPair res;
	res.left = 0;
	res.right = 0;
	int vol = apu->ch0.lev ? apu->ch0.vol : 0;
	res = mixer(res, vol, vol, 100);
	vol = apu->ch1.lev ? apu->ch1.vol : 0;
	res = mixer(res, vol, vol, 100);
	vol = apu->ch2.vol;
	res = mixer(res, vol, vol, 100);
	vol = apu->ch3.lev ? apu->ch3.vol : 0;
	res = mixer(res, vol, vol, 100);

	res.left <<= 4;
	res.right <<= 4;

	return res;
}
