#ifndef	_XSOUNDRIVE
#define	_XSOUNDRIVE
// soundrive/covox type

#include "sndcommon.h"
#include "../devbus.h"

enum {
	SDRV_NONE = 0,
	SDRV_COVOX,	// 1 channel only
	SDRV_105_1,	// mode1: ports 0f,1f,4f,5f
	SDRV_105_2	// mode2: ports f1,f3,f9,fb (default)
};

#include "sndcommon.h"

typedef struct {
	int type;
	unsigned char chan[4];
} SDrive;

void* sdCreate();
void sdDestroy(void*);
void sdRequest(void*, xDevBus*);
sndPair sdVolume(void*);

#endif
