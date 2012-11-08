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
	vid->mode = VID_NORMAL;

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
#define	MTF_4T		(1<<3)
#define MTF_ATM_TXT	(1<<4)	// each 8th dot from 320x200 screen edge in each 8th line (meaning: take symbol and color)
// dot type (for ZX 256 x 192)
#define MTT_INVIS	0
#define	MTT_BORDER	1
#define	MTT_PT0		2
#define	MTT_PT2		3
#define	MTT_PT4		4
#define	MTT_PT6		5
#define	MTT_PTX		6
#define	MTT_BRDATR	7	// this is same MTT_PT4, but 4 pix before screen. actual color = border. action = get 1st pixels byte
// dot type (for ATM 320 x 200)
#define	MTT_ATM_NONE	0	// none: for dot outside 320x200 screen
#define	MTT_ATM_0	1
#define	MTT_ATM_2	2
#define	MTT_ATM_4	3
#define	MTT_ATM_6	4
#define	MTT_ATM_DOT	5

/*
waits for 128K, +2
	--wwwwww wwwwww-- : 16-dots cycle, start on border 8 dots before screen
waits for +2a, +3
	ww--wwww wwwwwwww : same
*/

#include <assert.h>

int waitsTab_A[16] = {5,5,4,4,3,3,2,2,1,1,0,0,0,0,6,6};	// 48K
int waitsTab_B[16] = {1,1,0,0,7,7,6,6,5,5,4,4,3,3,2,2};	// +2A,+3

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
			vid->matrix[i].wait = 0;
			vid->matrix[i].atmType = MTT_ATM_NONE;
// waits: dots from -2 dots to +253 (254 is already non-wait)
			if ((x > (vid->bord.h - 3)) && (x < (vid->bord.h + 254)) && (y >= vid->bord.v) && (y < (vid->bord.v + 192))) {	// on screen
				vid->matrix[i].wait = waitsTab_A[(x - vid->bord.h) & 15];
			}
// atm ega & text mode
			if ((y > 75) && (y < 276) && (x > 95) && (x < 416)) {
				switch ((x - 96) & 7) {
					case 0:
						vid->matrix[i].atmType = MTT_ATM_0;
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmadr;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmadr;
						if (((y - 76) & 7) == 0) {
							vid->matrix[i].flag |= MTF_ATM_TXT;
							vid->matrix[i].atm5.txtptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmTadr;
							vid->matrix[i].atm7.txtptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmTadr;
							vid->matrix[i].atm5.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmTadr + 0x2000;
							vid->matrix[i].atm7.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmTadr + 0x2000;
						}
						break;
					case 2:
						vid->matrix[i].atmType = MTT_ATM_2;
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmadr;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmadr;
						break;
					case 4:
						vid->matrix[i].atmType = MTT_ATM_4;
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmadr + 0x2000;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmadr + 0x2000;
						if (((y - 76) & 7) == 0) {
							vid->matrix[i].flag |= MTF_ATM_TXT;
							vid->matrix[i].atm5.txtptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmTadr + 0x2000;
							vid->matrix[i].atm7.txtptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmTadr + 0x2000;
							vid->matrix[i].atm5.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,1) + atmTadr + 1;
							vid->matrix[i].atm7.txtatrptr = memGetPagePtr(vid->mem,MEM_RAM,3) + atmTadr + 1;
							atmTadr++;
							if ((atmTadr & 0x3f) == 40) atmTadr = (atmTadr & 0x3fc0) + 0x40;	// next 64
						}
						break;
					case 6:
						vid->matrix[i].atmType = MTT_ATM_6;
						vid->matrix[i].atm5.egaptr = memGetPagePtr(vid->mem,MEM_RAM,5) + atmadr + 0x2000;
						vid->matrix[i].atm7.egaptr = memGetPagePtr(vid->mem,MEM_RAM,7) + atmadr + 0x2000;
						atmadr++;
						break;
					default:
						vid->matrix[i].atmType = MTT_ATM_DOT;
						break;
				}
			}
// common screen | alco mode
			if ((y < vid->lcut.v) || (y >= vid->rcut.v) || (x < vid->lcut.h) || (x >= vid->rcut.h)) {
				vid->matrix[i].type = MTT_INVIS;
			} else {
				if ((x & 7) == 0) {
					vid->matrix[i].flag |= MTF_4T;
				}
				if ((y < vid->bord.v) || (y > vid->bord.v + 191) || (x < scrShift) || (x > scrShift + 255)) {
					vid->matrix[i].type = MTT_BORDER;
				} else {
					switch ((x - scrShift) & 7) {
						case 0:
							vid->matrix[i].type = MTT_PT0;
							vid->matrix[i].atr5ptr = vid->scr5atr[adr];
							vid->matrix[i].atr7ptr = vid->scr7atr[adr];
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac00;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac10;
							if (vid->matrix[i-4].type == MTT_BORDER) {
								vid->matrix[i-4].type = MTT_BRDATR;
							}
							vid->matrix[i-4].scr5ptr = vid->scr5pix[adr];
							vid->matrix[i-4].scr7ptr = vid->scr7pix[adr];
							break;
						case 1:
							vid->matrix[i].type = MTT_PTX;
							break;
						case 2:
							vid->matrix[i].type = MTT_PT2;
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac01;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac11;
							break;
						case 3:
							vid->matrix[i].type = MTT_PTX;
							break;
						case 4:
							vid->matrix[i].type = MTT_PT4;
							vid->matrix[i].scr5ptr = vid->scr5pix[adr];
							vid->matrix[i].scr7ptr = vid->scr7pix[adr];
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac02;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac12;
							break;
						case 5:
							vid->matrix[i].type = MTT_PTX;
							break;
						case 6:
							vid->matrix[i].alco5ptr = vid->ladrz[adr].ac03;
							vid->matrix[i].alco7ptr = vid->ladrz[adr].ac13;
							vid->matrix[i].type = MTT_PT6;
							break;
						case 7:
							adr++;
							vid->matrix[i].type = MTT_PTX;
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
int adr;
unsigned char* fntptr;
mtrxItem* mtx = NULL;
mtrxItem* nmtx = NULL;

unsigned char pixBuffer[8];
unsigned char bitMask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

void vidDarkTail(Video* vid) {
	unsigned char* ptr = vid->scrptr;
	int idx = vid->dotCount;
	do {
		mtx = &vid->matrix[idx];
		idx++;
		if (mtx->type != MTT_INVIS) {
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

int vidSync(Video* vid, float dotDraw) {
	int i;
	int res = 0;
	vid->pxcnt += dotDraw;
	vid->drawed += dotDraw;
	while (vid->pxcnt >= 1) {
		mtx = &vid->matrix[vid->dotCount];
		vid->dotCount++;
		if (mtx->flag & MTF_FRMEND) vid->dotCount = 0;
		nmtx = &vid->matrix[vid->dotCount];
		if ((nmtx->flag & MTF_INT) && (!(mtx->flag & MTF_INT))) res |= VID_INT;
		vid->intSignal = (mtx->flag & MTF_INT) ? 1 : 0;
		if (mtx->flag & MTF_4T) vid->brdcol = vid->nextbrd;

		if (mtx->type != MTT_INVIS) {
			switch (vid->mode) {
				case VID_ATM_UNDEF:
				case VID_ATM_HWM:
					col = 2;
					break;
				case VID_ATM_EGA:
					switch (mtx->atmType) {
						case MTT_ATM_NONE:
							col = vid->brdcol;
							break;
						case MTT_ATM_0:
						case MTT_ATM_2:
						case MTT_ATM_4:
						case MTT_ATM_6:
							scrbyte = vid->curscr ? *(mtx->atm7.egaptr) : *(mtx->atm5.egaptr);
							col = inkTab[scrbyte & 0x7f];
							break;
						default:
							col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
							break;
					}
					break;
				case VID_ATM_TEXT:
					switch (mtx->atmType) {
						case MTT_ATM_NONE:
							col = vid->brdcol;
							break;
						default:
							if (mtx->flag & MTF_ATM_TXT) {
								// get symbol, color, symbol address and draw "8" x 8 matrix
								scrbyte = vid->curscr ? *(mtx->atm7.txtptr) : *(mtx->atm5.txtptr);
								col = vid->curscr ? *(mtx->atm7.txtatrptr) : *(mtx->atm5.txtatrptr);
								ink = (col & 7) | ((col & 0x40) >> 3);
								pap = ((col & 0x38) >> 3) | ((col & 0x80) >> 4);
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
							break;
					}
					break;
				case VID_NORMAL:
					switch(mtx->type) {
						case MTT_BRDATR:
							scrbyte = vid->curscr ? *(mtx->scr7ptr) : *(mtx->scr5ptr);
						case MTT_BORDER:
							col = vid->brdcol;
							break;
						case MTT_PT0:
							vid->atrbyte = vid->curscr ? *(mtx->atr7ptr) : *(mtx->atr5ptr);
							if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 255;
							ink = inkTab[vid->atrbyte & 0x7f];
							pap = papTab[vid->atrbyte & 0x7f];
							for (col = 0; col < 8; col++) {
								pixBuffer[col] = (scrbyte & bitMask[col]) ? ink : pap;
							}
							ink = 1;
							col = pixBuffer[0];
							break;
						case MTT_PT4:
							scrbyte = vid->curscr ? *(mtx->scr7ptr) : *(mtx->scr5ptr);
						default:
							col = pixBuffer[ink++];
							break;
					}
					break;
				case VID_ALCO:
					switch(mtx->type) {
						case MTT_BRDATR:
						case MTT_BORDER:
							col = vid->brdcol;
							break;
						case MTT_PT0:
						case MTT_PT2:
						case MTT_PT4:
						case MTT_PT6:
							scrbyte = vid->curscr ? *(mtx->alco7ptr) : *(mtx->alco5ptr);
							col = inkTab[scrbyte & 0x7f];
							break;
						default:
							col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
							break;
					}
					break;
			}
			if ((vid->mode != VID_ATM_TEXT) || (mtx->atmType == MTT_ATM_NONE)) {
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
				vidFlag |= VF_CHANGED;
			}
		}
		if ((mtx->flag & MTF_LINEND) && (vidFlag & VF_DOUBLE)) {
			vid->scrptr += vid->wsze.h;
		}
		if (mtx->flag & MTF_FRMEND) {
			res |= VID_FRM;
//			vid->dotCount = 0;
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
