#include <stdlib.h>
#include <string.h>

#include "ayym.h"

// tables

char noizes[0x20000];		// here iz noize values 1/0 [generated at start] TODO:obsolete, rewrite parts using it

#if 0
int ayDACvol[32] = {0x0000,0x0000,0x001D,0x003A,0x0052,0x0065,0x007D,0x009A,
			   0x00C2,0x00F0,0x0122,0x0146,0x0186,0x01D6,0x0226,0x0274,
			   0x02EA,0x037E,0x041C,0x04B2,0x0594,0x06AF,0x07C8,0x08EB,
			   0x0AA0,0x0CC4,0x0EE6,0x1108,0x143A,0x1820,0x1C14,0x1FFF};
#elif 1
int ayDACvol[32] = {0x0000,0x0000,0x003B,0x0074,0x00A4,0x00CA,0x00FB,0x0134,
			   0x0184,0x01E0,0x0244,0x028D,0x030C,0x03AD,0x044C,0x04E8,
			   0x05D4,0x06FD,0x0838,0x0965,0x0B28,0x0D5F,0x0F91,0x11D7,
			   0x1540,0x1988,0x1DCC,0x2211,0x2874,0x3040,0x3828,0x3FFF};
#endif

// AY/YM sound chip common

void sc_dum_res(aymChip* chip) {}
int sc_dum_rd(aymChip* chip, int adr) {return -1;}
void sc_dum_wr(aymChip* chip, int adr, int data) {}
void sc_dum_sync(aymChip* chip, int ns) {}
sndPair sc_dum_vol(aymChip* chip) {sndPair vol = {0,0}; return vol;}

static const scDesc snd_chip_tab[] = {
	{SND_AY, "AY-3-8910", "AY8910", 1.7744, ay_reset, ay_rd, ay_wr, ay_sync, ay_vol},
	{SND_YM, "Yamaha-2149", "YM2149", 1.75, ay_reset, ym_rd, ym_wr, ay_sync, ym_vol},
	{SND_NONE, "Dummy", "NULL", 1.0, sc_dum_res, sc_dum_rd, sc_dum_wr, sc_dum_sync, sc_dum_vol}
};

const scDesc* find_chip_type(int id) {
	int i = 0;
	while((snd_chip_tab[i].id != SND_NONE) && (snd_chip_tab[i].id != id))
		i++;
	return &snd_chip_tab[i];
}

void chip_set_type(aymChip* chip, int id) {
	const scDesc* dsc = find_chip_type(id);
	chip->type = dsc->id;
	chip->res = dsc->res;
	chip->rd = dsc->rd;
	chip->wr = dsc->wr;
	chip->sync = dsc->sync;
	chip->vol = dsc->vol;
	if (chip->frq == 0)
		chip->frq = dsc->frq;
	if (chip->frq == 0)
		chip->frq = 1.0;
	chip->per = 500 / chip->frq;		// 1000/frq = full period, 500/frq = half-period
	chip->cnt = chip->per;
}

aymChip* aymCreate(int tp) {
	aymChip* chip = (aymChip*)malloc(sizeof(aymChip));
	memset(chip, 0x00, sizeof(aymChip));
	chip_set_type(chip, tp); // aymSetType(ay,tp);
	chip->stereo = AY_MONO;
	return chip;
}

void aymDestroy(aymChip* chip) {
	free(chip);
}

void aymResetChan(aymChan* ch) {
	ch->per = 16;		// period 1: 16T
	ch->step = 0;
	ch->vol = 0;
	ch->cnt = 0;
	ch->lev = 0;
	ch->tdis = 1;
	ch->ndis = 1;
	ch->een = 0;
}

// TurboSound

TSound* tsCreate(int tp,int tpA,int tpB) {
	TSound* ts = (TSound*)malloc(sizeof(TSound));
	ts->type = tp;
	ts->chipA = aymCreate(tpA);
	ts->chipB = aymCreate(tpB);
	ts->chipC = aymCreate(SND_NONE);
	ts->chipD = aymCreate(SND_NONE);
	ts->mute_l = 0;
	ts->mute_r = 0;
	return ts;
}

void tsDestroy(TSound* ts) {
	aymDestroy(ts->chipA);
	aymDestroy(ts->chipB);
	aymDestroy(ts->chipC);
	aymDestroy(ts->chipD);
	free(ts);
}

void tsSync(TSound* ts, int ns) {
	ts->chipA->sync(ts->chipA, ns);
	ts->chipB->sync(ts->chipB, ns);
	ts->chipC->sync(ts->chipC, ns);
	ts->chipD->sync(ts->chipD, ns);
}

sndPair tsGetVolume(TSound* ts) {
	sndPair res = ts->chipA->vol(ts->chipA);
	sndPair tmp = ts->chipB->vol(ts->chipB);
	sndPair vc = ts->chipC->vol(ts->chipC);
	sndPair vd = ts->chipC->vol(ts->chipC);
	res.left = ts->mute_l ? 0 : res.left + tmp.left + vc.left + vd.left;
	res.right = ts->mute_r ? 0 : res.right + tmp.right + vc.right + vd.right;
	return res;
}

void tsReset(TSound* ts) {
	ts->chipA->res(ts->chipA);
	ts->chipB->res(ts->chipB);
	ts->chipC->res(ts->chipC);
	ts->chipD->res(ts->chipD);
	ts->curChip = ts->chipA;
	ts->mute_l = 0;
	ts->mute_r = 0;
}

// fffd (a14 = 1)	wr:reg.num	rd:reg.data
// bffd (a14 = 0)	wr:reg.data	rd:FF

int tsIn(TSound* ts, int port) {
	return ts->curChip->rd(ts->curChip, (port >> 14) & 1);
}

void tsOut(TSound* ts, int port, int val) {
	port >>= 14;
	if (port & 1) {
		switch (ts->type) {
			case TS_NEDOPC:
				if ((val & 0xf8) == 0xf8) {
					ts->curChip = (val & 1) ? ts->chipB : ts->chipA;
				} else {
					ts->curChip->wr(ts->curChip, port, val);
				}
				break;
			case TS_ZXNEXT:
				if ((val & 0x9c) == 0x9c) {
					switch (val & 3) {
						case 0: ts->curChip = ts->chipD; break;		// sid
						case 1: ts->curChip = ts->chipC; break;		// psg 2
						case 2: ts->curChip = ts->chipB; break;		// psg 1
						case 3: ts->curChip = ts->chipA; break;		// psg 0 (default)
					}
					ts->mute_l = (val & 0x40) ? 1 : 0;
					ts->mute_r = (val & 0x20) ? 1 : 0;
				} else {
					ts->curChip->wr(ts->curChip, port, val);
				}
				break;
			default:
				ts->curChip->wr(ts->curChip, port, val);
				break;
		}

	} else {
		ts->curChip->wr(ts->curChip, port, val);
	}
}

// overall

// obsolete
void initNoise() {
	int cur = 0xffff;
	int lev;
	int i;
	for (i=0; i<0x20000; i++) {
		lev = (cur & 0x10000) ? 1 : 0;
		noizes[i] = lev;
		cur = ((cur << 1) + ((lev == (((cur & 0x2000) == 0x2000) ? 1 : 0)) ? 0 : 1)) & 0x1ffff;
	}
}
