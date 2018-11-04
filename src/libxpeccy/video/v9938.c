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
static unsigned char m2lev[8] = {0x00,0x33,0x5c,0x7f,0xa2,0xc1,0xe1,0xff};
static unsigned char tbyte;

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

// colors mixer
// scol : new color
// dcol : old color
// type & 15 : mix mode
unsigned char vdp_mix_col(unsigned char scol, unsigned char dcol, unsigned char mode) {
	if ((scol == 0) && (mode & 8)) return dcol;	// transparent
	switch (mode & 7) {
		case 0: dcol = scol; break;
		case 1: dcol &= scol; break;
		case 2: dcol |= scol; break;
		case 3: dcol ^= scol; break;
		case 4: dcol = ~scol; break;
	}
	return dcol;
}

// convert addr bus to absolute memory addr (main ram / expansion ram)
int vdpGetAddr(Video* vid, int adr) {
	adr &= vid->memMask;
	if (adr & 0x10000) return adr;	// high 64K
	if (vid->reg[45] & 0x40)	// ext ram on
		adr |= 0x20000;
	return  adr;
}

// return relative dot addr for coordinates (x,y) and current video mode (bitmaps only)
int vdpDotAddr(Video* vid, int x, int y, int ext) {
	int adr;
	switch (vid->vmode) {
		case VDP_GRA4: adr = (x >> 1) | (y << 7); break;
		case VDP_GRA5: adr = (x >> 2) | (y << 7); break;
		case VDP_GRA6: adr = (x >> 1) | (y << 8); break;
		case VDP_GRA7: adr = x | (y << 8); break;
		default: adr = -1;
	}
	if ((adr > 0) && ext)
		adr = (adr & 0xffff) | MEM_128K;
	return  adr;
}

// start of scanline: prepare sprites line

int vdp_sprcoll_chk(Video* gpu, int xpos, int dx) {
	int res = 0;
	while (dx > 0) {
		if (gpu->line[xpos & 0x1ff]) {
			res = 1;
			dx = 0;
		}
		xpos++;
		dx--;
	}
	return res;
}

void vdp_draw_sprlin(Video* gpu, int xpos, int dx, unsigned char dt, unsigned char atr) {
	int bt;
	unsigned char col = atr & 0x0f;
	unsigned short data;
	if (gpu->reg[1] & 1) {
		data = 0;
		for (bt = 0; bt < 8; bt++) {
			data <<= 2;
			if (dt & 0x80)
				data |= 3;
			dt <<= 1;
		}
		bt = 16;
	} else {
		data = (dt << 8) & 0xff00;
		bt = 8;
	}
	if (atr & 0x40) {						// CC=1 lines
		if (vdp_sprcoll_chk(gpu, xpos, dx)) {			// collided with CC=0 lines only
			while (bt > 0) {
				if (data & 0x8000) {
					if (gpu->line[xpos & 0x1ff]) {		// x CC=0 : ORed color, no collision
						gpu->linb[xpos & 0x1ff] = gpu->line[xpos & 0x1ff] | col;
					} else if (gpu->linb[xpos & 0x1ff]) {	// x CC=1 : collision
						if (!(atr & 0x20))		// b5: don't cause conflict
							gpu->sr[0] |= 0x20;
					} else {				// free dot : put in here
						gpu->linb[xpos & 0x1ff] = col;
					}
				}
				xpos++;
				data <<= 1;
				bt--;
			}
		}
	} else {
		while (bt > 0) {
			if (data & 0x8000) {
				if (gpu->line[xpos & 0x1ff] || gpu->linb[xpos & 0x1ff]) {	// collision
					if (!(atr & 0x20))					// b5: don't cause conflict
						gpu->sr[0] |= 0x20;
				} else {							// free dot
					gpu->line[xpos & 0x1ff] = col;
				}
			}
			xpos++;
			data <<= 1;
			bt--;
		}
	}
}

// sprites mode 1 (g1, g2)
void vdp_line(Video* gpu) {
	memset(gpu->line, 0x00, 512);
	if (gpu->vbrd) return;		// sprites is visible on screen only
	int sadr = gpu->OBJAttr;
	int i;
	int xpos;
	int ypos;
	unsigned char tile;
	unsigned char flag;
	unsigned char dx = 8;			// full sprite size (dx x dx)
	int xadr;
	int scnt = 0;
	if (gpu->reg[1] & 1)	// zoom
		dx <<= 1;
	if (gpu->reg[1] & 2)	// 2x2
		dx <<= 1;
	for (i = 0; i < 32; i++) {
		ypos = gpu->ram[sadr];
		if (ypos == 0xd0) {						// stop position
			i = 32;
		} else {
			ypos = gpu->ray.ys - ((ypos + 1 - gpu->reg[0x17]) & 0xff);	// relative position
			if ((ypos >= 0)	&& (ypos < dx)) {			// sprite is crossing current line
				if (scnt < 5) {					// check 5th sprite in line
					xpos = gpu->ram[sadr + 1] & 0xff;
					tile = gpu->ram[sadr + 2];
					flag = gpu->ram[sadr + 3];
					if (flag & 0x80)			// early clock
						xpos -= 32;
					if (gpu->reg[1] & 2)
						tile &= ~3;
					if (gpu->reg[1] & 1)			// zoom doubles line -> relative position / 2
						ypos >>= 1;
					xadr = gpu->OBJTiles | (tile << 3) | (ypos & 7);	// addr of byte in 1st tile
					if (ypos > 7)
						xadr += 8;
					tile = gpu->ram[xadr];
					vdp_draw_sprlin(gpu, xpos, dx, tile, flag & 0x8f);
					if (gpu->reg[1] & 2) {
						xpos += (gpu->reg[1] & 1) ? 16 : 8;
						tile = gpu->ram[xadr + 16];
						vdp_draw_sprlin(gpu, xpos, dx, tile, flag & 0x8f);
					}
					scnt++;
				} else {
					gpu->sr[0] |= 0x40;
					gpu->sr[0] &= ~0x1f;
					gpu->sr[0] |= (i & 0x1f);
					i = 32;
				}
			}
		}
		sadr += 4;
	}
}

// sprites mode 2 (G3,4,5,6,7)
void vdp_linex(Video* gpu) {
	memset(gpu->line, 0x00, 512);
	memset(gpu->linb, 0x00, 512);
	if (gpu->reg[8] & 2) return;		// sprites disabled
	if (gpu->vbrd) return;			// on screen only
	int aadr = (gpu->OBJAttr & ~0x3ff);	// attributes
	int tadr = aadr + 0x200;		// obj dsc
	int ypos;
	int xpos;
	int xadr;
	unsigned char tile;
	unsigned char atr;
	int scnt = 0;
	unsigned char dx = 8;			// full sprite size (dx x dx)
	if (gpu->reg[1] & 1)	// zoom
		dx <<= 1;
	if (gpu->reg[1] & 2)	// 2x2
		dx <<= 1;
	for (int i = 0; i < 32; i++) {
		ypos = gpu->ram[tadr];
		if (ypos == 0xd8) {
			i = 32;
		} else {
			ypos = gpu->ray.ys - ((ypos + 1 - gpu->reg[0x17]) & 0xff);
			xpos = gpu->ram[tadr + 1] & 0xff;
			tile = gpu->ram[tadr + 2] & 0xff;
			if (gpu->reg[1] & 2)
				tile &= ~3;
			if ((ypos >= 0)	&& (ypos < dx)) {
				if (scnt < 8) {
					if (gpu->reg[1] & 1)					// zoom doubles line -> relative position / 2
						ypos >>= 1;
					atr = gpu->ram[(aadr + ypos) & gpu->memMask];		// sprite line attribute;
					if (atr & 0x80)						// EC flag is different for each line
						xpos -= 0x20;
					xadr = gpu->OBJTiles | (tile << 3) | (ypos & 7);		// sprite tile line addr
					if (ypos > 7)
						xadr += 8;
					tile = gpu->ram[xadr & gpu->memMask];			// tile line data
					vdp_draw_sprlin(gpu, xpos, dx, tile, atr);
					if (gpu->reg[1] & 2) {
						xpos += (gpu->reg[1] & 1) ? 16 : 8;
						tile = gpu->ram[xadr + 16];
						// atr = gpu->mem[(aadr + ypos + 0x10) & gpu->memMask];
						vdp_draw_sprlin(gpu, xpos, dx, tile, atr);
					}
					scnt++;
				} else {
					gpu->sr[0] |= 0x40;
					gpu->sr[0] &= ~0x1f;
					gpu->sr[0] |= (i & 0x1f);
					i = 32;
				}
			}
		}
		tadr += 0x04;
		aadr += 0x10;
	}
	for (int i = 0; i < 512; i++) {
		if (gpu->linb[i])
			gpu->line[i] = gpu->linb[i];
	}
}

// v9918 TEXT1 (40 x 24 text)

void vdpT1ini(Video* vid) {
	vid->scrn.x = 240;	// 40 * 6
	vidUpdateLayout(vid);
}

void vdpText1(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		col = vid->reg[7] & 0x0f;
	} else {
		yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
		if ((vid->ray.xs % 6) == 0) {
			adr = vid->ram[((yscr & 0xf8) * 5) + (vid->ray.xs / 6)];
			scrbyte = vid->ram[0x800 | (adr << 3) | (yscr & 7)];
			ink = (vid->reg[7] & 0xf0) >> 4;
			pap = vid->reg[7] & 0x0f;
		}
		col = (scrbyte & 0x80) ? ink : pap;
		scrbyte <<= 1;
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9918 G1 (32 x 24 text)

void vdpG1ini(Video* vid) {
	vid->scrn.x = 256;	// 40 * 6
	vidUpdateLayout(vid);
}

void vdpGra1(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		col = vid->reg[7] & 0x0f;
	} else {
		yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
		if (!(xscr & 7)) {
			adr = vid->ram[vid->BGMap + (vid->ray.xs >> 3) + ((yscr & 0xf8) << 2)];
			scrbyte = vid->ram[vid->BGTiles + (adr << 3) + (yscr & 7)];
			atrbyte = vid->ram[vid->BGColors + (adr >> 3)];
			ink = (atrbyte & 0xf0) >> 4;
			pap = atrbyte & 0x0f;
		}
		if (vid->line[vid->ray.xs & 0x1ff])
			col = vid->line[vid->ray.xs & 0x1ff];
		scrbyte <<= 1;

	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9918 G2 (256 x 192)

void vdpGra2(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		col = vid->reg[7] & 0x0f;
	} else {
		yscr = (vid->ray.xs + vid->reg[0x17]) & 0xff;
		if ((vid->ray.xs & 7) == 0) {
			adr = vid->ram[vid->BGMap | (vid->ray.xs >> 3) | ((yscr & 0xf8) << 2)] | ((yscr & 0xc0) << 2);	// tile nr
			scrbyte = vid->ram[(vid->BGTiles & ~0x1fff) | (adr << 3) | (yscr & 7)];
			atrbyte = vid->ram[(vid->BGColors & ~0x1fff) | (adr << 3) | (yscr & 7)];
			ink = (atrbyte >> 4) & 0x0f;
			pap = atrbyte & 0x0f;
		}
		if (vid->line[vid->ray.xs & 0x1ff])
			col = vid->line[vid->ray.xs & 0x1ff];
		scrbyte <<= 1;
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9918 MC (multicolor)

void vdpMultcol(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		col = vid->reg[7] & 0x0f;
	} else {
		yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
		adr = vid->ram[vid->BGMap | (vid->ray.xs >> 3) | ((yscr & 0xf8) << 2)];		// color index
		adr = vid->BGTiles | (adr << 3) | ((yscr & 0x18) >> 2) | ((yscr & 4) >> 2);	// color adr
		col = vid->ram[adr];								// color for 2 dots
		if (!(xscr & 4)) {
			col >>= 4;
		}
		col &= 0x0f;
		if (vid->line[vid->ray.xs & 0x1ff])
			col = vid->line[vid->ray.xs & 0x1ff];
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// v9938 G4 (256x212 4bpp)

void vdpG4ini(Video* vid) {
	vid->scrn.x = 256;	// 40 * 6
	vidUpdateLayout(vid);
}

void vdpGra4(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		col = vid->reg[7] & 0x0f;
	} else {
		if (vid->ray.xs & 1) {
			col = ink & 0x0f;
		} else {
			yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
			adr = (vid->BGMap & ~0x7fff) | (vid->ray.xs >> 1) | (yscr << 7);
			ink = vid->ram[adr & vid->memMask];		// color byte
			col = (ink >> 4) & 15;
		}
		if (vid->line[vid->ray.xs & 0x1ff])
			col = vid->line[vid->ray.xs & 0x1ff];
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

void vdpG4pset(Video* vid, int x, int y, unsigned char col) {
	adr = ((x >> 1) | (y << 7)) & vid->memMask;
	tbyte = vid->ram[adr];
	if (x & 1) {
		pap = tbyte & 15;
		col = vdp_mix_col(col, pap, vid->reg[0x2e] & 15);
		tbyte = (tbyte & 0xf0) | (col & 0x0f);
	} else {
		pap = (tbyte >> 4) & 15;
		col = vdp_mix_col(col, pap, vid->reg[0x2e] & 15);
		tbyte = (tbyte & 0x0f) | ((col << 4) & 0xf0);
	}
	vid->ram[adr] = tbyte;
}

unsigned char vdpG4col(Video* vid, int x, int y) {
	adr = ((x >> 1) | (y << 7)) & vid->memMask;
	tbyte = vid->ram[adr];
	return (x & 1) ? tbyte & 0x0f : (tbyte >> 4) & 0x0f;
}

// v9938 G5 (512x212 2bpp)

void vdpGra5(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		vidPutDot(&vid->ray, vid->pal, vid->reg[7] & 3);
	} else {
		yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
		if (vid->ray.xs & 1) {
			adr = (vid->BGMap & ~0x7fff) | (vid->ray.xs >> 1) | (yscr << 7);
			col = vid->ram[adr];
		}
		if (vid->line[vid->ray.xs & 0x1ff]) {
			vidPutDot(&vid->ray, vid->pal, vid->line[vid->ray.xs & 0x1ff]);
		} else {
			vidSingleDot(&vid->ray, vid->pal, (col & 0xc0) >> 6);
			vidSingleDot(&vid->ray, vid->pal, (col & 0x30) >> 4);
		}
		col <<= 4;
	}
}

void vdpG5pset(Video* vid, int x, int y, unsigned char col) {
	adr = ((x >> 2) | (y << 7)) & vid->memMask;
	tbyte = vid->ram[adr];
	pap = (tbyte  >> ((~x & 3) << 1)) & 3;
	col = vdp_mix_col(col, pap, vid->reg[0x2e] & 3);
	switch (x & 3) {
		case 0: tbyte = (tbyte & 0x3f) | ((col << 6) & 0xc0); break;
		case 1: tbyte = (tbyte & 0xcf) | ((col << 4) & 0x30); break;
		case 2: tbyte = (tbyte & 0xf3) | ((col << 2) & 0x0c); break;
		case 3: tbyte = (tbyte & 0xfc) | (col & 3); break;
	}
	vid->ram[adr] = tbyte;
}

unsigned char vdpG5col(Video* vid, int x, int y) {
	adr = (x >> 2) | (y << 7);
	tbyte = vid->ram[adr & vid->memMask];
	return (tbyte  >> ((~x & 3) << 1)) & 3;
}

// v9938 G6 (512x212 4bpp)

void vdpGra6(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		vidPutDot(&vid->ray, vid->pal, vid->reg[7]);
	} else {
		yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
		adr = (vid->BGMap & ~0xffff) | vid->ray.xs | (yscr << 8);
		col = vid->ram[adr];
		if (vid->line[vid->ray.xs & 0x1ff]) {
			vidPutDot(&vid->ray, vid->pal, vid->line[vid->ray.xs & 0x1ff]);
		} else {
			vidSingleDot(&vid->ray, vid->pal, (col & 0xf0) >> 4);
			vidSingleDot(&vid->ray, vid->pal, col & 0x0f);
		}
	}
}

void vdpG6pset(Video* vid, int x, int y, unsigned char col) {
	adr = (x | (y << 8)) & vid->memMask;
	tbyte = vid->ram[adr];
	pap = (tbyte >> ((~x & 1) << 2)) & 0x0f;
	col = vdp_mix_col(col, pap, vid->reg[0x2e] & 15);
	if (x & 1) {
		tbyte = (tbyte & 0xf0) | (col & 0x0f);
	} else {
		tbyte = (tbyte & 0x0f) | ((col << 4) & 0x0f);
	}
	vid->ram[adr] = tbyte;
}

unsigned char vdpG6col(Video* vid, int x, int y) {
	adr = (x | (y << 8)) & vid->memMask;
	tbyte = vid->ram[adr];
	return (tbyte >> ((~x & 1) << 2)) & 0x0f;
}

// v9938 G6 (256x212 8bpp)
// note:sprites color is different

void vdpGra7(Video* vid) {
	if (vid->vbrd || vid->hbrd || !(vid->reg[1] & 0x40)) {
		col = vid->reg[7];
	} else {
		yscr = (vid->ray.ys + vid->reg[0x17]) & 0xff;
		adr = ((vid->reg[2] & 0x20) ? 0x10000 : 0) | vid->ray.xs | (yscr << 8);
		if (vid->line[vid->ray.xs & 0x1ff]) {
			col = vid->line[vid->ray.xs & 0x1ff];
			col = ((col & 4) ? 0xe0 : 0) | ((col & 2) ? 0x1c : 0) | ((col & 1) ? 3 : 0);
		} else {
			col = vid->ram[adr & vid->memMask];
		}
		vid->pal[0xff].r = (col << 3) & 0xe0;
		vid->pal[0xff].g = col & 0xe0;
		vid->pal[0xff].b = (col << 6) & 0xe0;
		col = 0xff;
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

void vdpG7pset(Video* vid, int x, int y, unsigned char col) {
	adr = x | (y << 8);
	vid->ram[adr & vid->memMask] = col;
}

unsigned char vdpG7col(Video* vid, int x, int y) {
	adr = x | (y << 8);
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

typedef struct {
	int id;
	int wid;	// screen width in dots (256|512)
	int dpb;	// dots per byte (bitmaps: 1|2|4)
	int swd;	// screen width in bytes
	void(*pset)(Video*,int,int,unsigned char);
	unsigned char(*col)(Video*,int,int);
} vdpMode;


static vdpMode vdpTab[] = {
	{VDP_TEXT1, 256, 1, 40, NULL, NULL},
	{VDP_TEXT2, 256, 1, 80, NULL, NULL},
	{VDP_MCOL, 256, 1, 32, NULL, NULL},
	{VDP_GRA1, 256, 1, 32, NULL, NULL},
	{VDP_GRA2, 256, 1, 32, NULL, NULL},
	{VDP_GRA3, 256, 1, 32, NULL, NULL},
	{VDP_GRA4, 256, 2, 128, vdpG4pset, vdpG4col},
	{VDP_GRA5, 512, 4, 128, vdpG5pset, vdpG5col},
	{VDP_GRA6, 512, 2, 256, vdpG6pset, vdpG6col},
	{VDP_GRA7, 256, 1, 256, vdpG7pset, vdpG7col},
	{VID_UNKNOWN, 256, 1, 32, NULL, NULL}
};

void vdpSetMode(Video* vid, int mode) {
	int idx = 0;
	//printf("v9938 mode = %i\n",vdp->vmode);
	while((vdpTab[idx].id != mode) && (vdpTab[idx].id != VID_UNKNOWN)) {
		idx++;
	}
	vidSetMode(vid, vdpTab[idx].id);
	vid->pset = vdpTab[idx].pset;
	vid->col = vdpTab[idx].col;
	vid->scrsize.x = vdpTab[idx].wid;
	vid->dpb = vdpTab[idx].dpb;
}

void vdpReset(Video* vid) {
	memset(vid->reg, 0x00, 64);
	memset(vid->sr, 0x00, 16);
	memset(vid->ram, 0x00, MEM_128K);
	vdpSetMode(vid, VDP_TEXT1);
	for (int i = 0; i < 16; i++)
		vid->pal[i] = msxPalete[i];
	vid->memMask = MEM_128K - 1;
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

void vdpSend(Video* vid, unsigned char val) {
	if (vid->pset)
		vid->pset(vid, vid->dst.x, vid->dst.y, val);
	vid->dst.x += vid->step.x;
	vid->sr[2] |= 0x80;
	if (--vid->rct.x < 1) {
		vid->rct.x = vid->rctx;
		vid->dst.x = vid->dstx;
		vid->dst.y += vid->step.y;
		if (--vid->rct.y < 1) {
			vid->sr[2] &= ~0x81;
		}
	}
}

unsigned char vdpGet(Video* vid) {
	unsigned char col = vid->col ? vid->col(vid, vid->src.x, vid->src.y) : 0x00;
	vid->src.x += vid->step.x;
	vid->sr[2] |= 0x80;
	if (--vid->rct.x < 1) {
		vid->rct.x = vid->rctx;
		vid->src.x = vid->srcx;
		vid->src.y += vid->step.y;
		if (--vid->rct.y < 1) {
			vid->sr[2] &= ~0x81;
		}
	}
	return col;
}

void vdpCopy(Video* vid) {
	if (vid->col && vid->pset) {
		col = vid->col(vid, vid->src.x, vid->src.y);
		vid->pset(vid, vid->dst.x, vid->dst.y, col);
	}
	vid->sr[2] |= 0x80;
	vid->src.x += vid->step.x;
	vid->dst.x += vid->step.x;
	if (--vid->rct.x < 1) {
		vid->src.x = vid->srcx;
		vid->dst.x = vid->dstx;
		vid->rct.x = vid->rctx;
		vid->src.y += vid->step.y;
		vid->dst.y += vid->step.y;
		if (--vid->rct.y < 1) {
			vid->sr[2] &= ~0x81;
		}
	}
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
			if (vid->hblank) res |= 0x20;
			if (vid->vbrd) res |= 0x40;
			// if (vid->busy) res |= 0x01;
			break;
		case 3: res = xscr & 0xff; break;
		case 4: res = (xscr >> 8) & 1; break;
		case 5: res = yscr & 0xff; break;
		case 6: res = (yscr >> 8) & 3; break;
		case 7: if (vid->sr[2] & 0x80) {
				vid->sr[7] = vdpGet(vid);
			}
			break;
		default:
			break;
	}
//	printf("SR %i = %.2X\n",idx,res);
	return res;
}


typedef struct {
	int adr;
	int bpl;	// bytes per line
	int dpb;	// dots per byte
	int dx;
} xVDPArgs;

xVDPArgs vdp_get_hcom(Video* vid, vCoord crd, vCoord rect) {
	xVDPArgs res;
	res.adr = 0;
	res.bpl = 0;
	res.dpb = 1;
	res.dx = 0;
	crd.x &= 0x1ff;
	crd.y &= 0x3ff;
	switch (vid->vmode) {
		case VDP_GRA4:
			res.adr = (crd.x >> 1) | (crd.y << 7);
			res.bpl = 128;
			res.dpb = 2;
			res.dx = rect.x >> 1;
			break;
		case VDP_GRA5:
			res.adr = (crd.x >> 2) | (crd.y << 7);
			res.bpl = 128;
			res.dpb = 4;
			res.dx = rect.x >> 2;
			break;
		case VDP_GRA6:
			res.adr = (crd.x >> 1) | (crd.y << 8);
			res.bpl = 256;
			res.dpb = 2;
			res.dx = rect.x >> 1;
			break;
		case VDP_GRA7:
			res.adr = (crd.x & 0xff) | (crd.y << 8);
			res.bpl = 256;
			res.dpb = 1;
			res.dx = rect.x;
			break;
	}
	vid->step.y = (vid->reg[0x2d] & 8) ? -res.bpl : res.bpl;
	return res;
}

// commands
// + 0 : STOP
// ~ 4 : get color
// + 5 : PSET	(dstX, dstY)	col = r2C;
// ~ 6 : search color
// + 7 : LINE	start:(dstX,dstY);	MAJ = r2D & 1;	dx = MAJ ? dLong : dShort;	dy = MAJ ? dShort : dLong;	col = r2C;
// ~ 8 : fill rect (dots)
// * 9 : copy rect
// ~ A : get rect (dots) vdp->cpu
// + B : put rect (dots)
// ~ C : fill rect (bytes)
// * D : copy rect (bytes)
// ~ E : move right side of lines up
//   F : put rect (bytes)

// TODO: block commands executed line by line

void vdp_com_info(Video* vid) {
	printf("vdp9938 command %.2X, arg %.2X\n",vid->com, vid->arg);
	printf("src:%i %i\ndst:%i %i\nrct:%i %i\n", vid->src.x, vid->src.y, vid->dst.x, vid->dst.y, vid->rct.x, vid->rct.y);
}

// static unsigned char cbuf[512];

void vdpExec(Video* vid) {
//	int spx,dpx;
	if ((vid->sr[2] & 1) && vid->com) return;	// busy & not stop command

	xVDPArgs darg;
	xVDPArgs sarg;
	unsigned char xcol;

	vid->src.x = (vid->reg[0x20] | (vid->reg[0x21] << 8)) & 0x1ff;
	vid->src.y = (vid->reg[0x22] | (vid->reg[0x23] << 8)) & 0x3ff;
	vid->srcx = vid->src.x;
	vid->dst.x = (vid->reg[0x24] | (vid->reg[0x25] << 8)) & 0x1ff;
	vid->dst.y = (vid->reg[0x26] | (vid->reg[0x27] << 8)) & 0x3ff;
	vid->dstx = vid->dst.x;
	vid->rct.x = (vid->reg[0x28] | (vid->reg[0x29] << 8)) & 0x3ff;
	vid->rct.y = (vid->reg[0x2a] | (vid->reg[0x2b] << 8)) & 0x3ff;
	vid->rctx = vid->rct.x;
	vid->step.x = (vid->reg[0x2d] & 4) ? -1 : 1;
	vid->step.y = (vid->reg[0x2d] & 8) ? -1 : 1;

//	if ((vid->dst.y > 191) && (vid->dst.y < 256)) vdp_com_info(vid);

	vid->sr[2] |= 1;
	switch (vid->com) {
		case 0x00:
			vid->sr[2] &= ~0x81;			// stop
			break;
		case 0x04:					// color
			vid->sr[7] = vdpGet(vid);
			vid->sr[2] &= ~0x81;
			break;
		case 0x05:					// pset
			vdpSend(vid, vid->reg[0x2c]);
			vid->sr[2] &= ~0x81;
			break;
		case 0x06:					// search
			vid->rct.x = (vid->reg[0x2d] & 4) ? vid->src.x : vid->scrsize.x - vid->src.x;
			vid->rct.y = 1;
			vid->sr[2] &= ~0x40;
			do {
				xcol = vdpGet(vid);
				if ((vid->reg[0x2d] & 2) && (xcol == vid->reg[0x2c])) {		// search equal color
					vid->sr[2] |= 0x40;
					vid->sr[2] &= ~0x81;
				} else if (!(vid->reg[0x2d] & 2) && (xcol != vid->reg[0x2c])) {	// search not equal color
					vid->sr[2] |= 0x40;
					vid->sr[2] &= ~0x81;
				}
			} while (vid->sr[2] & 1);
			vid->sr[8] = vid->src.x & 0xff;
			vid->sr[9] = ((vid->src.x >> 8) & 1) | 0xfe;
			break;
		case 0x07:					// line
			vid->dst.x = (vid->dst.x << 4) + 8;
			vid->dst.y = (vid->dst.y << 4) + 8;
			vid->count = (vid->rct.x > vid->rct.y) ? vid->rct.x : vid->rct.y;
			vid->busy = 96 * vid->rct.x * vid->rct.y;
			// printf("size = %i %i\n",vdp->size.x,vdp->size.y);
			if (vid->reg[0x2d] & 1) {
				vid->step.x = vid->rct.y ? (vid->rct.x << 4) / vid->rct.y : 0;
				vid->step.y = 0x10;
			} else {
				vid->step.x = 0x10;
				vid->step.y = vid->rct.y ? (vid->rct.x << 4) / vid->rct.y : 0;
			}
			if (vid->reg[0x2d] & 4)
				vid->step.x = -vid->step.x;
			if (vid->reg[0x2d] & 8)
				vid->step.y = -vid->step.y;
			do {
				if (vid->pset)
					vid->pset(vid, vid->dst.x >> 4, vid->dst.y >> 4, vid->reg[0x2c]);
				vid->dst.x += vid->step.x;
				vid->dst.y += vid->step.y;
				vid->count--;
			} while (vid->count > 0);
			vid->sr[2] &= ~1;
			break;
		case 0x08:					// fill rect (dots)
			do {
				vdpSend(vid, vid->reg[0x2c]);
			} while (vid->sr[2] & 1);
			vid->sr[2] &= ~0x81;
			break;
		case 0x09:					// copy rect (dots) src->dst
			vid->busy = 96 * vid->rct.x * vid->rct.y;
			do {
				vdpCopy(vid);
			} while (vid->sr[2] & 0x80);
			vid->sr[2] &= ~1;
			break;
		case 0x0a:				// get rect (dots)
			vid->sr[7] = vdpGet(vid);
			vid->sr[2] |= 0x80;
			break;
		case 0x0b:				// copy rect (dots) cpu->dst by reg 2C
			vid->busy = 112 * vid->rct.x * vid->rct.y;
			vdpSend(vid, vid->reg[0x2c]);
			break;
		case 0x0c:				// fill rect (bytes)
			vid->busy = 64 * vid->rct.x * vid->rct.y;
			darg = vdp_get_hcom(vid, vid->dst, vid->rct);
			if (darg.bpl) {
				if (vid->reg[0x2d] & 4)
					darg.adr = (darg.adr - darg.dx + 1);	// move to left edge
				do {
					memset(vid->ram + (darg.adr & vid->memMask), vid->reg[0x2c], darg.dx);
					darg.adr += vid->step.y;
					vid->rct.y--;
				} while (vid->rct.y > 0);
			}
			vid->sr[2] &= ~1;
			break;
		case 0x0e:						// copy right (left) rect
			vid->src.x = vid->dst.x;
			vid->rct.x = (vid->step.x < 0) ? vid->dst.x : (vid->scrsize.x - vid->dst.x);
		case 0x0d:						// copy rect (bytes) src->dst
			vid->busy = 96 * vid->rct.x * vid->rct.y;
			sarg = vdp_get_hcom(vid, vid->src, vid->rct);
			darg = vdp_get_hcom(vid, vid->dst, vid->rct);
			if (sarg.bpl) {
				if (vid->reg[0x2d] & 4) {
					sarg.adr = (sarg.adr - sarg.dx + 1);
					darg.adr = (darg.adr - darg.dx + 1);
				}
				vid->src.y += vid->step.y * vid->rct.y;
				vid->dst.y += vid->step.y * vid->rct.y;
				do {
					memcpy(vid->ram + (darg.adr & vid->memMask), vid->ram + (sarg.adr & vid->memMask), sarg.dx);
					sarg.adr += vid->step.y;
					darg.adr += vid->step.y;
				} while (--vid->rct.y > 0);
			}
			vid->sr[2] &= ~1;
			break;
		default:
			printf("vdp9938 command %.2X, arg %.2X\n",vid->com, vid->arg);
			vid->sr[2] &= ~0x81;
			assert(0);
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
		case 0x09: vid->scrn.y = (val & 0x80) ? 212 : 192;		// mode reg 3
			vidUpdateLayout(vid);
			break;
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
		case 0x11: break;
			// command registers
		case 0x20:
		case 0x21:
			break;
		case 0x22:
		case 0x23:
			break;
		case 0x24:
		case 0x25:
			break;
		case 0x26:
		case 0x27:
			break;
		case 0x28:
		case 0x29:
			break;
		case 0x2a:
		case 0x2b:
			break;
		case 0x2c:			// color code | block data transfer cpu->vdp
			if (vid->sr[2] & 0x80) {
				vdpSend(vid, val);
			}
			break;
		case 0x2d: break;		// command argument
		case 0x2e:
			vid->arg = val & 0x0f;
			vid->com = (val >> 4) & 0x0f;
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

void vdpHBlk(Video* vid) {
	yscr = vid->ray.x - vid->bord.y;
	if (yscr == vid->intp.y) {		// HINT
		if (vid->reg[0] & VDP_IE1) {
			vid->sr[1] |= 1;
			vid->inth = 64;
		}
	}
	if (vid->ray.y == vid->vend.y) {
		vdpVBlk(vid);
	}
}

void vdpVBlk(Video* vid) {
	if (vid->reg[1] & VDP_IE0) {
		vid->sr[0] |= 0x80;
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
			res = vid->ram[vid->vadr & vid->memMask];
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
			vid->ram[vid->vadr & vid->memMask] = val;
			vid->vadr++;
			break;
		case 1:
//			printf("port1.%i:%.2X\n",vdp->high,val);
			if (vid->high) {
				if (val & 0x80) {
					vdpRegWr(vid, val & 0x3f, vid->dat);
				} else {
					vid->vadr = (vid->vadr & ~0x3fff) | ((val << 8) & 0x3f00) | (vid->dat & 0xff);
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
