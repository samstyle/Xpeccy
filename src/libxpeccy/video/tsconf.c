#include "video.h"

#include <stdio.h>
#include <string.h>

// tsconf sprites & tiles

static int adr,xscr,yscr;
static unsigned char col, ink, pap, scrbyte, nxtbyte;

void vidDrawByteDD(Video*);

static int fadr;
static int tile;
static int sadr;	// adr in sprites dsc
static int xadr;	// = pos with XFlip

int vidTSLRenderTiles(Video* vid, int lay, unsigned short yoffs, unsigned short xoffs, unsigned char gpage, unsigned char palhi) {
	int j;
	int res = 0;
	yscr = vid->ray.y - vid->tsconf.yPos + yoffs;						// line in TMap
	adr = (vid->tsconf.TMPage << 14) | ((yscr & 0x1f8) << 5) | (lay ? 0x80 : 0x00);		// start of TMap line (full.adr)
	xscr = (0x200 - xoffs) & 0x1ff;								// pos in line buf
	xadr = vid->tsconf.tconfig & (lay ? 8 : 4);
	do {											// 64 tiles in row
		tile = vid->mrd(adr, vid->xptr) | (vid->mrd(adr + 1, vid->xptr) << 8);		// tile dsc
		adr += 2;

		if ((tile & 0xfff) || xadr) {							// !0 or (0 enabled)
			fadr = gpage << 14;
			fadr += ((tile & 0xfc0) << 5) | ((yscr & 7) << 8) | ((tile & 0x3f) << 2);	// full addr of row of this tile
			if (tile & 0x8000) fadr ^= 0x0700;						// YFlip
			res += 2;			// 8 dots, 2 memory readings
			col = palhi | ((tile >> 8) & 0x30);					// palette (b7..4 of color)
			if (tile & 0x4000) {							// XFlip
				xscr += 8;
				for (j = 0; j < 4; j++) {
					col &= 0xf0;
					col |= (vid->mrd(fadr, vid->xptr) & 0xf0) >> 4;		// left pixel
					xscr--;
					if (col & 0x0f) vid->line[xscr & 0x1ff] = col;
					col &= 0xf0;
					col |= vid->mrd(fadr, vid->xptr) & 0x0f;			// right pixel
					xscr--;
					if (col & 0x0f) vid->line[xscr & 0x1ff] = col;
					fadr++;
				}
				xscr += 8;
			} else {								// no XFlip
				for (j = 0; j < 4; j++) {
					col &= 0xf0;
					col |= (vid->mrd(fadr, vid->xptr) & 0xf0) >> 4;				// left pixel
					if (col & 0x0f) {
						vid->line[xscr & 0x1ff] = col;
					}
					xscr++;
					col &= 0xf0;
					col |= vid->mrd(fadr, vid->xptr) & 0x0f;					// right pixel
					if (col & 0x0f) {
						vid->line[xscr & 0x1ff] = col;
					}
					xscr++;
					fadr++;
				}
			}
		} else {
			xscr += 8;
		}
	} while (adr & 0x7f);
	return res;
}

typedef struct {
	unsigned y:9;		// 0[0:7], 1:0
	unsigned ys:3;		// 1[1:3]
	unsigned res1:1;	// 1:4
	unsigned act:1;		// 1:5
	unsigned leap:1;	// 1:6
	unsigned yf:1;		// 1:7
	unsigned x:9;		// 2[0:7], 3:0
	unsigned xs:3;		// 3[1:3]
	unsigned res2:3;	// 3[4:6]
	unsigned xf:1;		// 3:7
	unsigned tnum:12;	// 4[0:7], 5[0:3]
	unsigned pal:4;		// 5[4:7]
} TSpr;

int vidTSLRenderSprites(Video* vid) {
	unsigned char* ptr;
	TSpr spr;
	int res = 0;
	while (sadr < (0x200 - 6)) {
		ptr = &vid->tsconf.sfile[sadr];
		spr.y = ptr[0];
		spr.y |= (ptr[1] & 1) << 8;
		spr.ys = (ptr[1] & 0x0e) >> 1;
		spr.res1 = (ptr[1] & 0x10) ? 1 : 0;
		spr.act = (ptr[1] & 0x20) ? 1 : 0;
		spr.leap = (ptr[1] & 0x40) ? 1 : 0;
		spr.yf = (ptr[1] & 0x80) ? 1 : 0;
		spr.x = ptr[2];
		spr.x |= (ptr[3] & 1) << 8;
		spr.xs = (ptr[3] & 0x0e) >> 1;
		spr.res2 = (ptr[3] & 0x70) >> 4;
		spr.xf = (ptr[3] & 0x80) ? 1 : 0;
		spr.tnum = ptr[4];
		spr.tnum |= (ptr[5] & 0x0f) << 8;
		spr.pal = (ptr[5] & 0xf0) >> 4;
		if (spr.act) {
			adr = spr.y;
			xscr = (spr.ys + 1) << 3;		// Ysize - 000:8; 001:16; 010:24; ...
			yscr = vid->ray.y - vid->tsconf.yPos;	// line on screen
			if (((yscr - adr) & 0x1ff) < xscr) {	// if sprite visible on current line
				res += xscr >> 2;		// 1/4 : 4 dots each memory access
				yscr -= adr;			// line inside sprite;
				if (spr.yf) yscr = xscr - yscr - 1;	// YFlip (Ysize - norm.pos - 1)
				tile = spr.tnum + ((yscr & 0x1f8) << 3);	// shift to current tile line

				fadr = vid->tsconf.SGPage << 14;
				fadr += ((tile & 0xfc0) << 5) | ((yscr & 7) << 8) | ((tile & 0x3f) << 2);	// fadr = adr of pix line to put in buf

				col = spr.pal << 4;
				xadr = (spr.xs + 1) << 3;	// xsoze
				adr = spr.x;			// xpos
				if (spr.xf) adr += xadr - 1;	// xpos of right pixel (xflip)
				for (xscr = xadr; xscr > 0; xscr -= 2) {
					col &= 0xf0;
					col |= ((vid->mrd(fadr, vid->xptr) & 0xf0) >> 4);		// left pixel;
					if (col & 0x0f) vid->line[adr & 0x1ff] = col;
					if (spr.xf) adr--; else adr++;
					col &= 0xf0;
					col |= (vid->mrd(fadr, vid->xptr) & 0x0f);		// right pixel
					if (col & 0x0f) vid->line[adr & 0x1ff] = col;
					if (spr.xf) adr--; else adr++;
					fadr++;
				}
			}
		}
		sadr += 6;
		if (spr.leap) break;		// LEAP
	}
	return res;
}

// pre-render bitmap planes to vid->tsconv.linb[512]
int vidTSLRender16c(Video* vid) {
	xscr = vid->tsconf.xOffset & 0x1ff;
	yscr = (vid->tsconf.scrLine + vid->tsconf.yOffset) & 0x1ff;
	adr = ((vid->tsconf.vidPage & 0xf8) << 14) + (yscr << 8) + (xscr >> 1);
	xadr = adr & ~0xff;
	fadr = 0;
	while (fadr < vid->scrsize.x) {
		scrbyte = vid->mrd(adr, vid->xptr);
		adr = ((adr + 1) & 0xff) | xadr;
		vid->linb[fadr] = vid->tsconf.scrPal | ((scrbyte >> 4) & 0x0f);
		fadr++;
		vid->linb[fadr] = vid->tsconf.scrPal | (scrbyte & 0x0f);
		fadr++;
	}
	return vid->scrsize.x >> 2;		// 1/4
}

int vidTSLRender256c(Video* vid) {
	xscr = vid->tsconf.xOffset & 0x1ff;
	yscr = (vid->tsconf.scrLine + vid->tsconf.yOffset) & 0x1ff;
	adr = ((vid->tsconf.vidPage & 0xf0) << 14) + (yscr << 9) + xscr;
	xadr = adr & ~0x1ff;
	fadr = 0;
	while (fadr < vid->scrsize.x) {
		vid->linb[fadr] = vid->mrd(adr, vid->xptr);
		fadr++;
		adr = ((adr + 1) & 0x1ff) | xadr;
	}
	return vid->scrsize.x >> 1;		// 1/2
}

// return ticks @ 7MHz (aka dots) eaten for line rendering
int vidTSRender(Video* vid) {
	int res = 0;

// tilemap reading
	yscr = (vid->ray.y - vid->tsconf.yPos + 8);
	if (yscr < 0) yscr += vid->full.y;
	if  (yscr < vid->scrsize.y) {
		if (vid->tsconf.tconfig & 0x20) res += 8;
		if (vid->tsconf.tconfig & 0x40) res += 8;
	}
// dma portion
/*
	if (vid->tsconf.dmabytes > 0) {
		sadr = (vid->tsconf.dmabytes > 32) ? 32 : vid->tsconf.dmabytes;
		vid->tsconf.dmabytes -= sadr;
		res += sadr;
	}
*/
// ...
	if (vid->ray.y < vid->tsconf.yPos) return res;
	if (vid->ray.y >= (vid->tsconf.yPos + vid->scrsize.y)) return res;
// prepare layers
	sadr = 0x000;					// adr inside SFILE
	memset(vid->line,0x00,0x200);		// clear tile-sprite line
	memset(vid->linb,0x00,0x200);
// bitplane
	switch(vid->vmode) {
		case VID_TSL_16:
			res += vidTSLRender16c(vid);
			break;
		case VID_TSL_256:
			res += vidTSLRender256c(vid);
			break;
		case VID_TSL_TEXT:
			// TODO : text line pre-render
			break;
	}
	if (vid->vmode != VID_TSL_NORMAL) res += 32;		// shit
// S0
	if (vid->tsconf.tconfig & 0x80) res += vidTSLRenderSprites(vid);
// T0
	if (vid->tsconf.tconfig & 0x20) res += vidTSLRenderTiles(vid,0,vid->tsconf.T0YOffset,vid->tsconf.T0XOffset,vid->tsconf.T0GPage,vid->tsconf.T0Pal76);
// S1
	if (vid->tsconf.tconfig & 0x80) res += vidTSLRenderSprites(vid);
// T1
	if (vid->tsconf.tconfig & 0x40) res += vidTSLRenderTiles(vid,1,vid->tsconf.T1YOffset,vid->tsconf.T1XOffset,vid->tsconf.T1GPage,vid->tsconf.T1Pal76);
// S2
	if (vid->tsconf.tconfig & 0x80) res += vidTSLRenderSprites(vid);
	vid->tsconf.scrLine++;
	return res;
}

static const int tslXRes[4] = {256,320,320,360};
static const int tslYRes[4] = {192,200,240,288};

void tslUpdatePorts(Video* vid) {
	unsigned char val = vid->tsconf.p00af;
	vid->scrsize.x = tslXRes[(val >> 6) & 3];
	vid->scrsize.y = tslYRes[(val >> 6) & 3];
	vid->tsconf.xPos = (vid->vend.x - vid->scrsize.x) / 2;
	vid->tsconf.yPos = (vid->vend.y - vid->scrsize.y) / 2;
	// printf("%i x %i (%i x %i) = %i x %i\n",vid->scrsize.x,vid->scrsize.y,vid->vsze.x,vid->vsze.y,vid->tsconf.xPos,vid->tsconf.yPos);
	switch(val & 3) {
		case 0: vid_set_mode(vid,VID_TSL_NORMAL); break;
		case 1: vid_set_mode(vid,VID_TSL_16); break;
		case 2: vid_set_mode(vid,VID_TSL_256); break;
		case 3: vid_set_mode(vid,VID_TSL_TEXT); break;
	}
	vid->nogfx = (val & 0x20) ? 1 : 0;
	val = vid->tsconf.p07af;
	vid->tsconf.scrPal = (val << 4) & 0xf0;
	vid->tsconf.T0Pal76 = (val << 2) & 0xc0;
	vid->tsconf.T1Pal76 = (val & 0xc0);
}

// tsconf line callback
// TODO : emulate drawing time
void vidTSline(Video* vid) {
	int res = vidTSRender(vid);		// dots eaten by rendering
	vid->intp.x = vid->tsconf.hsint + res;
	if (vid->inten & 2) {
		vid->intLINE = 1;
		vid->xirq(IRQ_VID_LINE, vid->xptr);
	}
	tslUpdatePorts(vid);
}

void scanExtLine(Video* vid) {
	xscr = vid->ray.x - vid->tsconf.xPos;
	yscr = vid->ray.y - vid->tsconf.yPos;
	if ((yscr >= 0) && (yscr < vid->scrsize.y) && (xscr >= 0) && (xscr < vid->scrsize.x)) {
		if (((vid->vmode == VID_TSL_16) || (vid->vmode == VID_TSL_256)) && !vid->nogfx)		// put bitmap pixel
			col = vid->linb[xscr];
		if (vid->line[xscr] & 0x0f)							// put not-transparent tiles/sprites pixel
			col = vid->line[xscr];
	}
}

// tsconf normal screen (separated 'cuz of palette)

void vidDrawTSLNormal(Video* vid) {
	xscr = vid->ray.x - vid->bord.x;
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr >= vid->scrn.y) || vid->nogfx) {
		col = vid->brdcol;
	} else {
		xadr = vid->tsconf.vidPage;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mrd(MADR(xadr, adr), vid->xptr);
		}
		if ((xscr < 0) || (xscr >= vid->scrn.x)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = 0x1800 | ((yscr & 0xc0) << 2) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
				vid->atrbyte = vid->mrd(MADR(xadr, adr), vid->xptr);
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = (vid->atrbyte & 0x07) | ((vid->atrbyte & 0x40) >> 3);
				pap = (vid->atrbyte & 0x78) >> 3;
			}
			col = vid->tsconf.scrPal | ((scrbyte & 0x80) ? ink : pap);
			scrbyte <<= 1;
		}
	}
	scanExtLine(vid);
	vid_dot_full(vid, col);
	//vidPutDot(&vid->ray, vid->pal, col);
}

// tsconf extend mode (out pre-rendered bitmap/TSU layers)

void vidDrawTSLExt(Video* vid) {
	col = vid->brdcol;
	scanExtLine(vid);
	vid_dot_full(vid, col);
	// vidPutDot(&vid->ray, vid->pal, col);
}

// tsconf text

void vidDrawTSLText(Video* vid) {
	xscr = vid->ray.x - vid->tsconf.xPos;
	yscr = vid->ray.y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr >= vid->scrsize.x) || (yscr < 0) || (yscr >= vid->scrsize.y)) {
		vid_dot_full(vid, vid->brdcol);
		//vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		if ((xscr & 3) == 0) {
			xscr += vid->tsconf.xOffset;
			yscr += vid->tsconf.yOffset;
			xscr &= 0x1ff;
			yscr &= 0x1ff;
			adr = (vid->tsconf.vidPage << 14) + ((yscr & 0x1f8) << 5) + (xscr >> 2);	// 256 bytes in row
			scrbyte = vid->mrd(adr, vid->xptr);
			col = vid->mrd(adr | 0x80, vid->xptr);
			ink = (col & 0x0f) | (vid->tsconf.scrPal);
			pap = ((col & 0xf0) >> 4)  | (vid->tsconf.scrPal);
			scrbyte = vid->mrd(MADR(vid->tsconf.vidPage ^ 1, (scrbyte << 3) | (yscr & 7)), vid->xptr);
//			vidDrawByteDD(vid);
		}
		if (vid->line[xscr] & 0x0f) {							// put not-transparent tiles/sprites pixel
			vid_dot_full(vid, vid->line[xscr]);
			//vidPutDot(&vid->ray, vid->pal, vid->line[xscr]);
		} else {
			//vidSingleDot(&vid->ray, vid->pal, (scrbyte & 0x80) ? ink : pap);
			//vidSingleDot(&vid->ray, vid->pal, (scrbyte & 0x40) ? ink : pap);
			vid_dot_half(vid, (scrbyte & 0x80) ? ink : pap);
			vid_dot_half(vid, (scrbyte & 0x40) ? ink : pap);
		}
		scrbyte <<= 2;
	}
}
