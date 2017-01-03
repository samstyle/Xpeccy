#ifndef _GBC_VID_H
#define _GBC_VID_H

#include "vidcommon.h"

struct GBCVid {
	unsigned lcdon:1;
	unsigned winen:1;		// window layer enabled
	unsigned spren:1;		// sprite layer enabled
	unsigned bgen:1;		// bg layer enabled
	unsigned bigspr:1;		// sprite 8x16 (8x8 if 0)

	unsigned short tilesadr;	// tiles data adr (0:8800, 1:8000)
	unsigned short winmapadr;	// window layer timemap (0:9800, 1:9c00)
	unsigned short bgmapadr;	// bg layer timemap (0:9800, 1:9c00)

	unsigned char ram[0x2000];	// 8K of video memory

	vRay* ray;
	vLayout* lay;

	void(*draw)(struct GBCVid*);
	void(*cbLine)(struct GBCVid*);
	void(*cbFram)(struct GBCVid*);
};
typedef struct GBCVid GBCVid;

#endif
