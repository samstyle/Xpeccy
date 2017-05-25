#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "soundrive.h"

void* sdCreate() {
	SDrive* sdrv = (SDrive*)malloc(sizeof(SDrive));
	memset(sdrv, 0x00, sizeof(SDrive));
	sdrv->type = SDRV_COVOX;
	return sdrv;
}

void sdDestroy(void* sdrv) {
	free(sdrv);
}

void sdRequest(void* ptr, xDevBus* bus) {
	if (bus->busy) return;		// bus is free
	if (!bus->iorq) return;		// IO only
	if (bus->rd) return;		// write only
	if (bus->dos) return;		// !dos
	SDrive* sdrv = (SDrive*)ptr;
	int ch = -1;
	switch (sdrv->type) {
		case SDRV_COVOX:
			if ((bus->adr & 0xff) == 0xfb)
				ch = 4;			// to all channels
			break;
		case SDRV_105_1:
			if ((bus->adr & 0xaf) == 0x0f)
				ch = ((bus->adr & 0x10) >> 4) | ((bus->adr & 0x40) >> 5);
			break;
		case SDRV_105_2:
			if ((bus->adr & 0xf5) == 0xf1)
				ch = ((bus->adr & 0x02) >> 1) | ((bus->adr & 0x08) >> 2);
			break;
	}
	if (ch < 0) return;
	bus->busy = 1;
	if (ch < 4) {
		sdrv->chan[ch] = bus->data;
	} else {
		sdrv->chan[0] = bus->data;
		sdrv->chan[1] = bus->data;
		sdrv->chan[2] = bus->data;
		sdrv->chan[3] = bus->data;
	}
}

sndPair sdVolume(void* ptr) {
	sndPair res;
	SDrive* sdrv = (SDrive*)ptr;
	res.left = ((sdrv->chan[0] >> 2) + (sdrv->chan[1] >> 2));
	res.right = ((sdrv->chan[2] >> 2) + (sdrv->chan[3] >> 2));
	return res;
}
