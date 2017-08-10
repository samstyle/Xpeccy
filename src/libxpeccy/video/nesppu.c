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
	{0x00,0xfc,0xfc},{0xd8,0xd8,0xd8},{0x00,0x00,0x00},{0x00,0x00,0x00}
};

unsigned char nesInitIdx[32] = {
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
	ppu->ntadr = 0x2000;
	ppu->bgadr = 0;
	ppu->spadr = 0;
	ppu->scx = 0;
	ppu->scy = 0;
	memcpy(ppu->mem + 0x3f00, nesInitIdx, 32);
}

extern unsigned char col,ink,pap;

void ppuDraw(nesPPU* ppu) {
	col = 0;
	pap = ppu->bgline[(ppu->ray->x + ppu->scx) & 0x1ff];
	ink = ppu->spline[ppu->ray->x];

	if ((pap & 3) && (ppu->prline[ppu->ray->x] & 0x80))
		ppu->sp0hit = 1;

	if (ppu->bglock) pap = 0;
	if (ppu->splock) ink = 0;

	if (ppu->prline[ppu->ray->x] & 0x20) {			// spr behind bg
		col = (pap & 3) ? pap : ink;
	} else {					// spr above bg
		col = (ink & 3) ? ink : pap;
	}
	if ((col & 3) == 0) col = 0;				// universal color 0
	col = ppu->mem[0x3f00 | (col & 0x3f)];			// pal index -> col index
	if (ppu->greyscale) col &= 0x30;			// greyscale
	vidPutDot(ppu->ray, nesPal, col);
}

void ppuLine(nesPPU* ppu) {
	memset(ppu->bgline, 0x00, 512);		// bg
	memset(ppu->spline, 0x00, 256);		// sprites
	memset(ppu->prline, 0x00, 256);		// sprites priority
	if (ppu->ray->y > 239) return;
	if (ppu->ray->y == 0) ppu->sp0hit = 0;
	unsigned char x,y,flag;
	unsigned char shi,sln;
	int cnt;
	int lin;
	int vadr;
	int tadr;
	unsigned short data;
	unsigned char tile;
	unsigned char col;
	// render tiles
	if (ppu->bgen) {
		lin = (ppu->ray->y + ppu->scy);				// full line number
		if (lin > 0xef) lin += 0x10;				// shift to skip attr block
		if (lin > 0x1ef) lin += 0x10;
		lin &= 0x1ff;
		vadr = ppu->ntadr | ((lin & 0xf8) << 2);		// adr of line start @ left half
		if (lin > 255) vadr ^= 0x800;				// if line number > 255 - move to other top/bottom half
		cnt = 0;
		while (cnt < 512) {
			tile = ppu->mem[vadr & ppu->ntmask];
			tadr = ppu->bgadr | (tile << 4) | (lin & 7);	// 16 bytes/tile + line low 3 bits
			data = ppu->mrd(tadr, ppu->data);
			data |= (ppu->mrd(tadr + 8, ppu->data) << 8);
			do {
				col = (data & 0x80) ? 1 : 0;		// get direct color index 0-3
				if (data & 0x8000) col |= 2;
				ppu->bgline[cnt] = col;			// store to bg buffer (w/o shift to palete yet)
				data <<= 1;
				cnt++;
			} while (cnt & 7);				// repeat 8 times
			if ((vadr & 0x1f) == 0x1f) {
				vadr &= ~0x1f;
				vadr ^= 0x400;				// move to other half
			} else {
				vadr++;					// to next tile inside current half
			}
		}
		// attrs = bg color bit 2,3
		vadr = ppu->ntadr | 0x3c0 | ((lin & 0xe0) >> 2);	// adr of start of line attrs
		if (lin > 255) vadr ^= 0x800;
		cnt = 0;
		while (cnt < 512) {
			tile = ppu->mem[vadr & ppu->ntmask];
			if (lin & 0x10) tile >>= 4;			// lower 4x2 block
			col = (tile & 3) << 2;
			do {
				ppu->bgline[cnt] |= col;
				cnt++;
			} while (cnt & 0x0f);				// attr for 2 tiles = 16 dots
			col = tile & 0x0c;
			do {
				ppu->bgline[cnt] |= col;
				cnt++;
			} while (cnt & 0x0f);
			if ((vadr & 7) == 7) {
				vadr &= ~7;
				vadr ^= 0x400;
			} else {
				vadr++;
			}
		}
		if (!ppu->bgleft8) memset(ppu->bgline + ppu->scx, 0x00, 0x08);
	}
	// render sprites
	ppu->spover = 0;
	if (ppu->spen) {
		cnt = 0;
		lin = 0;
		vadr = 0;			// oam addr
		shi = ppu->bigspr ? 16 : 8;	// sprite height
		while ((cnt < 8) && (lin < 64)) {
			y = ppu->oam[vadr++];
			tile = ppu->oam[vadr++];
			flag = ppu->oam[vadr++];
			x = ppu->oam[vadr++];
			if ((x < 0xfa) && (y < 0xf0)) {		// sprite is visible
				sln = ppu->ray->y - y;
				if (sln < shi) {		// sprite is crossing current line
					if (flag & 0x80) sln = shi - sln - 1;		// VFlip
					if (ppu->bigspr) {
						tadr = ((tile & 1) ? 0x1000 : 0) | ((tile & 0xfe) << 4) | (sln & 7);
					} else {
						tadr = ppu->spadr | (tile << 4) | (sln & 7);
					}
					data = ppu->mrd(tadr, ppu->data);
					data |= (ppu->mrd(tadr + 8, ppu->data) << 8);
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
						ppu->spline[x + y] = col | 0x10;	// sprite colors are 10-1f
						ppu->prline[x + y] = flag & 0x20;	// !0 -> sprite behind bg, visible where bg col = 0
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
	ppu->vadr &= 0x3fff;
	if (ppu->vadr < 0x2000) {
		ppu->mwr(ppu->vadr, val, ppu->data);
	} else if (ppu->vadr < 0x3f00) {
		ppu->mem[(ppu->vadr & ppu->ntmask) | 0x2000] = val;		// nametables (! can be mapped to cartridge)
	} else {
		ppu->mem[(ppu->vadr & 0x1f) | 0x3f00] = val;		// palette
		if ((ppu->vadr & 0x1f) == 0x10) ppu->mem[0x3f00] = val;
		else if ((ppu->vadr & 0x1f) == 0x00) ppu->mem[0x3f10] = val;
	}
	ppu->vadr += ppu->vadrinc;
}

unsigned char ppuRead(nesPPU* ppu) {
	unsigned char res = 0xff;
	ppu->vadr &= 0x3fff;
	if (ppu->vadr < 0x3f00) {
		res = ppu->vbuf;
		if (ppu->vadr < 0x2000) {
			ppu->vbuf = ppu->mrd(ppu->vadr, ppu->data);
		} else {
			ppu->vbuf = ppu->mem[(ppu->vadr & ppu->ntmask) | 0x2000];
		}
	} else {
		res = ppu->mem[(ppu->vadr & 0x1f) | 0x3f00];		// palette
		ppu->vbuf = ppu->mem[(ppu->vadr & 0xfff & ppu->ntmask) | 0x2000];
	}
	ppu->vadr += ppu->vadrinc;
	return res;
}
