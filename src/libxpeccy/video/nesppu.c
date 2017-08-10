#include "nesppu.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// full line 341
// nes layout: 256x240
// hblank: 85 dots ?
// vblank: 20 lines
// 1 cpu tick = 3 dots

// palette taken here:
// https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#NES

static xColor nesPal[64] = {
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
	{0x00,0xfc,0xfc},{0xd8,0xd8,0xd8},{0x00,0x00,0x00},{0x00,0x00,0x00}
};

static unsigned char nesInitIdx[32] = {
	0x09,0x01,0x00,0x01,0x00,0x02,0x02,0x0d,
	0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2c,
	0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,
	0x08,0x3a,0x00,0x02,0x00,0x20,0x2c,0x08
};

nesPPU* ppuCreate(vRay* rp) {
	nesPPU* ppu = (nesPPU*)malloc(sizeof(nesPPU));
	memset(ppu, 0x00, sizeof(nesPPU));
	ppu->ray = rp;
	return ppu;
}

void ppuDestroy(nesPPU* ppu) {
	if (ppu) free(ppu);
}

void ppuReset(nesPPU* ppu) {
	ppu->inten = 0;
	ppu->latch = 0;
	ppu->bgen = 1;
	ppu->spen = 1;
	ppu->bgadr = 0;
	ppu->spadr = 0;
	ppu->vadr = 0;
	ppu->tadr = 0;
	ppu->finex = 0;
	memcpy(ppu->mem + 0x3f00, nesInitIdx, 32);
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

void ppuDraw(nesPPU* ppu) {
	unsigned char col = 0;
	unsigned char bgc = ppu->bgline[ppu->ray->x & 0xff];		// background color
	unsigned char spc = ppu->spline[ppu->ray->x & 0xff];		// sprite color

	if ((bgc & 3) && (ppu->prline[ppu->ray->x] & 0x80))
		ppu->sp0hit = 1;

	if (ppu->bglock) bgc = 0;
	if (ppu->splock) spc = 0;

	if (ppu->prline[ppu->ray->x] & 0x20) {			// spr behind bg
		col = (bgc & 3) ? bgc : spc;
	} else {					// spr above bg
		col = (spc & 3) ? spc : bgc;
	}
	if ((col & 3) == 0) col = 0;				// universal color 0
	col = ppu->mem[0x3f00 | (col & 0x3f)];			// pal index -> col index
	if (ppu->greyscale) col &= 0x30;			// greyscale
	vidPutDot(ppu->ray, nesPal, col);
}

// from nesdev wiki:
// tile address      = 0x2000 | (v & 0x0FFF)
// attribute address = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07)

void ppuLine(nesPPU* ppu) {

	unsigned char x,y,flag;
	unsigned char shi,sln;
	int cnt;
	int lin;
	unsigned short adr;
	unsigned short bgadr;
	unsigned short data;
	unsigned char tile;
	unsigned char atr;
	unsigned char col;

	if (ppu->ray->y < 239) {			// only on visible screen
		if (ppu->ray->y == 0) {
			if (ppu->bgen) {
				ppu->vadr &= 0x041f;		// copy Y related bits
				ppu->vadr |= (ppu->tadr & 0x7be0);
			}
			ppu->sp0hit = 0;
		} else if (ppu->bgen) {
			ppu->vadr = ppuYinc(ppu->vadr);	// increment vertical position in vadr
		}
		if (ppu->bgen) {
			ppu->vadr &= 0x7be0;			// copy X related bits...
			ppu->vadr |= (ppu->tadr & 0x041f);	// ...from tadr to vadr
		}
	}

	memset(ppu->bgline, 0x00, 512);		// bg
	memset(ppu->spline, 0x00, 256);		// sprites
	memset(ppu->prline, 0x00, 256);		// sprites priority

	if (ppu->ray->y > 239) return;

// render tiles

	if (ppu->bgen) {
		cnt = 0;
		adr = ppu->vadr;
		while (cnt < 0x108) {
			tile = ppu->mem[(0x2000 | (adr & 0x0fff)) & ppu->ntmask];						// tile num
			atr = ppu->mem[(0x23c0 | (adr & 0x0c00) | ((adr >> 4) & 0x38) | ((adr >> 2) & 7)) & ppu->ntmask];	// attribute
			bgadr = ppu->bgadr | ((tile << 4) & 0x0ff0) | ((adr >> 12) & 7);
			data = ppu->mrd(bgadr, ppu->data);
			data |= (ppu->mrd(bgadr + 8, ppu->data) << 8);
			if (adr & 0x0040) atr >>= 4;			// bit 3,4 = attribute of current tile
			if (~adr & 0x0002) atr <<= 2;
			do {
				col = (data & 0x80) ? 1 : 0;
				if (data & 0x8000) col |= 2;
				col |= (atr & 0x0c);
				if (cnt >= ppu->finex)
					ppu->bgline[cnt - ppu->finex] = col;
				data <<= 1;
				cnt++;
			} while (cnt & 7);
			adr = ppuXcoarse(adr);		// move to next tile
		}
		if (!ppu->bgleft8)					// hide left 8 pixels?
			memset(ppu->bgline, 0x00, 8);
	}

// render sprites

	ppu->spover = 0;
	if (ppu->spen) {
		cnt = 0;
		lin = 0;
		adr = 0;			// oam addr
		shi = ppu->bigspr ? 16 : 8;	// sprite height
		while ((cnt < 8) && (lin < 64)) {
			y = ppu->oam[adr++];
			tile = ppu->oam[adr++];
			flag = ppu->oam[adr++];
			x = ppu->oam[adr++];
			if (y < 0xf0) {				// sprite is visible
				sln = ppu->ray->y - y;
				if (sln < shi) {		// sprite is crossing current line
					if (flag & 0x80) sln = shi - sln - 1;		// VFlip
					if (ppu->bigspr) {
						bgadr = ((tile & 1) ? 0x1000 : 0) | ((tile << 4) & 0xfe0) | (sln & 7);
					} else {
						bgadr = ppu->spadr | ((tile << 4) & 0x0ff0) | (sln & 7);
					}
					data = ppu->mrd(bgadr, ppu->data);
					data |= (ppu->mrd(bgadr + 8, ppu->data) << 8);
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
						col |= 0x10;
						if (!(ppu->spline[x + y] & 3)) {
							ppu->spline[x + y] = col;	// sprite colors are 10-1f
							ppu->prline[x + y] = flag & 0x20;	// !0 -> sprite behind bg, visible where bg col = 0
						}
						if ((lin == 0) && (col & 3))
							ppu->prline[x + y] |= 0x80;

					}
					cnt++;
				}
			}
			lin++;
		}
		if (cnt > 7) ppu->spover = 1;
		if (!ppu->spleft8) {
			memset(ppu->spline, 0x00, 0x08);
			memset(ppu->prline, 0x00, 0x08);
		}
	}
}

// frame callback (no need?)

void ppuFram(nesPPU* ppu) {
}

// rd/wr

void ppuWrite(nesPPU* ppu, unsigned char val) {
//	ppu->vadr &= 0x3fff;
	unsigned short adr = ppu->vadr & 0x3fff;
	if (adr < 0x2000) {
		ppu->mwr(adr, val, ppu->data);
	} else if (adr < 0x3f00) {
		ppu->mem[(adr & ppu->ntmask) | 0x2000] = val;		// nametables (! can be mapped to cartridge)
	} else {
		ppu->mem[(adr & 0x1f) | 0x3f00] = val;		// palette
		if ((adr & 0x1f) == 0x10)
			ppu->mem[0x3f00] = val;
		else if ((adr & 0x1f) == 0x00)
			ppu->mem[0x3f10] = val;
	}
	ppu->vadr += ppu->vadrinc;
}

unsigned char ppuRead(nesPPU* ppu) {
	unsigned char res = 0xff;
	unsigned short adr = ppu->vadr & 0x3fff;
	if (adr < 0x3f00) {
		res = ppu->vbuf;
		if (adr < 0x2000) {
			ppu->vbuf = ppu->mrd(adr, ppu->data);
		} else {
			ppu->vbuf = ppu->mem[(adr & ppu->ntmask) | 0x2000];
		}
	} else {
		res = ppu->mem[(adr & 0x1f) | 0x3f00];		// palette
		ppu->vbuf = ppu->mem[(adr & 0x2fff & ppu->ntmask) | 0x2000];
	}
	ppu->vadr += ppu->vadrinc;
	return res;
}
