#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define NS_PER_DOT	140

#include "video.h"

unsigned char* screenBuf = NULL;
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
	if (screenBuf == NULL) {
		screenBuf = (unsigned char*)malloc(1024 * 1024 * sizeof(unsigned char));
	}
	vid->mem = me;
/*
	int i,j,k,l;
	int idx=0;
	int sadr=0x0000;
	int aadr=0x1800;
	for (i=0;i<3;i++) {
		for (j=0;j<8;j++) {
			for (k=0;k<8;k++) {
				for (l=0;l<32;l++) {
					vid->scr5pix[idx] = memGetPagePtr(me,MEM_RAM,5) + sadr;
					vid->scr5atr[idx] = memGetPagePtr(me,MEM_RAM,5) + aadr;
					vid->scr7pix[idx] = memGetPagePtr(me,MEM_RAM,7) + sadr;
					vid->scr7atr[idx] = memGetPagePtr(me,MEM_RAM,7) + aadr;
					vid->ladrz[idx].ac00 = memGetPagePtr(me,MEM_RAM,4) + sadr;
					vid->ladrz[idx].ac01 = memGetPagePtr(me,MEM_RAM,5) + sadr;
					vid->ladrz[idx].ac02 = memGetPagePtr(me,MEM_RAM,4) + sadr + 0x2000;
					vid->ladrz[idx].ac03 = memGetPagePtr(me,MEM_RAM,5) + sadr + 0x2000;
					vid->ladrz[idx].ac10 = memGetPagePtr(me,MEM_RAM,6) + sadr;
					vid->ladrz[idx].ac11 = memGetPagePtr(me,MEM_RAM,7) + sadr;
					vid->ladrz[idx].ac12 = memGetPagePtr(me,MEM_RAM,6) + sadr + 0x2000;
					vid->ladrz[idx].ac13 = memGetPagePtr(me,MEM_RAM,7) + sadr + 0x2000;
					idx++;
					sadr++;
					aadr++;
				}
				sadr += 0xe0;	// -#20 +#100
				aadr -= 0x20;
			}
			sadr -= 0x7e0;		// -#800 +#20
			aadr += 0x20;
		}
		sadr += 0x700;			// -#100 +#800
	}
*/
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

	vid->curscr = 0;
	vid->fcnt = 0;

//	vid->dotCount = 0;
//	mtx = vid->matrix;
	vid->nsDraw = 0;

	vid->x = 0;
	vid->y = 0;

	vid->scrimg = screenBuf;
	vid->scrptr = vid->scrimg;

	vid->flags = 0;
//	vid->intSignal = 0;

	return vid;
}

void vidDestroy(Video* vid) {
	free(vid);
}

unsigned char* vidGetScreen() {
	return screenBuf;
}
// dot flags
#define	MTF_LINEND	1
#define	MTF_FRMEND	(1<<1)
#define	MTF_INT		(1<<2)
#define	MTF_INTSTRB	(1<<3)
#define MTF_2P		(1<<4)
#define MTF_4P		(1<<5)
#define MTF_8P		(1<<6)
#define	MTF_VISIBLE	(1<<7)
#define MTF_SCREEN	(1<<8)
#define	MTF_PIX		(1<<9)
#define	MTF_ATR		(1<<10)
#define MTF_ATMSCR	(1<<11)
#define	MTF_ATMTXT	(1<<12)

/*
waits for 128K, +2
	--wwwwww wwwwww-- : 16-dots cycle, start on border 8 dots before screen
waits for +2a, +3
	ww--wwww wwwwwwww : same
*/

/*
#include <assert.h>

unsigned char waitsTab_A[16] = {5,5,4,4,3,3,2,2,1,1,0,0,0,0,6,6};	// 48K
unsigned char waitsTab_B[16] = {1,1,0,0,7,7,6,6,5,5,4,4,3,3,2,2};	// +2A,+3

void vidFillMatrix(Video* vid) {
	int x,y,i,adr,atmadr,atmTadr;
	int tk = 0;
	i = 0;
	atmadr = 0;
	atmTadr = 0x01c0;
	adr = 0;
	int scrShift = vid->bord.h;
	for (y = 0; y < vid->full.v; y++) {
		for (x = 0; x < vid->full.h; x++) {
			vid->matrix[i].flag = 0;
			if ((x & 1) == 0) vid->matrix[i].flag |= MTF_2P;
			if ((x & 3) == 0) vid->matrix[i].flag |= MTF_4P;
			if ((x & 7) == 0) vid->matrix[i].flag |= MTF_8P;
			vid->matrix[i].wait = 0;
// waits: dots from -2 dots to +253 (254 is already non-wait)
			if ((x > (vid->bord.h - 3)) && (x < (vid->bord.h + 254)) && (y >= vid->bord.v) && (y < (vid->bord.v + 192))) {
				vid->matrix[i].wait = waitsTab_A[(x - vid->bord.h) & 15];
			}
// atm ega & text mode
			if ((y > 75) && (y < 276) && (x > 95) && (x < 416)) {
				vid->matrix[i].flag |= MTF_ATMSCR;
				switch ((x - 96) & 7) {
					case 0:
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmadr;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmadr;
						vid->matrix[i].atm5.hwmpix = memGetPagePtr(vid->mem,MEM_RAM,5) + atmadr;
						vid->matrix[i].atm5.hwmatr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmadr;
						vid->matrix[i].atm7.hwmpix = memGetPagePtr(vid->mem,MEM_RAM,7) + atmadr;
						vid->matrix[i].atm7.hwmatr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmadr;
						if (((y - 76) & 7) == 0) {
							vid->matrix[i].flag |= MTF_ATMTXT;
							vid->matrix[i].atm5.txtptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmTadr;
							vid->matrix[i].atm7.txtptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmTadr;
							vid->matrix[i].atm5.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmTadr + 0x2000;
							vid->matrix[i].atm7.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmTadr + 0x2000;
						}
						break;
					case 2:
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmadr;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmadr;
						break;
					case 4:
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmadr + 0x2000;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmadr + 0x2000;
						vid->matrix[i].atm5.hwmpix = memGetPagePtr(vid->mem,MEM_RAM,5) + atmadr + 0x2000;
						vid->matrix[i].atm5.hwmatr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmadr + 0x2000;
						vid->matrix[i].atm7.hwmpix = memGetPagePtr(vid->mem,MEM_RAM,7) + atmadr + 0x2000;
						vid->matrix[i].atm7.hwmatr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmadr + 0x2000;
						if (((y - 76) & 7) == 0) {
							vid->matrix[i].flag |= MTF_ATMTXT;
							vid->matrix[i].atm5.txtptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmTadr + 0x2000;
							vid->matrix[i].atm7.txtptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmTadr + 0x2000;
							vid->matrix[i].atm5.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmTadr + 1;
							vid->matrix[i].atm7.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmTadr + 1;
							atmTadr++;
							if ((atmTadr & 0x3f) == 40) atmTadr = (atmTadr & 0x3fc0) + 0x40;	// next 64
						}
						break;
					case 6:
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmadr + 0x2000;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmadr + 0x2000;
						atmadr++;
						break;
				}
			}
// common screen | alco mode
			if ((y < vid->lcut.v) || (y >= vid->rcut.v) || (x < vid->lcut.h) || (x >= vid->rcut.h)) {
				// vid->matrix[i].type = MTT_INVIS;
			} else {
				vid->matrix[i].flag |= MTF_VISIBLE;
				if ((y < vid->bord.v) || (y > vid->bord.v + 191) || (x < scrShift) || (x > scrShift + 255)) {
					// vid->matrix[i].type = MTT_BORDER;
				} else {
					vid->matrix[i].flag |= MTF_SCREEN;
					switch ((x - scrShift) & 7) {
						case 0:
							vid->matrix[i].dotMask = 0x80;
							vid->matrix[i].flag |= MTF_ATR;
							vid->matrix[i].atr5ptr = vid->scr5atr[adr];
							vid->matrix[i].atr7ptr = vid->scr7atr[adr];
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac00;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac10;
							vid->matrix[i-4].flag |= MTF_PIX;
							vid->matrix[i-4].scr5ptr = vid->scr5pix[adr];
							vid->matrix[i-4].scr7ptr = vid->scr7pix[adr];
							break;
						case 1:
							vid->matrix[i].dotMask = 0x40;
							break;
						case 2:
							vid->matrix[i].dotMask = 0x20;
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac01;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac11;
							break;
						case 3:
							vid->matrix[i].dotMask = 0x10;
							break;
						case 4:
							vid->matrix[i].dotMask = 0x08;
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac02;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac12;
							break;
						case 5:
							vid->matrix[i].dotMask = 0x04;
							break;
						case 6:
							vid->matrix[i].dotMask = 0x02;
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac03;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac13;
							break;
						case 7:
							vid->matrix[i].dotMask = 0x01;
							adr++;
							break;
					}
				}
			}
			i++;
		}
		if ((y >= vid->lcut.v) && (y < vid->rcut.v)) vid->matrix[i-1].flag |= MTF_LINEND;
	}
	vid->matrix[i-1].flag |= MTF_FRMEND;
	adr = vid->intpos.v * vid->full.h + vid->intpos.h;
	vid->matrix[adr].flag |= MTF_INTSTRB;
	for (i = 0; i < vid->intsz; i++) {
		vid->matrix[adr].flag |= MTF_INT;
		adr++;
		if (adr >= vid->full.h * vid->full.v) adr = 0;
	}

	adr = vid->intpos.v * vid->full.h + vid->intpos.h;
	tk = 0;
	for (i = 0; i < vid->full.h * vid->full.v; i++) {
		if (tk < adr) {
			vid->matrix[i].tick = (tk + vid->full.h * vid->full.v - adr) / 2;
		} else {
			vid->matrix[i].tick = (tk - adr) / 2;
		}
		tk++;
	}
}
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
//	vidFillMatrix(vid);
}

int xscr = 0;
int yscr = 0;
int adr = 0;
unsigned char col = 0;
unsigned char ink = 0;
unsigned char pap = 0;
unsigned char scrbyte = 0;
unsigned char nxtbyte = 0;
unsigned char* fntptr;
//static mtrxItem* mtx = NULL;

void vidDarkTail(Video* vid) {
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
	if ((~vidFlag & VF_CHECKCHA) || (*vid->scrptr != colr)) vidFlag |= VF_CHANGED;
	*(vid->scrptr++) = colr;
	if (vidFlag & VF_DOUBLE) *(vid->scrptr++)=colr;
}

// video drawing

void vidDrawUnknown(Video* vid) {
	vidPutDot(vid,0);
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
	pap = papTab[colr & 0x3f] | ((col & 0x80) >> 4);
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
	vidFlag |= VF_CHANGED;
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

// weiter

void vidSetMode(Video* vid, int mode) {
	vid->vmode = mode;
	switch (mode) {
		case VID_NORMAL: vid->callback = &vidDrawNormal; break;
		case VID_ALCO: vid->callback = &vidDrawAlco; break;
		case VID_HWMC: vid->callback = &vidDrawHwmc; break;
		case VID_ATM_EGA: vid->callback = &vidDrawATMega; break;
		case VID_ATM_TEXT: vid->callback = &vidDrawATMtext; break;
		case VID_ATM_HWM: vid->callback = &vidDrawATMhwmc; break;
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
