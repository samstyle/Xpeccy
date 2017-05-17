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
#include "cartridge.h"

#include "sound/ayym.h"
#include "sound/gs.h"
#include "sound/saa1099.h"
#include "sound/soundrive.h"
#include "sound/gbsound.h"

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

// universal bus to device

typedef struct {
	unsigned iorge:1;	// request catched (use to prevent devices conflict)
	unsigned iorq:1;	// io request
	unsigned memrq:1;	// mem request
	unsigned rd:1;		// read (device will return data in the 'data' field)
	unsigned wr:1;		// write
	unsigned short adr;	// address bus (16 bit)
	unsigned char data;	// data bus (8 bit)
} xDevBus;

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
	xDevPtr(*create)();
	void(*destroy)(xDevPtr);
	void(*reset)(xDevPtr);
	int(*rd)(xDevPtr, xDevBus*);		// read byte from device
	int(*wr)(xDevPtr, xDevBus*);		// write byte to device
	void(*sync)(xDevPtr, int);		// emulate device work during some time (ns)
	xDevPtr ptr;				// pointer to device-specified data
} xDevice;

xDevice* devCreate(int);
void devDestroy(xDevice*);

#if __cplusplus
}
#endif

#endif
