#ifndef _GBSOUND_H
#define _GBSOUND_H

#include "sndcommon.h"

typedef struct {
	unsigned enable:1;
	bitChan* ch1;
	bitChan* ch2;
	bitChan* ch3;
	bitChan* ch4;
} gbSound;

gbSound* gbsCreate();
void gbsDestroy(gbSound*);
void gbsSync(gbSound*, int);
sndPair gbsVolume(gbSound*);

#endif
