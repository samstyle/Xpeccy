#include "video.h"
//#include "memory.h"
//#include "z80.h"
#include "spectrum.h"
#include "emulwin.h"

//#include <QApplication>
//#include <QDesktopWidget>

#define NOVIDEO 0

Video::Video() {
//	sys->mem = new Memory;
	int i,j,k,l;
	int idx=0;
	int sadr=0x0000;
	int aadr=0x1800;
	for (i=0;i<16;i++) {
		pal[i].b = (i & 1)?((i & 8)?0xff:0xc0):0;
		pal[i].r = (i & 2)?((i & 8)?0xff:0xc0):0;
		pal[i].g = (i & 4)?((i & 8)?0xff:0xc0):0;
	}
	for (i=0;i<3;i++) {
		for (j=0;j<8;j++) {
			for (k=0;k<8;k++) {
				for (l=0;l<32;l++) {
					ladrz[idx].scr5 = sys->mem->ram[5] + sadr;
					ladrz[idx].atr5 = sys->mem->ram[5] + aadr;
					ladrz[idx].scr7 = sys->mem->ram[7] + sadr;
					ladrz[idx].atr7 = sys->mem->ram[7] + aadr;
					ladrz[idx].ac00 = sys->mem->ram[4] + sadr;
					ladrz[idx].ac10 = sys->mem->ram[6] + sadr;
					ladrz[idx].ac01 = sys->mem->ram[5] + sadr;
					ladrz[idx].ac11 = sys->mem->ram[7] + sadr;
					ladrz[idx].ac02 = sys->mem->ram[4] + sadr + 0x2000;
					ladrz[idx].ac12 = sys->mem->ram[6] + sadr + 0x2000;
					ladrz[idx].ac03 = sys->mem->ram[5] + sadr + 0x2000;
					ladrz[idx].ac13 = sys->mem->ram[7] + sadr + 0x2000;
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

//	setgeometry(448,320,136,80,64,32,64);
	curr.h = curr.v = 0;
	curscr = false;
	t = 0;
	fcnt = 0.0;
//	mod = - synh.h - synh.v * full.h;	// VS + 1st HS
}

void Video::setborder(float prc) {brdsize = prc; update();}

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
	scrn.h = full.h - synh.h;		// full visible size (w/o sync)
	scrn.v = full.v - synh.v;
	frmsz = full.h * full.v;
	update();
	return true;
}
/*
void Video::setgeometry(int p1,int p2,int p3,int p4,int p5,int p6,int p7) {
	full.h = p1; full.v = p2;	// full size (+sync)
	bord.h = p3; bord.v = p4;	// border size
	synh.h = p5; synh.v = p6;
	intsz = p7;
	frmsz = p1 * p2;
	update();
}
*/

/*
void Video::blockFullScreen(bool p) {
	if (p) {
		flags |= VF_BLOCKFULLSCREEN;
	} else {
		flags &= ~VF_BLOCKFULLSCREEN;
	}
	update();
}
*/

void Video::update() {
	lcut.h = synh.h + (bord.h - synh.h) * (float)(1.0 - brdsize);
	lcut.v = synh.v + (bord.v - synh.v) * (float)(1.0 - brdsize);
	rcut.h = full.h - (float)(1.0 - brdsize)*(full.h - bord.h - 256);
	rcut.v = full.v - (float)(1.0 - brdsize)*(full.v - bord.v - 192);
	vsze.h = rcut.h - lcut.h;
	vsze.v = rcut.v - lcut.v;
	wsze.h = vsze.h * ((flags & VF_DOUBLE) ? 2 : 1);
	wsze.v = vsze.v * ((flags & VF_DOUBLE) ? 2 : 1);
	if (mwin != NULL) {
		mwin->setFixedSize(wsze.h,wsze.v);
	//	mwin->showFullScreen();
	}
	int sdlflg = SDL_HWSURFACE;// | SDL_FULLSCREEN;
	int szh = wsze.h;
	int szw = wsze.v;
	if ((flags & VF_FULLSCREEN) && !(flags & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
//		szh = QApplication::desktop()->width();
//		szw = QApplication::desktop()->height();
	}
#if SDLMAINWIN
	surf = SDL_SetVideoMode(szh,szw,8,sdlflg);
	SDL_WM_SetCaption("Xpeccy 0.4.1 beta Win32","");
#else
	surf = SDL_SetVideoMode(szh,szw,8,sdlflg | SDL_NOFRAME);
#ifdef WIN32
	if (mwin) SetWindowPos(mwin->inf.window,HWND_TOP,0,0,szh,szw,0);
#endif
#endif
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,pal,0,256);
	scrptr = (unsigned char*)surf->pixels;
}

void Video::sync(int tk,float fr) {
	pxcnt += 7.0 * tk / fr;
	t += pxcnt;
//	printf("+ %f = %i\n",pxcnt,t);
	while (pxcnt > 0) {
		tick();
		pxcnt -= 1.0;
	}
}

void Video::tick() {
	bool onscr = (curr.v >= lcut.v) && (curr.v < rcut.v);
#if !NOVIDEO
	if ((curr.h >= lcut.h) && (curr.h < rcut.h) && onscr) {
		unsigned char col = 5;
		if ((curr.h < bord.h) || (curr.h > bord.h + 255) || (curr.v < bord.v) || (curr.v > bord.v + 191)) {
			col=brdcol;
		} else {
//			printf("%X / %X\n",ladrz[0].atr5,sys->mem->ram[5] + 0x1800); throw(0);
			switch (mode) {
				case VID_NORMAL:
					if (((curr.h - bord.h) & 7) == 0) {
						scrbyte = curscr ? (*ladrz[iacount].scr7) : (*ladrz[iacount].scr5);
						atrbyte = curscr ? (*ladrz[iacount].atr7) : (*ladrz[iacount].atr5);
//						printf("%.2X : %.2X\n",scrbyte,atrbyte);
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
			*(scrptr + surf->w - 1) = col;
			*(scrptr + surf->w) = col;
			*(scrptr++)=col;
		}
	}
#endif
	if (++curr.h >= full.h) {
		curr.h = 0;
		if (onscr) {
			scrptr += surf->w - wsze.h;
			if (flags & VF_DOUBLE) scrptr += surf->w;
		}
		if (++curr.v >= full.v) {
			curr.v = 0;
//			intupt = true;
//	printf("%i\n",sys->cpu->t - sys->cpu->tb);
			sys->cpu->tb = sys->cpu->t;
			fcnt++; flash = fcnt & 0x20;
			scrptr = (unsigned char*)surf->pixels;
			iacount=0;
		}
		intupt = (curr.v==intpos) && (curr.h < intsz);
		sys->istrb |= intupt;
	}
}
