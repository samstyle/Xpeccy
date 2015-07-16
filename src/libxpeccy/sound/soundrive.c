#include <stdlib.h>
#include "soundrive.h"

SDrive* sdrvCreate(int type) {
	SDrive* sdrv = (SDrive*)malloc(sizeof(SDrive));
	sdrv->type = type;
	sdrv->chanA = 0;
	sdrv->chanB = 0;
	sdrv->chanC = 0;
	sdrv->chanD = 0;
	return sdrv;
}

void sdrvDestroy(SDrive* sdrv) {
	free(sdrv);
}

void sdrvOut(SDrive* sdrv,unsigned short port,unsigned char val) {
	switch (sdrv->type) {
		case SDRV_COVOX:
			if (port == 0xfb) sdrv->chanA = val;
			break;
		case SDRV_105_1:
			if (port == 0x0f) sdrv->chanA = val;
			if (port == 0x1f) sdrv->chanB = val;
			if (port == 0x4f) sdrv->chanC = val;
			if (port == 0x5f) sdrv->chanD = val;
			break;
		case SDRV_105_2:
			if (port == 0xf1) sdrv->chanA = val;
			if (port == 0xf3) sdrv->chanB = val;
			if (port == 0xf9) sdrv->chanC = val;
			if (port == 0xfb) sdrv->chanD = val;
			break;
		default:
			sdrv->chanA = 0;
			sdrv->chanB = 0;
			sdrv->chanC = 0;
			sdrv->chanD = 0;
			break;
	}
}

sndPair sdrvGetVolume(SDrive* sdrv) {
	sndPair res;
	switch (sdrv->type) {
		case SDRV_COVOX:
			res.left = (sdrv->chanA >> 1);
			res.right = (sdrv->chanA >> 1);
			break;
		case SDRV_105_1:
		case SDRV_105_2:
			res.left = ((sdrv->chanA >> 1) + (sdrv->chanB >> 1));
			res.right = ((sdrv->chanC >> 1) + (sdrv->chanD >> 1));
			break;
		default:
			res.left = 0;
			res.right = 0;
			break;
	}
	return res;
}
