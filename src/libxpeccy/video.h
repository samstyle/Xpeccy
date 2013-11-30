#ifndef _VIDEO_H
#define _VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "memory.h"

// vidFlags (emul)
#define VF_FULLSCREEN		1
#define VF_DOUBLE		(1<<1)
#define VF_BLOCKFULLSCREEN	(1<<2)
#define	VF_FRAMEDBG		(1<<3)
#define VF_NOFLIC		(1<<4)
#define	VF_GREY			(1<<5)
#define	VF_INIT			(1<<6)
#define	VF_BLOCK		(1<<7)
// vid->flags (vid)
/*
#define	VID_BORDER_4T		1
#define	VID_NOGFX		(1<<1)
#define	VID_INTSTROBE		(1<<2)
#define VID_NEXTROW		(1<<3)
#define	VID_CONT2		(1<<4)		// +2a or +3 contended memory pattern
#define	VF_TSCONF		(1<<6)
*/
// screen drawing mode
#define	VID_NOSCREEN	0
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
#define	VID_UNKNOWN	0xff

typedef struct {
	size_t h;
	size_t v;
} VSize;

#ifdef WORDS_BIG_ENDIAN
	#define VPAIR(p,h,l) union{unsigned short p; struct {unsigned char h; unsigned char l;};}
#else
	#define VPAIR(p,h,l) union{unsigned short p; struct {unsigned char l; unsigned char h;};}
#endif

struct Video {
	union {
		int flag;
		struct {
			unsigned border4t:1;
			unsigned nogfx:1;
			unsigned intstrobe:1;
			unsigned nextrow:1;
			unsigned istsconf:1;
			unsigned change:1;
			unsigned forceDraw:1;
		};
	};
	int flash;
	int curscr;
	unsigned char brdcol;
	unsigned char nextbrd;
	unsigned char fcnt;
	unsigned char atrbyte;
	unsigned char* scrptr;
	unsigned char* scrimg;
	size_t x;
	size_t y;
	size_t frmsz;
	size_t vmode;
	size_t nsDraw;
	VSize full;
	VSize bord;
	VSize sync;
	VSize lcut;
	VSize rcut;
	VSize vsze;
	VSize wsze;
	VSize intpos;
	size_t intsz;
	Memory* mem;
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
		unsigned char line[512];	// buffer for render sprites & tiles
		unsigned char cram[0x200];	// pal
		unsigned char sfile[0x200];	// sprites
	} tsconf;
	unsigned char font[0x800];		// ATM text mode font
	void(*callback)(struct Video*);
};

typedef struct Video Video;

extern int vidFlag;
extern float brdsize;

Video* vidCreate(Memory*);
void vidDestroy(Video*);

void vidSync(Video*,int);
void vidSetMode(Video*,int);
void vidWait(Video*);
void vidDarkTail(Video*);

void vidUpdate(Video*);

//unsigned char vidGetAttr(Video*);
void vidSetFont(Video*,char*);

#ifdef __cplusplus
}
#endif

#endif
