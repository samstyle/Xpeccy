#include "sndcommon.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
// mixer
#define XMAXVOL 64

sndPair mixer(sndPair cur, int levL, int levR, int vol) {

	levL = levL * vol / 100;
	levR = levR * vol / 100;

	// max (64 + 64) * 64 / (64 + 64) = 64
	// min (0 + 0) * 64 / (64 + 0) = 0
	// one (64 + 0) * 64 / (64 + 0) = 64
	// mix (32 + 32) * 64 / (64 + (32 * 32) / 64) = 51
	cur.left = (cur.left + levL) * XMAXVOL / (XMAXVOL + (cur.left * levL) / XMAXVOL);
	cur.right = (cur.right + levR) * XMAXVOL / (XMAXVOL + (cur.right * levR) / XMAXVOL);
	return cur;
}
*/

// 1-bit channel with transient response

#define OVERDIV 88			// ns/256 : transient const (ns to rise/lower sound level 1 step)
#define OVERLIM (OVERDIV * 256)		// ns to full sound level restore

bitChan* bcCreate() {
	bitChan* ch = malloc(sizeof(bitChan));
	memset(ch, 0x00, sizeof(bitChan));
	return ch;
}

void bcDestroy(bitChan* ch) {
	free(ch);
}

void bcTransient(bitChan* ch, int ns) {
	if (ch->lev) {
		if (ns > OVERLIM) {
			ch->val = 0xff;
		} else {
			ch->val += ns / OVERDIV;
			if (ch->val > 0xff)
				ch->val = 0xff;
		}
	} else {
		if (ns > OVERLIM) {
			ch->val = 0;
		} else {
			ch->val -= ns / OVERDIV;
			if (ch->val < 0)
				ch->val = 0;
		}
	}
}

void bcSync(bitChan* ch, int ns) {
	int per;
	if (ns < 1) {
		ns = ch->accum;
		ch->accum = 0;
	}
	if (ns < 1) return;

	bcTransient(ch, ns);		// transient process of current wave
	if (ch->perH && ch->perL) {	// emulate waves
		// printf("%i %i %i\n",ch->pcount, ch->perH, ch->perL);
		ch->pcount -= ns;
		while (ch->pcount < 1) {
			ch->lev ^= 1;
			ch->step++;
			per = ch->lev ? ch->perH : ch->perL;
			ch->pcount += per;
			if (ch->pcount > 0) per -= ch->pcount;
			bcTransient(ch, per);
		}
	}
}
