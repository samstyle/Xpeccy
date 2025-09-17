#include <string.h>

#include "ayym.h"

extern void aymResetChan(aymChan* ch);
static const int ay_val_mask[16] = {0xff,0x0f,0xff,0x0f,0xff,0x0f,0x1f,0xff,0x1f,0x1f,0x1f,0xff,0xff,0x0f,0xff,0xff};
// extern int ayDACvol[32];

static int ayDACvol[32] = {0x0000,0x0000,0x00D0,0x00D0,0x0130,0x0130,0x01BC,0x01BC,
                    0x0291,0x0291,0x03C4,0x03C4,0x0544,0x0544,0x089F,0x089F,
                    0x0A27,0x0A27,0x1053,0x1053,0x16C8,0x16C8,0x1C96,0x1C96,
                    0x2417,0x2417,0x2D54,0x2D54,0x35E8,0x35E8,0x3FFF,0x3FFF};

void ay_reset(aymChip* chip) {
	memset(chip->reg, 0x00, 256);
	aymResetChan(&chip->chanA);
	aymResetChan(&chip->chanB);
	aymResetChan(&chip->chanC);
	aymResetChan(&chip->chanE);
	aymResetChan(&chip->chanN);
	chip->chanN.per = 1;
	chip->chanN.step = 0xffff;
}

int ay_rd(aymChip* ay, int adr) {
	unsigned char res = 0xff;
	if (adr & 1) {
		switch(ay->curReg & 0x0f) {					// AY:16 registers + mirrors
			case 14:
				if (!(ay->reg[7] & 0x40))
					res = ay->reg[14];
				break;
			case 15:
				if (!(ay->reg[7] & 0x80))
					res = ay->reg[15];
				break;
			default:
				res = ay->reg[ay->curReg];
				res &= ay_val_mask[ay->curReg & 0x0f];		// AY:reset unused bits
				break;
		}
	}
	return res;
}

void ay_set_reg(aymChip* chip, int val) {
	int tone;
	if ((chip->curReg != 14) && (chip->curReg != 15))
		chip->reg[chip->curReg] = val & 0xff;
	switch (chip->curReg) {
		case 0x00:
		case 0x01:
			tone = chip->reg[0] | ((chip->reg[1] & 0x0f) << 8);
			if (tone == 0) tone++;
			chip->chanA.per = tone << 4;		// min 16T on half-period
			break;
		case 0x02:
		case 0x03:
			tone = chip->reg[2] | ((chip->reg[3] & 0x0f) << 8);
			if (tone == 0) tone++;
			chip->chanB.per = tone << 4;
			break;
		case 0x04:
		case 0x05:
			tone = chip->reg[4] | ((chip->reg[5] & 0x0f) << 8);
			if (tone == 0) tone++;
			chip->chanC.per = tone << 4;
			break;
		case 0x06:					// noise
			tone = val & 0x1f;
			if (tone == 0) tone++;
			chip->chanN.per = tone << 5;		// min 16T x2 half-periods
			break;
		case 0x07:
			chip->chanA.tdis = (val & 1) ? 1 : 0;
			chip->chanB.tdis = (val & 2) ? 1 : 0;
			chip->chanC.tdis = (val & 4) ? 1 : 0;
			chip->chanA.ndis = (val & 8) ? 1 : 0;
			chip->chanB.ndis = (val & 16) ? 1 : 0;
			chip->chanC.ndis = (val & 32) ? 1 : 0;
			break;
		case 0x08:
			chip->chanA.vol = ((val & 15) << 1) | 1;
			chip->chanA.een = (val & 16) ? 1 : 0;
			break;
		case 0x09:
			chip->chanB.vol = ((val & 15) << 1) | 1;
			chip->chanB.een = (val & 16) ? 1 : 0;
			break;
		case 0x0a:
			chip->chanC.vol = ((val & 15) << 1) | 1;
			chip->chanC.een = (val & 16) ? 1 : 0;
			break;
		case 0x0b:
		case 0x0c:
			tone = chip->reg[11] | (chip->reg[12] << 8);
			if (tone == 0) tone++;
			chip->chanE.per = tone << 4;
			break;
		case 0x0d:
			chip->eForm = val & 0x0f;
			chip->chanE.cnt = 0;					// only if form changed?
			chip->chanE.vol = (val & 4) ? 0 : 31;
			chip->chanE.step = (val & 4) ? 1 : -1;
			break;
		case 0x0e:
			if (chip->reg[7] & 0x40)
				chip->reg[14] = val & 0xff;
			break;
		case 0x0f:
			if (chip->reg[7] & 0x80)
				chip->reg[15] = val & 0xff;
			break;
	}
}

void ay_wr(aymChip* chip, int adr, int val) {
	if (adr & 1) {								// set current reg
		chip->curReg = val & 0x0f;					// AY:16 registers + mirrors
	} else {								// write data
		ay_set_reg(chip, val);
	}
}

void ay_sync(aymChip* ay, int ns) {
	if (ay->per < 1) return;
	ay->cnt -= ns;
	while (ay->cnt < 0) {
		ay->cnt += ay->per;
		if (++ay->chanA.cnt >= ay->chanA.per) {
			ay->chanA.cnt = 0;
			ay->chanA.lev ^= 1;
		}
		if (++ay->chanB.cnt >= ay->chanB.per) {
			ay->chanB.cnt = 0;
			ay->chanB.lev ^= 1;
		}
		if (++ay->chanC.cnt >= ay->chanC.per) {
			ay->chanC.cnt = 0;
			ay->chanC.lev ^= 1;
		}
		if (++ay->chanN.cnt >= ay->chanN.per) {
			ay->chanN.cnt = 0;
			ay->chanN.step = (ay->chanN.step << 1) | ((((ay->chanN.step >> 13) ^ (ay->chanN.step >> 16)) & 1) ^ 1);
			ay->chanN.lev = (ay->chanN.step >> 16) & 1;
		}
		if (++ay->chanE.cnt >= ay->chanE.per) {
			ay->chanE.cnt = 0;
			ay->chanE.vol += ay->chanE.step;
			if (ay->chanE.vol & ~31) {				// 32 || -1
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

sndPair ay_mix_stereo(int volA, int volB, int volC, int id) {
	int lef,cen,rig;
	sndPair res;
	switch (id) {
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

int ay_chan_vol(aymChip* ay, aymChan* ch) {
	int vol = 0;
#if 1
	int mixlev = (ch->tdis || ch->lev) && (ch->ndis || ay->chanN.lev);
	if (ch->een) {
		if (mixlev) {
			vol = ayDACvol[ay->chanE.vol & 0x1f];
			if (ch->per < 0x60) vol >>= 1;
		}
	} else {
		if ((ch->per < 0x60) || mixlev) {
			vol = ayDACvol[ch->vol & 0x1f];
		}
	}
#else
	int lev = (ch->per < 0x60) ? 1 : ch->lev;
	if ((ch->tdis || /*ch->*/lev) && (ch->ndis || ay->chanN.lev)) {
		vol = ch->een ? ay->chanE.vol : (ch->ndis && !ch->tdis && !lev/* && (ch->per < 0x60)*/) ? 0 : ch->vol;
	} else {
		vol = 0;
	}
	vol = ayDACvol[vol];						// AY:4-bit DAC volume
//	if (ch->per < 0x60) vol >>= 1;
#endif
	return vol;
}

sndPair ay_vol(aymChip* chip) {
	int volA = ay_chan_vol(chip, &chip->chanA);
	int volB = ay_chan_vol(chip, &chip->chanB);
	int volC = ay_chan_vol(chip, &chip->chanC);
	return ay_mix_stereo(volA, volB, volC, chip->stereo);
}
