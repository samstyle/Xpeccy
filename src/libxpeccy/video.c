#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

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
	int i,j,k,l;
	int idx=0;
	int sadr=0x0000;
	int aadr=0x1800;
	if (screenBuf == NULL) {
		screenBuf = (unsigned char*)malloc(1024 * 1024 * sizeof(unsigned char));
	}
	vid->mem = me;
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
	vidSetLayout(vid,448,320,128,80,80,32,0,0,64);
	vidSetMode(vid,VID_NORMAL);

	vid->curr.h = 0;
	vid->curr.v = 0;
	vid->curscr = 0;
	vid->fcnt = 0;

	vid->dotCount = 0;
	vid->pxcnt = 0;
	vid->drawed = 0;

	vid->scrimg = screenBuf;
	vid->scrptr = vid->scrimg;

	vid->flags = 0;
	vid->firstFrame = 1;
	vid->intSignal = 0;

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

void vidUpdate(Video* vid) {
	vid->lcut.h = (int)floor(vid->sync.h + ((vid->bord.h - vid->sync.h) * (1.0 - brdsize))) & 0xfffe;
	vid->lcut.v = (int)floor(vid->sync.v + ((vid->bord.v - vid->sync.v) * (1.0 - brdsize))) & 0xfffe;
	vid->rcut.h = (int)floor(vid->full.h - ((1.0 - brdsize) * (vid->full.h - vid->bord.h - 256))) & 0xfffe;
	vid->rcut.v = (int)floor(vid->full.v - ((1.0 - brdsize) * (vid->full.v - vid->bord.v - 192))) & 0xfffe;
	vid->vsze.h = vid->rcut.h - vid->lcut.h;
	vid->vsze.v = vid->rcut.v - vid->lcut.v;
	vid->wsze.h = vid->vsze.h * ((vidFlag & VF_DOUBLE) ? 2 : 1);
	vid->wsze.v = vid->vsze.v * ((vidFlag & VF_DOUBLE) ? 2 : 1);
	vidFillMatrix(vid);
}

unsigned char col = 0;
unsigned char ink = 0;
unsigned char pap = 0;
unsigned char scrbyte = 0;
unsigned char nxtbyte = 0;
int adr;
unsigned char* fntptr;
mtrxItem* mtx = NULL;

//unsigned char pixBuffer[8];
//unsigned char bitMask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

void vidDarkTail(Video* vid) {
	unsigned char* ptr = vid->scrptr;
	int idx = vid->dotCount;
	do {
		mtx = &vid->matrix[idx];
		idx++;
		if (mtx->flag & MTF_VISIBLE) {
			*(ptr++) &= 0x0f;
			if (vidFlag & VF_DOUBLE) {
				*(ptr + vid->wsze.h - 1) &= 0x0f;
				*(ptr + vid->wsze.h) &= 0x0f;
				*(ptr++) &= 0x0f;
			}
		}
		if ((mtx->flag & MTF_LINEND) && (vidFlag & VF_DOUBLE)) ptr += vid->wsze.h;
	} while (~mtx->flag & MTF_FRMEND);
}

int vidGetWait(Video* vid) {
	return vid->matrix[vid->dotCount].wait;
}

void vidSetFont(Video* vid, char* src) {
	memcpy(vid->font,src,0x800);
}

unsigned char vidGetAttr(Video* vid) {
	return (mtx->flag & MTF_SCREEN) ? vid->atrbyte : vid->brdcol;
}

void vidPutDot(Video* vid, unsigned char col) {
	if ((vidFlag & (VF_NOFLIC | VF_FRAMEDBG)) == VF_NOFLIC) {
		col = (((*vid->scrptr) & 0x0f) << 4) | (col & 0x0f);
	} else {
		col |= (col << 4);
	}
	*(vid->scrptr++) = col;
	if (vidFlag & VF_DOUBLE) {
		*(vid->scrptr + vid->wsze.h - 1) = col;
		*(vid->scrptr + vid->wsze.h) = col;
		*(vid->scrptr++)=col;
	}
}

// video drawing

void vidDrawUnknown(Video* vid) {
	vidPutDot(vid,0);
}

void vidDrawNormal(Video* vid) {
	if (mtx->flag & MTF_PIX) nxtbyte = vid->curscr ? *(mtx->scr7ptr) : *(mtx->scr5ptr);
	if (mtx->flag & MTF_ATR) {
		scrbyte = nxtbyte;
		vid->atrbyte = vid->curscr ? *(mtx->atr7ptr) : *(mtx->atr5ptr);
		if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 255;
		ink = inkTab[vid->atrbyte & 0x7f];
		pap = papTab[vid->atrbyte & 0x7f];
	}
	col = (mtx->flag & MTF_SCREEN) ? ((scrbyte & mtx->dotMask) ? ink : pap) : vid->brdcol;
	vidPutDot(vid,col);
}

void vidDrawAlco(Video* vid) {
	if (mtx->flag & MTF_SCREEN) {
		if (mtx->flag & MTF_2P) {
			scrbyte = vid->curscr ? *(mtx->alco7ptr) : *(mtx->alco5ptr);
			col = inkTab[scrbyte & 0x7f];
		} else {
			col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
		}
	} else {
		col = vid->brdcol;
	}
	vidPutDot(vid,col);
}

void vidDrawHwmc(Video* vid) {
	if (mtx->flag & MTF_PIX) nxtbyte = vid->curscr ? *(mtx->scr7ptr) : *(mtx->scr5ptr);
	if (mtx->flag & MTF_ATR) {
		scrbyte = nxtbyte;
		vid->atrbyte = vid->curscr ? *(mtx->scr7ptr + 0x2000) : *(mtx->scr5ptr + 0x2000);
		ink = inkTab[vid->atrbyte & 0x7f];
		pap = papTab[vid->atrbyte & 0x7f];
	}
	col = (mtx->flag & MTF_SCREEN) ? ((scrbyte & mtx->dotMask) ? ink : pap) : vid->brdcol;
	vidPutDot(vid,col);
}

void vidDrawATMega(Video* vid) {
	if (mtx->flag & MTF_ATMSCR) {
		if (mtx->flag & MTF_2P) {
			scrbyte = vid->curscr ? *(mtx->atm7.egaptr) : *(mtx->atm5.egaptr);
			col = inkTab[scrbyte & 0x7f];
		} else {
			col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
		}
	} else {
		col = vid->brdcol;
	}
	vidPutDot(vid,col);
}

void vidDrawATMtext(Video* vid) {
	if (mtx->flag & MTF_ATMSCR) {
		if (mtx->flag & MTF_ATMTXT) {
			scrbyte = vid->curscr ? *(mtx->atm7.txtptr) : *(mtx->atm5.txtptr);
			col = vid->curscr ? *(mtx->atm7.txtatrptr) : *(mtx->atm5.txtatrptr);
			ink = inkTab[col & 0x7f];
			pap = papTab[col & 0x3f] | ((col & 0x80) >> 4);
			adr = (scrbyte << 3);
			fntptr = vid->scrptr;
			for (col = 0; col < 8; col++) {
				scrbyte = vid->font[adr];
				if (vidFlag & VF_DOUBLE) {
					*(fntptr) = (scrbyte & 0x80) ? ink : pap;
					*(fntptr+1) = (scrbyte & 0x40) ? ink : pap;
					*(fntptr+2) = (scrbyte & 0x20) ? ink : pap;
					*(fntptr+3) = (scrbyte & 0x10) ? ink : pap;
					*(fntptr+4) = (scrbyte & 0x08) ? ink : pap;
					*(fntptr+5) = (scrbyte & 0x04) ? ink : pap;
					*(fntptr+6) = (scrbyte & 0x02) ? ink : pap;
					*(fntptr+7) = (scrbyte & 0x01) ? ink : pap;
					memcpy(fntptr + vid->wsze.h, fntptr, 8);
					fntptr += vid->wsze.h;
				} else {
					*(fntptr) = (scrbyte & 0xc0) ? ink : pap;
					*(fntptr+1) = (scrbyte & 0x30) ? ink : pap;
					*(fntptr+2) = (scrbyte & 0x0c) ? ink : pap;
					*(fntptr+3) = (scrbyte & 0x03) ? ink : pap;
				}
				fntptr += vid->wsze.h;
				adr++;
			}
		}
		vid->scrptr++;
		if (vidFlag & VF_DOUBLE) vid->scrptr++;
	} else {
		vidPutDot(vid,vid->brdcol);
	}
}

void vidDrawATMhwmc(Video* vid) {
	if (mtx->flag & MTF_ATMSCR) {
		if (mtx->flag & MTF_4P) {
			scrbyte = vid->curscr ? *(mtx->atm7.hwmpix) : *(mtx->atm5.hwmpix);
			col = vid->curscr ? *(mtx->atm7.hwmatr) : *(mtx->atm5.hwmatr);
			ink = inkTab[col & 0x7f];
			pap = papTab[col & 0x3f] | ((col & 0x80) >> 4);
			fntptr = vid->scrptr;
			if (vidFlag & VF_DOUBLE) {
				for (col = 0; col < 8; col++) {
					*(fntptr + col) = (scrbyte & 0x80) ? ink : pap;
					scrbyte <<= 1;
				}
				memcpy(fntptr + vid->wsze.h, fntptr, 8);
			} else {
				for (col = 0; col < 4; col++) {
					*(fntptr + col) = (scrbyte & 0xc0) ? ink : pap;
					scrbyte <<= 2;
				}
			}
		} else {
			vid->scrptr++;
			if (vidFlag & VF_DOUBLE) vid->scrptr++;
		}
	} else {
		vidPutDot(vid,vid->brdcol);
	}
}

// weiter

void vidSetMode(Video* vid, int mode) {
	vid->vmode = mode;
	switch (mode) {
		case VID_NORMAL: vid->draw = &vidDrawNormal; break;
		case VID_ALCO: vid->draw = &vidDrawAlco; break;
		case VID_HWMC: vid->draw = &vidDrawHwmc; break;
		case VID_ATM_EGA: vid->draw = &vidDrawATMega; break;
		case VID_ATM_TEXT: vid->draw = &vidDrawATMtext; break;
		case VID_ATM_HWM: vid->draw = &vidDrawATMhwmc; break;
		default: vid->draw = &vidDrawUnknown; break;
	}
}

int vidSync(Video* vid, float dotDraw) {
	int i;
	int res = 0;
	vid->pxcnt += dotDraw;
	vid->drawed += dotDraw;
	while (vid->pxcnt >= 1) {
		mtx = &vid->matrix[vid->dotCount];
		vid->dotCount++;
		if (mtx->flag & MTF_INTSTRB) res |= VID_INT;
		vid->intSignal = (mtx->flag & MTF_INT) ? 1 : 0;
		if (mtx->flag & MTF_8P) vid->brdcol = vid->nextbrd;

		if (mtx->flag & MTF_VISIBLE) vid->draw(vid);

		if ((mtx->flag & MTF_LINEND) && (vidFlag & VF_DOUBLE)) vid->scrptr += vid->wsze.h;
		if (mtx->flag & MTF_FRMEND) {
			res |= VID_FRM;
			vid->dotCount = 0;
			vid->fcnt++;
			vid->flash = vid->fcnt & 0x20;
			vid->scrptr = vid->scrimg;
			vid->firstFrame = 0;
			if (vidFlag & VF_FRAMEDBG) {
				for(i = 0; i < (vid->wsze.h * vid->wsze.v); i++) vid->scrimg[i] &= 0x0f;
			}
		}
		vid->pxcnt--;
	}
	return res;
}

// LAYOUTS

void vidSetLayout(Video *vid, int fh, int fv, int bh, int bv, int sh, int sv, int ih, int iv, int is) {
	vid->full.h = fh;
	vid->full.v = fv;
	vid->bord.h = bh;
	vid->bord.v = bv;
	vid->sync.h = sh;
	vid->sync.v = sv;
	vid->intpos.h = ih;
	vid->intpos.v = iv;
	vid->intsz = is;
	vid->frmsz = fh * fv;
	vidUpdate(vid);
}
