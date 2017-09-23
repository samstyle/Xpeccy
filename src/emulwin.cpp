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

unsigned char screen[4096 * 2048 * 3];		// scaled image (up to fullscreen)
unsigned char scrn[1024 * 512 * 3];		// 2:1 image
unsigned char sbufa[1024 * 512 * 3];		// Emulation render -> (this)buffer -> scrn for postprocessing
unsigned char prvScr[1024 * 512 * 3];		// copy of last 2:1 image (for noflic)
extern int bytesPerLine;

// temp emulation
unsigned short pc,af,de,ix;

// mainwin

void MainWin::updateHead() {
	QString title(XPTITLE);
#ifdef ISDEBUG
	title.append(" | debug");
#endif
	if (conf.prof.cur != NULL) {
		title.append(" | ").append(QString::fromLocal8Bit(conf.prof.cur->name.c_str()));
		title.append(" | ").append(QString::fromLocal8Bit(conf.prof.cur->layName.c_str()));
	}
	if (ethread.fast) {
		title.append(" | fast");
	}
	setWindowTitle(title);
}

void MainWin::updateWindow() {
	block = 1;
	vidSetBorder(comp->vid, conf.brdsize);
	sndCalibrate(comp);
	ethread.sndNs = 0;
	int szw = comp->vid->vsze.x * conf.vid.scale;
	int szh = comp->vid->vsze.y * conf.vid.scale;
	if (conf.vid.fullScreen) {
		QSize wsz = QApplication::desktop()->screenGeometry().size();
		szw = wsz.width();
		szh = wsz.height();
		setWindowState(windowState() | Qt::WindowFullScreen);
//		grabMice = 1;
//		grabMouse();
	} else {
		setWindowState(windowState() & ~Qt::WindowFullScreen);
//		grabMice = 0;
//		releaseMouse();
	}
//	if (grabMice || conf.vid.fullScreen) {
//		setCursor(Qt::BlankCursor);
//	} else {
//		unsetCursor();
//	}
	setFixedSize(szw, szh);
	lineBytes = szw * 3;
	frameBytes = szh * lineBytes;
	scrImg = QImage(screen, szw, szh, QImage::Format_RGB888);
	bytesPerLine = scrImg.bytesPerLine();
	updateHead();
	block = 0;
	if (dbg->isVisible()) dbg->fillAll();		// hmmm... why?
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
						yep &= saveFile(comp,flp->path,FT_DISK,i);
						break;
					case QMessageBox::Cancel:
						yep = false;
						break;
				}
			}
		}
		if (!yep) break;
	}
/*
	bool yep = saveChangedDisk(comp,0);
	yep &= saveChangedDisk(comp,1);
	yep &= saveChangedDisk(comp,2);
	yep &= saveChangedDisk(comp,3);
*/
	return yep;
}

void MainWin::pause(bool p, int msk) {
	ethread.fast = 0;
	if (p) {
		pauseFlags |= msk;
	} else {
		pauseFlags &= ~msk;
	}
	if (!grabMice || ((pauseFlags != 0) && grabMice)) {
		releaseMouse();
	}
	if (pauseFlags == 0) {
		setWindowIcon(icon);
		if (grabMice) grabMouse(QCursor(Qt::BlankCursor));
		ethread.block = 0;
	} else {
		setWindowIcon(QIcon(":/images/pause.png"));
		ethread.block = 1;
	}
	updateHead();
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
	pauseFlags = 0;
	scrCounter = 0;
	scrInterval = 0;
	grabMice = 0;
	block = 0;

	QFile file(":/font.bin");		// on-screen messages bitmap font 12x12
	file.open(QFile::ReadOnly);
	font = file.readAll();
	file.close();
	msgTimer = 0;
	msg.clear();

	initKeyMap();
	conf.scrShot.format = "png";
	vLayout vlay = {{448,320},{74,48},{64,32},{256,192},{0,0},64};
	addLayout("default", vlay);

	shotFormat["bmp"] = SCR_BMP;
	shotFormat["png"] = SCR_PNG;
	shotFormat["jpg"] = SCR_JPG;
	shotFormat["scr"] = SCR_SCR;
	shotFormat["hobeta"] = SCR_HOB;

	opt = new SetupWin(this);
	dbg = new DebugWin(this);
	watcher = new xWatcher(this);

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
	connect(opt,SIGNAL(closed()),this,SLOT(optApply()));
	connect(dbg,SIGNAL(closed()),this,SLOT(dbgReturn()));

	tapeWin = new TapeWin(this);
	connect(tapeWin,SIGNAL(stateChanged(int,int)),this,SLOT(tapStateChanged(int,int)));

	rzxWin = new RZXWin(this);
	connect(rzxWin,SIGNAL(stateChanged(int)),this,SLOT(rzxStateChanged(int)));

	keywin = new keyWindow();
	QPixmap pxm(":/images/keymap.png");
	keywin->setPixmap(pxm);
	keywin->setFixedSize(pxm.size());
	keywin->setWindowIcon(QIcon(":/images/keyboard.png"));
	keywin->setWindowTitle("ZX Keyboard");

	initUserMenu();

	timer.setInterval(20);
	connect(&timer,SIGNAL(timeout()),this,SLOT(onTimer()));

	ethread.conf = &conf;
	connect(&ethread,SIGNAL(dbgRequest()),SLOT(doDebug()));
	connect(&ethread,SIGNAL(tapeSignal(int,int)),this,SLOT(tapStateChanged(int,int)));
	connect(&ethread,SIGNAL(picReady()),this,SLOT(convImage()));
	ethread.start();

	scrImg = QImage(100,100,QImage::Format_RGB888);
	connect(userMenu,SIGNAL(aboutToShow()),SLOT(menuShow()));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));

	loadConfig();
	fillUserMenu();

	timer.start();
}

// scale screen

void processPicture(unsigned char*, int);

void MainWin::convImage() {

	if (!pauseFlags || comp->debug) {
		if (ethread.fast || comp->debug) {
			memcpy(scrn, comp->vid->scrimg, comp->vid->vBytes);
		} else {
			memcpy(scrn, sbufa, comp->vid->vBytes);
			// processPicture(comp->vid->scrimg, comp->vid->vBytes);
		}
		// scrMix(prvScr, scrn, comp->vid->vBytes, conf.vid.noFlick ? 0.5 : 0.7);
		if (conf.vid.grayScale)
			scrGray(scrn, comp->vid->vBytes);
	}

	if (conf.vid.fullScreen) {
		QSize dstsize = QApplication::desktop()->screenGeometry().size();
		scrFS(scrn, comp->vid->vsze.x, comp->vid->vsze.y, screen, dstsize.width(), dstsize.height());
	} else {
		switch(conf.vid.scale) {
			case 1:	scrX1(scrn, comp->vid->vsze.x, comp->vid->vsze.y, screen);
				break;
			case 2:	scrX2(scrn, comp->vid->vsze.x, comp->vid->vsze.y, screen);
				break;
			case 3:	scrX3(scrn, comp->vid->vsze.x, comp->vid->vsze.y, screen);
				break;
			case 4: scrX4(scrn, comp->vid->vsze.x, comp->vid->vsze.y, screen);
				break;
		}
	}
	ethread.waitpic = 1;
// [put leds],make screenshot,[put leds]
	if (!conf.scrShot.noLeds) putLeds();		// put leds before screenshot if on
	if (scrCounter > 0) {
		if (scrInterval > 0) {
			scrInterval--;
		} else {
			screenShot();
			scrCounter--;
			scrInterval = conf.scrShot.interval;
		}
	}
	if (conf.scrShot.noLeds) putLeds();		// put leds if it isn't done yet
// put messages
	if (msgTimer > 0) {
		if (conf.led.message)
			drawMessage();
		msgTimer--;
	}
	setUpdatesEnabled(true);
	repaint();
	setUpdatesEnabled(false);
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
						if (state == xjm.state)
							mapPress(comp, xjm);
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

void MainWin::onTimer() {
	if (opt->block) return;
	if (opt->prfChanged) {
		comp = conf.prof.cur->zx;
		ethread.comp = comp;
		opt->prfChanged = 0;
	}
	if (block) return;
	if (comp->rzx.start) {
		rzxWin->startPlay();
	}
	if (comp->rzx.overio) {
		comp->rzx.overio = 0;
		pause(true, PR_RZX);
		shitHappens("RZX playback error");
		rzxWin->stop();
		pause(false, PR_RZX);
	}
// process sdl event (gamepad)

/* TODO : rewrite using this:
	SDL_JoystickGetAxis(joy, 0);
	SDL_JoystickGetButton(joy, 0);
	SDL_JoystickGetHat(joy, 0);
*/
	if (conf.joy.joy && !pauseFlags) {
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
// if window is not active release keys & buttons, release mouse
	if (!isActiveWindow()) {
		if (!keywin->isVisible())
			keyReleaseAll(comp->keyb);
		mouseReleaseAll(comp->mouse);
		unsetCursor();
		if (grabMice) {
			grabMice = 0;
			releaseMouse();
		}
	}
// update satellites
	updateSatellites();
// fill fake buffer if paused
	if (pauseFlags) {
		conf.snd.fill = 1;
		do {
			sndSync(comp, 1, 1);
		} while (conf.snd.fill);
		convImage();
	} else if (ethread.fast) {
		convImage();
	}

	emutex.unlock();
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
						tapeWin->setState(TWS_PLAY);
					} else {
						tapeWin->setState(TWS_STOP);
					}
					break;
				case TWS_STOP:
					tapStop(comp->tape);
					tapeWin->setState(TWS_STOP);
					break;
				case TWS_REC:
					tapRec(comp->tape);
					tapeWin->setState(TWS_REC);
					break;
				case TWS_OPEN:
					pause(true,PR_FILE);
					loadFile(comp,"",FT_TAPE,-1);
					tapeWin->buildList(comp->tape);
					tapeWin->setCheck(comp->tape->block);
					pause(false,PR_FILE);
					break;
			}
			break;
		case TW_REWIND:
			tapRewind(comp->tape,val);
			break;
		case TW_BREAK:
			comp->tape->blkData[val].breakPoint ^= 1;
			tapeWin->drawStops(comp->tape);
			break;
	}
}

// connection between rzx player and emulation state
void MainWin::rzxStateChanged(int state) {
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
			loadFile(comp,"",FT_RZX,0);
			if (comp->rzx.play) {
				rzxWin->startPlay();
			}
			pause(false,PR_RZX);
			break;
	}
}

void MainWin::paintEvent(QPaintEvent*) {
	if (block) return;
	QPainter pnt;
	pnt.begin(this);
	pnt.drawImage(0,0,scrImg);
	pnt.end();
}

void MainWin::keyPressEvent(QKeyEvent *ev) {
	// if (ev->isAutoRepeat()) return;
	int keyid = qKey2id(ev->key());
	keyEntry kent = getKeyEntry(keyid);
	if (pckAct->isChecked()) {
		if (comp->hw->keyp) {
			comp->hw->keyp(comp, kent);
		}
/*
		keyPressXT(comp->keyb, kent.keyCode);
		if (!kent.zxKey.key2)			// don't press 2-key keys in PC-mode
			keyPress(comp->keyb,kent.zxKey,0);
		keyPress(comp->keyb, kent.extKey, 1);
		keyPress(comp->keyb, kent.msxKey, 2);
*/
		if (ev->key() == Qt::Key_ScrollLock) {
			compReset(comp,RES_DEFAULT);
			rzxWin->stop();
		}
	} else if (ev->modifiers() & Qt::AltModifier) {
		switch(ev->key()) {
			case Qt::Key_Return:
				conf.vid.fullScreen ^= 1;
				setMessage(conf.vid.fullScreen ? " fullscreen on " : " fullscreen off ");
				updateWindow();
				saveConfig();
				break;
			case Qt::Key_Home:
				debugAction();
				break;
			case Qt::Key_1:
				conf.vid.scale = 1;
				updateWindow();
				convImage();
				saveConfig();
				setMessage(" size x1 ");
				break;
			case Qt::Key_2:
				conf.vid.scale = 2;
				updateWindow();
				convImage();
				saveConfig();
				setMessage(" size x2 ");
				break;
			case Qt::Key_3:
				conf.vid.scale = 3;
				updateWindow();
				convImage();
				saveConfig();
				setMessage(" size x3 ");
				break;
			case Qt::Key_4:
				conf.vid.scale = 4;
				updateWindow();
				convImage();
				saveConfig();
				setMessage(" size x4 ");
				break;
			case Qt::Key_F4:
				close();
				break;
			case Qt::Key_F7:
				scrCounter = conf.scrShot.count;
				scrInterval = 0;
				break;	// ALT+F7 combo
			case Qt::Key_F12:
				compReset(comp,RES_DOS);
				rzxWin->stop();
				break;
			case Qt::Key_K:
				if (keywin->isVisible()) {
					keywin->close();
				} else {
					keywin->show();
					// activateWindow();
					// raise();
				}
				break;
			case Qt::Key_N:
				if (conf.vid.noflic < 15)
					conf.vid.noflic = 25;
				else if (conf.vid.noflic < 35)
					conf.vid.noflic = 50;
				else	conf.vid.noflic = 0;
				saveConfig();
				memcpy(prvScr, scrn, comp->vid->frmsz * 6);
				setMessage(QString(" noflick %0% ").arg(conf.vid.noflic * 2));
				break;
			case Qt::Key_R:
				conf.vid.keepRatio ^= 1;
				setMessage(conf.vid.keepRatio ? " keep aspect ratio " : " ignore aspect ratio ");
				saveConfig();
				break;
			case Qt::Key_M:
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
//		if (comp->hw->id == HW_GBC)
//			gbPress(comp, kent.name);
//		keyPress(comp->keyb, kent.zxKey, 0);
//		if (kent.msxKey.key1) keyPress(comp->keyb,kent.msxKey,2);
		switch(ev->key()) {
			case Qt::Key_Pause:
				pauseFlags ^= PR_PAUSE;
				pause(true,0);
				break;
			case Qt::Key_Escape:
				ethread.fast = 0;
				pause(true, PR_DEBUG);
				setUpdatesEnabled(true);
				dbg->start(comp);
				break;
			case Qt::Key_Menu:
				userMenu->popup(pos() + QPoint(20,20));
				userMenu->setFocus();
				break;
			case Qt::Key_Insert:
				if (pauseFlags) break;
				ethread.fast ^= 1;
				updateHead();
				break;
			case Qt::Key_F1:
				pause(true, PR_OPTS);
				opt->start(conf.prof.cur);
				break;
			case Qt::Key_F2:
				pause(true,PR_FILE);
				saveFile(comp,"",FT_ALL,-1);
				pause(false,PR_FILE);
				break;
			case Qt::Key_F3:
				pause(true,PR_FILE);
				loadFile(comp,"",FT_ALL,-1);
				pause(false,PR_FILE);
				checkState();
				break;
			case Qt::Key_F4:
				if (comp->tape->on) {
					tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					tapStateChanged(TW_STATE,TWS_PLAY);
				}
				break;
			case Qt::Key_F5:
				if (comp->tape->on) {
					tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					tapStateChanged(TW_STATE,TWS_REC);
				}
				break;
			case Qt::Key_F7:
				if (scrCounter == 0) {
					scrCounter = 1;
					scrInterval = 0;
				} else {
					scrCounter = 0;
				}
				break;
			case Qt::Key_F8:
				if (rzxWin->isVisible()) {
					rzxWin->hide();
				} else {
					rzxWin->show();
				}
				break;
			case Qt::Key_F9:
				pause(true,PR_FILE);
				saveChanged();
				pause(false,PR_FILE);
				break;
			case Qt::Key_F10:
				if (comp->cpu->type != CPU_Z80) break;
				comp->nmiRequest = 1;
				break;
			case Qt::Key_F11:
				if (tapeWin->isVisible()) {
					tapeWin->hide();
				} else {
					tapeWin->buildList(comp->tape);
					tapeWin->show();
				}
				break;
			case Qt::Key_F12:
				compReset(comp,RES_DEFAULT);
				rzxWin->stop();
				break;
			default:
				if (comp->hw->keyp) {
					comp->hw->keyp(comp, kent);
				}
				//keyPressXT(comp->keyb, kent.keyCode);
				break;
		}
	}
	if (keywin->isVisible()) keywin->update();
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	int keyid = qKey2id(ev->key());
	keyEntry kent = getKeyEntry(keyid);
	if (comp->hw->keyr)
		comp->hw->keyr(comp, kent);
/*
	gbRelease(comp, kent.name);
	if (pckAct->isChecked()) {
		keyReleaseXT(comp->keyb, kent.keyCode);
		if (!kent.zxKey.key2)
			keyRelease(comp->keyb, kent.zxKey, 0);
		keyRelease(comp->keyb, kent.extKey, 1);
		keyRelease(comp->keyb, kent.msxKey, 2);
	} else {
		keyRelease(comp->keyb, kent.zxKey, 0);
		if (kent.msxKey.key1) keyRelease(comp->keyb,kent.msxKey,2);
	}
*/
	if (keywin->isVisible()) keywin->update();
}

void MainWin::mousePressEvent(QMouseEvent *ev){
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
	if (pauseFlags != 0) return;
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
	if (grabMice && comp->mouse->hasWheel) {
		mousePress(comp->mouse, (ev->delta() < 0) ? XM_WHEELDN : XM_WHEELUP, 0);
	}
}

void MainWin::mouseMoveEvent(QMouseEvent *ev) {
	if (!grabMice || (pauseFlags !=0 )) return;
	comp->mouse->xpos = ev->globalX() & 0xff;
	comp->mouse->ypos = 256 - (ev->globalY() & 0xff);
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
		loadFile(comp,fpath.toUtf8().data(),FT_ALL,0);
	}
}


void MainWin::closeEvent(QCloseEvent* ev) {
	std::ofstream file;
	std::string fname;
	pause(true,PR_EXIT);
	foreach(xProfile* prf, conf.prof.list) {
		prfSave(prf->name);
		fname = conf.path.confDir + SLASH + prf->name + ".cmos";
		file.open(fname.c_str());
		if (file.good()) {
			file.write((const char*)prf->zx->cmos.data,256);
			file.close();
		}
		if (prf->zx->ide->type == IDE_SMUC) {
			fname = conf.path.confDir + SLASH + prf->name + ".nvram";
			file.open(fname.c_str());
			if (file.good()) {
				file.write((const char*)prf->zx->ide->smuc.nv->mem,0x800);
				file.close();
			}
		}
	}
	if (saveChanged()) {
		timer.stop();
		ideCloseFiles(comp->ide);
		sdcCloseFile(comp->sdc);
		sltEject(comp->slot);		// this must save cartridge ram
		ethread.finish = 1;
		emutex.unlock();		// unlock emulation thread
		ethread.wait();			// wait until it exits
		keywin->close();
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
	if (comp->rzx.start) rzxWin->startPlay();
	tapeWin->buildList(comp->tape);
	tapeWin->setCheck(comp->tape->block);
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
	QImage img(screen, width(), height(), QImage::Format_RGB888);
	int x,y,dx,dy;
	char pageBuf[0x4000];
	memGetPageData(comp->mem,MEM_RAM,comp->vid->curscr,pageBuf);
	switch (frm) {
		case SCR_HOB:
			file.open(fnam.c_str(),std::ios::binary);
			file.write((char*)hobHead,17);
			file.write(pageBuf,0x1b00);
			file.close();
			break;
		case SCR_SCR:
			file.open(fnam.c_str(),std::ios::binary);
			file.write(pageBuf,0x1b00);
			file.close();
			break;
		case SCR_BMP:
		case SCR_JPG:
		case SCR_PNG:
			if (img.isNull()) break;
			if (conf.scrShot.noBorder) {
				x = (comp->vid->lay.bord.x - comp->vid->lcut.x) * conf.vid.scale;
				y = (comp->vid->lay.bord.y - comp->vid->lcut.y) * conf.vid.scale;
				dx = comp->vid->lay.scr.x * conf.vid.scale;
				dy = comp->vid->lay.scr.y * conf.vid.scale;
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

void MainWin::putLeds() {
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
		addLed(3, 70, ":/images/tape.png", 50);
	}
	if (conf.led.disk) {
		if (comp->dif->fdc->flp->rd) {
			comp->dif->fdc->flp->rd = 0;
			addLed(3, 90, ":/images/floppy.png", 50);
		} else if (comp->dif->fdc->flp->wr) {
			comp->dif->fdc->flp->wr = 0;
			addLed(3, 90, ":/images/floppyRed.png", 50);
		}
	}
	pnt.begin(&scrImg);
	drawLeds(pnt);
	if (conf.led.keys) pnt.drawImage(3,3,kled);
	pnt.end();
}

void MainWin::setMessage(QString str, double dur) {
	msgTimer = dur * 50;
	msg = str;
}

#define CHSIZE (12*12*3)

void drawChar(QByteArray chr, int scradr) {
	int x, y;
	int r,g,b;
	int adr = 0;
	int vadr;
	for (y = 0; y < 12; y++) {
		for (x = 0; x < 12*3; x+=3) {
			r = chr.at(adr++);
			g = chr.at(adr++);
			b = chr.at(adr++);
			vadr = scradr + x;
			if (r || g || b) {
				screen[vadr++] = r;
				screen[vadr++] = g;
				screen[vadr] = b;
			} else {
				screen[vadr++] >>= 2;
				screen[vadr++] >>= 2;
				screen[vadr] >>= 2;
			}
		}
		scradr += bytesPerLine;
	}
}

void MainWin::drawMessage() {
	int wid = size().width();
	int x = 5;
	int y = size().height() - 17;
	int scradr = y * bytesPerLine + x * 3;
	int chr;
	QByteArray chrdata;
	for (int i = 0; i < msg.size(); i++) {
		chr = msg.at(i).unicode() - 32;
		if (chr > 95)
			chr = '_' - 32;
		chrdata = font.mid(chr * CHSIZE, CHSIZE);
		drawChar(chrdata, scradr + x * 3);
		x += 12;
		if (x >= (wid - 12)) break;
	}
}

void MainWin::updateSatellites() {
	if (block) return;
// update rzx window
	if (comp->rzx.play && rzxWin->isVisible()) {
		rzxWin->setProgress(comp->rzx.fCurrent, comp->rzx.fTotal);
	}
// update tape window
	if (tapeWin->isVisible()) {
		if (comp->tape->on && !comp->tape->rec) {
			tapeWin->setProgress(tapGetBlockTime(comp->tape,comp->tape->block,comp->tape->pos),tapGetBlockTime(comp->tape,comp->tape->block,-1));
		}
		if (comp->tape->blkChange) {
			if (!comp->tape->on) {
				tapStateChanged(TW_STATE,TWS_STOP);
			}
			tapeWin->setCheck(comp->tape->block);
			comp->tape->blkChange = 0;
		}
		if (comp->tape->newBlock) {
			tapeWin->buildList(comp->tape);
			comp->tape->newBlock = 0;
		}
	}
// update watcher
	if (watcher->isVisible()) {
		watcher->fillFields(comp);
	}
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
	userMenu->addAction(QIcon(":/images/tape.png"),"Tape player",tapeWin,SLOT(show()));
	userMenu->addAction(QIcon(":/images/video.png"),"RZX player",rzxWin,SLOT(show()));
	userMenu->addSeparator();
	pckAct = userMenu->addAction(QIcon(":/images/keyboard.png"),"PC keyboard");
	pckAct->setCheckable(true);
	userMenu->addAction(QIcon(":/images/keyboardzx.png"),"ZX Keyboard",keywin,SLOT(show()));
	userMenu->addAction(QIcon(":/images/objective.png"),"Watcher",watcher,SLOT(show()));
	userMenu->addAction(QIcon(":/images/other.png"),"Options",this,SLOT(doOptions()));

	connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	connect(layoutMenu,SIGNAL(triggered(QAction*)),this,SLOT(chLayout(QAction*)));
	connect(resMenu,SIGNAL(triggered(QAction*)),this,SLOT(reset(QAction*)));
	connect(fileMenu,SIGNAL(triggered(QAction*)),this,SLOT(umOpen(QAction*)));

	fileMenu->addAction(QIcon(":/images/memory.png"),"Snapshot")->setData(FT_SNAP | FT_SPG);
	fileMenu->addAction(QIcon(":/images/tape.png"),"Tape")->setData(FT_TAPE);
	fileMenu->addAction(QIcon(":/images/floppy.png"),"Floppy")->setData(FT_DISK);

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
	opt->start(conf.prof.cur);
}

void MainWin::optApply() {
	comp = conf.prof.cur->zx;
	fillUserMenu();
	updateWindow();
	pause(false, PR_OPTS);
}

void MainWin::doDebug() {
	ethread.fast = 0;
	pause(true, PR_DEBUG);
	dbg->start(comp);
}

void MainWin::dbgReturn() {
	pause(false, PR_DEBUG);
}

void MainWin::bookmarkSelected(QAction* act) {
	loadFile(comp,act->data().toString().toLocal8Bit().data(),FT_ALL,0);
	setFocus();
}

void MainWin::setProfile(std::string nm) {
	ethread.block = 1;
	if (nm != "") {
		if (!prfSetCurrent(nm)) {
			prfSetCurrent("default");
		}
	}
	comp = conf.prof.cur->zx;
	ethread.comp = comp;
	keywin->kb = comp->keyb;
//	nsAct->setChecked(comp->vid->noScreen);
	updateWindow();
	if (comp->firstRun) {
		compReset(comp, RES_DEFAULT);
		comp->firstRun = 0;
	}
	saveConfig();
//	timer.setInterval(1000 / comp->vid->fps);
	opt->prfChanged = 1;
	ethread.block = 0;
}

void MainWin::profileSelected(QAction* act) {
	std::string str = QString(act->data().toByteArray()).toStdString();
	setProfile(str);
}

void MainWin::reset(QAction* act) {
	rzxWin->stop();
	compReset(comp,act->data().toInt());
}

void MainWin::chLayout(QAction* act) {
	std::string str = QString(act->data().toByteArray()).toStdString();
	prfSetLayout(NULL, str);
	prfSave("");
	updateWindow();
}

void MainWin::umOpen(QAction* act) {
	loadFile(comp, NULL, act->data().toInt(), -1);
}

// labels

void MainWin::loadLabels(const char* nm) {
	dbg->loadLabels(QString(nm));
}

// debug stufffff
void MainWin::saveVRAM() {
	QString path = QFileDialog::getSaveFileName(this,"Save VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->v9938.ram, 0x20000);
		file.write((char*)comp->vid->v9938.reg, 64);
		for (int i = 0; i < 16; i++) {
			file.putChar(comp->vid->v9938.pal[i].r);
			file.putChar(comp->vid->v9938.pal[i].g);
			file.putChar(comp->vid->v9938.pal[i].b);
		}

		file.close();
	}
}

void MainWin::saveGBVRAM() {
	QString path = QFileDialog::getSaveFileName(this,"Save GB VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->gbc->ram, 0x2000);
		file.write((char*)comp->gb.iomap, 0x80);
		file.close();
	}
}

void MainWin::saveNESPPU() {
	QString path = QFileDialog::getSaveFileName(this,"Save GB VRAM");
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)comp->vid->ppu->mem, 0x4000);
		file.write((char*)comp->vid->ppu->oam, 0x100);
		file.close();
	}
}

void MainWin::debugAction() {
	qDebug() << gethexword(comp->vid->gbc->bgmapadr) << comp->vid->gbc->gbmode;
}
