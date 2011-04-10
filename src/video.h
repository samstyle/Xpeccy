#ifndef _VIDEO_H
#define _VIDEO_H

#include <vector>
#include <string>

#ifdef WIN32
	#undef main
	#include <SDL.h>
	#undef main
	#define SDLMAINWIN 1
#else
	#include <SDL.h>
#endif

#include "memory.h"

#define VF_FULLSCREEN		1
#define	VF_DOUBLE		2
#define VF_BLOCKFULLSCREEN	4

#define	VID_NORMAL	0
#define	VID_ALCO	1

class VSize {
	public:
		uint32_t h;
		uint32_t v;
		bool operator =(VSize p) {
			h = p.h;
			v = p.v;
			return true;
		}
};

class VidLayout {
	public:
		VidLayout() {}
		VidLayout(std::string nm,uint fh,uint fv,uint bh,uint bv,uint sh,uint sv,uint isz,uint ips) {
			name = nm;
			full.h = fh; full.v = fv;
			bord.h = bh; bord.v = bv;
			sync.h = sh; sync.v = sv;
			intsz = isz; intpos = ips;
		}
		std::string name;
		VSize full,sync,bord;
		uint intsz,intpos;
};

class Video {
	public:
		Video(Memory*);
		int flags;
//		SDL_Surface *surf;
//		SDL_Color pal[256];
		uint8_t* scrptr;
		VSize full,bord,scrn,curr,synh;
		VSize lcut,rcut,vsze,wsze;
		uint32_t frmsz, t, intsz, intpos;
		struct {
			uint8_t *scr5,*atr5;		// screen 0
			uint8_t *scr7,*atr7;		// screen 1
			uint8_t *ac00,*ac01,*ac02,*ac03;	// alco parts (screen 0)
			uint8_t *ac10,*ac11,*ac12,*ac13;	// alco parts (screen 1)
		} ladrz[0x1800];	// адреса на экране
		int iacount;		// счетчик для ladrz
		int32_t mode;
		float zoom,brdsize, pxcnt;
		bool intupt,flash,curscr;
		uint8_t fcnt,brdcol,scrbyte,prescr,atrbyte,ink,pap;
		std::vector<VidLayout> layout;	// screen layouts
		std::string curlay;		// current layout name
		void tick();
		void sync(int,float);
		bool setlayout(std::string);
//	void setgeometry(int,int,int,int,int,int,int);
		void setborder(float);
		void update();
//	void blockFullScreen(bool);
};

// extern Video *vid;

#endif
