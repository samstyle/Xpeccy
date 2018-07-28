#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include <assert.h>

#include "video.h"

int vidFlag = 0;
// float brdsize = 1.0;

unsigned char inkTab[128] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

unsigned char papTab[128] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
  0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
  0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};

// zx screen adr
static unsigned short scrAdrs[8192];
static unsigned short atrAdrs[8192];

void vidInitAdrs() {
	unsigned short sadr = 0x0000;
	unsigned short aadr = 0x1800;
	int idx = 0;
	int a,b,c,d;
	for (a = 0; a < 4; a++) {	// parts (4th for profi)
		for (b = 0; b < 8; b++) {	// box lines
			for (c = 0; c < 8; c++) {	// pixel lines
				for (d = 0; d < 32; d++) {	// x bytes
					scrAdrs[idx] = (sadr + d) & 0xffff;
					atrAdrs[idx] = (aadr + d) & 0xffff;
					idx++;
				}
				sadr += 0x100;
			}
			sadr -= 0x7e0;
			aadr += 0x20;
		}
		sadr += 0x700;
	}
}

Video* vidCreate(vcbmrd cb, void* dptr) {
	Video* vid = (Video*)malloc(sizeof(Video));
	memset(vid,0x00,sizeof(Video));
	vid->mrd = cb;
	vid->data = dptr;
	vidSetMode(vid, VID_UNKNOWN);
//	vid->mem = me;
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	vidSetLayout(vid, vlay);
	vid->frmsz = vid->lay.full.x * vid->lay.full.y;
	vid->inten = 0x01;		// FRAME INT for all

	vid->ula = ulaCreate();
	vid->gbc = gbcvCreate(&vid->ray, &vid->lay);
	vid->ppu = ppuCreate(&vid->ray);

//	vidSetFps(vid, 50);
	vidSetBorder(vid, 0.5);

	vid->brdstep = 1;
	vid->nextbrd = 0;
	vid->curscr = 5;
	vid->fcnt = 0;

	vid->nsDraw = 0;
	vid->ray.x = 0;
	vid->ray.y = 0;
	vid->idx = 0;

	vid->ray.ptr = vid->scrimg;

	vid->ray.img = vid->scrimg;
	vid->v9938.lay = &vid->lay;
	vid->v9938.ray = &vid->ray;

	return vid;
}

void vidDestroy(Video* vid) {
	ulaDestroy(vid->ula);
	gbcvDestroy(vid->gbc);
	ppuDestroy(vid->ppu);
	free(vid);
}

void vidUpdateTimings(Video* vid, int nspd) {
	vid->nsPerDot = nspd;
	vid->nsPerLine = vid->nsPerDot * vid->lay.full.x;
	vid->nsPerFrame = vid->nsPerLine * vid->lay.full.y;
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
	vid->ray.x = 0;
	vid->ray.y = 0;
	vid->ray.xb = vid->lay.blank.x;
	vid->ray.yb = vid->lay.blank.y;
	vid->idx = 0;
	vid->ray.ptr = vid->ray.img;
	vid->nsDraw = 0;
	vidSetMode(vid, VID_NORMAL);
}

// new layout:
// [ bord ][ scr ][ ? ][ blank ]
// [ <--------- full --------> ]
// ? = brdr = full - bord - scr - blank
void vidUpdateLayout(Video* vid) {
	vCoord brdr;	// size of right/bottom border parts
	vid->ssze.x = vid->lay.full.x - vid->lay.blank.x;
	vid->ssze.y = vid->lay.full.y - vid->lay.blank.y;
	brdr.x = vid->ssze.x - vid->lay.bord.x - vid->lay.scr.x;
	brdr.y = vid->ssze.y - vid->lay.bord.y - vid->lay.scr.y;
	vid->lcut.x = (int)floor(vid->lay.bord.x * (1.0 - vid->brdsize));
	vid->lcut.y = (int)floor(vid->lay.bord.y * (1.0 - vid->brdsize));
	vid->rcut.x = (int)floor(vid->lay.bord.x + vid->lay.scr.x + brdr.x * vid->brdsize);
	vid->rcut.y = (int)floor(vid->lay.bord.y + vid->lay.scr.y + brdr.y * vid->brdsize);
	vid->vsze.x = vid->rcut.x - vid->lcut.x;
	vid->vsze.y = vid->rcut.y - vid->lcut.y;
	vid->vBytes = vid->vsze.x * vid->vsze.y * 6;	// real size of image buffer (3 bytes/dot x2:x1)
	vidUpdateTimings(vid, vid->nsPerDot);
#ifdef ISDEBUG
	printf("%i : ",vid->lay.bord.x);
	printf("%i - %i, %i - %i\n",vid->lcut.x, vid->rcut.x, vid->lcut.y, vid->rcut.y);
#endif
}

void vidSetLayout(Video* vid, vLayout lay) {
	if (vid->lockLayout) return;
	vid->lay = lay;
	vid->frmsz = lay.full.x * lay.full.y;
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
	xscr = vid->ray.x;
	yscr = vid->ray.y;
	unsigned char* ptr = vid->ray.ptr;
	do {
		if ((yscr >= vid->lcut.y) && (yscr < vid->rcut.y) && (xscr >= vid->lcut.x) && (xscr < vid->rcut.x)) {
#if 0
			*(ptr++) >>= 1;
			*(ptr++) >>= 1;
			*(ptr++) >>= 1;
			*(ptr++) >>= 1;
			*(ptr++) >>= 1;
			*(ptr++) >>= 1;
#else
			*ptr = ((*ptr - 0x80) >> 1) + 128; ptr++;
			*ptr = ((*ptr - 0x80) >> 1) + 128; ptr++;
			*ptr = ((*ptr - 0x80) >> 1) + 128; ptr++;
			*ptr = ((*ptr - 0x80) >> 1) + 128; ptr++;
			*ptr = ((*ptr - 0x80) >> 1) + 128; ptr++;
			*ptr = ((*ptr - 0x80) >> 1) + 128; ptr++;
#endif
		}
		if (++xscr >= vid->lay.full.x) {
			xscr = 0;
			if (++yscr >= vid->lay.full.y)
				ptr = NULL;
		}
	} while (ptr);
	vid->tail = 1;
}

//const unsigned char emptyBox[8] = {0x00,0x18,0x3c,0x7e,0x7e,0x3c,0x18,0x00};
//const unsigned char emptyBox[8] = {0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x81};
static unsigned char emptyBox[8] = {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00};

void vidGetScreen(Video* vid, unsigned char* dst, int bank, int shift, int flag) {
	if ((bank == 0xff) && (shift > 0x2800)) shift = 0x2800;
//	unsigned char* pixs = vid->mem->ramData + MADR(bank, shift);
//	unsigned char* atrs = pixs + 0x1800;
	int pixadr = MADR(bank, shift);
	int atradr = pixadr + 0x1800;
	unsigned char sbyte, abyte, aink, apap;
	int prt, lin, row, xpos, bitn, cidx;
	int sadr, aadr;
	for (prt = 0; prt < 3; prt++) {
		for (lin = 0; lin < 8; lin++) {
			for (row = 0; row < 8; row++) {
				for (xpos = 0; xpos < 32; xpos++) {
					sadr = (prt << 11) | (lin << 5) | (row << 8) | xpos;
					aadr = (prt << 8) | (lin << 5) | xpos;
					sbyte = (flag & 2) ? emptyBox[row] : vid->mrd(pixadr + sadr, vid->data);
					abyte = (flag & 1) ? 0x47 : vid->mrd(atradr + aadr, vid->data);
					aink = inkTab[abyte & 0x7f];
					apap = papTab[abyte & 0x7f];
					for (bitn = 0; bitn < 8; bitn++) {
						cidx = (sbyte & (128 >> bitn)) ? aink : apap;
						if ((flag & 4) && ((lin ^ xpos) & 1)) {
							*(dst++) = ((vid->pal[cidx].r - 0x80) >> 1) + 0x80;
							*(dst++) = ((vid->pal[cidx].g - 0x80) >> 1) + 0x80;
							*(dst++) = ((vid->pal[cidx].b - 0x80) >> 1) + 0x80;
						} else {
							*(dst++) = vid->pal[cidx].r;
							*(dst++) = vid->pal[cidx].g;
							*(dst++) = vid->pal[cidx].b;
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

int contTabA[] = {12,12,10,10,8,8,6,6,4,4,2,2,0,0,0,0};		// 48K 128K +2 (bank 1,3,5,7)
int contTabB[] = {2,1,0,0,14,13,12,11,10,9,8,7,6,5,4,3};	// +2A +3 (bank 4,5,6,7)

void vidWait(Video* vid) {
	if (vid->ray.y < vid->lay.bord.y) return;		// above screen
	if (vid->ray.y >= (vid->lay.bord.y + vid->lay.scr.y)) return;	// below screen
	xscr = vid->ray.x - vid->lay.bord.x; // + 2;
	if (xscr < 0) return;
	if (xscr > (vid->lay.scr.x - 2)) return;
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
	yscr = vid->ray.y - vid->lay.bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->brdcol;
		if (vid->ula->active) col |= 8;
		vid->atrbyte = 0xff;
	} else {
		xscr = vid->ray.x - vid->lay.bord.x;
		if ((xscr & 7) == 4) {
			nxtbyte = vid->mrd(MADR(vid->curscr, scrAdrs[vid->idx]), vid->data);
			//adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			//nxtbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
		}
		if ((vid->ray.x < vid->lay.bord.x) || (vid->ray.x > vid->lay.bord.x + 255)) {
			col = vid->brdcol;
			if (vid->ula->active) col |= 8;
			vid->atrbyte = 0xff;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				vid->atrbyte = vid->mrd(MADR(vid->curscr, atrAdrs[vid->idx]), vid->data);
				if (vid->idx < 0x1b00) vid->idx++;
				//adr = 0x1800 | ((yscr & 0xc0) << 2) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
				//vid->atrbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
				if (vid->ula->active) {
					ink = ((vid->atrbyte & 0xc0) >> 2) | (vid->atrbyte & 7);
					pap = ((vid->atrbyte & 0xc0) >> 2) | ((vid->atrbyte & 0x38) >> 3) | 8;
				} else {
					if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
					ink = inkTab[vid->atrbyte & 0x7f];
					pap = papTab[vid->atrbyte & 0x7f];
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
	yscr = vid->ray.y - vid->lay.bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->brdcol;
	} else {
		xscr = vid->ray.x - vid->lay.bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->brdcol;
		} else {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
			switch (xscr & 7) {
				case 0:
					scrbyte = vid->mrd(MADR(vid->curscr ^ 1, adr), vid->data);
					col = inkTab[scrbyte & 0x7f];
					break;
				case 2:
					scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
					col = inkTab[scrbyte & 0x7f];
					break;
				case 4:
					scrbyte = vid->mrd(MADR(vid->curscr ^ 1, adr + 0x2000), vid->data);
					col = inkTab[scrbyte & 0x7f];
					break;
				case 6:
					scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->data);
					col = inkTab[scrbyte & 0x7f];
					break;
				default:
					col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
					break;

			}
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// hardware multicolor
void vidDrawHwmc(Video* vid) {
	yscr = vid->ray.y - vid->lay.bord.y;
	if ((yscr < 0) || (yscr > 191)) {
		col = vid->brdcol;
	} else {
		xscr = vid->ray.x - vid->lay.bord.x;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
		}
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
				vid->atrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = inkTab[vid->atrbyte & 0x7f];
				pap = papTab[vid->atrbyte & 0x7f];
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col);
}

// atm ega
void vidDrawATMega(Video* vid) {
	yscr = vid->ray.y - 76 + 32;
	xscr = vid->ray.x - 96 + 64;
	if ((yscr < 0) || (yscr > 199) || (xscr < 0) || (xscr > 319)) {
		col = vid->brdcol;
	} else {
		adr = (yscr * 40) + (xscr >> 3);
		switch (xscr & 7) {
			case 0:
				scrbyte = vid->mrd(MADR(vid->curscr ^ 4, adr), vid->data);
				col = inkTab[scrbyte & 0x7f];
				break;
			case 2:
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
				col = inkTab[scrbyte & 0x7f];
				break;
			case 4:
				scrbyte = vid->mrd(MADR(vid->curscr ^ 4, adr + 0x2000), vid->data);
				col = inkTab[scrbyte & 0x7f];
				break;
			case 6:
				scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->data);
				col = inkTab[scrbyte & 0x7f];
				break;
			default:
				col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
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
	ink = inkTab[colr & 0x7f];
	pap = papTab[colr & 0x3f] | ((colr & 0x80) >> 4);
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
				scrbyte = vid->mrd(MADR(vid->curscr, adr), vid->data);
				col = vid->mrd(MADR(vid->curscr ^ 4, adr + 0x2000), vid->data);
			} else {
				scrbyte = vid->mrd(MADR(vid->curscr, adr + 0x2000), vid->data);
				col = vid->mrd(MADR(vid->curscr ^ 4, adr + 1), vid->data);
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
	yscr = vid->ray.y - vid->lay.bord.y; // + 24;
	if ((yscr < 0) || (yscr > 239)) {
		vidPutDot(&vid->ray, vid->pal, vid->brdcol);
	} else {
		xscr = vid->ray.x - vid->lay.bord.x;
		if ((xscr < 0) || (xscr > 255)) {
			vidPutDot(&vid->ray, vid->pal, vid->brdcol);
		} else {
			if ((xscr & 3) == 0) {
				adr = scrAdrs[vid->idx & 0x1fff] & 0x1fff;
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
				ink = inkTab[col & 0x47];
				pap = papTab[(col & 0x3f) | ((col >> 1) & 0x40)];
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

// v9938

void vidDrawV9938(Video* vid) {
	vdpDraw(&vid->v9938);
}

void vidLineV9938(Video* vid) {
	vdpLine(&vid->v9938);
}

void vidFrameV9938(Video* vid) {
	vdpVBlk(&vid->v9938);
}

// gameboy

void vidGBDraw(Video* vid) {
	vid->gbc->draw(vid->gbc);
}

void vidGBLine(Video* vid) {
	if (vid->gbc->cbLine)
		vid->gbc->cbLine(vid->gbc);
}

void vidGBFram(Video* vid) {
	if (vid->gbc->cbFram)
		vid->gbc->cbFram(vid->gbc);
}

// nes
void vidNESDraw(Video* vid) {
	ppuDraw(vid->ppu);
}

void vidNESLine(Video* vid) {
	ppuLine(vid->ppu);
}

void vidNES_HB(Video* vid) {
	ppuHBL(vid->ppu);
}

void vidNESFram(Video* vid) {
	ppuFram(vid->ppu);
}

// c64 vic-II

// text mode
void vidC64TDraw(Video* vid) {
	if (vid->regs[0x11] & 0x10)
	yscr = vid->ray.y - vid->lay.bord.y;
	if ((yscr < 0) || (yscr >= vid->lay.scr.y) || !(vid->regs[0x11] & 0x10)) {
		col = vid->regs[0x20];				// border color
	} else {
		xscr = vid->ray.x - vid->lay.bord.x;
		if ((xscr < 0) || (xscr >= vid->lay.scr.x)) {
			col = vid->regs[0x20];			// border color
		} else {
			if ((xscr & 7) == 0) {
				adr = ((yscr >> 3) * 40) + (xscr >> 3);
				ink = vid->mrd(adr | ((vid->regs[0x18] & 0xf0) << 6), vid->data);				// tile nr
				if ((~vid->vbank & 1) && ((vid->regs[0x18] & 0x0c) == 0x04)) {
					scrbyte = vid->font[(ink << 3) | (yscr & 7)];	// from char rom
				} else {
					scrbyte = vid->mrd(((vid->regs[0x18] & 0x0e) << 10) | (ink << 3) | (yscr & 7), vid->data);	// tile row data
				}
				ink = vid->colram[adr & 0x3ff];			// tile color
				pap = vid->regs[0x21];				// background color
			}
			if (vid->line[xscr] != 0xff) {
				col = vid->line[xscr];
			} else if (scrbyte & 0x80) {
				col = ink;
			} else if (vid->linb[xscr] != 0xff) {
				col = vid->linb[xscr];
			} else {
				col = pap;
			}
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

// multicolor text
// if bit 4 in color ram = 1, this is multicolor cell
// if bit 4 in color ram = 0, this is common cell

void vidC64TMDraw(Video* vid) {
	yscr = vid->ray.y - vid->lay.bord.y;
	if ((yscr < 0) || (yscr >= vid->lay.scr.y) || !(vid->regs[0x11] & 0x10)) {
		col = vid->regs[0x20];				// border color
	} else {
		xscr = vid->ray.x - vid->lay.bord.x;
		if ((xscr < 0) || (xscr >= vid->lay.scr.x)) {
			col = vid->regs[0x20];			// border color
		} else {
			if ((xscr & 7) == 0) {
				adr = ((yscr >> 3) * 40) + (xscr >> 3);						// offset to tile
				ink = vid->mrd(adr | ((vid->regs[0x18] & 0xf0) << 6), vid->data);		// tile nr
				if ((~vid->vbank & 1) && ((vid->regs[0x18] & 0x0c) == 0x04)) {
					scrbyte = vid->font[(ink << 3) | (yscr & 7)];
				} else {
					scrbyte = vid->mrd(((vid->regs[0x18] & 0x0e) << 10) | (ink << 3) | (yscr & 7), vid->data);
				}
				atrbyte = vid->colram[adr & 0x3ff];			// tile color
				pap = vid->regs[0x21];				// background color
			}
			if (vid->line[xscr] != 0xff) {
				col = vid->line[xscr] & 0x0f;
			} else if (atrbyte & 8) {
				switch (scrbyte & 0xc0) {
					case 0x00:
						if (vid->linb[xscr] != 0xff) {
							col = vid->linb[xscr] & 0x0f;
						} else {
							col = vid->regs[0x21];
						}
						break;
					case 0x40:
						col = vid->regs[0x22];
						break;
					case 0x80:
						col = vid->regs[0x23];
						break;
					case 0xc0:
						col = atrbyte & 7;	// only colors 0..7
						break;
				}
				if (xscr & 1) scrbyte <<= 2;
			} else {			// not multicolor
				if (scrbyte & 0x80) {
					col = atrbyte;
				} else if (vid->linb[xscr] != 0xff) {
					col = vid->linb[xscr];
				} else {
					col = pap;
				}
				scrbyte <<= 1;
			}
		}
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

// bitmap
void vidC64BDraw(Video* vid) {
	if (vid->regs[0x11] & 0x10) {
		yscr = vid->ray.y - vid->lay.bord.y;
		if ((yscr < 0) || (yscr >= vid->lay.scr.y)) {
			col = vid->regs[0x20];
		} else {
			xscr = vid->ray.x - vid->lay.bord.x;
			if ((xscr < 0) || (xscr >= vid->lay.scr.x)) {
				col = vid->regs[0x20];
			} else {
				if ((xscr & 7) == 0) {
					adr = (yscr >> 3) * 320 + (xscr & ~7) + (yscr & 7);
					scrbyte = vid->mrd(adr | ((vid->regs[0x18] & 0x08) << 10), vid->data);
					adr = (yscr >> 3) * 40 + (xscr >> 3);
					ink = vid->mrd(adr | ((vid->regs[0x18] & 0xf0) << 6), vid->data);
					pap = ink & 0x0f;		// 0
					ink = (ink >> 4) & 0x0f;	// 1
				}
				col = (scrbyte & 0x80) ? ink : pap;
				scrbyte <<= 1;
			}
		}
	} else {
		col = vid->regs[0x20];
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

// multicolor bitmap
void vidC64BMDraw(Video* vid) {
	if (vid->regs[0x11] & 0x10) {
		yscr = vid->ray.y - vid->lay.bord.y;
		if ((yscr < 0) || (yscr >= vid->lay.scr.y)) {
			col = vid->regs[0x20];
		} else {
			xscr = vid->ray.x - vid->lay.bord.x;
			if ((xscr < 0) || (xscr >= vid->lay.scr.x)) {
				col = vid->regs[0x20];
			} else {
				if ((xscr & 7) == 0) {
					adr = (yscr >> 3) * 320 + (xscr & 0xf8) + (yscr & 7);
					scrbyte = vid->mrd(adr | ((vid->regs[0x18] & 0x08) << 10), vid->data);
				}
				adr = (yscr >> 3) * 40 + (xscr >> 3);
				ink = vid->mrd(adr | ((vid->regs[0x18] & 0xf0) << 6), vid->data);
				if ((xscr & 1) == 0) {
					switch (scrbyte & 0xc0) {
						case 0x00:
							col = vid->regs[0x21];
							break;
						case 0x40:
							col = (ink >> 4) & 0x0f;
							break;
						case 0x80:
							col = ink & 0x0f;
							break;
						case 0xc0:
							col = vid->colram[adr & 0x3ff] >> 4;
							break;
					}
					scrbyte <<= 2;
				}
			}
		}
	} else {
		col = vid->regs[0x20];
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

void vidC64Fram(Video* vid) {
}

// default size: 24x21 pix
// 07f8..ff	sprites pointers (+ [53272] shift)
// d000/01	sprite 0 x,y
// d00e/0f	sprite 7 x,y
// d010	bits	sprite x 8th bit
// d015	bits	sprite enabled
// d017 bits	sprite is double height
// d01b	bits	sprite priority (0:above screen, 1:behind screen)
// d01c	bist	sprite multicolor
// d01d bits	sprite is double width
// d01e	bits	wr:spr-bgr collision enabled; rd:detected collisions
// d01f bits	wr:spr-spr collision enabled; rd:detected collisions
// d025		spr extra color 1
// d026		spr extra color 2
// d027..2e	sprite colors

void vidC64Line(Video* vid) {
	if (vid->lay.intpos.y == vid->ray.y) {		// if current line == interrupt line and raster interrupt is enabled
		vid->intout |= (vid->inten & VIC_IRQ_RASTER);
	} else if (vid->intout & VIC_IRQ_RASTER) {
		vid->intout &= ~VIC_IRQ_RASTER;
	}
	// sprites
	memset(vid->line, 0xff, 512);	// ff as transparent color
	memset(vid->linb, 0xff, 512);
	int yscr = vid->ray.y - vid->lay.bord.y;	// current screen line
	unsigned char sprxh = vid->regs[0x10];	// bit x : sprite x X 8th bit
	unsigned char spren = vid->regs[0x15];	// bit x : sprite x enabled
	unsigned char sprmc = vid->regs[0x1c];	// bit x : sprite x multicolor
	unsigned char sprhi = vid->regs[0x17];	// bit x : sprite x double height
	unsigned char sprpr = vid->regs[0x1b];	// bit x : sprite x behind screen
	unsigned char sprwi = vid->regs[0x1d];	// bit x : sprite x double width
	unsigned char msk = 1;
	int posy;
	int posx;
	int sadr;
	unsigned char dat;
	unsigned char scol;
	unsigned char* ptr;
	int bn;
	int bt;
	int i;
	vid->sprxspr = 0;
	for (i = 0; i < 8; i++) {
		if (spren & msk) {
			posy = yscr - vid->regs[i * 2 + 1];		// line inside sprite
			if (sprhi & msk)
				posy = posy / 2;
			if ((posy >= 0) && (posy < 21)) {
				posx = vid->regs[i * 2];
				if (sprxh & msk)
					posx |= 0x100;
//				printf("%.2X, %.4X, %.2X\n",vid->regs[0x18] & 0x07, (((vid->regs[0x18] & 0x07) << 11) | 0x7f8) + i, vid->vbank);
//				assert(0);
				sadr = vid->mrd(((vid->regs[0x18] & 0xf0) << 6) + 0x3f8 + i, vid->data) << 6;		// address of sprite data
				sadr += posy * 3;									// address of current line
				for (bn = 0; bn < 2; bn++) {
					dat = vid->mrd(sadr, vid->data);
					sadr++;
					ptr = (sprpr & msk) ? vid->linb : vid->line;
					if (sprmc & msk) {		// multicolor
						for (bt = 0; bt < 4; bt++) {
							switch (dat & 0xc0) {
								case 0x00: scol = 0xff; break;	// transparent
								case 0x40: scol = vid->regs[0x25] & 0x0f; break;
								case 0x80: scol = vid->regs[0x27 + i] & 0x0f; break;
								default: scol = vid->regs[0x26] & 0x0f; break;
							}
							//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
							ptr[posx & 0x1ff] = scol;
							posx++;
							//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
							ptr[posx & 0x1ff] = scol;
							posx++;
							if (sprwi & msk) {
								//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
								ptr[posx & 0x1ff] = scol;
								posx++;
								//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
								ptr[posx & 0x1ff] = scol;
								posx++;
							}
							dat <<= 2;
						}
					} else {			// common
						for (bt = 0x80; bt > 0; bt >>= 1) {
							scol = (dat & bt) ? (vid->regs[0x27 + i] & 0x0f) : 0xff;
							if (ptr[posx] == 0xff) {
								ptr[posx & 0x1ff] = scol;
							} else {
								vid->sprxspr |= msk;
							}
							posx++;
							if (sprwi & msk) {
								if (ptr[posx] == 0xff) {
									ptr[posx & 0x1ff] = scol;
								} else {
									vid->sprxspr |= msk;
								}
								ptr[posx & 0x1ff] = scol;
								posx++;
							}
							// dat <<= 1;
						}
					}
				}
			}
		}
		msk <<= 1;
	}
}

// debug

void vidBreak(Video* vid) {
	printf("vid->mode = 0x%.2X\n",vid->vmode);
	// assert(0);
}

// weiter

typedef struct {
	int id;
	void(*callback)(Video*);		// each dot
	void(*lineCall)(Video*);		// hblank start
	void(*hbendCall)(Video*);		// hblank end
	void(*framCall)(Video*);		// vblank start
} xVideoMode;

// id,(@every_visible_dot),(@HBlank),(@LineStart),(@VBlank)
static xVideoMode vidModeTab[] = {
	{VID_NORMAL, vidDrawNormal, NULL, NULL, NULL},
	{VID_ALCO, vidDrawAlco, NULL, NULL, NULL},
	{VID_HWMC, vidDrawHwmc, NULL, NULL, NULL},
	{VID_ATM_EGA, vidDrawATMega, NULL, NULL, NULL},
	{VID_ATM_TEXT, vidDrawATMtext, NULL, NULL, NULL},
	{VID_ATM_HWM, vidDrawATMhwmc, NULL, NULL, NULL},
	{VID_EVO_TEXT, vidDrawEvoText, NULL, NULL, NULL},
	{VID_TSL_NORMAL, vidDrawTSLNormal, vidTSline, NULL, NULL},
	{VID_TSL_16, vidDrawTSLExt, vidTSline, NULL, NULL},			// vidDrawTSL16
	{VID_TSL_256, vidDrawTSLExt, vidTSline, NULL, NULL},			// vidDrawTSL256
	{VID_TSL_TEXT, vidDrawTSLText, vidTSline, NULL, NULL},
	{VID_PRF_MC, vidProfiScr, NULL, NULL, NULL},
	{VID_V9938, vidDrawV9938, NULL, NULL, vidFrameV9938},
	{VID_GBC, vidGBDraw, vidGBLine, NULL, vidGBFram},
	{VID_NES, vidNESDraw, vidNES_HB, vidNESLine, vidNESFram},
	{VID_C64_TEXT, vidC64TDraw, NULL, vidC64Line, vidC64Fram},
	{VID_C64_TEXT_MC, vidC64TMDraw, NULL, vidC64Line, vidC64Fram},
	{VID_C64_BITMAP, vidC64BDraw, NULL, vidC64Line, vidC64Fram},
	{VID_C64_BITMAP_MC, vidC64BMDraw, NULL, vidC64Line, vidC64Fram},
	{VID_UNKNOWN, vidDrawBorder, NULL, NULL, NULL}
};

void vidSetMode(Video* vid, int mode) {
	vid->vmode = mode;
	int i = 0;
	while ((vidModeTab[i].id != VID_UNKNOWN) && (vidModeTab[i].id != mode)) {
		i++;
	}
	vid->callback = vid->noScreen ? vidDrawBorder : vidModeTab[i].callback;
	vid->lineCall = vidModeTab[i].lineCall;
	vid->framCall = vidModeTab[i].framCall;
	vid->hbendCall = vidModeTab[i].hbendCall;
}

void vidSync(Video* vid, int ns) {
	vid->nsDraw += ns;
	vid->time += ns;
	while (vid->nsDraw >= vid->nsPerDot) {

		vid->nsDraw -= vid->nsPerDot;
//		vid->time += vid->nsPerDot;

		if ((vid->ray.x & vid->brdstep) == 0)
			vid->brdcol = vid->nextbrd;

		// if ray is on visible screen & video has drawing callback...
		if ((vid->ray.y >= vid->lcut.y) && (vid->ray.y < vid->rcut.y) \
			&& (vid->ray.x >= vid->lcut.x) && (vid->ray.x < vid->rcut.x) \
			&& vid->callback) {
				vid->callback(vid);		// put dot callback
		}
		// move ray to next dot, update counters
		vid->ray.x++;
		vid->ray.xb++;

		if (vid->ray.xb == vid->lay.blank.x) {			// hblank end
			vid->hblank = 0;
			vid->hbstrb = 0;
			vid->ray.x = 0;
			if (vid->hbendCall) vid->hbendCall(vid);
		} else if (vid->ray.xb >= vid->lay.full.x) {		// hblank start
			vid->hblank = 1;
			vid->hbstrb = 1;
			vid->ray.xb = 0;
			vid->ray.y++;
			vid->ray.yb++;
			if (vid->ray.yb == vid->lay.blank.y) {		// vblank end
				vid->vblank = 0;
				vid->vbstrb = 0;
				vid->ray.y = 0;
				vid->tsconf.scrLine = 0;
				if (vid->lineCall) vid->lineCall(vid);
				if (vid->debug) vidDarkTail(vid);
			} else if (vid->ray.yb >= vid->lay.full.y) {	// vblank start
				vid->vblank = 1;
				vid->vbstrb = 1;
				vid->ray.yb = 0;
				vid->fcnt++;
				vid->flash = (vid->fcnt & 0x20) ? 1 : 0;
				vid->ray.ptr = vid->ray.img;
				vid->idx = 0;
				vid->newFrame = 1;
				vid->tail = 0;
				if (vid->lineCall) vid->lineCall(vid);		// new line callback
				if (vid->framCall) vid->framCall(vid);		// frame callback
			} else if (vid->lineCall) {
				vid->lineCall(vid);
			}
		}
		// generate int
		if (vid->intFRAME) vid->intFRAME--;
		if ((vid->ray.yb == vid->lay.intpos.y) && (vid->ray.xb == vid->lay.intpos.x)) {
			vid->intFRAME = vid->lay.intSize;
		}
	}
}
