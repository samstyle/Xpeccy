#include "video.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <assert.h>

// master clock: 21.48MHz (v9938)
// 1365(1368) master ticks / line (341.25 | 342 dots)
// 4 ticks / dot = 5.37MHz
// text: | 23 border | 240 screen | 22 border | 57 blank | = 342
// gra:  | 15 border | 256 screen | 14 border | 57 blank | = 342
// 192:  | 13 border | 192 screen | 12 border | 96 blank | = 313 (PAL)
// 212:  | 11 border | 212 screen | 10 border | 80 blank | = 313 (PAL)

extern int xscr, yscr, adr;
extern unsigned char scrbyte, atrbyte, col, ink, pap;

/*
// v9938 modes
enum {
	VDP_UNKNOWN = -1,
	VDP_TEXT1 = 0,
	VDP_GRA1,
	VDP_GRA2,
	VDP_MCOL,
	VDP_GRA3,
	VDP_GRA4,
	VDP_GRA5,
	VDP_GRA6,
	VDP_GRA7,
	VDP_TEXT2,
};
*/

static unsigned char m2lev[8] = {0x00,0x33,0x5c,0x7f,0xa2,0xc1,0xe1,0xff};

// colors was taken from wikipedia article
// https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_palettes#Original_MSX

static xColor msxPalete[16] = {
	{0,0,0},	// 0 : transparent (black)
	{0,0,0},	// 1 : black
	{62,184,73},	// 2 : medium green
	{116,208,123},	// 3 : light green
	{89,85,224},	// 4 : dark blue
	{128,118,241},	// 5 : light blue
	{185,94,81},	// 6 : dark red
	{101,219,239},	// 7 : cyan
	{219,101,89},	// 8 : medium red
	{255,137,125},	// 9 : light red
	{204,195,94},	// 10: dark yellow
	{222,208,135},	// 11: light yellow
	{58,162,165},	// 12: dark green
	{183,102,181},	// 13: magenta
	{204,204,204},	// 14: gray
	{255,255,255},	// 15: white
};

// color mix
unsigned char vdpMixColor(unsigned char colS, unsigned char colD, unsigned char mode) {
	if ((mode & 8) && !colD) return colS;
	switch (mode & 7) {
		case 0: colS = colD; break;
		case 1: colS &= colD; break;
		case 2: colS |= colD; break;
		case 3: colS ^= colD; break;
		case 4: colS = ~colD; break;
		default: colS = 0; break;
	}
	return colS;
}

// convert addr bus to absolute memory addr (main ram / expansion ram)
int vdpGetAddr(Video* vid, int adr) {
	adr &= vid->memMask;
	if (adr & 0x10000) return adr;	// high 64K
	if (vid->reg[45] & 0x40)	// expansion ram on
		adr |= 0x20000;
	return  adr;
}

// return relative dot addr for coordinates (x,y) and current video mode (bitmaps only)
int vdpDotAddr(Video* vid, int x, int y) {
	int adr = y * vid->scrsize.x; // vdp->core->wid;
	adr += x;
	adr /= vid->dpb;
	return  adr;
}

// sprites (mode 1, mode 2)

unsigned char atrp[8];

void vdpPutSpriteDot(Video* vid, int adr, unsigned char atr) {
	if (atr & 0x40) {
		vid->sprImg[adr] |= (atr & 0x0f);
	} else if (!vid->sprImg[adr]) {
		vid->sprImg[adr] = atr & 0x0f;
	}
}

void vdpDrawTile(Video* vid, int xpos, int ypos, int pat) {
	unsigned char src;
	int i,j,adr,xsav;
	int tadr = vid->OBJTiles + (pat << 3);
	for (i = 0; i < 8; i++) {
		src = vid->ram[tadr & vid->memMask];			// tile byte
		if ((ypos >= 0) && (ypos < vid->lines)) {		// if line onscreen
			xsav = xpos;
			if (atrp[i] & 0x80) {				// early clock (shift left 32 pix)
				xpos -= 32;
			}
			for (j = 0; j < 8; j++) {
				if ((xpos >= 0) && (xpos < 256) && (src & 0x80)) {	// if sprite has dot onscreen
					adr = (ypos << 8) + xpos;
					vdpPutSpriteDot(vid, adr, atrp[i]);
					if (vid->reg[1] & 1) {
						vdpPutSpriteDot(vid, adr+1, atrp[i]);
						vdpPutSpriteDot(vid, adr+256, atrp[i]);
						vdpPutSpriteDot(vid, adr+257, atrp[i]);
						xpos++;
					}
				}
				src <<= 1;
				xpos++;
			}
			xpos = xsav;
		}
		tadr++;
		ypos += (vid->reg[1] & 1) ? 2 : 1;
	}
}

void vdpFillSprites(Video* vid) {
//	vdpVBlk(vid);
	memset(vid->sprImg, 0x00, 0xd400);
	if (vid->reg[8] & 2) return;
	int i,sh;
	int adr = vid->OBJAttr;
	int ypos, xpos, pat, atr;
	for (i = 0; i < 32; i++) {
		if (vid->ram[adr] == 0xd0) break;
		ypos = (vid->ram[adr] + 1) & 0xff;
		xpos = vid->ram[adr+1] & 0xff;
		pat = vid->ram[adr+2] & 0xff;
		atr = vid->ram[adr+3] & 0xff;
		memset(atrp, atr & 0x8f, 8);
		if (vid->reg[1] & 2) {
			pat &= 0xfc;
			sh = (vid->reg[1] & 1) ? 16 : 8;
			vdpDrawTile(vid, xpos, ypos, pat);
			vdpDrawTile(vid, xpos, ypos+sh, pat+1);
			vdpDrawTile(vid, xpos+sh, ypos, pat+2);
			vdpDrawTile(vid, xpos+sh, ypos+sh, pat+3);
		} else {
			vdpDrawTile(vid, xpos, ypos, pat);
		}
		adr += 4;
	}
}

void vdpFillSprites2(Video* vid) {
//	vdpVBlk(vid);
	memset(vid->sprImg, 0x00, 0x10000);
	if (vid->reg[8] & 2) return;			// disable OBJ sprites
	int i,sh;
	int tadr = (vid->OBJAttr & 0x1fc00) | 0x200;	// a16...a10.1.x..x
	int aadr = tadr - 0x200;			// sprite color table
	int ypos, xpos, pat;
	for (i = 0; i < 32; i++) {
		if (vid->ram[tadr] == 0xd8) {
			i = 32;
		} else {
			ypos = (vid->ram[tadr] + 1) & 0xff;
			xpos = vid->ram[tadr + 1] & 0xff;
			pat = vid->ram[tadr + 2] & 0xff;
			memcpy(atrp, &vid->ram[aadr], 8);
			if (vid->reg[1] & 2) {
				pat &= 0xfc;
				sh = (vid->reg[1] & 1) ? 16 : 8;
				vdpDrawTile(vid, xpos, ypos, pat);		// UL
				vdpDrawTile(vid, xpos+sh, ypos, pat+2);		// UR
				memcpy(atrp, &vid->ram[aadr + 8], 8);
				vdpDrawTile(vid, xpos, ypos+sh, pat+1);		// DL
				vdpDrawTile(vid, xpos+sh, ypos+sh, pat+3);	// DR
			} else {
				vdpDrawTile(vid, xpos, ypos, pat);
			}
			aadr += 0x10;
			tadr += 4;
		}
	}
}

// v9918 TEXT1 (40 x 24 text)

void vdpText1(Video* vid) {
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->reg[7] & 15;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr > 239)) {
			col = vid->reg[7] & 15;
		} else {
			col = 0;
			if ((xscr % 6) == 0) {
				adr = vid->ram[((yscr & 0xf8) * 5) + (xscr / 6)];
				scrbyte = vid->ram[0x800 + ((adr << 3) | (yscr & 7))];
				ink = (vid->reg[7] & 0xf0) >> 4;
				pap = vid->reg[7] & 0x0f;
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9918 G1 (32 x 24 text)

void vdpGra1(Video* vid) {
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->reg[7] & 15;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->reg[7] & 15;
		} else {
			if ((xscr & 7) == 0) {
				adr = vid->ram[vid->BGMap + (xscr >> 3) + ((yscr & 0xf8) << 2)];
				scrbyte = vid->ram[vid->BGTiles + (adr << 3) + (yscr & 7)];
				atrbyte = vid->ram[vid->BGColors + (adr >> 3)];
				ink = (atrbyte & 0xf0) >> 4;
				pap = atrbyte & 0x0f;
			}
			col = vid->sprImg[(yscr << 8) | xscr];
			if (!col) {
				col = (scrbyte & 0x80) ? ink : pap;
			}
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9918 G2 (256 x 192)

void vdpGra2(Video* vid) {
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->reg[7];
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->reg[7];
		} else {
			if ((xscr & 7) == 0) {
				adr = vid->ram[vid->BGMap + (xscr >> 3) + ((yscr & 0xf8) << 2)] | ((yscr & 0xc0) << 2);	// tile nr
				scrbyte = vid->ram[(vid->BGTiles & ~0x1fff) + (adr << 3) + (yscr & 7)];
				atrbyte = vid->ram[(vid->BGColors & ~0x1fff) + (adr << 3) + (yscr & 7)];
				ink = (atrbyte & 0xf0) >> 4;
				pap = atrbyte & 0x0f;
			}
			col = vid->sprImg[(yscr << 8) | xscr];
			if (!col) {
				col = (scrbyte & 0x80) ? ink : pap;
			}
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9918 MC (multicolor)

void vdpMultcol(Video* vid) {
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->reg[7];
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->reg[7];
		} else {
			adr = vid->ram[vid->BGMap + (xscr >> 3) + ((yscr & 0xf8) << 2)];		// color index
			adr = vid->BGTiles + (adr << 3) + ((yscr & 0x18) >> 2) + ((yscr & 4) >> 2);	// color adr
			col = vid->ram[adr];								// color for 2 dots
			if (!(xscr & 4)) {
				col >>= 4;
			}
			col &= 0x0f;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9938 G4 (256x212 4bpp)

void vdpGra4(Video* vid) {
	yscr = vid->ray.y - vid->bord.y - vid->sc.y;
	if ((yscr < 0) || (yscr >= vid->lines)) {
		col = vid->reg[7] & 15;
	} else {
		xscr = vid->ray.x - vid->bord.x - vid->sc.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->reg[7] & 15;
		} else {
			yscr += vid->reg[0x17];
			yscr &= 0xff;
			xscr &= 0xff;
			if (xscr & 1) {
				col = ink & 15;
			} else {
				adr = (vid->BGMap & 0x18000) | (xscr >> 1) | (yscr << 7);
				ink = vid->ram[adr & vid->memMask];		// color byte
				col = (ink >> 4) & 15;
			}
			pap = vid->sprImg[(yscr << 8) | xscr];
			if (pap) col = pap;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

void vdpG4pset(Video* vid, int x, int y, unsigned char col) {
	//adr = (vdp->BGMap & 0x18000) + (x >> 1) + (y << 7);
	adr = ((x & 0xff) >> 1) | (y << 7);
	scrbyte = vid->ram[adr & vid->memMask];
	if (x & 1) {
		pap = vdpMixColor((scrbyte & 0xf0) >> 4, col & 0x0f, vid->arg) & 0x0f;
		scrbyte = (scrbyte & 0xf0) | (pap & 0x0f);
	} else {
		pap = vdpMixColor(scrbyte & 0x0f, col & 0x0f, vid->arg) & 0x0f;
		scrbyte = (scrbyte & 0x0f) | ((pap << 4) & 0xf0);
	}
	vid->ram[adr] = scrbyte;
}

unsigned char vdpG4col(Video* vid, int x, int y) {
	// adr = (vdp->BGMap & 0x18000) + (x >> 1) + (y << 7);
	adr = ((x & 0xff) >> 1) | (y << 7);
	return (x & 1) ? vid->ram[adr] & 0x0f : (vid->ram[adr] & 0xf0) >> 4;
}

// v9938 G5 (512x212 2bpp)

void vdpGra5(Video* vid) {
	yscr = vid->ray.y - vid->bord.y - vid->sc.y;
	if ((yscr < 0) || (yscr >= vid->lines)) {
		vidPutDot(&vid->ray, vid->pal, vid->reg[7] & 3);
	} else {
		xscr = vid->ray.x - vid->bord.x - vid->sc.x;
		if ((xscr < 0) || (xscr > 255)) {
			vidPutDot(&vid->ray, vid->pal, vid->reg[7] & 3);
		} else {
			yscr += vid->reg[0x17];
			yscr &= 0xff;
			xscr &= 0xff;
			if (xscr & 1) {
				adr = (vid->BGMap & 0x18000) + (xscr >> 1) + (yscr << 7);
				col = vid->ram[adr];
			}
			vidSingleDot(&vid->ray, vid->pal, (col & 0xc0) >> 6);
			vidSingleDot(&vid->ray, vid->pal, (col & 0x30) >> 4);
			col <<= 4;
		}
	}
}

void vdpG5pset(Video* vid, int x, int y, unsigned char col) {
	adr = ((x & 0x1ff) >> 2) | (y << 7);
	scrbyte = vid->ram[adr & vid->memMask];
	col &= 3;
	switch (x & 3) {
		case 0:
			pap = vdpMixColor((scrbyte & 0x3f) >> 6, col, vid->arg) & 3;
			scrbyte = (scrbyte & 0x3f) | ((pap << 6) & 0xc0);
			break;
		case 1:
			pap = vdpMixColor((scrbyte & 0xcf) >> 4, col, vid->arg) & 3;
			scrbyte = (scrbyte & 0xcf) | ((pap << 4) & 0x30);
			break;
		case 2:
			pap = vdpMixColor((scrbyte & 0xf3) >> 2, col, vid->arg) & 3;
			scrbyte = (scrbyte & 0xf3) | ((pap << 2) & 0x0c);
			break;
		case 3:
			pap = vdpMixColor(scrbyte & 0xfc, col, vid->arg) & 3;
			scrbyte = (scrbyte & 0xfc) | (pap & 0x03);
			break;
	}
	vid->ram[adr] = scrbyte;
}

unsigned char vdpG5col(Video* vid, int x, int y) {
	adr = ((x & 0x1ff) >> 2) | (y << 7);
	switch (x & 3) {
		case 0: pap = (vid->ram[adr] & 0xc0) >> 6; break;
		case 1: pap = (vid->ram[adr] & 0x30) >> 4; break;
		case 2: pap = (vid->ram[adr] & 0x0c) >> 2; break;
		case 3: pap = vid->ram[adr] & 0x03; break;
	}
	return pap;
}

// v9938 G6 (512x212 4bpp)

void vdpGra6(Video* vid) {
	yscr = vid->ray.y - vid->bord.y - vid->sc.y;
	if ((yscr < 0) || (yscr >= vid->lines)) {
		vidPutDot(&vid->ray, vid->pal, vid->reg[7]);
	} else {
		xscr = vid->ray.x - vid->bord.x - vid->sc.x;
		if ((xscr < 0) || (xscr > 255)) {
			vidPutDot(&vid->ray, vid->pal, vid->reg[7]);
		} else {
			yscr += vid->reg[0x17];
			yscr &= 0xff;
			xscr &= 0xff;
			adr = (vid->BGMap & 0x18000) + xscr + (yscr << 8);
			col = vid->ram[adr];
			vidSingleDot(&vid->ray, vid->pal, (col & 0xf0) >> 4);
			vidSingleDot(&vid->ray, vid->pal, col & 0x0f);
		}
	}
}

void vdpG6pset(Video* vid, int x, int y, unsigned char col) {
	adr = ((x & 0x1ff) >> 1) | (y << 8);
	scrbyte = vid->ram[adr];
	if (x & 1) {
		pap = vdpMixColor((scrbyte & 0xf0) >> 4, col & 0x0f, vid->arg) & 0x0f;
		scrbyte = (scrbyte & 0xf0) | (pap & 0x0f);
	} else {
		pap = vdpMixColor(scrbyte & 0x0f, col & 0x0f, vid->arg) & 0x0f;
		scrbyte = (scrbyte & 0x0f) | ((pap << 4) & 0xf0);
	}
	vid->ram[adr] = scrbyte;
}

unsigned char vdpG6col(Video* vid, int x, int y) {
	adr = ((x & 0x1ff) >> 1) | (y << 8);
	return (x & 1) ? vid->ram[adr] & 0x0f : (vid->ram[adr] & 0xf0) >> 4;
}

// v9938 G6 (256x212 8bpp)
// note:sprites color is different

void vdpGra7(Video* vid) {
	unsigned char col;
	yscr = vid->ray.y - vid->bord.y - vid->sc.y;
	if ((yscr < 0) || (yscr >= vid->lines)) {
		col = vid->reg[7];
	} else {
		xscr = vid->ray.x - vid->bord.x - vid->sc.x;
		if (xscr & ~0xff) {
			col = vid->reg[7];
		} else {
			adr = ((vid->reg[2] & 0x20) ? 0x10000 : 0) | xscr | (yscr << 8);
			col = vid->ram[adr & vid->memMask];
		}
	}
	(*vid->ray.ptr++) = (col << 3) & 0xe0;		// r:3
	(*vid->ray.ptr++) = col & 0xe0;			// g:3
	(*vid->ray.ptr++) = (col << 6) & 0xe0;		// b:2
	(*vid->ray.ptr++) = (col << 3) & 0xe0;
	(*vid->ray.ptr++) = col & 0xe0;
	(*vid->ray.ptr++) = (col << 6) & 0xe0;
}

void vdpG7pset(Video* vid, int x, int y, unsigned char col) {
	adr = (x & 0xff) | (y << 8);
	vid->ram[adr & vid->memMask] = col;
}

unsigned char vdpG7col(Video* vid, int x, int y) {
	adr = (x & 0xff) | (y << 8);
	return vid->ram[adr & vid->memMask];
}

// dummy

void vdpBreak(Video* vid) {
	// assert(0);
}

void vdpDummy(Video* vid) {
	vidPutDot(&vid->ray, vid->pal, 0);
}

// tab

#if 0
typedef struct {
	int id;
	int wid;	// screen width (256|512)
	int dpb;	// dots per byte (bitmaps: 1|2|4)
	void(*draw)(Video*);
	void(*line)(Video*);
	void(*fram)(Video*);
	void(*pset)(Video*,int,int,unsigned char);
	unsigned char(*col)(Video*,int,int);
} vdpMode;


static vdpMode vdpTab[] = {
	{VDP_TEXT1, 256, 1, vdpText1, NULL, vdpFillSprites, NULL, NULL},
	{VDP_TEXT2, 256, 1, vdpDummy, NULL, vdpHBlk, NULL, NULL},
	{VDP_MCOL, 256, 1, vdpMultcol, NULL, vdpFillSprites, NULL, NULL},
	{VDP_GRA1, 256, 1, vdpGra1, NULL, vdpFillSprites, NULL, NULL},
	{VDP_GRA2, 256, 1, vdpGra2, NULL, vdpFillSprites, NULL, NULL},
	{VDP_GRA3, 256, 1, vdpGra2, NULL, vdpFillSprites2, NULL, NULL},
	{VDP_GRA4, 256, 2, vdpGra4, NULL, vdpFillSprites2, vdpG4pset, vdpG4col},
	{VDP_GRA5, 512, 4, vdpGra5, NULL, vdpFillSprites2, vdpG5pset, vdpG5col},
	{VDP_GRA6, 512, 2, vdpGra6, NULL, vdpFillSprites2, vdpG6pset, vdpG6col},
	{VDP_GRA7, 256, 1, vdpGra7, NULL, vdpFillSprites2, vdpG7pset, vdpG7col},
	{VDP_UNKNOWN, 256, 1, vdpDummy, NULL, NULL, NULL, NULL}
};
#else

typedef struct {
	int id;
	int wid;	// screen width (256|512)
	int dpb;	// dots per byte (bitmaps: 1|2|4)
	void(*pset)(Video*,int,int,unsigned char);
	unsigned char(*col)(Video*,int,int);
} vdpMode;


static vdpMode vdpTab[] = {
	{VDP_TEXT1, 256, 1, NULL, NULL},
	{VDP_TEXT2, 256, 1, NULL, NULL},
	{VDP_MCOL, 256, 1, NULL, NULL},
	{VDP_GRA1, 256, 1, NULL, NULL},
	{VDP_GRA2, 256, 1, NULL, NULL},
	{VDP_GRA3, 256, 1, NULL, NULL},
	{VDP_GRA4, 256, 2, vdpG4pset, vdpG4col},
	{VDP_GRA5, 512, 4, vdpG5pset, vdpG5col},
	{VDP_GRA6, 512, 2, vdpG6pset, vdpG6col},
	{VDP_GRA7, 256, 1, vdpG7pset, vdpG7col},
	{VID_UNKNOWN, 256, 1, NULL, NULL}
};

#endif

void vdpSetMode(Video* vid, int mode) {
	int idx = 0;
	//printf("v9938 mode = %i\n",vdp->vmode);
	while((vdpTab[idx].id != mode) && (vdpTab[idx].id != VID_UNKNOWN)) {
		idx++;
	}
//	vid->vmode = vdpTab[idx].id;
//	vid->cbDot = vdpTab[idx].draw;
//	vid->cbLine = vdpTab[idx].line;
//	vid->cbFrame = vdpTab[idx].fram;
	vidSetMode(vid, vdpTab[idx].id);
	vid->pset = vdpTab[idx].pset;
	vid->col = vdpTab[idx].col;
	vid->scrsize.x = vdpTab[idx].wid;
	vid->dpb = vdpTab[idx].dpb;
}

void vdpReset(Video* vid) {
//	for (int i = 0; i < 64; i++)
//		vid->reg[i] = 0;
	memset(vid->reg, 0x00, 64);
	memset(vid->ram, 0x00, MEM_128K);
	vdpSetMode(vid, VDP_TEXT1);
	for (int i = 0; i < 16; i++)
		vid->pal[i] = msxPalete[i];
	vid->BGColors = 0;
	vid->BGMap = 0;
	vid->BGTiles = 0;
	vid->OBJAttr = 0;
	vid->OBJTiles = 0;
	vid->sc.x = 0;
	vid->sc.y = 0;
	vid->vadr = 0;
	vid->high = 0;
	vid->palhi = 0;
}

unsigned char vdpReadSR(Video* vid) {
	unsigned char idx = vid->reg[0x0f] & 0x0f;
	unsigned char res = vid->sr[idx];
	xscr = vid->ray.x - vid->scrn.x;
	yscr = vid->ray.y - vid->scrn.y;
	switch (idx) {
		case 0:
			vid->sr[0] &= 0x7f;		// reset VINT flag
			break;
		case 1:
			vid->sr[1] &= 0xfe;		// reset HINT flag
			break;
		case 2:
			res &= 0x9f;
			if ((xscr < 0) || (xscr > 255))			// horizontal retrace (border + HBlank)
				res |= 0x20;
			if ((yscr < 0) || (yscr >= vid->lines))		// vertical retrace (border + VBlank)
				res |= 0x40;
			break;
		case 3: res = xscr & 0xff; break;
		case 4: res = (xscr >> 8) & 1; break;
		case 5: res = yscr & 0xff; break;
		case 6: res = (yscr >> 8) & 3; break;
		default:
			break;
	}
//	printf("SR %i = %.2X\n",idx,res);
	return res;
}

void vdpSend(Video* vid, unsigned char val) {
	if (vid->pset)
		vid->pset(vid, vid->pos.x, vid->pos.y, val);
	vid->pos.x += vid->delta.x;
	if (++vid->cnt.x >= vid->size.x) {
		vid->pos.x = vid->dst.x;
		vid->cnt.x = 0;
		vid->pos.y += vid->delta.y;
		if (++vid->cnt.y >= vid->size.y) {
			vid->sr[2] &= 0xfe;
		}
	}
}

// commands
// + 0 : STOP
// + 5 : PSET	(dstX, dstY)	col = r2C;
// + 7 : LINE	start:(dstX,dstY);	MAJ = r2D & 1;	dx = MAJ ? dLong : dShort;	dy = MAJ ? dShort : dLong;	col = r2C;
// * 9 : copy rect
// + B : copy rect CPU->VDP
// ~ C : fill rect
// * D : copy rect
// ~ E : move right side of lines up

void vdpExec(Video* vid) {
	int spx,dpx;
	switch (vid->com) {
		case 0:
			vid->sr[2] &= 0xfe;
			break;
		case 5:
			if (vid->pset)
				vid->pset(vid, vid->dst.x, vid->dst.y, vid->reg[0x2c]);
			break;
		case 7:
			vid->pos.x = (vid->dst.x << 4) + 8;
			vid->pos.y = (vid->dst.y << 4) + 8;
			vid->count = (vid->size.x > vid->size.y) ? vid->size.x : vid->size.y;
			// printf("size = %i %i\n",vdp->size.x,vdp->size.y);
			if (vid->reg[0x2d] & 1) {
				vid->delta.x = vid->size.y ? (vid->size.x << 4) / vid->size.y : 0;
				vid->delta.y = 0x10;
			} else {
				vid->delta.x = 0x10;
				vid->delta.y = vid->size.y ? (vid->size.x << 4) / vid->size.y : 0;
			}
			if (vid->reg[0x2d] & 4)
				vid->delta.x = -vid->delta.x;
			if (vid->reg[0x2d] & 8)
				vid->delta.y = -vid->delta.y;
			while (vid->count > 0) {
				if (vid->pset)
					vid->pset(vid, vid->pos.x >> 4, vid->pos.y >> 4, vid->reg[0x2c]);
				vid->pos.x += vid->delta.x;
				vid->pos.y += vid->delta.y;
				vid->count--;
			}
			break;
		case 9:					// LMMM
			vid->delta.x = (vid->reg[0x2d] & 4) ? -1 : 1;
			vid->delta.y = (vid->reg[0x2d] & 8) ? -1 : 1;
			vid->cnt.x = 0;
			vid->cnt.y = 0;
			vid->sr[2] |= 1;
			spx = vid->src.x;
			dpx = vid->dst.x;
			while (vid->sr[2] & 1) {
				pap = vid->col ? vid->col(vid, vid->src.x, vid->src.y) : 0;
				if (vid->pset)
					vid->pset(vid, vid->dst.x, vid->dst.y, pap);
				vid->dst.x += vid->delta.x;
				vid->src.x += vid->delta.x;
				if (++vid->cnt.x >= vid->size.x) {
					vid->cnt.x = 0;
					vid->dst.x = dpx;
					vid->src.x = spx;
					vid->dst.y += vid->delta.y;
					vid->src.y += vid->delta.y;
					if (++vid->cnt.y >= vid->size.y) {
						vid->sr[2] &= 0xfe;
					}
				}
			}
			break;
		case 0xb:
			vid->cnt.x = 0;
			vid->cnt.y = 0;
			vid->delta.x = (vid->reg[0x2d] & 4) ? -1 : 1;
			vid->delta.y = (vid->reg[0x2d] & 8) ? -1 : 1;
			vid->pos = vid->dst;
			vid->sr[2] |= 1;
			vdpSend(vid, vid->reg[0x2c]);
			break;
		case 0xc:
			vid->cnt.x = 0;
			vid->cnt.y = 0;
			vid->delta.x = (vid->reg[0x2d] & 4) ? -1 : 1;
			vid->delta.y = (vid->reg[0x2d] & 8) ? -1 : 1;
			vid->pos = vid->dst;
			vid->sr[2] |= 1;
			while (vid->sr[2] & 1) {
				vdpSend(vid, vid->reg[0x2c]);
			}
			break;
		case 0x0d:
			// printf("com D : (%i,%i)x(%i,%i)->(%i,%i):%.2X\n",vdp->src.x, vdp->src.y, vdp->size.x, vdp->size.y, vdp->dst.x, vdp->dst.y, vdp->reg[45]);
			if (vid->reg[45] & 0x04) {			// dX left
				vid->src.x -= vid->size.x;
				vid->dst.x -= vid->size.x;
			}
			if (vid->reg[45] & 0x08) {			// dY up
				vid->src.y -= vid->size.y;
				vid->dst.y -= vid->size.y;
			}
			spx = vid->BGMap | vdpDotAddr(vid, vid->src.x, vid->src.y);	// source addr
			dpx = vid->BGMap | vdpDotAddr(vid, vid->dst.x, vid->dst.y);	// dest addr
			if (vid->reg[45] & 0x10) spx = ((spx & 0xffff) | 0x20000);	// src exp ram
			if (vid->reg[45] & 0x20) dpx = ((dpx & 0xffff) | 0x20000);	// dst exp ram
			vid->cnt.x = vid->size.x / vid->dpb;			// dX bytes
			vid->delta.y = vid->scrsize.x / vid->dpb;			// bytes / line
			while ((vid->size.y > 0) && (spx >= 0) && (dpx >= 0)) {
				memcpy(vid->ram + dpx, vid->ram + spx, vid->cnt.x);
				spx += vid->delta.y;
				dpx += vid->delta.y;
				vid->size.y--;
			}
			break;
		case 0x0e:
			vid->delta.x = (vid->reg[0x2d] & 4) ? -1 : 1;
			vid->delta.y = (vid->reg[0x2d] & 8) ? -1 : 1;
			vid->size.x = (vid->delta.x < 0) ? vid->dst.x : (512 - vid->dst.x);		// TODO: get X-size of screen
			vid->cnt.y = vid->size.y;
			while (vid->cnt.y > 0) {
				vid->pos.x = vid->dst.x;
				vid->cnt.x = vid->size.x;
				while (vid->cnt.x > 0) {
					vid->cnt.x--;
					pap = vid->col ? vid->col(vid, vid->pos.x, vid->src.y) : 0;
					if (vid->pset)
						vid->pset(vid, vid->pos.x, vid->dst.y, pap);
					vid->pos.x += vid->delta.x;
				}
				vid->dst.y += vid->delta.y;
				vid->src.y += vid->delta.y;
				vid->cnt.y--;
			}
			break;
		default:
			printf("vdp9938 command %.2X, arg %.2X\n",vid->com, vid->arg);
			break;
	}
}

void vdpRegWr(Video* vid, int reg, unsigned char val) {
	int vmode;
	reg &= 0x3f;
	vid->reg[reg] = val;
	// printf("%i:#%.2X\n",reg,val);
	switch (reg) {
		// mode registers
		case 0x00:
		case 0x01:
			vmode = ((vid->reg[1] & 0x10) >> 4) | ((vid->reg[1] & 8) >> 2) | ((vid->reg[0] & 0x0e) << 1);
			// printf("v9938 mode = %.2X\n",vmode);
			switch (vmode) {
				case 0x01: vdpSetMode(vid, VDP_TEXT1); break;	// text 40x24
				case 0x09: vdpSetMode(vid, VDP_TEXT2); break;	// text 80x24
				case 0x02: vdpSetMode(vid, VDP_MCOL); break;	// multicolor 4x4
				case 0x00: vdpSetMode(vid, VDP_GRA1); break;	// text 32x24
				case 0x04: vdpSetMode(vid, VDP_GRA2); break;	// 256x192
				case 0x08: vdpSetMode(vid, VDP_GRA3); break;	// scr2 8spr/line
				case 0x0c: vdpSetMode(vid, VDP_GRA4); break;	// 256x212,4bpp
				case 0x10: vdpSetMode(vid, VDP_GRA5); break;	// 512x212,2bpp
				case 0x14: vdpSetMode(vid, VDP_GRA6); break;	// 512x212,4bpp
				case 0x1c: vdpSetMode(vid, VDP_GRA7); break;	// 256x212,8bpp
				default:
					printf("v9938 mode %.2X\n",vmode);
					// assert(0);
					vdpSetMode(vid, VID_UNKNOWN);
					break;
			}
			break;
		case 0x08: break;						// mode reg 2
		case 0x09: vid->lines = (val & 0x80) ? 212 : 192; break;	// mode reg 3
			// address registers
		case 0x02:
			vid->BGMap = (val & 0x7f) << 10;
			// printf("BGMap = %X (%.2X)\n",vdp->BGMap,val);
			break;
		case 0x04: vid->BGTiles = (val & 0x3f) << 11; break;
		case 0x06: vid->OBJTiles = (val & 0x3f) << 11; break;

		case 0x03:
		case 0x0a: vid->BGColors  = ((vid->reg[0x0a] & 7) << 14) | (vid->reg[0x03] << 6); break;

		case 0x05:
		case 0x0b: vid->OBJAttr = ((vid->reg[0x0b] & 3) << 15) | (vid->reg[0x05] << 7); break;
			// color registers
		case 0x07: break;			// border color = BG in R7
		case 0x0c: break;			// inv/blink colors
		case 0x0d:			// blink period
			vid->blink0 = ((val >> 4) & 0x0f) * 6;		// 1 = ~6 frames
			vid->blink1 = (val & 0x0f) * 6;
			if (!vid->blink0) vid->bpage = 1;
			if (!vid->blink1) vid->bpage = 0;
			vid->blink = vid->bpage ? vid->blink1 : vid->blink0;
			break;
		case 0x14: break;			// 14,15,16 : color burst registers
		case 0x15: break;
		case 0x16: break;
			// display registers
		case 0x12: vid->sc.x = (val & 15) - (val & 8) ? 16 : 0;
			vid->sc.y = ((val & 0xf0) >> 4) - (val & 0x80) ? 16 : 0;
			break;
		case 0x17: break;			// display Y offset
		case 0x13: vid->intp.y = val; break;
			// access registers
		case 0x0e:
			vid->vadr = (vid->vadr & 0x3fff) | ((val & 7) << 14);
			// printf("%.2X:%X\n",val,vdp->vadr);
			break;
		case 0x0f: break;			// status reg num (0..9)
		case 0x10: break;			// palette num
		case 0x11: /*vdp->regIdx = val & 0x3f*/; break;
			// command registers
		case 0x20:
		case 0x21: vid->src.x = (vid->reg[0x20] | (vid->reg[0x21] << 8)) & 0x1ff; break;

		case 0x22:
		case 0x23: vid->src.y = (vid->reg[0x22] | (vid->reg[0x23] << 8)) & 0x3ff; break;

		case 0x24:
		case 0x25: vid->dst.x = (vid->reg[0x24] | (vid->reg[0x25] << 8)) & 0x1ff; break;

		case 0x26:
		case 0x27: vid->dst.y = (vid->reg[0x26] | (vid->reg[0x27] << 8)) & 0x3ff; break;

		case 0x28:
		case 0x29: vid->size.x = (vid->reg[0x28] | (vid->reg[0x29] << 8)) & 0x1ff; break;

		case 0x2a:
		case 0x2b: vid->size.y = (vid->reg[0x2a] | (vid->reg[0x2b] << 8)) & 0x3ff; break;

		case 0x2c:			// color code | block data transfer cpu->vdp
			if (vid->sr[2] & 1) {
				vdpSend(vid, val);
			}
			break;
		case 0x2d: break;		// command argument
		case 0x2e:
			vid->arg = val & 15;
			vid->com = (val & 0xf0) >> 4;
			vdpExec(vid);
			break;
		case 0x2f:			// WUT???
			break;
		default:
			printf("v9938 register : wr #%.2X,#%.2X\n",reg,val);
			// assert(0);
			break;
	}
}

// 20150923

void vdpDraw(Video* vid) {
//	if (vid->inth) vid->inth--;
//	if (vid->intf) vid->intf--;
	//vdp->draw(vdp);
}

/*
void vdpLine(Video* vdp) {
	if (vdp->line)
		vdp->line(vdp);
}
*/

void vdpHBlk(Video* vid) {
	yscr = vid->ray.x - vid->bord.y;
	if (yscr == vid->intp.y) {		// HINT
		vid->sr[1] |= 1;
		if (vid->reg[0] & VDP_IE1) {
			vid->inth = 64;
		}
	}
	if (vid->ray.y == vid->vend.y) {
		vdpVBlk(vid);
	}
}

void vdpVBlk(Video* vid) {
	vid->sr[0] |= 0x80;
	if (vid->reg[1] & VDP_IE0) {
		vid->intf = 64;
	}
	vid->blink--;
	if (vid->blink < 0) {
		vid->blink = vid->bpage ? vid->blink0 : vid->blink1;
		if (vid->blink)
			vid->bpage ^= 1;
	}
}

unsigned char vdpRead(Video* vid, int port) {
	unsigned char res = 0xff;
	vid->high = 0;
	switch (port & 3) {
		case 0:
			res = vid->ram[vdpGetAddr(vid, vid->vadr)];
			vid->vadr++;
			break;
		case 1:
			res = vdpReadSR(vid);
			break;
	}
	return res;
}

void vdpWrite(Video* vid, int port, unsigned char val) {
	int num;
	switch (port & 3) {
		case 0:
			vid->ram[vdpGetAddr(vid, vid->vadr)] = val;
			vid->vadr++;
			break;
		case 1:
//			printf("port1.%i:%.2X\n",vdp->high,val);
			if (vid->high) {
				if (val & 0x80) {
					vdpRegWr(vid, val & 0x3f, vid->dat);
				} else {
					vid->vadr &= 0x1c000;
					vid->vadr |= vid->dat;
					vid->vadr |= ((val & 0x3f) << 8);
				}
				vid->high = 0;
			} else {
				vid->dat = val;
				vid->high = 1;
			}
			break;
		case 2:
			num = vid->reg[16] & 15;
			if (vid->palhi) {
				vid->pal[num].g = m2lev[val & 7];
				vid->reg[16] = (num + 1) & 15;
				vid->palhi = 0;
			} else {
				vid->pal[num].b = m2lev[val & 7];
				vid->pal[num].r = m2lev[(val >> 4) & 7];
				vid->palhi = 1;
			}
			break;
		case 3:
			num = vid->reg[17] & 0x3f;
			if (num != 17) {
				vdpRegWr(vid, num, val);
			}
			if (!(vid->reg[17] & 0x80)) {
				num++;
				vid->reg[17] &= 0x80;
				vid->reg[17] |= (num & 0x3f);
			}
			break;
	}
}
