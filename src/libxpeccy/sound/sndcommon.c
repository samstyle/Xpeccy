#include "sndcommon.h"

#include <stdlib.h>
#include <string.h>

// 1-bit channel with transient response

//#define OVERSHOOT 22500			// ns to overshoot process
#define OVERDIV 88		// ns/256 : post-compensation

bitChan* bcCreate() {
	bitChan* ch = malloc(sizeof(bitChan));
	memset(ch, 0x00, sizeof(bitChan));
	ch->on = 1;
	return ch;
}

void bcDestroy(bitChan* ch) {
	free(ch);
}

void bcSync(bitChan* ch, int ns) {
	int lev;
	if (ns < 1) {
		ns = ch->accum;
		ch->accum = 0;
	}
	if (ns < 1) return;
	if (ch->on) {
		lev = ch->val & 0xff;
		if (ch->lev) {
			lev += ns / OVERDIV;
			if (lev > 0xff)
				lev = 0xff;
		} else {
			lev -= ns / OVERDIV;
			if (lev < 0)
				lev = 0;
		}
		ch->val = lev & 0xff;
		if (ch->perH && ch->perL) {
			ch->pcount -= ns;
			while (ch->pcount < 1) {
				ch->lev = !ch->lev;
				ch->pcount += ch->lev ? ch->perH : ch->perL;
			}
		}
	}
}
