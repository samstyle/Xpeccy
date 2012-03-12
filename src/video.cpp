#include <math.h>
#include <stdio.h>
#include "video.h"

// video layouts
std::vector<VidLayout> layoutList;
uint8_t* screenBuf = new uint8_t[1024 * 1024];

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
	Video* vid = new Video;
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
	vid->brdsize = 1.0;
	vid->flags = 0;
	vidSetLayout(vid,"default");

	vid->curr.h = 0;
	vid->curr.v = 0;
	vid->curscr = false;
	vid->fcnt = 0;

	vid->nextBorder = 0xff;
	vid->dotCount = 0;
	vid->pxcnt = 0;

	vid->scrimg = screenBuf;
	vid->scrptr = vid->scrimg;

	vid->firstFrame = true;

	return vid;
}

void vidDestroy(Video* vid) {
	delete(vid);
}

uint8_t* vidGetScreen() {
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
	uint32_t x,y,i,adr;
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
	vid->lcut.h = vid->synh.h + (vid->bord.h - vid->synh.h) * (1.0 - vid->brdsize);
	vid->lcut.v = vid->synh.v + (vid->bord.v - vid->synh.v) * (1.0 - vid->brdsize);
	vid->rcut.h = vid->full.h - (1.0 - vid->brdsize) * (vid->full.h - vid->bord.h - 256);
	vid->rcut.v = vid->full.v - (1.0 - vid->brdsize) * (vid->full.v - vid->bord.v - 192);
	vid->vsze.h = vid->rcut.h - vid->lcut.h;
	vid->vsze.v = vid->rcut.v - vid->lcut.v;
	vid->wsze.h = vid->vsze.h * ((vid->flags & VF_DOUBLE) ? 2 : 1);
	vid->wsze.v = vid->vsze.v * ((vid->flags & VF_DOUBLE) ? 2 : 1);
	vidFillMatrix(vid);
}

uint8_t col = 0;
uint16_t mtx = 0;
uint8_t ink = 0;
uint8_t pap = 0;
uint8_t scrbyte = 0;
uint8_t alscr2,alscr4,alscr6;
float dotDraw = 0;

uint8_t pixBuffer[8];
uint8_t bitMask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

void vidSync(Video* vid,int tk,float fr) {
	vid->intStrobe = false;
	dotDraw = 7.0 * tk / fr;
	vid->pxcnt += dotDraw;
	while (vid->pxcnt >= 1) {
		mtx = vid->matrix[vid->dotCount++];
		switch (mtx) {
			case MTRX_ZERO:
				if (vid->flags & VF_DOUBLE) vid->scrptr += vid->wsze.h;
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
								if (vid->curscr) {
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
								if (vid->curscr) {
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
					if (vid->flags & VF_DOUBLE) {
						*(vid->scrptr + vid->wsze.h - 1) = col;
						*(vid->scrptr + vid->wsze.h) = col;
						*(vid->scrptr++)=col;
					}
					vid->flags |= VF_CHANGED;
				} else {
					vid->scrptr++;
					if (vid->flags & VF_DOUBLE) vid->scrptr++;
				}
				break;
		}
		vid->pxcnt--;
		if (vid->dotCount >= vid->frmsz) {
			vid->dotCount = 0;
			vid->fcnt++;
			vid->flash = vid->fcnt & 0x20;
			vid->scrptr = vid->scrimg;
			vid->intStrobe = true;
			vid->firstFrame = false;
		}
	}
	if (vid->nextBorder < 8) {
		vid->brdcol = vid->nextBorder;
		vid->nextBorder = 0xff;
	}
}

// LAYOUTS

void addLayout(VidLayout nlay) {
printf("addLayout %s\n",nlay.name.c_str());
	for(uint32_t i=0; i<layoutList.size(); i++) {
		if (layoutList[i].name == nlay.name) return;
	}
	layoutList.push_back(nlay);
}

void addLayout(std::string nm, int* par) {
	for(uint32_t i=0; i<layoutList.size(); i++) {
		if (layoutList[i].name == nm) return;		// prevent layouts with same names
	}
	VidLayout nlay;
	nlay.name = nm;
	nlay.full.h = par[0];
	nlay.full.v = par[1];
	nlay.bord.h = par[2];
	nlay.bord.v = par[3];
	nlay.sync.h = par[4];
	nlay.sync.v = par[5];
	nlay.intsz = par[6];
	nlay.intpos = par[7];
	layoutList.push_back(nlay);
}

std::vector<VidLayout> getLayoutList() {
	std::vector<VidLayout> res = layoutList;
	return res;
}

bool vidSetLayout(Video* vid,std::string nm) {
	for (uint32_t i=0; i < layoutList.size(); i++) {
		if (layoutList[i].name == nm) {
			vid->curlay = nm;
			vid->full = layoutList[i].full;
			vid->bord = layoutList[i].bord;
			vid->synh = layoutList[i].sync;
			vid->intsz = layoutList[i].intsz;
			vid->intpos = layoutList[i].intpos;
			vid->frmsz = vid->full.h * vid->full.v;
			vidUpdate(vid);
			return true;
		}
	}
	return false;
}
