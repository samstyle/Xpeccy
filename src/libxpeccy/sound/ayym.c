#include <stdlib.h>
#include <string.h>

#include "ayym.h"

// tables

char noizes[0x20000];		// here iz noize values 1/0 [generated at start]

#if 0
static int ayDACvol[32] = {0x0000,0x0000,0x001D,0x003A,0x0052,0x0065,0x007D,0x009A,
			   0x00C2,0x00F0,0x0122,0x0146,0x0186,0x01D6,0x0226,0x0274,
			   0x02EA,0x037E,0x041C,0x04B2,0x0594,0x06AF,0x07C8,0x08EB,
			   0x0AA0,0x0CC4,0x0EE6,0x1108,0x143A,0x1820,0x1C14,0x1FFF};
#elif 1
static int ayDACvol[32] = {0x0000,0x0000,0x003B,0x0074,0x00A4,0x00CA,0x00FB,0x0134,
			   0x0184,0x01E0,0x0244,0x028D,0x030C,0x03AD,0x044C,0x04E8,
			   0x05D4,0x06FD,0x0838,0x0965,0x0B28,0x0D5F,0x0F91,0x11D7,
			   0x1540,0x1988,0x1DCC,0x2211,0x2874,0x3040,0x3828,0x3FFF};
#endif

// AY/YM sound chip

void aymSetType(aymChip* ay, int tp) {
	ay->type = tp;
	switch (tp) {
		case SND_AY:
			if (ay->frq == 0)
				ay->frq = 1.774400;
			ay->coarse = 1;
			break;
		case SND_YM:
			if (ay->frq == 0)
				ay->frq = 1.750000;
			ay->coarse = 0;
			break;
		case SND_YM2203:		// 4.2 is base freq
			if (ay->frq == 0)
				ay->frq = 4.2;
			break;
		default:
			if (ay->frq == 0)
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
	ch->vol = 0;
	ch->cnt = 0;
	ch->lev = 0;
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
	ay->chanN.step = 0xffff;
}

static const int ay_val_mask[16] = {0xff,0x0f,0xff,0x0f,0xff,0x0f,0x1f,0xff,0x1f,0x1f,0x1f,0xff,0xff,0x0f,0xff,0xff};

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
			ay->chanN.per = (tone + 1) << 6;
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
			ay->chanA.vol = ((val & 15) << 1) | 1;
			ay->chanA.een = (val & 16) ? 1 : 0;
			break;
		case 0x09:
			ay->chanB.vol = ((val & 15) << 1) | 1;
			ay->chanB.een = (val & 16) ? 1 : 0;
			break;
		case 0x0a:
			ay->chanC.vol = ((val & 15) << 1) | 1;
			ay->chanC.een = (val & 16) ? 1 : 0;
			break;
		case 0x0b:
		case 0x0c:
			tone = ay->reg[11] | (ay->reg[12] << 8);
			if (!tone) tone++;
			ay->chanE.per = tone << 4;
			break;
		case 0x0d:
			ay->eForm = val & 0x0f;
			ay->chanE.cnt = 0;			// only if form changed?
			ay->chanE.vol = (val & 4) ? 0 : 31;
			ay->chanE.step = (val & 4) ? 1 : -1;
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

// todo: use chip rd/wr procedures
void aymWr(aymChip* ay, int adr, unsigned char val) {
	if (adr & 1) {				// set current reg
		ay->curReg = val;
		if (ay->type == SND_AY)
			ay->curReg &= 0x0f;
	} else {				// write data
		aymSetReg(ay, val);
	}
}

unsigned char aymRd(aymChip* ay, int adr) {
	unsigned char res = 0xff;
	if (adr & 1) {
		switch(ay->curReg) {
			case 14: if (ay->reg[7] & 0x40) res = ay->reg[14];
				break;
			case 15: if (ay->reg[7] & 0x80) res = ay->reg[15];
				break;
			default: res = ay->reg[ay->curReg];
				if (ay->type == SND_AY)
					res &= ay_val_mask[ay->curReg & 0x0f];
				break;
		}
	}
	return res;
}

void aymSync(aymChip* ay, int ns) {
	if (ay->per < 1) return;
	if (ay->type == SND_NONE) return;
	ay->cnt -= ns;
	while (ay->cnt < 0) {
		ay->cnt += ay->per;
		if (ay->chanA.per) {
			if (++ay->chanA.cnt >= ay->chanA.per) {
				ay->chanA.cnt = 0;
				ay->chanA.lev ^= 1;
			}
		}
		if (ay->chanB.per) {
			if (++ay->chanB.cnt >= ay->chanB.per) {
				ay->chanB.cnt = 0;
				ay->chanB.lev ^= 1;
			}
		}
		if (ay->chanC.per) {
			if (++ay->chanC.cnt >= ay->chanC.per) {
				ay->chanC.cnt = 0;
				ay->chanC.lev ^= 1;
			}
		}
		if (ay->chanN.per) {
			if (++ay->chanN.cnt >= ay->chanN.per) {
				ay->chanN.cnt = 0;
				ay->chanN.step = (ay->chanN.step << 1) | ((((ay->chanN.step >> 13) ^ (ay->chanN.step >> 16)) & 1) ^ 1);
				ay->chanN.lev = (ay->chanN.step >> 16) & 1;
			}
		}
		if (ay->chanE.per) {
			if (++ay->chanE.cnt >= ay->chanE.per) {
				ay->chanE.cnt = 0;
				ay->chanE.vol += ay->chanE.step;
				if (ay->chanE.vol & ~31) {		// 32 || -1
					if (ay->eForm & 8) {				// 1xxx
						if (ay->eForm & 1) {			// 1xx1 : 9,B,D,F : stop
							ay->chanE.vol -= ay->chanE.step;
							ay->chanE.step = 0;
							if (ay->eForm & 2) {		// 1x11 : B,F : invert volume
								ay->chanE.vol ^= 0x1f;
							}
						} else if (ay->eForm & 2) {		// 1x10 : A,E : change direction (wave)
							ay->chanE.step = -ay->chanE.step;
							ay->chanE.vol += ay->chanE.step;
						} else {				// 1x00 : 8,C : repeat (saw)
							ay->chanE.vol &= 0x1f;
						}
					} else {					// 0xxx : silent, stop
						ay->chanE.vol = 0;
						ay->chanE.step = 0;
					}
				}
			}
		}
	}
}

// NOTE : if tone & noise disabled, signal is 1 - take amp/env for digital music

static int vol;

int ayGetChanVol(aymChip* ay, aymChan* ch) {
	if (((ch->lev && ch->ten) || (ch->nen && ay->chanN.lev)) || !(ch->ten || ch->nen)) {
		vol = ch->een ? ay->chanE.vol : ch->vol;
	} else {
		vol = 0;
	}
	if (ay->coarse)
		vol |= 1;
	return ayDACvol[vol];
}

sndPair aymGetVolume(aymChip* ay) {
	sndPair res;
	int volA = ayGetChanVol(ay, &ay->chanA);
	int volB = ayGetChanVol(ay, &ay->chanB);
	int volC = ayGetChanVol(ay, &ay->chanC);
	int lef,cen,rig;

	switch (ay->stereo) {
		case AY_ABC:
			lef = volA;
			cen = volB;
			rig = volC;
			break;
		case AY_ACB:
			lef = volA;
			cen = volC;
			rig = volB;
			break;
		case AY_BAC:
			lef = volB;
			cen = volA;
			rig = volC;
			break;
		case AY_BCA:
			lef = volB;
			cen = volC;
			rig = volA;
			break;
		case AY_CAB:
			lef = volC;
			cen = volA;
			rig = volB;
			break;
		case AY_CBA:
			lef = volC;
			cen = volB;
			rig = volA;
			break;
		default:
			lef = (volA + volB + volC) / 3;
			cen = lef;
			rig = lef;
			break;
	}

	res.left = lef + (cen >> 1);
	res.right = rig + (cen >> 1);
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
	res.left = (res.left + tmp.left);
	res.right = (res.right + tmp.right);
	return res;
}

void tsReset(TSound* ts) {
	aymReset(ts->chipA);
	aymReset(ts->chipB);
	ts->curChip = ts->chipA;
}

// fffd (a14 = 1)	wr:reg.num	rd:reg.data
// bffd (a14 = 0)	wr:reg.data	rd:FF

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
		if (ts->curChip->type == SND_AY) {
			res &= ay_val_mask[ts->curChip->curReg & 0x0f];
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
						if (ts->curChip->type == SND_AY)
							val &= 0x0f;
						ts->curChip->curReg = val;
					}
					break;
				default:
					if (ts->curChip->type == SND_AY)
						val &= 0x0f;
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
		noizes[i] = lev;
		cur = ((cur << 1) + ((lev == (((cur & 0x2000) == 0x2000) ? 1 : 0)) ? 0 : 1)) & 0x1ffff;
	}
}
