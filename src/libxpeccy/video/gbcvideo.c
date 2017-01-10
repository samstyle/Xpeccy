#include <stdlib.h>
#include <string.h>

#include "gbcvideo.h"

#include <stdio.h>
#include <assert.h>

void gbcvDraw(GBCVid* vid) {
//	int pos = vid->ray->x + vid->sc.x;
	unsigned char col = vid->line[vid->xpos & 0xff] & 3;
	vidPutDot(vid->ray, vid->pal, col);
	vid->xpos++;
}

// line begin : create full line image in the buffer
void gbcvLine(GBCVid* vid) {
	memset(vid->line, 0x00, 256);
	if (!vid->lcdon) return;
	unsigned char tile;
	int tadr;
	unsigned char col;
	unsigned short data;
	int tx, bi;
	int y;
	int adr;
	int pos;
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

	vid->xpos = vid->sc.x;
//	vid->pos.y++;

	vid->lyequal = (vid->ray->y == vid->lyc) ? 1 : 0;
	if (vid->lyequal && (vid->inten.lyc))
		vid->intr = 1;
}

void gbcvFram(GBCVid* vid) {
	// vid->pos.y = vid->sc.y;
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
