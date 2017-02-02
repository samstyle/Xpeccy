// 2:1 -> fullscreen

#include <math.h>
#include <string.h>

#include <QSize>
#include <QColor>
#include <QApplication>
#include <QDesktopWidget>

#include "xcore/xcore.h"

void scrFS(unsigned char* src, int srcw, int srch) {
	QSize dstsize = QApplication::desktop()->screenGeometry().size();
	int dstw = dstsize.width();
	int dsth = dstsize.height();
	double scalex = dstw / (double)srcw;
	double scaley = dsth / (double)srch;
	int resw = dstw;
	int resh = dsth;
	if (conf.vid.keepRatio) {
		if (scalex < scaley) {
			scaley = scalex;
		} else {
			scalex = scaley;
		}
		resw = srcw * (double)scalex;		// dest picture size, = dstw,dsth if don't keep ratio
		resh = srch * (double)scaley;
	}
	int posx = (resw < dstw) ? ((dstw - resw) >> 1) : 0;
	int posy = (resh < dsth) ? ((dsth - resh) >> 1) : 0;

	memset(conf.vid.screen, 0x00, dstw * dsth * 3);

	unsigned char* dst = conf.vid.screen + (posy * dstw * 3) + (posx * 3);		// drawing start here
	unsigned char* ptr = dst;
	scalex = scalex / 2.0;			// because of source 2:1
	srcw <<= 1;
	double cntx = scalex;
	double cnty = scaley;
	int x,y;
	int lsiz = dstw * 3;
	int vlsz = resw * 3;
	for (y = 0; y < srch; y++) {
		// ptr = dst;
		for (x = 0; x < srcw; x++) {
			while(cntx > 1.0) {
				memcpy(ptr, src, 3);
				ptr += 3;
				cntx -= 1.0;
			}
			src += 3;
			cntx += scalex;
		}
		ptr = dst + lsiz;
		cnty -= 1.0;			// 1st line completed
		while(cnty > 1.0) {
			memcpy(ptr, dst, vlsz);
			ptr += lsiz;
			cnty -= 1.0;
		}
		cnty += scaley;
		dst = ptr;
	}
}

// 2:1 -> 4:4 = double each pixel & copy each row 3 times
void scrX4(unsigned char* src, int wid, int lines) {
	unsigned char* dst = conf.vid.screen;
	unsigned char* ptr;
	int cnt;
	while(lines > 0) {
		ptr = dst;
		for (cnt = 0; cnt < wid; cnt++) {
			memcpy(dst, src, 3);
			memcpy(dst+3, src, 3);
			memcpy(dst+6, src+3, 3);
			memcpy(dst+9, src+3, 3);
			dst += 12;
			src += 6;
		}
		cnt = dst - ptr;
		memcpy(dst, ptr, cnt);
		dst += cnt;
		memcpy(dst, ptr, cnt);
		dst += cnt;
		memcpy(dst, ptr, cnt);
		dst += cnt;
		lines--;
	}
}

// 2:1 -> 3:3
void scrX3(unsigned char* src, int wid, int lines) {
	unsigned char* dst = conf.vid.screen;
	unsigned char* ptr;
	int cnt;
	while (lines > 0) {
		ptr = dst;
		for (cnt = 0; cnt < wid; cnt++) {
			memcpy(dst, src, 3);
			memcpy(dst+3, src, 3);
			memcpy(dst+6, src+3, 3);
			dst += 9;
			src += 6;
		}
		cnt = dst - ptr;
		memcpy(dst, ptr, cnt);
		dst += cnt;
		memcpy(dst, ptr, cnt);
		dst += cnt;
		lines--;
	}
}

// 2:1 -> 2:2 = double each line
void scrX2(unsigned char* src, int wid, int lines) {
	unsigned char* dst = conf.vid.screen;
	wid *= 6;
	while (lines > 0) {
		memcpy(dst, src, wid);
		dst += wid;
		memcpy(dst, src, wid);
		dst += wid;
		src += wid;
		lines--;
	}
}

// 2:1 -> 1:1 = take each 2nd pixel in a row

void scrX1(unsigned char* src, int wid, int lines) {
	unsigned char* dst = conf.vid.screen;
	int cnt;
	while (lines > 0) {
		for (cnt = 0; cnt < wid; cnt++) {
			memcpy(dst, src, 3);
			dst += 3;
			src += 6;
		}
		lines--;
	}
}
