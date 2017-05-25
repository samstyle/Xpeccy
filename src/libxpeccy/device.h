#ifndef _DEVICE_H
#define _DEVICE_H

// This is experimental part
// Attempt to unify external device structure

#if __cplusplus
extern "C" {
#endif

#define	MAX_DEV_COUNT	32

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

#include "devbus.h"

// pointer to device

#include "input.h"
#include "tape.h"
#include "fdc.h"
#include "hdd.h"
#include "sdcard.h"
#include "cartridge.h"

#include "sound/ayym.h"
#include "sound/gs.h"
#include "sound/saa1099.h"
#include "sound/soundrive.h"
#include "sound/gbsound.h"

typedef union {
	void* ptr;
//	Keyboard* keyb;
//	Joystick* joy;
//	Mouse* mouse;
//	Tape* tape;
//	DiskIF* dif;
//	IDE* ide;
//	SDCard* sdc;
	GSound* gs;
//	TSound* ts;
//	saaChip* saa;
	SDrive* sdrv;
//	gbSound* gbsnd;
} xDevPtr;

// common device struct

typedef struct {
	int type;
	void*(*create)();			// create device data
	void(*destroy)(void*);			// destroy device data
	void(*reset)(void*);			// reset device
	void(*req)(void*, xDevBus*);		// send bus signals
	void(*sync)(void*, int);		// emulate device work during some time (ns)
	void(*flush)(void*);			// flush device
	sndPair(*vol)(void*);			// get volume if device produce sound
	xDevPtr ptr;				// pointer to device-specified data
} xDevice;

xDevice* devCreate(int);
void devDestroy(xDevice*);

#if __cplusplus
}
#endif

#endif
