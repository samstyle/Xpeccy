#ifndef	_XSOUNDRIVE
#define	_XSOUNDRIVE
// soundrive/covox type
enum {
	SDRV_NONE = 0,
	SDRV_COVOX,	// 1 channel only
	SDRV_105_1,	// mode1: ports 0f,1f,4f,5f
	SDRV_105_2	// mode2: ports f1,f3,f9,fb (default)
};
// soundrive channels
enum {
	SDRV_CHANA = 0,
	SDRV_CHANB,
	SDRV_CHANC,
	SDRV_CHAND,
};

#include "sndcommon.h"

typedef struct {
	int type;
	unsigned char chanA;
	unsigned char chanB;
	unsigned char chanC;
	unsigned char chanD;
} SDrive;

SDrive* sdrvCreate(int);
void sdrvDestroy(SDrive*);
void sdrvOut(SDrive*,unsigned short,unsigned char);
sndPair sdrvGetVolume(SDrive*);

#endif
