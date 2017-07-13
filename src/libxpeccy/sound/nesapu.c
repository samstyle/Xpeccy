#include "nesapu.h"

#include <stdlib.h>
#include <string.h>

nesAPU* apuCreate() {
	nesAPU* apu = (nesAPU*)malloc(sizeof(nesAPU));
	memset(apu, 0x00, sizeof(nesAPU));
	return apu;
}

void apuDestroy(nesAPU* apu) {
	if (apu) free(apu);
}


void apuChanTick(nesPulseChan* ch) {
	if (ch->counter > 0) {
		ch->counter--;
		if (ch->counter < 1) {
			ch->lev ^= 1;
			ch->counter = ch->lev ? ch->per1 : ch->per0;
		}
	}
}

void apuSync(nesAPU* apu, int ns) {
	apu->tick -= ns;
	if (apu->tick > 0) return;
	apu->tick += 1e9/192;		// ns for 192Hz tick
	apuChanTick(&apu->ch0);
	apuChanTick(&apu->ch1);
}
