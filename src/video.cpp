#include <math.h>
#include "video.h"
#include "emulwin.h"

#define NOVIDEO 0

extern EmulWin* mwin;

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
	layout.clear();
	curlay = "default";
	layout.push_back(VidLayout("default",448,320,138,80,64,32,64,0));	// add default (pentagon) layout
	setlayout("default");

	curr.h = curr.v = 0;
	curscr = false;
	t = 0;
	fcnt = 0.0;
}

void Video::setborder(float prc) {
	brdsize = prc;
	mwin->updateWin();
}

bool Video::setlayout(std::string nm) {
	uint i;
	VidLayout* lay = NULL;
	for (i=0; i<layout.size(); i++) {
		if (layout[i].name == nm) {
			lay = &layout[i];
			break;
		}
	}
	if (lay == NULL) return false;
	curlay = lay->name;
	full = lay->full;
	bord = lay->bord;
	synh = lay->sync;
	intsz = lay->intsz;
	intpos = lay->intpos;
	frmsz = full.h * full.v;
	update();
	return true;
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
}

void Video::sync(int tk,float fr) {
	intStrobe = false;
	pxcnt += 7.0 * tk / fr;
	t += pxcnt;
	while (pxcnt > 0) {
		tick();
		pxcnt -= 1.0;
	}
}

// drawing 1 pixel
void Video::tick() {
	bool onscr = (curr.v >= lcut.v) && (curr.v < rcut.v);
#if !NOVIDEO
	if ((curr.h >= lcut.h) && (curr.h < rcut.h) && onscr) {
		uint8_t col = 5;
		if ((curr.h < bord.h) || (curr.h > bord.h + 255) || (curr.v < bord.v) || (curr.v > bord.v + 191)) {
			col=brdcol;
		} else {
			switch (mode) {
				case VID_NORMAL:
					if (((curr.h - bord.h) & 7) == 0) {
						scrbyte = curscr ? (*ladrz[iacount].scr7) : (*ladrz[iacount].scr5);
						atrbyte = curscr ? (*ladrz[iacount].atr7) : (*ladrz[iacount].atr5);
						if ((atrbyte & 0x80) && flash) scrbyte ^= 255;
						ink=((atrbyte & 0x40)>>3) | (atrbyte&7);
						pap=((atrbyte & 0x78)>>3);
						iacount++;
					}
					col = (scrbyte & 0x80) ? ink : pap;
					scrbyte <<= 1;
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
#endif
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
