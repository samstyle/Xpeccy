#ifndef X_GBC_VID_H
#define X_GBC_VID_H

#include "vidcommon.h"

/*
struct GBCVid {
	unsigned lcdon:1;
	unsigned winen:1;		// window layer enabled
	unsigned spren:1;		// sprite layer enabled
	unsigned bgen:1;		// bg layer enabled
	unsigned altile:1;		// tileset 0:9800; 1:9c00, tiles num -128..127
	unsigned bigspr:1;		// sprite 8x16 (8x8 if 0)
	unsigned gbmode:1;		// GB capability mode (not-GBC)
	// unsigned bwlock:1;		// gbmode - remove bg & win (!b0,FF40 in gbmode)
	unsigned bgprior:1;		// !gbmode - bg/win has priority on bottom layer sprites

	unsigned bgblock:1;		// block some gfx layers (external)
	unsigned winblock:1;
	unsigned sprblock:1;

	unsigned intrq:1;		// stat interrupt request
	unsigned char inten;		// int enabling flags
	unsigned char wline;

	unsigned short tilesadr;	// tiles data adr (0:8800, 1:8000)
	unsigned short winmapadr;	// window layer timemap (0:9800, 1:9c00)
	unsigned short bgmapadr;	// bg layer timemap (0:9800, 1:9c00)

	unsigned char ram[0x4000];	// 2x8K of video memory
	unsigned char oam[0x100];	// oem : sprites data
	// current line images
	unsigned char wtline[256];	// win layer with priority
	unsigned char wbline[256];	// win layer without priority
	unsigned char stline[256];	// spr layer with priority
	unsigned char sbline[256];	// spr layer without priority

	int mode;
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
*/
typedef struct Video Video;

void gbcvReset(Video*);
void gbcvDraw(Video*);
void gbcvLine(Video*);
void gbcvFram(Video*);

#endif
