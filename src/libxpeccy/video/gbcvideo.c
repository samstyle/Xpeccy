#include <stdlib.h>
#include <string.h>

#include "gbcvideo.h"

#include <stdio.h>
#include <assert.h>

void gbcvDraw(GBCVid* vid) {
#if 1
	unsigned char col = 0;
	int x,y;
	unsigned char tile;
	int adr, data;
	if (vid->lcdon) {
		if (vid->stline[vid->ray->x]) {									// top sprites
			col = vid->stline[vid->ray->x];
		} else if (vid->winen && (vid->ray->y >= vid->win.y) && (vid->ray->x >= vid->win.x)) {		// window
			col = vid->line[vid->ray->x];
		} else if (vid->bgen && !vid->bgblock) {
			x = (vid->sc.x + vid->ray->x) & 0xff;
			y = (vid->sc.y + vid->ray->y) & 0xff;
			adr = vid->bgmapadr + ((y & 0xf8) << 2) + (x >> 3);		// tile adr
			tile = vid->ram[adr & 0x1fff];					// tile nr
			if (vid->altile) tile += 0x80;					// tile nr for alt.tileset
			adr = vid->tilesadr + (tile << 4) + ((y & 7) << 1);		// tile data adr
			data = vid->ram[adr & 0x1fff] & 0xff;
			adr++;
			data |= vid->ram[adr & 0x1fff] << 8;
			data <<= (x & 7);
			col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);
		}
		if ((col == 0) && vid->sbline[vid->ray->x]) {		// bg sprites : only if bg/win color = 0
			col = vid->sbline[vid->ray->x];
		}
	}
	vidPutDot(vid->ray, vid->pal, col);
#else
	unsigned char col = vid->line[vid->xpos & 0xff];
	vidPutDot(vid->ray, vid->pal, col);
	vid->xpos++;
#endif
	if (vid->ray->y < vid->lay->scr.y) {
		switch(vid->ray->x) {
			case 0:				// 40 : oam + vram, mode 2
				vid->mode = 2;
				if (vid->inten & 32)
					vid->intrq = 1;
				break;
			case 40:			// 86 : vram, mode 3
				vid->mode = 3;
				break;
			case 86:			// 102 : hblank, mode 0
				vid->mode = 0;
				if (vid->inten & 8)
					vid->intrq = 1;	// int @ hblank
				break;
		}
	}
}

// line begin : create full line image in the buffer
void gbcvLine(GBCVid* vid) {
	memset(vid->line, 0x00, 256);
	if (!vid->lcdon) return;
	unsigned char tile;
	int tadr;
	unsigned char col;
	unsigned short data;
	unsigned short flip;
	int tx, flag, bi;
	int x;
	int y;
	int adr;
	int pos;
#if 0
// BG
	if (vid->bgen) {
		y = (vid->sc.y + vid->ray->y) & 0xff;
		tadr = vid->bgmapadr + ((y & 0xf8) << 2);
		pos = 0;
		for (tx = 0; tx < 32; tx++) {					// 32 tiles in row
			tile = vid->ram[tadr & 0x1fff];				// get tile number
			if (vid->altile) tile += 0x80;				// alt.tiles -128..127 -> 0..255
			tadr++;
			adr = vid->tilesadr + (tile << 4) + ((y & 7) << 1);	// tile byte addr
			data = vid->ram[adr & 0x1fff] & 0xff;			// low byte
			data |= vid->ram[(adr + 1) & 0x1fff] << 8;		// hi byte
			for (bi = 0; bi < 8; bi++) {
				col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index 0..3
				vid->line[pos & 0xff] = col;
				pos++;
				data <<= 1;
			}
		}
	}
#endif
// WIN
	if (vid->winen && !vid->winblock && (vid->ray->y >= vid->win.y)) {
		tadr = vid->winmapadr + ((vid->wline & 0xf8) << 2);
		//pos = (vid->sc.x + vid->win.x) & 0xff;
		pos = vid->win.x;
		for (tx = 0; tx < 32; tx++) {
			tile = vid->ram[tadr & 0x1fff];
			if (vid->altile) tile += 0x80;
			tadr++;
			adr = vid->tilesadr + (tile << 4) + ((vid->wline & 7) << 1);
			data = vid->ram[adr & 0x1fff] & 0xff;
			data |= vid->ram[(adr + 1) & 0x1fff] << 8;
			for (bi = 0; bi < 8; bi++) {
				col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index 0..3
				vid->line[pos & 0xff] = col;
				pos++;
				data <<= 1;
			}
		}
		vid->wline++;
	}
// OAM (sprites)
	if (vid->spren && !vid->sprblock) {
		adr = 0;
		tx = 10;	// max 10 sprites in line
		memset(vid->stline, 0x00, 256);
		memset(vid->sbline, 0x00, 256);
		while ((adr < 0xa0) && (tx > 0)) {
			y = vid->oam[adr++] - 16;
			x = vid->oam[adr++] - 8;
			tile = vid->oam[adr++];
			flag = vid->oam[adr++];
			if (vid->bigspr)
				tile &= 0xfe;
			pos = (vid->ray->y - y) & 0xff;				// line inside sprite
			if (pos < (vid->bigspr ? 16 : 8)) {			// if line visible
				if (flag & 0x40) {				// flip Y
					pos = (vid->bigspr ? 15 : 7) - pos;
				}

				tadr = (tile << 4) + (pos << 1);		// adr of tile line
				data = vid->ram[tadr & 0x1fff] & 0xff;		// 8 dots color data
				data |= vid->ram[(tadr + 1) & 0x1fff] << 8;

				if (flag & 0x20) {		// flip X
					flip = 0;
					for (bi = 0; bi < 8; bi++) {
						flip <<= 1;
						flip |= (data & 0x0101);
						data >>= 1;
					}
					data = flip;
				}
				//pos = (vid->sc.x + x) & 0xff;				// X position in line buffer (shift to BG x)
				pos = x & 0xff;
				for (bi = 0; bi < 8; bi++) {
					col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index
					if (col) {			// not-transparent
						col |= (flag & 0x10) ? 0x44 : 0x40;			// shift to sprite palete
						if (flag & 0x80) {					// behind bg/win
							//if ((vid->line[pos & 0xff] == 0) && (vid->sline[pos & 0xff] == 0)) {
							if (vid->sbline[0xff] == 0) {
								vid->sbline[pos & 0xff] = col;
							}
						} else {
							if (vid->stline[pos & 0xff] == 0) {
								vid->stline[pos & 0xff] = col;
							}
						}
					}
					pos++;
					data <<= 1;
				}
				tx--;
			}
		}
//		for (pos = 0; pos < 256; pos++) {
//			if (vid->sline[pos] != 0)
//				vid->line[pos] = vid->sline[pos];
//		}
	}

	if ((vid->ray->y == vid->lyc) && (vid->inten & 64)) vid->intrq = 1;		// ly = lyc
	if (vid->ray->y < vid->lay->scr.y) {
		vid->mode = 2;		// visible line start : mode 2
	} else if (vid->ray->y == vid->lay->blank.y) {
		vid->mode = 1;		// line 144+ : vblank, mode 1
		if (vid->inten & 16)
			vid->intrq = 1;	// int @ vblank
	}
	vid->xpos = vid->sc.x;
}

void gbcvFram(GBCVid* vid) {
	vid->wline = 0;
}

// game boy palette

xColor iniCol[4] = {
	{0xE1, 0xF7, 0xD1},
	{0x87, 0xC3, 0x72},
	{0x33, 0x70, 0x53},
	{0x09, 0x20, 0x21}
};

GBCVid* gbcvCreate(vRay* ray, vLayout* lay) {
	GBCVid* vid = malloc(sizeof(GBCVid));
	memset(vid, 0x00, sizeof(GBCVid));
	vid->ray = ray;
	vid->lay = lay;
	vid->draw = gbcvDraw;
	vid->cbLine = gbcvLine;
	vid->cbFram = gbcvFram;
	for (int i = 0; i < 4; i++) {
		vid->pal[i] = iniCol[i];
	}
	return vid;
}

void gbcvDestroy(GBCVid* vid) {
	free(vid);
}

void gbcvReset(GBCVid* vid) {
	vid->sc.x = 0;
	vid->sc.y = 0;
	vid->win.x = 0;
	vid->win.y = 0;
	vid->bgen = 0;
	vid->winen = 0;
	vid->spren = 0;
	memset(vid->ram, 0x00, 0x2000);
	memset(vid->oam, 0x00, 0x100);
}
