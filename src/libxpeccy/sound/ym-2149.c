#include "ayym.h"

extern int ayDACvol[32];

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
	int vol;
	if (((ch->lev && ch->ten) || (ch->nen && ay->chanN.lev)) || !(ch->ten || ch->nen)) {
		vol = ch->een ? ay->chanE.vol : ch->vol;
	} else {
		vol = 0;
	}
	return ayDACvol[vol & 0x1f];						// YM:5-bit DAC volume
}

sndPair ym_vol(aymChip* chip) {
	int volA = ym_chan_vol(chip, &chip->chanA);
	int volB = ym_chan_vol(chip, &chip->chanB);
	int volC = ym_chan_vol(chip, &chip->chanC);
	return ay_mix_stereo(volA, volB, volC, chip->stereo);
}
