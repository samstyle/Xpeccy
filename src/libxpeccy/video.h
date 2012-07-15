#ifndef _VIDEO_H
#define _VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "memory.h"

// vidFlags
#define VF_FULLSCREEN		1
#define VF_DOUBLE		(1<<1)
#define VF_BLOCKFULLSCREEN	(1<<2)
#define VF_CHANGED		(1<<3)
// screen drawing mode
#define	VID_NORMAL	0
#define	VID_ALCO	1
// flags returned by vidSync
#define	VID_INT		1
#define	VID_FRM		(1<<1)

typedef struct {
	int h;
	int v;
} VSize;

typedef struct {
	int flag;
	int type;
	unsigned char* scr5ptr;
	unsigned char* atr5ptr;
	unsigned char* scr7ptr;
	unsigned char* atr7ptr;
	unsigned char* alco5ptr;
	unsigned char* alco7ptr;
} mtrxItem;

typedef struct {
	char intSignal;
	int firstFrame;
	int flash;
	int curscr;
	unsigned char brdcol;
	unsigned char fcnt;
	unsigned char atrbyte;
	unsigned char* scrptr;
	unsigned char* scrimg;
	int frmsz;
	int mode;
	float pxcnt;
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
void vidSetLayout(Video*, int, int, int, int, int, int, int, int, int);
void vidUpdate(Video*);

unsigned char* vidGetScreen();

#ifdef __cplusplus
}
#endif

#endif
