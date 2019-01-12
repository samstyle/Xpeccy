#include "video.h"

#include <string.h>

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
	if (vid->hbrd || vid->vbrd || (vid->cutscr && (vid->ray.ys > 0x3f))) {
		cola = 0;
		colb = 0;
	} else {
		xscr = vid->ray.xs;
		if ((vid->ray.x & 3) == 0) {
			yscr = (vid->ray.ys + vid->sc.y - 0xd8) & 0xff;
			xadr = ((vid->curscr ? 7 : 1) << 14) | (yscr << 6) | ((xscr >> 2) & 0x3f);
			if (vid->cutscr) xadr |= 0x3000;
			sbyte = vid->mrd(xadr, vid->data);
		}
		cola = (sbyte & 0x01) ? 0x15 : 0x14;
		colb = (sbyte & 0x02) ? 0x15 : 0x14;
		sbyte >>= 2;
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
	if (vid->hbrd || vid->vbrd || (vid->cutscr && (vid->ray.ys > 0x3f))) {
		cola = 0;
	} else {
		xscr = vid->ray.xs;
		if ((vid->ray.x & 3) == 0) {
			yscr = (vid->ray.ys + vid->sc.y - 0xd8) & 0xff;
			xadr = ((vid->curscr ? 7 : 1) << 14) | (yscr << 6) | ((xscr >> 2) & 0x3f);
			if (vid->cutscr) xadr |= 0x3000;
			sbyte = vid->mrd(xadr, vid->data);
		}
		cola = sbyte & 3;
		sbyte >>= 2;
	}
	vid_dot_full(vid, cola | vid->paln);
}
