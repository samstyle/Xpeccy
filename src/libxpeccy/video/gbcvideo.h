#ifndef _GBC_VID_H
#define _GBC_VID_H

#include "vidcommon.h"

struct GBCVid {
	unsigned lcdon:1;
	unsigned winen:1;		// window layer enabled
	unsigned spren:1;		// sprite layer enabled
	unsigned bgen:1;		// bg layer enabled
	unsigned altile:1;		// tileset 01:9c00, tiles num -128..127
	unsigned bigspr:1;		// sprite 8x16 (8x8 if 0)
	unsigned lyequal:1;		// ly = lyc
	unsigned intr:1;

	struct {
		unsigned lyc:1;
		unsigned hblank:1;
		unsigned vblank:1;
		unsigned oam:1;
	} inten;			// video interrupt enabling

	unsigned short tilesadr;	// tiles data adr (0:8800, 1:8000)
	unsigned short winmapadr;	// window layer timemap (0:9800, 1:9c00)
	unsigned short bgmapadr;	// bg layer timemap (0:9800, 1:9c00)

	unsigned char ram[0x2000];	// 8K of video memory
	unsigned char oem[0x100];	// oem : sprites data
	unsigned char line[256];	// full line image (color indexes)

	vCoord sc;			// scx/scy
	vCoord win;			// wx/wy
	int xpos;
	unsigned char lyc;		// Y coordinate to compare

	vRay* ray;
	vLayout* lay;
	xColor pal[256];

	void(*draw)(struct GBCVid*);
	void(*cbLine)(struct GBCVid*);
	void(*cbFram)(struct GBCVid*);
};
typedef struct GBCVid GBCVid;

GBCVid* gbcvCreate(vRay*, vLayout*);
void gbcvDestroy(GBCVid*);

#endif
