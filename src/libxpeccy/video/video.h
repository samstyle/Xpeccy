#ifndef VIDEO_H
#define VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "../memory.h"

#include "ulaplus.h"
#include "v9938.h"
#include "gbcvideo.h"
#include "nesppu.h"

// C64 vic interrupts
#define VIC_IRQ_RASTER	0x01
#define	VIC_IRQ_SPRBGR	0x02
#define VIC_IRQ_SPRSPR	0x04
#define VIC_IRQ_LPEN	0x08
#define VIC_IRQ_ALL	(VIC_IRQ_RASTER | VIC_IRQ_SPRBGR | VIC_IRQ_SPRSPR | VIC_IRQ_LPEN)

// screen mode
enum {
	VID_NORMAL = 0,
	VID_ALCO,
	VID_ATM_EGA,
	VID_ATM_TEXT,
	VID_ATM_HWM,
	VID_HWMC,
	VID_EVO_TEXT,
	VID_TSL_16,	// TSConf 4bpp
	VID_TSL_256,	// TSConf 8bpp
	VID_TSL_NORMAL,	// TSConf common screen
	VID_TSL_TEXT,
	VID_PRF_MC,	// Profi multicolor
	VID_V9938,	// MSX2
	VID_GBC,	// Gameboy
	VID_NES,	// NES PPU
	VID_C64,
	VID_UNKNOWN = -1
};

struct Video {
	unsigned nogfx:1;	// tsl : nogfx flag
	unsigned newFrame:1;	// set @ start of VBlank
	int intFRAME;		// aka INT
	unsigned intLINE:1;	// for TSConf
	unsigned intDMA:1;	// for TSConf
	unsigned lockLayout:1;	// fix layout & don't change it by vidSetLayout
	unsigned noScreen:1;
	unsigned debug:1;
	unsigned tail:1;

	unsigned hblank:1;	// HBlank signal
	unsigned hbstrb:1;	// HBlank strobe 0->1
	unsigned vblank:1;	// VBlank signal
	unsigned vbstrb:1;	// VBlank strobe 0->1

	int nsPerFrame;
	int nsPerLine;
	int nsPerDot;
	int nsDraw;
	int time;		// +nsPerDot each dot

	int flash;
	int curscr;

	int brdstep;
	double brdsize;
	unsigned char brdcol;
	unsigned char nextbrd;

	int fcnt;
	unsigned char atrbyte;
	unsigned char scrimg[1024 * 512 * 3];	// 512x512 rgb (dX x2)
	size_t frmsz;
	size_t vBytes;
	int vmode;
	unsigned char inten;	// interrupts enable (8 bits = 8 signals)
	unsigned char intout;	// interrupt output signals (8 bits)
	vRay ray;
	vLayout lay;
	vCoord lcut;
	vCoord rcut;
	vCoord ssze;
	vCoord vsze;
	int idx;
	struct {
		int xPos;			// position of screen @ monitor [32|12] x [44|24|0]
		int yPos;
		int xSize;			// size of tsconf screen ([320|360] x [200|240|288])
		int ySize;
		unsigned char tconfig;		// port 06AF
		unsigned char TMPage;		// tiles map page
		unsigned char T0GPage;		// lay 0 graphics
		unsigned char T1GPage;		// lay 1 graphics
		unsigned char SGPage;		// sprites graphics
		unsigned char T0Pal76;		// b7.6 of tiles palete (07AF)
		unsigned char T1Pal76;
		unsigned char scrPal;		// b7..4: bitmap palete
		unsigned char vidPage;		// 1st video page
		int hsint;			// tsconf INT x pos = p22AF << 1
		unsigned char p00af;
		unsigned char p07af;
		ePair(xOffset,soxh,soxl);	// offsets of screen corner
		ePair(yOffset,soyh,soyl);
		ePair(T0XOffset,t0xh,t0xl);	// tile 0 offsets
		ePair(T0YOffset,t0yh,t0yl);
		ePair(T1XOffset,t1xh,t1xl);	// tile 1 offsets
		ePair(T1YOffset,t1yh,t1yl);
		ePair(scrLine, loffh, loffl);
		ePair(intLine, ilinh, ilinl);	// INT line
		unsigned char linb[0x200];	// buffer for rendered bitplane
		unsigned char line[0x200];	// buffer for render sprites & tiles
		unsigned char cram[0x200];	// pal
		unsigned char sfile[0x200];	// sprites
		int dmabytes;
	} tsconf;
	unsigned char regs[0x100];		// internal small video mem
	unsigned char font[0x800];		// ATM/C64 text mode font
	void(*callback)(struct Video*);		// call every dot
	void(*lineCall)(struct Video*);		// call every line
	void(*hbendCall)(struct Video*);	// @ hblank end
	void(*framCall)(struct Video*);		// call every frame
	xColor pal[256];
	Memory* mem;				// TODO: replace with external mrd(adr, void*) callback: zx modes only?
	ulaPlus* ula;
	VDP9938 v9938;
	GBCVid* gbc;
	nesPPU* ppu;

};

typedef struct Video Video;

extern int vidFlag;

Video* vidCreate(Memory*);
void vidDestroy(Video*);

void vidInitAdrs();

void vidReset(Video*);
void vidSync(Video*,int);
void vidSetMode(Video*,int);
void vidWait(Video*);
void vidDarkTail(Video*);

void vidSetLayout(Video*, vLayout);
void vidSetBorder(Video*, double);
void vidUpdateTimings(Video*, int);

void vidSetFont(Video*,char*);
void vidGetScreen(Video*, unsigned char*, int, int, int);

void tslUpdatePorts(Video*);

#ifdef __cplusplus
}
#endif

#endif
