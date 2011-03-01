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

#define VF_FULLSCREEN		1
#define	VF_DOUBLE		2
#define VF_BLOCKFULLSCREEN	4

enum VMode {
	VID_NORMAL = 0,
	VID_ALCO
};

struct VSize {
	unsigned int h;
	unsigned int v;
	bool operator =(VSize p) {h = p.h; v = p.v; return true;}
};

struct VidLayout {
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

struct Video {
	Video();
	int flags;
	SDL_Surface *surf;
	SDL_Color pal[256];
	unsigned char* scrptr;
	VSize full,bord,scrn,curr,synh;
	VSize lcut,rcut,vsze,wsze;
	unsigned int frmsz, t, intsz, intpos;
	struct {
		unsigned char *scr5,*atr5;		// screen 0
		unsigned char *scr7,*atr7;		// screen 1
		unsigned char *ac00,*ac01,*ac02,*ac03;	// alco parts (screen 0)
		unsigned char *ac10,*ac11,*ac12,*ac13;	// alco parts (screen 1)
	} ladrz[0x1800];	// адреса на экране
	int iacount;		// счетчик для ladrz
	VMode mode;
	float zoom,brdsize, pxcnt;
	bool intupt,flash,curscr;
	unsigned char fcnt,brdcol,scrbyte,prescr,atrbyte,ink,pap;
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
