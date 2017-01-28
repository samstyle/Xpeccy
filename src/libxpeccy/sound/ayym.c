#include <stdlib.h>
#include <string.h>

#include "ayym.h"

// tables

int noizes[0x20000];		// here iz noize values [generated at start]

unsigned char envforms[16][33]={
/*	  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32	*/
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},	// 8
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},	// 10
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},	// 12
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},	// 14
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255}
};

// AY/YM sound chip

void aymSetType(aymChip* ay, int tp) {
	ay->type = tp;
	switch (tp) {
		case SND_AY:
			ay->frq = 1.774400;
			break;
		case SND_YM:
			ay->frq = 1.750000;
			break;
		case SND_YM2203:		// 4.2 is base freq
			ay->frq = 4.2;
			break;
		default:
			ay->frq = 1;
			ay->type = SND_NONE;
	}
	if (ay->type == SND_NONE) return;
	ay->per = 1000 / ay->frq;
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

void aymReset(aymChip* ay) {
	memset(ay->reg, 0x00, 256);
	ay->chanA.per = 0;
	ay->chanB.per = 0;
	ay->chanC.per = 0;
	ay->chanE.per = 0;
	ay->chanN.per = 0;
	ay->chanE.step = 0;
	ay->chanN.step = 0;
	ay->chanA.cnt = 0;
	ay->chanB.cnt = 0;
	ay->chanC.cnt = 0;
	ay->chanE.cnt = 0;
	ay->chanN.cnt = 0;
}

void aymSetReg(aymChip* ay, unsigned char val) {
	if ((ay->curReg != 14) && (ay->curReg != 15))
		ay->reg[ay->curReg] = val;
	int tone;
	switch (ay->curReg) {
		case 0x00:
		case 0x01:
			tone = ay->reg[0] | ((ay->reg[1] & 0x0f) << 8);
			ay->chanA.per = tone << 4;		// 16 chip ticks / tone pulse
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
			ay->chanN.per = tone << 4;
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
			ay->chanE.per = tone << 4;		// must be << 8, but it didn't work WHYYYY?
			break;
		case 0x0d:
			ay->eForm = val & 0x0f;
			ay->chanE.step = 0;
			ay->chanE.cnt = ay->chanE.per;
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
			if (ay->chanA.cnt < 1) {
				ay->chanA.cnt = ay->chanA.per;
				ay->chanA.lev ^= 1;
			}
		}
		if (ay->chanB.per) {
			ay->chanB.cnt--;
			if (ay->chanB.cnt < 1) {
				ay->chanB.cnt = ay->chanB.per;
				ay->chanB.lev ^= 1;
			}
		}
		if (ay->chanC.per) {
			ay->chanC.cnt--;
			if (ay->chanC.cnt < 1) {
				ay->chanC.cnt = ay->chanC.per;
				ay->chanC.lev ^= 1;
			}
		}
		if (ay->chanN.per) {
			ay->chanN.cnt--;
			if (ay->chanN.cnt < 1) {
				ay->chanN.cnt = ay->chanN.per;
				ay->chanN.step++;
			}
		}
		if (ay->chanE.per) {
			ay->chanE.cnt--;
			if (ay->chanE.cnt < 1) {
				ay->chanE.cnt = ay->chanE.per;
				ay->chanE.step++;
				if ((ay->chanE.step & 31) == 0) {
					if ((ay->eForm & 9) == 8) {
						ay->chanE.step = 0;		// repeat
					} else {
						ay->chanE.step = 31;		// hold
					}
				}
			}
		}
	}
}

// NOTE : if tone & noise disabled, signal is 1 - take amp/env for digital music

int ayGetChanVol(aymChan* ch, int env, int noi) {
	int vol = ch->een ? env : ch->vol;
	if (ch->ten || ch->nen) {
		if (!(ch->ten && ch->lev) && !(ch->nen && noi)) {
			vol = 0;
		}
	}
	return vol;
}

sndPair aymGetVolume(aymChip* ay) {
	sndPair res;
	int volA;
	int volB;
	int volC;

	ay->chanN.lev = noizes[ay->chanN.step & 0x1ffff] ? 1 : 0;	// noise value
	ay->chanE.vol = envforms[ay->eForm][ay->chanE.step];		// envelope value

	volA = ayGetChanVol(&ay->chanA, ay->chanE.vol, ay->chanN.lev);
	volB = ayGetChanVol(&ay->chanB, ay->chanE.vol, ay->chanN.lev);
	volC = ayGetChanVol(&ay->chanC, ay->chanE.vol, ay->chanN.lev);

	switch (ay->stereo) {
		case AY_ABC:
			res.left = volA + 0.7 * volB;
			res.right = volC + 0.7 * volB;
			break;
		case AY_ACB:
			res.left = volA + 0.7 * volC;
			res.right = volB + 0.7 * volC;
			break;
		case AY_BAC:
			res.left = volB + 0.7 * volA;
			res.right = volC + 0.7 * volA;
			break;
		case AY_BCA:
			res.left = volB + 0.7 * volC;
			res.right = volA + 0.7 * volC;
			break;
		case AY_CAB:
			res.left = volC + 0.7 * volA;
			res.right = volB + 0.7 * volA;
			break;
		case AY_CBA:
			res.left = volC + 0.7 * volB;
			res.right = volA + 0.7 * volB;
			break;
		default:
			res.left = (volA + volB + volC) * 0.7;
			res.right = res.left;
			break;
	}
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

void tsSync(TSound* ts, long ns) {
	aymSync(ts->chipA,ns);
	aymSync(ts->chipB,ns);
}

sndPair tsGetVolume(TSound* ts) {
	sndPair res = aymGetVolume(ts->chipA);
	sndPair tmp = aymGetVolume(ts->chipB);
	res.left += tmp.left;
	res.right += tmp.right;
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
		lev = ((cur & 0x10000) == 0) ? 0 : 1;
		noizes[i] = lev;
		cur = ((cur << 1) + ((lev == (((cur & 0x2000) == 0x2000) ? 1 : 0)) ? 0 : 1)) & 0x1ffff;
	}
}
