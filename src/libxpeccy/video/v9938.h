#ifndef X_V9938_H
#define X_V9938_H

#include "vidcommon.h"

typedef struct VDP9938 VDP9938;

typedef struct {
	int id;
	int wid;	// screen width (256|512)
	int dpb;	// dots per byte (bitmaps: 1|2|4)
	void(*draw)(VDP9938*);
	void(*line)(VDP9938*);
	void(*fram)(VDP9938*);
	void(*pset)(VDP9938*,int,int,unsigned char);
	unsigned char(*col)(VDP9938*,int,int);
} vdpMode;

struct VDP9938 {
	unsigned high:1;		// indicates hi byte in 2-byte writing
	unsigned palhi:1;

	int intf;			// VINT
	int inth;			// HINT (@ iLine)

	int bpage:1;		// blink trigger
	int blink;		// blink current period
	int blink0;		// blink periods (frames)
	int blink1;

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
//	int regIdx;
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
	unsigned char ram[0x30000];	// VRAM (16K for v9918, 128K+64K for v9938)
	unsigned char sprImg[0x10000];	// 256x256 image with foreground sprites (rebuild @ frame start)
	vdpMode* core;
/*
	void(*draw)(struct VDP9938*);
	void(*cbLine)(struct VDP9938*);
	void(*cbFram)(struct VDP9938*);
	void(*plot)(struct VDP9938*, int, int, unsigned char);		// put dot
	unsigned char(*color)(struct VDP9938*, int, int);		// pick dot color
*/
};

void vdpReset(VDP9938*);
void vdpWrite(VDP9938*, int, unsigned char);
unsigned char vdpRead(VDP9938*, int);

void vdpDraw(VDP9938*);
void vdpLine(VDP9938*);
void vdpHBlk(VDP9938*);
void vdpVBlk(VDP9938*);

#endif
