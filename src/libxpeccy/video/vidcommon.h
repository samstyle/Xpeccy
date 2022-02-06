#pragma once

#ifdef WORDS_BIG_ENDIAN
	#define ePair(p,h,l) union{unsigned short p; struct {unsigned char h; unsigned char l;};}
#else
	#define ePair(p,h,l) union{unsigned short p; struct {unsigned char l; unsigned char h;};}
#endif

typedef struct {
	int x;
	int y;
} vCoord;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} xColor;

typedef struct {
	unsigned char* ptr;
	unsigned char* lptr;
	int x;
	int y;
	int xb;
	int yb;
	int xs;
	int ys;
} vRay;

typedef struct {
	vCoord full;			// full size = visible + blank
	vCoord bord;			// left/top border size
	vCoord blank;			// blank zones
	vCoord scr;			// screen size (256:192 for zx)
	vCoord intpos;
	int intSize;
} vLayout;

#define MADR(_bnk,_adr)	((_bnk) << 14) + ((_adr) & 0x3fff)
