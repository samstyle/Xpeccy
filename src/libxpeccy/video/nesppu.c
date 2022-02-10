#include "video.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// full line 341 dots
// nes layout: 256x240 = 32x30 of 8x8 tiles
// hblank: 85 dots ?
// vblank: 20 lines
// 1 cpu tick = 3 dots

// palette taken here:
// https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#NES

xColor nesPal[64] = {
	{0x7c,0x7c,0x7c},{0x00,0x00,0x7c},{0x00,0x00,0xbc},{0x44,0x28,0xbc},
	{0x94,0x00,0x84},{0xa8,0x00,0x20},{0xa8,0x10,0x00},{0x88,0x14,0x00},
	{0x50,0x30,0x00},{0x00,0x78,0x00},{0x00,0x68,0x00},{0x00,0x58,0x00},
	{0x00,0x40,0x58},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},

	{0xbc,0xbc,0xbc},{0x00,0x78,0xf8},{0x00,0x58,0xf8},{0x68,0x44,0xfc},
	{0xd8,0x00,0xcc},{0xe4,0x00,0x58},{0xf8,0x38,0x00},{0xe4,0x5c,0x10},
	{0xac,0x7c,0x00},{0x00,0xb8,0x00},{0x00,0xa8,0x00},{0x00,0xa8,0x44},
	{0x00,0x88,0x88},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},

	{0xf8,0xf8,0xf8},{0x3c,0xbc,0xfc},{0x68,0x88,0xfc},{0x98,0x78,0xf8},
	{0xf8,0x78,0xf8},{0xf8,0x58,0x98},{0xf8,0x78,0x58},{0xfc,0xa0,0x44},
	{0xf8,0xb8,0x00},{0xb8,0xf8,0x18},{0x58,0xd8,0x54},{0x58,0xf8,0x98},
	{0x00,0xe8,0xd8},{0x78,0x78,0x78},{0x00,0x00,0x00},{0x00,0x00,0x00},

	{0xfc,0xfc,0xfc},{0xa4,0xe4,0xfc},{0xb8,0xb8,0xf8},{0xd8,0xb8,0xf8},
	{0xf8,0xb8,0xf8},{0xf8,0xa4,0xc0},{0xf0,0xd0,0xb0},{0xfc,0xe0,0xa8},
	{0xf8,0xd8,0x78},{0xd8,0xf8,0x78},{0xb8,0xf8,0xb8},{0xb8,0xf8,0xd8},
	{0x00,0xfc,0xfc},{0xd8,0xd8,0xd8},{0x00,0x00,0x00},{0x00,0x00,0x00},
};

static unsigned char nesInitIdx[32] = {
	0x09,0x01,0x00,0x01,0x00,0x02,0x02,0x0d,
	0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2c,
	0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,
	0x08,0x3a,0x00,0x02,0x00,0x20,0x2c,0x08
};

void ppuFillPal(Video* vid, int tint) {
	xColor xcol;
	for (int i = 0; i < 64; i++) {
		xcol = nesPal[i];
		if (tint & 0x80) xcol.b >>= 1;
		if (tint & 0x40) {
			if (vid->ntsc) xcol.g >>= 1; else xcol.r >>= 1;
		}
		if (tint & 0x20) {
			if (vid->ntsc) xcol.r >>= 1; else xcol.g >>= 1;
		}
		vid_set_col(vid, i, xcol);
	}
}

void ppuReset(Video* vid) {
	vid->inten = 0;
	vid->latch = 0;
	vid->vastep = 0;
	vid->bgen = 0;
	vid->spren = 0;
	vid->bgadr = 0;
	vid->spadr = 0;
	vid->vadr = 0;
	vid->tadr = 0;
	vid->finex = 0;
	memset(vid->oam, 0xff, 0x100);
	memset(vid->ram, 0x00, 0x4000);
	memcpy(vid->ram + 0x3f00, nesInitIdx, 32);
	ppuFillPal(vid, 0);
}

// extern unsigned char col,ink,pap;

unsigned short ppuYinc(unsigned short v) {
	int y;
	if ((v & 0x7000) != 0x7000) {
		v += 0x1000;
	} else {
		v &= ~0x7000;
		y = (v & 0x3e0) >> 5;
		if (y == 29) {		// lines 0..29 is visible
			y = 0;
			v ^= 0x0800;
		} else if (y == 31) {	// pseudo lines 30,31 @ attributes area
			y = 0;
		} else {
			y++;
		}
		v = (v & ~0x03e0) | ((y << 5) & 0x3e0);
	}
	return v;
}

unsigned short ppuXcoarse(unsigned short v) {
	if ((v & 0x1f) == 0x1f) {
		v &= ~0x1f;
		v ^= 0x400;
	} else {
		v++;
	}
	return v;
}

// from nesdev wiki:
// tile address      = 0x2000 | (v & 0x0FFF)
// attribute address = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07)

void ppuRenderTile(Video* vid, unsigned char* buf, int offset, unsigned short adr, unsigned short tadr) {
	int cnt;
	unsigned char col;
	unsigned char tile = vid->mrd(0x2000 | (adr & 0x0fff), vid->data) & 0xff;						// tile num
	unsigned char atr = vid->mrd(0x23c0 | (adr & 0x0c00) | ((adr >> 4) & 0x38) | ((adr >> 2) & 7), vid->data) & 0xff;	// attribute
	unsigned short bgadr = tadr | ((tile << 4) & 0x0ff0) | ((adr >> 12) & 7);					// tile adr
	unsigned short data = vid->mrd(bgadr, vid->data) & 0xff;								// tile data (2 bytes)
	data |= ((vid->mrd(bgadr + 8, vid->data) << 8) & 0xff00);
	if (adr & 0x0040) atr >>= 4;							// bit 2,3 = attribute of current tile
	if (~adr & 0x0002) atr <<= 2;
	for (cnt = 0; cnt < 8; cnt++) {							// put 8 color indexes in buffer
		col = (data & 0x80) ? 1 : 0;
		if (data & 0x8000) col |= 2;
		col |= (atr & 0x0c);
		buf[(cnt + offset) & 0xff] = col;
		data <<= 1;
	}
}

void ppuRenderBGLine(Video* vid, unsigned char* buf, unsigned short adr, int xoff, unsigned short ppubga) {
	int cnt;
	xoff &= 7;
	for(cnt = 0; cnt < 0x100; cnt += 8) {
		ppuRenderTile(vid, buf, cnt - xoff, adr, ppubga);
		adr = ppuXcoarse(adr);
	}
}

// return sprites @ line
int ppuRenderSpriteLine(Video* vid, int line, unsigned char* sbuf, unsigned char* pbuf, unsigned short ppuspa, int maxspr) {
	unsigned char tmpbuf[0x100];
	if (!sbuf) sbuf = tmpbuf;
	if (!pbuf) pbuf = tmpbuf;
	memset(sbuf, 0x00, 0x100);
	memset(pbuf, 0x00, 0x100);
	int cnt = 0;					// visible sprites count
	int lin = 0;					// oam sprites count (0-63)
	int adr = 0;					// oam addr
	unsigned char shi = vid->bigspr ? 16 : 8;	// sprite size
	unsigned char x, y, tile, flag;
	unsigned char sln;
	unsigned char col;
	unsigned short bgadr, data;
	while (lin < 64) {
		y = vid->oam[adr++];
		tile = vid->oam[adr++];
		flag = vid->oam[adr++];
		x = vid->oam[adr++];
		if ((y < 0xf0) && (x < 0xff)) {				// sprite is visible
			sln = (line - 1 - y) & 0xff;
			if (sln < shi) {				// sprite is crossing current line
				if (cnt < maxspr) {
					if (flag & 0x80)
						sln = shi - sln - 1;		// VFlip
					// calculate address of tile line
					if (vid->bigspr) {
						bgadr = (tile & 1) ? 0x1000 : 0x0000;
						tile &= 0xfe;
						if (sln & 8) tile++;
						bgadr = bgadr | ((tile << 4) & 0xff0) | (sln & 7);
					} else {
						bgadr = ppuspa | ((tile << 4) & 0x0ff0) | (sln & 7);
					}
					// fetch tile data
					data = vid->mrd(bgadr, vid->data) & 0xff;
					data |= ((vid->mrd(bgadr + 8, vid->data) << 8) & 0xff00);
					for (y = 0; y < 8; y++) {
						if (flag & 0x40) {			// HFlip
							col = (data & 0x01) ? 1 : 0;
							col |= (data & 0x100) ? 2 : 0;
							data >>= 1;
						} else {
							col = (data & 0x80) ? 1 : 0;
							col |= (data & 0x8000) ? 2 : 0;
							data <<= 1;
						}
						col |= (flag & 3) << 2;			// add sprite palete
						col |= 0x10;				// sprite colors are 10-1f
						if (x + y < 0x100) {
							if (!(sbuf[x + y] & 3)) {
								sbuf[x + y] = col;
								pbuf[x + y] = flag & 0x20;	// !0 -> sprite behind bg, visible where bg col = 0
							}
							if ((lin == 0) && (col & 3))
								pbuf[x + y] |= 0x80;		// non-transparent sprite 0 pixel
						}
					}
				}
				cnt++;
			}
		}
		lin++;
	}
	// !!! if there is less than 8 sprites in line, dummy fetches of tile FF occured
	lin = cnt;
	if (vid->bigspr)
		bgadr = 0x1ff0;
	else
		bgadr = ppuspa | 0xff0;
	while (lin < maxspr) {
		vid->mrd(bgadr, vid->data);
		lin++;
	}
	return  cnt;
}

// @ every visible dot
void ppuDraw(Video* vid) {
	if (vid->hblank || vid->vblank) return;		// do not process on hblank

	if (!(vid->ray.x & 7) && vid->ray.x && vid->bgen) {			// no fetch @ X=0
		ppuRenderTile(vid, vid->bgline, vid->ray.x + 8, vid->vadr, vid->bgadr);
		vid->vadr = ppuXcoarse(vid->vadr);
	}

	unsigned char col = 0;
	unsigned char bgc = vid->bgline[(vid->ray.x + vid->finex) & 0xff];		// background color
	unsigned char spc = vid->spline[vid->ray.x & 0xff];				// sprite color

	if (vid->ray.x < 8) {
		if (!vid->bgleft8) bgc = 0;
		if (!vid->spleft8) spc = 0;
	}

	if ((bgc & 3) && (vid->prline[vid->ray.x] & 0x80))
		vid->sp0hit = 1;

	if (vid->bgblock) bgc = 0;
	if (vid->sprblock) spc = 0;

	if (vid->prline[vid->ray.x] & 0x20) {			// spr behind bg
		col = (bgc & 3) ? bgc : spc;
	} else {						// spr above bg
		col = (spc & 3) ? spc : bgc;
	}
	if ((col & 3) == 0) col = 0;				// universal color 0
	col = vid->ram[0x3f00 | (col & 0x3f)];			// pal index -> col index
	if (vid->greyscale) col &= 0x30;			// greyscale
	vid_dot_full(vid, col);
}

// @ start of HBlank
// Y here : 0 @ pre-render, 1 @ line 0, etc
void ppuHBL(Video* vid) {
	// here: render sprites
	int cnt;
	if (vid->ray.y > 240) return;		// 0 (pre-render) to 240 (239th scanline)

	if (vid->bgen) {
		if (vid->ray.y == 0) {		// @pre-render line do dummy BG fetches
			ppuRenderBGLine(vid, vid->bgline, vid->vadr, -vid->finex, vid->bgadr);
		}
		vid->vadr = ppuYinc(vid->vadr);		// increment vertical position in vadr
		vid->vadr &= ~0x041f;			// copy X related bits...
		vid->vadr |= (vid->tadr & 0x041f);	// ...from tadr to vadr
	}

	memset(vid->spline, 0x00, 256);		// sprites
	memset(vid->prline, 0x00, 256);		// sprites priority
	if (vid->spren) {
		cnt = ppuRenderSpriteLine(vid, vid->ray.y, vid->spline, vid->prline, vid->spadr, 8);
		if (cnt > 8) vid->spover = 1;		// 9+ sprites @ line (8 is NOT a problem)
//		if (!ppu->spleft8) {
//			memset(ppu->spline, 0x00, 0x08);
//			memset(ppu->prline, 0x00, 0x08);
//		}
	}
}

// @ start of new line (end of HBlank)
// Y here : real line number (pre-render = 261 (311))
void ppuLine(Video* vid) {

	memset(vid->bgline, 0x00, 256);		// clear bg

	// NES NTSC: 241/261; NES PAL:241/311; Dendy: 291/311
	if (vid->ray.y == vid->vbsline) {
		vid->ppu_vb = 1;
	} else if (vid->ray.y == vid->vbrline) {
		vid->ppu_vb = 0;
		vid->sp0hit = 0;
		vid->spover = 0;
		// if NTSC, x++ @ every other frame
		if ((vid->fcnt & 1) && vid->ntsc)
			vidSync(vid, vid->nsPerDot);
	}

	if (vid->ray.y > 239) return;		// 239? vid->vbsline?

	if (vid->ray.y == 0) {				// @ very 1st line
		if (vid->bgen) {
			vid->vadr &= 0x041f;		// copy Y related bits
			vid->vadr |= (vid->tadr & ~0x041f);
		}
	}
	// pre-render 2 tiles
	if (vid->bgen) {
		ppuRenderTile(vid, vid->bgline, 0, vid->vadr, vid->bgadr);
		vid->vadr = ppuXcoarse(vid->vadr);
		ppuRenderTile(vid, vid->bgline, 8, vid->vadr, vid->bgadr);
		vid->vadr = ppuXcoarse(vid->vadr);
	}
}

// frame callback (no need?)

void ppuFram(Video* vid) {
}

// rd/wr registers

int ppuRead(Video* vid, int reg) {
	int res = -1;
	int adr;
	switch (reg & 7) {
		case 2:
			res = vid->reg[2] & 0x1f;		// bits previously written to register
			if (vid->ppu_vb) res |= 0x80;		// clear this bit at reading
			if (vid->sp0hit) res |= 0x40;
			if (vid->spover) res |= 0x20;
			vid->ppu_vb = 0;
			vid->latch = 0;
			break;
		case 4:
			res = vid->oam[vid->oamadr & 0xff];
			break;
		case 7:
			adr = vid->vadr & 0x3fff;
			if (adr < 0x3f00) {
				res = vid->vbuf;
				vid->vbuf = vid->mrd(adr, vid->data) & 0xff;
			} else {
				res = vid->ram[(adr & 0x1f) | 0x3f00];		// palette
				vid->vbuf = vid->mrd(adr & 0x2fff, vid->data) & 0xff;
			}
			vid->vadr += vid->vastep ? 32 : 1;
			break;
	}
	return  res;
}

void ppuWrite(Video* vid, int reg, int val) {
	unsigned short adr;
	vid->reg[reg & 7] = val & 0xff;
	switch (reg & 7) {
		case 0:		// PPUCTRL
			vid->vastep = (val & 0x04) ? 1 : 0;
			vid->spadr = (val & 0x08) ? 0x1000 : 0x0000;
			vid->bgadr = (val & 0x10) ? 0x1000 : 0x0000;
			vid->bigspr = (val & 0x20) ? 1 : 0;
			vid->master = (val & 0x40) ? 1 : 0;
			vid->inten = (val & 0x80) ? 1 : 0;

			vid->tadr = (vid->tadr & ~0x0c00) | ((val << 10) & 0x0c00);
			vid->nt = val & 3;
			break;
		case 1:		// PPUMASK
			vid->greyscale = (val & 0x01) ? 1 : 0;
			vid->bgleft8 = (val & 0x02) ? 1 : 0;
			vid->spleft8 = (val & 0x04) ? 1 : 0;
			vid->bgen = (val & 0x08) ? 1 : 0;
			vid->spren = (val & 0x10) ? 1 : 0;
			// TODO: b5,6,7 = color tint
			ppuFillPal(vid, val & 0xe0);
			break;
		case 3:
			vid->oamadr = val & 0xff;
			break;
		case 4:
			vid->oam[vid->oamadr & 0xff] = val & 0xff;
			vid->oamadr++;
			break;
		case 5:
			if (vid->latch) {
				vid->tadr &= 0x0c1f;
				vid->tadr |= ((val & 0x07) << 12);
				vid->tadr |= ((val & 0xf8) << 2);
				vid->sc.y = (val >> 3) & 0x1f;
				vid->finey = val & 7;
				vid->latch = 0;
			} else {
				vid->tadr &= ~0x001f;
				vid->tadr |= ((val >> 3) & 0x1f);
				vid->finex = val & 7;
				vid->sc.x = (val >> 3) & 0x1f;
				vid->latch = 1;
			}
			break;
		case 6:
			if (vid->latch) {
				vid->tadr &= 0xff00;
				vid->tadr |= (val & 0xff);
				vid->vadr = vid->tadr;
				vid->latch = 0;
			} else {
				vid->tadr &= 0x00ff;
				vid->tadr |= ((val << 8) & 0x3f00);
				vid->latch = 1;
			}
			break;
		case 7:
			adr = vid->vadr & 0x3fff;
			if (adr < 0x3f00) {
				vid->mwr(adr, val, vid->data);
			} else {
				vid->ram[(adr & 0x1f) | 0x3f00] = val & 0xff;
				if ((adr & 0x1f) == 0x10)
					vid->ram[0x3f00] = val & 0xff;
				else if ((adr & 0x1f) == 0x00)
					vid->ram[0x3f10] = val & 0xff;
			}
			vid->vadr += vid->vastep ? 32 : 1;
			break;
	}
}
