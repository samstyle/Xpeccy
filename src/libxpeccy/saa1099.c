#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "saa1099.h"

// 253 : pos=0 (repeat)
// 255 : pos-- (stay)
const unsigned char saaEnvForms[8][33] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},	// 000 : 0 stay
	{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,253},	// 001 : F repeat = F stay
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,255},	// 010 : down stay
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},	// 011 : down repeat
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 255},	// 100 : up-down stay
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 253},	// 101 : up-down repeat
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 255},	// 110 : up,0 stay
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},		// 111 : up repeat
};

saaChip* saaCreate() {
	saaChip* saa = (saaChip*)malloc(sizeof(saaChip));
	memset(saa, 0x00, sizeof(saaChip));
	return saa;
}

void saaDestroy(saaChip* saa) {
	if (saa) free(saa);
}

void saaReset(saaChip* saa) {
	int i;
	for (i = 0; i < 6; i++) {
		saa->chan[i].period = 0;
		saa->chan[i].count = 0;
	}
	for (i = 0; i < 2; i++) {
		saa->noiz[i].period = 0;
		saa->noiz[i].period = 0;
		saa->env[i].period = 0;
		saa->env[i].count = 0;
		saa->env[i].pos = 0;
	}
}

void saaSetNoise(saaNoise* nch, int val, saaChan* fch) {
	if ( val != 3 )
		nch->period = 0x8000 << val;
	else
		nch->period = (511 - fch->freq) << (14 - fch->octave);
}

void saaUpdateEnv(saaEnv* env, saaChan* ch) {
	env->buf.update = 0;
	env->count = 0;
	env->pos = 0;
	env->form = env->buf.form;
	env->invRight = env->buf.invRight;
	env->extCLK = env->buf.extCLK;
	if (!env->extCLK) {
		env->period = ch->period;
	} else {
		env->period = 0;
	}
}

void saaEnvStep(saaEnv* env, saaChan* ch) {
	env->pos++;
	if (env->lowRes) env->pos = (env->pos + 1) & ~1;
	switch (saaEnvForms[env->form][env->pos]) {
		case 255:			// non-cycled env end : can be updated immediately
			env->busy = 0;
			if (env->buf.update) saaUpdateEnv(env, ch);
			env->pos--;
			if (env->lowRes) env->pos &= ~1;
			break;
		case 253:			// cycled env : can be updated @ end of cycle
			if (env->buf.update) saaUpdateEnv(env, ch);
			env->pos = 0;
			break;
	}
	env->vol = saaEnvForms[env->form][env->pos];
//	if (env->lowRes) env->vol &= 0x0e;	// 3bit control (ORLY?)
}

int saaWrite(saaChip* saa, int adr, unsigned char val) {
	if ((adr & 0xff) != 0xff) return 0;
	int i, num;
	if (adr & 0x100) {
		saa->curReg = val & 0x1f;
// ORLY: external envelope clock (address write pulse)
		if (saa->env[0].extCLK && saa->env[0].enable) saaEnvStep(&saa->env[0], &saa->chan[1]);
		if (saa->env[1].extCLK && saa->env[1].enable) saaEnvStep(&saa->env[1], &saa->chan[4]);
	} else {
		switch (saa->curReg) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				saa->chan[saa->curReg].ampLeft = val & 0x0f;
				saa->chan[saa->curReg].ampRight = (val >> 4) & 0x0f;
				break;
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d:
				saa->chan[saa->curReg & 7].freq = val;
				break;
			case 0x10:
			case 0x11:
			case 0x12:
				num = (saa->curReg & 3) << 1;
				saa->chan[num].octave = val & 7;
				saa->chan[num + 1].octave = (val >> 4) & 7;
				break;
			case 0x14:
				for (i = 0; i < 6; i++) {
					saa->chan[i].freqEn = (val & (1 << i)) ? 1 : 0;
				}
				break;
			case 0x15:
				for (i = 0; i < 6; i++) {
					saa->chan[i].noizEn = (val & (1 << i)) ? 1 : 0;
				}
				break;
			case 0x16:
				saaSetNoise(&saa->noiz[0], val & 3, &saa->chan[0]);
				saaSetNoise(&saa->noiz[1], (val >> 4) & 3, &saa->chan[3]);
				break;
			case 0x18:
			case 0x19:
				num = saa->curReg & 1;
				saa->env[num].buf.invRight = (val & 1) ? 1 : 0;
				saa->env[num].buf.form = (val >> 1) & 7;
				saa->env[num].buf.extCLK = (val & 32) ? 1 : 0;
				saa->env[num].buf.update = 1;
				saa->env[num].lowRes = (val & 16) ? 1 : 0;
				if (!saa->env[num].busy) saaUpdateEnv(&saa->env[num], &saa->chan[num ? 4 : 1]);
				saa->env[num].enable = (val & 128) ? 1 : 0;
				saa->env[num].busy = saa->env[num].enable;
				break;
			case 0x1c:
//				if (val & 2) printf("saa out 1C,%X\n",val);
				saa->off = (val & 1) ? 0 : 1;
				if (val & 2) {
					for (i = 0; i < 6; i++) saa->chan[i].count = 0;
					saa->noiz[0].count = 0;
					saa->noiz[1].count = 0;
					saa->env[0].count = 0;
					saa->env[1].count = 0;
				}
				break;
		}
		for (i = 0; i < 6; i++) {
			saa->chan[i].period = (511 - saa->chan[i].freq) << (15 - saa->chan[i].octave);
		}
	}
	return 1;
}

void saaSync(saaChip* saa, int ns) {
	if (!saa->enabled) return;
	int i;
	saaNoise* noiz;
	saaChan* cha;
	for (i = 0; i < 6; i++) {
		cha = &saa->chan[i];
		if (cha->period != 0) {
			cha->count += ns;
			while (cha->count > 0) {
				cha->count -= cha->period;
				cha->lev ^= 1;
				
				if ( i == 1 && !saa->env[0].extCLK )
					saaEnvStep(&saa->env[0], cha);
				else if ( i == 4  && !saa->env[1].extCLK )
					saaEnvStep(&saa->env[1], cha);
			}
		}
	}
	for (i = 0; i < 2; i++) {
		noiz = &saa->noiz[i];
		if (noiz->period != 0) {
			noiz->count += ns;
			while (noiz->count > 0) {
				noiz->count -= noiz->period;
				noiz->pos++;
			}
		}
	}
}

sndPair saaMixTN(saaChan* ch, saaNoise* noiz) {
	sndPair res;
	int signal = 1;
	if ( (ch->freqEn && ch->lev) || (ch->noizEn && noiz->lev) ) {
		if ( (ch->freqEn && ch->lev) && (ch->noizEn && noiz->lev) )
			signal = 2;
		else
			signal = 0;
	}
	if ( signal ) {
		res.left = ch->ampLeft / signal;
		res.right = ch->ampRight / signal;
	} else {
		res.left = 0;
		res.right = 0;
	}
	return res;
}

sndPair saaMixTNE(saaChan* ch, saaNoise* noiz, saaEnv* env) {
	if (!env->enable) return saaMixTN(ch, noiz);
	sndPair res;
	int signal = 1;
	if ( (ch->freqEn && ch->lev) || (ch->noizEn && noiz->lev) ) {
		if ( (ch->freqEn && ch->lev) && (ch->noizEn && noiz->lev) )
			signal = 2;
		else
			signal = 0;
	}
	if ( signal ) {
		res.left = ch->ampLeft * 7 * env->vol / 15 / 8 / signal;
		res.right = ch->ampRight * 7 * ((env->invRight) ? (env->vol ^ 0x0f) : env->vol) / 15 / 8 / signal;
	} else {
		res.left = 0;
		res.right = 0;
	}
	return res;
}


sndPair saaGetVolume(saaChip* saa) {
	sndPair res;
	int levl = 0;
	int levr = 0;
	if (!saa->off && saa->enabled) {
		saa->noiz[0].lev = noizes[saa->noiz[0].pos & 0x1ffff] ? 1 : 0;
		saa->noiz[1].lev = noizes[saa->noiz[1].pos & 0x1ffff] ? 1 : 0;
		for (int i = 0; i < 6; i++) {
			switch (i) {
				case 0:
				case 1: res = saaMixTN(&saa->chan[i], &saa->noiz[0]); break;
				case 2:	res = saaMixTNE(&saa->chan[i], &saa->noiz[0], &saa->env[0]); break;
				case 3:
				case 4:	res = saaMixTN(&saa->chan[i], &saa->noiz[1]); break;
				case 5:	res = saaMixTNE(&saa->chan[i], &saa->noiz[1], &saa->env[1]); break;
			}
			levl += res.left;
			levr += res.right;
		}
	}
	if (saa->mono) {
		levl = (levl + levr) / 2;
		levr = levl;
	}
	res.left = levl;
	res.right = levr;
	return res;
}
