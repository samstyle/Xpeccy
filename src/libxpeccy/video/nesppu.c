#include "nesppu.h"

#include <stdlib.h>
#include <string.h>

// full line 341
// nes layout: 256x240
// hblank: 85 cycles ?
// vblank: 20 lines
// 1 cpu tick = 3 dots

nesPPU* ppuCreate(vRay* rp) {
	nesPPU* ppu = (nesPPU*)malloc(sizeof(nesPPU));
	memset(ppu, 0x00, sizeof(nesPPU));
	ppu->ray = rp;
	return ppu;
}

void ppuDestroy(nesPPU* ppu) {
	if (ppu) free(ppu);
}

void ppuDraw(nesPPU* ppu) {

}

void ppuLine(nesPPU* ppu) {
	memset(ppu->bgline, 0x00, 256);		// bg
	memset(ppu->spline, 0x00, 256);		// sprites
	memset(ppu->prline, 0x00, 256);		// sprites priority
	int cnt;
	int lin;
	int vadr;
	int tadr;
	unsigned short data;
	unsigned char tile;
	unsigned char col;
	// render tiles
	if (ppu->bgen) {
		lin = ppu->ray->y + ppu->scy;				// full line number
		vadr = ppu->ntadr | ((lin & 0xf8) << 2);		// adr of line start @ left half
		if (lin > 255) vadr ^= 0x800;				// if line number > 255 - move to other top/bottom half
		cnt = 0;
		while (cnt < 512) {
			tile = ppu->mem[vadr];
			tadr = ppu->bgadr | (tile << 4) | (lin & 7);	// 16 bytes/tile + line low 3 bits
			data = ppu->mem[tadr] & 0xff;			// 8 pix x 2bpp color data
			data |= (ppu->mem[tadr + 0x10]);
			do {
				col = (data & 0x80) ? 1 : 0;		// get direct color index 0-3
				if (data & 0x8000) col |= 2;
				ppu->bgline[cnt] = col;			// store to bg buffer
				data >>= 1;
				cnt++;
			} while (cnt & 7);				// repeat 8 times
			if ((vadr & 0x1f) == 0x1f) {
				vadr &= ~0x1f;
				vadr ^= 0x400;				// move to other half
			} else {
				vadr++;					// to next tile inside current half
			}
		}

	}
	// render sprites
	if (ppu->spen) {

	}

}

void ppuFram(nesPPU* ppu) {

}
