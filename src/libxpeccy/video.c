#include <stdlib.h>
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
	if (screenBuf == NULL) {
		screenBuf = (unsigned char*)malloc(1024 * 1024 * sizeof(unsigned char));
	}
	Video* vid = (Video*)malloc(sizeof(Video));
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
	vid->zoom = 1.0;
	vidSetLayout(vid,448,320,138,80,64,32,0,64,0);
	vid->mode = VID_NORMAL;

	vid->curr.h = 0;
	vid->curr.v = 0;
	vid->curscr = 0;
	vid->fcnt = 0;

	vid->nextBorder = 0xff;
	vid->dotCount = 0;
	vid->pxcnt = 0;

	vid->scrimg = screenBuf;
	vid->scrptr = vid->scrimg;

	vid->firstFrame = 1;

	return vid;
}

void vidDestroy(Video* vid) {
	free(vid);
}

unsigned char* vidGetScreen() {
	return screenBuf;
}

#define	MTRX_INVIS	0x4000		// invisible: blank spaces
#define	MTRX_BORDER	0x4001		// border
#define	MTRX_ZERO	0x4002		// "line feed"
#define	MTRX_SHIFT	0x4003		// 1,3,5,7 dots in byte
#define	MTRX_DOT2	0x4004		// 2nd dot
#define	MTRX_DOT4	0x4005		// 4th dot
#define	MTRX_DOT6	0x4006		// 6th dot
// all other numbers is index of memaddr in 0-dot

void vidFillMatrix(Video* vid) {
	int x,y,i,adr;
	i = 0;
	adr = 0;
	for (y = 0; y < vid->full.v; y++) {
		for (x = 0; x < vid->full.h; x++) {
			if ((y < vid->lcut.v) || (y >= vid->rcut.v) || (x < vid->lcut.h) || (x >= vid->rcut.h)) {
				vid->matrix[i] = ((x==0) && (y > vid->lcut.v) && (y < vid->rcut.v)) ? MTRX_ZERO : MTRX_INVIS;
			} else {
				if ((y < vid->bord.v) || (y > vid->bord.v + 191) || (x < vid->bord.h) || (x > vid->bord.h + 255)) {
					vid->matrix[i] = MTRX_BORDER;
				} else {
					switch ((x - vid->bord.h) & 7) {
						case 0: vid->matrix[i] = adr; adr++; break;
						case 2: vid->matrix[i] = MTRX_DOT2; break;
						case 4: vid->matrix[i] = MTRX_DOT4; break;
						case 6: vid->matrix[i] = MTRX_DOT6; break;
						default: vid->matrix[i] = MTRX_SHIFT; break;
					}
				}
			}
			i++;
		}
	}
}

void vidUpdate(Video* vid) {
	vid->lcut.h = vid->sync.h + (vid->bord.h - vid->sync.h) * (1.0 - brdsize);
	vid->lcut.v = vid->sync.v + (vid->bord.v - vid->sync.v) * (1.0 - brdsize);
	vid->rcut.h = vid->full.h - (1.0 - brdsize) * (vid->full.h - vid->bord.h - 256);
	vid->rcut.v = vid->full.v - (1.0 - brdsize) * (vid->full.v - vid->bord.v - 192);
	vid->vsze.h = vid->rcut.h - vid->lcut.h;
	vid->vsze.v = vid->rcut.v - vid->lcut.v;
	vid->wsze.h = vid->vsze.h * ((vidFlag & VF_DOUBLE) ? 2 : 1);
	vid->wsze.v = vid->vsze.v * ((vidFlag & VF_DOUBLE) ? 2 : 1);
	vidFillMatrix(vid);
}

unsigned char col = 0;
unsigned short mtx = 0;
unsigned char ink = 0;
unsigned char pap = 0;
unsigned char scrbyte = 0;
unsigned char alscr2,alscr4,alscr6;

unsigned char pixBuffer[8];
unsigned char bitMask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

void vidSync(Video* vid, float dotDraw) {
	vid->intStrobe = 0;
	vid->pxcnt += dotDraw;
	while (vid->pxcnt >= 1) {
		mtx = vid->matrix[vid->dotCount++];
		switch (mtx) {
			case MTRX_ZERO:
				if (vidFlag & VF_DOUBLE) vid->scrptr += vid->wsze.h;
				break;
			case MTRX_INVIS:
				break;
			default:
				switch (vid->mode) {
					case VID_NORMAL:
						switch(mtx) {
							case MTRX_BORDER:
								col = vid->brdcol;
								break;
							case MTRX_DOT2:
							case MTRX_DOT4:
							case MTRX_DOT6:
							case MTRX_SHIFT:
								col = pixBuffer[ink++];
								break;
							default:
								if (vid->curscr != 0) {
									scrbyte = *(vid->scr7pix[mtx]);
									vid->atrbyte = *(vid->scr7atr[mtx]);
								} else {
									scrbyte = *(vid->scr5pix[mtx]);
									vid->atrbyte = *(vid->scr5atr[mtx]);
								}
								if ((vid->atrbyte & 0x80) && vid->flash) scrbyte ^= 255;
								ink = inkTab[vid->atrbyte & 0x7f];
								pap = papTab[vid->atrbyte & 0x7f];
								for (col = 0; col < 8; col++) {
									pixBuffer[col] = (scrbyte & bitMask[col]) ? ink : pap;
								}
								ink = 1;
								col = pixBuffer[0];
								break;
						}
						break;
					case VID_ALCO:
						switch (mtx) {
							case MTRX_BORDER:
								col = vid->brdcol;
								break;
							case MTRX_SHIFT:
								col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
								break;
							case MTRX_DOT2:
								scrbyte = alscr2;
								col = inkTab[scrbyte & 0x7f];
								break;
							case MTRX_DOT4:
								scrbyte = alscr4;
								col = inkTab[scrbyte & 0x7f];
								break;
							case MTRX_DOT6:
								scrbyte = alscr6;
								col = inkTab[scrbyte & 0x7f];
								break;
							default:
								if (vid->curscr != 0) {
									scrbyte = *(vid->ladrz[mtx].ac10);
									alscr2 = *(vid->ladrz[mtx].ac11);
									alscr4 = *(vid->ladrz[mtx].ac12);
									alscr6 = *(vid->ladrz[mtx].ac13);
								} else {
									scrbyte = *(vid->ladrz[mtx].ac00);
									alscr2 = *(vid->ladrz[mtx].ac01);
									alscr4 = *(vid->ladrz[mtx].ac02);
									alscr6 = *(vid->ladrz[mtx].ac03);
								}
								col = inkTab[scrbyte & 0x7f];
								break;
						}
						break;
				}
				if (vid->firstFrame || (*vid->scrptr != col)) {
					*(vid->scrptr++) = col;
					if (vidFlag & VF_DOUBLE) {
						*(vid->scrptr + vid->wsze.h - 1) = col;
						*(vid->scrptr + vid->wsze.h) = col;
						*(vid->scrptr++)=col;
					}
					vidFlag |= VF_CHANGED;
				} else {
					vid->scrptr++;
					if (vidFlag & VF_DOUBLE) vid->scrptr++;
				}
				break;
		}
		vid->pxcnt--;
		if (vid->dotCount >= vid->frmsz) {
			vid->dotCount = 0;
			vid->fcnt++;
			vid->flash = vid->fcnt & 0x20;
			vid->scrptr = vid->scrimg;
			vid->intStrobe = 1;
			vid->firstFrame = 0;
		}
	}
	if (vid->nextBorder < 8) {
		vid->brdcol = vid->nextBorder;
		vid->nextBorder = 0xff;
	}
}

// LAYOUTS

void vidSetLayout(Video *vid, int fh, int fv, int bh, int bv, int sh, int sv, int ih, int iv, int is) {
	vid->full.h = fh;
	vid->full.v = fv;
	vid->bord.h = bh;
	vid->bord.v = bv;
	vid->sync.h = sh;
	vid->sync.v = sv;
	vid->intpos = iv;
	vid->intsz = is;
	vid->frmsz = fh * fv;
	vidUpdate(vid);
}
