#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "vidcommon.h"
#include "../defines.h"

typedef struct Video Video;

#include <stdlib.h>
#include <stdint.h>

#include "ulaplus.h"
#include "v9938.h"
#include "gbcvideo.h"
#include "nesppu.h"

#define vid_irq(_v, _n) _v->xirq(_n, _v->xptr)

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
	CGA_TXT_L,	// txt 40
	CGA_TXT_H,	// txt 80
	CGA_GRF_L,	// grf 320 2bpp (cga)
	CGA_GRF_H,	// grf 640 1bpp (cga)
	VGA_GRF_L,	// grf 320 4bpp (ega)
	VGA_GRF_H,	// grf 640 4bpp (ega)
	VGA_GRF_256,	// grf 320 8bpp (vga)
};

extern int bufSize;
extern int bytesPerLine;
extern int greyScale;
//extern int scanlines;
extern int noflic;
extern int noflicMode;
extern float noflicGamma;

extern unsigned char* scrimg;
extern unsigned char* bufimg;
extern unsigned char pscr[];

extern int xstep;
extern int ystep;
extern int lefSkip;
extern int pixSkip;
extern int rigSkip;
extern int topSkip;
extern int botSkip;

void vid_dot_full(Video*, unsigned char);
void vid_dot_half(Video*, unsigned char);

//typedef int(*vcbmrd)(int, void*);
//typedef void(*vcbmwr)(int, int, void*);
typedef void(*cbvid)(Video*);
typedef void(*vcbptr)(void*);

typedef struct {
	int id;
	cbvid init;
	cbvid dot;		// each dot
	cbvid hbl;		// hblank start
	cbvid line;		// visible line start
	cbvid vbl;		// @vblank (right after last line)
	cbvid frm;		// @1st visible line (called before cbLine)
} xVideoMode;

struct Video {
	unsigned nogfx:1;	// tsl : nogfx flag
	unsigned newFrame:1;	// set @ start of VBlank
	int intFRAME;		// aka INT
	unsigned intLINE:1;	// for TSConf
	unsigned intDMA:1;	// for TSConf
//	unsigned noScreen:1;
	unsigned debug:1;
	unsigned upd:1;
	unsigned tail:1;
	unsigned cutscr:1;
	unsigned linedbl:1;	// lines doubler

	unsigned hblank:1;	// HBlank signal
	unsigned hsync:1;	// HSync (pc)
	unsigned vblank:1;	// VBlank signal
	unsigned vsync:1;	// VSync (pc)

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
	int dotPerFrame;

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
	uint32_t pal[256];	// ABGR inside int, R = LSB, A = FF
	uint32_t gpal[256];	// greyscale copy of pal
	uint32_t bpal[256];	// base palette (loaded preset)

	int vmode;
	xVideoMode* cb;
//	cbvid cbDot;		// call every dot
//	cbvid cbHBlank;		// call every line
//	cbvid cbVBlank;
//	cbvid cbLine;		// @ hblank end
//	cbvid cbFrame;		// call every frame
	cbvid cbCount;		// call when busy count down to 0

	cbxrd mrd;		// external memory reading
	cbxwr mwr;		// external memory writing
	cbirq xirq;		// interrupt
	void* xptr;

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
	vCoord res;		// current resolution (-1 = from layout)

	int idx;

	unsigned sprblock:1;	// hw block sprites
	unsigned bgblock:1;	// hw block bg
	vCoord scrsize;		// << tsconf.xSize, tsconf.ySize, v9938::wid
	vCoord sc;		// screen scroll registers
	// nes
	unsigned ntsc:1;	// set if ntsc, prerender line is 1 dot shorter each other frame
	unsigned ppu_vb:1;	// set at vbs line, reset at vbrline or reading reg2
	unsigned greyscale:1;	// show in greyscale
	int vadr;		// nes videomem access addr
	int fadr;
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
	unsigned char gbcmode;	// lcd mode (0,1,2,3)
	unsigned short winmapadr;
	unsigned short tilesadr;
	unsigned short bgmapadr;
	unsigned char wline;
	int xpos;
//	unsigned char wtline[256];	// win layer with priority
//	unsigned char wbline[256];	// win layer without priority
//	unsigned char stline[256];	// spr layer with priority
//	unsigned char sbline[256];	// spr layer without priority
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
		unsigned char cram[0x200];	// pal = colram?
		unsigned char sfile[0x200];	// sprites = ram?
		int dmabytes;
	} tsconf;
	struct {
		unsigned atrig:1;		// 3c0 flip-flop
		unsigned blinken:1;		// blink enabled
		unsigned cga:1;			// 1 if there is no ega/vga bios
		int crt_idx;			// registers indexes
		int seq_idx;
		int grf_idx;
		int atr_idx;
		unsigned char dac_idx;
		int dac_cnt;			// counter to read/write bytes to palette
		int dac_mode;			// 0 read, 1 write
		int dac_mask;			// mask for dac index
		int cpl;			// 40/80 chars per line
//		int line;			// chars line
		int chline;			// line inside char
		int chsize;			// char height (0-31)
		int cadr;			// cursor address
		unsigned char latch[4];
		xColor dac_col;
		cbvid ega_cbline;
	} vga;

	unsigned char line[0x500];		// buffer for render sprites & tiles
	unsigned char linb[0x200];		// buffer for rendered bitplane
	unsigned char font[0x2000];		// ATM/C64/CGA text mode font (8K for CGA font)		NOTE: pc98xx kanji.rom size is 282KB

	unsigned char sprxspr;			// c64 spr-spr collisions
	unsigned char sprxbgr;			// c64 spr-bgr collisions
	unsigned char vbank;			// vicII bank (from CIA2)
	unsigned char sbank;			// screen offset (reg#18 b1..3)
	unsigned char cbank;			// char data offset (reg#18 b4..7)
	unsigned char colram[0x1000];		// vicII color ram

	unsigned char bios[MEM_64K];			// ega/vga bios
	unsigned char ram[MEM_256K];			// video memory
	unsigned char oam[MEM_256];			// nes/gb oam memory
	unsigned char reg[256];				// max 256 registers
//	bool flag[256];

	ulaPlus* ula;

};

Video* vidCreate(cbxrd, cbirq, void*);
void vidDestroy(Video*);

void vid_reset(Video*);
void vid_sync(Video*,int);
// void vid_irq(Video*, int);
void vid_set_mode(Video*,int);
void vid_reset_ray(Video*);
void vid_set_ray(Video*, int);

int vid_wait(Video*, int);
void vid_dark_tail(Video*);

void vid_set_layout(Video*, vLayout*);
void vid_set_resolution(Video*, int, int);
void vid_set_border(Video*, double);
void vid_upd_layout(Video*);
void vid_upd_timings(Video*, int);

void vid_get_screen(Video*, unsigned char*, int, int, int);

void vid_set_grey(int);
xColor vid_get_col(Video*, int);
void vid_set_col(Video*, int, xColor);
void vid_set_bcol(Video*, int, xColor);
void vid_reset_col(Video*, int);

void tslUpdatePorts(Video*);

#ifdef __cplusplus
}
#endif
