#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTime>
#include <QUrl>
#include <QMimeData>
#include <QPainter>
#include <QDesktopWidget>

#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xcore.h"
#include "sound.h"
#include "emulwin.h"
#include "filer.h"
#include "watcher.h"

#include "vfilters.h"
#include "vscalers.h"

#include "version.h"

#define STR_EXPAND(tok) #tok
#define	STR(tok) STR_EXPAND(tok)
#define	XPTITLE	STR(Xpeccy VERSION)

// main

extern unsigned char* bufimg;
extern unsigned char* scrimg;

extern int bytesPerLine;

static QPainter pnt;
static QImage alphabet;

// mainwin

void MainWin::updateHead() {
	QString title(XPTITLE);
#ifdef ISDEBUG
	title.append(" | debug");
#endif
	if (conf.prof.cur) {
		title.append(" | ").append(QString::fromLocal8Bit(conf.prof.cur->name.c_str()));
		title.append(" | ").append(QString::fromLocal8Bit(conf.prof.cur->layName.c_str()));
	}
	if (conf.emu.fast) {
		title.append(" | fast");
	}
	setWindowTitle(title);
}

void MainWin::updateWindow() {
	block = 1;
	vidSetBorder(comp->vid, conf.brdsize);		// to call vidUpdateLayout???
	int szw = comp->vid->vsze.x * conf.vid.scale;
	int szh = comp->vid->vsze.y * conf.vid.scale;
	if (conf.vid.fullScreen) {
		QSize wsz = QApplication::desktop()->screenGeometry().size();
		szw = wsz.width();
		szh = wsz.height();
		setWindowState(windowState() | Qt::WindowFullScreen);
	} else {
		setWindowState(windowState() & ~Qt::WindowFullScreen);
	}
	setFixedSize(szw, szh);
	lineBytes = szw * 3;
	frameBytes = szh * lineBytes;
	bytesPerLine = lineBytes;
	if (bytesPerLine & 3)		// 4 bytes align for QImage data
		bytesPerLine = (bytesPerLine & ~3) + 4;
	vid_set_zoom(conf.vid.scale);
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
						yep &= save_file(comp, flp->path, FG_DISK, i);
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
	setWindowTitle(XPTITLE);
	setMouseTracking(true);
	icon = QIcon(":/images/xpeccy.png");
	setWindowIcon(icon);
	setAcceptDrops(true);
	setAutoFillBackground(false);
	setUpdatesEnabled(false);
	conf.emu.pause = 0;
	scrCounter = 0;
	scrInterval = 0;
	grabMice = 0;
	block = 0;

	msgTimer = 0;
	msg.clear();
	alphabet.load(":/font.png");

	conf.scrShot.format = "png";
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	addLayout("default", vlay);

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

	timid = startTimer(20);
	secid = startTimer(200);

	connect(userMenu,SIGNAL(aboutToShow()),SLOT(menuShow()));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));

	fillUserMenu();

#ifdef USENETWORK
	srv.listen(QHostAddress::LocalHost, conf.port);
	if (!srv.isListening()) {
		shitHappens("Listen server can't start");
	} else {
		printf("Listening port %i\n",conf.port);
	}

	connect(&srv, SIGNAL(newConnection()),this,SLOT(connected()));
#endif
}

// gamepad mapper
// xJoyMapEntry::key is XKEY_*

void MainWin::mapRelease(Computer* comp, xJoyMapEntry ent) {
	switch(ent.dev) {
		case JMAP_KEY:
			xkey_release(ent.key, 0);
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
			xkey_press(ent.key, 0);
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
	for(xJoyMapEntry& xjm : conf.joy.map) {
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

void MainWin::timerEvent(QTimerEvent* ev) {
// fps (1000 ms)
	if (ev->timerId() == secid) {
		if (!conf.emu.pause) {
			fpsmem.append(conf.vid.fcount);
			conf.vid.curfps = conf.vid.fcount - fpsmem.first();
			while(fpsmem.size() > 5)
				fpsmem.removeFirst();
		}
	} else {
// updater
		if (conf.prof.changed) {
			comp = conf.prof.cur->zx;
			conf.prof.changed = 0;
		}
		if (block) return;
#if HAVEZLIB
		if (comp->rzx.start) {
			emit s_rzx_start();
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
		for(xJoyMapEntry& xjm : conf.joy.map) {
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
// process sdl event (gamepad)
		if (conf.joy.joy && !conf.emu.pause) {
			SDL_Event ev;
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
				}
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
// redraw window (if fast || paused)
		if (conf.emu.fast || conf.emu.pause) {
			setUpdatesEnabled(true);
			repaint();
			setUpdatesEnabled(false);
		}
	}
}

// if window is not active release keys & buttons, release mouse
void MainWin::focusOutEvent(QFocusEvent*) {
//	kbdReleaseAll(comp->keyb);
	mouseReleaseAll(comp->mouse);
	unsetCursor();
	if (grabMice) {
		grabMice = 0;
		releaseMouse();
	}
	emit s_keywin_rall(comp->keyb);
}

void MainWin::menuShow() {
	layoutMenu->setDisabled(comp->vid->lockLayout);
	pause(true,PR_MENU);
}

void MainWin::menuHide() {
	setFocus();
	pause(false,PR_MENU);
}


// connection between tape window & tape state

void MainWin::tapStateChanged(int wut, int val) {
	switch(wut) {
		case TW_STATE:
			switch(val) {
				case TWS_PLAY:
					if (tapPlay(comp->tape)) {
						//emit s_tape_state(TWS_PLAY);
						//tapeWin->setState(TWS_PLAY);
					} else {
						//emit s_tape_state(TWS_STOP);
						//tapeWin->setState(TWS_STOP);
					}
					break;
				case TWS_STOP:
					tapStop(comp->tape);
					//emit s_tape_state(TWS_STOP);
					//tapeWin->setState(TWS_STOP);
					break;
				case TWS_REC:
					tapRec(comp->tape);
					//emit s_tape_state(TWS_REC);
					//tapeWin->setState(TWS_REC);
					break;
				case TWS_OPEN:
					pause(true,PR_FILE);
					//loadFile(comp,"",FT_TAPE,-1);
					load_file(comp, NULL, FG_TAPE, -1);
					//emit s_tape_list(comp->tape);
					//tapeWin->buildList(comp->tape);
					//tapeWin->setCheck(comp->tape->block);
					pause(false,PR_FILE);
					break;
			}
			break;
		case TW_REWIND:
			tapRewind(comp->tape,val);
			emit s_tape_upd(comp->tape);
			//tapeWin->buildList(comp->tape);
			break;
		case TW_BREAK:
			comp->tape->blkData[val].breakPoint ^= 1;
			emit s_tape_upd(comp->tape);
			//tapeWin->drawStops(comp->tape);
			break;
	}
}

// connection between rzx player and emulation state
void MainWin::rzxStateChanged(int state) {
#ifdef HAVEZLIB
	switch(state) {
		case RWS_PLAY:
			pause(false,PR_RZX);
			break;
		case RWS_PAUSE:
			pause(true,PR_RZX);
			break;
		case RWS_STOP:
			rzxStop(comp);
			pause(false,PR_RZX);
			break;
		case RWS_OPEN:
			pause(true,PR_RZX);
			//loadFile(comp,"",FT_RZX,0);
			load_file(comp, NULL, FG_RZX, -1);
			if (comp->rzx.play) {
				emit s_rzx_start();
				//rzxWin->startPlay();
			}
			pause(false,PR_RZX);
			break;
	}
#endif
}

void MainWin::d_frame() {
	if (conf.emu.fast) return;
	setUpdatesEnabled(true);
	repaint();
	setUpdatesEnabled(false);
}

static char numbuf[32];

void MainWin::paintEvent(QPaintEvent*) {
	if (block) return;
	int i,x,y,chr;
	pnt.begin(this);
	pnt.drawImage(0,0, QImage(comp->debug ? scrimg : bufimg, width(), height(), QImage::Format_RGB888));
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
	if (comp->joy->used && conf.led.joy) {
		pnt.drawImage(3, 30, QImage(":/images/joystick.png").scaled(16, 16));
	}
	if (comp->mouse->used && conf.led.mouse) {
		pnt.drawImage(3, 50, QImage(":/images/mouse.png").scaled(16, 16));
		comp->mouse->used = 0;
	}
	if (comp->tape->on && conf.led.tape) {
		if (comp->tape->rec) {
			pnt.drawImage(3, 70, QImage(":/images/tapeRed.png").scaled(16,16));
		} else {
			pnt.drawImage(3, 70, QImage(":/images/tapeYellow.png").scaled(16,16));
		}
	}
	if (conf.led.disk) {
		if (comp->dif->fdc->flp->rd) {
			comp->dif->fdc->flp->rd = 0;
			pnt.drawImage(3, 90, QImage(":/images/diskGreen.png").scaled(16,16));
		} else if (comp->dif->fdc->flp->wr) {
			comp->dif->fdc->flp->wr = 0;
			pnt.drawImage(3, 90, QImage(":/images/diskRed.png").scaled(16,16));
		}
	}
// put fps
	if (conf.led.fps) {
		sprintf(numbuf, " %d ", conf.vid.curfps);
		x = 5;
		y = 5;
		i = 0;
		while (numbuf[i] != 0) {
			chr = numbuf[i] - 32;
			pnt.drawImage(x, y, alphabet, 0, chr * 12, 12, 12);
			x += 12;
			i++;
		}
	}
// put messages
	if (msgTimer > 0) {
		if (conf.led.message) {
			x = 5;
			y = height() - 20;
			for (int i = 0; i < msg.size(); i++) {
				chr = msg.at(i).unicode() - 32;
				pnt.drawImage(x, y, alphabet, 0, chr * 12, 12, 12);
				x += 12;
			}
		}
		msgTimer--;
	}
// end
	pnt.end();
}

void MainWin::kPress(QKeyEvent* ev) {
	keyPressEvent(ev);
}

void MainWin::kRelease(QKeyEvent* ev) {
	keyReleaseEvent(ev);
}

void MainWin::keyPressEvent(QKeyEvent *ev) {
// #if __APPLE__
	if (ev->isAutoRepeat()) return;
// #endif
	if (comp->debug) {
		ev->ignore();
	} else {
#if defined(__linux) || defined(_WIN32)
		int keyid = ev->nativeScanCode();
#else
		int keyid = qKey2id(ev->key());
#endif
		xkey_press(keyid, ev->modifiers());
	}
}

void MainWin::xkey_press(int xkey, Qt::KeyboardModifiers mod) {
	keyEntry kent = getKeyEntry(xkey);
//	qDebug() << kent.name << kent.zxKey.key1 << kent.zxKey.key2;
	if (pckAct->isChecked()) {
		xt_press(comp->keyb, kent.keyCode);
		if (comp->hw->keyp) {
			comp->hw->keyp(comp, kent);
		}
		if (xkey == XKEY_F12) {
			compReset(comp,RES_DEFAULT);
			emit s_rzx_stop();
			// rzxWin->stop();
		}
	} else if (mod & Qt::AltModifier) {
		switch(xkey) {
			case XKEY_ENTER:
				vid_set_fullscreen(!conf.vid.fullScreen);
				setMessage(conf.vid.fullScreen ? " fullscreen on " : " fullscreen off ");
				updateWindow();
				saveConfig();
				break;
#ifdef ISDEBUG
			case XKEY_HOME:
				debugAction();
				break;
#endif
			case XKEY_1:
				vid_set_zoom(1);
				updateWindow();
				saveConfig();
				setMessage(" size x1 ");
				break;
			case XKEY_2:
				vid_set_zoom(2);
				updateWindow();
				saveConfig();
				setMessage(" size x2 ");
				break;
			case XKEY_3:
				vid_set_zoom(3);
				updateWindow();
				saveConfig();
				setMessage(" size x3 ");
				break;
			case XKEY_4:
				vid_set_zoom(4);
				updateWindow();
				saveConfig();
				setMessage(" size x4 ");
				break;
			case XKEY_F4:
				close();
				break;
			case XKEY_F7:
				scrCounter = conf.scrShot.count;
				scrInterval = 0;
				break;	// ALT+F7 combo
			case XKEY_F12:
				compReset(comp,RES_DOS);
				emit s_rzx_stop();
				break;
			case XKEY_K:
				emit s_keywin_shide();
				break;
			case XKEY_N:
				if (noflic < 15)
					noflic = 25;
				else if (noflic < 35)
					noflic = 50;
				else noflic = 0;
				saveConfig();
				setMessage(QString(" noflick %0% ").arg(noflic * 2));
				break;
			case XKEY_R:
				vid_set_ratio(!conf.vid.keepRatio);
				setMessage(conf.vid.keepRatio ? " keep aspect ratio " : " ignore aspect ratio ");
				saveConfig();
				break;
			case XKEY_M:
				grabMice = !grabMice;
				if (grabMice) {
					grabMouse(QCursor(Qt::BlankCursor));
					setMessage(" grab mouse ");
				} else {
					releaseMouse();
					setMessage(" release mouse ");
				}
				break;
		}
	} else {
//		if (ev->modifiers() & Qt::ShiftModifier) {
//			if (comp->hw->keyr)
//				comp->hw->keyr(comp, getKeyEntry(XKEY_LSHIFT));
//		}
		switch(xkey) {
			case XKEY_PAUSE:
				conf.emu.pause ^= PR_PAUSE;
				pause(true,0);
				break;
			case XKEY_ESC:
				conf.emu.fast = 0;
				pause(true, PR_DEBUG);
				setUpdatesEnabled(true);
				emit s_debug(comp);
				break;
			case XKEY_MENU:
				userMenu->popup(pos() + QPoint(20,20));
				userMenu->setFocus();
				break;
			case XKEY_NUMLCK:
			case XKEY_INS:
				if (conf.emu.pause) break;
				conf.emu.fast ^= 1;
				updateHead();
				break;
			case XKEY_F1:
				pause(true, PR_OPTS);
				emit s_options(conf.prof.cur);
				break;
			case XKEY_F2:
				pause(true,PR_FILE);
				save_file(comp, NULL, FG_ALL, -1);
				pause(false,PR_FILE);
				break;
			case XKEY_F3:
				pause(true,PR_FILE);
				load_file(comp, NULL, FG_ALL, -1);
				pause(false,PR_FILE);
				checkState();
				break;
			case XKEY_F4:
				if (comp->tape->on) {
					tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					tapStateChanged(TW_STATE,TWS_PLAY);
				}
				break;
			case XKEY_F5:
				if (comp->tape->on) {
					tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					tapStateChanged(TW_STATE,TWS_REC);
				}
				break;
			case XKEY_F7:
				if (scrCounter == 0) {
					scrCounter = 1;
					scrInterval = 0;
				} else {
					scrCounter = 0;
				}
				break;
			case XKEY_F8:
				/*
				if (rzxWin->isVisible()) {
					rzxWin->hide();
				} else {
					rzxWin->show();
				}
				*/
				break;
			case XKEY_F9:
				pause(true,PR_FILE);
				saveChanged();
				pause(false,PR_FILE);
				break;
			case XKEY_F10:
				if (comp->cpu->type != CPU_Z80) break;
				comp->nmiRequest = 1;
				break;
			case XKEY_F11:
				/*
				if (tapeWin->isVisible()) {
					tapeWin->hide();
				} else {
					tapeWin->buildList(comp->tape);
					tapeWin->show();
				}
				*/
				break;
			case XKEY_F12:
				compReset(comp,RES_DEFAULT);
				emit s_rzx_stop();
				break;
			default:
				// printf("%s %c %c\n", kent.name, kent.zxKey.key1, kent.zxKey.key2);
				xt_press(comp->keyb, kent.keyCode);
				if (comp->hw->keyp) {
					comp->hw->keyp(comp, kent);
				}
				break;
		}
	}
	emit s_keywin_upd(comp->keyb);
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	if (ev->isAutoRepeat())
		return;
	int keyid;
	if (comp->debug) {
		ev->ignore();
	} else {
#if defined(__linux) || defined(_WIN32)
		keyid = ev->nativeScanCode();
#else
		keyid = qKey2id(ev->key());
#endif
		xkey_release(keyid, ev->modifiers());
	}
}

void MainWin::xkey_release(int keyid, Qt::KeyboardModifiers) {
	keyEntry kent = getKeyEntry(keyid);
//	qDebug() << "release" << kent.name;
	xt_release(comp->keyb, kent.keyCode);
	if (comp->hw->keyr)
		comp->hw->keyr(comp, kent);
	emit s_keywin_upd(comp->keyb);
}

void MainWin::mousePressEvent(QMouseEvent *ev){
	if (comp->debug) {
		ev->ignore();
		return;
	}
	switch (ev->button()) {
		case Qt::LeftButton:
			if (!grabMice) break;
			comp->mouse->lmb = 1;
			break;
		case Qt::RightButton:
			if (grabMice) {
				comp->mouse->rmb = 1;
			} else {
				userMenu->popup(QPoint(ev->globalX(),ev->globalY()));
				userMenu->setFocus();
			}
			break;
		default: break;
	}
}

void MainWin::mouseReleaseEvent(QMouseEvent *ev) {
	if (conf.emu.pause) return;
	if (comp->debug) {
		ev->ignore();
		return;
	}
	switch (ev->button()) {
		case Qt::LeftButton:
			if (!grabMice) break;
			comp->mouse->lmb = 0;
			break;
		case Qt::RightButton:
			if (!grabMice) break;
			comp->mouse->rmb = 0;
			break;
		case Qt::MidButton:
			grabMice = !grabMice;
			if (grabMice) {
				grabMouse(QCursor(Qt::BlankCursor));
				setMessage(" grab mouse ");
			} else {
				releaseMouse();
				setMessage(" release mouse ");
			}
			break;
		default: break;
	}
}

void MainWin::wheelEvent(QWheelEvent* ev) {
	if (comp->debug) {
		ev->ignore();
		return;
	}
	if (grabMice) {
		if (comp->mouse->hasWheel)
			mousePress(comp->mouse, (ev->delta() < 0) ? XM_WHEELDN : XM_WHEELUP, 0);
	} else {
		if (ev->delta() < 0) {
			conf.snd.vol.master -= 5;
			if (conf.snd.vol.master < 0)
				conf.snd.vol.master = 0;
		} else {
			conf.snd.vol.master += 5;
			if (conf.snd.vol.master > 100)
				conf.snd.vol.master = 100;
		}
		setMessage(QString(" volume %0% ").arg(conf.snd.vol.master));
	}
}

static int dumove = 0;

void MainWin::mouseMoveEvent(QMouseEvent *ev) {
	if (!grabMice || conf.emu.pause) return;
	if (dumove) {			// it was dummy move to center of screen
		dumove = 0;
	} else {
		QRect rct = QApplication::desktop()->screenGeometry();
		rct.setWidth(rct.width() / 2);
		rct.setHeight(rct.height() / 2);
		comp->mouse->xpos += ev->globalX() - rct.width();
		comp->mouse->ypos -= ev->globalY() - rct.height();
		dumove = 1;
		cursor().setPos(rct.width(), rct.height());
	}
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
#ifdef _WIN32
		fpath.remove(0,1);	// by some reason path will start with /
#endif
		//loadFile(comp,fpath.toUtf8().data(),FT_ALL,0);
		load_file(comp, fpath.toLocal8Bit().data(), FG_ALL, 0);
	}
}


void MainWin::closeEvent(QCloseEvent* ev) {
	FILE* file;
	char fname[FILENAME_MAX];
	pause(true,PR_EXIT);
	foreach(xProfile* prf, conf.prof.list) {
		prfSave(prf->name);
		strcpy(fname, conf.path.confDir);
		strcat(fname, SLASH);
		strcat(fname, prf->name.c_str());
		strcat(fname, ".cmos");
		file = fopen(fname, "wb");
		if (file) {
			fwrite((const char*)prf->zx->cmos.data,256,1,file);
			fclose(file);
		}
		if (prf->zx->ide->type == IDE_SMUC) {
			strcpy(fname, conf.path.confDir);
			strcat(fname, SLASH);
			strcat(fname, prf->name.c_str());
			strcat(fname, ".nvram");
			file = fopen(fname, "wb");
			if (file) {
				fwrite((const char*)prf->zx->ide->smuc.nv->mem,0x800,1,file);
				fclose(file);
			}
		}
	}
	if (saveChanged()) {
		killTimer(timid);
		killTimer(secid);
		ideCloseFiles(comp->ide);
		sdcCloseFile(comp->sdc);
		sltEject(comp->slot);		// this must save cartridge ram
		// emutex.unlock();		// unlock emulation thread
		emit s_keywin_close();
		if (conf.joy.joy)
			SDL_JoystickClose(conf.joy.joy);
		saveConfig();
#ifdef USENETWORK
		foreach(QTcpSocket* sock, clients) {
			sock->close();
			sock->deleteLater();
		}
		srv.close();
#endif
		ev->accept();
	} else {
		ev->ignore();
		pause(false,PR_EXIT);
	}
}

void MainWin::checkState() {
#ifdef HAVEZLIB
	if (comp->rzx.start)
		emit s_rzx_start();
		//rzxWin->startPlay();
#endif
	//emit s_tape_list(comp->tape);
	//tapeWin->buildList(comp->tape);
	//tapeWin->setCheck(comp->tape->block);
}

// ...

uchar hobHead[] = {'s','c','r','e','e','n',' ',' ','C',0,0,0,0x1b,0,0x1b,0xe7,0x81};	// last 2 bytes is crc

void MainWin::screenShot() {
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
	QImage img(bufimg, width(), height(), QImage::Format_RGB888);
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
	setMessage(trUtf8("screenshot saved"));
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
// update rzx window
#ifdef HAVEZLIB
	emit s_rzx_upd(comp);
#endif
// update tape window
	if (comp->tape->on && !comp->tape->rec)
		emit s_tape_progress(comp->tape);
	if ((comp->tape->on && !comp->tape->rec) || comp->tape->blkChange || comp->tape->newBlock) {
		emit s_tape_upd(comp->tape);
		if (comp->tape->blkChange) comp->tape->blkChange = 0;
		if (comp->tape->newBlock) comp->tape->newBlock = 0;
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
	resMenu = userMenu->addMenu(QIcon(":/images/shutdown.png"),"Reset...");

	userMenu->addSeparator();
	userMenu->addAction(QIcon(":/images/tape.png"), "Tape player", this, SIGNAL(s_tape_show()));
	userMenu->addAction(QIcon(":/images/video.png"),"RZX player", this, SIGNAL(s_rzx_show()));
	userMenu->addSeparator();
	pckAct = userMenu->addAction(QIcon(":/images/keyboard.png"),"Grab keyboard");
	pckAct->setCheckable(true);
	userMenu->addAction(QIcon(":/images/keyboardzx.png"),"ZX Keyboard",this,SIGNAL(s_keywin_shide()));
	userMenu->addAction(QIcon(":/images/objective.png"),"Watcher", this, SIGNAL(s_watch_show()));
	userMenu->addAction(QIcon(":/images/other.png"),"Options",this,SLOT(doOptions()));

	connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	connect(layoutMenu,SIGNAL(triggered(QAction*)),this,SLOT(chLayout(QAction*)));
	connect(resMenu,SIGNAL(triggered(QAction*)),this,SLOT(reset(QAction*)));
	connect(fileMenu,SIGNAL(triggered(QAction*)),this,SLOT(umOpen(QAction*)));

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

void MainWin::fillBookmarkMenu() {
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
}

void MainWin::fillProfileMenu() {
	profileMenu->clear();
	foreach(xProfile* prf, conf.prof.list) {
		profileMenu->addAction(prf->name.c_str())->setData(prf->name.c_str());
	}
}

void MainWin::fillLayoutMenu() {
	layoutMenu->clear();
	foreach(xLayout lay, conf.layList) {
		layoutMenu->addAction(lay.name.c_str())->setData(lay.name.c_str());
	}
}

void MainWin::fillUserMenu() {
	fillBookmarkMenu();
	fillProfileMenu();
	fillLayoutMenu();
}

// SLOTS

void MainWin::doOptions() {
	pause(true, PR_OPTS);
	emit s_options(conf.prof.cur);
}

void MainWin::optApply() {
	comp = conf.prof.cur->zx;
	fillUserMenu();
	updateWindow();
#ifdef USENETWORK
	if (srv.serverPort() != conf.port) {
		if (srv.isListening()) {
			foreach(QTcpSocket* sock, clients)
				sock->close();
			srv.close();
		}
		srv.listen(QHostAddress::LocalHost, conf.port);
		if (!srv.isListening()) {
			shitHappens("Listen server can't start");
		} else {
			printf("Listening port %i\n", conf.port);
		}
	}
#endif
	pause(false, PR_OPTS);
}

void MainWin::doDebug() {
	conf.emu.fast = 0;
	pause(true, PR_DEBUG);
	emit s_debug(comp);
}

void MainWin::dbgReturn() {
	pause(false, PR_DEBUG);
}

void MainWin::bookmarkSelected(QAction* act) {
	load_file(comp, act->data().toString().toLocal8Bit().data(), FG_ALL, 0);
	setFocus();
}

void MainWin::setProfile(std::string nm) {
	if (nm != "") {
		if (!prfSetCurrent(nm)) {
			prfSetCurrent("default");
		}
	}
	comp = conf.prof.cur->zx;
	emit s_keywin_upd(comp->keyb);
	updateWindow();
	if (comp->firstRun) {
		compReset(comp, RES_DEFAULT);
		comp->firstRun = 0;
	}
	saveConfig();
	emit s_prf_change(conf.prof.cur);
	conf.prof.changed = 1;
}

void MainWin::profileSelected(QAction* act) {
	std::string str = QString(act->data().toByteArray()).toStdString();
	setProfile(str);
}

void MainWin::reset(QAction* act) {
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
	load_file(comp, NULL, act->data().toInt(), -1);
}

// socket

void MainWin::connected() {
#ifdef USENETWORK
	QTcpSocket* sock = srv.nextPendingConnection();
	clients.append(sock);
	sock->write("hello\n");
	connect(sock,SIGNAL(destroyed()),this,SLOT(disconnected()));
	connect(sock,SIGNAL(readyRead()),this,SLOT(socketRead()));
#endif
}

void MainWin::disconnected() {
#ifdef USENETWORK
	QTcpSocket* sock = (QTcpSocket*)sender();
	disconnect(sock);
	clients.removeAll(sock);
	sock->deleteLater();
#endif
}

void MainWin::socketRead() {
#ifdef USENETWORK
	QTcpSocket* sock = (QTcpSocket*)sender();
	QByteArray arr = sock->readAll();
	// and do something with this
#endif
}

// debug stufffff

void MainWin::saveVRAM() {
	QString path = QFileDialog::getSaveFileName(this,"Save VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->ram, 0x20000);
		file.write((char*)comp->vid->reg, 64);
		for (int i = 0; i < 16; i++) {
			file.putChar(comp->vid->pal[i].r);
			file.putChar(comp->vid->pal[i].g);
			file.putChar(comp->vid->pal[i].b);
		}

		file.close();
	}
}

void MainWin::saveGBVRAM() {
	QString path = QFileDialog::getSaveFileName(this,"Save GB VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
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
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->ram, 0x4000);
		file.write((char*)comp->vid->oam, 0x100);
		file.close();
	}
}

void MainWin::debugAction() {
	sndDebug();
}
