// #include <math.h>
#include <string.h>

#include "xcore.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

void vid_upd_scale() {
	QSize scrsz;
	int dwid;
	int dhei;
	if (conf.vid.fullScreen) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
		scrsz = QApplication::screens().first()->size();
#else
		scrsz = QApplication::desktop()->screenGeometry().size();
#endif
		dwid = scrsz.width(); // QApplication::desktop()->screenGeometry().width();
		dhei = scrsz.height(); // QApplication::desktop()->screenGeometry().height();
		xstep = dwid * 0x100 / conf.prof.cur->zx->vid->vsze.x;
		ystep = dhei * 0x100 / conf.prof.cur->zx->vid->vsze.y;
		if (conf.vid.keepRatio) {
			// minimal step is default
			if (xstep < ystep) {
				ystep = xstep;
			} else {
				xstep = ystep;
			}
			// for BK stretch by X
			xstep *= conf.prof.cur->zx->hw->xscale;
			// calculate black spaces
			// TODO: recalculate for OpenGL
			lefSkip = (dwid - (conf.prof.cur->zx->vid->vsze.x * xstep / 256)) / 2 * 3;
			topSkip = (dhei - (conf.prof.cur->zx->vid->vsze.y * ystep / 256)) / 2;
#ifdef USEOPENGL
			lefSkip = lefSkip / 3;
			lefSkip = lefSkip * conf.prof.cur->zx->vid->vsze.x / dwid;
			lefSkip = lefSkip * 6;
#endif
			rigSkip = lefSkip;
			botSkip = topSkip;
		} else {
			lefSkip = 0;
			rigSkip = 0;
			topSkip = 0;
			botSkip = 0;
		}
	} else {
		lefSkip = 0;
		rigSkip = 0;
		topSkip = 0;
		botSkip = 0;
		xstep = conf.vid.scale << 8;		// :xzoom
		xstep *= conf.prof.cur->zx->hw->xscale;
		ystep = conf.vid.scale << 8;
	}
//	printf("%i x %i : %i %i %i : %X %X : %i %i %i %i\n",dwid,dhei,conf.vid.fullScreen, conf.vid.keepRatio, conf.vid.scale, xstep, ystep, topSkip, botSkip, lefSkip, rigSkip);
}

void vid_set_zoom(int zoom) {
	if (zoom < 1) return;
	if (zoom > 4) return;
	conf.vid.scale = zoom;
	vid_upd_scale();
}

void vid_set_fullscreen(int f) {
	conf.vid.fullScreen = f ? 1 : 0;
	vid_upd_scale();
}

void vid_set_ratio(int f) {
	conf.vid.keepRatio = f ? 1 : 0;
	vid_upd_scale();
}

