#ifndef _SNDCOMMON_H
#define _SNDCOMMON_H

typedef struct {
	unsigned char left;
	unsigned char right;
} sndPair;

extern int noizes[0x20000];

#include "ayym.h"
#include "saa1099.h"
#include "soundrive.h"		// covox/soundrive
#include "gs.h"

#endif
