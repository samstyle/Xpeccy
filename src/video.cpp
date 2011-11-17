#include <math.h>
#include "video.h"

// video layouts
std::vector<VidLayout> layoutList;

#define	VM_INVIS	1
#define	VM_BORDER	2
#define	VM_SCREEN	3

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

Video::Video(Memory* me) {
	int i,j,k,l;
	int idx=0;
	int sadr=0x0000;
	int aadr=0x1800;
	for (i=0;i<3;i++) {
		for (j=0;j<8;j++) {
			for (k=0;k<8;k++) {
				for (l=0;l<32;l++) {
					ladrz[idx].scr5 = me->ram[5] + sadr;
					ladrz[idx].atr5 = me->ram[5] + aadr;
					ladrz[idx].scr7 = me->ram[7] + sadr;
					ladrz[idx].atr7 = me->ram[7] + aadr;
					ladrz[idx].ac00 = me->ram[4] + sadr;
					ladrz[idx].ac10 = me->ram[6] + sadr;
					ladrz[idx].ac01 = me->ram[5] + sadr;
					ladrz[idx].ac11 = me->ram[7] + sadr;
					ladrz[idx].ac02 = me->ram[4] + sadr + 0x2000;
					ladrz[idx].ac12 = me->ram[6] + sadr + 0x2000;
					ladrz[idx].ac03 = me->ram[5] + sadr + 0x2000;
					ladrz[idx].ac13 = me->ram[7] + sadr + 0x2000;
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
	iacount=0;
	zoom = 1.0;
	brdsize = 1.0;
	flags = 0;
	setLayout("default");

	curr.h = curr.v = 0;
	curscr = false;
	t = 0;
	fcnt = 0.0;
	
	matrix = new int16_t[512 * 512];
}

#define	MTRX_INVIS	-1		// invisible: blank spaces
#define	MTRX_BORDER	-2		// border
#define	MTRX_SHIFT	-3		// screen dots except each 8th. on each 8th is reading of screen byte and attributes
#define	MTRX_ZERO	-4		// 

void Video::fillMatrix() {
	uint x,y,i,adr;
	i = 0;
	adr = 0;
	for (y = 0; y < full.v; y++) {
		for (x = 0; x < full.h; x++) {
			if ((y < lcut.v) || (y >= rcut.v) || (x < lcut.h) || (x >= rcut.h)) {
				matrix[i] = MTRX_INVIS;
			} else {
				if ((y < bord.v) || (y > bord.v + 191) || (x < bord.h) || (x > bord.h + 255)) {
					matrix[i] = MTRX_BORDER;
				} else {
					if (((x - bord.h) & 7) == 0) {
						matrix[i] = adr;
						adr++;
					} else {
						matrix[i] = MTRX_SHIFT;
					}
				}
			}
			if ((y >= lcut.v) && (x == full.h - 1)) matrix[i] = MTRX_ZERO;
			i++;
		}
	}
}

void Video::update() {
	lcut.h = synh.h + floor((bord.h - synh.h) * (1.0 - brdsize));
	lcut.v = synh.v + floor((bord.v - synh.v) * (1.0 - brdsize));
	rcut.h = full.h - floor((1.0 - brdsize)*(full.h - bord.h - 256));
	rcut.v = full.v - floor((1.0 - brdsize)*(full.v - bord.v - 192));
	vsze.h = rcut.h - lcut.h;
	vsze.v = rcut.v - lcut.v;
	wsze.h = vsze.h * ((flags & VF_DOUBLE) ? 2 : 1);
	wsze.v = vsze.v * ((flags & VF_DOUBLE) ? 2 : 1);
	fillMatrix();
}

//bool onscr = false;
uint8_t col = 0;
int16_t mtx = 0;

void Video::sync(int tk,float fr) {
	intStrobe = false;
	pxcnt += 7.0 * tk / fr;
	t += pxcnt;
#if 0
	while (pxcnt > 0) {
		pxcnt -= 1.0;
		onscr = (curr.v >= lcut.v) && (curr.v < rcut.v);
		if ((curr.h >= lcut.h) && (curr.h < rcut.h) && onscr) {
			if ((curr.v < bord.v) || (curr.v > bord.v + 191) || (curr.h < bord.h) || (curr.h > bord.h + 255)) {
				col = brdcol;
			} else {
				switch (mode) {
					case VID_NORMAL:
						if (((curr.h - bord.h) & 7) == 0) {
							scrbyte = curscr ? (*ladrz[iacount].scr7) : (*ladrz[iacount].scr5);
							atrbyte = curscr ? (*ladrz[iacount].atr7) : (*ladrz[iacount].atr5);
							if ((atrbyte & 0x80) && flash) scrbyte ^= 255;
							ink = inkTab[atrbyte & 0x7f];
							pap = papTab[atrbyte & 0x7f];
							iacount++;
						}
						col = (scrbyte & 0x80) ? ink : pap;
						scrbyte<<= 1;
						break;
					case VID_ALCO:
						if (((curr.h - bord.h) & 1) == 0) {
							switch ((curr.h - bord.h) & 0x06) {
								case 0x00: scrbyte = curscr ? (*ladrz[iacount].ac10) : (*ladrz[iacount].ac00); break;
								case 0x02: scrbyte = curscr ? (*ladrz[iacount].ac11) : (*ladrz[iacount].ac01); break;
								case 0x04: scrbyte = curscr ? (*ladrz[iacount].ac12) : (*ladrz[iacount].ac02); break;
								case 0x06: scrbyte = curscr ? (*ladrz[iacount].ac13) : (*ladrz[iacount].ac03); iacount++; break;
							}
							col = ((scrbyte & 0x07) | ((scrbyte & 0x40)>>3));
						} else {
							col = ((scrbyte & 0x38)>>3) | ((scrbyte & 0x80)>>4);
						}
						break;
				}
			}
			*(scrptr++)=col;
			if (flags & VF_DOUBLE) {
				*(scrptr + wsze.h - 1) = col;
				*(scrptr + wsze.h) = col;
				*(scrptr++)=col;
			}
		}
		if (++curr.h >= full.h) {
			curr.h = 0;
			if (onscr) {
				if (flags & VF_DOUBLE) scrptr += wsze.h;
			}
			if (++curr.v >= full.v) {
				curr.v = 0;
				fcnt++; flash = fcnt & 0x20;
				scrptr = scrimg;
				iacount=0;
			}
			intSignal = (curr.v==intpos) && (curr.h < intsz);
			intStrobe |= intSignal;
		}
	}
#else
	while (pxcnt > 0) {
		mtx = matrix[dotCount];
		if (mtx == MTRX_ZERO) {
			if (flags & VF_DOUBLE) scrptr += wsze.h;
			mtx = MTRX_INVIS;
		}
		if (mtx != MTRX_INVIS) {
			switch (mtx) {
				case MTRX_BORDER:
					col = brdcol;
					break;
				case MTRX_SHIFT:
					scrbyte<<=1;
					col = (scrbyte & 0x80) ? ink : pap;
					break;
				default:
					if (curscr) {
						scrbyte = *(ladrz[mtx].scr7);
						atrbyte = *(ladrz[mtx].atr7);
					} else {
						scrbyte = *(ladrz[mtx].scr5);
						atrbyte = *(ladrz[mtx].atr5);
					}
					if ((atrbyte & 0x80) && flash) scrbyte ^= 255;
					ink = inkTab[atrbyte & 0x7f];
					pap = papTab[atrbyte & 0x7f];
					col = (scrbyte & 0x80) ? ink : pap;
					break;
			}
			*(scrptr++) = col;
			if (flags & VF_DOUBLE) {
				*(scrptr + wsze.h - 1) = col;
				*(scrptr + wsze.h) = col;
				*(scrptr++)=col;
			}
		}
		pxcnt -= 1.0;
		if (++dotCount >= frmsz) {
			dotCount = 0;
			fcnt++;
			flash = fcnt & 0x20;
			scrptr = scrimg;
			intStrobe = true;
		}
	}
#endif
}

// LAYOUTS

void addLayout(VidLayout nlay) {
printf("addLayout %s\n",nlay.name.c_str());
	for(uint i=0; i<layoutList.size(); i++) {
		if (layoutList[i].name == nlay.name) return;
	}
	layoutList.push_back(nlay);
}

void addLayout(std::string nm, int* par) {
	for(uint i=0; i<layoutList.size(); i++) {
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

bool Video::setLayout(std::string nm) {
	for (uint i=0; i<layoutList.size(); i++) {
		if (layoutList[i].name == nm) {
			curlay = nm;
			full = layoutList[i].full;
			bord = layoutList[i].bord;
			synh = layoutList[i].sync;
			intsz = layoutList[i].intsz;
			intpos = layoutList[i].intpos;
			frmsz = full.h * full.v;
			update();
			return true;
		}
	}
	return false;
}
