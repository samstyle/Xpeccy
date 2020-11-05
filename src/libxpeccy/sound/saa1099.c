#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "saa1099.h"

#define TSTEP 256
#define USE_TONE_ENV 1

// xx0 : stay
// xx1 : repeat
static unsigned char saaEnvForms[8][33] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 253},		// 000 : 0 stay
	{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15, 255},		// 001 : F repeat = F stay
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 253},		// 010 : down stay
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 255},		// 011 : down repeat
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 253},	// 100 : up-down stay
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 255},	// 101 : up-down repeat
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 253},		// 110 : up,0 stay
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 255},		// 111 : up repeat
};

saaChip* saaCreate() {
	saaChip* saa = (saaChip*)malloc(sizeof(saaChip));
	memset(saa, 0x00, sizeof(saaChip));
	return saa;
}

void saaDestroy(saaChip* saa) {
	if (saa == NULL) return;
	free(saa);
}

void saaReset(saaChip* saa) {
	int i;
	for (i = 0; i < 6; i++) {
		saa->chan[i].freqEn = 0;
		saa->chan[i].noizEn = 0;
		saa->chan[i].period = 0;
		saa->chan[i].count = 0;
	}
	for (i = 0; i < 2; i++) {
		saa->noiz[i].period = 0;
		saa->env[i].period = 0;
		saa->env[i].form = 0;
		saa->env[i].count = 0;
		saa->env[i].pos = 0;
		saa->env[i].enable = 0;
	}
}

// 0 : 0x100 ticks = 31250 KHz
// 1 : 0x200 ticks = 15625 KHz
// 2 : 0x400 ticks = 7812.5KHz
// 3 : channel period

void saaSetNoise(saaNoise* nch, int val, saaChan* fch) {
	if ( val != 3 )
		nch->period = 0x100 << val;
	else
		nch->period = fch->period;
}

void saaUpdateEnv(saaEnv* env) {
	env->buf.update = 0;
	env->count = 0;
	env->pos = 0;
	env->form = env->buf.form;
	env->invRight = env->buf.invRight;
	env->extCLK = env->buf.extCLK;
	env->period = env->buf.period;
}

void saaEnvStep(saaEnv* env) {
	env->pos++;
	switch (saaEnvForms[env->form][env->pos]) {
		case 253:
			env->pos--;
			if (env->buf.update)
				saaUpdateEnv(env);
			break;
		case 255:			// return to start of cycle
			env->pos = 0;
			env->vol = saaEnvForms[env->form][env->pos | (env->lowRes ? 1 : 0)];
			if (env->buf.update)
				saaUpdateEnv(env);
		default:
			env->vol = saaEnvForms[env->form][env->pos | (env->lowRes ? 1 : 0)];
			break;
	}
}

// input CLK: 8MHz (125ns/T)
//		doc.min	doc.max		T		T
// oct	0	31Hz	61Hz		0x40000 (30.51)	0x20000
//	1	61Hz	122Hz		0x20000		0x10000
//	2	122Hz	244Hz		0x10000		0x8000
//	3	244Hz	488Hz		0x8000		0x4000
//	4	488Hz	976Hz		0x4000		0x2000
//	5	976Hz	1952Hz		0x2000		0x1000
//	6	1952Hz	3904Hz		0x1000		0x800
//	7	3904hz	7808Hz		0x800		0x400 (7812.5Hz)
// tone: 0x00..0xFF
// (512 - tone) : 0x200..0x100
// (512 - tone) << (9 - oct) : 0x800..0x400 (7 oct), 0x1000..0x800 (6 oct), ticks

int saaWrite(saaChip* saa, int adr, int data) {
	int i, num;
	if ((adr & 0xff) != 0xff) return 0;
//	saaFlush(saa);
	if (adr & 0x100) {
		saa->curReg = data & 0x1f;
// ORLY: external envelope clock (address write pulse)
		if (saa->env[0].extCLK && saa->env[0].enable)
			saaEnvStep(&saa->env[0]);
		if (saa->env[1].extCLK && saa->env[1].enable)
			saaEnvStep(&saa->env[1]);
	} else {
		switch (saa->curReg) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				saa->chan[saa->curReg].ampLeft = data & 0x0f;
				saa->chan[saa->curReg].ampRight = (data >> 4) & 0x0f;
				break;
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d:
				saa->chan[saa->curReg & 7].freq = data;
				break;
			case 0x10:
			case 0x11:
			case 0x12:
				num = (saa->curReg & 3) << 1;
				saa->chan[num].octave = data & 7;
				saa->chan[num + 1].octave = (data >> 4) & 7;
				break;
			case 0x14:
				for (i = 0; i < 6; i++) {
					saa->chan[i].freqEn = (data & (1 << i)) ? 1 : 0;
				}
				break;
			case 0x15:
				for (i = 0; i < 6; i++) {
					saa->chan[i].noizEn = (data & (1 << i)) ? 1 : 0;
				}
				break;
			case 0x16:
				saaSetNoise(&saa->noiz[0], data & 3, &saa->chan[0]);
				saaSetNoise(&saa->noiz[1], (data >> 4) & 3, &saa->chan[3]);
				break;
			case 0x18:
			case 0x19:
				num = saa->curReg & 1;
				saa->env[num].buf.invRight = (data & 1) ? 1 : 0;
				saa->env[num].buf.form = (data >> 1) & 7;
				saa->env[num].buf.extCLK = (data & 0x20) ? 1 : 0;
				saa->env[num].buf.update = 1;
				saa->env[num].buf.period = saa->chan[num ? 4 : 1].period >> 1;
				saa->env[num].lowRes = (data & 0x10) ? 1 : 0;
				saa->env[num].enable = (data & 0x80) ? 1 : 0;
				break;
			case 0x1c:
				saa->off = (data & 1) ? 0 : 1;
				if (data & 2) {
					for (i = 0; i < 6; i++)
						saa->chan[i].count = 0;
					saa->noiz[0].count = 0;
					saa->noiz[1].count = 0;
					saa->env[0].count = 0;
					saa->env[1].count = 0;
				}
				break;
		}
		for (i = 0; i < 6; i++) {
			saa->chan[i].period = (512 - saa->chan[i].freq) << (8 - saa->chan[i].octave);		// in ticks @ 8MHz
		}
	}
	return 1;
}

// every 1T @ 8MHz, period is calculated in ticks
void saa_tone_tick(saaChan* cha, saaEnv* env) {
	if (cha->period < 1) return;
	cha->count += TSTEP;
	if (cha->count >= cha->period) {
		cha->count -= cha->period;
		cha->lev ^= 1;
		if (env != NULL) {			// envelope step on half tone period
			if (!env->extCLK)
				saaEnvStep(env);
		}
	}
}

void saa_noiz_tick(saaNoise* cha) {
	cha->count += TSTEP;
	if (cha->count >= cha->period) {
		cha->count -= cha->period;
		cha->pos++;
	}
}

void saaSync(saaChip* saa, int ns) {
	if (!saa->enabled) return;
	saa->time += ns;
	while (saa->time > 0) {
		saa->time -= 125 * TSTEP;			// 125ns/T @ CLK 8MHz
		saa_tone_tick(&saa->chan[0], NULL);
		saa_tone_tick(&saa->chan[1], &saa->env[0]);	// env.frq.generator taken from FG1 & FG4, but mixed with CH2 & CH5
		saa_tone_tick(&saa->chan[2], NULL);
		saa_tone_tick(&saa->chan[3], NULL);
		saa_tone_tick(&saa->chan[4], &saa->env[1]);
		saa_tone_tick(&saa->chan[5], NULL);
		saa_noiz_tick(&saa->noiz[0]);
		saa_noiz_tick(&saa->noiz[1]);
	}
}

sndPair saaMixTNE(saaChan* ch, saaNoise* noiz, saaEnv* env) {
	sndPair res;
	if ((ch->freqEn && !ch->lev) || (ch->noizEn && !noiz->lev)) {
		res.left = 0;
		res.right = 0;
	} else {
		res.left = ch->ampLeft;
		res.right = ch->ampRight;
	}
	if (env == NULL) {		// channel without envelope
		res.left <<= 4;
		res.right <<= 4;
	} else if (env->enable) {	// envelope enabled
		res.left *= env->vol;
		res.right *= env->invRight ? env->vol ^ 0x0f : env->vol;
		res.left &= 0xe0;
		res.right &= 0xe0;
	} else {			// envelope disabled
		res.left <<= 4;
		res.right <<= 4;
	}
	return res;
}

sndPair saaVolume(saaChip* ptr) {
	saaChip* saa = (saaChip*)ptr;
	sndPair res;
	int levl = 0;
	int levr = 0;
	if (!saa->off && saa->enabled) {
		saa->noiz[0].lev = noizes[saa->noiz[0].pos & 0x1ffff] ? 1 : 0;
		saa->noiz[1].lev = noizes[saa->noiz[1].pos & 0x1ffff] ? 1 : 0;
		for (int i = 0; i < 6; i++) {
			switch (i) {
				case 0:
				case 1: res = saaMixTNE(&saa->chan[i], &saa->noiz[0], NULL); break;
				case 2:	res = saaMixTNE(&saa->chan[i], &saa->noiz[0], &saa->env[0]); break;
				case 3:
				case 4:	res = saaMixTNE(&saa->chan[i], &saa->noiz[1], NULL); break;
				case 5:	res = saaMixTNE(&saa->chan[i], &saa->noiz[1], &saa->env[1]); break;
			}
			levl += res.left;
			levr += res.right;
		}
	}
	res.left = levl << 4;
	res.right = levr << 4;
	return res;
}
