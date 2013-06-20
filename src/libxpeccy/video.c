#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define NS_PER_DOT	140

#include "video.h"

int vidFlag = VF_BLOCK;
float brdsize = 1.0;

unsigned char inkTab[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

unsigned char papTab[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
  0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
  0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};

Video* vidCreate(Memory* me) {
	Video* vid = (Video*)malloc(sizeof(Video));
	vid->mem = me;
	vid->full.h = 448;
	vid->full.v = 320;
	vid->bord.h = 128;
	vid->bord.v = 80;
	vid->sync.h = 80;
	vid->sync.v = 32;
	vid->intpos.h = 0;
	vid->intpos.v = 0;
	vid->intsz = 64;
	vid->frmsz = vid->full.h * vid->full.v;
	vidUpdate(vid);

	vidSetMode(vid,VID_NORMAL);

	vid->nextbrd = 0;
	vid->curscr = 0;
	vid->fcnt = 0;

	vid->nsDraw = 0;
	vid->x = 0;
	vid->y = 0;

	vid->scrimg = NULL;
	vid->scrptr = vid->scrimg;

	vid->flags = 0;

	return vid;
}

void vidDestroy(Video* vid) {
	free(vid);
}

/*
waits for 128K, +2
	--wwwwww wwwwww-- : 16-dots cycle, start on border 8 dots before screen
waits for +2a, +3
	ww--wwww wwwwwwww : same
unsigned char waitsTab_A[16] = {5,5,4,4,3,3,2,2,1,1,0,0,0,0,6,6};	// 48K
unsigned char waitsTab_B[16] = {1,1,0,0,7,7,6,6,5,5,4,4,3,3,2,2};	// +2A,+3
*/

void vidUpdate(Video* vid) {
	vid->lcut.h = (int)floor(vid->sync.h + ((vid->bord.h - vid->sync.h) * (1.0 - brdsize))) & 0xfffc;
	vid->lcut.v = (int)floor(vid->sync.v + ((vid->bord.v - vid->sync.v) * (1.0 - brdsize))) & 0xfffc;
	vid->rcut.h = (int)floor(vid->full.h - ((1.0 - brdsize) * (vid->full.h - vid->bord.h - 256))) & 0xfffc;
	vid->rcut.v = (int)floor(vid->full.v - ((1.0 - brdsize) * (vid->full.v - vid->bord.v - 192))) & 0xfffc;
	vid->vsze.h = vid->rcut.h - vid->lcut.h;
	vid->vsze.v = vid->rcut.v - vid->lcut.v;
	vid->wsze.h = vid->vsze.h * ((vidFlag & VF_DOUBLE) ? 2 : 1);
	vid->wsze.v = vid->vsze.v * ((vidFlag & VF_DOUBLE) ? 2 : 1);
}

int xscr = 0;
int yscr = 0;
int adr = 0;
unsigned char col = 0;
unsigned char ink = 0;
unsigned char pap = 0;
unsigned char scrbyte = 0;
unsigned char nxtbyte = 0;
//unsigned char* fntptr;

void vidDarkTail(Video* vid) {
	xscr = vid->x;
	yscr = vid->y;
	unsigned char* ptr = vid->scrptr;
	do {
		if ((yscr >= vid->lcut.v) && (yscr < vid->rcut.v) && (xscr >= vid->lcut.h) && (xscr < vid->rcut.h)) {
			*(ptr++) &= 0x0f;
			if (vidFlag & VF_DOUBLE) {
				*(ptr++) &= 0x0f;
			}
		}
		if (++xscr >= vid->full.h) {
			xscr = 0;
			if ((yscr >= vid->lcut.v) && (yscr < vid->rcut.v) && (vidFlag & VF_DOUBLE)) {
				memcpy(ptr,ptr - vid->wsze.h,vid->wsze.h);
				ptr += vid->wsze.h;
			}
			if (++yscr >= vid->full.v)
				ptr = NULL;
		}
	} while (ptr);
}

//int vidGetWait(Video* vid) {
//	return 0; //vid->matrix[vid->dotCount].wait;
//}

void vidSetFont(Video* vid, char* src) {
	memcpy(vid->font,src,0x800);
}

unsigned char vidGetAttr(Video* vid) {
	return 0xff; // (vid->matrix[vid->dotCount].flag & MTF_SCREEN) ? vid->atrbyte : vid->brdcol;
}

inline void vidPutDot(Video* vid, unsigned char colr) {
	if ((vidFlag & (VF_NOFLIC | VF_FRAMEDBG)) == VF_NOFLIC) {
		colr = (((*vid->scrptr) & 0x0f) << 4) | (colr & 0x0f);
	} else {
		colr |= (colr << 4);
	}
//	if ((~vidFlag & VF_CHECKCHA) || (*vid->scrptr != colr)) vidFlag |= VF_CHANGED;
	*(vid->scrptr++) = colr;
	if (vidFlag & VF_DOUBLE) *(vid->scrptr++)=colr;
}

// video drawing

void vidDrawUnknown(Video* vid) {
	vidPutDot(vid,0);
}

void vidDrawBorder(Video* vid) {
	vidPutDot(vid,vid->brdcol);
}

void vidBorderTop(Video* vid) {
	if (vid->y < vid->scrPos) {
		vidPutDot(vid,vid->brdcol);
	} else {
		vid->callback = vid->vidScrCallback;
		vid->callback(vid);
	}
}

void vidBorderBottom(Video* vid) {
	if (vid->y > vid->scrPos) {
		vidPutDot(vid,vid->brdcol);
	} else {
		vid->callback = &vidBorderTop;
		vid->callback(vid);
	}
}

// common 256 x 192
void vidDrawNormal(Video* vid) {
	yscr = vid->y - vid->scrPos;
	if (yscr > 191) {
		vid->callback = &vidBorderBottom;
		vid->callback(vid);
	} else {
		xscr = vid->x - vid->bord.h;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
		}
		if ((vid->x < vid->bord.h) || (vid->x > vid->bord.h + 255)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = 0x1800 | ((yscr & 0xc0) << 2) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
				vid->atrbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = inkTab[vid->atrbyte & 0x7f];
				pap = papTab[vid->atrbyte & 0x7f];
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vidPutDot(vid,col);
}

// alco 16col
void vidDrawAlco(Video* vid) {
	yscr = vid->y - vid->scrPos;
	if (yscr > 191) {
		vid->callback = &vidBorderBottom;
		vid->callback(vid);
	} else {
		xscr = vid->x - vid->bord.h;
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->brdcol;
		} else {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
			switch (xscr & 7) {
				case 0:
					scrbyte = vid->mem->ram[vid->curscr ? 6 : 4].data[adr];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 2:
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 4:
					scrbyte = vid->mem->ram[vid->curscr ? 6 : 4].data[adr + 0x2000];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 6:
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr + 0x2000];
					col = inkTab[scrbyte & 0x7f];
					break;
				default:
					col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
					break;

			}
		}
		vidPutDot(vid,col);
	}
}

// hardware multicolor
void vidDrawHwmc(Video* vid) {
	yscr = vid->y - vid->scrPos;
	if (yscr > 191) {
		vid->callback = &vidBorderBottom;
		vid->callback(vid);
	} else {
		xscr = vid->x - vid->bord.h;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
		}
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | ((xscr & 0xf8) >> 3);
				vid->atrbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = inkTab[vid->atrbyte & 0x7f];
				pap = papTab[vid->atrbyte & 0x7f];
			}
			col = (scrbyte & 0x80) ? ink : pap;
			scrbyte <<= 1;
		}
	}
	vidPutDot(vid,col);
}

// atm ega
void vidDrawATMega(Video* vid) {
	if (vid->y > 275) {
		vid->callback = &vidBorderBottom;
		vid->callback(vid);
	} else {
		if ((vid->x < 96) || (vid->x > 415)) {
			col = vid->brdcol;
		} else {
			xscr = vid->x - 96;
			yscr = vid->y - 76;
			adr = (yscr * 40) + (xscr >> 3);
			switch (xscr & 7) {
				case 0:
					scrbyte = vid->mem->ram[vid->curscr ? 3 : 1].data[adr];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 2:
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 4:
					scrbyte = vid->mem->ram[vid->curscr ? 3 : 1].data[adr + 0x2000];
					col = inkTab[scrbyte & 0x7f];
					break;
				case 6:
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr + 0x2000];
					col = inkTab[scrbyte & 0x7f];
					break;
				default:
					col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
					break;
			}
		}
		vidPutDot(vid,col);
	}
}

// atm text
void vidATMDoubleDot(Video* vid,unsigned char colr) {
	ink = inkTab[colr & 0x7f];
	pap = papTab[colr & 0x3f] | ((colr & 0x80) >> 4);
	if (vidFlag & VF_DOUBLE) {
		*(vid->scrptr) = (scrbyte & 0x80) ? ink : pap;
		*(vid->scrptr + 1) = (scrbyte & 0x40) ? ink : pap;
		*(vid->scrptr + 2) = (scrbyte & 0x20) ? ink : pap;
		*(vid->scrptr + 3) = (scrbyte & 0x10) ? ink : pap;
		*(vid->scrptr + 4) = (scrbyte & 0x08) ? ink : pap;
		*(vid->scrptr + 5) = (scrbyte & 0x04) ? ink : pap;
		*(vid->scrptr + 6) = (scrbyte & 0x02) ? ink : pap;
		*(vid->scrptr + 7) = (scrbyte & 0x01) ? ink : pap;
	} else {
		*(vid->scrptr) = (scrbyte & 0xc0) ? ink : pap;
		*(vid->scrptr + 1) = (scrbyte & 0x30) ? ink : pap;
		*(vid->scrptr + 2) = (scrbyte & 0x0c) ? ink : pap;
		*(vid->scrptr + 3) = (scrbyte & 0x03) ? ink : pap;
	}
//	vidFlag |= VF_CHANGED;
}

void vidDrawATMtext(Video* vid) {
	if (vid->y > 275) {
		vid->callback = &vidBorderBottom;
		vid->callback(vid);
	} else {
		if ((vid->x < 96) || (vid->x > 415)) {
			vidPutDot(vid,vid->brdcol);
		} else {
			xscr = vid->x - 96;
			yscr = vid->y - 76;
			adr = 0x1c0 + ((yscr & 0xf8) << 3) + (xscr >> 3);
			if ((xscr & 3) == 0) {
				if ((xscr & 7) == 0) {
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr];
					col = vid->mem->ram[vid->curscr ? 3 : 1].data[adr + 0x2000];
				} else {
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr + 0x2000];
					col = vid->mem->ram[vid->curscr ? 3 : 1].data[adr + 1];
				}
				scrbyte = vid->font[(scrbyte << 3) | (yscr & 7)];
				vidATMDoubleDot(vid,col);
			}
			vid->scrptr++;
			if (vidFlag & VF_DOUBLE) vid->scrptr++;
		}
	}
}

// atm hardware multicolor
void vidDrawATMhwmc(Video* vid) {
	if (vid->y > 275) {
		vid->callback = &vidBorderBottom;
		vid->callback(vid);
	} else {
		if ((vid->x < 96) || (vid->x > 415)) {
			vidPutDot(vid,vid->brdcol);
		} else {
			xscr = vid->x - 96;
			yscr = vid->y - 76;
			adr = (yscr * 40) + (xscr >> 3);
			if ((xscr & 3) == 0) {
				if ((xscr & 7) == 0) {
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr];
					col = vid->mem->ram[vid->curscr ? 3 : 1].data[adr];
				} else {
					scrbyte = vid->mem->ram[vid->curscr ? 7 : 5].data[adr + 0x2000];
					col = vid->mem->ram[vid->curscr ? 3 : 1].data[adr + 0x2000];
				}
				vidATMDoubleDot(vid,col);
			}
			vid->scrptr++;
			if (vidFlag & VF_DOUBLE) vid->scrptr++;
		}
	}
}

// tsconf sprites & tiles

void vidTSPut(Video* vid, unsigned char* ptr) {
	int ofs = 0;
	adr = vid->tsconf.xPos - vid->lcut.h;
	if (adr > 0) {
		ptr += adr;
		if (vidFlag & VF_DOUBLE) ptr += adr;
	}
	for (xscr = 0; xscr < vid->tsconf.xSize; xscr++) {
		if ((adr >= 0) && (adr < vid->vsze.h)) {			// visible
			col = vid->tsconf.line[ofs];
			if (col & 0x0f) {					// not transparent
				*(ptr++) = col;
				if (vidFlag & VF_DOUBLE) *(ptr++) = col;
			} else {
				ptr++;
				if (vidFlag & VF_DOUBLE) ptr++;
			}
		}
		adr++;
		ofs++;
	}
}

int fadr;
int tile;
int sadr;	// adr in sprites dsc
int xadr;	// = pos with XFlip
unsigned char* mptr;

#include <assert.h>

void vidTSTiles(Video* vid, int lay, unsigned short yoffs, unsigned short xoffs, unsigned char gpage, unsigned char palhi) {
	int j;
	yscr = vid->y - vid->tsconf.yPos + yoffs;						// line in TMap
	adr = (vid->tsconf.TMPage << 14) | ((yscr & 0xf8) << 5) | (lay ? 0x80 : 0x00);		// start of TMap line (full.adr)
	xscr = (0x200 - xoffs) & 0x1ff;									// pos in line buf
	xadr = vid->tsconf.tconfig & (lay ? 8 : 4);
	do {								// 64 tiles in row
		tile = vid->mem->ramData[adr] | (vid->mem->ramData[adr + 1] << 8);		// tile dsc
		adr += 2;

		if ((tile & 0xfff) || xadr) {			// !0 or (0 enabled)
			fadr = gpage << 14;
			fadr += ((tile & 0xfc0) << 5) | ((yscr & 7) << 8) | ((tile & 0x3f) << 2);	// full addr of row of this tile
			if (tile & 0x8000) fadr ^= 0x0700;						// YFlip

			col = palhi | ((tile >> 8) & 0x30);					// palette (b7..4 of color)
			if (tile & 0x4000) {							// XFlip
				xscr += 8;
				for (j = 0; j < 4; j++) {
					col &= 0xf0;
					col |= (vid->mem->ramData[fadr] & 0xf0) >> 4;				// left pixel
					xscr--;
					if (col & 0x0f) vid->tsconf.line[xscr & 0x1ff] = col;
					col &= 0xf0;
					col |= vid->mem->ramData[fadr] & 0x0f;					// right pixel
					xscr--;
					if (col & 0x0f) vid->tsconf.line[xscr & 0x1ff] = col;
					fadr++;
				}
				xscr += 8;
			} else {								// no XFlip
				for (j = 0; j < 4; j++) {
					col &= 0xf0;
					col |= (vid->mem->ramData[fadr] & 0xf0) >> 4;				// left pixel
					if (col & 0x0f) {
						vid->tsconf.line[xscr & 0x1ff] = col;
					}
					xscr++;
					col &= 0xf0;
					col |= vid->mem->ramData[fadr] & 0x0f;					// right pixel
					if (col & 0x0f) {
						vid->tsconf.line[xscr & 0x1ff] = col;
					}
					xscr++;
					fadr++;
				}
			}
		} else {
			xscr += 8;
		}
	} while (adr & 0x7f);
}

void vidTSSprites(Video* vid) {
//	memset(vid->tsconf.line,0x00,512);
	while (sadr < (0x400 - 6)) {
		mptr = &vid->tsconf.tsAMem[sadr];
		if (mptr[1] & 0x20) {						// ACT
			adr = mptr[0] | ((mptr[1] & 1) << 8);			// Ypos of spr
			xscr = ((mptr[1] & 0x0e) + 2) << 2;			// Ysize - 000:8; 001:16; 010:24; ...
			yscr = vid->y - vid->tsconf.yPos;			// line on screen
			if (((yscr - adr) & 0x1ff) < xscr) {			// if sprite visible on current line
				yscr -= adr;					// line inside sprite;
				if (mptr[1] & 0x80) yscr = xscr - yscr - 1;	// YFlip (Ysize - norm.pos - 1)
				tile = mptr[4] | ((mptr[5] & 0x0f) << 8);	// pos of 1st tile
				tile += ((yscr & 0x1f8) << 3);			// shift to current tile line

				fadr = vid->tsconf.SGPage << 14;
				fadr += ((tile & 0xfc0) << 5) | ((yscr & 7) << 8) | ((tile & 0x3f) << 2);	// fadr = adr of pix line to put in buf

				adr = mptr[2] | ((mptr[3] & 1) << 8);		// Xpos (First pixel adr)
				col = (mptr[5] & 0xf0);
				xadr = adr;
				if (mptr[3] & 0x80) xadr += (((mptr[3] & 0x0e) + 2) << 2) - 1;
				for (xscr = ((mptr[3] & 0x0e) + 2) << 2; xscr > 0; xscr-=2) {
					col &= 0xf0;
					col |= ((vid->mem->ramData[fadr] & 0xf0) >> 4);		// left pixel;
					if (col & 0x0f) vid->tsconf.line[xadr & 0x1ff] = col;
					if (mptr[3] & 0x80) xadr--; else xadr++;
					adr++;
					col &= 0xf0;
					col |= (vid->mem->ramData[fadr] & 0x0f);		// right pixel
					if (col & 0x0f) vid->tsconf.line[xadr & 0x1ff] = col;
					if (mptr[3] & 0x80) xadr--; else xadr++;
					adr++;
					fadr++;
				}
			}
		}
		if (mptr[sadr + 1] & 0x40) break;		// LEAP
		sadr += 6;
	}
	sadr += 6;
}

void vidTSRender(Video* vid, unsigned char* ptr) {
	if (vid->y < vid->tsconf.yPos) return;
	if (vid->y >= (vid->tsconf.yPos + vid->tsconf.ySize)) return;
// prepare layers in VID->LINE & out visible part to PTR
	sadr = 0x200;			// start of sprites tab in altera mem
	memset(vid->tsconf.line,0x00,0x200);		// clear tile-sprite line
// S0 ?
//	if (vid->tsconf.tconfig & 0x80) {
//		vidTSSprites(vid);
//	}
// T0
	if (vid->tsconf.tconfig & 0x20) {
		vidTSTiles(vid,0,vid->tsconf.T0YOffset,vid->tsconf.T0XOffset,vid->tsconf.T0GPage,vid->tsconf.T0Pal76);
	}
// S1
	if (vid->tsconf.tconfig & 0x80) {
		vidTSSprites(vid);
	}
// T1
	if (vid->tsconf.tconfig & 0x40) {
		vidTSTiles(vid,1,vid->tsconf.T1YOffset,vid->tsconf.T1XOffset,vid->tsconf.T1GPage,vid->tsconf.T1Pal76);
	}
// S2
	if (vid->tsconf.tconfig & 0x80) {
		vidTSSprites(vid);
	}
	if (vid->tsconf.tconfig & 0xe0) vidTSPut(vid,ptr);		// if tile or sprites visible, put line over screen
	vid->tsconf.scrLine++;
}

// tsconf normal screen (separated 'cuz of palette)

void vidDrawTSLNormal(Video* vid) {
	xscr = vid->x - vid->tsconf.xPos;
	yscr = vid->y - vid->tsconf.yPos;
	if ((yscr < 0) || (yscr > 191) || (vid->flags & VID_NOGFX)) {
		col = vid->brdcol;
	} else {
		xadr = vid->tsconf.vidPage ^ (vid->curscr ? 2 : 0);	// TODO : ORLY? Current video page
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mem->ram[xadr].data[adr];
		}
		if ((xscr < 0) || (xscr > 255)) {
			col = vid->brdcol;
		} else {
			if ((xscr & 7) == 0) {
				scrbyte = nxtbyte;
				adr = 0x1800 | ((yscr & 0xc0) << 2) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
				vid->atrbyte = vid->mem->ram[xadr].data[adr];
				if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 0xff;
				ink = inkTab[vid->atrbyte & 0x7f];
				pap = papTab[vid->atrbyte & 0x7f];
			}
			col = vid->tsconf.scrPal | ((scrbyte & 0x80) ? ink : pap);
			scrbyte <<= 1;
		}
	}
	*(vid->scrptr++) =  col;
	if (vidFlag & VF_DOUBLE) *(vid->scrptr++) = col;
}

// tsconf 4bpp

void vidDrawTSL16(Video* vid) {
	xscr = vid->x - vid->tsconf.xPos;
	yscr = vid->y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr >= vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize)) {
		col = vid->brdcol;
	} else if (vid->flags & VID_NOGFX) {
		col = vid->brdcol;
	} else {
		xscr += vid->tsconf.xOffset;
		yscr = vid->tsconf.scrLine;
		xscr &= 0x1ff;
		yscr &= 0x1ff;
		adr = ((vid->tsconf.vidPage & 0xf8) << 14) + (yscr << 8) + (xscr >> 1);
		scrbyte = vid->mem->ramData[adr];
		if (xscr & 1) {
			col = scrbyte & 0x0f;			// right pixel
		} else {
			col = (scrbyte & 0xf0) >> 4 ;		// left pixel
		}
		col |= vid->tsconf.scrPal;
	}
	*(vid->scrptr++) = col;
	if (vidFlag & VF_DOUBLE) *(vid->scrptr++) = col;
}

// tsconf 8bpp

void vidDrawTSL256(Video* vid) {
	xscr = vid->x - vid->tsconf.xPos;
	yscr = vid->y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr >= vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize) || (vid->flags & VID_NOGFX)) {
		col = vid->brdcol;
	} else {
		xscr += vid->tsconf.xOffset;
		yscr = vid->tsconf.scrLine;
		xscr &= 0x1ff;
		yscr &= 0x1ff;
		adr = (vid->tsconf.vidPage << 14) + (yscr << 9) + xscr;
		col = vid->mem->ramData[adr];
	}
	*(vid->scrptr++) = col;
	if (vidFlag & VF_DOUBLE) *(vid->scrptr++) = col;
}

// tsconf text (not yet)

void vidDrawTSLText(Video* vid) {
	vidPutDot(vid,vid->brdcol);
}

// weiter

#define	TC_LAYOUT	1
#define	TC_ATM		2
#define	TC_TSL		3

typedef struct {
	int id;
	void(*callback)(Video*);
	int topCtrl;
} vidModeItem;

vidModeItem vidModeTab[] = {
	{VID_NOSCREEN,&vidDrawBorder,TC_LAYOUT},
	{VID_NORMAL,&vidDrawNormal,TC_LAYOUT},
	{VID_ALCO,&vidDrawAlco,TC_LAYOUT},
	{VID_HWMC,&vidDrawHwmc,TC_LAYOUT},
	{VID_ATM_EGA,&vidDrawATMega,TC_ATM},
	{VID_ATM_TEXT,&vidDrawATMtext,TC_ATM},
	{VID_ATM_HWM,&vidDrawATMhwmc,TC_ATM},
	{VID_TSL_NORMAL,&vidDrawTSLNormal,TC_LAYOUT},
	{VID_TSL_16,&vidDrawTSL16,TC_TSL},
	{VID_TSL_256,&vidDrawTSL256,TC_TSL},
	{VID_TSL_TEXT,&vidDrawTSLText,TC_TSL},
	{-1,NULL,TC_LAYOUT}
};

void vidSetMode(Video* vid, int mode) {
	vid->vmode = mode;
	int idx = 0;
	vid->vidScrCallback = &vidDrawBorder;
	while (vidModeTab[idx].id != -1) {
		if (vidModeTab[idx].id == mode) {
			vid->vidScrCallback = vidModeTab[idx].callback;
			break;
		}
		idx++;
	}
	switch(vidModeTab[idx].topCtrl) {
		case TC_ATM: vid->scrPos = 76; break;
		case TC_TSL: vid->scrPos = vid->tsconf.yPos; break;
		default: vid->scrPos = vid->bord.v; break;
	}
	vid->callback = &vidBorderTop;
}

void vidSync(Video* vid, int ns) {
	vid->nsDraw += ns;
	while (vid->nsDraw >= NS_PER_DOT) {
		if ((vid->y >= vid->lcut.v) && (vid->y < vid->rcut.v)) {
			if ((vid->x >= vid->lcut.h) && (vid->x < vid->rcut.h)) {
				if (vid->x & 8) vid->brdcol = vid->nextbrd;
				if (vid->scrimg == NULL) printf("NULL\n");
				vid->callback(vid);		// put dot
			}
		}
		if ((vid->x == vid->intpos.h) && (vid->y == vid->intpos.v)) vid->flags |= VID_INTSTROBE;
		if (++vid->x >= vid->full.h) {
			vid->x = 0;
			if (vid->flags & VF_TSCONF) vidTSRender(vid, vid->scrptr - vid->wsze.h);
			if ((vid->y >= vid->lcut.v) && (vid->y < vid->rcut.v) && (vidFlag & VF_DOUBLE)) {
				if (vid->scrimg == NULL) printf("NULL\n");
				memcpy(vid->scrptr, vid->scrptr - vid->wsze.h, vid->wsze.h);
				vid->scrptr += vid->wsze.h;
			}
			if (++vid->y >= vid->full.v) {
				vid->y = 0;
				vid->scrptr = vid->scrimg;
				vid->fcnt++;
				vid->flash = vid->fcnt & 0x20;
				vid->tsconf.scrLine = vid->tsconf.yOffset;
			}
		}
		vid->nsDraw -= NS_PER_DOT;
	}
}
