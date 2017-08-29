#include "nesapu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern char noizes[0x20000];

nesAPU* apuCreate(extmrd cb, void* d) {
	nesAPU* apu = (nesAPU*)malloc(sizeof(nesAPU));
	memset(apu, 0x00, sizeof(nesAPU));
	apu->mrd = cb;
	apu->data = d;
	return apu;
}

void apuDestroy(nesAPU* apu) {
	if (apu) free(apu);
}

void apuResetChan(apuChannel* ch) {
	ch->en = 0;
	ch->len = 0;
	ch->hper = 0;
}

void apuReset(nesAPU* apu) {
	apuResetChan(&apu->ch0);
	apuResetChan(&apu->ch1);
	apuResetChan(&apu->cht);
	apuResetChan(&apu->chn);
	apuResetChan(&apu->chd);
	apu->cht.vol = 0;
	apu->cht.dir = 1;
}

// duty

static int dutyTab[4] = {1,2,4,6};

void apuToneDuty(apuChannel* ch) {
	ch->per1 = (ch->hper + 1) * dutyTab[ch->duty] / 4;
	ch->per0 = (ch->hper + 1) * 2 - ch->per1;
}

// tone channel

void apuToneSync(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	ch->pcnt--;
	if ((ch->pcnt < 0) && ch->hper) {
		ch->lev ^= 1;
		ch->pcnt += ch->lev ? ch->per1 : ch->per0;
	}
}

void apuToneSweep(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	if (!ch->sweep) return;
	int scha;
	ch->scnt--;
	if (ch->scnt == 0) {
		ch->scnt = ch->sper;
		if (ch->sshi) {
			scha = (ch->hper & 0x7ff) >> ch->sshi;
			if (ch->sdir) scha = -scha;
			if ((ch->hper > 8) && ((ch->hper + scha) < 0x7ff))
				ch->hper += scha;
			else
				ch->len = 0;
			apuToneDuty(ch);
		}
	}
}

void apuToneEnv(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	if (!ch->env) return;
	ch->ecnt--;
	if (ch->ecnt < 0) {
		ch->ecnt = ch->eper;
		if (ch->vol > 0) {
			ch->vol--;
		} else if (ch->elen) {
			ch->len = 0;
		} else {
			ch->vol = 0x0f;
		}
	}
}

void apuToneLen(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->elen) return;
	if (ch->len > 0)
		ch->len--;
}

int apuToneVolume(apuChannel* ch) {
	if (ch->off) return 0;
//	if (!ch->en) return 0;
//	if (!ch->len) return 0;
	return ch->lev ? (ch->vol << 1) : 0;
}

// triangle

void apuTriSync(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	if (!ch->lcnt) return;
	ch->pcnt--;
	if (ch->pcnt < 0) {
		ch->pcnt = ch->hper;
		if (ch->dir) {
			if (ch->vol < 0x0f) {
				ch->vol++;
			} else {
				ch->dir = 0;
			}
		} else {
			if (ch->vol > 0) {
				ch->vol--;
			} else {
				ch->dir = 1;
			}
		}
	}
}

void apuTriLen(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->elen) return;
	if (ch->len > 0)
		ch->len--;
}

void apuTriLinear(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->elen) return;
	if (ch->sweep) {
		ch->sweep = 0;
		ch->lcnt = ch->lval;
	} else if (ch->lcnt > 0) {
		ch->lcnt++;
	}
}

int apuTriVolume(apuChannel* ch) {
	if (ch->off) return  0;
//	if (!ch->en) return 0;
//	if (!ch->len) return 0;
//	if (!ch->lcnt) return 0;
	return ch->vol << 2;
}

// noise

void apuNoiseSync(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	ch->pcnt--;
	if (ch->pcnt < 0) {
		ch->pcnt = ch->hper;
		ch->pstp++;
		ch->lev = noizes[ch->pstp & 0x1ffff] ? 1 : 0;
	}
}

// digital

void apuDigiSync(apuChannel* ch, extmrd mrd, void* data) {
	if (!ch->en) return;
	if (!ch->len) return;
	ch->pcnt--;
	if (ch->pcnt < 0) {
		ch->pcnt = ch->hper;			// restore period

		if ((ch->scnt & 7) == 0) {		// let it be bits counter (b0,1,2)
			ch->buf = mrd((ch->cadr & 0x7fff) | 0x8000, data);
			ch->cadr++;
			ch->len--;			// length in bytes
		}

		if (ch->buf & 0x80) {			// inc/dec volume level
			if (ch->vol < 0x7f) ch->vol++;
		} else {
			if (ch->vol > 0x00) ch->vol--;
		}
		ch->scnt++;		// no matter ++ or --
		ch->buf <<= 1;

		if (ch->len == 0) {
			if (ch->elen) {		// loop
				ch->len = ch->lcnt;
				ch->cadr = ch->sadr;
			} else if (ch->env) {	// irq enable
				ch->irq = 1;
			}
		}
	}
}

int apuDigiVolume(apuChannel* ch) {
	if (ch->off) return 0;
	return  ch->vol;			// 00..7f -> 00..3f
}

// ...

void apuSync(nesAPU* apu, int ns) {
	// Waveform generator clock = CPU/16
//	apu->time += ns;
	apu->wcnt -= ns;
	while (apu->wcnt < 0) {
		// 222 KHz here (CPU/8)
//		printf("apu frm time : %i ns (%f Hz)\n",apu->time,1e9/apu->time);
//		apu->time = 0;
		apu->wcnt += apu->wper;
		apu->wstp++;
		apuToneSync(&apu->ch0);
		apuToneSync(&apu->ch1);
		apuNoiseSync(&apu->chn);
		apuDigiSync(&apu->chd, apu->mrd, apu->data);
		if (!(apu->wstp & 3))			// triangle step = CPU/32
			apuTriSync(&apu->cht);
	}
	if (apu->chd.irq) {
		apu->chd.irq = 0;
		apu->dirq = 1;
	}

	// 240Hz clock
	apu->tcnt -= ns;
	while (apu->tcnt < 0) {
		apu->tcnt += apu->tper;
		if (apu->tstp > 0)
			apu->tstp--;
		else
			apu->tstp = apu->step5 ? 4 : 3;

		if (apu->tstp < 4) {		// tick 5 on pal mode is skiped
			// each tick : envelope & trinagle linear counter
			// 240Hz (192Hz)
			apuToneEnv(&apu->ch0);
			apuToneEnv(&apu->ch1);
			apuToneEnv(&apu->chn);
			apuTriLinear(&apu->cht);
			// each %xx1 tick : sweep
			if (apu->tstp & 1) {				// 120Hz (96Hz)
				apuToneSweep(&apu->ch0);
				apuToneSweep(&apu->ch1);
			}
			// each %x11 tick : length counters & frame IRQ (step5 must be 0)
			if ((apu->tstp & 3) == 3) {			// correct (tested): 60Hz (48Hz)
				apuToneLen(&apu->ch0);
				apuToneLen(&apu->ch1);
				apuTriLen(&apu->cht);
				apuToneLen(&apu->chn);
				if (!apu->step5 && apu->irqen) {
					apu->firq = 1;
				}
			}
		}
	}
}

// TODO: http://wiki.nesdev.com/w/index.php/APU_Mixer

sndPair apuVolume(nesAPU* apu) {
	sndPair res;
	int lev = apuToneVolume(&apu->ch0);		// tone channel 0
	res.left = lev;
	res.right = lev;
	lev = apuToneVolume(&apu->ch1);			// tone channel 1
	res = mixer(res, lev, lev, 100);
	lev = apuTriVolume(&apu->cht);			// triangle channel
	res = mixer(res, lev, lev, 100);
	lev = apuToneVolume(&apu->chn);			// noise
	res = mixer(res, lev, lev, 100);
	lev = apuDigiVolume(&apu->chd);
	res = mixer(res, lev, lev, 100);
//	res.left <<= 2;
//	res.right <<= 2;
	return res;
}

// taked from nesdev wiki
static int apuNoisePer[16] = {0x02,0x04,0x08,0x10,0x20,0x30,0x40,0x50,0x65,0x7F,0xBE,0xFE,0x17D,0x1FC,0x3F9,0x7F2};
static int apuPcmPer[16] = {54, 48, 42, 40, 36, 32, 28, 27, 24, 20, 18, 16, 13, 11, 9, 7};				// this is APU ticks (CPU/8)
static int apuLenPAL[8] = {5,10,20,40,80,30,7,13};
static int apuLenNTSC[8] = {6,12,24,48,96,36,8,16};
static int apuLenGeneral[16] = {127,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

int apuGetLC(unsigned char val) {
	int len;
	switch (val & 0x88) {
		case 0x00: len = apuLenPAL[(val >> 4) & 7]; break;
		case 0x80: len = apuLenNTSC[(val >> 4) & 7]; break;
		default: len = apuLenGeneral[(val >> 4) & 15]; break;
	}
	return len;
}

// write to registers 00..13

void apuWrite(nesAPU* apu, int reg, unsigned char val) {
	switch (reg & 0x1f) {
		case 0x00:
			apu->ch0.duty = (val >> 6) & 3;
			apu->ch0.env = (val & 0x10) ? 0 : 1;
			apu->ch0.elen = (val & 0x20) ? 0 : 1;
			if (apu->ch0.env) {
				//printf("ch0 env %.2X\n",val);
				apu->ch0.eper = ((val & 15) + 1);
				apu->ch0.ecnt = apu->ch0.eper;
				apu->ch0.vol = 0x0f;
			} else {
				apu->ch0.vol = val & 15;
			}
			apuToneDuty(&apu->ch0);
			break;
		case 0x01:
//				printf("ch0 sweep %.2X\n",val);
			apu->ch0.sweep = (val & 0x80) ? 1 : 0;
			apu->ch0.sper = ((val >> 4) & 7) + 1;
			apu->ch0.sdir = (val & 0x08) ? 1 : 0;
			apu->ch0.sshi = val & 7;
			apu->ch0.scnt = apu->ch0.sper;
			break;
		case 0x02:
			apu->ch0.hper &= 0x700;
			apu->ch0.hper |= (val & 0xff);
			apuToneDuty(&apu->ch0);
			break;
		case 0x03:
			//printf("wr 4003,%.2X\n",val);
			apu->ch0.hper &= 0xff;
			apu->ch0.hper |= ((val << 8) & 0x0700);
			switch (val & 0x88) {
				case 0x00: apu->ch0.len = apuLenPAL[(val >> 4) & 7]; break;
				case 0x80: apu->ch0.len = apuLenNTSC[(val >> 4) & 7]; break;
				default: apu->ch0.len = apuLenGeneral[(val >> 4) & 15]; break;
			}
			apuToneDuty(&apu->ch0);
			apu->ch0.pcnt = apu->ch0.lev ? apu->ch0.per1 : apu->ch0.per0;
			apu->ch0.ecnt = apu->ch0.eper;
			//printf("CH0 len = %i, p0 = %i, p1 = %i\n", apu->ch0.len, apu->ch0.per0, apu->ch0.per1);
			break;
		// ch1 : tone 1
		case 0x04:
			apu->ch1.duty = (val >> 6) & 3;
			apu->ch1.env = (val & 0x10) ? 0 : 1;
			apu->ch1.elen = (val & 0x20) ? 0 : 1;
			if (apu->ch1.env) {
				apu->ch1.eper = ((val & 15) + 1);
				apu->ch1.ecnt = apu->ch1.eper;
				apu->ch1.vol = 0x0f;
			} else {
				apu->ch1.vol = val & 15;
			}
			apuToneDuty(&apu->ch1);
			break;
		case 0x05:
			apu->ch1.sweep = (val & 0x80) ? 1 : 0;
			apu->ch1.sper = ((val >> 4) & 7) + 1;
			apu->ch1.sdir = (val & 0x08) ? 1 : 0;
			apu->ch1.sshi = val & 7;
			apu->ch1.scnt = apu->ch1.sper;
			break;
		case 0x06:
			apu->ch1.hper &= 0x700;
			apu->ch1.hper |= val;
			apuToneDuty(&apu->ch1);
			break;
		case 0x07:
			apu->ch1.hper &= 0xff;
			apu->ch1.hper |= ((val << 8) & 0x0700);
			apu->ch1.len = apuGetLC(val);
			apuToneDuty(&apu->ch1);
			apu->ch1.pcnt = apu->ch1.lev ? apu->ch1.per1 : apu->ch1.per0;
			apu->ch1.ecnt = apu->ch1.eper;
			break;
		// ch2 : triangle
		case 0x08:
			apu->cht.elen = (val & 0x80) ? 0 : 1;
			apu->cht.lcnt = (val & 0x7f);
			apu->cht.lval = (val & 0x7f);
			break;
		case 0x09:
			break;
		case 0x0a:
			apu->cht.hper &= 0x700;
			apu->cht.hper |= val & 0xff;
			apu->cht.pcnt = apu->cht.hper;
			break;
		case 0x0b:
			apu->cht.hper &= 0xff;
			apu->cht.hper |= (val << 8) & 0x700;
			apu->cht.len = apuGetLC(val);
			apu->cht.sweep = 1;
			break;
		// ch3 : noise
		case 0x0c:
			apu->chn.elen = (val & 0x20) ? 0 : 1;
			apu->chn.env = (val & 0x10) ? 0 : 1;
			if (apu->chn.env) {
				apu->chn.eper = (val & 15) + 1;
				apu->chn.ecnt = apu->chn.eper;
				apu->chn.vol = 0x0f;
			} else {
				apu->chn.vol = val & 15;
			}
			break;
		case 0x0d:
			break;
		case 0x0e:
			apu->chn.hper = apuNoisePer[val & 15];
			apu->chn.pcnt = apu->chn.hper;
			// b7:loop noise (short generator)
			break;
		case 0x0f:
			apu->chn.len = apuGetLC(val);
			break;
		// ch4 : dmc
		case 0x10:
			apu->chd.env = (val & 0x80) ? 1 : 0;		// IRQ enable
			apu->chd.elen = (val & 0x40) ? 1 : 0;		// loop
			apu->chd.hper = apuPcmPer[val & 0x0f];		// period
			apu->chd.pcnt = apu->chd.hper;
			break;
		case 0x11:
			apu->chd.vol = val & 0x7f;
			break;
		case 0x12:
			apu->chd.sadr = 0xc000 | ((val << 6) & 0x3fc0);
			apu->chd.cadr = apu->chd.sadr;
			break;
		case 0x13:
			apu->chd.lcnt = ((val << 4) & 0x0ff0) | 1;	// this is length in bytes
			apu->chd.len = apu->chd.lcnt;
			break;
	}
}
