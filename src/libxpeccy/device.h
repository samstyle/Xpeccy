#ifndef _DEVICE_H
#define _DEVICE_H

// This is experimental part
// Attempt to unify external device structure

#if __cplusplus
extern "C" {
#endif

#include "input.h"
#include "tape.h"
#include "fdc.h"
#include "hdd.h"
#include "sdcard.h"

#include "sound/ayym.h"
#include "sound/gs.h"
#include "sound/saa1099.h"
#include "sound/soundrive.h"
#include "sound/gbsound.h"

#define	MAX_DEV_COUNT	16

// device type

enum {
	DEV_NONE = 0,
	DEV_KEYBOARD,	// input
	DEV_JOYSTICK,
	DEV_MOUSE,
	DEV_TAPE,	// storage
	DEV_FDC,
	DEV_IDE,
	DEV_SDCARD,
	DEV_GSOUND,	// sound
	DEV_TSOUND,
	DEV_SAA,
	DEV_SDRIVE
};

// pointer to device

typedef union {
	void* ptr;
	Keyboard* keyb;
	Joystick* joy;
	Mouse* mouse;
	Tape* tape;
	DiskIF* dif;
	IDE* ide;
	SDCard* sdc;
	GSound* gs;
	TSound* ts;
	saaChip* saa;
	SDrive* sdrv;
	gbSound* gbsnd;
} xDevPtr;

// common device struct

typedef struct {
	int type;
	unsigned char(*rd)(xDevPtr, unsigned short);		// receive byte from device
	void(*wr)(xDevPtr, unsigned short, unsigned char);	// send byte to device
	void(*sync)(xDevPtr, unsigned long);			// emulate device work during some time (ns)
	xDevPtr ptr;						// pointer to device-specified data
} xDevice;

#if __cplusplus
}
#endif

#endif
