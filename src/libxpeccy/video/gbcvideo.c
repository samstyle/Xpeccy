#include <stdlib.h>
#include <string.h>

#include "video.h"

#include <stdio.h>
#include <assert.h>

// listen here, you little sh*t... why don't work properly?

#define GBRENDERVER 1

#if GBRENDERVER > 0
void gbcRenderBG(Video* vid) {
	int x,y,ry,adr,tadr,data,tx,bi;
	unsigned char tile,flag,col;
	if (vid->bgen && !vid->bgblock) {
		y = (vid->sc.y + vid->ray.y) & 0xff;
		adr = vid->bgmapadr + ((y & 0xf8) << 2) + (vid->sc.x >> 3);	// 1st visible tile adr
		x = (vid->sc.x & 0xf8) - vid->sc.x;				// coord of left tile pixel (may be -7..0)
		for (tx = 0; tx < 32; tx++) {
			tile = vid->ram[adr & 0x1fff];					// tile nr
			if (vid->altile) tile += 0x80;
			flag = vid->gbmode ? 0 : vid->ram[0x2000 | (adr & 0x1fff)];	// tile flags (GBC)
			ry = (flag & 0x40) ? y ^ 7 : y;		// VFlip, don't touch y
			tadr = vid->tilesadr + (tile << 4) + ((ry & 7) << 1);		// tile data adr
			tadr &= 0x1fff;
			if (flag & 8) tadr |= 0x2000;
			data = vid->ram[tadr] & 0xff;
			data |= (vid->ram[tadr+1] << 8);
			for (bi = 0; bi < 8; bi++) {
				if (flag & 0x20) {		// HFlip
					col = (data & 1) | ((data >> 7) & 2);
					data >>= 1;
				} else {
					col = ((data >> 7) & 1) | ((data >> 14) & 2);
					data <<= 1;
				}
				col |= (flag & 7) << 2;
				if ((!vid->gbmode && vid->bgprior) || (flag & 0x80)) col |= 0x80;
				vid->line[x & 0xff] = col;
				x++;
			}
			adr = (adr & ~0x1f) | ((adr + 1) & 0x1f);
		}
	}
}
#endif

void gbcRenderWIN(Video* vid) {
	int adr,tadr,y,pos,tx,bi,data;
	unsigned char tile,flag,col;
	if (vid->winen && !vid->winblock && (vid->ray.y >= vid->win.y)) {
		adr = vid->winmapadr + ((vid->wline & 0xf8) << 2);
		pos = vid->win.x;
		for (tx = 0; (tx < 32) & (pos < 168); tx++) {
			tile = vid->ram[adr & 0x1fff];
			if (vid->altile) tile += 0x80;
			flag = vid->gbmode ? 0 : vid->ram[(adr & 0x1fff) | 0x2000];
			adr = (adr & ~0x1f) | ((adr + 1) & 0x1f);
			y = vid->wline;
			if (flag & 0x40) y ^= 7;
			tadr = vid->tilesadr + (tile << 4) + ((y & 7) << 1);
			if (flag & 8) tadr |= 0x2000;
			data = vid->ram[tadr & 0x3fff] & 0xff;
			data |= vid->ram[(tadr + 1) & 0x3fff] << 8;
			for (bi = 0; bi < 8; bi++) {
				if (flag & 0x20) {
					col = ((data & 0x01) | ((data & 0x0100) ? 2 : 0));
					data >>= 1;
				} else {
					col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);	// color index 0..3
					data <<= 1;
				}
				col |= (flag & 7) << 2;
#if GBRENDERVER==0
				if (flag & 0x80) {
					vid->wtline[pos & 0xff] = col;
				} else {
					vid->wbline[pos & 0xff] = col;
				}
#else
				if ((!vid->gbmode && vid->bgprior) || (flag & 0x80)) col |= 0x80;
				if (vid->line[pos & 0xff] & 0x80) col |= 0x80;
				vid->line[pos & 0xff] = col;
#endif
				pos++;
			}
		}
		vid->wline++;
	}
}

void gbcRenderSPR(Video* vid) {
	int tx = 0;
	int adr,tadr,bi,x,y,pos,data;
	unsigned char tile,flag,pal,col;
	if (vid->spren) {
		adr = 0;
		while ((adr < 0xa0) && (tx < 10)) {	// max 10 sprites in line
			y = vid->oam[adr++] - 16;
			x = vid->oam[adr++] - 8;
			tile = vid->oam[adr++];
			flag = vid->oam[adr++];
			if (vid->bigspr)
				tile &= 0xfe;
			pos = (vid->ray.y - y) & 0xff;				// line inside sprite
			if (pos < (vid->bigspr ? 16 : 8)) {			// if line visible
				if (!vid->sprblock) {				// need to count anyway
					if (vid->gbmode) {				// start of palete color idx (0x40+ for sprites)
						pal = (flag & 0x10) ? 0x44 : 0x40;
						flag &= 0xf8;
					} else {
						pal = 0x40 | ((flag & 7) << 2);
					}
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
							col = (data & 1) | ((data >> 7) & 2);
							data >>= 1;
						} else {
							col = ((data >> 7) & 1) | ((data >> 14) & 2);
							data <<= 1;
						}
						if (col) {							// not-transparent
							col += pal;						// shift to sprite palete
#if GBRENDERVER==0
							if (flag & 0x80) {					// behind bg/win
								if (vid->sbline[0xff] == 0) {
									vid->sbline[pos & 0xff] = col;
								}
							} else {
								if (vid->stline[pos & 0xff] == 0) {
									vid->stline[pos & 0xff] = col;
								}
							}
#else
							col |= 0x80;		// to prevent next sprites overlap
							if ((!vid->gbmode && vid->bgprior) || (vid->line[pos & 0xff] & 0x80) || (flag & 0x80)) {			// only above BG/WIN color 00
								if (!(vid->line[pos & 0xff] & 3)) {
									vid->line[pos & 0xff] = col;
								}
							} else {
								vid->line[pos & 0xff] = col;
							}
#endif
						}
						pos++;
					}
				}
				tx++;
			}
		}
	}
}

int gbcCountSPR(Video* vid) {
	int tx = 0;
	int adr,y;
	if (vid->spren) {
		adr = 0;
		while ((adr < 0xa0) && (tx < 10)) {	// max 10 sprites in line
			y = vid->oam[adr++] - 16;
			y = (vid->ray.y - y) & 0xff;				// line inside sprite
			if (y < (vid->bigspr ? 16 : 8)) {			// if line visible
				tx++;
			}
		}
	}
	return tx;
}

void gbcvDraw(Video* vid) {
	unsigned char col = 0xff;		// color FF = white

	if (vid->lcdon) {
#if GBRENDERVER==0
		int x,y;
		unsigned char tile;
		unsigned char flag;
		int adr, data;
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
#elif GBRENDERVER==1
		col = vid->line[vid->ray.x & 0xff] & 0x7f;
#endif
	}
#if GBRENDERER < 2
	vid_dot_full(vid, col);
#endif
	// vidPutDot(&vid->ray, vid->pal, col);
	if (!vid->vblank) {				// on visible screen
		if (vid->ray.x == 0) {			// visible line start : mode 2
			vid->gbcmode = 2;
			if ((vid->inten & 32) && !vid->intrq) {
				vid->intrq = 1;
				vid_irq(vid, IRQ_VID_INT);
			}
		} else if (vid->ray.x == 80) {		// end of oem/vmem access : mode 3
			vid->gbcmode = 3;
			vid->xpos = 172 + 80 + (vid->sc.x & 7);
			int tx = gbcCountSPR(vid);
			if (tx > 0) vid->xpos += 12 * tx - 4;
		} else if (vid->ray.x == vid->xpos) {
			gbcvHBL(vid);
		}
	}
}

// line begin : create full line image in the buffer
// TODO: create bgmap here too

void gbcvLine(Video* vid) {
	vid->intrq = 0;
#if GBRENDERVER==0
	memset(vid->wtline, 0xff, 256);
	memset(vid->wbline, 0xff, 256);
	memset(vid->stline, 0x00, 256);
	memset(vid->sbline, 0x00, 256);
#elif GBRENDERVER==1
	memset(vid->line, 0x00, 256);
#endif

	if ((vid->ray.y == vid->intp.y) && (vid->inten & 64)) {		// ly = lyc (prior int)
		vid->intrq = 1;
		vid_irq(vid, IRQ_VID_INT);
	}

	if (!vid->lcdon) return;
	if (vid->ray.y >= vid->scrn.y) return;
// BG
#if GBRENDERVER==1
	gbcRenderBG(vid);
#endif
// WIN
	gbcRenderWIN(vid);
// OAM (sprites)
	gbcRenderSPR(vid);
}

void gbcvHBL(Video* vid) {
	if (!vid->vblank) {
		vid->gbcmode = 0;		// hblank start: mode 0 (not during vblank)
		if ((vid->inten & 8) && !vid->intrq) {
			vid->intrq = 1;
			vid_irq(vid, IRQ_VID_INT);
		}
	}
}

void gbcvVBL(Video* vid) {
	vid->gbcmode = 1;		// vblank start : mode 1
	if ((vid->inten & 16) && !vid->intrq) {		// int @ vblank
		vid->intrq = 1;
		vid_irq(vid, IRQ_VID_INT);
	}
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

void gbcvReset(Video* vid) {
	vid->sc.x = 0;
	vid->sc.y = 0;
	vid->win.x = 0;
	vid->win.y = 0;
	vid->bgen = 1;
	vid->winen = 0;
	vid->spren = 0;
	vid->gbmode = 0;
	vid->bgprior = 0;		// gb can't change it, gbc only
	vid->intp.x = 2000;		// remove standart int
	memset(vid->ram, 0x00, 0x4000);
	memset(vid->oam, 0x00, 0x100);
	for (int i = 0; i < 4; i++) {
		vid_set_col(vid, i, iniCol[i]);
	}
}
