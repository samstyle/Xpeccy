#include "gbsound.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char gbEnv[16] = {0,2,4,6,8,9,10,11,12,12,13,13,14,14,15,15};

gbSound* gbsCreate() {
	gbSound* gbs = malloc(sizeof(gbSound));
	if (!gbs) return NULL;
	memset(gbs, 0x00, sizeof(gbSound));
	return gbs;
}

void gbsDestroy(gbSound* gbs) {
	if (!gbs) return;
	free(gbs);
}

void gbchSync(gbsChan* ch, long tick) {	// input ticks @ 128KHz
	if (!ch->on) return;
	ch->cnt--;
	if (ch->cnt < 0) {
		ch->lev ^= 1;
		ch->cnt = ch->lev ? ch->perH : ch->perL;
		ch->step++;
	}
	if (tick & 0x1ff) return;	// 256Hz (len)
	if (!ch->cont) {
		ch->dur--;
		if (ch->dur < 0)
			ch->on = 0;
	}
	if (tick & 0x3ff) return;	// 128Hz (sweep)
	if (ch->sweep.step) {		// step = 0 : sweep off
		ch->sweep.cnt--;
		if (ch->sweep.cnt < 0) {
			ch->sweep.cnt = ch->sweep.per;
			if (ch->sweep.dir) {
				ch->perH -= ch->perH / (2 ^ ch->sweep.step);
				ch->perL -= ch->perL / (2 ^ ch->sweep.step);
			} else {
				ch->perH += ch->perH / (2 ^ ch->sweep.step);
				ch->perL += ch->perL / (2 ^ ch->sweep.step);
			}
		}
	}
	if (tick & 0x7ff) return;	// 64Hz (env)
	ch->env.cnt--;
	if (ch->env.cnt < 0) {
		ch->env.cnt = ch->env.per;
		if (ch->env.on) {
			if (ch->env.dir) {
				if (ch->env.vol < 15)
					ch->env.vol++;
			} else {
				if (ch->env.vol > 0)
					ch->env.vol--;
			}
		}
	}
}

void gbsSync(gbSound* gbs, int ns) {
	if (gbs->wav.period == 0) return;
	gbs->wav.count -= ns;
	while (gbs->wav.count < 0) {
		gbs->wav.tick++;
		gbs->wav.count += gbs->wav.period;
		gbchSync(&gbs->ch1, gbs->wav.tick);
		gbchSync(&gbs->ch2, gbs->wav.tick);
		gbchSync(&gbs->ch3, gbs->wav.tick);
		gbchSync(&gbs->ch4, gbs->wav.tick);
	}
}

sndPair gbsVolume(gbSound* gbs) {
	int left = 0;
	int right = 0;
	int lev;

	if (gbs->on) {
		// ch 1 : tone, env, sweep
		lev = gbs->ch1.lev ? 0xff : 0x00;		// 00.FF
		lev *= gbEnv[gbs->ch1.env.vol & 15];		// 00.FF0
		lev >>= 4;					// 00.FF
		if (gbs->ch1.so1) left += lev;
		if (gbs->ch1.so2) right += lev;
		// ch 2 : tone, env
		lev = gbs->ch2.lev ? 0xff : 0x00;
		lev *= gbEnv[gbs->ch2.env.vol & 15];
		lev >>= 4;
		if (gbs->ch2.so1) left += lev;			// 200
		if (gbs->ch2.so2) right += lev;
		// ch 3 : waveform
		lev = gbs->wave[gbs->ch3.step & 0x1f];
		switch(gbs->ch3vol & 3) {
			case 0: lev = 0; break;
			case 1: break;
			case 2: lev >>= 1; break;
			case 3: lev >>= 2; break;
		}
		if (gbs->ch3.so1) left += lev;			// 300
		if (gbs->ch3.so2) right += lev;
		// ch 4 : noise, env
		lev = noizes[gbs->ch4.step & 0x1ffff] ? 0x80 : 0x00;
		lev *= gbEnv[gbs->ch4.env.vol & 15];
		lev >>= 4;
		if (gbs->ch4.so1) left += lev;			// 400
		if (gbs->ch4.so2) right += lev;
		// mix
		left <<= 4;
		right <<= 4;
	}
	sndPair res;
	res.left = left;
	res.right = right;
	return res;
}
