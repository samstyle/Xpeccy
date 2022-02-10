#include "nesapu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// http://wiki.nesdev.com/w/index.php/APU

nesAPU* apuCreate(aextmrd cb, cbirq ci, void* d) {
	nesAPU* apu = (nesAPU*)malloc(sizeof(nesAPU));
	memset(apu, 0x00, sizeof(nesAPU));
	apu->mrd = cb;
	apu->xirq = ci;
	apu->data = d;
	apu->wdiv = 3729;
	return apu;
}

void apuDestroy(nesAPU* apu) {
	if (apu)
		free(apu);
}

void apuResetChan(apuChannel* ch) {
	ch->en = 0;
	ch->len = 0;
	ch->lval = 0;
	ch->lcnt = 0;
	ch->hper = 0;
	ch->eper = 0;
	ch->mute = 0;
	ch->vol = 0;
	ch->evol = 0x0f;
	ch->dir = 1;
	ch->nseed = 1;
}

void apuReset(nesAPU* apu) {
	apuResetChan(&apu->ch0);
	apuResetChan(&apu->ch1);
	apuResetChan(&apu->cht);
	apuResetChan(&apu->chn);
	apuResetChan(&apu->chd);
	apu->step5 = 0;
	apu->wcnt = 0;
	apu->wstp = 0;
	apu->tstp = 0;
}

// duty

static int dutyTab[4][8] = {
	{0,1,0,0,0,0,0,0},
	{0,1,1,0,0,0,0,0},
	{0,1,1,1,1,0,0,0},
	{1,0,0,1,1,1,1,1}
};

void apuTargetPeriod(apuChannel* ch) {
	int sha = ch->hper >> (ch->sshi & 7);
	if (ch->sdir) sha = -sha;
	ch->tper = ch->hper + sha;	// next sweep period
	// if (ch->sweep) printf("%i > %i\n",ch->hper,ch->tper);
	if (ch->sdir && (ch->hper < 8)) {
		ch->mute = 1;
		ch->sweep = 0;
	} else if (!ch->sdir && (ch->tper > 0x7ff)) {
		ch->mute = 1;
		ch->sweep = 0;
	} else {
		ch->mute = 0;
	}
}

// tone channel

void apu_tone_tick(apuChannel* ch) {
	if (--ch->pcnt < 1) {
		ch->pcnt = ch->hper;
		ch->pstp++;
	}
}

void apuToneSweep(apuChannel* ch) {
	if (!ch->sweep) return;
	if (--ch->scnt < 1) {
		ch->scnt = ch->sper;
		ch->hper = ch->tper;		// load new freq
		apuTargetPeriod(ch);		// calculate next freq
	}
}

void apuToneEnv(apuChannel* ch) {
	if (!ch->env) return;
	if (--ch->ecnt < 1) {
		ch->ecnt = ch->eper;
		if (ch->evol > 0) {
			ch->evol--;
		} else if (ch->elen) {		// elen: 1 if envelope loop
			ch->evol = 0x0f;
		}
	}
}

void apuToneLen(apuChannel* ch) {
	if (!ch->elen) return;
	if (ch->len > 0)
		ch->len--;
}

int apuToneVolume(apuChannel* ch) {
	if (!ch->off && ch->en && ch->len && ch->hper && !ch->mute && (ch->hper > 7)) {
		ch->lev = dutyTab[~ch->duty & 3][ch->pstp & 7] & 1;
		ch->out = ch->lev ? (ch->env ? ch->evol : ch->vol) : 0;
	}
	return ch->out;		// 00..0f
}

// triangle

static unsigned char triSeq[32] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void apuTriSync(apuChannel* ch) {
	if (!ch->en) return;
	if (!ch->len) return;
	if (!ch->lcnt) return;
	if (--ch->pcnt < 1) {
		ch->pcnt = ch->hper;
		ch->pstp++;
		ch->vol = triSeq[ch->pstp & 0x1f];
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
	if (ch->sweep) {		// linear counter reload flag
		ch->sweep = 0;
		ch->lcnt = ch->lval;
	} else if (ch->lcnt > 0) {
		ch->lcnt--;
	}
	if (ch->elen)			// control flag = 0 (inverse = 1)
		ch->sweep = 0;
}

int apuTriVolume(apuChannel* ch) {
	if (!ch->off && ch->en && ch->lcnt && ch->len) {
		ch->out = ch->vol;
	}
	return ch->out;			// 00..0f
}

// noise

void apuNoiseSync(apuChannel* ch) {
	int fbk;
	if (ch->mode) {
		fbk = ((ch->nseed ^ (ch->nseed >> 6)) & 1) ? 0x4000 : 0;
	} else {
		fbk = ((ch->nseed ^ (ch->nseed >> 1)) & 1) ? 0x4000 : 0;
	}
	ch->nseed >>= 1;
	ch->nseed |= fbk;
	if (!ch->en) return;
	if (!ch->len) return;
	if (--ch->pcnt < 1) {
		ch->pcnt = ch->hper;
		ch->lev = fbk ? 1 : 0;
	}
}

int apuNoiseVolume(apuChannel* ch) {
	if (!ch->off && ch->en && ch->len && ch->hper && !ch->mute) {
		ch->out = ch->lev ? (ch->env ? ch->evol : ch->vol) : 0;
	}
	return ch->out;		// 00..0f
}

// digital

void apuDigiSync(apuChannel* ch, aextmrd mrd, void* data) {
	if (!ch->en) return;
	if (!ch->len) return;
	if (--ch->pcnt < 1) {
		ch->pcnt = ch->hper;

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
			} else {		// irq enable
				ch->irq = ch->env;
			}
		}
	}
}

int apuDigiVolume(apuChannel* ch) {
	if (!ch->off) {
		ch->out = ch->vol;
	}
	return ch->out;
}

// ...

static const int seqMode0[4] = {1,3,1,7};		// e--|el-|e--|elf
static const int seqMode1[5] = {3,1,3,1,0};		// el-|e--|el-|e--|---

void apuFlush(nesAPU* apu) {}

// tone
// F = Fcpu / (16 * (t + 1))	1 channel tick = 16 * (t + 1) cpu ticks = 8 * (t + 1) wf ticks; (t + 1) because of 8-step duty

void apuSync(nesAPU* apu, int ns) {
	int tmp;
// Waveform generator clock = CPU/2	~890KHz (NTSC)
	apu->wcnt -= ns;
	while (apu->wcnt < 0) {
		apu->wcnt += apu->wper;
		apu->wstp++;
		// sync all channels
		apu_tone_tick(&apu->ch0);
		apu_tone_tick(&apu->ch1);
		apuNoiseSync(&apu->chn);			// noise channel
		apuDigiSync(&apu->chd, apu->mrd, apu->data);	// pcm channel
		apuTriSync(&apu->cht);
		// pcm irq
		if (apu->chd.irq) {
			apu->chd.irq = 0;
			apu->dirq = 1;
			apu->xirq(IRQ_APU, apu->data);
		}
		// frame counter
		if ((apu->wstp % apu->wdiv) == 0) {				// 14915 / 4 = ~3729 : ~240Hz here (NTSC)
			//printf("%i ns (%f Hz)\n",apu->time, 1e9/apu->time); apu->time = 0;	// 4136450ns = 240Hz (tested)
			apu->tstp++;
			if (apu->step5) {
				tmp = seqMode1[apu->tstp % 5];
			} else {
				tmp = seqMode0[apu->tstp % 4];
			}
			if (tmp & 1) {		// 240Hz (192Hz)
				apuToneEnv(&apu->ch0);
				apuToneEnv(&apu->ch1);
				apuToneEnv(&apu->chn);
				apuTriLinear(&apu->cht);
			}			// 120Hz (96Hz)
			if (tmp & 2) {
				apuToneSweep(&apu->ch0);
				apuToneSweep(&apu->ch1);
				apuToneLen(&apu->ch0);
				apuToneLen(&apu->ch1);
				apuTriLen(&apu->cht);
				apuToneLen(&apu->chn);
			}
			if (tmp & 4) {		// 60Hz (48Hz)
				if (apu->irqen)
					apu->xirq(IRQ_APU, apu->data);
				apu->firq = apu->irqen;
			}
		}
	}
}

// TODO: http://wiki.nesdev.com/w/index.php/APU_Mixer

sndPair apuVolume(nesAPU* apu) {
//	apuFlush(apu);
	sndPair res;
	int v1 = apuToneVolume(&apu->ch0);
	int v2 = apuToneVolume(&apu->ch1);
	float pout = 0.0;
	if (v1 || v2)
		pout = 95.88 / (100.0 + (8128.0 / (v1 + v2)));			// 0,2584 max
	v1 = apuTriVolume(&apu->cht);
	v2 = apuNoiseVolume(&apu->chn);
	int v3 = apuDigiVolume(&apu->chd);
	float tnd = 0.0;
	if (v1 || v2 || v3)
		tnd = 159.79 / (100.0 + (1.0 / ((v1 / 8227.0) + (v2 / 12241.0) + (v3 / 22638.0))));		// 0,8686 max
	res.left = (int)((pout + tnd) * 0x4000);
	res.right = res.left;
	return res;
}

// taked from nesdev wiki
static int apuNoisePer[16] = {0x02,0x04,0x08,0x10,0x20,0x30,0x40,0x50,0x65,0x7F,0xBE,0xFE,0x17D,0x1FC,0x3F9,0x7F2};
static int apuPcmPer[16] = {54, 48, 42, 40, 36, 32, 28, 27, 24, 20, 18, 16, 13, 11, 9, 7};
static int apuLenTab[32] = {10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30};

int apuGetLen(apuChannel* ch, unsigned char val) {
	if (!ch->en) return 0;
	return apuLenTab[(val >> 3) & 0x1f];
}

// write to registers 00..13

void apuWrite(nesAPU* apu, int reg, int val) {
	// printf("%.2X = %.2X\n",reg,val);
	// apuFlush(apu);
	switch (reg & 0x1f) {
		case 0x00:
			apu->ch0.duty = (val >> 6) & 3;
			apu->ch0.env = (val & 0x10) ? 0 : 1;
			apu->ch0.elen = (val & 0x20) ? 0 : 1;
			if (apu->ch0.env) {
				//printf("ch0 env %.2X\n",val);
				apu->ch0.eper = ((val & 15) + 1);
				apu->ch0.ecnt = apu->ch0.eper;
				apu->ch0.evol = 0x0f;
			} else {
				apu->ch0.vol = val & 15;
			}
			break;
		case 0x01:
//			printf("ch0 sweep %.2X\n",val);
			apu->ch0.sweep = (val & 0x80) ? 1 : 0;
			apu->ch0.sper = ((val >> 4) & 7) + 1;
			apu->ch0.sdir = (val & 0x08) ? 1 : 0;
			apu->ch0.sshi = val & 7;
			apu->ch0.scnt = apu->ch0.sper;
			break;
		case 0x02:			// update only on writing high byte?
			apu->ch0.hper &= 0x700;
			apu->ch0.hper |= (val & 0xff);
			apuTargetPeriod(&apu->ch0);
			break;
		case 0x03:
			//printf("3:2X\n",val);
			apu->ch0.hper &= 0xff;
			apu->ch0.hper |= ((val << 8) & 0x0700);
			apu->ch0.len = apuGetLen(&apu->ch0, val);
			apu->ch0.pstp = 0;			// reset tone phase
			apu->ch0.pcnt = apu->ch0.hper;
//			apu->ch0.ecnt = apu->ch0.eper;		// restart envelope
//			apu->ch0.evol = 0x0f;
			apuTargetPeriod(&apu->ch0);
			break;
		// ch1 : tone 1
		case 0x04:
//			printf("4:%.2X\n",val);
			apu->ch1.duty = (val >> 6) & 3;
			apu->ch1.env = (val & 0x10) ? 0 : 1;
			apu->ch1.elen = (val & 0x20) ? 0 : 1;
			if (apu->ch1.env) {
				apu->ch1.eper = ((val & 15) + 1);
				apu->ch1.ecnt = apu->ch1.eper;
				apu->ch1.evol = 0x0f;
			} else {
				apu->ch1.vol = val & 15;
			}
			break;
		case 0x05:
			//printf("5:%.2X\n",val);
			apu->ch1.sweep = (val & 0x80) ? 1 : 0;
			apu->ch1.sper = ((val >> 4) & 7) + 1;
			apu->ch1.sdir = (val & 0x08) ? 1 : 0;
			apu->ch1.sshi = val & 7;
			apu->ch1.scnt = apu->ch1.sper;
			break;
		case 0x06:
			//printf("6:%.2X\n",val);
			apu->ch1.hper &= 0x700;
			apu->ch1.hper |= val;
			apuTargetPeriod(&apu->ch1);
			break;
		case 0x07:
			//printf("7:%.2X\n",val);
			apu->ch1.hper &= 0xff;
			apu->ch1.hper |= ((val << 8) & 0x0700);
			apu->ch1.len = apuGetLen(&apu->ch1, val);
			apu->ch1.pstp = 0;
			apu->ch1.pcnt = apu->ch1.hper;
//			apu->ch1.ecnt = apu->ch1.eper;
//			apu->ch1.evol = 0x0f;
			apuTargetPeriod(&apu->ch1);
			break;
		// ch2 : triangle
		case 0x08:
			apu->cht.elen = (val & 0x80) ? 0 : 1;		// inverse control flag
			apu->cht.lcnt = (val & 0x7f);
			apu->cht.lval = (val & 0x7f);
			break;
		case 0x09:
			break;
		case 0x0a:
			apu->cht.hper &= 0x700;
			apu->cht.hper |= val & 0xff;
			break;
		case 0x0b:
			apu->cht.hper &= 0xff;
			apu->cht.hper |= (val << 8) & 0x700;
			apu->cht.pcnt = apu->cht.hper;
			apu->cht.len = apuGetLen(&apu->cht, val);
			apu->cht.sweep = 1;
			break;
		// ch3 : noise
		case 0x0c:
			apu->chn.elen = (val & 0x20) ? 0 : 1;
			apu->chn.env = (val & 0x10) ? 0 : 1;
			if (apu->chn.env) {
				apu->chn.eper = (val & 15) + 1;
				apu->chn.ecnt = apu->chn.eper;
				apu->chn.evol = 0x0f;
			} else {
				apu->chn.vol = val & 15;
			}
			break;
		case 0x0d:
			break;
		case 0x0e:
			apu->chn.hper = apuNoisePer[val & 15];
			apu->chn.pcnt = apu->chn.hper;
			apu->chn.mode = (val & 0x80) ? 1 : 0;
			break;
		case 0x0f:
			apu->chn.len = apuGetLen(&apu->chn, val);
			break;
		// ch4 : dmc
		case 0x10:
			apu->chd.env = (val & 0x80) ? 1 : 0;		// IRQ enable
			apu->chd.elen = (val & 0x40) ? 0 : 1;		// loop
			apu->chd.hper = apuPcmPer[val & 0x0f] << 2;	// period
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
