#include "video.h"

static int xadr;
static unsigned char sbyte;
static unsigned char cola;
static unsigned char colb;
static int xscr;
static int yscr;

// b/w mode
// 1bit = 1dot
// 512 x 256
// line size = 512/8 = 64bytes
// pal: 0-black 1-white
void bk_bw_dot(Video* vid) {
	if (vid->hbrd || vid->vbrd) {
		cola = 0;
		colb = 0;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((vid->ray.x & 3) == 0) {
			yscr = vid->ray.y - vid->bord.y;
			xadr = 0x4000 | (yscr << 6) | ((xscr >> 2) & 0x1f);
			sbyte = vid->mrd(xadr, vid->data);
		}
		cola = (sbyte & 0x80) ? 1 : 0;
		colb = (sbyte & 0x40) ? 1 : 0;
		sbyte <<= 2;
	}
	vid_dot_half(vid, cola);
	vid_dot_half(vid, colb);
}

// color mode
// 2bit = 1dot
// 256 x 256
// line size = 256/4 = 64bytes
// pal: 0-black, 1-red, 2-green 3-blue
void bk_col_dot(Video* vid) {
	if (vid->hbrd || vid->vbrd) {
		cola = 0;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((vid->ray.x & 3) == 0) {
			yscr = vid->ray.y - vid->bord.y;
			xadr = 0x4000 | (yscr << 6) | ((xscr >> 2) & 0x1f);
			sbyte = vid->mrd(xadr, vid->data);
		}
		cola = (sbyte & 0xc0) >> 6;
		sbyte <<= 2;
	}
	vid_dot_full(vid, cola);
}
