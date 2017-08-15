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

void apuReset(nesAPU* apu) {
	apu->ch0.len = 0;
	apu->ch1.len = 0;
	apu->cht.len = 0;
	apu->chn.len = 0;
	apu->chd.len = 0;
}

// tone channel

void apuToneSync(apuChannel* ch) {
	if (!ch->len) return;
	ch->pcnt--;
	if ((ch->pcnt < 0) && ch->hper) {
		ch->lev ^= 1;
		ch->pcnt += ch->lev ? ch->per1 : ch->per0;
	}
}

static int dutyTab[4] = {1,2,4,6};

void apuToneDuty(apuChannel* ch) {
	ch->per1 = (ch->hper + 1) * dutyTab[ch->duty] / 4;
	ch->per0 = (ch->hper + 1) * 2 - ch->per1;
}

void apuToneSweep(apuChannel* ch) {
	if (!ch->len) return;
	if (!ch->sweep) return;
	int scha;
	ch->scnt--;
	if (ch->scnt == 0) {
		ch->scnt = ch->sper;
		if (ch->sshi) {
			scha = ch->hper >> ch->sshi;
			if (ch->sdir) scha = -scha;
			ch->hper += scha;
			apuToneDuty(ch);
			if ((ch->hper < 8) || (ch->hper > 0x7ff)) {
				ch->len = 0;
			}
		}
	}
}

void apuToneEnv(apuChannel* ch) {
	if (!ch->len) return;
	if (!ch->env) return;
	ch->ecnt--;
	if (ch->ecnt == 0) {
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
	if (!ch->elen) return;
	if (ch->len > 0)
		ch->len--;
}

int apuToneVolume(apuChannel* ch) {
	if (!ch->len) return 0;
	return ch->lev ? ch->vol : 0;
}

// triangle

void apuTriSync(apuChannel* ch) {
	if (!ch->len) return;
	if (!ch->lcnt) return;
	ch->pcnt--;
	if (ch->pcnt > 0) return;
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

void apuTriLen(apuChannel* ch) {
	if (!ch->elen) return;
	if (ch->len > 0)
		ch->len--;
}

int apuTriVolume(apuChannel* ch) {
	if (!ch->len) return 0;
	if (!ch->lcnt) return 0;
	return ch->vol;
}

// noise

void apuNoiseSync(apuChannel* ch) {
	if (!ch->len) return;
	ch->pcnt--;
	if (ch->pcnt > 0) return;
	ch->pcnt = ch->hper;
	ch->pstp++;
	ch->lev = noizes[ch->pstp & 0x1ffff] ? 1 : 0;
}

// digital

void apuDigiSync(apuChannel* ch, extmrd mrd, void* data) {
	if (!ch->len) return;
	ch->pcnt--;
	if (ch->pcnt > 0) return;
	ch->pcnt = ch->hper;
}

// ...

void apuSync(nesAPU* apu, int ns) {
	// Waveform generator clock = CPU/16
	apu->wcnt -= ns;
	while (apu->wcnt < 0) {
		apu->wcnt += apu->wper;
		apu->wstp++;
		apuToneSync(&apu->ch0);
		apuToneSync(&apu->ch1);
		if (!(apu->wstp & 15))
			apuTriSync(&apu->cht);
		if (!(apu->wstp & 15))
			apuNoiseSync(&apu->chn);
		apuDigiSync(&apu->chd, apu->mrd, apu->data);
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
			apuToneEnv(&apu->ch0);
			apuToneEnv(&apu->ch1);
			apuToneEnv(&apu->chn);
			if (apu->cht.lcnt > 0) {
				apu->cht.lcnt--;
			} else {
				apu->cht.len = 0;
			}
			// each %xx1 tick : sweep
			if (apu->tstp & 1) {
				apuToneSweep(&apu->ch0);
				apuToneSweep(&apu->ch1);
			}
			// each %x11 tick : length counters & frame IRQ (step5 must be 0)
			apuToneLen(&apu->ch0);
			apuToneLen(&apu->ch1);
			apuTriLen(&apu->cht);
			apuToneLen(&apu->chn);
			if ((apu->tstp & 3) == 3) {
				if (!apu->step5 && apu->irqen)
					apu->frm = 1;
			}
		}
	}
}

sndPair apuVolume(nesAPU* apu) {
	sndPair res;
	int lev = apuToneVolume(&apu->ch0) << 2;		// tone channel 0
	res.left = lev;
	res.right = lev;
	lev = apuToneVolume(&apu->ch1) << 2;			// tone channel 1
	res = mixer(res, lev, lev, 100);
	lev = apuTriVolume(&apu->cht) << 2;			// triangle channel
	res = mixer(res, lev, lev, 100);
	lev = apuToneVolume(&apu->chn) << 2;			// noise
	res = mixer(res, lev, lev, 100);

	return res;
}
