#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define NS_PER_DOT	140

#include "video.h"

int vidFlag = 0;
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
	vid->lcut.h = (int)floor(vid->sync.h + ((vid->bord.h - vid->sync.h) * (1.0 - brdsize))) & 0xfffe;
	vid->lcut.v = (int)floor(vid->sync.v + ((vid->bord.v - vid->sync.v) * (1.0 - brdsize))) & 0xfffe;
	vid->rcut.h = (int)floor(vid->full.h - ((1.0 - brdsize) * (vid->full.h - vid->bord.h - 256))) & 0xfffe;
	vid->rcut.v = (int)floor(vid->full.v - ((1.0 - brdsize) * (vid->full.v - vid->bord.v - 192))) & 0xfffe;
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

/*
	mtrxItem* mtx;
	unsigned char* ptr = vid->scrptr;
	int idx = vid->dotCount;
	do {
		mtx = &vid->matrix[idx];
		idx++;
		if (mtx->flag & MTF_VISIBLE) {
			*(ptr++) &= 0x0f;
			if (vidFlag & VF_DOUBLE) *(ptr++) &= 0x0f;
		}
		if ((mtx->flag & MTF_LINEND) && (vidFlag & VF_DOUBLE)) {
			memcpy(ptr,ptr - vid->wsze.h,vid->wsze.h);
			ptr += vid->wsze.h;
		}
	} while (~mtx->flag & MTF_FRMEND);
*/
}

int vidGetWait(Video* vid) {
	return 0; //vid->matrix[vid->dotCount].wait;
}

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

// common 256 x 192
void vidDrawNormal(Video* vid) {
	if ((vid->y < vid->bord.v) || (vid->y > vid->bord.v + 191)) {
		col = vid->brdcol;
	} else {
		xscr = vid->x - vid->bord.h;
		yscr = vid->y - vid->bord.v;
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
	if ((vid->y < vid->bord.v) || (vid->y > vid->bord.v + 191)) {
		col = vid->brdcol;
	} else {
		if ((vid->x < vid->bord.h) || (vid->x > vid->bord.h + 255)) {
			col = vid->brdcol;
		} else {
			xscr = vid->x - vid->bord.h;
			yscr = vid->y - vid->bord.v;
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
	}
	vidPutDot(vid,col);
}

// hardware multicolor
void vidDrawHwmc(Video* vid) {
	if ((vid->y < vid->bord.v) || (vid->y > vid->bord.v + 191)) {
		col = vid->brdcol;
	} else {
		xscr = vid->x - vid->bord.h;
		yscr = vid->y - vid->bord.v;
		if ((xscr & 7) == 4) {
			adr = ((yscr & 0xc0) << 5) | ((yscr & 7) << 8) | ((yscr & 0x38) << 2) | (((xscr + 4) & 0xf8) >> 3);
			nxtbyte = vid->mem->ram[vid->curscr ? 7 :5].data[adr];
		}
		if ((vid->x < vid->bord.h) || (vid->x > vid->bord.h + 255)) {
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
	if ((vid->y < 76) || (vid->y > 275) || (vid->x < 96) || (vid->x > 415)) {
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

// atm text
void vidATMDoubleDot(Video* vid,unsigned char colr) {
	ink = inkTab[colr & 0x7f];
	pap = papTab[colr & 0x3f] | ((colr & 0x80) >> 4);
	scrbyte = vid->font[(scrbyte << 3) | (yscr & 7)];
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
	if ((vid->y < 76) || (vid->y > 275) || (vid->x < 96) || (vid->x > 415)) {
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
			vidATMDoubleDot(vid,col);
		}
		vid->scrptr++;
		if (vidFlag & VF_DOUBLE) vid->scrptr++;
	}
}

// atm hardware multicolor
void vidDrawATMhwmc(Video* vid) {
	if ((vid->y < 76) || (vid->y > 275) || (vid->x < 96) || (vid->x > 415)) {
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

// tsconf 4bpp

void vidDrawTSL16(Video* vid) {
	xscr = vid->x - vid->tsconf.xPos;
	yscr = vid->y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr > vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize)) {
		vidPutDot(vid,vid->brdcol);
	} else {
		xscr += (vid->tsconf.xOffset & 0x01ff);
		yscr += (vid->tsconf.yOffset & 0x01ff);
		adr = (yscr << 8) | (xscr >> 1);
		scrbyte = vid->mem->ram[(vid->tsconf.vidPage & 0xf8) + (adr >> 14)].data[adr & 0x3fff];
		if (xscr & 1) {
			col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);	// right pixel
		} else {
			col = inkTab[scrbyte & 0x7f];				// left pixel
		}
		vidPutDot(vid,col);
	}
}

// tsconf 8bpp

void vidDrawTSL256(Video* vid) {
	xscr = vid->x - vid->tsconf.xPos;
	yscr = vid->y - vid->tsconf.yPos;
	if ((xscr < 0) || (xscr > vid->tsconf.xSize) || (yscr < 0) || (yscr >= vid->tsconf.ySize)) {
		vidPutDot(vid,vid->brdcol);
	} else {
		xscr += (vid->tsconf.xOffset & 0x01ff);
		yscr += (vid->tsconf.yOffset & 0x01ff);
		adr = (yscr << 9) | xscr;
		col = vid->mem->ram[(vid->tsconf.vidPage & 0xf8) + (adr >> 14)].data[adr & 0x3fff];
		*(vid->scrptr++) = col;
		if (vidFlag & VF_DOUBLE) *(vid->scrptr++) = col;
	}
}

// weiter

void vidSetMode(Video* vid, int mode) {
	vid->vmode = mode;
	switch (mode) {
		case VID_NOSCREEN: vid->callback = &vidDrawBorder; break;
		case VID_NORMAL: vid->callback = &vidDrawNormal; break;
		case VID_ALCO: vid->callback = &vidDrawAlco; break;
		case VID_HWMC: vid->callback = &vidDrawHwmc; break;
		case VID_ATM_EGA: vid->callback = &vidDrawATMega; break;
		case VID_ATM_TEXT: vid->callback = &vidDrawATMtext; break;
		case VID_ATM_HWM: vid->callback = &vidDrawATMhwmc; break;
		case VID_TSL_16: vid->callback = &vidDrawTSL16; break;
		case VID_TSL_256: vid->callback = &vidDrawTSL256; break;
		default: vid->callback = &vidDrawUnknown; break;
	}
}

void vidSync(Video* vid, int ns) {
	vid->nsDraw += ns;
	while (vid->nsDraw >= NS_PER_DOT) {
		if ((vid->y >= vid->lcut.v) && (vid->y < vid->rcut.v)) {
			if ((vid->x >= vid->lcut.h) && (vid->x < vid->rcut.h)) {
				if (vid->x & 8) vid->brdcol = vid->nextbrd;
				vid->callback(vid);		// put dot
			}
		}
		if (++vid->x >= vid->full.h) {
			vid->x = 0;
			if ((vid->y >= vid->lcut.v) && (vid->y < vid->rcut.v) && (vidFlag & VF_DOUBLE)) {
				memcpy(vid->scrptr, vid->scrptr - vid->wsze.h, vid->wsze.h);
				vid->scrptr += vid->wsze.h;
			}
			if (++vid->y >= vid->full.v) {
				vid->y = 0;
				vid->scrptr = vid->scrimg;
				vid->fcnt++;
				vid->flash = vid->fcnt & 0x20;
			}
		}
		vid->nsDraw -= NS_PER_DOT;
	}
}
