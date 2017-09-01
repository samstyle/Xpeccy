#include <stdlib.h>
#include <string.h>

#include "ayym.h"

// tables

char noizes[0x20000];		// here iz noize values 1/0 [generated at start]

static unsigned char envforms[16][34]={
/*	  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32	*/
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,255},		// 0..3: max->0,stay
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,255},
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,255},
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,255},

	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, 0,255},	// 4..7: 0->max,invert,stay
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, 0,255},

	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},		// 8: max->0, repeat
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,255},		// 9: max->0, stay
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,254},		// A: max->0, invert, repeat
	{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,31,255},	// B: max->0, invert, stay

	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,253},		// C: 0->max, repeat
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,255},		// D: 0->max, stay
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,254},		// E: 0->max, invert, repeat
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, 0,255}	// F: 0->max, invert, stay
};

// try to use it somehow :)
//unsigned char ayDACvol[16] = {0,0,0,0,0,1,1,1,1,2,2,3,4,7,10,15};		// from AY manual
//unsigned char ayDACvol[16] = {0,0,0,0,0,0,0,1,1,3,4,5,7,9,12,15};		// from ayumi

// AY/YM sound chip

void aymSetType(aymChip* ay, int tp) {
	ay->type = tp;
	switch (tp) {
		case SND_AY:
			ay->frq = 1.774400;
			ay->coarseEnv = 1;
			break;
		case SND_YM:
			ay->frq = 1.750000;
			ay->coarseEnv = 0;
			break;
		case SND_YM2203:		// 4.2 is base freq
			ay->frq = 4.2;
			break;
		default:
			ay->frq = 1;
			ay->type = SND_NONE;
	}
	if (ay->type == SND_NONE) return;
	ay->per = 500 / ay->frq;		// 1000/frq = full period, 500/frq = half-period
	ay->cnt = ay->per;
}

aymChip* aymCreate(int tp) {
	aymChip* ay = (aymChip*)malloc(sizeof(aymChip));
	memset(ay, 0x00, sizeof(aymChip));
	aymSetType(ay,tp);
	ay->stereo = AY_MONO;
	return ay;
}

void aymDestroy(aymChip* ay) {
	free(ay);
}

void aymResetChan(aymChan* ch) {
	ch->per = 0;
	ch->step = 0;
	ch->cnt = 0;
	ch->ten = 0;
	ch->nen = 0;
	ch->een = 0;
}

void aymReset(aymChip* ay) {
	memset(ay->reg, 0x00, 256);
	aymResetChan(&ay->chanA);
	aymResetChan(&ay->chanB);
	aymResetChan(&ay->chanC);
	aymResetChan(&ay->chanE);
	aymResetChan(&ay->chanN);
	ay->chanN.per = 1;
}

void aymSetReg(aymChip* ay, unsigned char val) {
	if ((ay->curReg != 14) && (ay->curReg != 15))
		ay->reg[ay->curReg] = val;
	int tone;
	switch (ay->curReg) {
		case 0x00:
		case 0x01:
			tone = ay->reg[0] | ((ay->reg[1] & 0x0f) << 8);
			ay->chanA.per = tone << 4;		// 16 chip ticks / tone tick
			break;
		case 0x02:
		case 0x03:
			tone = ay->reg[2] | ((ay->reg[3] & 0x0f) << 8);
			ay->chanB.per = tone << 4;
			break;
		case 0x04:
		case 0x05:
			tone = ay->reg[4] | ((ay->reg[5] & 0x0f) << 8);
			ay->chanC.per = tone << 4;
			break;
		case 0x06:					// noise
			tone = val & 0x1f;
			ay->chanN.per = (tone + 1) << 5;
			break;
		case 0x07:
			ay->chanA.ten = (val & 1) ? 0 : 1;
			ay->chanB.ten = (val & 2) ? 0 : 1;
			ay->chanC.ten = (val & 4) ? 0 : 1;
			ay->chanA.nen = (val & 8) ? 0 : 1;
			ay->chanB.nen = (val & 16) ? 0 : 1;
			ay->chanC.nen = (val & 32) ? 0 : 1;
			break;
		case 0x08:
			ay->chanA.vol = val & 15;
			ay->chanA.een = (val & 16) ? 1 : 0;
			break;
		case 0x09:
			ay->chanB.vol = val & 15;
			ay->chanB.een = (val & 16) ? 1 : 0;
			break;
		case 0x0a:
			ay->chanC.vol = val & 15;
			ay->chanC.een = (val & 16) ? 1 : 0;
			break;
		case 0x0b:
		case 0x0c:
			tone = ay->reg[11] | (ay->reg[12] << 8);
			ay->chanE.per = tone << 4;
			break;
		case 0x0d:
			ay->eForm = val & 0x0f;
			ay->chanE.step = 0;
			ay->chanE.cnt = ay->chanE.per;
			ay->chanE.lev = 0;
			break;
		case 0x0e:
			if (ay->reg[7] & 0x40)
				ay->reg[14] = val;
			break;
		case 0x0f:
			if (ay->reg[7] & 0x80)
				ay->reg[15] = val;
			break;
	}
}

void aymSync(aymChip* ay, int ns) {
	if (ay->per < 1) return;
	if (ay->type == SND_NONE) return;
	ay->cnt -= ns;
	while (ay->cnt < 0) {
		ay->cnt += ay->per;
		if (ay->chanA.per) {
			ay->chanA.cnt--;
			if (ay->chanA.cnt < 0) {
				ay->chanA.cnt = ay->chanA.per;
				ay->chanA.lev ^= 1;
			}
		}
		if (ay->chanB.per) {
			ay->chanB.cnt--;
			if (ay->chanB.cnt < 0) {
				ay->chanB.cnt = ay->chanB.per;
				ay->chanB.lev ^= 1;
			}
		}
		if (ay->chanC.per) {
			ay->chanC.cnt--;
			if (ay->chanC.cnt < 0) {
				ay->chanC.cnt = ay->chanC.per;
				ay->chanC.lev ^= 1;
			}
		}
		if (ay->chanN.per) {
			ay->chanN.cnt--;
			if (ay->chanN.cnt < 0) {
				ay->chanN.cnt = ay->chanN.per;
				ay->chanN.step++;
				ay->chanN.lev = noizes[ay->chanN.step & 0x1ffff] ? 1 : 0;
			}
		}
		if (ay->chanE.per) {
			ay->chanE.cnt--;
			if (ay->chanE.cnt < 0) {
				ay->chanE.cnt = ay->chanE.per;
				if (ay->chanE.lev) {
					ay->chanE.step--;
					if (ay->chanE.step < 0) {
						ay->chanE.lev = 0;
						ay->chanE.step = 0;
					}
				} else {
					ay->chanE.step++;
				}
				switch (envforms[ay->eForm][ay->chanE.step]) {
					case 253:
						ay->chanE.step = 0;
						break;
					case 254:
						ay->chanE.lev = 1;
						ay->chanE.step--;
						break;
					case 255:
						ay->chanE.step--;
						break;
				}
				ay->chanE.vol = envforms[ay->eForm][ay->chanE.step];
				if (ay->coarseEnv) {
					ay->chanE.vol |= 1;
				}
			}
		}
	}
}

// NOTE : if tone & noise disabled, signal is 1 - take amp/env for digital music
int ayGetChanVol(aymChip* ay, aymChan* ch) {
	int vol = ch->een ? ay->chanE.vol : ch->vol;
	if (ch->ten || ch->nen) {
		vol = ((ch->lev && ch->ten) || (ay->chanN.lev && ch->nen)) ? vol : 0;
	}
	return (vol << 1) | (vol >> 3);		// 0..1f (env) -> 00..3f
	// return ayDACvol[vol];
}

#define STEREOFACTOR 0.5

sndPair aymGetVolume(aymChip* ay) {
	sndPair res;
	// ay->chanN.lev = noizes[ay->chanN.step & 0x1ffff] ? 1 : 0;	// noise value
	// ay->chanE.vol = envforms[ay->eForm][ay->chanE.step];		// envelope value
	int volA = ayGetChanVol(ay, &ay->chanA);
	int volB = ayGetChanVol(ay, &ay->chanB);
	int volC = ayGetChanVol(ay, &ay->chanC);

	switch (ay->stereo) {
		case AY_ABC:
			res.left = volA + STEREOFACTOR * volB;			// max = 7F (SF = 1.0)
			res.right = volC + STEREOFACTOR * volB;
			break;
		case AY_ACB:
			res.left = volA + STEREOFACTOR * volC;
			res.right = volB + STEREOFACTOR * volC;
			break;
		case AY_BAC:
			res.left = volB + STEREOFACTOR * volA;
			res.right = volC + STEREOFACTOR * volA;
			break;
		case AY_BCA:
			res.left = volB + STEREOFACTOR * volC;
			res.right = volA + STEREOFACTOR * volC;
			break;
		case AY_CAB:
			res.left = volC + STEREOFACTOR * volA;
			res.right = volB + STEREOFACTOR * volA;
			break;
		case AY_CBA:
			res.left = volC + STEREOFACTOR * volB;
			res.right = volA + STEREOFACTOR * volB;
			break;
		default:
			res.left = (volA + volB + volC) * STEREOFACTOR;
			res.right = res.left;
			break;
	}
	res.left >>= 1;
	res.right >>= 1;
	return res;
}

// TurboSound

TSound* tsCreate(int tp,int tpA,int tpB) {
	TSound* ts = (TSound*)malloc(sizeof(TSound));
	ts->type = tp;
	ts->chipA = aymCreate(tpA);
	ts->chipB = aymCreate(tpB);
	return ts;
}

void tsDestroy(TSound* ts) {
	aymDestroy(ts->chipA);
	aymDestroy(ts->chipB);
	free(ts);
}

void tsSync(TSound* ts, int ns) {
	aymSync(ts->chipA,ns);
	aymSync(ts->chipB,ns);
}

sndPair tsGetVolume(TSound* ts) {
	sndPair res = aymGetVolume(ts->chipA);
	sndPair tmp = aymGetVolume(ts->chipB);
	res = mixer(res, tmp.left, tmp.right, 100);
	return res;
}

void tsReset(TSound* ts) {
	aymReset(ts->chipA);
	aymReset(ts->chipB);
	ts->curChip = ts->chipA;
}

unsigned char tsIn(TSound* ts, int port) {
	unsigned char res = 0xff;
	if (port == 0xfffd) {
		switch (ts->curChip->curReg) {
			case 14:
				if (ts->curChip->reg[7] & 0x40) res = ts->curChip->reg[14];
				break;
			case 15:
				if (ts->curChip->reg[7] & 0x80) res = ts->curChip->reg[15];
				break;
			default:
				if (ts->curChip->curReg < 14) res = ts->curChip->reg[ts->curChip->curReg];
				break;
		}
	}
	return res;
}

void tsOut(TSound* ts, int port, unsigned char val) {
	switch (port) {
		case 0xbffd:
			aymSetReg(ts->curChip,val);
			break;
		case 0xfffd:
			switch (ts->type) {
				case TS_NEDOPC:
					if ((val & 0xf8) == 0xf8) {
						ts->curChip = (val & 1) ? ts->chipB : ts->chipA;
					} else {
						ts->curChip->curReg = val; break;
					}
					break;
				default:
					ts->curChip->curReg = val;
					break;
			}
			break;
	}
}

// overall

void initNoise() {
	int cur = 0xffff;
	int lev;
	int i;
	for (i=0; i<0x20000; i++) {
		lev = (cur & 0x10000) ? 1 : 0;
		noizes[i] = lev & 1;
		cur = ((cur << 1) + ((lev == (((cur & 0x2000) == 0x2000) ? 1 : 0)) ? 0 : 1)) & 0x1ffff;
	}
}
