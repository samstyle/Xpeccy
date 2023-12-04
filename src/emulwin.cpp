#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTime>
#include <QUrl>
#include <QMimeData>
#include <QPainter>
#include <QFileDialog>

// QDesktopWidget removed in Qt6
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	#include <QDesktopWidget>
#endif

#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xcore/xcore.h"
#include "xcore/sound.h"
#include "emulwin.h"
#include "filer.h"
#include "watcher.h"

#include "xcore/vfilters.h"
#include "xcore/vscalers.h"

#include "version.h"

#define STR_EXPAND(tok) #tok
#define	STR(tok) STR_EXPAND(tok)
#ifdef USEOPENGL
#define	XPTITLE	STR(Xpeccy VERSION (OpenGL))
#else
#define	XPTITLE	STR(Xpeccy VERSION)
#endif

#if defined(__WIN32)
#include <windows.h>
#include <winuser.h>
#endif

// main

static QImage alphabet;

// mainwin

void MainWin::updateHead() {
	QString title(XPTITLE);
#ifdef ISDEBUG
	title.append(" | debug");
#endif
	if (conf.prof.cur) {
		title.append(" | ").append(conf.prof.cur->name.c_str());
		title.append(" | ").append(conf.prof.cur->layName.c_str());
	}
	if (conf.emu.fast) {
		title.append(" | fast");
	}
	setWindowTitle(title);
}

void MainWin::updateWindow() {
	block = 1;
//	vidSetBorder(comp->vid, conf.brdsize);		// to call vidUpdateLayout???
	int szw;
	int szh;
	QSize wsz;
	Computer* comp = conf.prof.cur->zx;
	blockSignals(true);
	if (conf.vid.fullScreen) {
		wsz = SCREENSIZE;
		szw = wsz.width();
		szh = wsz.height();
		szw &= ~3;
		setFixedSize(szw, szh);
		setWindowState(windowState() | Qt::WindowFullScreen);
	} else {
		szw = comp->vid->vsze.x * conf.vid.scale;
		szh = comp->vid->vsze.y * conf.vid.scale;
		szw *= conf.prof.cur->zx->hw->xscale;
		szw &= ~3;
		setWindowState(windowState() & ~Qt::WindowFullScreen);
		setFixedSize(szw, szh);
	}
	blockSignals(false);
	vid_set_zoom(conf.vid.scale);
#ifdef USEOPENGL
	bytesPerLine = (lefSkip + comp->vid->vsze.x * 8 + rigSkip);
	bufSize = bytesPerLine * comp->vid->vsze.y;
#else
	bytesPerLine = szw << 2;
	bufSize = bytesPerLine * ((comp->vid->vsze.y * ystep) >> 8);
#endif
	updateHead();
	block = 0;
}

bool MainWin::saveChanged() {
	xProfile* prf;
	bool yep = true;
	int res;
	int i;
	QString str;
	Floppy* flp;
	foreach(prf, conf.prof.list) {
		for(i = 0; (i < 4) && yep; i++) {
			flp = prf->zx->dif->fdc->flop[i];
			if (flp->changed) {
				str = QString("Disk %0 of profile '%1' was changed<br>Save it?").arg(QChar('A' + i)).arg(prf->name.c_str());
				res = askYNC(str.toLocal8Bit().data());
				switch(res) {
					case QMessageBox::Yes:
						yep &= save_file(prf->zx, flp->path, FG_DISK, i);
						break;
					case QMessageBox::Cancel:
						yep = false;
						break;
				}
			}
		}
		if (!yep) break;
	}
	return yep;
}

void MainWin::pause(bool p, int msk) {
	conf.emu.fast = 0;
	if (p) {
		conf.emu.pause |= msk;
	} else {
		conf.emu.pause &= ~msk;
	}
	if (!grabMice || (conf.emu.pause && grabMice)) {
		releaseMouse();
	}
	if (conf.emu.pause) {
		setWindowIcon(QIcon(":/images/pause.png"));
	} else {
		setWindowIcon(icon);
		if (grabMice)
			grabMouse(QCursor(Qt::BlankCursor));
	}
}

// Main window

MainWin::MainWin() {
	qDebug() << "constructor";

	setWindowTitle(XPTITLE);
	setMouseTracking(true);
	icon = QIcon(":/images/xpeccy.png");
	setWindowIcon(icon);
	setAcceptDrops(true);
	// setAutoFillBackground(false);
	setUpdatesEnabled(false);
	conf.emu.pause = 0;
	scrCounter = 0;
	scrInterval = 0;
	grabMice = 0;
	block = 0;
//	relskip = 0;

	msgTimer = 0;
	msg.clear();
	alphabet.load(":/font.png");

	shotFormat["bmp"] = SCR_BMP;
	shotFormat["png"] = SCR_PNG;
	shotFormat["jpg"] = SCR_JPG;
	shotFormat["scr"] = SCR_SCR;
	shotFormat["hobeta"] = SCR_HOB;

	if (SDL_NumJoysticks() > 0) {
		conf.joy.joy = SDL_JoystickOpen(0);
		if (conf.joy.joy) {
			printf("Joystick opened\n");
			printf("%i axis\n", SDL_JoystickNumAxes(conf.joy.joy));
			printf("%i buttons\n", SDL_JoystickNumButtons(conf.joy.joy));
		}
	} else {
		printf("Joystick not opened\n");
		conf.joy.joy = NULL;
	}
	initFileDialog(this);
	initUserMenu();
	setFocus();

	frm_ns = 0;
	timid = startTimer(20);		// 1/50 sec
	secid = startTimer(200);	// 1/5 sec
	cmsid = startTimer(1000);	// 1 sec

	connect(&frm_tmr, SIGNAL(timeout()), this, SLOT(frame_timer()));
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	frm_tmr.setTimerType(Qt::PreciseTimer);
#endif
	frm_tmr.start(20);

	connect(userMenu,SIGNAL(aboutToShow()),SLOT(menuShow()));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));
	fillUserMenu();

#ifdef USENETWORK
	openServer();
	connect(&srv, SIGNAL(newConnection()),this, SLOT(connected()));
#endif

	qDebug() << "end:constructor";
}

MainWin::~MainWin() {
#if defined(USEOPENGL) && !BLOCKGL
	delete(vtx_shd);
	delete(frg_shd);
//	for(int i = 0; i < 4; i++)
//		deleteTexture(texids[i]);
#endif
}

// gamepad mapper
// xJoyMapEntry::key is XKEY_*

void MainWin::mapRelease(Computer* comp, xJoyMapEntry ent) {
	switch(ent.dev) {
		case JMAP_KEY:
			xkey_release(ent.key);
			break;
		case JMAP_JOY:
			joyRelease(comp->joy, ent.dir);
			break;
		case JMAP_MOUSE:
			mouseRelease(comp->mouse, ent.dir);
			break;
	}
}

void MainWin::mapPress(Computer* comp, xJoyMapEntry ent) {
	switch(ent.dev) {
		case JMAP_KEY:
			xkey_press(ent.key);
			break;
		case JMAP_JOY:
			joyPress(comp->joy, ent.dir);
			break;
		case JMAP_MOUSE:
			mousePress(comp->mouse, ent.dir, abs(ent.state / 4096));
			break;
	}
}

int sign(int v) {
	if (v < 0) return -1;
	if (v > 0) return 1;
	return 0;
}

// NOTE: -1 -> 1 | 1 -> -1 = release old one

static QMap<int, QMap<int, int> > jState;

void MainWin::mapJoystick(Computer* comp, int type, int num, int st) {
	int state;
	int hst;
	if (type == JOY_HAT) {
		state = st;
		hst = jState[type][num] ^ st;		// changed only
	} else {
		state = sign(st);
		hst = 0;
	}
	if (jState[type][num] == state) return;
	jState[type][num] = state;
	// xJoyMapEntry xjm;
	QList<xJoyMapEntry> presslist;
	// foreach(xJoyMapEntry& xjm, conf.joy.map) {
	// for(xJoyMapEntry& xjm : conf.joy.map) {
	for (int i = 0; i < conf.joy.map.size(); i++) {
		xJoyMapEntry& xjm = conf.joy.map[i];
		if ((type == xjm.type) && (num == xjm.num)) {
			if ((state == 0) && (type != JOY_HAT)) {
				mapRelease(comp, xjm);
				xjm.cnt = 0;
			} else {
				switch(type) {
					case JOY_AXIS:
						if (sign(state) == sign(xjm.state)) {
							xjm.state = st;
							xjm.cnt = xjm.rpt;
							xjm.rps = 1;
							presslist.append(xjm);
						} else {
							xjm.cnt = 0;
							mapRelease(comp, xjm);
						}
						break;
					case JOY_HAT:
						if (hst & xjm.state) {			// state changed
							if (state & xjm.state) {	// pressed
								xjm.cnt = xjm.rpt;
								xjm.rps = 1;
								presslist.append(xjm);
							} else {			// released
								mapRelease(comp, xjm);
								xjm.cnt = 0;
							}
						}
						break;
					case JOY_BUTTON:
						xjm.cnt = xjm.rpt;
						xjm.rps = 1;
						presslist.append(xjm);
						break;
				}
			}
		}
	}
	foreach(xJoyMapEntry xjm, presslist) {
		mapPress(comp, xjm);
	}
}

// calling on timer every 20ms

static QList<int> fpsmem;

#if defined(__WIN32) && STICKY_KEY

static struct {
	int vkey;
	int state;
	int qkey;		// Qt::Key
	qint32 nCode;		// nativeScanCode = XKEY_* on windows
} win_mod_tab[6] = {
	{VK_RSHIFT, 0, Qt::Key_Shift, XKEY_RSHIFT},
	{VK_LSHIFT, 0, Qt::Key_Shift, XKEY_LSHIFT},
	{VK_RCONTROL, 0, Qt::Key_Control, XKEY_RCTRL},
	{VK_LCONTROL, 0, Qt::Key_Control, XKEY_LCTRL},
	{VK_RMENU, 0, Qt::Key_Alt, XKEY_RALT},
	{VK_LMENU, 0, Qt::Key_Alt, XKEY_LALT}
};

#endif

void MainWin::timerEvent(QTimerEvent* ev) {
	Computer* comp = conf.prof.cur->zx;
	if (ev->timerId() == secid) {		// 0.2 sec timer, fps counter
		// printf("0.2 sec timer event\n");
		if (!conf.emu.pause) {
			// printf("%i (%i)\n", comp->vid->fcnt, fpsmem.first());
			fpsmem.append(conf.vid.fcount);
			conf.vid.curfps = conf.vid.fcount - fpsmem.first();
			while(fpsmem.size() > 5)
				fpsmem.removeFirst();
			// printf("%i\n", conf.vid.curfps);
		}
		if (comp->vid->upd) {
			comp->vid->upd = 0;
			updateWindow();
		}
	} else if (ev->timerId() == cmsid) {	// 1-sec: cmos interrupt
		// TODO: do
	} else if (ev->timerId() == timid) {
// updater
//		if (conf.prof.changed) {
//			comp = conf.prof.cur->zx;
//			conf.prof.changed = 0;
//		}
		if (block) return;
#if HAVEZLIB
		if (comp->rzx.start) {
			emit s_rzx_start();
		} else if (comp->rzx.stop) {
			emit s_rzx_stop();
		}
		if (comp->rzx.overio) {
			comp->rzx.overio = 0;
			pause(true, PR_RZX);
			shitHappens("RZX playback error");
			emit s_rzx_stop();
			pause(false, PR_RZX);
		}
#endif
// buttons autorepeat switcher
		// for(xJoyMapEntry& xjm : conf.joy.map) {
		for (int i = 0; i < conf.joy.map.size(); i++) {
			xJoyMapEntry& xjm = conf.joy.map[i];
			if (xjm.cnt > 0) {
				xjm.cnt--;
				if (xjm.cnt == 0) {
					xjm.cnt = xjm.rpt;
					xjm.rps = !xjm.rps;
					if (xjm.rps) {
						mapPress(comp, xjm);
					} else {
						mapRelease(comp, xjm);
					}
				}
			}
		}
// process sdl events (gamepad)
		int act = conf.joy.joy && !conf.emu.pause && isActiveWindow();
		SDL_Event ev;
		if (act)
			SDL_JoystickUpdate();
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_JOYAXISMOTION:
					if (abs(ev.jaxis.value) < conf.joy.dead) {
						mapJoystick(comp, JOY_AXIS, ev.jaxis.axis, 0);
					} else {
						mapJoystick(comp, JOY_AXIS, ev.jaxis.axis, ev.jaxis.value);
					}
					break;
				case SDL_JOYBUTTONDOWN:
					mapJoystick(comp, JOY_BUTTON, ev.jbutton.button, 32768);
					break;
				case SDL_JOYBUTTONUP:
					mapJoystick(comp, JOY_BUTTON, ev.jbutton.button, 0);
					break;
				case SDL_JOYHATMOTION:
					mapJoystick(comp, JOY_HAT, ev.jhat.hat, ev.jhat.value);
					break;
#if HAVESDL2
				// TODO: select device, if there is more than one
				case SDL_JOYDEVICEREMOVED:
				case SDL_JOYDEVICEADDED:
					if (ev.jdevice.which != 0) break;
					if (conf.joy.joy) {
						SDL_JoystickClose(conf.joy.joy);
						conf.joy.joy = NULL;
					}
					if (SDL_NumJoysticks() > 0) {
						conf.joy.joy = SDL_JoystickOpen(0);
					}
					emit s_gamepad_plug();
					break;
#endif
			}
		}
// process mouse auto move
		comp->mouse->xpos += comp->mouse->autox;
		comp->mouse->ypos += comp->mouse->autoy;
// if computer sends a message, show it
		if (comp->msg) {
			setMessage(QString(comp->msg));
			comp->msg = NULL;
		}
// satelites
		updateSatellites();
#if defined(__WIN32) && STICKY_KEY
		if (!conf.emu.pause && isActiveWindow()) {		// if not paused (!)
			int state;
			QKeyEvent* ev;
			// events: nativeScanCode = 0, nativeVirtualKey = XKEY_*
			for (int i = 0; i < 6; i++) {
				state = GetKeyState(win_mod_tab[i].vkey) & 0x8000;
				if (state != win_mod_tab[i].state) {	// state changed
					if (state) {			// press
						ev = new QKeyEvent(QKeyEvent::KeyPress, win_mod_tab[i].qkey, Qt::NoModifier, 0, win_mod_tab[i].nCode, 0);
					} else {			// release
						ev = new QKeyEvent(QKeyEvent::KeyRelease, win_mod_tab[i].qkey, Qt::NoModifier, 0, win_mod_tab[i].nCode, 0);
					}
					QApplication::postEvent(this, ev);
					win_mod_tab[i].state = state;
				}
			}
		}
#endif
	}
}

// if window is not active release keys & buttons, release mouse
void MainWin::focusOutEvent(QFocusEvent*) {
	Computer* comp = conf.prof.cur->zx;
	mouseReleaseAll(comp->mouse);
	unsetCursor();
	if (grabMice) {
		grabMice = 0;
		releaseMouse();
	}
	emit s_keywin_rall(comp->keyb);
}

void MainWin::focusInEvent(QFocusEvent*) {
	if (conf.emu.pause & PR_DEBUG)
		emit s_debug();
}

void MainWin::moveEvent(QMoveEvent* ev) {
	conf.xpos = pos().x();
	conf.ypos = pos().y();
}

void MainWin::menuShow() {
	Computer* comp = conf.prof.cur->zx;
	layoutMenu->setDisabled(comp->hw->lay != NULL);
	pause(true,PR_MENU);
}

void MainWin::menuHide() {
	setFocus();
	pause(false,PR_MENU);
}


// connection between tape window & tape state

void MainWin::tapStateChanged(int wut, int val) {
	Computer* comp = conf.prof.cur->zx;
	switch(wut) {
		case TW_STATE:
			switch(val) {
				case TWS_PLAY:
					tapPlay(comp->tape);
					emit s_tape_upd(comp->tape);
					break;
				case TWS_STOP:
					tapStop(comp->tape);
					emit s_tape_upd(comp->tape);
					break;
				case TWS_REC:
					tapRec(comp->tape);
					emit s_tape_upd(comp->tape);
					break;
				case TWS_OPEN:
					pause(true,PR_FILE);
					load_file(comp, NULL, FG_TAPE, -1);
					emit s_tape_upd(comp->tape);
					pause(false,PR_FILE);
					break;
			}
			break;
		case TW_REWIND:
			tapRewind(comp->tape,val);
			emit s_tape_upd(comp->tape);
			break;
		case TW_BREAK:
			comp->tape->blkData[val].breakPoint ^= 1;
			emit s_tape_upd(comp->tape);
			break;
	}
}

// connection between rzx player and emulation state
void MainWin::rzxStateChanged(int state) {
#ifdef HAVEZLIB
	Computer* comp = conf.prof.cur->zx;
	switch(state) {
		case RWS_PLAY:
			comp->rzx.start = 0;
			pause(false,PR_RZX);
			break;
		case RWS_PAUSE:
			pause(true,PR_RZX);
			break;
		case RWS_STOP:
			rzxStop(comp);
			comp->rzx.stop = 0;
			pause(false,PR_RZX);
			break;
		case RWS_OPEN:
			pause(true,PR_RZX);
			load_file(comp, NULL, FG_RZX, -1);
			if (comp->rzx.play) {
				emit s_rzx_start();
			}
			pause(false,PR_RZX);
			break;
	}
#endif
}

void MainWin::frame_timer() {
	Computer* comp = conf.prof.cur->zx;
	if (comp) {
		frm_ns += comp->vid->nsPerFrame;
		frm_tmr.setInterval(frm_ns / 1000000);		// 1e6 ns = 1 ms. next frame shot
		frm_ns = frm_ns % 1000000;			// remains
	}
#if defined(USEOPENGL) && !BLOCKGL
	if (conf.emu.fast || conf.emu.pause) {
		glBindTexture(GL_TEXTURE_2D, texids[curtex]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bytesPerLine / 4, comp->vid->vsze.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, comp->debug ? scrimg : bufimg);
		queue.clear();
		queue.append(texids[curtex]);
	}
#endif
	blockSignals(true);
	setUpdatesEnabled(true);
	repaint();				// (?) recursive repaint if signals is on
	setUpdatesEnabled(false);
	blockSignals(false);
}

void MainWin::d_frame() {
	if (conf.emu.fast) return;
#if defined(USEOPENGL) && !BLOCKGL
	Computer* comp = conf.prof.cur->zx;
	queue.append(texids[curtex]);
	if (queue.size() > 3)
		queue.takeFirst();
	glBindTexture(GL_TEXTURE_2D, texids[curtex]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bytesPerLine / 4, comp->vid->vsze.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, comp->debug ? scrimg : bufimg);
	curtex++;
#endif
}

static char numbuf[32];

void drawText(QPainter* pnt, int x, int y, const char* buf) {
	size_t len = strlen(buf);
	for (size_t i = 0; i < len; i++) {
		pnt->drawImage(x, y, alphabet, 0, (buf[i] - 32) * 12, 12, 12);
		x += 12;
	}
}

// bug? QOpenGLWidget crash during show->event (ResizeEvent): calling random slot-signal connection
// repaint/update causes recursion on QOpenGLWidget (whyyyyy?) even if there no paintEvent/paingGL
// if paintEvent exists, paintGL do not called

void MainWin::paintEvent(QPaintEvent*) {
#if defined(USEOPENGL)
#if !BLOCKGL
	makeCurrent();
	if (prg.isLinked() && shd_support) {
		prg.bind();
		prg.setUniformValue("rubyInputSize",GLfloat(bytesPerLine/4.0), GLfloat(conf.prof.cur->zx->vid->vsze.y));
		prg.setUniformValue("rubyOutputSize",GLfloat(width()), GLfloat(height()));
		prg.setUniformValue("rubyTextureSize",GLfloat(bytesPerLine/4.0), GLfloat(conf.prof.cur->zx->vid->vsze.y));
	}
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();
	// draw texture
	if (!queue.isEmpty())
		curtxid = queue.takeFirst();
	glBindTexture(GL_TEXTURE_2D, curtxid);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);	// RT
	glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);	// LT
	glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);	// RB
	glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);	// LB
	glEnd();
	if (prg.isLinked() && shd_support)
		prg.release();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glFlush();
	QPainter pnt(this);
	drawIcons(pnt);
	pnt.end();
#endif
#else
	Computer* comp = conf.prof.cur->zx;
	QPainter pnt(this);
	pnt.drawImage(0,0, QImage(comp->debug ? scrimg : bufimg, width(), height(), QImage::Format_RGBA8888));
	drawIcons(pnt);
	pnt.end();
#endif
}

void MainWin::drawIcons(QPainter& pnt) {
	Computer* comp = conf.prof.cur->zx;
// screenshot
	if (scrCounter > 0) {
		if (scrInterval > 0) {
			scrInterval--;
		} else {
			scrInterval = conf.scrShot.interval;
			scrCounter--;
			screenShot();
		}
	}
// put leds
// keyboard
	if (conf.led.keys && (comp->hw->grp == HWG_ZX)) {
		pnt.drawImage(3, 10, QImage(":/images/scanled.png"));
		if (~comp->keyb->port & 0x01) pnt.fillRect(3 + 3, 10 + 17, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x02) pnt.fillRect(3 + 3, 10 + 14, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x04) pnt.fillRect(3 + 3, 10 + 11, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x08) pnt.fillRect(3 + 3, 10 + 8, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x80) pnt.fillRect(3 + 12, 10 + 17, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x40) pnt.fillRect(3 + 12, 10 + 14, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x20) pnt.fillRect(3 + 12, 10 + 11, 8, 2, Qt::gray);
		if (~comp->keyb->port & 0x10) pnt.fillRect(3 + 12, 10 + 8, 8, 2, Qt::gray);
	}
	comp->keyb->port = 0xff;
// joystick
	if (comp->joy->used && conf.led.joy) {
		pnt.drawImage(3, 30, QImage(":/images/joystick.png"));
		comp->joy->used = 0;
	}
// mouse
	if (comp->mouse->used && conf.led.mouse) {
		pnt.drawImage(3, 50, QImage(":/images/mouse.png"));
		comp->mouse->used = 0;
	}
// tape
	if (comp->tape->on && conf.led.tape) {
		pnt.drawImage(3, 70, comp->tape->rec ? QImage(":/images/tapeRed.png") : QImage(":/images/tapeYellow.png"));
	}
// disc
	if (conf.led.disk) {
		if (comp->dif->fdc->flp->rd) {
			comp->dif->fdc->flp->rd = 0;
			pnt.drawImage(3, 90, QImage(":/images/diskGreen.png"));
		} else if (comp->dif->fdc->flp->wr) {
			comp->dif->fdc->flp->wr = 0;
			pnt.drawImage(3, 90, QImage(":/images/diskRed.png"));
		}
	}
// waveout
	if (conf.snd.wavout) {
		pnt.drawImage(3, 110, QImage(":/images/wav.png"));
	}
// put fps
	if (conf.led.fps) {
		sprintf(numbuf, " %d ", conf.vid.curfps);
		drawText(&pnt, width() - (strlen(numbuf) * 12) - 5, 5, numbuf);
	}
// put halt counter
	if (conf.led.halt) {
		sprintf(numbuf, " %d : %d ",comp->hCount, comp->fCount);
		drawText(&pnt, width() - strlen(numbuf) * 12 - 8, height() - 20, numbuf);
	}
// put messages
	if (msgTimer > 0) {
		if (conf.led.message) {
			drawText(&pnt, 5, height() - 20, msg.toLocal8Bit().data());
		}
		msgTimer--;
	}
// end
}


void MainWin::dragEnterEvent(QDragEnterEvent* ev) {
	if (ev->mimeData()->hasUrls()) {
		ev->acceptProposedAction();
	}
}

void MainWin::dropEvent(QDropEvent* ev) {
	QList<QUrl> urls = ev->mimeData()->urls();
	QString fpath;
	raise();
	activateWindow();
	for (int i = 0; i < urls.size(); i++) {
		fpath = urls.at(i).path();
#if defined(__WIN32)
		fpath.remove(0,1);	// by some reason path will start with /
#endif
		load_file(conf.prof.cur->zx, fpath.toLocal8Bit().data(), FG_ALL, 0);
	}
}


void MainWin::closeEvent(QCloseEvent* ev) {
	pause(true,PR_EXIT);
	Computer* comp = conf.prof.cur->zx;
	if (conf.confexit) {
		if (!areSure("Quit?")) {
			ev->ignore();
			pause(false, PR_EXIT);
			return;
		}
	}
//	std::string fname;
	foreach(xProfile* prf, conf.prof.list) {
		prfSave(prf->name);
	}
	if (saveChanged()) {
		snd_wav_close();
		frm_tmr.stop();
		killTimer(timid);
		killTimer(secid);
		ideCloseFiles(comp->ide);
		sdcCloseFile(comp->sdc);
		sltEject(comp->slot);		// this must save cartridge ram
		emit s_keywin_close();
		if (conf.joy.joy)
			SDL_JoystickClose(conf.joy.joy);
		saveConfig();
#ifdef USENETWORK
		closeServer();
#endif
		emit s_emulwin_close();
		ev->accept();
	} else {
		ev->ignore();
		pause(false,PR_EXIT);
	}
}

void MainWin::checkState() {
#ifdef HAVEZLIB
	Computer* comp = conf.prof.cur->zx;
	if (comp->rzx.start) {
		emit s_rzx_start();
	} else if (comp->rzx.stop) {
		emit s_rzx_stop();
	}
		//rzxWin->startPlay();
#endif
	//emit s_tape_list(comp->tape);
	//tapeWin->buildList(comp->tape);
	//tapeWin->setCheck(comp->tape->block);
}

// ...

uchar hobHead[] = {'s','c','r','e','e','n',' ',' ','C',0,0,0,0x1b,0,0x1b,0xe7,0x81};	// last 2 bytes is crc

void MainWin::screenShot() {
	Computer* comp = conf.prof.cur->zx;
	int frm = shotFormat[conf.scrShot.format];
	std::string fext;
	switch (frm) {
		case SCR_BMP: fext = "bmp"; break;
		case SCR_PNG: fext = "png"; break;
		case SCR_JPG: fext = "jpg"; break;
		case SCR_SCR: fext = "scr"; break;
		case SCR_HOB: fext = "$C"; break;
	}
	QString fnams = QString(conf.scrShot.dir.c_str()).append(SLASH);
	fnams.append(QString("xpeccy_%0.%1").arg(QTime::currentTime().toString("HHmmss_zzz")).arg(fext.c_str()));
	std::string fnam(fnams.toUtf8().data());
	std::ofstream file;
#if defined(USEOPENGL)
	QImage img(bufimg, bytesPerLine / 4, comp->vid->vsze.y, QImage::Format_RGBA8888);
	img = img.scaled(width(), height());
#else
	QImage img(bufimg, width(), height(), QImage::Format_RGBA8888);
#endif
	int x,y,dx,dy;
	char* sptr = (char*)(comp->mem->ramData + (comp->vid->curscr << 14));
	switch (frm) {
		case SCR_HOB:
			file.open(fnam.c_str(),std::ios::binary);
			file.write((char*)hobHead,17);
			file.write(sptr, 0x1b00);
			file.close();
			break;
		case SCR_SCR:
			file.open(fnam.c_str(),std::ios::binary);
			file.write(sptr, 0x1b00);
			file.close();
			break;
		case SCR_BMP:
		case SCR_JPG:
		case SCR_PNG:
			if (img.isNull()) break;
			if (conf.scrShot.noBorder) {
				x = (comp->vid->bord.x - comp->vid->lcut.x) * conf.vid.scale;
				y = (comp->vid->bord.y - comp->vid->lcut.y) * conf.vid.scale;
				dx = comp->vid->scrn.x * conf.vid.scale;
				dy = comp->vid->scrn.y * conf.vid.scale;
				img = img.copy(x, y, dx, dy);
			}
			img.save(QString(fnam.c_str()),fext.c_str());
			break;
	}
	setMessage("screenshot saved");
}

// video drawing

QList<xLed> leds;

void drawLeds(QPainter& pnt) {
	xLed* led;
	int x,y;
	int i;
	for (i = 0; i < leds.size(); i++) {
		led = &leds[i];
		if (led->showTime > 0) {
			led->showTime--;
			x = led->x;
			y = led->y;
			if (x < 0) x = pnt.device()->width() + x;
			if (y < 0) y = pnt.device()->height() + y;
			pnt.drawImage(x, y, QImage(led->imgName).scaled(16,16));
		}
	}
}

void addLed(int x, int y, QString name, int time) {
	xLed led;
	led.x = x;
	led.y = y;
	led.imgName = name;
	led.showTime = time;
	for (int i = 0; i < leds.size(); i++) {
		if (leds[i].imgName == name) {
			leds[i] = led;
			name.clear();
		}
	}
	if (!name.isEmpty()) {
		leds.append(led);
	}
}

void MainWin::setMessage(QString str, double dur) {
	msgTimer = (int)(dur * 50);
	msg = str;
}

void MainWin::updateSatellites() {
	if (block) return;
	Computer* comp = conf.prof.cur->zx;
// update rzx window
#ifdef HAVEZLIB
	emit s_rzx_upd(comp);
#endif
// update tape window
	if (comp->tape->on && !comp->tape->rec)
		emit s_tape_progress(comp->tape);
	if ((comp->tape->on && !comp->tape->rec) || comp->tape->blkChange || comp->tape->newBlock) {
		emit s_tape_upd(comp->tape);
		if (comp->tape->blkChange || comp->tape->newBlock) {
			emit s_tape_blk(comp->tape);
			comp->tape->blkChange = 0;
			comp->tape->newBlock = 0;
		}
	}
// update watcher
	emit s_watch_upd(comp);
}

// USER MENU

void MainWin::initUserMenu() {
	userMenu = new QMenu(this);
// submenu
	fileMenu = userMenu->addMenu(QIcon(":/images/fileopen.png"),"Open...");
	bookmarkMenu = userMenu->addMenu(QIcon(":/images/star.png"),"Bookmarks");
	profileMenu = userMenu->addMenu(QIcon(":/images/profile.png"),"Profiles");
	layoutMenu = userMenu->addMenu(QIcon(":/images/display.png"),"Layout");
	keyMenu = userMenu->addMenu(QIcon(":/images/keyboardzx.png"), "Keymap...");
	resMenu = userMenu->addMenu(QIcon(":/images/shutdown.png"),"Reset...");
	shdMenu = userMenu->addMenu(QIcon(":/images/shader.png"), "Shaders");

	userMenu->addSeparator();
	userMenu->addAction(QIcon(":/images/tape.png"), "Tape player", this, SIGNAL(s_tape_show()));
	userMenu->addAction(QIcon(":/images/video.png"),"RZX player", this, SIGNAL(s_rzx_show()));
	userMenu->addSeparator();
	pckAct = userMenu->addAction(QIcon(":/images/keyboard.png"),"Grab keyboard");
	pckAct->setCheckable(true);
	userMenu->addAction(QIcon(":/images/keyboardzx.png"),"ZX Keyboard",this,SIGNAL(s_keywin_shide()));
	userMenu->addAction(QIcon(":/images/objective.png"),"Watcher", this, SIGNAL(s_watch_show()));
	userMenu->addAction(QIcon(":/images/bug.png"), "deBUGa", this, SLOT(doDebug()));
	userMenu->addAction(QIcon(":/images/other.png"),"Options",this,SLOT(doOptions()));

	connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	connect(layoutMenu,SIGNAL(triggered(QAction*)),this,SLOT(chLayout(QAction*)));
	connect(resMenu,SIGNAL(triggered(QAction*)),this,SLOT(reset(QAction*)));
	connect(fileMenu,SIGNAL(triggered(QAction*)),this,SLOT(umOpen(QAction*)));
	connect(shdMenu,SIGNAL(triggered(QAction*)),this,SLOT(shdSelected(QAction*)));
	connect(keyMenu,SIGNAL(triggered(QAction*)),this,SLOT(keySelected(QAction*)));

	fileMenu->addAction(QIcon(":/images/memory.png"),"Snapshot")->setData(FG_SNAPSHOT);
	fileMenu->addAction(QIcon(":/images/tape.png"),"Tape")->setData(FG_TAPE);
	fileMenu->addAction(QIcon(":/images/floppy.png"),"Floppy")->setData(FH_DISKS);

	resMenu->addAction("default")->setData(RES_DEFAULT);
	resMenu->addSeparator();
	resMenu->addAction("ROMpage0")->setData(RES_128);
	resMenu->addAction("ROMpage1")->setData(RES_48);
	resMenu->addAction("ROMpage2")->setData(RES_SHADOW);
	resMenu->addAction("ROMpage3")->setData(RES_DOS);
#ifdef ISDEBUG
	userMenu->addSeparator();
	dbgMenu = userMenu->addMenu(QIcon(":/images/debuga.png"),"Debug");
	dbgMenu->addAction(QIcon(),QString("Save v9938 vram..."),this,SLOT(saveVRAM()));
	dbgMenu->addAction(QIcon(),QString("Save GB VRAM..."), this, SLOT(saveGBVRAM()));
//	dbgMenu->addAction(QIcon(),QString("Save GS RAM..."),this,SLOT(saveGSRAM()));
	dbgMenu->addAction(QIcon(),QString("Save NES PPU vram..."),this,SLOT(saveNESPPU()));
#endif
}

void MainWin::fillUserMenu() {
	// fill bookmark menu
	bookmarkMenu->clear();
	QAction* act;
	if (conf.bookmarkList.size() == 0) {
		bookmarkMenu->addAction("None")->setEnabled(false);
	} else {
		foreach(xBookmark bkm, conf.bookmarkList) {
			act = bookmarkMenu->addAction(QString::fromLocal8Bit(bkm.name.c_str()));
			act->setData(QVariant(QString::fromLocal8Bit(bkm.path.c_str())));
		}
	}
	// fill profile menu
	profileMenu->clear();
	foreach(xProfile* prf, conf.prof.list) {
		profileMenu->addAction(prf->name.c_str())->setData(prf->name.c_str());
	}
	// fill layout menu
	layoutMenu->clear();
	foreach(xLayout lay, conf.layList) {
		layoutMenu->addAction(lay.name.c_str())->setData(lay.name.c_str());
	}
	// fill keymaps menu
	keyMenu->clear();
	act = keyMenu->addAction("Default");
	act->setData("");
	act->setCheckable(true);
	if (conf.prof.cur) {
		if (conf.prof.cur->kmapName.empty()) act->setChecked(true);
		QDir dir(conf.path.confDir.c_str());
		QStringList lst = dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name);
		dir.setPath(dir.path().append("/keymaps/"));
		lst.append(dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name));
		lst.sort();
		foreach(QString str, lst) {
			act = keyMenu->addAction(str);
			act->setData(str);
			act->setCheckable(true);
			act->setChecked(conf.prof.cur->kmapName == std::string(str.toUtf8().data()));
		}
	}
	// fill shader menu
	shdMenu->clear();
	act = shdMenu->addAction("none");
	act->setData("");
	act->setCheckable(true);
	if (conf.vid.shader.empty()) act->setChecked(true);
#if defined(USEOPENGL) && !BLOCKGL
	if (shd_support) {
		QDir dir(conf.path.shdDir.c_str());
		QFileInfoList lst = dir.entryInfoList(QStringList() << "*.txt", QDir::Files, QDir::Name);
		foreach(QFileInfo inf, lst) {
			act = shdMenu->addAction(inf.fileName());
			act->setData(inf.fileName());
			act->setCheckable(true);
			act->setChecked(inf.fileName() == conf.vid.shader.c_str());
		}
	}
#endif
}

// SLOTS

void MainWin::doOptions() {
	pause(true, PR_OPTS);
	emit s_options();
}

void MainWin::optApply() {
//	relskip = 1;
//	comp = conf.prof.cur->zx;
	Computer* comp = conf.prof.cur->zx;
	fillUserMenu();
	updateWindow();
#ifdef USENETWORK
	if (srv.serverPort() != conf.port) {
		closeServer();
		openServer();

	}
#endif
#if defined(USEOPENGL) && !BLOCKGL
	loadShader();
#endif
	emit s_tape_upd(comp->tape);
	pause(false, PR_OPTS);
}

void MainWin::doDebug() {
	conf.emu.fast = 0;
	pause(true, PR_DEBUG);
	emit s_debug();
}

void MainWin::dbgReturn() {
	pause(false, PR_DEBUG);
}

void MainWin::bookmarkSelected(QAction* act) {
	Computer* comp = conf.prof.cur->zx;
	load_file(comp, act->data().toString().toLocal8Bit().data(), FG_ALL, 0);
	setFocus();
}

void MainWin::onPrfChange() {
	Computer* comp = conf.prof.cur->zx;
	if (comp->firstRun) {
		compReset(comp, RES_DEFAULT);
		comp->firstRun = 0;
	}
	emit s_keywin_upd(comp->keyb);
	vid_upd_scale();
	updateWindow();
//	saveConfig();
//	emit s_prf_change(conf.prof.cur);
//	conf.prof.changed = 1;
}

void MainWin::profileSelected(QAction* act) {
	std::string str = QString(act->data().toByteArray()).toStdString();
	prfSetCurrent(str);
	onPrfChange();
}

void MainWin::reset(QAction* act) {
	Computer* comp = conf.prof.cur->zx;
	emit s_rzx_stop();
	compReset(comp,act->data().toInt());
}

void MainWin::chLayout(QAction* act) {
	std::string str = QString(act->data().toByteArray()).toStdString();
	prfSetLayout(NULL, str);
	prfSave("");
	updateWindow();
}

void MainWin::umOpen(QAction* act) {
	pause(true, PR_FILE);
	Computer* comp = conf.prof.cur->zx;
	load_file(comp, NULL, act->data().toInt(), -1);
	pause(false, PR_FILE);
}

void MainWin::keySelected(QAction* act) {
	QString str = act->data().toString();
	if (str.isEmpty()) {
		conf.prof.cur->kmapName.clear();
	} else {
		conf.prof.cur->kmapName = std::string(str.toUtf8().data());
	}
	loadKeys();
}

// debug stufffff

void MainWin::saveVRAM() {
	QString path = QFileDialog::getSaveFileName(this,"Save VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	xColor xcol;
	Computer* comp = conf.prof.cur->zx;
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->ram, 0x20000);
		file.write((char*)comp->vid->reg, 64);
		for (int i = 0; i < 16; i++) {
			xcol = vid_get_col(comp->vid, i);
			file.putChar(xcol.r);
			file.putChar(xcol.g);
			file.putChar(xcol.b);
		}

		file.close();
	}
}

void MainWin::saveGBVRAM() {
	QString path = QFileDialog::getSaveFileName(this,"Save GB VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	Computer* comp = conf.prof.cur->zx;
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->ram, 0x2000);
		file.write((char*)comp->gb.iomap, 0x80);
		file.close();
	}
}

void MainWin::saveNESPPU() {
	QString path = QFileDialog::getSaveFileName(this,"Save GB VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	Computer* comp = conf.prof.cur->zx;
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->ram, 0x4000);
		file.write((char*)comp->vid->oam, 0x100);
		file.close();
	}
}

void MainWin::debugAction() {
	sndDebug();
}
