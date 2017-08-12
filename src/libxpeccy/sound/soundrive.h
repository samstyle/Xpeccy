#ifndef	XSOUNDRIVE
#define	XSOUNDRIVE
// soundrive/covox type

#include "sndcommon.h"

enum {
	SDRV_NONE = 0,
	SDRV_COVOX,	// 1 channel only
	SDRV_105_1,	// mode1: ports 0f,1f,4f,5f
	SDRV_105_2	// mode2: ports f1,f3,f9,fb (default)
};

enum {
	SDRV_ARG_MODE = 0
};

#include "sndcommon.h"

typedef struct {
	int type;
	unsigned char chan[4];
} SDrive;

SDrive* sdrvCreate(int);
void sdrvDestroy(SDrive*);
int sdrvWrite(SDrive*, unsigned short, unsigned char);
sndPair sdrvVolume(SDrive*);

#endif
