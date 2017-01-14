#include <stdlib.h>
#include <string.h>

#include "gbcvideo.h"

#include <stdio.h>
#include <assert.h>

void gbcvDraw(GBCVid* vid) {
	unsigned char col = vid->line[vid->xpos & 0xff];
	vidPutDot(vid->ray, vid->pal, col);
	vid->xpos++;
	if ((vid->inten & 8) && (vid->ray->x == vid->lay->blank.x)) vid->intrq = 1;	// hblank
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
// OAM (sprites)
	if (vid->spren) {
		adr = 0;
		tx = 10;	// max 10 sprites in line
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
					pos = (vid->bigspr ? 16 : 8) - pos;
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

				pos = (vid->sc.x + x) & 0xff;				// X position in line buffer (shift to BG x)
				for (bi = 0; bi < 8; bi++) {
					col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index
					if (col && !(vid->line[pos & 0xff] && (flag & 0x80))) {
						col |= (flag & 0x10) ? 0x44 : 0x40;				// set sprite palete
						vid->line[pos & 0xff] = col;
					}
					pos++;
					data <<= 1;
				}
				tx--;
			}
		}

	}

	if ((vid->ray->x == 0) && (vid->inten & 32)) vid->intrq = 1;			// oam (start of line?)
	if ((vid->ray->y == vid->lyc) && (vid->inten & 64)) vid->intrq = 1;		// ly = lyc
	if ((vid->ray->y == vid->lay->blank.y) && (vid->inten & 16)) vid->intrq = 1;	// vblank
	vid->xpos = vid->sc.x;
}

void gbcvFram(GBCVid* vid) {

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
