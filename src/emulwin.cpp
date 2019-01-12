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
	// sndCalibrate(comp);				// why?
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
						//yep &= saveFile(comp, flp->path, FT_DISK, i);
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

//	QFile file(":/font.bin");		// on-screen messages bitmap font 12x12
//	file.open(QFile::ReadOnly);
//	font = file.readAll();
//	file.close();
	msgTimer = 0;
	msg.clear();
	alphabet.load(":/font.png");

	initKeyMap();
	conf.scrShot.format = "png";
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	addLayout("default", vlay);

	shotFormat["bmp"] = SCR_BMP;
	shotFormat["png"] = SCR_PNG;
	shotFormat["jpg"] = SCR_JPG;
	shotFormat["scr"] = SCR_SCR;
	shotFormat["hobeta"] = SCR_HOB;

//	opt = new SetupWin(this);
//	dbg = new DebugWin(this);
//	watcher = new xWatcher(this);

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

	timer.setInterval(20);
	connect(&timer,SIGNAL(timeout()),this,SLOT(onTimer()));

	connect(userMenu,SIGNAL(aboutToShow()),SLOT(menuShow()));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));

	fillUserMenu();

	timer.start();
}

// gamepad mapper

void MainWin::mapRelease(Computer* comp, xJoyMapEntry ent) {
	QKeyEvent ev(QEvent::KeyRelease, key2qid(ent.key), Qt::NoModifier);
	switch(ent.dev) {
		case JMAP_KEY:
			keyReleaseEvent(&ev);
			// keyRelease(comp->keyb, kent.zxKey, 0);
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
	QKeyEvent ev(QEvent::KeyPress, key2qid(ent.key), Qt::NoModifier);
	switch(ent.dev) {
		case JMAP_KEY:
			keyPressEvent(&ev);
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

void MainWin::mapJoystick(Computer* comp, int type, int num, int state) {
	// printf("map %i %i %i\n",type, num, state);
	foreach(xJoyMapEntry xjm, conf.joy.map) {
		if ((type == xjm.type) && (num == xjm.num)) {
			if (state == 0) {
				mapRelease(comp, xjm);
			} else {
				switch(type) {
					case JOY_AXIS:
						if (sign(state) == sign(xjm.state)) {
							xjm.state = state;
							mapPress(comp, xjm);
						}
						break;
					case JOY_HAT:
						if (state & xjm.state)
							mapPress(comp, xjm);
						else
							mapRelease(comp, xjm);
						break;
					case JOY_BUTTON:
						mapPress(comp, xjm);
						break;
				}
			}
		}
	}
}

// calling on timer every 20ms

static QList<int> fpsmem;

void MainWin::onTimer() {
//	if (opt->block) return;
	if (conf.prof.changed) {
		comp = conf.prof.cur->zx;
		conf.prof.changed = 0;
	}
	if (block) return;
#if HAVEZLIB
	if (comp->rzx.start) {
		emit s_rzx_start();
		// rzxWin->startPlay();
	}
	if (comp->rzx.overio) {
		comp->rzx.overio = 0;
		pause(true, PR_RZX);
		shitHappens("RZX playback error");
		emit s_rzx_stop();
		//rzxWin->stop();
		pause(false, PR_RZX);
	}
#endif
// process sdl event (gamepad)

/* TODO : rewrite using this:
	SDL_JoystickGetAxis(joy, 0);
	SDL_JoystickGetButton(joy, 0);
	SDL_JoystickGetHat(joy, 0);
*/
	if (conf.joy.joy && !conf.emu.pause) {
		SDL_Event ev;
		SDL_JoystickUpdate();
		while(SDL_PollEvent(&ev)) {
			// printf("%i\n", ev.type);
			switch(ev.type) {
				case SDL_JOYAXISMOTION:
					if (abs(ev.jaxis.value) < conf.joy.dead)
						ev.jaxis.value = 0;
					mapJoystick(comp, JOY_AXIS, ev.jaxis.axis, ev.jaxis.value);
					//printf("Axis %i %i\n", ev.jaxis.axis, ev.jaxis.value);
					break;
				case SDL_JOYBUTTONDOWN:
					mapJoystick(comp, JOY_BUTTON, ev.jbutton.button, 32768);
					//printf("Button %i down\n", ev.jbutton.button);
					break;
				case SDL_JOYBUTTONUP:
					mapJoystick(comp, JOY_BUTTON, ev.jbutton.button, 0);
					//printf("Button %i up\n", ev.jbutton.button);
					break;
				case SDL_JOYHATMOTION:
					mapJoystick(comp, JOY_HAT, ev.jhat.hat, ev.jhat.value);
					// printf("hat %i %i\n", ev.jhat.hat, ev.jhat.value);
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
// fps counter
	if (!conf.emu.pause) {
		fpsmem.append(conf.vid.fcount);
		while (fpsmem.size() > 50)
			fpsmem.removeFirst();
	}
	if (!fpsmem.isEmpty())
		conf.vid.curfps = conf.vid.fcount - fpsmem.first();
	else
		conf.vid.curfps = 0;
// update satellites
	updateSatellites();
// redraw window (if fast)
	if (conf.emu.fast || comp->debug) {
		setUpdatesEnabled(true);
		repaint();
		setUpdatesEnabled(false);
	}
}

// if window is not active release keys & buttons, release mouse
void MainWin::focusOutEvent(QFocusEvent* ev) {
	kbdReleaseAll(comp->keyb);
	mouseReleaseAll(comp->mouse);
	unsetCursor();
	if (grabMice) {
		grabMice = 0;
		releaseMouse();
	}
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

void MainWin::keyPressEvent(QKeyEvent *ev) {
	if (comp->debug) {
		ev->ignore();
		return;
	}
	int keyid = qKey2id(ev->key());
	keyEntry kent = getKeyEntry(keyid);
	if (pckAct->isChecked()) {
		if (comp->hw->keyp) {
			comp->hw->keyp(comp, kent);
		}
		if (keyid == XKEY_F12) {
			compReset(comp,RES_DEFAULT);
			emit s_rzx_stop();
			// rzxWin->stop();
		}
	} else if (ev->modifiers() & Qt::AltModifier) {
		switch(keyid) {
			case XKEY_ENTER:
				vid_set_fullscreen(!conf.vid.fullScreen);
				setMessage(conf.vid.fullScreen ? " fullscreen on " : " fullscreen off ");
				updateWindow();
				saveConfig();
				break;
			case XKEY_HOME:
				debugAction();
				break;
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
		switch(keyid) {
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
				if (comp->hw->keyp) {
					comp->hw->keyp(comp, kent);
				}
				break;
		}
	}
	emit s_keywin_upd(comp->keyb);
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	if (comp->debug) {
		ev->ignore();
		return;
	}
	int keyid = qKey2id(ev->key());
	keyEntry kent = getKeyEntry(keyid);
	if (comp->hw->keyr)
		comp->hw->keyr(comp, kent);
	emit s_keywin_upd(comp->keyb);
	//if (keywin->isVisible()) keywin->update();
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
	if (grabMice && comp->mouse->hasWheel) {
		mousePress(comp->mouse, (ev->delta() < 0) ? XM_WHEELDN : XM_WHEELUP, 0);
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
		timer.stop();
		ideCloseFiles(comp->ide);
		sdcCloseFile(comp->sdc);
		sltEject(comp->slot);		// this must save cartridge ram
		// emutex.unlock();		// unlock emulation thread
		emit s_keywin_close();
		if (conf.joy.joy)
			SDL_JoystickClose(conf.joy.joy);
		saveConfig();
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

/*
void MainWin::putLeds() {
	if (!comp) return;
	QPainter pnt;
	QImage kled(":/images/scanled.png");
	if (conf.led.keys) {
		pnt.begin(&kled);
		unsigned char prt = ~comp->keyb->port;
		comp->keyb->port = 0xff;
		if (prt & 0x01) pnt.fillRect(3,17,8,2,Qt::white);
		if (prt & 0x02) pnt.fillRect(3,14,8,2,Qt::white);
		if (prt & 0x04) pnt.fillRect(3,11,8,2,Qt::white);
		if (prt & 0x08) pnt.fillRect(3,8,8,2,Qt::white);
		if (prt & 0x10) pnt.fillRect(12,8,8,2,Qt::white);
		if (prt & 0x20) pnt.fillRect(12,11,8,2,Qt::white);
		if (prt & 0x40) pnt.fillRect(12,14,8,2,Qt::white);
		if (prt & 0x80) pnt.fillRect(12,17,8,2,Qt::white);
		pnt.end();
	}
	if (comp->joy->used && conf.led.joy) {
		addLed(3, 30, ":/images/joystick.png", 50);
		comp->joy->used = 0;
	}
	if (comp->mouse->used && conf.led.mouse) {
		addLed(3, 50, ":/images/mouse.png", 50);
		comp->mouse->used = 0;
	}
	if (comp->tape->on && conf.led.tape) {
		if (comp->tape->rec) {
			addLed(3, 70, ":/images/tapeRed.png", 50);
		} else {
			addLed(3, 70, ":/images/tapeYellow.png", 50);
		}
	}
	if (conf.led.disk) {
		if (comp->dif->fdc->flp->rd) {
			comp->dif->fdc->flp->rd = 0;
			addLed(3, 90, ":/images/diskGreen.png", 50);
		} else if (comp->dif->fdc->flp->wr) {
			comp->dif->fdc->flp->wr = 0;
			addLed(3, 90, ":/images/diskRed.png", 50);
		}
	}
	pnt.begin(&scrImg);
	drawLeds(pnt);
	if (conf.led.keys) pnt.drawImage(3,3,kled);
	pnt.end();
}
*/

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
	// opt->start(conf.prof.cur);
}

void MainWin::optApply() {
	comp = conf.prof.cur->zx;
	fillUserMenu();
	updateWindow();
	pause(false, PR_OPTS);
}

void MainWin::doDebug() {
	conf.emu.fast = 0;
	pause(true, PR_DEBUG);
	emit s_debug(comp);
	// dbg->start(comp);
}

void MainWin::dbgReturn() {
	pause(false, PR_DEBUG);
}

void MainWin::bookmarkSelected(QAction* act) {
//	loadFile(comp,act->data().toString().toLocal8Bit().data(),FT_ALL,0);
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
	conf.prof.changed = 1;
}

void MainWin::profileSelected(QAction* act) {
	std::string str = QString(act->data().toByteArray()).toStdString();
	setProfile(str);
}

void MainWin::reset(QAction* act) {
	//rzxWin->stop();
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
//	loadFile(comp, NULL, act->data().toInt(), -1);
	load_file(comp, NULL, act->data().toInt(), -1);
}

// labels

/*
void MainWin::loadLabels(const char* nm) {
	emit s_labels(QString(nm));
	// dbg->loadLabels(QString(nm));
}
*/

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
