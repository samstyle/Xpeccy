#include <stdlib.h>
#include <string.h>

#include "video.h"

#include <stdio.h>
#include <assert.h>

// listen here, you little sh*t... why don't work properly?

xColor iniCol[4] = {
	{0xE1, 0xF7, 0xD1},
	{0x87, 0xC3, 0x72},
	{0x33, 0x70, 0x53},
	{0x09, 0x20, 0x21}
};

void gbcRenderBG(Video* vid) {
	int x,y,ry,adr,tadr,data,tx,bi;
	unsigned char tile,flag,col;
	if (vid->bgen && !vid->bgblock) {
		y = (vid->sc.y + vid->lcnt) & 0xff;
		adr = vid->bgmapadr + ((y & 0xf8) << 2) + (vid->sc.x >> 3);	// 1st visible tile adr
		x = (vid->sc.x & 0xf8) - vid->sc.x;				// coord of left tile pixel (may be -7..0)
		for (tx = 0; tx < 21; tx++) {
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

void gbcRenderWIN(Video* vid) {
	int adr,tadr,y,pos,tx,bi,data;
	unsigned char tile,flag,col;
	if (vid->winen && !vid->winblock && (vid->lcnt >= vid->win.y)) {
		adr = vid->winmapadr + ((vid->wline & 0xf8) << 2);
		pos = vid->win.x;
		for (tx = 0; (tx < 21) && (pos < 161); tx++) {
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
				if ((!vid->gbmode && vid->bgprior) || (flag & 0x80)) col |= 0x80;
				if (vid->line[pos & 0xff] & 0x80) col |= 0x80;
				vid->line[pos & 0xff] = col;
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
			pos = (vid->lcnt - y) & 0xff;				// line inside sprite
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
							col |= 0x80;		// to prevent next sprites overlap
							if ((!vid->gbmode && vid->bgprior) || (vid->line[pos & 0xff] & 0x80) || (flag & 0x80)) {			// only above BG/WIN color 00
								if (!(vid->line[pos & 0xff] & 3)) {
									vid->line[pos & 0xff] = col;
								}
							} else {
								vid->line[pos & 0xff] = col;
							}
						}
						pos++;
					}
				}
				tx++;
			}
		}
	}
}

void gbcRenderLine(Video* vid) {
	memset(vid->line, 0x00, 256);
// BG
	gbcRenderBG(vid);
// WIN
	gbcRenderWIN(vid);
// OAM (sprites)
	gbcRenderSPR(vid);
}

int gbcCountSPR(Video* vid) {
	int tx = 0;
	int adr,y;
	if (vid->spren) {
		adr = 0;
		while ((adr < 0xa0) && (tx < 10)) {	// max 10 sprites in line
			y = vid->oam[adr++] - 16;
			y = (vid->lcnt - y) & 0xff;				// line inside sprite
			if (y < (vid->bigspr ? 16 : 8)) {			// if line visible
				tx++;
			}
		}
	}
	return tx;
}

// delay (dots) for a number of sprites in a row
static int sprDelay[11] = {0, 8, 20, 32, 44, 52, 64, 76, 88, 96, 108};

void gbcvMode0(Video* vid) {
	vid->intrq = 0;
	vid->gbcmode = 0;
	if ((vid->inten & 8) && !vid->intrq) {
		vid->intrq = 1;
		vid_irq(vid, IRQ_VID_INT);
	}
}

void gbcvMode3(Video* vid) {
	vid->gbcmode = 3;
	vid->busy = 172 + (vid->sc.x & 7);
	int n = gbcCountSPR(vid);
	vid->busy += sprDelay[n];
	vid->cbCount = gbcvMode0;
}

void gbcvMode2(Video* vid) {
	if (vid->vblank) {
		vid->intrq = 0;
		vid->gbcmode = 1;
	} else {
		vid->gbcmode = 2;
		vid->busy = 80;
		vid->cbCount = gbcvMode3;
		if ((vid->inten & 32) && !vid->intrq) {
			vid->intrq = 1;
			vid_irq(vid, IRQ_VID_INT);
		}
	}
}

// comparer ly-lyc
void gbcvLYC(Video* vid) {
	if ((vid->lcnt == vid->intp.y) && (vid->inten & 64) && !vid->intrq) {
		vid->intrq = 1;
		vid_irq(vid, IRQ_VID_INT);
	}
}

void gbcvDraw(Video* vid) {
	unsigned char col;
	if (vid->lcdon) {
		col = vid->line[vid->ray.x & 0xff] & 0x7f;
	} else {
		col = 0x03;		// color = white
	}
	vid_dot_full(vid, col);
}

// line begin : create full line image in the buffer
// GB: compare LYC0 @ line 153, don't compare @ line 0, (LY=0 @ line 153 ? )
// GBC wants line153 as line153 (without ly=0) ? OR line0 use palette from line153 ?

void gbcvLine(Video* vid) {
	if (vid->ray.y != 0) vid->lcnt++;			// ly=0 since line 153, don't increase until line 1
	if (vid->gbmode) {
		if (vid->ray.y == vid->full.y - 1) {
			vid->lcnt = 0;
			gbcRenderLine(vid);
		}
		if (vid->ray.y != 0) gbcvLYC(vid);						// @line153 compare LYC with LY=0
	} else {
		if (vid->ray.y != 0) gbcvLYC(vid);
		if (vid->ray.y == vid->full.y - 1) {
			vid->lcnt = 0;
			gbcRenderLine(vid);
		}
	}
	gbcvMode2(vid);						// start mode 2, or mode 1 if vblank

	if (!vid->lcdon) return;
	if (vid->vblank) return;		// vblank

	if (vid->ray.y > 0) gbcRenderLine(vid);
}

/*
void gbcvHBL(Video* vid) {
	if (!vid->vblank) {
		vid->gbcmode = 0;		// hblank start: mode 0 (not during vblank)
		if ((vid->inten & 8) && !vid->intrq) {
			vid->intrq = 1;
			vid_irq(vid, IRQ_VID_INT);
		}
	}
}
*/
void gbcvVBL(Video* vid) {
	vid->gbcmode = 1;		// vblank start : mode 1
	if ((vid->inten & 16) && !vid->intrq) {		// int @ vblank
		vid->intrq = 1;
		vid_irq(vid, IRQ_VID_INT);
	}
}

void gbcvFram(Video* vid) {
	vid->wline = 0;
	vid->lcnt = 0;
}

// registers r/w

void setGrayScale(Video* vid, int base, unsigned char val) {
	vid_set_col(vid, base, iniCol[val & 3]);
	vid_set_col(vid, base + 1, iniCol[(val >> 2) & 3]);
	vid_set_col(vid, base + 2, iniCol[(val >> 4) & 3]);
	vid_set_col(vid, base + 3, iniCol[(val >> 6) & 3]);
}

void gbcv_wr(Video* vid, int port, int val) {
	int sadr,dadr;
	port &= 0x0f;
	port |= 0x40;
	vid->reg[port] = val & 0xff;
	switch(port) {
		case 0x40:
			vid->lcdon = (val & 0x80) ? 1 : 0;
			vid->winmapadr = (val & 0x40) ? 0x9c00 : 0x9800;
			vid->winen = (val & 0x20) ? 1 : 0;
			vid->altile = (val & 0x10) ? 0 : 1;			// signed tile num
			vid->tilesadr = (val & 0x10) ? 0x8000 : 0x8800;
			vid->bgmapadr = (val & 0x08) ? 0x9c00 : 0x9800;
			vid->bigspr = (val & 0x04) ? 1 : 0;
			vid->spren  = (val & 0x02) ? 1 : 0;
			if (vid->gbmode) {
				vid->bgen = (val & 1);		// gbc in gb mode has different effect
				vid->bgprior = 0;
			} else {
				vid->bgprior = (val & 1);		// change bg/win priority (1 if spr under bg)
			}
			break;
		case 0x41:						// lcd stat interrupts enabling
			vid->inten = val;
			break;
		case 0x42:
			vid->sc.y = val;
			break;
		case 0x43:
			vid->sc.x = val;
			break;
		case 0x44:						// TODO: writing will reset the counter (?) or it's read-only and is ray.y?
			// vid->lcnt = 0;
			break;
		case 0x45:
			vid->intp.y = val;
			//gbcvLYC(vid);					// and compare immediately (?)
			break;
		case 0x46:						// TODO: block CPU memory access for 160 microsec (except ff80..fffe)
			sadr = (val << 8) & 0xffff;
			dadr = 0;
			while (dadr < 0xa0) {
				vid->oam[dadr] = vid->mrd(sadr, vid->xptr);	// = memRd(comp->mem, sadr) & 0xff;
				sadr++;
				dadr++;
			}
			break;
		case 0x47:						// palete for bg/win
			if (vid->gbmode)
				setGrayScale(vid, 0, val);
			break;
		case 0x48:
			if (vid->gbmode)
				setGrayScale(vid, 0x40, val);	// object pal 0
			break;
		case 0x49:
			if (vid->gbmode)
				setGrayScale(vid, 0x44, val);	// object pal 1
			break;
		case 0x4a:
			vid->win.y = val;
			break;
		case 0x4b:
			vid->win.x = val - 7;
			break;
	}
}

// game boy palette

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
