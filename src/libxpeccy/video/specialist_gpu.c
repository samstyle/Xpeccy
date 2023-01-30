#include "video.h"

static int adr;
static int scrbyte;
static unsigned char col;

void spcv_ini(Video* vid) {
	xColor blk = {0,0,0};
	xColor wht = {255,255,255};
	vid_set_col(vid, 0, blk);
	vid_set_col(vid, 1, wht);
}

void spc_dot(Video* vid) {
	if ((vid->ray.x & 7) == 0) {					// every 8 dots
		adr = (vid->ray.y & 0xff) | (vid->ray.x << 5);		// 0x9000 + y + (x / 8 * 256)
		scrbyte = vid->mrd(adr, vid->xptr);
	}
	col = (scrbyte & 0x80) ? 1 : 0;
	scrbyte <<= 1;
	vid_dot_full(vid, col);
}
