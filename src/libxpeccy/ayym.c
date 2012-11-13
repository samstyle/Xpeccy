#include <stdlib.h>

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
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255}
};

// AY/YM sound chip

void aymSetType(aymChip* ay, int tp) {
	ay->type = tp;
	switch (tp) {
		case SND_AY: ay->freq = 1774400; break;
		case SND_YM: ay->freq = 1750000; break;
		default: ay->type = SND_NONE;
	}
	if (ay->type != SND_NONE) ay->aycoe = 400 * 448 * 320 / (double)ay->freq;
}

aymChip* aymCreate(int tp) {
	aymChip* ay = (aymChip*)malloc(sizeof(aymChip));
	aymSetType(ay,tp);
	ay->stereo = AY_MONO;
	ay->chanA.period = 0;
	ay->chanB.period = 0;
	ay->chanC.period = 0;
	ay->chanN.period = 0;
	ay->chanE.period = 0;
	ay->chanA.lev = 0;
	ay->chanB.lev = 0;
	ay->chanC.lev = 0;
	ay->chanN.lev = 0;
	ay->chanE.lev = 0;
	return ay;
}

void aymDestroy(aymChip* ay) {
	free(ay);
}

void aymReset(aymChip* ay) {
	int i;
	for (i = 0; i < 16; i++) ay->reg[i] = 0;
	ay->eCur = 0;
	ay->nPos = 0;
	ay->ePos = 0;
	ay->chanA.period = 0;
	ay->chanB.period = 0;
	ay->chanC.period = 0;
	ay->chanN.period = 0;
	ay->chanE.period = 0;
	ay->chanA.count = 0;
	ay->chanB.count = 0;
	ay->chanC.count = 0;
	ay->chanN.count = 0;
	ay->chanE.count = 0;
}

void aymSetReg(aymChip* ay, unsigned char val) {
	if (ay->curReg > 15) return;
	if (ay->curReg < 14) ay->reg[ay->curReg] = val;
	switch (ay->curReg) {
		case 0x00:
		case 0x01: ay->chanA.period = ay->aycoe * (ay->reg[0] + ((ay->reg[1] & 0x0f) << 8)); break;
		case 0x02:
		case 0x03: ay->chanB.period = ay->aycoe * (ay->reg[2] + ((ay->reg[3] & 0x0f) << 8)); break;
		case 0x04:
		case 0x05: ay->chanC.period = ay->aycoe * (ay->reg[4] + ((ay->reg[5] & 0x0f) << 8)); break;
		case 0x06: ay->chanN.period = 2 * ay->aycoe * (val & 0x1f); break;
		case 0x0b:
		case 0x0c: ay->chanE.period = 2 * ay->aycoe * (ay->reg[11] + (ay->reg[12] << 8)); break;
		case 0x0d: ay->eCur = val & 0x0f; ay->ePos = 0; ay->chanE.count = 0; break;
		case 0x0e: if (ay->reg[7] & 0x40) ay->reg[14] = val; break;
		case 0x0f: if (ay->reg[7] & 0x80) ay->reg[15] = val; break;
	}
}

void aymSync(aymChip* ay, int tk) {
	if (ay->type == SND_NONE) return;
	if (ay->chanA.period != 0) {
		ay->chanA.count -= tk;
		while (ay->chanA.count < 0) {
			ay->chanA.count += ay->chanA.period;
			ay->chanA.lev = !ay->chanA.lev;
		}
	}
	if (ay->chanB.period != 0) {
		ay->chanB.count -= tk;
		while (ay->chanB.count < 0) {
			ay->chanB.count += ay->chanB.period;
			ay->chanB.lev = !ay->chanB.lev;
		}
	}
	if (ay->chanC.period != 0) {
		ay->chanC.count -= tk;
		while (ay->chanC.count < 0) {
			ay->chanC.count += ay->chanC.period;
			ay->chanC.lev = !ay->chanC.lev;
		}
	}
	if (ay->chanN.period != 0) {
		ay->chanN.count -= tk;
		while (ay->chanN.count < 0) {
			ay->chanN.count += ay->chanN.period;
			ay->nPos++;
		}
		ay->chanN.lev = noizes[ay->nPos & 0x1ffff];
	}
	if (ay->chanE.period != 0) {
		ay->chanE.count -= tk;
		while (ay->chanE.count < 0) {
			ay->chanE.count += ay->chanE.period;
			ay->ePos++;
			switch (envforms[ay->eCur][ay->ePos]) {
				case 255: ay->ePos--; break;
				case 253: ay->ePos = 0; break;
			}
		}
	}
}

int volA,volB,volC;

tsPair aymGetVolume(aymChip* ay) {
	tsPair res;
	volA = (ay->reg[8] & 16) ? envforms[ay->eCur][ay->ePos] : (ay->reg[8] & 15);
	volB = (ay->reg[9] & 16) ? envforms[ay->eCur][ay->ePos] : (ay->reg[9] & 15);
	volC = (ay->reg[10] & 16) ? envforms[ay->eCur][ay->ePos] : (ay->reg[10] & 15);
	if ((ay->reg[7] & 0x09) != 0x09) {
		volA *= ((ay->reg[7] & 1) ? 0 : ay->chanA.lev) + ((ay->reg[7] & 8) ? 0 : ay->chanN.lev);
	}
	if ((ay->reg[7] & 0x12) != 0x12) {
		volB *= ((ay->reg[7] & 2) ? 0 : ay->chanB.lev) + ((ay->reg[7] & 16) ? 0 : ay->chanN.lev);
	}
	if ((ay->reg[7] & 0x24) != 0x24) {
		volC *= ((ay->reg[7] & 4) ? 0 : ay->chanC.lev) + ((ay->reg[7] & 32) ? 0 : ay->chanN.lev);
	}
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

void tsSync(TSound* ts, int tk) {
	aymSync(ts->chipA,tk);
	aymSync(ts->chipB,tk);
}

tsPair tsGetVolume(TSound* ts) {
	tsPair res = aymGetVolume(ts->chipA);
	tsPair tmp = aymGetVolume(ts->chipB);
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
					switch (val) {
						case 0xfe: ts->curChip = ts->chipA; break;
						case 0xff: ts->curChip = ts->chipB; break;
						default: ts->curChip->curReg = val & 0x0f; break;
					}
					break;
				default:
					ts->curChip->curReg = val & 0x0f;
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
