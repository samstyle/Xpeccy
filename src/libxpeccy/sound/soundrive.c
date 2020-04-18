#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "soundrive.h"

SDrive* sdrvCreate(int type) {
	SDrive* sdrv = (SDrive*)malloc(sizeof(SDrive));
	memset(sdrv, 0x00, sizeof(SDrive));
	sdrv->type = type;
	return sdrv;
}

void sdrvDestroy(SDrive* sdrv) {
	free(sdrv);
}

int sdrvWrite(SDrive* sdrv, int adr, int data) {
	int ch = -1;
	switch (sdrv->type) {
		case SDRV_COVOX:
			if ((adr & 0xff) == 0xfb)
				ch = 4;			// to all channels
			break;
		case SDRV_105_1:
			if ((adr & 0xaf) == 0x0f)
				ch = ((adr & 0x10) >> 4) | ((adr & 0x40) >> 5);
			break;
		case SDRV_105_2:
			if ((adr & 0xf5) == 0xf1)
				ch = ((adr & 0x02) >> 1) | ((adr & 0x08) >> 2);
			break;
	}
	if (ch < 0) return 0;
	if (ch < 4) {
		sdrv->chan[ch] = data;
	} else {
		sdrv->chan[0] = data;
		sdrv->chan[1] = data;
		sdrv->chan[2] = data;
		sdrv->chan[3] = data;
	}
	return 1;
}

sndPair sdrvVolume(SDrive* sdrv) {
	sndPair res;
	res.left = ((sdrv->chan[0] << 3) + (sdrv->chan[1] << 3));
	res.right = ((sdrv->chan[2] << 3) + (sdrv->chan[3] << 3));
	return res;
}
