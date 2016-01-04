#ifndef _VIDEO_H
#define _VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "memory.h"
#include "ulaplus.h"

// screen drawing mode
#define	VID_CURRENT	0	// 'refresh' current mode. used on changing VF_NOSCREEN flag
#define	VID_NORMAL	1
#define	VID_ALCO	2
#define	VID_ATM_EGA	3
#define	VID_ATM_TEXT	4
#define	VID_ATM_HWM	5
#define	VID_HWMC	6
#define	VID_EVO_TEXT	7
#define VID_TSL_16	8	// TSConf 4bpp
#define	VID_TSL_256	9	// TSConf 8bpp
#define	VID_TSL_NORMAL	10	// TSConf common screen
#define	VID_TSL_TEXT	11
#define VID_PRF_MC	12	// Profi multicolor
#define VID_MSX_SCR0	0x20	// MSX 1/2 modes
#define VID_MSX_SCR1	0x21
#define VID_MSX_SCR2	0x22
#define VID_MSX_SCR3	0x23
#define VID_MSX_SCR4	0x24
#define VID_MSX_SCR5	0x25
#define VID_MSX_SCR6	0x26
#define VID_MSX_SCR7	0x27
#define	VID_UNKNOWN	0xff

typedef struct {
	int h;
	int v;
} VSize;

#ifdef WORDS_BIG_ENDIAN
	#define VPAIR(p,h,l) union{unsigned short p; struct {unsigned char h; unsigned char l;};}
#else
	#define VPAIR(p,h,l) union{unsigned short p; struct {unsigned char l; unsigned char h;};}
#endif

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} xColor;

struct Video {
	unsigned border4t:1;
	unsigned nogfx:1;	// tsl : nogfx flag
	unsigned newFrame:1;	// set @ start of VBlank
	unsigned intFRAME:1;	// aka INT
	unsigned intLINE:1;	// for TSConf
	unsigned intDMA:1;	// for TSConf
	unsigned nextrow:1;	// = not-masked intLINE
	unsigned istsconf:1;
	unsigned noScreen:1;
	unsigned debug:1;

	int flash;
	int curscr;
	unsigned char brdcol;
	unsigned char nextbrd;
	unsigned char fcnt;
	unsigned char atrbyte;
	unsigned char scrimg[1024 * 512 * 3];
	unsigned char* scrptr;
	unsigned char* linptr;
	int x;
	int y;
	size_t frmsz;
	size_t vBytes;
	int vmode;
	int nsDraw;
	VSize full;
	VSize bord;
	VSize sync;
	VSize lcut;
	VSize rcut;
	VSize vsze;
	VSize intpos;
	size_t intSize;
	unsigned char intMask;		// tsconf only, other machines have 1 here
	Memory* mem;
	ulaPlus* ula;
	int idx;
	struct {
		unsigned char vidPage;		// 1st video page
		int xPos;			// position of screen @ monitor [32|12] x [44|24|0]
		int yPos;
		int xSize;			// size of tsconf screen ([320|360] x [200|240|288])
		int ySize;
		int scrLine;			// bitmap line counter (TSConf)
		unsigned char tconfig;		// port 06AF
		unsigned char TMPage;		// tiles map page
		unsigned char T0GPage;		// lay 0 graphics
		unsigned char T1GPage;		// lay 1 graphics
		unsigned char SGPage;		// sprites graphics
		unsigned char T0Pal76;		// b7.6 of tiles palete (07AF)
		unsigned char T1Pal76;
		unsigned char scrPal;		// b7..4: bitmap palete
		VPAIR(xOffset,soxh,soxl);	// offsets of screen corner
		VPAIR(yOffset,soyh,soyl);
		VPAIR(T0XOffset,t0xh,t0xl);	// tile 0 offsets
		VPAIR(T0YOffset,t0yh,t0yl);
		VPAIR(T1XOffset,t1xh,t1xl);	// tile 1 offsets
		VPAIR(T1YOffset,t1yh,t1yl);
		unsigned char line[0x200];	// buffer for render sprites & tiles
		unsigned char cram[0x200];	// pal
		unsigned char sfile[0x200];	// sprites
	} tsconf;
	unsigned char font[0x800];		// ATM text mode font
	void(*callback)(struct Video*);
	xColor pal[256];
	struct {
		unsigned high:1;		// indicates hi byte in 2-byte writing
		unsigned wr:1;			// data direction (b6.hi during writing VADR)
		int vmode;			// current mode
		int sr0;			// ststus register 0
		int vadr;			// VRAM address
		unsigned char data;		// 1st byte in 2-byte writing
		unsigned char reg[48];		// VDP registers (8 for v9918, 48? for v9938)
		unsigned char ram[0x20000];	// VRAM (16K for v9918, 128K for v9938)
	} v9918;
};

typedef struct Video Video;

extern int vidFlag;

Video* vidCreate(Memory*);
void vidDestroy(Video*);

void vidInitAdrs();

void vidSync(Video*,int);
void vidSetMode(Video*,int);
void vidWait(Video*);
void vidDarkTail(Video*);

void vidUpdate(Video*, float);

void vidSetFont(Video*,char*);
void vidGetScreen(Video*, unsigned char*, int, int, int);

#ifdef __cplusplus
}
#endif

#endif
