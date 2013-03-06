#ifndef _VIDEO_H
#define _VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "memory.h"

// vidFlags (emul)
#define VF_FULLSCREEN		1
#define VF_DOUBLE		(1<<1)
#define VF_BLOCKFULLSCREEN	(1<<2)
#define VF_CHANGED		(1<<3)
#define	VF_FRAMEDBG		(1<<4)
#define VF_NOFLIC		(1<<5)
#define	VF_GREY			(1<<6)
#define	VF_CHECKCHA		(1<<7)
// vid->flags (vid)
#define	VID_BORDER_4T		(1<<1)
// screen drawing mode
#define	VID_NORMAL	0
#define	VID_ALCO	1
#define	VID_ATM_EGA	2
#define	VID_ATM_TEXT	3
#define	VID_ATM_HWM	4
#define	VID_HWMC	5
#define	VID_EVO_TEXT	6
#define	VID_UNKNOWN	0xff

typedef struct {
	int h;
	int v;
} VSize;

struct Video {
	int flags;
	int flash;
	int curscr;
	unsigned char brdcol;
	unsigned char nextbrd;
	unsigned char fcnt;
	unsigned char atrbyte;
	unsigned char* scrptr;
	unsigned char* scrimg;
	int x;
	int y;
	int frmsz;
	int vmode;
	int nsDraw;
	VSize full;
	VSize bord;
	VSize sync;
	VSize lcut;
	VSize rcut;
	VSize vsze;
	VSize wsze;
	VSize intpos;
	Memory* mem;
	int intsz;
	unsigned char font[0x800];	// ATM text mode font
	void(*callback)(struct Video*);
};

typedef struct Video Video;

extern int vidFlag;
extern float brdsize;

Video* vidCreate(Memory*);
void vidDestroy(Video*);

void vidSync(Video*,int);
void vidSetMode(Video*,int);
int vidGetWait(Video*);
void vidDarkTail(Video*);

void vidUpdate(Video*);

unsigned char* vidGetScreen();
unsigned char vidGetAttr(Video*);
void vidSetFont(Video*,char*);

#ifdef __cplusplus
}
#endif

#endif
