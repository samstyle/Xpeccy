#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>


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

Video* vidCreate(Memory* me) {
	Video* vid = (Video*)malloc(sizeof(Video));
	memset(vid,0x00,sizeof(Video));
	vidSetMode(vid, VID_UNKNOWN);
	vid->mem = me;
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	vidSetLayout(vid, vlay);
	vid->frmsz = vid->lay.full.x * vid->lay.full.y;
	vid->intMask = 0x01;		// FRAME INT for all

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
const unsigned char emptyBox[8] = {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00};

void vidGetScreen(Video* vid, unsigned char* dst, int bank, int shift, int flag) {
	if ((bank == 0xff) && (shift > 0x2800)) shift = 0x2800;
	unsigned char* pixs = vid->mem->ramData + MADR(bank, shift);
	unsigned char* atrs = pixs + 0x1800;
	unsigned char sbyte, abyte, aink, apap;
	int prt, lin, row, xpos, bitn, cidx;
	int sadr, aadr;
	for (prt = 0; prt < 3; prt++) {
		for (lin = 0; lin < 8; lin++) {
			for (row = 0; row < 8; row++) {
				for (xpos = 0; xpos < 32; xpos++) {
					sadr = (prt << 11) | (lin << 5) | (row << 8) | xpos;
					aadr = (prt << 8) | (lin << 5) | xpos;
					sbyte = (flag & 2) ? emptyBox[row] : *(pixs + sadr);
					abyte = (flag & 1) ? 0x47 : *(atrs + aadr);
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
			nxtbyte = vid->mem->ramData[MADR(vid->curscr, scrAdrs[vid->idx])];
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
				vid->atrbyte = vid->mem->ramData[MADR(vid->curscr, atrAdrs[vid->idx])];
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
					scrbyte = vid->mem->ramData[MADR(vid->curscr ^ 1, adr)];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 2:
					scrbyte = vid->mem->ramData[MADR(vid->curscr, adr)];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 4:
					scrbyte = vid->mem->ramData[MADR(vid->curscr ^ 1, adr + 0x2000)];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 6:
					scrbyte = vid->mem->ramData[MADR(vid->curscr, adr + 0x2000)];
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
			nxtbyte = vid->mem->ramData[MADR(vid->curscr, adr)];
		}
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
				vid->atrbyte = vid->mem->ramData[MADR(vid->curscr, adr)];
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
				scrbyte = vid->mem->ramData[MADR(vid->curscr ^ 4, adr)];
				col = inkTab[scrbyte & 0x7f];
				break;
			case 2:
				scrbyte = vid->mem->ramData[MADR(vid->curscr, adr)];
				col = inkTab[scrbyte & 0x7f];
				break;
			case 4:
				scrbyte = vid->mem->ramData[MADR(vid->curscr ^ 4, adr + 0x2000)];
				col = inkTab[scrbyte & 0x7f];
				break;
			case 6:
				scrbyte = vid->mem->ramData[MADR(vid->curscr, adr + 0x2000)];
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
				scrbyte = vid->mem->ramData[MADR(vid->curscr, adr)];
				col = vid->mem->ramData[MADR(vid->curscr ^ 4, adr + 0x2000)];
			} else {
				scrbyte = vid->mem->ramData[MADR(vid->curscr, adr + 0x2000)];
				col = vid->mem->ramData[MADR(vid->curscr ^ 4, adr + 1)];
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
		xscr = vid->ray.x - 96;
		yscr = vid->ray.y - 76;
		adr = (yscr * 40) + (xscr >> 3);
		if ((xscr & 3) == 0) {
			if ((xscr & 7) == 0) {
				scrbyte = vid->mem->ramData[MADR(vid->curscr, adr)];
				col = vid->mem->ramData[MADR(vid->curscr ^ 4, adr)];
			} else {
				scrbyte = vid->mem->ramData[MADR(vid->curscr, adr + 0x2000)];
				col = vid->mem->ramData[MADR(vid->curscr ^ 4, adr + 0x2000)];
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
				scrbyte = vid->mem->ramData[MADR(vid->curscr + 3, adr)];
				col = vid->mem->ramData[MADR(vid->curscr + 3, adr + 0x3000)];
			} else {
				scrbyte = vid->mem->ramData[MADR(vid->curscr + 3, adr + 0x1000)];
				col = vid->mem->ramData[MADR(vid->curscr + 3, adr + 0x2001)];
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
					scrbyte = vid->mem->ramData[MADR(6, adr)];
					col = vid->mem->ramData[MADR(0x3a, adr)];		// b0..2 ink, b3..5 pap, b6 inkBR, b7 papBR
				} else {
					scrbyte = vid->mem->ramData[MADR(4, adr)];
					col = vid->mem->ramData[MADR(0x38, adr)];
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
	vid->v9938.draw(&vid->v9938);
}

void vidLineV9938(Video* vid) {
	if (vid->v9938.cbLine)
		vid->v9938.cbLine(&vid->v9938);
}

void vidFrameV9938(Video* vid) {
	if (vid->v9938.cbFram)
		vid->v9938.cbFram(&vid->v9938);
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

void vidNESFram(Video* vid) {
	ppuFram(vid->ppu);
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
	{VID_NES, vidNESDraw, NULL, vidNESLine, vidNESFram},
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
	while (vid->nsDraw >= vid->nsPerDot) {

		vid->nsDraw -= vid->nsPerDot;
		vid->time += vid->nsPerDot;

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
			vid->v9938.sr[0] |= 0x80;
		}
	}
}
