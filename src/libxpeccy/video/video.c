#include <stdio.h>
#include <string.h>
#include <math.h>

#include <assert.h>

#include "video.h"

#if defined(USEOPENGL)
#define SCRBUF_SIZE	2048*768*4
#else
#define SCRBUF_SIZE	3700*2050*4
#endif
#define DRAWING_F	1

int bytesPerLine = 768;
int greyScale = 0;
int noflic = 0;
int scanlines = 0;

static unsigned char bufa[SCRBUF_SIZE];
static unsigned char bufb[SCRBUF_SIZE];
unsigned char* scrimg = bufa;			// current screen (raw/bw)
unsigned char* bufimg = bufb;			// previous screen (mixed)
static int curbuf = 0;
int bufSize = 3;

int xstep = 0x100;
int ystep = 0x100;
int lefSkip = 0;
int pixSkip = 0;
int rigSkip = 0;
int topSkip = 0;
int botSkip = 0;

unsigned char pscr[SCRBUF_SIZE];		// previous screen (raw)

#if !defined(USEOPENGL)
static int xpos = 0;
static int ypos = 0;
#elif !DRAWING_F
// static unsigned char pcol;
#endif

typedef void(*cbdot)(Video*, unsigned char);

int vid_visible(Video* vid) {
	if (vid->ray.y < vid->lcut.y) return 0;
	if (vid->ray.y >= vid->rcut.y) return 0;
	if (vid->ray.x < vid->lcut.x) return 0;
	if (vid->ray.x >= vid->rcut.x) return 0;
	return 1;
}

static int32_t outcol;

inline void vid_dot_full(Video* vid, unsigned char idx) {
	if (vid->hvis && vid->vvis) {
		outcol = greyScale ? vid->gpal[idx] : vid->pal[idx];
#if defined(USEOPENGL)
		*(int32_t*)(vid->ray.ptr) = outcol;
		vid->ray.ptr += 4;
		*(int32_t*)(vid->ray.ptr) = outcol;
		vid->ray.ptr += 4;
#else
		xpos += xstep;
		while (xpos > 0xff) {
			xpos -= 0x100;
			*(int32_t*)(vid->ray.ptr) = outcol;
			vid->ray.ptr += 4;
		}
#endif
	}
}

inline void vid_dot_half(Video* vid, unsigned char idx) {
	if (vid->hvis && vid->vvis) {
		outcol = greyScale ? vid->gpal[idx] : vid->pal[idx];
#if defined(USEOPENGL)
		*(int32_t*)(vid->ray.ptr) = outcol;
		vid->ray.ptr += 4;
#else
		xpos += xstep/2;
		while (xpos > 0xff) {
			xpos -= 0x100;
			*(int32_t*)(vid->ray.ptr) = outcol;
			vid->ray.ptr += 4;
		}
#endif
	}
}

static int blkcol = 0xff000000;

void vid_fill_black(unsigned char* ptr, int len) {
	while (len > 0) {
		*(int32_t*)(ptr) = blkcol;
		ptr += sizeof(int32_t);
		len -= sizeof(int32_t);
	}
}

void vid_line(Video* vid) {
	if (rigSkip > 0)
		vid_fill_black(vid->ray.ptr, rigSkip);
	if (vid->linedbl) {
		memcpy(vid->ray.lptr+bytesPerLine, vid->ray.lptr, bytesPerLine);
		vid->ray.lptr += bytesPerLine;
	}
	vid->ray.lptr += bytesPerLine;
#if !defined(USEOPENGL)
	ypos += ystep;
	if (vid->linedbl) ypos += ystep;
	ypos -= 0x100;		// 1 line is already drawn
	xpos = 0;
	if (scanlines) {
		int x;
		double rs = 1.0 - 128.0 / ystep;
		while (ypos > 0) {
			ypos -= 0x100;
			for (x = 0; x < bytesPerLine; x++) {
				*(vid->ray.lptr + x) = *(vid->ray.lptr + x - bytesPerLine) * rs;
			}
			vid->ray.lptr += bytesPerLine;
		}
	} else {
		while (ypos > 0) {
			ypos -= 0x100;
			memcpy(vid->ray.lptr, vid->ray.lptr - bytesPerLine, bytesPerLine);
			vid->ray.lptr += bytesPerLine;
		}
	}
#endif
	if (lefSkip > 0)
		vid_fill_black(vid->ray.lptr, bytesPerLine);
	vid->ray.ptr = vid->ray.lptr + lefSkip;
}

void vid_line_fill(Video* vid) {
	// TODO: apply scanlines here
#if !defined(USEOPENGL)
	int ytmp = ypos;
	unsigned char* ptr = vid->ray.lptr;
	ytmp += ystep;
	while (ytmp > 0x100) {
		ytmp -= 0x100;
		memcpy(ptr + bytesPerLine, ptr, bytesPerLine);
		ptr += bytesPerLine;
	}
#endif
}

void vid_frame(Video* vid) {
	if (botSkip > 0) {
		vid_fill_black(vid->ray.lptr, botSkip * bytesPerLine);
	}
// scanlines TODO: bad @ fullscreen
#if !defined(USEOPENGL)
	ypos = 0;
	/*
	int x,y,ys;
	double p, ps;
	unsigned char* ptr;
	if (scanlines) {
		ptr = scrimg;
		p = 0.0;
		ps = 256.0 / ystep;
		ys = 0;
		y = 0;
		while (y < vid->vsze.y) {
			for (x = 0; x < bytesPerLine; x++)
				*(ptr + x) = *(ptr + x) * (1.0 - p / 2);
			ptr += bytesPerLine;
			p += ps;
			if (p >= 1.0) {
				p = 0.0;
				y++;
			}
		}
	}
*/
#endif
	if (!vid->debug) {
		scrimg = curbuf ? bufb : bufa;
		bufimg = curbuf ? bufa : bufb;
		curbuf = !curbuf;
	}
	if (topSkip > 0) {
		vid_fill_black(scrimg, topSkip * bytesPerLine);
	}
	vid->ray.lptr = scrimg + topSkip * bytesPerLine;
	if (lefSkip > 0)
		vid_fill_black(vid->ray.lptr, lefSkip);
	vid->ray.ptr = vid->ray.lptr + lefSkip;
	vid->newFrame = 1;
	vid->xirq(IRQ_VID_FRAME, vid->xptr);
}

Video* vidCreate(cbxrd cb, cbirq ci, void* dptr) {
	Video* vid = (Video*)malloc(sizeof(Video));
	memset(vid,0x00,sizeof(Video));
	vid->mrd = cb;
	vid->xirq = ci;
	vid->xptr = dptr;
	vid->nsPerDot = 150;
	vid->res.x = -1;
	vid->res.y = -1;
	vid_set_mode(vid, VID_UNKNOWN);
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	vid_set_layout(vid, &vlay);
	vid->inten = 0x01;		// FRAME INT for all

	vid->ula = ula_create();

	vid_set_border(vid, 0.5);

	vid->brdstep = 1;
	vid->nextbrd = 0;
	vid->curscr = 5;
	vid->fcnt = 0;

	vid->nsDraw = 0;
	vid->ray.x = 0;
	vid->ray.y = 0;
	vid->idx = 0;

	vid->ray.ptr = scrimg;
	vid->ray.lptr = scrimg;

	return vid;
}

void vidDestroy(Video* vid) {
	ula_destroy(vid->ula);
	free(vid);
}

void vid_upd_timings(Video* vid, int nspd) {
	vid->nsPerDot = nspd;
	vid->nsPerLine = vid->nsPerDot * vid->full.x;
	vid->nsPerFrame = vid->nsPerLine * vid->full.y;
#ifdef ISDEBUG
	// printf("%i / %i / %i\n", vid->nsPerDot, vid->nsPerLine, vid->nsPerFrame);
#endif
}

// TODO: for zx only?
void vid_reset(Video* vid) {
	int i;
	for (i = 0; i<16; i++) {
		vid_reset_col(vid, i);
	}
	vid->ula->active = 0;
	vid->curscr = 5;
	vid->nsDraw = 0;
//	vidSetMode(vid, VID_NORMAL);
}

// move ray to 1 dot before INT
void vid_reset_ray(Video* vid) {
/*
	vid->ray.x = vid->intp.x - 1;
	vid->ray.y = vid->intp.y;
	if (vid->ray.x < 0) {
		vid->ray.x += vid->full.x;
		vid->ray.y--;
		if (vid->ray.y < 0)
			vid->ray.y += vid->full.y;
	}
*/
	vid_set_ray(vid, -1);
}

void vid_set_ray(Video* vid, int dots) {
	dots += vid->full.x * vid->intp.y;
	dots += vid->intp.x;
	dots %= vid->dotPerFrame;
	vid->ray.y = dots / vid->full.x;
	vid->ray.x = dots % vid->full.x;
	vid->ray.ptr = scrimg + (dots * 6);
}

// new layout:
// [ bord ][ scr ][ ? ][ blank ]
// [ <--------- full --------> ]
// ? = brdr = full - bord - scr - blank
void vid_upd_layout(Video* vid) {
	vCoord brdr;	// size of right/bottom border parts
	vid->vend.x = vid->full.x - vid->blank.x;		// visible right dot
	vid->vend.y = vid->full.y - vid->blank.y;		// visible bottom line
	brdr.x = vid->vend.x - vid->bord.x - vid->scrn.x;	// border right
	brdr.y = vid->vend.y - vid->bord.y - vid->scrn.y;	// border bottom
	vid->lcut.x = vid->bord.x * (1.0 - vid->brdsize);	// cut border left
	vid->lcut.y = vid->bord.y * (1.0 - vid->brdsize);	// cut border top
	vid->rcut.x = vid->bord.x + vid->scrn.x + brdr.x * vid->brdsize;	// cut border right
	vid->rcut.y = vid->bord.y + vid->scrn.y + brdr.y * vid->brdsize;	// cut border bottom
	vid->vsze.x = vid->rcut.x - vid->lcut.x;		// visible area size
	vid->vsze.y = vid->rcut.y - vid->lcut.y;
	vid->send.x = vid->bord.x + vid->scrn.x;		// screen end column
	vid->send.y = vid->bord.y + vid->scrn.y;		// screen end line
	vid->vBytes = vid->vsze.x * vid->vsze.y * 8;		// real size of image buffer (4 bytes/dot x2:x1)
	vid->dotPerFrame = vid->full.y * vid->full.x;
	vid_upd_timings(vid, vid->nsPerDot);
}

void vid_set_layout(Video* vid, vLayout* lay) {
	vid->full = lay->full;
	vid->bord = lay->bord;
	vid->blank = lay->blank;
	vid->scrn = lay->scr;
	vid->intp = lay->intpos;
	vid->intsize = lay->intSize;
	vid->frmsz = lay->full.x * lay->full.y;
	vid_upd_layout(vid);
}

// set visible area size
void vid_set_resolution(Video* vid, int w, int h) {
	if ((vid->vsze.y == h) && (vid->vsze.x == w)) return;
	if ((vid->vsze.x <= 0) || (vid->vsze.y <= 0)) return;
	printf("vid_set_resolution %i x %i\n",w,h);
	vid->res.x = w;
	vid->res.y = h;
	vid->scrn = vid->res;
	vid->blank.y = vid->full.y - h;
	vid->blank.x = vid->full.x - w;
	vid_upd_layout(vid);
	vid->upd = 1;
}

void vid_set_border(Video* vid, double brd) {
	if (brd < 0.0) brd = 0.0;
	else if (brd > 1.0) brd = 1.0;
	vid->brdsize = brd;
	vid_upd_layout(vid);
}

static int xscr = 0;
static int yscr = 0;
static int adr = 0;
static unsigned char col = 0;
static unsigned char ink = 0;
static unsigned char pap = 0;
static unsigned char scrbyte = 0;
static unsigned char nxtbyte = 0;
static unsigned char nxtatr = 0;

void vid_dark_tail(Video* vid) {
	if (vid->tail) return;				// no filling while current fill is active (till end of frame)
	unsigned char* ptr = vid->ray.ptr;		// fill current line till EOL
	unsigned char* zptr = bufimg + (vid->ray.ptr - scrimg); // ptr to prev.frame (place is same as ray at cur.frame)
	unsigned char* btr = scrimg;			// begin of current buffer
	// current line
	while (ptr - vid->ray.lptr < bytesPerLine) {	// dark tail from prev.frame to cur.frame
		*ptr = ((*zptr - 0x80) >> 2) + 0x80;
		zptr++;
		ptr++;
	}
#if !defined(USEOPENGL)
	// copy current line
	vid_line_fill(vid);				// copy filled line due zoom value
	int ytmp = ypos + ystep;
	// ptr = vid->ray.lptr;				// move ptr to start of next real line
	while(ytmp > 0x100) {
		ytmp -= 0x100;
		ptr += bytesPerLine;
		zptr += bytesPerLine;
	}
#else
	//ptr = vid->ray.lptr + bytesPerLine;		// already
#endif
// fill all till end
	while (ptr - btr < bufSize) {
		*ptr = ((*zptr - 0x80) >> 2) + 0x80;
		zptr++;
		ptr++;
	}
	vid->tail = 1;
}

void vid_dark_all() {
	unsigned char* ptr = scrimg;
	int len = bufSize;
	while (len > 0) {
		*ptr = ((*ptr - 0x80) >> 2) + 0x80;
		ptr++;
		len--;
	}
}

//const unsigned char emptyBox[8] = {0x00,0x18,0x3c,0x7e,0x7e,0x3c,0x18,0x00};
//const unsigned char emptyBox[8] = {0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x81};
static unsigned char emptyBox[8] = {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00};

void vid_get_screen(Video* vid, unsigned char* dst, int bank, int shift, int flag) {
	if ((bank == 0xff) && (shift > 0x2800)) shift = 0x2800;
	int pixadr = MADR(bank, shift);
	int atradr = pixadr + 0x1800;
	unsigned char sbyte, abyte, aink, apap;
	int prt, lin, row, xpos, bitn, cidx;
	int sadr, aadr;
	unsigned char cr,cg,cb;
	for (prt = 0; prt < 3; prt++) {
		for (lin = 0; lin < 8; lin++) {
			for (row = 0; row < 8; row++) {
				for (xpos = 0; xpos < 32; xpos++) {
					sadr = (prt << 11) | (lin << 5) | (row << 8) | xpos;
					aadr = (prt << 8) | (lin << 5) | xpos;
					sbyte = (flag & 2) ? emptyBox[row] : vid->mrd(pixadr + sadr, vid->xptr);
					abyte = (flag & 1) ? 0x47 : vid->mrd(atradr + aadr, vid->xptr);
					aink = (abyte & 0x07) | ((abyte & 0x40) >> 3);
					apap = (abyte & 0x78) >> 3;
					for (bitn = 0; bitn < 8; bitn++) {
						cidx = (sbyte & (128 >> bitn)) ? aink : apap;
						cb = (cidx & 1) ? ((cidx & 8) ? 0xff : 0xa0) : 0x00;
						cr = (cidx & 2) ? ((cidx & 8) ? 0xff : 0xa0) : 0x00;
						cg = (cidx & 4) ? ((cidx & 8) ? 0xff : 0xa0) : 0x00;
						if ((flag & 4) && ((lin ^ xpos) & 1)) {
							*(dst++) = ((cr - 0x80) >> 1) + 0x80;
							*(dst++) = ((cg - 0x80) >> 1) + 0x80;
							*(dst++) = ((cb - 0x80) >> 1) + 0x80;
						} else {
							*(dst++) = cr;
							*(dst++) = cg;
							*(dst++) = cb;
						}
					}
				}
			}
		}
	}
}

// ula 5c/6c horizontal timings:
// 0	255	screen
// 256	319	right border (64)
// 320	415	HBlank (96)
// 416	447	left border (32)
// int @ 64 lines above screen

// ula vertical timings
// 0	191	screen
// 192	247	bottom border (56)
// 248	255	VBlank (8)
// 256	311	top border (56)

// NOTE: waiting cycle starts 8 dots before screen?
// (T14336 here), 4dots pre-wait, 2dots ula read pixels, 2dots ula read attr, (T14340:output starts here), 2 dots ula read next pixels, 2 dots ula read next atr, 4 no-wait dots
// ^ repeat 16 times each 16 dots in each of 192 screen rows

static int contTabA[] = {12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0};		// 48K 128K +2 (bank 1,3,5,7)
static int contTabB[] = {2,1,0,0,14,13,12,11,10,9,8,7,6,5,4,3};	// +2A +3 (bank 4,5,6,7)

int vid_wait(Video* vid, int adr) {
	int* contTab = NULL;
	switch (vid->ula->conttype) {
		case CONT_PATA:
			adr &= 0x4000;			// pages 1,3,5,7
			contTab = contTabA;
			break;
		case CONT_PATB:				// pages 4,5,6,7
			adr &= 0x10000;
			contTab = contTabB;
			break;
	}
	if (!contTab) return 0;				// unknown patern
	if (!adr) return 0;
	if (vid->ray.y < vid->bord.y) return 0;				// above screen
	if (vid->ray.y >= (vid->bord.y + vid->scrn.y)) return 0;	// below screen
	xscr = vid->ray.x - vid->bord.x + 8;
	if (xscr < 0) return 0;						// line before contention
	if (xscr >= vid->scrn.x) return 0;				// line after contention
	return contTab[xscr & 0x0f] * vid->nsPerDot;			// return time (ns)
}

void vid_set_grey(int f) {
	greyScale = f;
}

// palette

xColor vid_get_col(Video* vid, int i) {
	xColor xcol;
	i = vid->pal[i & 0xff];
	xcol.r = i & 0xff;
	xcol.g = (i >> 8) & 0xff;
	xcol.b = (i >> 16) & 0xff;
	return xcol;
}

void vid_set_col(Video* vid, int i, xColor xcol) {
	vid->pal[i & 0xff] = xcol.r | (xcol.g << 8) | (xcol.b << 16) | (0xff << 24);
	outcol = (xcol.b * 30 + xcol.r * 76 + xcol.g * 148) >> 8;
	vid->gpal[i & 0xff] = outcol | (outcol << 8) | (outcol << 16) | (0xff << 24);
}

// set base color palette (used for preset loading)
void vid_set_bcol(Video* vid, int i, xColor xcol) {
	vid->bpal[i & 0xff] = xcol.r | (xcol.g << 8) | (xcol.b << 16) | (0xff << 24);
}

// set current palette color from preloaded preset
void vid_reset_col(Video* vid, int i) {
	vid->pal[i & 0xff] = vid->bpal[i & 0xff];
}

// video drawing

void vidDrawBorder(Video* vid) {
	vid_dot_full(vid, vid->brdcol);
}

// ZX Screen 256 x 192
void vidDrawNormal(Video* vid) {
	if (vid->vbrd) {
		col = vid->brdcol;
		if (vid->ula->active) col |= 8;
		vid->atrbyte = 0xff;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		yscr = vid->ray.y - vid->bord.y;
		if ((xscr & 7) == 3) {
			adr = (vid->idx & 0x181f) | ((vid->idx & 0x700) >> 3) | ((vid->idx & 0xe0) << 3);
			nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
		}
		if (vid->hbrd) {
			col = vid->brdcol;
			if (vid->ula->active) col |= 8;
			vid->atrbyte = 0xff;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = 0x1800 | ((vid->idx & 0x1f00) >> 3) | (vid->idx & 0x1f);
				vid->atrbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				if (vid->idx < 0x1b00) vid->idx++;
				if (vid->ula->active) {
					ink = ((vid->atrbyte & 0xc0) >> 2) | (vid->atrbyte & 7);
					pap = ((vid->atrbyte & 0xc0) >> 2) | ((vid->atrbyte & 0x38) >> 3) | 8;
				} else {
					if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
					ink = (vid->atrbyte & 0x07) | ((vid->atrbyte & 0x40) >> 3);
					pap = (vid->atrbyte & 0x78) >> 3;
				}
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vid_dot_full(vid, col);
}

// this mode default for ZX48K ULA (defferent moments of pix/atr read)
void ula_dot(Video* vid) {
	if (vid->vbrd) {
		col = vid->brdcol;
		if (vid->ula->active) col |= 8;
		vid->atrbyte = 0xff;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		yscr = vid->ray.y - vid->bord.y;
		switch(xscr & 15) {
			case 12:
				adr = (vid->idx & 0x181f) | ((vid->idx & 0x700) >> 3) | ((vid->idx & 0xe0) << 3);
				nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				break;		// 4dots before each even box: box pix
			case 14:
				adr = 0x1800 | ((vid->idx & 0x1f00) >> 3) | (vid->idx & 0x1f);
				nxtatr = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				break;		// 2dots before each even box: box atr
			case 0:
				scrbyte = nxtbyte;
				vid->atrbyte = nxtatr;
				vid->idx++;		// lame (idx is still not updated, but we need address of next box)
				adr = (vid->idx & 0x181f) | ((vid->idx & 0x700) >> 3) | ((vid->idx & 0xe0) << 3);
				nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				vid->idx--;
				break;		// start of even box: next (odd) box pix
			case 1:
				adr = 0x1800 | ((vid->idx & 0x1f00) >> 3) | (vid->idx & 0x1f);
				nxtatr = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				break;		// 2nd dot of even box: next (odd) box atr
			case 8:
				scrbyte = nxtbyte;
				vid->atrbyte = nxtatr;
				break;		// odd box start
		}
		if (vid->hbrd) {
			col = vid->brdcol;
			if (vid->ula->active) col |= 8;
			vid->atrbyte = 0xff;
		} else {
			if ((xscr & 7) == 0) {
				if (vid->idx < 0x1b00) vid->idx++;
				if (vid->ula->active) {
					ink = ((vid->atrbyte & 0xc0) >> 2) | (vid->atrbyte & 7);
					pap = ((vid->atrbyte & 0xc0) >> 2) | ((vid->atrbyte & 0x38) >> 3) | 8;
				} else {
					if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
					ink = (vid->atrbyte & 0x07) | ((vid->atrbyte & 0x40) >> 3);
					pap = (vid->atrbyte & 0x78) >> 3;
				}
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vid_dot_full(vid, col);
}

// alco 16col
void vidDrawAlco(Video* vid) {
	if (vid->vbrd || vid->hbrd) {
		col = vid->brdcol;
	} else {
		yscr = vid->ray.y - vid->bord.y;
		xscr = vid->ray.x - vid->bord.x;
//		if ((xscr < 0) || (xscr > 255)) {
//			col = vid->brdcol;
//		} else {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
			switch (xscr & 7) {
				case 0:
					scrbyte = vid->mrd(MADR(vid->curscr ^ 1, adr), vid->xptr);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				case 2:
					scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				case 4:
					scrbyte = vid->mrd(MADR(vid->curscr ^ 1, adr + 0x2000), vid->xptr);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				case 6:
					scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->xptr);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				default:
					col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
					break;

			}
//		}
	}
	vid_dot_full(vid, col);
}

// hardware multicolor
void vidDrawHwmc(Video* vid) {
	if (vid->vbrd) {
		col = vid->brdcol;
	} else {
		xscr = vid->ray.x - vid->bord.x;
		yscr = vid->ray.y - vid->bord.y;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
		}
		if (vid->hbrd) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
				vid->atrbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = (vid->atrbyte & 0x07) | ((vid->atrbyte & 0x40) >> 3);
				pap = (vid->atrbyte & 0x78) >> 3;
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vid_dot_full(vid, col);
}

// atm ega
void vidDrawATMega(Video* vid) {
	yscr = vid->ray.y - 76 + 32;	// ???
	xscr = vid->ray.x - 96 + 64;
	if ((yscr < 0) || (yscr > 199) || (xscr < 0) || (xscr > 319)) {
		col = vid->brdcol;
	} else {
		adr = (yscr * 40) + (xscr >> 3);
		switch (xscr & 7) {
			case 0:
				scrbyte = vid->mrd(MADR(vid->curscr ^ 4, adr), vid->xptr) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3); // inkTab[scrbyte & 0x7f];
				break;
			case 2:
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
				break;
			case 4:
				scrbyte = vid->mrd(MADR(vid->curscr ^ 4, adr + 0x2000), vid->xptr) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
				break;
			case 6:
				scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->xptr) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
				break;
			default:
				col = ((scrbyte & 0x38) >> 3) | ((scrbyte & 0x80) >> 4);
				break;
		}
	}
	vid_dot_full(vid, col);
}

// atm text

void vidDrawByteDD(Video* vid) {		// draw byte $scrbyte with colors $ink,$pap at double-density mode
	for (int i = 0x80; i > 0; i >>= 1) {
		vid_dot_half(vid, (scrbyte & i) ? ink : pap);
	}
}

void vidATMDoubleDot(Video* vid,unsigned char colr) {
	ink = (colr & 0x07) | ((colr & 0x40) >> 3);
	pap = ((colr & 0x38) >> 3) | ((colr & 0x80) >> 4);
	vidDrawByteDD(vid);
}

void vidDrawATMtext(Video* vid) {
	yscr = vid->ray.y - 76 + 32;
	xscr = vid->ray.x - 96 + 64;
	if ((yscr < 0) || (yscr > 199) || (xscr < 0) || (xscr > 319)) {
		vid_dot_full(vid, vid->brdcol);
	} else {
		adr = 0x1c0 + ((yscr & 0xf8) << 3) + (xscr >> 3);
		if ((xscr & 3) == 0) {
			if ((xscr & 7) == 0) {
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr) & 0xff;
				col = vid->mrd(MADR(vid->curscr ^ 4, adr ^ 0x2000), vid->xptr) & 0xff;
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr, adr ^ 0x2000), vid->xptr) & 0xff;
				col = vid->mrd(MADR(vid->curscr ^ 4, adr + 1), vid->xptr) & 0xff;
			}
			scrbyte = vid->font[(scrbyte << 3) | (yscr & 7)];
			vidATMDoubleDot(vid,col);
		}
	}
}

// atm hardware multicolor
void vidDrawATMhwmc(Video* vid) {
	yscr = vid->ray.y - 76 + 32;
	xscr = vid->ray.x - 96 + 64;
	if ((yscr < 0) || (yscr > 199) || (xscr < 0) || (xscr > 319)) {
		vid_dot_full(vid, vid->brdcol);
	} else {
		//xscr = vid->ray.x - 96;
		//yscr = vid->ray.y - 76;
		adr = (yscr * 40) + (xscr >> 3);
		if ((xscr & 3) == 0) {
			if ((xscr & 7) == 0) {
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->xptr);
				col = vid->mrd(MADR(vid->curscr ^ 4, adr), vid->xptr);
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->xptr);
				col = vid->mrd(MADR(vid->curscr ^ 4, adr + 0x2000), vid->xptr);
			}
			vidATMDoubleDot(vid,col);
		}
		//vid->ray.ptr++;
		//if (vidFlag & VF_DOUBLE) vid->ray.ptr++;
	}
}

// baseconf text

void vidDrawEvoText(Video* vid) {
	yscr = vid->ray.y - 76;
	xscr = vid->ray.x - 96;
	if ((yscr < 0) || (yscr > 199) || (xscr < 0) || (xscr > 319)) {
		vid_dot_full(vid, vid->brdcol);
	} else {
		if ((xscr & 3) == 0) {
			adr = 0x1c0 + ((yscr & 0xf8) << 3) + (xscr >> 3);
			if ((xscr & 7) == 0) {
				scrbyte = vid->mrd(MADR(vid->curscr + 3, adr), vid->xptr);
				col = vid->mrd(MADR(vid->curscr + 3, adr + 0x3000), vid->xptr);
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr + 3, adr + 0x1000), vid->xptr);
				col = vid->mrd(MADR(vid->curscr + 3, adr + 0x2001), vid->xptr);
			}
			scrbyte = vid->font[(scrbyte << 3) | (yscr & 7)];
			vidATMDoubleDot(vid,col);
		}
	}
}

// profi 512x240

void vidProfiScr(Video* vid) {
	yscr = vid->ray.y - vid->bord.y + 24;	// (240-192)/2 = 24
	if ((yscr < 0) || (yscr > 239)) {
		vid_dot_full(vid, vid->brdcol);
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			vid_dot_full(vid, vid->brdcol);
		} else {
			if ((xscr & 3) == 0) {
				//adr = scrAdrs[vid->idx & 0x1fff] & 0x1fff;
				adr = (vid->idx & 0x181f) | ((vid->idx & 0x700) >> 3) | ((vid->idx & 0xe0) << 3);
				if (xscr & 4) {
					vid->idx++;
				} else {
					adr |= 0x2000;
				}
				if (vid->curscr == 7) {
					scrbyte = vid->mrd(MADR(6, adr), vid->xptr);
					col = vid->mrd(MADR(0x3a, adr), vid->xptr);		// b0..2 ink, b3..5 pap, b6 inkBR, b7 papBR
				} else {
					scrbyte = vid->mrd(MADR(4, adr), vid->xptr);
					col = vid->mrd(MADR(0x38, adr), vid->xptr);
				}
				ink = (col & 0x07) | ((col & 0x40) >> 3);
				pap = (col & 0x78) >> 3;
				vidDrawByteDD(vid);
			}
		}
	}
}

// tsconf

void vidDrawTSLNormal(Video*);
void vidDrawTSLExt(Video*);
void vidTSline(Video*);
void vidDrawTSLText(Video*);
void vidDrawEvoText(Video*);

// c64 vic-II

void vidC64TDraw(Video*);
void vidC64TMDraw(Video*);
void vidC64BDraw(Video*);
void vidC64BMDraw(Video*);
void vidC64Line(Video*);
void vidC64Fram(Video*);

// debug

void vidBreak(Video* vid) {
	printf("vid->mode = 0x%.2X\n",vid->vmode);
	// assert(0);
}

// bk

void bk_bw_dot(Video*);
void bk_col_dot(Video*);

// specialist

void spc_dot(Video*);
void spcv_ini(Video*);

// vga

void cga_t40_frm(Video*);
void cga_t40_line(Video*);
void cga320_2bpp_line(Video*);
void cga640_1bpp_line(Video*);
void vga320_4bpp_line(Video*);
void vga640_4bpp_line(Video*);
void vga256_line(Video*);
void cga_t40_dot(Video*);
void cga_lores_dot(Video*);
void ega_hires_dot(Video*);
void cga_t40_ini(Video*);
void cga_t80_ini(Video*);
void vga_glo_ini(Video*);
void vga_ghi_ini(Video*);

// weiter

typedef struct {
	int id;
	void(*init)(Video*);
	void(*cbDot)(Video*);		// each dot
	void(*cbHBlank)(Video*);	// hblank start
	void(*cbLine)(Video*);		// visible line start
	void(*cbFrame)(Video*);		// vblank start
} xVideoMode;

// id,(@on),(@every_visible_dot),(@HBlank),(@LineStart),(@VBlank)
static xVideoMode vidModeTab[] = {
	{VID_NORMAL, NULL, vidDrawNormal, NULL, NULL, NULL},
	{VID_ULA_SCR, NULL, ula_dot, NULL, NULL, NULL},
	{VID_ALCO, NULL, vidDrawAlco, NULL, NULL, NULL},
	{VID_HWMC, NULL, vidDrawHwmc, NULL, NULL, NULL},
	{VID_ATM_EGA, NULL, vidDrawATMega, NULL, NULL, NULL},
	{VID_ATM_TEXT, NULL, vidDrawATMtext, NULL, NULL, NULL},
	{VID_ATM_HWM, NULL, vidDrawATMhwmc, NULL, NULL, NULL},
	{VID_EVO_TEXT, NULL, vidDrawEvoText, NULL, NULL, NULL},
	{VID_TSL_NORMAL, NULL, vidDrawTSLNormal, vidTSline, NULL, NULL},
	{VID_TSL_16, NULL, vidDrawTSLExt, vidTSline, NULL, NULL},			// vidDrawTSL16
	{VID_TSL_256, NULL, vidDrawTSLExt, vidTSline, NULL, NULL},			// vidDrawTSL256
	{VID_TSL_TEXT, NULL, vidDrawTSLText, vidTSline, NULL, NULL},
	{VID_PRF_MC, NULL, vidProfiScr, NULL, NULL, NULL},

	{VID_GBC, NULL, gbcvDraw, gbcvLine, NULL, gbcvFram},
	{VID_NES, NULL, ppuDraw, ppuHBL, ppuLine, ppuFram},

	{VDP_TEXT1, NULL, vdpText1, vdpHBlk, NULL, NULL},
	{VDP_TEXT2, NULL, vdpDummy, vdpHBlk, NULL, NULL},
	{VDP_MCOL, NULL, vdpMultcol, vdpHBlk, vdp_line, NULL},
	{VDP_GRA1, NULL, vdpGra1, vdpHBlk, vdp_line, NULL},
	{VDP_GRA2, NULL, vdpGra2, vdpHBlk, vdp_line, NULL},
	{VDP_GRA3, NULL, vdpGra2, vdpHBlk, vdp_linex, NULL},
	{VDP_GRA4, NULL, vdpGra4, vdpHBlk, vdp_linex, NULL},
	{VDP_GRA5, NULL, vdpGra5, vdpHBlk, vdp_linex, NULL},
	{VDP_GRA6, NULL, vdpGra6, vdpHBlk, vdp_linex, NULL},
	{VDP_GRA7, NULL, vdpGra7, vdpHBlk, vdp_linex, NULL},

	{VID_C64_TEXT, NULL, vidC64TDraw, NULL, vidC64Line, vidC64Fram},
	{VID_C64_TEXT_MC, NULL, vidC64TMDraw, NULL, vidC64Line, vidC64Fram},
	{VID_C64_BITMAP, NULL, vidC64BDraw, NULL, vidC64Line, vidC64Fram},
	{VID_C64_BITMAP_MC, NULL, vidC64BMDraw, NULL, vidC64Line, vidC64Fram},

	{VID_BK_BW, NULL, bk_bw_dot, NULL, NULL, NULL},
	{VID_BK_COL, NULL, bk_col_dot, NULL, NULL, NULL},

	{VID_SPCLST, spcv_ini, spc_dot, NULL, NULL, NULL},

	{CGA_TXT_L, cga_t40_ini, cga_lores_dot, NULL, cga_t40_line, cga_t40_frm},
	{CGA_TXT_H, cga_t80_ini, cga_t40_dot, NULL, cga_t40_line, cga_t40_frm},
	{CGA_GRF_L, NULL, cga_lores_dot, NULL, cga320_2bpp_line, cga_t40_frm},
	{CGA_GRF_H, NULL, cga_t40_dot, NULL, cga640_1bpp_line, cga_t40_frm},
	{VGA_GRF_L, vga_glo_ini, cga_t40_dot, NULL, vga320_4bpp_line, cga_t40_frm},
	{VGA_GRF_H, vga_ghi_ini, cga_t40_dot, NULL, vga640_4bpp_line, cga_t40_frm},
	{VGA_GRF_256, vga_glo_ini, cga_lores_dot, NULL, vga256_line, cga_t40_frm},

	{VID_UNKNOWN, NULL, vidDrawBorder, NULL, NULL, NULL}
};

void vid_set_mode(Video* vid, int mode) {
	vid->vmode = mode;
	int i = 0;
	while ((vidModeTab[i].id != VID_UNKNOWN) && (vidModeTab[i].id != mode)) {
		i++;
	}
	vid->cbDot = vid->noScreen ? vidDrawBorder : vidModeTab[i].cbDot;
	vid->cbHBlank = vidModeTab[i].cbHBlank;
	vid->cbFrame = vidModeTab[i].cbFrame;
	vid->cbLine = vidModeTab[i].cbLine;
	if (vidModeTab[i].init)
		vidModeTab[i].init(vid);
}

void vid_tick(Video* vid) {
	if ((vid->ray.x & vid->brdstep) == 0)
		vid->brdcol = vid->nextbrd;

#if DRAWING_F
	if (vid->cbDot)
		vid->cbDot(vid);
#else
	// if ray is on visible screen & video has drawing callback...
	if ((vid->ray.y >= vid->lcut.y) && (vid->ray.y < vid->rcut.y) \
		&& (vid->ray.x >= vid->lcut.x) && (vid->ray.x < vid->rcut.x) \
		&& vid->cbDot) {
			vid->cbDot(vid);		// put dot callback
	}
#endif
	// if debug, fill all line
	if (vid->debug)
		vid_line_fill(vid);
	// move ray to next dot, update counters
	vid->ray.x++;
	vid->ray.xb++;
	vid->ray.xs++;
	if (vid->ray.x >= vid->full.x) {			// new line
		vid->hblank = 0;
		vid->hbstrb = 0;
		vid->ray.x = 0;
		vid->ray.xs = -vid->bord.x;
		vid->ray.ys++;
		vid->lcnt++;
		if (vid->cbLine) vid->cbLine(vid);
	}
	if (vid->ray.x == vid->vend.x) {			// hblank start
		if ((vid->ray.y >= vid->lcut.y) && (vid->ray.y < vid->rcut.y))
			vid_line(vid);
		vid->hblank = 1;
		vid->hbstrb = 1;
		vid->ray.xb = 0;
		vid->ray.y++;
		vid->ray.yb++;
		if (vid->ray.y >= vid->full.y) {		// new frame
			vid_frame(vid);
			vid->lcnt = 0;
			vid->vblank = 0;
			vid->vbstrb = 0;
			vid->ray.y = 0;
			vid->tsconf.scrLine = 0;
			vid->fcnt++;
			vid->flash = (vid->fcnt & 0x10) ? 1 : 0;
			if (vid->cbFrame) vid->cbFrame(vid);
			vid->tail = 0;
			if (vid->debug)
				vid_dark_all();
		}
		if (vid->ray.y == vid->vend.y) {		// vblank start
			vid->xirq(IRQ_VID_VBLANK, vid->xptr);
			vid->vblank = 1;
			vid->vbstrb = 1;
			vid->ray.yb = 0;
			vid->idx = 0;
		}
		if (vid->ray.y == vid->lcut.y) {	// window visibility
			vid->vvis = 1;
		} else if (vid->ray.y == vid->rcut.y) {
			vid->vvis = 0;
		}
		if (vid->ray.y == vid->send.y) {	// screen end V
			vid->vbrd = 1;
		}
		if (vid->ray.y == vid->bord.y) {	// screen start V
			vid->vbrd = 0;
			vid->ray.ys = -1;		// will be 0 at start of next line, but during HBlank is -1
		}
		if (vid->cbHBlank) vid->cbHBlank(vid);
	}
	if (vid->ray.x == vid->lcut.x) {		// window visibility
		vid->hvis = 1;
	} else if (vid->ray.x == vid->rcut.x) {
		vid->hvis = 0;
	}
	if (vid->ray.x == vid->send.x) {		// screen end H
		vid->hbrd = 1;
	} else if (vid->ray.x == vid->bord.x) {		// screen start H
		vid->hbrd = 0;
	}
	// generate int
	if (vid->intFRAME) {
		vid->intFRAME--;
		if (!vid->intFRAME)
			vid->xirq(IRQ_VID_INT_E, vid->xptr);
	} else if ((vid->ray.yb == vid->intp.y) && (vid->ray.xb == vid->intp.x)) {
		vid->intTime = vid->time;
		// vid->intFRAME = vid->intsize;
		vid->xirq(IRQ_VID_INT, vid->xptr);
	}
	if (vid->busy > 0) vid->busy--;
	if (vid->inth > 0) vid->inth--;
	if (vid->intf > 0) vid->intf--;
}

void vid_sync(Video* vid, int ns) {
	vid->nsDraw += ns;
	while (vid->nsDraw >= vid->nsPerDot) {
		vid->nsDraw -= vid->nsPerDot;
		vid->time += vid->nsPerDot;
		vid_tick(vid);
	}
}

