#include "video.h"

#include <string.h>

// tsconf sprites & tiles

extern int adr,xscr,yscr;
extern unsigned char col, ink, pap, scrbyte, nxtbyte;
extern unsigned char inkTab[];
extern unsigned char papTab[];

void vidDrawByteDD(Video*);
void vidATMDoubleDot(Video*, unsigned char);
/*
void vidTSPut(Video* vid, unsigned char* ptr) {
	int ofs = 0;
	adr = vid->tsconf.xPos - vid->lcut.x;
	// printf("%i - %i = %i\n",vid->tsconf.xPos, vid->lcut.h, adr);
	if (adr > 0) {
		ptr += adr * 6; // ((vidFlag & VF_DOUBLE) ? 6 : 3);
	}
	for (xscr = 0; xscr < vid->tsconf.xSize; xscr++) {
		if ((adr >= 0) && (adr < vid->vsze.x)) {			// visible
			col = vid->tsconf.line[ofs];
			if (col & 0x0f) {					// not transparent
				*(ptr++) = vid->pal[col].r;
				*(ptr++) = vid->pal[col].g;
				*(ptr++) = vid->pal[col].b;
				*(ptr++) = vid->pal[col].r;
				*(ptr++) = vid->pal[col].g;
				*(ptr++) = vid->pal[col].b;
			} else {
				ptr += 6; // ((vidFlag & VF_DOUBLE) ? 6 : 3);
			}
		}
		adr++;
		ofs++;
	}
}
*/

int fadr;
int tile;
int sadr;	// adr in sprites dsc
int xadr;	// = pos with XFlip
unsigned char* mptr;

void vidTSTiles(Video* vid, int lay, unsigned short yoffs, unsigned short xoffs, unsigned char gpage, unsigned char palhi) {
	int j;
	yscr = vid->ray.y - vid->tsconf.yPos + yoffs;						// line in TMap
	adr = (vid->tsconf.TMPage << 14) | ((yscr & 0x1f8) << 5) | (lay ? 0x80 : 0x00);		// start of TMap line (full.adr)
	xscr = (0x200 - xoffs) & 0x1ff;									// pos in line buf
	xadr = vid->tsconf.tconfig & (lay ? 8 : 4);
	do {								// 64 tiles in row
		tile = vid->mem->ramData[adr] | (vid->mem->ramData[adr + 1] << 8);		// tile dsc
		adr += 2;

		if ((tile & 0xfff) || xadr) {			// !0 or (0 enabled)
			fadr = gpage << 14;
			fadr += ((tile & 0xfc0) << 5) | ((yscr & 7) << 8) | ((tile & 0x3f) << 2);	// full addr of row of this tile
			if (tile & 0x8000) fadr ^= 0x0700;						// YFlip

			col = palhi | ((tile >> 8) & 0x30);					// palette (b7..4 of color)
			if (tile & 0x4000) {							// XFlip
				xscr += 8;
				for (j = 0; j < 4; j++) {
					col &= 0xf0;
					col |= (vid->mem->ramData[fadr] & 0xf0) >> 4;				// left pixel
					xscr--;
					if (col & 0x0f) vid->tsconf.line[xscr & 0x1ff] = col;
					col &= 0xf0;
					col |= vid->mem->ramData[fadr] & 0x0f;					// right pixel
					xscr--;
					if (col & 0x0f) vid->tsconf.line[xscr & 0x1ff] = col;
					fadr++;
				}
				xscr += 8;
			} else {								// no XFlip
				for (j = 0; j < 4; j++) {
					col &= 0xf0;
					col |= (vid->mem->ramData[fadr] & 0xf0) >> 4;				// left pixel
					if (col & 0x0f) {
						vid->tsconf.line[xscr & 0x1ff] = col;
					}
					xscr++;
					col &= 0xf0;
					col |= vid->mem->ramData[fadr] & 0x0f;					// right pixel
					if (col & 0x0f) {
						vid->tsconf.line[xscr & 0x1ff] = col;
					}
					xscr++;
					fadr++;
				}
			}
		} else {
			xscr += 8;
		}
	} while (adr & 0x7f);
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
	unsigned tnum:12;	// 4[0:7], 4[0:3]
	unsigned pal:4;		// 4[4:7]
} TSpr;

void vidTSSprites(Video* vid) {
	unsigned char* ptr;
	TSpr spr;
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
					col |= ((vid->mem->ramData[fadr] & 0xf0) >> 4);		// left pixel;
					if (col & 0x0f) vid->tsconf.line[adr & 0x1ff] = col;
					if (spr.xf) adr--; else adr++;
					col &= 0xf0;
					col |= (vid->mem->ramData[fadr] & 0x0f);		// right pixel
					if (col & 0x0f) vid->tsconf.line[adr & 0x1ff] = col;
					if (spr.xf) adr--; else adr++;
					fadr++;
				}
			}
		}
		sadr += 6;
		if (spr.leap) break;		// LEAP
	}
}

// pre-render bitmap planes to vid->tsconv.linb[512]
void vidTSLRender16c(Video* vid) {
	xscr = vid->tsconf.xOffset & 0x1ff;
	yscr = (vid->tsconf.scrLine + vid->tsconf.yOffset) & 0x1ff;
	adr = ((vid->tsconf.vidPage & 0xf8) << 14) + (yscr << 8) + (xscr >> 1);
	xadr = adr & ~0xff;
	fadr = 0;
	while (fadr < vid->tsconf.xSize) {
		scrbyte = vid->mem->ramData[adr];
		adr = ((adr + 1) & 0xff) | xadr;
		vid->tsconf.linb[fadr] = vid->tsconf.scrPal | ((scrbyte >> 4) & 0x0f);
		fadr++;
		vid->tsconf.linb[fadr] = vid->tsconf.scrPal | (scrbyte & 0x0f);
		fadr++;
	}
}

void vidTSLRender256c(Video* vid) {
	xscr = vid->tsconf.xOffset & 0x1ff;
	yscr = (vid->tsconf.scrLine + vid->tsconf.yOffset) & 0x1ff;
	adr = ((vid->tsconf.vidPage & 0xf0) << 14) + (yscr << 9) + xscr;
	xadr = adr & ~0x1ff;
	fadr = 0;
	while (fadr < vid->tsconf.xSize) {
		vid->tsconf.linb[fadr] = vid->mem->ramData[adr];
		fadr++;
		adr = ((adr + 1) & 0x1ff) | xadr;
	}
}

void vidTSRender(Video* vid) {
	if (vid->ray.y < vid->tsconf.yPos) return;
	if (vid->ray.y >= (vid->tsconf.yPos + vid->tsconf.ySize)) return;
// prepare layers in VID->LINE & out visible part to PTR
	sadr = 0x000;					// adr inside SFILE
	memset(vid->tsconf.line,0x00,0x200);		// clear tile-sprite line
	memset(vid->tsconf.linb,0x00,0x200);
// bitplane
	switch(vid->vmode) {
		case VID_TSL_16: vidTSLRender16c(vid); break;
		case VID_TSL_256: vidTSLRender256c(vid); break;
	}
// S0
	if (vid->tsconf.tconfig & 0x80) vidTSSprites(vid);
// T0
	if (vid->tsconf.tconfig & 0x20) vidTSTiles(vid,0,vid->tsconf.T0YOffset,vid->tsconf.T0XOffset,vid->tsconf.T0GPage,vid->tsconf.T0Pal76);
// S1
	if (vid->tsconf.tconfig & 0x80) vidTSSprites(vid);
// T1
	if (vid->tsconf.tconfig & 0x40) vidTSTiles(vid,1,vid->tsconf.T1YOffset,vid->tsconf.T1XOffset,vid->tsconf.T1GPage,vid->tsconf.T1Pal76);
// S2
	if (vid->tsconf.tconfig & 0x80) vidTSSprites(vid);
	vid->tsconf.scrLine++;
}

// tsconf line callback

void vidTSline(Video* vid) {
	vidTSRender(vid);
	if (vid->intMask & 2)
		vid->intLINE = 1;
}

// tsconf normal screen (separated 'cuz of palette)

void vidDrawTSLNormal(Video* vid) {
	xscr = vid->ray.x - vid->lay.bord.x;
	yscr = vid->ray.y - vid->lay.bord.y;
	if ((yscr < 0) || (yscr >= vid->lay.scr.y) || vid->nogfx) {
		col = vid->brdcol;
	} else {
		//xadr = vid->tsconf.vidPage ^ (vid->curscr & 2);		// TODO : ORLY? Current video page
		xadr = vid->curscr;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mem->ramData[MADR(xadr, adr)];
		}
		if ((xscr < 0) || (xscr >= vid->lay.scr.x)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = 0x1800 | ((yscr & 0xc0) << 2) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
				vid->atrbyte = vid->mem->ramData[MADR(xadr, adr)];
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = inkTab[vid->atrbyte & 0x7f];
				pap = papTab[vid->atrbyte & 0x7f];
			}
			col = vid->tsconf.scrPal | ((scrbyte & 0x80) ? ink : pap);
			scrbyte <<= 1;
		}
	}
	// put pre-rendered bitmap/tiles/sprites line over std screen
	xscr = vid->ray.x - vid->tsconf.xPos;
	yscr = vid->ray.y - vid->tsconf.yPos;
	if ((yscr >= 0) && (yscr < vid->tsconf.ySize) && (xscr >= 0) && (xscr < vid->tsconf.xSize)) {
		if (((vid->vmode == VID_TSL_16) || (vid->vmode == VID_TSL_256)) && !vid->nogfx)		// put bitmap pixel
			col = vid->tsconf.linb[xscr];
		if (vid->tsconf.line[xscr] & 0x0f)							// put not-transparent tiles/sprites pixel
			col = vid->tsconf.line[xscr];
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

/*
// tsconf 4bpp

void vidDrawTSL16(Video* vid) {
	xscr = vid->ray.x - vid->tsconf.xPos;
	yscr = vid->ray.y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr >= vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize) || vid->nogfx) {
		col = vid->brdcol;
	} else {
		xscr += vid->tsconf.xOffset;
		yscr = (vid->ray.y - vid->tsconf.yPos + vid->tsconf.yOffset) & 0x1ff;
		xscr &= 0x1ff;
		yscr &= 0x1ff;
		adr = ((vid->tsconf.vidPage & 0xf8) << 14) + (yscr << 8) + (xscr >> 1);
		scrbyte = vid->mem->ramData[adr];
		if (xscr & 1) {
			col = scrbyte & 0x0f;			// right pixel
		} else {
			col = (scrbyte & 0xf0) >> 4 ;		// left pixel
		}
		col |= vid->tsconf.scrPal;
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// tsconf 8bpp

void vidDrawTSL256(Video* vid) {
	xscr = vid->ray.x - vid->tsconf.xPos;
	yscr = vid->ray.y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr >= vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize) || vid->nogfx) {
		col = vid->brdcol;
	} else {
		xscr += vid->tsconf.xOffset;
		yscr = (vid->ray.y - vid->tsconf.yPos + vid->tsconf.yOffset) & 0x1ff;
		xscr &= 0x1ff;
		yscr &= 0x1ff;
		adr = ((vid->tsconf.vidPage & 0xf0) << 14) + (yscr << 9) + xscr;
		col = vid->mem->ramData[adr];
	}
	vidPutDot(&vid->ray, vid->pal, col);
}
*/

// tsconf text

void vidDrawTSLText(Video* vid) {
	xscr = vid->ray.x - vid->tsconf.xPos;
	yscr = vid->ray.y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr >= vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize)) {
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		if ((xscr & 3) == 0) {
			xscr += vid->tsconf.xOffset;
			yscr += vid->tsconf.yOffset;
			xscr &= 0x1ff;
			yscr &= 0x1ff;
			adr = (vid->tsconf.vidPage << 14) + ((yscr & 0x1f8) << 5) + (xscr >> 2);	// 256 bytes in row
			scrbyte = vid->mem->ramData[adr];
			col = vid->mem->ramData[adr | 0x80];
			ink = (col & 0x0f) | (vid->tsconf.scrPal);
			pap = ((col & 0xf0) >> 4)  | (vid->tsconf.scrPal);
			scrbyte = vid->mem->ramData[MADR(vid->tsconf.vidPage ^ 1, (scrbyte << 3) | (yscr & 7))];
			vidDrawByteDD(vid);
		}
	}
}

// baseconf text

void vidDrawEvoText(Video* vid) {
	yscr = vid->ray.y - 76;
	xscr = vid->ray.x - 96;
	if ((yscr < 0) || (yscr > 199) || (xscr < 0) || (xscr > 319)) {
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		if ((xscr & 3) == 0) {
			adr = 0x1c0 + ((yscr & 0xf8) << 3) + (xscr >> 3);
			if ((xscr & 7) == 0) {
				scrbyte = vid->mem->ramData[MADR(vid->curscr + 3, adr)];
				col = vid->mem->ramData[MADR(vid->curscr + 3, adr + 0x3000)];
			} else {
				scrbyte = vid->mem->ramData[MADR(vid->curscr + 3, adr + 0x1000)];
				col = vid->mem->ramData[MADR(vid->curscr + 3, adr + 0x2001)];
			}
			scrbyte = vid->font[(scrbyte << 3) | (yscr & 7)];
			vidATMDoubleDot(vid,col);
		}
	}
}
