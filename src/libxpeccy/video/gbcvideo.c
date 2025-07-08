#include <stdlib.h>
#include <string.h>

#include "video.h"

#include <stdio.h>
#include <assert.h>

void gbcvDraw(Video* vid) {
	unsigned char col = 0xff;		// color FF = white
	int x,y;
	unsigned char tile;
	unsigned char flag;
	int adr, data;

	if (vid->lcdon) {
		// get bg color & flag
		x = (vid->sc.x + vid->ray.x) & 0xff;
		y = (vid->sc.y + vid->ray.y) & 0xff;
		adr = vid->bgmapadr + ((y & 0xf8) << 2) + (x >> 3);		// tile adr
		tile = vid->ram[adr & 0x1fff];					// tile nr
		flag = vid->gbmode ? 0 : vid->ram[0x2000 | (adr & 0x1fff)];	// tile flags (GBC)
		if (flag & 0x20) x ^= 7;		// HFlip
		if (flag & 0x40) y ^= 7;		// VFlip
		if (vid->altile) tile += 0x80;					// tile nr for alt.tileset
		adr = vid->tilesadr + (tile << 4) + ((y & 7) << 1);		// tile data adr
		adr &= 0x1fff;
		if (flag & 8)
			adr |= 0x2000;
		data = vid->ram[adr & 0x3fff] & 0xff;
		adr++;
		data |= vid->ram[adr & 0x3fff] << 8;
		data <<= (x & 7);

		int bgen = vid->bgen && !vid->bgblock;
		int winen = vid->winen && !vid->winblock && (vid->ray.y >= vid->win.y) && (vid->ray.x >= vid->win.x);
		int spen = vid->spren && !vid->sprblock;

		if (winen && vid->bgprior && (vid->wtline[vid->ray.x] != 0xff)) {	// WIN + priority
			col = vid->wtline[vid->ray.x];
		} else if (bgen && vid->bgprior && (flag & 0x80)) {			// BG + priority
			col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);
			col |= (flag & 7) << 2;
		} else if (spen && vid->stline[vid->ray.x]) {				// top SPR
			col = vid->stline[vid->ray.x];
		} else if (winen) {							// WIN
			col = vid->wbline[vid->ray.x];
		} else if (bgen) {							// BG
			col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);
			col |= (flag & 7) << 2;
		}
		if (!col && vid->sbline[vid->ray.x]) {					// bottom sprites
			col = vid->sbline[vid->ray.x];
		}
	}
	vid_dot_full(vid, col);
	// vidPutDot(&vid->ray, vid->pal, col);
	if (vid->ray.y < vid->scrn.y) {		// on visible screen
		if (vid->ray.x == 0) {			// visible line start : mode 2
			vid->gbcmode = 2;
			if (vid->inten & 32)
				vid->intrq = 1;
		} else if (vid->ray.x == 40) {		// end of oem/vmem access : mode 3
			vid->gbcmode = 3;
		}
	}
}

// line begin : create full line image in the buffer
// TODO: create bgmap here too

void gbcvLine(Video* vid) {
	memset(vid->wtline, 0xff, 256);
	memset(vid->wbline, 0xff, 256);
	memset(vid->stline, 0x00, 256);
	memset(vid->sbline, 0x00, 256);

	if ((vid->lcnt == vid->intp.y) && (vid->inten & 64)) vid->intrq = 1;		// ly = lyc
	vid->xpos = vid->sc.x;

	if (!vid->lcdon) return;
	if (vid->ray.y >= vid->scrn.y) return;

	unsigned char tile;
	unsigned char flag;
	unsigned char col;
	unsigned char pal;
	unsigned short data;
	int tx, bi;
	int x;
	int y;
	int tadr;
	int adr;
	int pos;
// WIN
	if (vid->winen && (vid->ray.y >= vid->win.y)) {
		tadr = vid->winmapadr + ((vid->wline & 0xf8) << 2);
		pos = vid->win.x;
		for (tx = 0; tx < 32; tx++) {
			tile = vid->ram[tadr & 0x1fff];
			if (vid->altile) tile += 0x80;
			flag = vid->gbmode ? 0 : vid->ram[(tadr & 0x1fff) | 0x2000];
			tadr++;
			y = vid->wline;
			if (flag & 0x40) y ^= 7;
			adr = vid->tilesadr + (tile << 4) + ((y & 7) << 1);
			if (flag & 8) adr |= 0x2000;
			data = vid->ram[adr & 0x3fff] & 0xff;
			data |= vid->ram[(adr + 1) & 0x3fff] << 8;
			for (bi = 0; bi < 8; bi++) {
				if (flag & 0x20) {
					col = ((data & 0x01) | ((data & 0x0100) ? 2 : 0));
					data >>= 1;
				} else {
					col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index 0..3
					data <<= 1;
				}
				col |= (flag & 7) << 2;
				if (flag & 0x80) {
					vid->wtline[pos & 0xff] = col;
				} else {
					vid->wbline[pos & 0xff] = col;
				}
				pos++;
			}
		}
		vid->wline++;
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
			pos = (vid->ray.y - y) & 0xff;				// line inside sprite
			if (vid->gbmode) {					// start of palete color idx (0x40+ for sprites)
				pal = (flag & 0x10) ? 0x44 : 0x40;
				flag &= 0xf8;
			} else {
				pal = 0x40 | ((flag & 7) << 2);
			}
			if (pos < (vid->bigspr ? 16 : 8)) {			// if line visible
				if (flag & 0x40) {				// flip Y
					pos = (vid->bigspr ? 15 : 7) - pos;
				}
				tadr = (tile << 4) | (pos << 1);		// adr of tile data line
				if (flag & 8)					// vbank 1
					tadr |= 0x2000;
				data = vid->ram[tadr & 0x3fff] & 0xff;		// 8 dots color data
				tadr++;
				data |= vid->ram[tadr & 0x3fff] << 8;
				pos = x & 0xff;
				for (bi = 0; bi < 8; bi++) {
					if (flag & 0x20) {
						col = ((data & 0x01) | ((data & 0x0100) ? 2 : 0));
						data >>= 1;
					} else {
						col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index
						data <<= 1;
					}
					if (col) {							// not-transparent
						col += pal;						// shift to sprite palete
						if (flag & 0x80) {					// behind bg/win
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
				}
				tx--;
			}
		}
	}
}

void gbcvHBL(Video* vid) {
	if (!vid->vblank) {
		vid->gbcmode = 0;		// hblank start: mode 0 (not during vblank)
		if (vid->inten & 8)
			vid->intrq = 1;
	}
}

void gbcvVBL(Video* vid) {
	vid->gbcmode = 1;		// vblank start : mode 1
	if (vid->inten & 16)		// int @ vblank
		vid->intrq = 1;
}

void gbcvFram(Video* vid) {
	vid->wline = 0;
}

// game boy palette

xColor iniCol[4] = {
	{0xE1, 0xF7, 0xD1},
	{0x87, 0xC3, 0x72},
	{0x33, 0x70, 0x53},
	{0x09, 0x20, 0x21}
};

/*
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
	// special color 255: blank pixel
	vid->pal[255].r = 0xf0;
	vid->pal[255].g = 0xf0;
	vid->pal[255].b = 0xf0;
	return vid;
}

void gbcvDestroy(GBCVid* vid) {
	free(vid);
}
*/

void gbcvReset(Video* vid) {
	vid->sc.x = 0;
	vid->sc.y = 0;
	vid->win.x = 0;
	vid->win.y = 0;
	vid->bgen = 1;
	vid->winen = 0;
	vid->spren = 0;
	vid->gbmode = 0;
	vid->intp.x = 2000;		// remove standart int
	memset(vid->ram, 0x00, 0x4000);
	memset(vid->oam, 0x00, 0x100);
	for (int i = 0; i < 4; i++) {
		vid_set_col(vid, i, iniCol[i]);
	}
}
