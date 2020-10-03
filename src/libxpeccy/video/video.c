#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <assert.h>

#include "video.h"

int bytesPerLine = 100;
int greyScale = 0;
int noflic = 0;

static unsigned char bufa[2560 * 1440 * 3];
static unsigned char bufb[2560 * 1440 * 3];
unsigned char* scrimg = bufa;
unsigned char* bufimg = bufb;
static int curbuf = 0;

static int xpos = 0;
static int ypos = 0;
int xstep = 0x100;
int ystep = 0x100;
int lefSkip = 0;
int rigSkip = 0;
int topSkip = 0;
int botSkip = 0;

static unsigned char pscr[2560 * 1440 * 3];
static unsigned char* pptr = pscr;
static unsigned char pcol;
static xColor xcol;

void vid_dot(Video* vid, unsigned char col) {
	xcol = vid->pal[col];
	if (greyScale) {
		pcol = (xcol.b * 30 + xcol.r * 76 + xcol.g * 148) >> 8;
		xcol.r = pcol;
		xcol.g = pcol;
		xcol.b = pcol;
	}
	pcol = xcol.r;
	*(vid->ray.ptr++) = (pcol * (100 - noflic) + *pptr * noflic) / 100;
	*(pptr++) = pcol;
	pcol = xcol.g;
	*(vid->ray.ptr++) = (pcol * (100 - noflic) + *pptr * noflic) / 100;
	*(pptr++) = pcol;
	pcol = xcol.b;
	*(vid->ray.ptr++) = (pcol * (100 - noflic) + *pptr * noflic) / 100;
	*(pptr++) = pcol;

}

void vid_dot_full(Video* vid, unsigned char col) {
	xpos += xstep;
	while (xpos > 0xff) {
		xpos -= 0x100;
		vid_dot(vid, col);
	}
}

void vid_dot_half(Video* vid, unsigned char col) {
	xpos += xstep / 2;
	while (xpos > 0xff) {
		xpos -= 0x100;
		vid_dot(vid, col);
	}
}

void vid_line(Video* vid) {
	memset(vid->ray.ptr, 0x00, 3);	// put black dot
	if (rigSkip)
		memset(vid->ray.ptr, 0x00, rigSkip);
	vid->ray.lptr += bytesPerLine;

	ypos += ystep;
	ypos -= 0x100;		// 1 line is already drawn
	xpos = 0;
	while (ypos > 0) {
		ypos -= 0x100;
		memcpy(vid->ray.lptr, vid->ray.lptr - bytesPerLine, bytesPerLine);
		vid->ray.lptr += bytesPerLine;
	}

	if (lefSkip)
		memset(vid->ray.lptr, 0x00, lefSkip);
	vid->ray.ptr = vid->ray.lptr + lefSkip;
}

void vid_line_fill(Video* vid) {
	int ytmp = ypos;
	unsigned char* ptr = vid->ray.lptr;
	ytmp += ystep;
	while (ytmp > 0x100) {
		ytmp -= 0x100;
		memcpy(ptr + bytesPerLine, ptr, bytesPerLine);
		ptr += bytesPerLine;
	}
}

void vid_frame(Video* vid) {
	if (botSkip)
		memset(vid->ray.lptr, 0x00, botSkip * bytesPerLine);
	ypos = 0;
	if (!vid->debug) {
		scrimg = curbuf ? bufb : bufa;
		bufimg = curbuf ? bufa : bufb;
		curbuf = !curbuf;
	}
	if (topSkip)
		memset(scrimg, 0x00, topSkip * bytesPerLine);
	vid->ray.lptr = scrimg + topSkip * bytesPerLine;
	if (lefSkip)
		memset(vid->ray.lptr, 0x00, lefSkip);
	vid->ray.ptr = vid->ray.lptr + lefSkip;
	pptr = pscr;
}

Video* vidCreate(vcbmrd cb, void* dptr) {
	Video* vid = (Video*)malloc(sizeof(Video));
	memset(vid,0x00,sizeof(Video));
	vid->mrd = cb;
	vid->data = dptr;
	vidSetMode(vid, VID_UNKNOWN);
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	vidSetLayout(vid, &vlay);
	vid->inten = 0x01;		// FRAME INT for all

	vid->ula = ula_create();

	vidSetBorder(vid, 0.5);

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

void vidUpdateTimings(Video* vid, int nspd) {
	vid->nsPerDot = nspd;
	vid->nsPerLine = vid->nsPerDot * vid->full.x;
	vid->nsPerFrame = vid->nsPerLine * vid->full.y;
#ifdef ISDEBUG
	// printf("%i / %i / %i\n", vid->nsPerDot, vid->nsPerLine, vid->nsPerFrame);
#endif
}

void vidReset(Video* vid) {
	int i;
	for (i = 0; i<16; i++) {
		vid->pal[i].b = (i & 1) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
		vid->pal[i].r = (i & 2) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
		vid->pal[i].g = (i & 4) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
	}
	vid->ula->active = 0;
	vid->curscr = 5;
	vid->nsDraw = 0;
//	vidSetMode(vid, VID_NORMAL);
}

// new layout:
// [ bord ][ scr ][ ? ][ blank ]
// [ <--------- full --------> ]
// ? = brdr = full - bord - scr - blank
void vidUpdateLayout(Video* vid) {
	vCoord brdr;	// size of right/bottom border parts
	vid->vend.x = vid->full.x - vid->blank.x;
	vid->vend.y = vid->full.y - vid->blank.y;
	brdr.x = vid->vend.x - vid->bord.x - vid->scrn.x;
	brdr.y = vid->vend.y - vid->bord.y - vid->scrn.y;
	vid->lcut.x = (int)floor(vid->bord.x * (1.0 - vid->brdsize));
	vid->lcut.y = (int)floor(vid->bord.y * (1.0 - vid->brdsize));
	vid->rcut.x = (int)floor(vid->bord.x + vid->scrn.x + brdr.x * vid->brdsize);
	vid->rcut.y = (int)floor(vid->bord.y + vid->scrn.y + brdr.y * vid->brdsize);
	vid->vsze.x = vid->rcut.x - vid->lcut.x;
	vid->vsze.y = vid->rcut.y - vid->lcut.y;
	vid->send.x = vid->bord.x + vid->scrn.x;
	vid->send.y = vid->bord.y + vid->scrn.y;
	vid->vBytes = vid->vsze.x * vid->vsze.y * 6;	// real size of image buffer (3 bytes/dot x2:x1)
	vidUpdateTimings(vid, vid->nsPerDot);
}

void vidSetLayout(Video* vid, vLayout* lay) {
	vid->full = lay->full;
	vid->bord = lay->bord;
	vid->blank = lay->blank;
	vid->scrn = lay->scr;
	vid->intp = lay->intpos;
	vid->intsize = lay->intSize;
	vid->frmsz = lay->full.x * lay->full.y;
	vidUpdateLayout(vid);
}

void vidSetBorder(Video* vid, double brd) {
	if (brd < 0.0) brd = 0.0;
	else if (brd > 1.0) brd = 1.0;
	vid->brdsize = brd;
	vidUpdateLayout(vid);
}

int xscr = 0;
int yscr = 0;
int adr = 0;
unsigned char col = 0;
unsigned char ink = 0;
unsigned char pap = 0;
unsigned char scrbyte = 0;
unsigned char atrbyte = 0;
unsigned char nxtbyte = 0;

void vidDarkTail(Video* vid) {
	if (vid->tail) return;				// no filling while current fill is active (till end of frame)
	unsigned char* ptr = vid->ray.ptr;		// fill current line till EOL
	unsigned char* btr = curbuf ? bufa : bufb;
	while (ptr - vid->ray.lptr < bytesPerLine) {
		*ptr = ((*ptr - 0x80) >> 2) + 0x80;
		ptr++;
	}
	vid_line_fill(vid);				// copy filled line due zoom value
	int ytmp = ypos + ystep;
	ptr = vid->ray.lptr;				// move ptr to start of next real line
	while(ytmp > 0x100) {
		ytmp -= 0x100;
		ptr += bytesPerLine;
	}						// fill all till end
	while (ptr - btr < sizeof(bufa)) {
		*ptr = ((*ptr - 0x80) >> 2) + 0x80;
		ptr++;
	}
	vid->tail = 1;
}

//const unsigned char emptyBox[8] = {0x00,0x18,0x3c,0x7e,0x7e,0x3c,0x18,0x00};
//const unsigned char emptyBox[8] = {0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x81};
static unsigned char emptyBox[8] = {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00};

void vidGetScreen(Video* vid, unsigned char* dst, int bank, int shift, int flag) {
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
					sbyte = (flag & 2) ? emptyBox[row] : vid->mrd(pixadr + sadr, vid->data);
					abyte = (flag & 1) ? 0x47 : vid->mrd(atradr + aadr, vid->data);
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

/*
waits for 128K, +2
	--wwwwww wwwwww-- : 16-dots cycle, start on border 8 dots before screen
waits for +2a, +3
	ww--wwww wwwwwwww : same
unsigned char waitsTab_A[16] = {5,5,4,4,3,3,2,2,1,1,0,0,0,0,6,6};	// 48K
unsigned char waitsTab_B[16] = {1,1,0,0,7,7,6,6,5,5,4,4,3,3,2,2};	// +2A,+3
*/

static int contTabA[] = {12,12,10,10,8,8,6,6,4,4,2,2,0,0,0,0};		// 48K 128K +2 (bank 1,3,5,7)
// static int contTabB[] = {2,1,0,0,14,13,12,11,10,9,8,7,6,5,4,3};	// +2A +3 (bank 4,5,6,7)

void vidWait(Video* vid) {
	if (vid->ray.y < vid->bord.y) return;		// above screen
	if (vid->ray.y >= (vid->bord.y + vid->scrn.y)) return;	// below screen
	xscr = vid->ray.x - vid->bord.x; // + 2;
	if (xscr < 0) return;
	if (xscr > (vid->scrn.x - 2)) return;
	vidSync(vid, contTabA[xscr & 0x0f] * vid->nsPerDot);
}

void vidSetFont(Video* vid, char* src) {
	memcpy(vid->font,src,0x800);
}

/*
void vidSetFps(Video* vid, int fps) {
	if (fps < 10) fps = 10;
	else if (fps > 100) fps = 100;
	vid->fps = fps;
	vidUpdateTimings(vid, vid->nsPerDot);
}
*/

// video drawing

void vidDrawBorder(Video* vid) {
	vidPutDot(&vid->ray, vid->pal, vid->brdcol);
}

// common 256 x 192
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
			nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
		}
		if (vid->hbrd) {
			col = vid->brdcol;
			if (vid->ula->active) col |= 8;
			vid->atrbyte = 0xff;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = 0x1800 | ((vid->idx & 0x1f00) >> 3) | (vid->idx & 0x1f);
				vid->atrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
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
	vidPutDot(&vid->ray, vid->pal, col);
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
					scrbyte = vid->mrd(MADR(vid->curscr ^ 1, adr), vid->data);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				case 2:
					scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				case 4:
					scrbyte = vid->mrd(MADR(vid->curscr ^ 1, adr + 0x2000), vid->data);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				case 6:
					scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->data);
					col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
					break;
				default:
					col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
					break;

			}
//		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
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
			nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
		}
		if (vid->hbrd) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
				vid->atrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = (vid->atrbyte & 0x07) | ((vid->atrbyte & 0x40) >> 3);
				pap = (vid->atrbyte & 0x78) >> 3;
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
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
				scrbyte = vid->mrd(MADR(vid->curscr ^ 4, adr), vid->data) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3); // inkTab[scrbyte & 0x7f];
				break;
			case 2:
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
				break;
			case 4:
				scrbyte = vid->mrd(MADR(vid->curscr ^ 4, adr + 0x2000), vid->data) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
				break;
			case 6:
				scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->data) & 0xff;
				col = (scrbyte & 7) | ((scrbyte & 0x40) >> 3);
				break;
			default:
				col = ((scrbyte & 0x38) >> 3) | ((scrbyte & 0x80) >> 4);
				break;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// atm text

void vidDrawByteDD(Video* vid) {		// draw byte $scrbyte with colors $ink,$pap at double-density mode
	for (int i = 0x80; i > 0; i >>= 1) {
		vidSingleDot(&vid->ray, vid->pal, (scrbyte & i) ? ink : pap);
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
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		adr = 0x1c0 + ((yscr & 0xf8) << 3) + (xscr >> 3);
		if ((xscr & 3) == 0) {
			if ((xscr & 7) == 0) {
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data) & 0xff;
				col = vid->mrd(MADR(vid->curscr ^ 4, adr ^ 0x2000), vid->data) & 0xff;
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr, adr ^ 0x2000), vid->data) & 0xff;
				col = vid->mrd(MADR(vid->curscr ^ 4, adr + 1), vid->data) & 0xff;
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
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		//xscr = vid->ray.x - 96;
		//yscr = vid->ray.y - 76;
		adr = (yscr * 40) + (xscr >> 3);
		if ((xscr & 3) == 0) {
			if ((xscr & 7) == 0) {
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
				col = vid->mrd(MADR(vid->curscr ^ 4, adr), vid->data);
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->data);
				col = vid->mrd(MADR(vid->curscr ^ 4, adr + 0x2000), vid->data);
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
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		if ((xscr & 3) == 0) {
			adr = 0x1c0 + ((yscr & 0xf8) << 3) + (xscr >> 3);
			if ((xscr & 7) == 0) {
				scrbyte = vid->mrd(MADR(vid->curscr + 3, adr), vid->data);
				col = vid->mrd(MADR(vid->curscr + 3, adr + 0x3000), vid->data);
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr + 3, adr + 0x1000), vid->data);
				col = vid->mrd(MADR(vid->curscr + 3, adr + 0x2001), vid->data);
			}
			scrbyte = vid->font[(scrbyte << 3) | (yscr & 7)];
			vidATMDoubleDot(vid,col);
		}
	}
}

// profi 512x240

void vidProfiScr(Video* vid) {
	yscr = vid->ray.y - vid->bord.y; // + 24;
	if ((yscr < 0) || (yscr > 239)) {
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			vidPutDot(&vid->ray, vid->pal, vid->brdcol);
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
					scrbyte = vid->mrd(MADR(6, adr), vid->data);
					col = vid->mrd(MADR(0x3a, adr), vid->data);		// b0..2 ink, b3..5 pap, b6 inkBR, b7 papBR
				} else {
					scrbyte = vid->mrd(MADR(4, adr), vid->data);
					col = vid->mrd(MADR(0x38, adr), vid->data);
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

	{VID_UNKNOWN, NULL, vidDrawBorder, NULL, NULL, NULL}
};

void vidSetMode(Video* vid, int mode) {
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

void vidSync(Video* vid, int ns) {
	vid->nsDraw += ns;
	vid->time += ns;
	while (vid->nsDraw >= vid->nsPerDot) {

		vid->nsDraw -= vid->nsPerDot;

		if ((vid->ray.x & vid->brdstep) == 0)
			vid->brdcol = vid->nextbrd;

		// if ray is on visible screen & video has drawing callback...
		if ((vid->ray.y >= vid->lcut.y) && (vid->ray.y < vid->rcut.y) \
			&& (vid->ray.x >= vid->lcut.x) && (vid->ray.x < vid->rcut.x) \
			&& vid->cbDot) {
				vid->cbDot(vid);		// put dot callback
		}
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
			vid->ray.y++;
			vid->ray.xb = 0;
			vid->ray.yb++;
			if (vid->ray.y >= vid->full.y) {		// new frame
				vid_frame(vid);
				vid->lcnt = 0;
				vid->vblank = 0;
				vid->vbstrb = 0;
				vid->ray.y = 0;
				vid->tsconf.scrLine = 0;
				vid->tail = 0;
				if (vid->debug)
					vidDarkTail(vid);
			}
			if (vid->ray.y == vid->vend.y) {		// vblank start
				vid->vblank = 1;
				vid->vbstrb = 1;
				vid->ray.yb = 0;
				vid->fcnt++;
				vid->flash = (vid->fcnt & 0x20) ? 1 : 0;
				vid->idx = 0;
				vid->newFrame = 1;
				if (vid->cbFrame) vid->cbFrame(vid);
			}
			if (vid->ray.y == vid->send.y) {		// screen end V
				vid->vbrd = 1;
			}
			if (vid->ray.y == vid->bord.y) {	// screen start V
				vid->vbrd = 0;
				vid->ray.ys = -1;		// will be 0 at start of next line, but during HBlank is -1
			}
			if (vid->cbHBlank) vid->cbHBlank(vid);
		}
		if (vid->ray.x == vid->send.x) {		// screen end H
			vid->hbrd = 1;
		} else if (vid->ray.x == vid->bord.x) {	// screen start H
			vid->hbrd = 0;
		}
		// generate int
		// TODO: ray.xb(yb) used here only
		if (vid->intFRAME) vid->intFRAME--;
		if ((vid->ray.yb == vid->intp.y) && (vid->ray.xb == vid->intp.x)) {
			vid->intFRAME = vid->intsize;
		}
		if (vid->busy) vid->busy--;
		if (vid->inth) vid->inth--;
		if (vid->intf) vid->intf--;
	}
}
