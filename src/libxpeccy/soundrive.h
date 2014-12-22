#ifndef	_XSOUNDRIVE
#define	_XSOUNDRIVE
// soundrive/covox type
#define	SDRV_NONE	0
#define	SDRV_COVOX	1	// 1 channel only
#define	SDRV_105_1	2	// mode1: ports 0f,1f,4f,5f
#define	SDRV_105_2	3	// mode2: ports f1,f3,f9,fb (default)
// soundrive channels
#define	SDRV_CHANA	0
#define	SDRV_CHANB	1
#define	SDRV_CHANC	2
#define	SDRV_CHAND	3

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
