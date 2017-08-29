#ifndef NESPPU_H
#define NESPPU_H

#include "vidcommon.h"

typedef struct {
	unsigned vbl:1;			// vertical blank (lines 241-260) : ack by status register bit
	unsigned vblstrb:1;		// strobe of vbl, can be reset
	unsigned sp0hit:1;		// sprite 0 hit
	unsigned spover:1;		// sprites overscan
	unsigned bigspr:1;		// 8x16 sprites
	unsigned master:1;
	unsigned inten:1;		// NMI @ vblank
	unsigned greyscale:1;		// show greyscale
	unsigned bgleft8:1;		// show left 8 pix of bg
	unsigned spleft8:1;		// show left 8 pix of spr
	unsigned bgen:1;		// bg render enabled
	unsigned spen:1;		// spr render enabled
	unsigned bglock:1;
	unsigned splock:1;
	unsigned latch:1;		// 0:scx/hi 1:scy/low
	unsigned vastep:1;
	// memory
	unsigned char vbuf;
	unsigned char reg[8];
	unsigned char oam[256];			// 256 bytes OAM
	unsigned char mem[0x4000];		// 16K of video mem
	// buffers
	unsigned char bgline[0x200];		// bg (full 2 screens)
	unsigned char spline[0x108];		// sprites (8 max)
	unsigned char prline[0x108];		// sprites priority
	// chr-rom rd/wr callbacks
	unsigned char (*mrd)(unsigned short, void*);
	void (*mwr)(unsigned short, unsigned char, void*);
	void* data;
	// layout
	vRay* ray;
	int vbsline;
	int vbrline;
	// address
	unsigned char oamadr;	// oam access address
	ePair(vadr,vah,val);	// videomem access addr
	unsigned short tadr;	// tmp vadr
	unsigned short zadr;	// for debug
	unsigned short spadr;	// 8x8 sprites tiles adr
	unsigned short bgadr;	// bg tiles adr
	int finex;		// = x scroll low 3 bits

	int nt;
	int finey;
	int scx;
	int scy;
} nesPPU;

nesPPU* ppuCreate(vRay*);
void ppuDestroy(nesPPU*);

void ppuReset(nesPPU*);

void ppuDraw(nesPPU*);
void ppuHBL(nesPPU*);
void ppuLine(nesPPU*);
void ppuFram(nesPPU*);

// rd/wr ppu registers
void ppuWrite(nesPPU*, int, unsigned char);
unsigned char ppuRead(nesPPU*, int);

void ppuRenderBGLine(nesPPU*, unsigned char*, unsigned short, int, unsigned short);
unsigned short ppuYinc(unsigned short);

#endif
