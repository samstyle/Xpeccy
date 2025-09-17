#include "ayym.h"

// extern int ayDACvol[32];

int ymDACvol[32] = {0x0000,0x0000,0x003B,0x0074,0x00A4,0x00CA,0x00FB,0x0134,
                    0x0184,0x01E0,0x0244,0x028D,0x030C,0x03AD,0x044C,0x04E8,
                    0x05D4,0x06FD,0x0838,0x0965,0x0B28,0x0D5F,0x0F91,0x11D7,
                    0x1540,0x1988,0x1DCC,0x2211,0x2874,0x3040,0x3828,0x3FFF};

// ym_reset = ay_reset

int ym_rd(aymChip* chip, int adr) {
	unsigned char res = 0xff;
	if (adr & 1) {
		switch(chip->curReg) {
			case 14:
				if (chip->reg[7] & 0x40)
					res = chip->reg[14];
				break;
			case 15:
				if (chip->reg[7] & 0x80)
					res = chip->reg[15];
				break;
			default:
				res = chip->reg[chip->curReg];			// YM:store unused bits
				break;
		}
	}
	return res;
}

extern void ay_set_reg(aymChip*, int);

void ym_wr(aymChip* chip, int adr, int val) {
	if (adr & 1) {								// set current reg
		chip->curReg = val & 0xff;					// YM:256 registers, no mirrors
	} else {								// write data
		ay_set_reg(chip, val);
	}
}

// ym_sync = ay_sync (with 5-bit volumes)

extern sndPair ay_mix_stereo(int, int, int, int);

int ym_chan_vol(aymChip* ay, aymChan* ch) {
	int vol = 0;
#if 1
	int mixlev = (ch->tdis || ch->lev) && (ch->ndis || ay->chanN.lev);
	if (ch->een) {
		if (mixlev) {
			vol = ymDACvol[ay->chanE.vol & 0x1f];
			if (ch->per < 0x60) vol >>= 1;
		}
	} else {
		if ((ch->per < 0x60) || mixlev) {
			vol = ymDACvol[ch->vol & 0x1f];
		}
	}
#else
	int lev = (ch->per < 0x60) ? 1 : ch->lev;
	if ((ch->tdis || lev) && (ch->ndis || ay->chanN.lev)) {
		vol = ch->een ? ay->chanE.vol : (ch->ndis && !ch->tdis && !lev) ? 0 : ch->vol;
	} else {
		vol = 0;
	}
	vol = ymDACvol[vol & 0x1f];						// YM:5-bit DAC volume
//	if (ch->per < 0x60) vol >>= 1;
#endif
	return vol;
}

sndPair ym_vol(aymChip* chip) {
	int volA = ym_chan_vol(chip, &chip->chanA);
	int volB = ym_chan_vol(chip, &chip->chanB);
	int volC = ym_chan_vol(chip, &chip->chanC);
	return ay_mix_stereo(volA, volB, volC, chip->stereo);
}
