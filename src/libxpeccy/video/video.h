#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define INLINE_VDOT	1

#include <stddef.h>

#include "vidcommon.h"
#include "../memory.h"

typedef struct Video Video;

#include "ulaplus.h"
#include "v9938.h"
#include "gbcvideo.h"
#include "nesppu.h"

// screen mode
enum {
	VID_UNKNOWN = -1,
// spectrum
	VID_NORMAL = 0,
	VID_ULA_SCR,
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
// v99xx
//	VID_V9938,	// MSX2
	VDP_TEXT1,
	VDP_GRA1,
	VDP_GRA2,
	VDP_MCOL,
	VDP_GRA3,
	VDP_GRA4,
	VDP_GRA5,
	VDP_GRA6,
	VDP_GRA7,
	VDP_TEXT2,
// game boy color
	VID_GBC,	// Gameboy
// nes
	VID_NES,	// NES PPU
// c64
	VID_C64_TEXT,
	VID_C64_TEXT_MC,
	VID_C64_BITMAP,
	VID_C64_BITMAP_MC,
// bk
	VID_BK_BW,
	VID_BK_COL,
// specialist
	VID_SPCLST,
// cga/ega/vga
	VID_CGA_T40,
	VID_CGA_T80,
	VID_CGA_G320,
	VID_CGA_G640,
	VGA_TXT_L,	// txt 40
	VGA_TXT_H,	// txt 80
	VGA_GRF_L,	// grf 320
	VGA_GRF_H,	// grf 640
};

extern int bufSize;
extern int bytesPerLine;
extern int greyScale;
extern int scanlines;
extern int noflic;

extern unsigned char* scrimg;
extern unsigned char* bufimg;
extern unsigned char pscr[];

extern int xstep;
extern int ystep;
extern int lefSkip;
extern int rigSkip;
extern int topSkip;
extern int botSkip;

#if INLINE_VDOT
void vid_dot_full(Video*, unsigned char);
void vid_dot_half(Video*, unsigned char);
#elif USEOPENGL
extern void(*vid_dot)(Video*, unsigned char);
#define vid_dot_full(_v, _c) vid_dot(_v, _c); vid_dot(_v, _c)
#define vid_dot_half(_v, _c) vid_dot(_v, _c)
#else
void vid_dot_full(Video*, unsigned char);
void vid_dot_half(Video*, unsigned char);
#endif

typedef int(*vcbmrd)(int, void*);
typedef void(*vcbmwr)(int, int, void*);
typedef void(*cbvid)(Video*);
typedef void(*vcbptr)(void*);

struct Video {
	unsigned nogfx:1;	// tsl : nogfx flag
	unsigned newFrame:1;	// set @ start of VBlank
	int intFRAME;		// aka INT
	unsigned intLINE:1;	// for TSConf
	unsigned intDMA:1;	// for TSConf
	unsigned noScreen:1;
	unsigned debug:1;
	unsigned upd:1;
	unsigned tail:1;
	unsigned cutscr:1;
	unsigned irq:1;

	unsigned hblank:1;	// HBlank signal
	unsigned hbstrb:1;	// HBlank strobe 0->1
	unsigned vblank:1;	// VBlank signal
	unsigned vbstrb:1;	// VBlank strobe 0->1

	unsigned hbrd:1;	// border.H
	unsigned vbrd:1;	// border.V

	unsigned hvis:1;
	unsigned vvis:1;

	int nsPerFrame;
	int nsPerLine;
	int nsPerDot;
	int nsDraw;
	int time;		// +nsPerDot each dot
	int busy;		// (cycles) to emulate busy period
	int intTime;

	int flash;
	int curscr;

	int brdstep;
	double brdsize;
	unsigned char brdcol;
	unsigned char nextbrd;

	unsigned char inten;	// interrupts enable (8 bits = 8 signals)
	unsigned char intrq;	// interrupt output signals (8 bits)
	unsigned char intbf;	// buffered int (last int signals)

	unsigned char paln;	// high bits = palete number
	xColor pal[256];	// palete. 256 colors rgb888

	int vmode;
	cbvid cbDot;		// call every dot
	cbvid cbHBlank;		// call every line
	cbvid cbLine;		// @ hblank end
	cbvid cbFrame;		// call every frame

	vcbmrd mrd;		// external memory reading
	vcbmwr mwr;		// external memory writing
	void* data;

	int fcnt;
	int lcnt;
	unsigned char atrbyte;
	size_t frmsz;
	size_t vBytes;
	vRay ray;
//	vLayout lay;
	vCoord full;
	vCoord blank;
	vCoord bord;
	vCoord scrn;
	vCoord send;
	vCoord vend;
	vCoord lcut;
	vCoord rcut;
	vCoord vsze;		// visible area size (cutted)
	vCoord intp;		// intp.y = gbc lyc = 9938 iLine
	int intsize;

	int idx;

	unsigned sprblock:1;	// hw block sprites
	unsigned bgblock:1;	// hw block bg
	unsigned greyscale:1;
	vCoord scrsize;		// << tsconf.xSize, tsconf.ySize, v9938::wid
	vCoord sc;		// screen scroll registers
	// nes
	unsigned ntsc:1;	// set if ntsc, prerender line is 1 dot shorter each other frame
	unsigned ppu_vb:1;	// set at vbs line, reset at vbrline or reading reg2
	int vadr;		// nes videomem access addr
	unsigned short tadr;	// nes tmp vadr
	int vbsline;		// 1st line of VBlank
	int vbrline;		// prerender line (usually last line of frame)
	unsigned char oamadr;
	// gbc
	unsigned lcdon:1;
	unsigned altile:1;
	unsigned bigspr:1;
	unsigned spren:1;	// sw enable sprites
	unsigned bgen:1;	// sw enable bg
	unsigned bgprior:1;
	unsigned winen:1;	// sw enable win
	unsigned winblock:1;	// hw block win
	unsigned gbmode:1;	// gameboy capatible
	unsigned char gbcmode;
	unsigned short winmapadr;
	unsigned short tilesadr;
	unsigned short bgmapadr;
	unsigned char wline;
	int xpos;
	unsigned char wtline[256];	// win layer with priority
	unsigned char wbline[256];	// win layer without priority
	unsigned char stline[256];	// spr layer with priority
	unsigned char sbline[256];	// spr layer without priority
	vCoord win;			// win layout position
	// v9938
	unsigned high:1;
	unsigned latch:1;
	unsigned vastep:1;
	unsigned spleft8:1;
	unsigned bgleft8:1;
	unsigned sp0hit:1;
	unsigned spover:1;
	unsigned master:1;
	unsigned palhi:1;

	unsigned bpage:1;
	int blink0;
	int blink1;
	int blink;

	unsigned char vbuf;
	unsigned short spadr;
	unsigned short bgadr;
	int memMask;
	int finex;
	int finey;
//	int lines;
	int inth;	// interrupts
	int intf;
	int nt;
	int dpb;	// dots per byte
	int count;
	unsigned char com;		// executed command
	unsigned char arg;		// command argument
	unsigned char dat;
	int BGTiles;
	int BGMap;
	int BGColors;
	int OBJTiles;
	int OBJAttr;
	// v9938 dma
	vCoord src;
	vCoord dst;
	vCoord rct;
	vCoord step;
	int srcx;
	int dstx;
	int rctx;
	unsigned char sr[16];			// ststus registers (0..9 actually)
	unsigned char bgline[0x200];		// bg (full 2 screens)
	unsigned char spline[0x108];		// sprites (8 max)
	unsigned char prline[0x108];		// sprites priority
	unsigned char sprImg[256 * 256];
	void(*pset)(Video*,int,int,unsigned char);
	unsigned char(*col)(Video*,int,int);

	struct {
		int xPos;			// position of screen @ monitor [32|12] x [44|24|0]
		int yPos;
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
		unsigned char cram[0x200];	// pal
		unsigned char sfile[0x200];	// sprites
		int dmabytes;
	} tsconf;
	struct {
		unsigned atrig:1;		// 3c0 flip-flop
		unsigned blinken:1;		// blink enabled
		int crt_idx;			// registers indexes
		int seq_idx;
		int grf_idx;
		int atr_idx;
		int line;			// chars line
		int chline;			// line inside char
		int chsize;			// char height (0-31)
		int cadr;			// cursor address
		unsigned char latch[4];
	} vga;

	unsigned char line[0x400];		// buffer for render sprites & tiles
	unsigned char linb[0x200];		// buffer for rendered bitplane
	unsigned char font[0x2000];		// ATM/C64/VGA text mode font (8K for VGA font)

	unsigned char sprxspr;			// c64 spr-spr collisions
	unsigned char sprxbgr;			// c64 spr-bgr collisions
	unsigned char vbank;			// vicII bank (from CIA2)
	unsigned char sbank;			// screen offset (reg#18 b1..3)
	unsigned char cbank;			// char data offset (reg#18 b4..7)
	unsigned char colram[0x1000];		// vicII color ram

	unsigned char ram[MEM_256K];			// video memory
	unsigned char oam[MEM_256];			// nes oam memory
	unsigned char reg[256];				// max 256 registers

	ulaPlus* ula;

};

Video* vidCreate(vcbmrd, void*);
void vidDestroy(Video*);

void vidReset(Video*);
void vidSync(Video*,int);
void vidSetMode(Video*,int);
void vid_reset_ray(Video*);

int vid_wait(Video*);
void vidDarkTail(Video*);

void vidSetLayout(Video*, vLayout*);
void vid_set_height(Video*, int);
void vidSetBorder(Video*, double);
void vidUpdateLayout(Video*);
void vidUpdateTimings(Video*, int);

//void vidSetFont(Video*,char*,int);
void vidGetScreen(Video*, unsigned char*, int, int, int);

void vid_set_grey(int);

void tslUpdatePorts(Video*);

#ifdef __cplusplus
}
#endif
