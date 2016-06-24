#ifndef _V9938_H
#define _V9938_H

#include "vidcommon.h"

struct VDP9938 {
	unsigned high:1;		// indicates hi byte in 2-byte writing
	unsigned wr:1;			// data direction (b6.hi during writing VADR)
	int vmode;			// current mode
	int vadr;			// VRAM address (128K)
	int memMask;
	unsigned char data;		// 1st byte in 2-byte writing
	int lines;			// 192/212
	int iLine;			// horisontal retrace INT line
	int hAdj, vAdj;			// horizontal/vertical display adjust
	unsigned char com;		// executed command
	unsigned char arg;		// command argument
	vCoord src;
	vCoord dst;
	vCoord cnt;
	vCoord delta;
	vCoord pos;
	vCoord size;
	int count;
	int regIdx;
	int BGTiles;
	int BGMap;
	int BGColors;
	int OBJTiles;
	int OBJAttr;
	vRay* ray;
	vLayout* lay;
	xColor pal[256];
	unsigned char sr[16];			// ststus registers (0..9 actually)
	unsigned char reg[0x40];		// VDP registers (8 for v9918, 48 for v9938)
	unsigned char ram[0x20000];	// VRAM (16K for v9918, 128K for v9938)
	unsigned char sprImg[0x10000];	// 256x256 image with foreground sprites (rebuild @ frame start)
	void(*draw)(struct VDP9938*);
	void(*cbLine)(struct VDP9938*);
	void(*cbFram)(struct VDP9938*);
	void(*plot)(struct VDP9938*, int, int, unsigned char);		// put dot
	unsigned char(*color)(struct VDP9938*, int, int);		// pick dot color
};

typedef struct VDP9938 VDP9938;

void vdpReset(VDP9938*);
void vdpMemWr(VDP9938*, unsigned char);
void vdpRegWr(VDP9938*, unsigned char);
void vdpPalWr(VDP9938*, unsigned char);
unsigned char vdpReadSR(VDP9938*);

#endif
