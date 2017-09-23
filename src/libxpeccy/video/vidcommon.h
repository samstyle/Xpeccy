#ifndef X_VIDCOMMON_H
#define X_VIDCOMMON_H

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
	unsigned char* img;
	unsigned char* ptr;
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

#define MADR(_bnk,_adr)	((_bnk) << 14) + (_adr)

#define vidSingleDot(_ray, _pal, _idx)	\
	*((_ray)->ptr++) = _pal[_idx].r;\
	*((_ray)->ptr++) = _pal[_idx].g;\
	*((_ray)->ptr++) = _pal[_idx].b;

#define vidPutDot(_ray, _pal, _idx) \
	vidSingleDot(_ray, _pal, _idx);\
	vidSingleDot(_ray, _pal, _idx);

#endif
