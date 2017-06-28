#ifndef _NESPPU_H
#define _NESPPU_H

#include "vidcommon.h"

typedef struct {
	unsigned bigspr:1;		// 8x16 sprites
	unsigned master:1;
	unsigned inten:1;		// NMI @ vblank
	unsigned greyscale:1;		// show greyscale
	unsigned bgleft8:1;		// show left 8 pix of bg
	unsigned spleft8:1;		// show left 8 pix of spr
	unsigned bgen:1;		// bg render enabled
	unsigned spen:1;		// spr render enabled
	unsigned sclatch:1;		// 0:scx 1:scy wr
	unsigned valatch:1;		// 0:high 1:low byte of addr

	unsigned char reg[8];
	unsigned char oam[256];			// 256 bytes OAM
	unsigned char mem[0x4000];		// 16K of video mem

	unsigned char bgline[256];		// bg
	unsigned char spline[256];		// sprites (8 max)
	unsigned char prline[256];		// sprites priority
	vRay* ray;
	int oamadr;	// oam access address
	int vidadr;	// videomem access addr
	int vadrinc;	// videomem addr increment (1 | 32)
	int ntadr;	// tilemap base adr
	int spadr;	// 8x8 sprites tiles adr
	int bgadr;	// bg tiles adr
	unsigned char scx;
	unsigned char scy;
} nesPPU;

nesPPU* ppuCreate(vRay*);
void ppuDestroy(nesPPU*);

void ppuDraw(nesPPU*);
void ppuLine(nesPPU*);
void ppuFram(nesPPU*);

#endif
