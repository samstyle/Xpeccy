#include "v9938.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <assert.h>

extern int xscr, yscr, adr;
extern unsigned char scrbyte, atrbyte, col, ink, pap;

// v9938 modes
enum {
	VID_MSX_SCR0 = 0,
	VID_MSX_SCR1,
	VID_MSX_SCR2,
	VID_MSX_SCR3,
	VID_MSX_SCR4,
	VID_MSX_SCR5,
	VID_MSX_SCR6,
	VID_MSX_SCR7,
	VID_MSX_SCR8,
	VID_MSX_SCR9,
	VID_MSX_NONE = -1
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

// sprites (mode 1, mode 2)

unsigned char atrp[8];

void vdpPutSpriteDot(VDP9938* vdp, int adr, unsigned char atr) {
	if (atr & 0x40) {
		vdp->sprImg[adr] |= (atr & 0x0f);
	} else if (!vdp->sprImg[adr]) {
		vdp->sprImg[adr] = atr & 0x0f;
	}
}

void vdpDrawTile(VDP9938* vdp, int xpos, int ypos, int pat) {
	unsigned char src;
	int i,j,adr,xsav;
	int tadr = vdp->OBJTiles + (pat << 3);
	for (i = 0; i < 8; i++) {
		src = vdp->ram[tadr & vdp->memMask];			// tile byte
		if ((ypos >= 0) && (ypos < vdp->lines)) {		// if line onscreen
			xsav = xpos;
			if (atrp[i] & 0x80) {				// early clock (shift left 32 pix)
				xpos -= 32;
			}
			for (j = 0; j < 8; j++) {
				if ((xpos >= 0) && (xpos < 256) && (src & 0x80)) {	// if sprite has dot onscreen
					adr = (ypos << 8) + xpos;
					vdpPutSpriteDot(vdp, adr, atrp[i]);
					if (vdp->reg[1] & 1) {
						vdpPutSpriteDot(vdp, adr+1, atrp[i]);
						vdpPutSpriteDot(vdp, adr+256, atrp[i]);
						vdpPutSpriteDot(vdp, adr+257, atrp[i]);
						xpos++;
					}
				}
				src <<= 1;
				xpos++;
			}
			xpos = xsav;
		}
		tadr++;
		ypos += (vdp->reg[1] & 1) ? 2 : 1;
	}
}

void vdpFillSprites(VDP9938* vdp) {
	memset(vdp->sprImg, 0x00, 0xd400);
	if (vdp->reg[8] & 2) return;
	int i,sh;
	int adr = vdp->OBJAttr;
	int ypos, xpos, pat, atr;
	for (i = 0; i < 32; i++) {
		if (vdp->ram[adr] == 0xd0) break;
		ypos = (vdp->ram[adr] + 1) & 0xff;
		xpos = vdp->ram[adr+1] & 0xff;
		pat = vdp->ram[adr+2] & 0xff;
		atr = vdp->ram[adr+3] & 0xff;
		memset(atrp, atr & 0x8f, 8);
		if (vdp->reg[1] & 2) {
			pat &= 0xfc;
			sh = (vdp->reg[1] & 1) ? 16 : 8;
			vdpDrawTile(vdp, xpos, ypos, pat);
			vdpDrawTile(vdp, xpos, ypos+sh, pat+1);
			vdpDrawTile(vdp, xpos+sh, ypos, pat+2);
			vdpDrawTile(vdp, xpos+sh, ypos+sh, pat+3);
		} else {
			vdpDrawTile(vdp, xpos, ypos, pat);
		}
		adr += 4;
	}
}

void vdpFillSprites2(VDP9938* vdp) {
	memset(vdp->sprImg, 0x00, 0xd400);
	if (vdp->reg[8] & 2) return;			// disable OBJ sprites
	int i,sh;
	int tadr = (vdp->OBJAttr & 0x1fc00) | 0x200;	// a16...a10.1.x..x
	int aadr = tadr - 0x200;			// sprite color table
	int ypos, xpos, pat;
	for (i = 0; i < 32; i++) {
		if (vdp->ram[tadr] == 0xd8) {
			i = 32;
		} else {
			ypos = (vdp->ram[tadr] + 1) & 0xff;
			xpos = vdp->ram[tadr + 1] & 0xff;
			pat = vdp->ram[tadr + 2] & 0xff;
			memcpy(atrp, &vdp->ram[aadr], 8);
			if (vdp->reg[1] & 2) {
				pat &= 0xfc;
				sh = (vdp->reg[1] & 1) ? 16 : 8;
				vdpDrawTile(vdp, xpos, ypos, pat);		// UL
				vdpDrawTile(vdp, xpos+sh, ypos, pat+2);		// UR
				memcpy(atrp, &vdp->ram[aadr + 8], 8);
				vdpDrawTile(vdp, xpos, ypos+sh, pat+1);		// DL
				vdpDrawTile(vdp, xpos+sh, ypos+sh, pat+3);	// DR
			} else {
				vdpDrawTile(vdp, xpos, ypos, pat);
			}
			aadr += 0x10;
			tadr += 4;
		}
	}
}

// v9918 scr 0 (40 x 24 text)

void vdpMsxScr0(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vdp->reg[7] & 15;
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x;
		if ((xscr < 0) || (xscr > 239)) {
			col = vdp->reg[7] & 15;
		} else {
			col = 0;
			if ((xscr % 6) == 0) {
				adr = vdp->ram[((yscr & 0xf8) * 5) + (xscr / 6)];
				scrbyte = vdp->ram[0x800 + ((adr << 3) | (yscr & 7))];
				ink = (vdp->reg[7] & 0xf0) >> 4;
				pap = vdp->reg[7] & 0x0f;
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vidPutDot(vdp->ray, vdp->pal, col);
}

// v9918 scr 1 (32 x 24 text)

void vdpMsxScr1(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vdp->reg[7] & 15;
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vdp->reg[7] & 15;
		} else {
			if ((xscr & 7) == 0) {
				adr = vdp->ram[vdp->BGMap + (xscr >> 3) + ((yscr & 0xf8) << 2)];
				scrbyte = vdp->ram[vdp->BGTiles + (adr << 3) + (yscr & 7)];
				atrbyte = vdp->ram[vdp->BGColors + (adr >> 3)];
				ink = (atrbyte & 0xf0) >> 4;
				pap = atrbyte & 0x0f;
			}
			col = vdp->sprImg[(yscr << 8) | xscr];
			if (!col) {
				col = (scrbyte & 0x80) ? ink : pap;
			}
			scrbyte <<= 1;
		}
	}
	vidPutDot(vdp->ray, vdp->pal, col);
}

// v9918 scr 2 (256 x 192)

void vdpMsxScr2(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vdp->reg[7];
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vdp->reg[7];
		} else {
			if ((xscr & 7) == 0) {
				adr = vdp->ram[vdp->BGMap + (xscr >> 3) + ((yscr & 0xf8) << 2)] | ((yscr & 0xc0) << 2);	// tile nr
				scrbyte = vdp->ram[(vdp->BGTiles & ~0x1fff) + (adr << 3) + (yscr & 7)];
				atrbyte = vdp->ram[(vdp->BGColors & ~0x1fff) + (adr << 3) + (yscr & 7)];
				ink = (atrbyte & 0xf0) >> 4;
				pap = atrbyte & 0x0f;
			}
			col = vdp->sprImg[(yscr << 8) | xscr];
			if (!col) {
				col = (scrbyte & 0x80) ? ink : pap;
			}
			scrbyte <<= 1;
		}
	}
	vidPutDot(vdp->ray, vdp->pal, col);
}

// v9918 scr 3 (multicolor)

void vdpMsxScr3(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vdp->reg[7];
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vdp->reg[7];
		} else {
			adr = vdp->ram[vdp->BGMap + (xscr >> 3) + ((yscr & 0xf8) << 2)];		// color index
			adr = vdp->BGTiles + (adr << 3) + ((yscr & 0x18) >> 2) + ((yscr & 4) >> 2);	// color adr
			col = vdp->ram[adr];								// color for 2 dots
			if (!(xscr & 4)) {
				col >>= 4;
			}
			col &= 0x0f;
		}
	}
	vidPutDot(vdp->ray, vdp->pal, col);
}

// v9938 scr 5 (256x212 4bpp)

void vdpMsxScr5(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y - vdp->vAdj;
	if ((yscr < 0) || (yscr >= vdp->lines)) {
		col = vdp->reg[7];
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x - vdp->hAdj;
		if ((xscr < 0) || (xscr > 255)) {
			col = vdp->reg[7];
		} else {
			yscr += vdp->reg[0x17];
			yscr &= 0xff;
			xscr &= 0xff;
			if (xscr & 1) {
				col = ink & 15;
			} else {
				adr = (vdp->BGMap & 0x18000) + (xscr >> 1) + (yscr << 7);
				ink = vdp->ram[adr & 0x1ffff];			// color byte
				col = (ink >> 4) & 15;
			}
			pap = vdp->sprImg[(yscr << 8) | xscr];
			if (pap) col = pap;
		}
	}
	vidPutDot(vdp->ray, vdp->pal, col);
}

void scr5plot(VDP9938* vdp, int x, int y, unsigned char col) {
	if ((x < 0) || (x > 255) || (y < 0) || (y > 212)) return;
	adr = (vdp->BGMap & 0x18000) + (x >> 1) + (y << 7);
	scrbyte = vdp->ram[adr];
	if (x & 1) {
		pap = vdpMixColor((scrbyte & 0xf0) >> 4, col & 0x0f, vdp->arg) & 0x0f;
		scrbyte = (scrbyte & 0xf0) | (pap & 0x0f);
	} else {
		pap = vdpMixColor(scrbyte & 0x0f, col & 0x0f, vdp->arg) & 0x0f;
		scrbyte = (scrbyte & 0x0f) | ((pap << 4) & 0xf0);
	}
	vdp->ram[adr] = scrbyte;
}

unsigned char scr5col(VDP9938* vdp, int x, int y) {
	if ((x < 0) || (x > 255) || (y < 0) || (y > 212)) return 0;
	adr = (vdp->BGMap & 0x18000) + (x >> 1) + (y << 7);
	return (x & 1) ? vdp->ram[adr] & 0x0f : (vdp->ram[adr] & 0xf0) >> 4;
}

// v9938 scr 6 (512x212 2bpp)

void vdpMsxScr6(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y - vdp->vAdj;
	if ((yscr < 0) || (yscr >= vdp->lines)) {
		vidPutDot(vdp->ray, vdp->pal, vdp->reg[7] & 3);
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x - vdp->hAdj;
		if ((xscr < 0) || (xscr > 255)) {
			vidPutDot(vdp->ray, vdp->pal, vdp->reg[7] & 3);
		} else {
			yscr += vdp->reg[0x17];
			yscr &= 0xff;
			xscr &= 0xff;
			if (xscr & 1) {
				adr = (vdp->BGMap & 0x18000) + (xscr >> 1) + (yscr << 7);
				col = vdp->ram[adr];
			}
			vidSingleDot(vdp->ray, vdp->pal, (col & 0xc0) >> 6);
			vidSingleDot(vdp->ray, vdp->pal, (col & 0x30) >> 4);
			col <<= 4;
		}
	}
}

void scr6plot(VDP9938* vdp, int x, int y, unsigned char col) {
	if ((x < 0) || (x > 511) || (y < 0) || (y > 212)) return;
	adr = (vdp->BGMap & 0x18000) + (x >> 2) + (y << 7);
	scrbyte = vdp->ram[adr];
	col &= 3;
	switch (x & 3) {
		case 0:
			pap = vdpMixColor((scrbyte & 0x3f) >> 6, col, vdp->arg) & 3;
			scrbyte = (scrbyte & 0x3f) | (pap << 6);
			break;
		case 1:
			pap = vdpMixColor((scrbyte & 0xcf) >> 4, col, vdp->arg) & 3;
			scrbyte = (scrbyte & 0xcf) | (pap << 4);
			break;
		case 2:
			pap = vdpMixColor((scrbyte & 0xf3) >> 2, col, vdp->arg) & 3;
			scrbyte = (scrbyte & 0xf3) | (pap << 2);
			break;
		case 3:
			pap = vdpMixColor(scrbyte & 0xfc, col, vdp->arg) & 3;
			scrbyte = (scrbyte & 0xfc) | pap;
			break;
	}
	vdp->ram[adr] = scrbyte;
}

unsigned char scr6col(VDP9938* vdp, int x, int y) {
	if ((x < 0) || (x > 511) || (y < 0) || (y > 212)) return 0;
	adr = (vdp->BGMap & 0x18000) + (x >> 2) + (y << 7);
	switch (x & 3) {
		case 0: pap = (vdp->ram[adr] & 0xc0) >> 6; break;
		case 1: pap = (vdp->ram[adr] & 0x30) >> 4; break;
		case 2: pap = (vdp->ram[adr] & 0x0c) >> 2; break;
		case 3: pap = vdp->ram[adr] & 0x03; break;
	}
	return pap;
}

// v9938 scr 7 (512x212 4bpp)

void vdpMsxScr7(VDP9938* vdp) {
	yscr = vdp->ray->y - vdp->lay->bord.y - vdp->vAdj;
	if ((yscr < 0) || (yscr >= vdp->lines)) {
		vidPutDot(vdp->ray, vdp->pal, vdp->reg[7]);
	} else {
		xscr = vdp->ray->x - vdp->lay->bord.x - vdp->hAdj;
		if ((xscr < 0) || (xscr > 255)) {
			vidPutDot(vdp->ray, vdp->pal, vdp->reg[7]);
		} else {
			yscr += vdp->reg[0x17];
			yscr &= 0xff;
			xscr &= 0xff;
			adr = (vdp->BGMap & 0x18000) + xscr + (yscr << 8);
			col = vdp->ram[adr];
			vidSingleDot(vdp->ray, vdp->pal, (col & 0xf0) >> 4);
			vidSingleDot(vdp->ray, vdp->pal, col & 0x0f);
		}
	}
}

void scr7plot(VDP9938* vdp, int x, int y, unsigned char col) {
	if ((x < 0) || (x > 511) || (y < 0) || (y > 212)) return;
	adr = (vdp->BGMap & 0x18000) + (x >> 1) + (y << 8);
	scrbyte = vdp->ram[adr];
	if (x & 1) {
		pap = vdpMixColor((scrbyte & 0xf0) >> 4, col & 0x0f, vdp->arg) & 0x0f;
		scrbyte = (scrbyte & 0xf0) | (pap & 0x0f);
	} else {
		pap = vdpMixColor(scrbyte & 0x0f, col & 0x0f, vdp->arg) & 0x0f;
		scrbyte = (scrbyte & 0x0f) | ((pap << 4) & 0xf0);
	}
	vdp->ram[adr] = scrbyte;
}

unsigned char scr7col(VDP9938* vdp, int x, int y) {
	if ((x < 0) || (x > 511) || (y < 0) || (y > 212)) return 0;
	adr = (vdp->BGMap & 0x18000) + (x >> 1) + (y << 8);
	return (x & 1) ? vdp->ram[adr] & 0x0f : (vdp->ram[adr] & 0xf0) >> 4;
}

typedef struct {
	int id;
	void(*draw)(VDP9938*);
	void(*line)(VDP9938*);
	void(*fram)(VDP9938*);
	void(*plot)(VDP9938*,int,int,unsigned char);
	unsigned char(*colr)(VDP9938*,int,int);
} vdpMode;

void vdpBreak(VDP9938* vdp) {
	// assert(0);
}

void vdpDrawDummy(VDP9938* vdp) {
	vidPutDot(vdp->ray, vdp->pal, 0);
}

vdpMode vdpTab[] = {
	{VID_MSX_SCR0, vdpMsxScr0, NULL, vdpFillSprites, NULL, NULL},
	{VID_MSX_SCR1, vdpMsxScr1, NULL, vdpFillSprites, NULL, NULL},
	{VID_MSX_SCR2, vdpMsxScr2, NULL, vdpFillSprites, NULL, NULL},
	{VID_MSX_SCR3, vdpMsxScr3, NULL, vdpFillSprites, NULL, NULL},
	{VID_MSX_SCR4, vdpMsxScr2, NULL, vdpFillSprites2, NULL, NULL},
	{VID_MSX_SCR5, vdpMsxScr5, NULL, vdpFillSprites2, scr5plot, scr5col},
	{VID_MSX_SCR6, vdpMsxScr6, NULL, vdpFillSprites2, scr6plot, scr6col},
	{VID_MSX_SCR7, vdpMsxScr7, NULL, vdpFillSprites2, scr7plot, scr7col},
	{VID_MSX_SCR8, vdpBreak, NULL, NULL, NULL, NULL},
	{VID_MSX_SCR9, vdpBreak, NULL, NULL, NULL, NULL},
	{VID_MSX_NONE, vdpDrawDummy, NULL, NULL, NULL, NULL}
};

void vdpSetMode(VDP9938* vdp, int mode) {
	int idx = 0;
	vdp->vmode = mode;
	//printf("v9938 mode = %i\n",vdp->vmode);
	while (1) {
		if ((vdpTab[idx].id == mode) || (vdpTab[idx].id == VID_MSX_NONE)) {
			vdp->draw = vdpTab[idx].draw;
			vdp->cbLine = vdpTab[idx].line;
			vdp->cbFram = vdpTab[idx].fram;
			vdp->plot = vdpTab[idx].plot;
			vdp->color = vdpTab[idx].colr;
			break;
		}
		idx++;
	}
}

xColor msxPalete[16] = {
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

void vdpReset(VDP9938* vdp) {
	for (int i = 0; i < 64; i++) vdp->reg[i] = 0;
	memset(vdp->ram, 0x00, 0x20000);
	vdpSetMode(vdp, VID_MSX_SCR0);
	for (int i = 0; i < 16; i++)
		vdp->pal[i] = msxPalete[i];
	vdp->BGColors = 0;
	vdp->BGMap = 0;
	vdp->BGTiles = 0;
	vdp->OBJAttr = 0;
	vdp->OBJTiles = 0;
	vdp->hAdj = 0;
	vdp->vAdj = 0;
	vdp->vadr = 0;
}

void vdpMemWr(VDP9938* vdp, unsigned char val) {
	vdp->ram[vdp->vadr & vdp->memMask] = val;
	vdp->vadr++;
}

unsigned char m2lev[8] = {0x00,0x33,0x5c,0x7f,0xa2,0xc1,0xe1,0xff};

void vdpPalWr(VDP9938* vdp, unsigned char val) {
	xColor col;
	if (vdp->high) {
		vdp->high = 0;
		col.b = m2lev[vdp->data & 7];
		col.r = m2lev[(vdp->data & 0x70) >> 4];
		col.g = m2lev[val & 7];
		vdp->pal[vdp->reg[0x10] & 0x0f] = col;
		// printf("col %i = (%i,%i,%i)\n",vdp->reg[0x10],col.r,col.g,col.b);
		vdp->reg[0x10]++;
	} else {
		vdp->data = val;
		vdp->high = 1;
	}
}

unsigned char vdpReadSR(VDP9938* vdp) {
	unsigned char idx = vdp->reg[0x0f] & 0x0f;
	unsigned char res = vdp->sr[idx];
	switch (idx) {
		case 0:
			vdp->sr[0] &= 0x5f;
			break;
		case 1:
			res &= 0xfe;
			if ((vdp->reg[0] & 0x10) && ((vdp->ray->y - vdp->lay->bord.y) == vdp->iLine))
				res |= 1;
			break;
		case 2:
			res &= 0x9f;
			if ((vdp->lay->full.x - vdp->ray->x) < vdp->lay->blank.x) res |= 0x20;
			if ((vdp->lay->full.y - vdp->ray->y) < vdp->lay->blank.y) res |= 0x40;
			break;
		default:
			break;

	}
	return res;
}

void vdpSend(VDP9938* vdp, unsigned char val) {
	if (vdp->plot)
		vdp->plot(vdp, vdp->pos.x, vdp->pos.y, val);
	vdp->pos.x += vdp->delta.x;
	if (++vdp->cnt.x >= vdp->size.x) {
		vdp->pos.x = vdp->dst.x;
		vdp->cnt.x = 0;
		vdp->pos.y += vdp->delta.y;
		if (++vdp->cnt.y >= vdp->size.y) {
			vdp->sr[2] &= 0xfe;
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

void vdpExec(VDP9938* vdp) {
	int spx,dpx;
	switch (vdp->com) {
		case 0:
			vdp->sr[2] &= 0xfe;
			break;
		case 5:
			if (vdp->plot)
				vdp->plot(vdp, vdp->dst.x, vdp->dst.y, vdp->reg[0x2c]);
			break;
		case 7:
			vdp->pos.x = (vdp->dst.x << 4) + 8;
			vdp->pos.y = (vdp->dst.y << 4) + 8;
			vdp->count = (vdp->size.x > vdp->size.y) ? vdp->size.x : vdp->size.y;
			// printf("size = %i %i\n",vdp->size.x,vdp->size.y);
			if (vdp->reg[0x2d] & 1) {
				vdp->delta.x = vdp->size.y ? (vdp->size.x << 4) / vdp->size.y : 0;
				vdp->delta.y = 0x10;
			} else {
				vdp->delta.x = 0x10;
				vdp->delta.y = vdp->size.y ? (vdp->size.x << 4) / vdp->size.y : 0;
			}
			if (vdp->reg[0x2d] & 4)
				vdp->delta.x = -vdp->delta.x;
			if (vdp->reg[0x2d] & 8)
				vdp->delta.y = -vdp->delta.y;
			while (vdp->count > 0) {
				if (vdp->plot)
					vdp->plot(vdp, vdp->pos.x >> 4, vdp->pos.y >> 4, vdp->reg[0x2c]);
				vdp->pos.x += vdp->delta.x;
				vdp->pos.y += vdp->delta.y;
				vdp->count--;
			}
			break;
		case 9:
			vdp->delta.x = (vdp->reg[0x2d] & 4) ? -1 : 1;
			vdp->delta.y = (vdp->reg[0x2d] & 8) ? -1 : 1;
			vdp->cnt.x = 0;
			vdp->cnt.y = 0;
			vdp->sr[2] |= 1;
			spx = vdp->src.x;
			dpx = vdp->dst.x;
			while (vdp->sr[2] & 1) {
				pap = vdp->color ? vdp->color(vdp, vdp->src.x, vdp->src.y) : 0;
				if (vdp->plot) vdp->plot(vdp, vdp->dst.x, vdp->dst.y, pap);
				vdp->dst.x += vdp->delta.x;
				vdp->src.x += vdp->delta.x;
				if (++vdp->cnt.x >= vdp->size.x) {
					vdp->cnt.x = 0;
					vdp->dst.x = dpx;
					vdp->src.x = spx;
					vdp->dst.y += vdp->delta.y;
					vdp->src.y += vdp->delta.y;
					if (++vdp->cnt.y >= vdp->size.y) {
						vdp->sr[2] &= 0xfe;
					}
				}
			}
			break;
		case 0xb:
			vdp->cnt.x = 0;
			vdp->cnt.y = 0;
			vdp->delta.x = (vdp->reg[0x2d] & 4) ? -1 : 1;
			vdp->delta.y = (vdp->reg[0x2d] & 8) ? -1 : 1;
			vdp->pos = vdp->dst;
			vdp->sr[2] |= 1;
			vdpSend(vdp, vdp->reg[0x2c]);
			break;
		case 0xc:
			vdp->cnt.x = 0;
			vdp->cnt.y = 0;
			vdp->delta.x = (vdp->reg[0x2d] & 4) ? -1 : 1;
			vdp->delta.y = (vdp->reg[0x2d] & 8) ? -1 : 1;
			vdp->pos = vdp->dst;
			vdp->sr[2] |= 1;
			while (vdp->sr[2] & 1) {
				vdpSend(vdp, vdp->reg[0x2c]);
			}
			break;
		case 0x0e:
			vdp->delta.x = (vdp->reg[0x2d] & 4) ? -1 : 1;
			vdp->delta.y = (vdp->reg[0x2d] & 8) ? -1 : 1;
			vdp->size.x = (vdp->delta.x < 0) ? vdp->dst.x : (512 - vdp->dst.x);		// TODO: get X-size of screen
			vdp->cnt.y = vdp->size.y;
			while (vdp->cnt.y > 0) {
				vdp->pos.x = vdp->dst.x;
				vdp->cnt.x = vdp->size.x;
				while (vdp->cnt.x > 0) {
					vdp->cnt.x--;
					pap = vdp->color ? vdp->color(vdp, vdp->pos.x, vdp->src.y) : 0;
					vdp->plot(vdp, vdp->pos.x, vdp->dst.y, pap);
					vdp->pos.x += vdp->delta.x;
				}
				vdp->dst.y += vdp->delta.y;
				vdp->src.y += vdp->delta.y;
				vdp->cnt.y--;
			}
			break;
		default:
			printf("vdp9938 command %.2X, arg %.2X\n",vdp->com, vdp->arg);
#ifdef ISDEBUG
			printf("src : %i : %i\n", vdp->src.x, vdp->src.y);
			printf("dst : %i : %i\n", vdp->dst.x, vdp->dst.y);
			printf("siz : %i : %i\n", vdp->size.x, vdp->size.y);
			printf("r2C : %.2X\n", vdp->reg[0x2c]);
			printf("r2D : %.2X\n", vdp->reg[0x2d]);
			assert(0);
#endif
			break;
	}
}

void vdpRegWr(VDP9938* vdp, unsigned char val) {
	int reg;
	int vmode;
	if (vdp->high) {
		if (val & 0x80) {		// b7.hi = 1 : VDP register setup
			reg = val & 0x3f;
			val = vdp->data;
			vdp->reg[reg] = val;
			switch (reg) {
			// mode registers
				case 0x00:
				case 0x01:
					vmode = ((vdp->reg[1] & 0x10) >> 4) | ((vdp->reg[1] & 8) >> 2) | ((vdp->reg[0] & 0x0e) << 1);
					// printf("v9938 mode = %.2X\n",vmode);
					switch (vmode) {
						case 1: vdpSetMode(vdp, VID_MSX_SCR0); break;	// text 40x24
						case 0: vdpSetMode(vdp, VID_MSX_SCR1); break;	// text 32x24
						case 4: vdpSetMode(vdp, VID_MSX_SCR2); break;	// 256x192
						case 2: vdpSetMode(vdp, VID_MSX_SCR3); break;	// multicolor 4x4
						case 0x08: vdpSetMode(vdp, VID_MSX_SCR4); break;	// scr2 8spr/line
						case 0x0c: vdpSetMode(vdp, VID_MSX_SCR5); break;	// 256x212,4bpp
						case 0x10: vdpSetMode(vdp, VID_MSX_SCR6); break;	// 512x212,2bpp
						case 0x14: vdpSetMode(vdp, VID_MSX_SCR7); break;	// 512x212,4bpp
						case 0x1c: vdpSetMode(vdp, VID_MSX_SCR8); break;	// 256x212,8bpp
						case 0x09: vdpSetMode(vdp, VID_MSX_SCR9); break;	// text 80x24
						default:
							printf("v9938 mode %.2X\n",vmode);
							assert(0);
							vdpSetMode(vdp, VID_MSX_NONE);
							break;
					}
					break;
				case 0x08: break;						// mode reg 2
				case 0x09: vdp->lines = (val & 0x80) ? 212 : 192; break;	// mode reg 3
			// address registers
				case 0x02: vdp->BGMap = (val & 0x7f) << 10; break;
				case 0x04: vdp->BGTiles = (val & 0x3f) << 11; break;
				case 0x06: vdp->OBJTiles = (val & 0x3f) << 11; break;

				case 0x03:
				case 0x0a: vdp->BGColors  = ((vdp->reg[0x0a] & 7) << 14) | (vdp->reg[0x03] << 6); break;

				case 0x05:
				case 0x0b: vdp->OBJAttr = ((vdp->reg[0x0b] & 3) << 15) | (vdp->reg[0x05] << 7); break;
			// color registers
				case 0x07: break;			// border color = BG in R7
				case 0x0c: break;			// inv/blink colors
				case 0x0d: break;			// blink period
				case 0x14: break;			// 14,15,16 : color burst registers
				case 0x15: break;
				case 0x16: break;
			// display registers
				case 0x12: vdp->hAdj = (val & 15) - (val & 8) ? 16 : 0;
					vdp->hAdj = ((val & 0xf0) >> 4) - (val & 0x80) ? 16 : 0;
					break;
				case 0x17: break;			// display Y offset
				case 0x13: vdp->iLine = val; break;
			// access registers
				case 0x0e: vdp->vadr = (vdp->vadr & 0x3fff) | ((val & 7) << 14); break;
				case 0x0f: break;			// status reg num (0..9)
				case 0x10: break;			// palette num
				case 0x11: vdp->regIdx = val & 0x3f; break;
			// command registers
				case 0x20:
				case 0x21: vdp->src.x = (vdp->reg[0x20] | (vdp->reg[0x21] << 8)) & 0x1ff; break;

				case 0x22:
				case 0x23: vdp->src.y = (vdp->reg[0x22] | (vdp->reg[0x23] << 8)) & 0x3ff; break;

				case 0x24:
				case 0x25: vdp->dst.x = (vdp->reg[0x24] | (vdp->reg[0x25] << 8)) & 0x1ff; break;

				case 0x26:
				case 0x27: vdp->dst.y = (vdp->reg[0x26] | (vdp->reg[0x27] << 8)) & 0x3ff; break;

				case 0x28:
				case 0x29: vdp->size.x = (vdp->reg[0x28] | (vdp->reg[0x29] << 8)) & 0x1ff; break;

				case 0x2a:
				case 0x2b: vdp->size.y = (vdp->reg[0x2a] | (vdp->reg[0x2b] << 8)) & 0x3ff; break;

				case 0x2c:			// color code | block data transfer cpu->vdp
					if (vdp->sr[2] & 1) {
						vdpSend(vdp, val);
					}
					break;
				case 0x2d: break;		// command argument
				case 0x2e:
					vdp->arg = val & 15;
					vdp->com = (val & 0xf0) >> 4;
					vdpExec(vdp);
					break;
				case 0x2f:			// WUT???
					break;
				default:
					printf("v9938 register : wr #%.2X,#%.2X\n",reg,val);
					// assert(0);
					break;
			}
		} else {			// b7.hi = 0 : VDP address setup
			vdp->vadr = (vdp->vadr & 0x1c000) | ((val & 0x3f) << 8) | vdp->data;
			vdp->wr = (val & 0x40) ? 1 : 0;
		}
		vdp->high = 0;
	} else {
		vdp->data = val;
		vdp->high = 1;
	}
}
