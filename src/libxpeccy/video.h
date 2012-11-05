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
// vid->flags (vid)
#define	VID_BORDER_4T		(1<<1)
// screen drawing mode
#define	VID_NORMAL	0
#define	VID_ALCO	1
#define	VID_ATM_EGA	2
#define	VID_ATM_TEXT	3
// flags returned by vidSync
#define	VID_INT		1
#define	VID_FRM		(1<<1)

typedef struct {
	int h;
	int v;
} VSize;

typedef struct {
	unsigned char* egaptr;
	unsigned char* txtptr;
	unsigned char* txtatrptr;
} atmItem;

typedef struct {
	int flag;
	int type;
	int atmType;
	int wait;		// dot's (ticks * 2) to be drawing on contended memory WAIT
	int tick;		// tick NR (just for debug)
	unsigned char* scr5ptr;
	unsigned char* atr5ptr;
	unsigned char* scr7ptr;
	unsigned char* atr7ptr;
	unsigned char* alco5ptr;
	unsigned char* alco7ptr;
	atmItem atm5;
	atmItem atm7;
} mtrxItem;

typedef struct {
	int flags;
	int intSignal;
	int firstFrame;
	int flash;
	int curscr;
	unsigned char brdcol;
	unsigned char nextbrd;
	unsigned char fcnt;
	unsigned char atrbyte;
	unsigned char* scrptr;
	unsigned char* scrimg;
	int frmsz;
	int mode;
	float pxcnt;
	float drawed;
	int dotCount;
	VSize full;
	VSize bord;
	VSize curr;
	VSize sync;
	VSize lcut;
	VSize rcut;
	VSize vsze;
	VSize wsze;
	VSize intpos;
	Memory* mem;
	int intsz;
	mtrxItem matrix[512 * 512];
	struct {
		unsigned char *ac00;
		unsigned char *ac01;
		unsigned char *ac02;
		unsigned char *ac03;
		unsigned char *ac10;
		unsigned char *ac11;
		unsigned char *ac12;
		unsigned char *ac13;
	} ladrz[0x1800];
	unsigned char* scr5pix[0x1800];
	unsigned char* scr5atr[0x1800];
	unsigned char* scr7pix[0x1800];
	unsigned char* scr7atr[0x1800];
} Video;

extern int vidFlag;
extern float brdsize;

Video* vidCreate(Memory*);
void vidDestroy(Video*);

int vidSync(Video*,float);
//int vidWaitSlow(Video*);
int vidGetWait(Video*);
void vidDarkTail(Video*);

void vidSetLayout(Video*, int, int, int, int, int, int, int, int, int);
void vidUpdate(Video*);

unsigned char* vidGetScreen();

#ifdef __cplusplus
}
#endif

#endif
