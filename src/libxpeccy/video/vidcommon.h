#ifndef X_VIDCOMMON_H
#define X_VIDCOMMON_H

#define VID_DIRECT_DRAW	0

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

#if VID_DIRECT_DRAW

#define vidSingleDot(_ray, _pal, _idx) \
	vid_dot_half(vid, _idx);

#define vidPutDot(_ray, _pal, _idx) \
	vid_dot_full(vid, _idx);

#else

#define vidSingleDot(_ray, _pal, _idx) \
	memcpy((_ray)->ptr, &(_pal)[_idx], 3);\
	(_ray)->ptr += 3;

#define vidPutDot(_ray, _pal, _idx) \
	vidSingleDot(_ray, _pal, _idx);\
	vidSingleDot(_ray, _pal, _idx);

#endif

#endif
