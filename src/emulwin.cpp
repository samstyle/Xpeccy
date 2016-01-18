#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTime>
#include <QUrl>

#include <fstream>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "xcore/xcore.h"
#include "sound.h"
#include "emulwin.h"
#include "filer.h"

#ifdef DRAWQT
	#include <QPainter>
#endif

#define STR_EXPAND(tok) #tok
#define	STR(tok) STR_EXPAND(tok)
#define	XPTITLE	STR(Xpeccy VERSION)

// main

// temp emulation
unsigned short pc,af,de,ix;
unsigned char* blkData = NULL;
int blk;

static unsigned char screen[2048 * 2048 * 3];

static unsigned char scrn[1024 * 512 * 3];
static unsigned char prvScr[1024 * 512 * 3];

// LEDS

static xLed leds[] = {
	{0, 3, 60, ":/images/mouse.png"},
	{0, 3, 30, ":/images/joystick.png"},
	{-1, -1, -1, ""}
};


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
	vidUpdate(comp->vid, conf.brdsize);
	sndCalibrate();
	int szw = comp->vid->vsze.h * conf.vid.scale;
	int szh = comp->vid->vsze.v * conf.vid.scale;
	setFixedSize(szw,szh);
	lineBytes = szw * 3;
	frameBytes = szh * lineBytes;
	scrImg = QImage(screen, szw, szh, QImage::Format_RGB888);
	updateHead();
	block = 0;
	if (dbg->isVisible()) dbg->fillAll();
}

bool MainWin::saveChanged() {
	bool yep = saveChangedDisk(comp,0);
	yep &= saveChangedDisk(comp,1);
	yep &= saveChangedDisk(comp,2);
	yep &= saveChangedDisk(comp,3);
	return yep;
}

void MainWin::pause(bool p, int msk) {
	ethread.fast = 0;
	if (p) {
		pauseFlags |= msk;
	} else {
		pauseFlags &= ~msk;
	}
	sndPause(pauseFlags != 0);
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
	if (msk & PR_PAUSE) return;
	if (conf.vid.fullScreen) updateWindow();
}

// Main window

MainWin::MainWin() {
	setWindowTitle(XPTITLE);
	setMouseTracking(true);
	icon = QIcon(":/images/xpeccy.png");
	setWindowIcon(icon);
	setAcceptDrops(true);
	setAutoFillBackground(false);
	pauseFlags = 0;
	scrCounter = 0;
	scrInterval = 0;
	grabMice = 0;
	block = 0;

	vidInitAdrs();
	sndInit();
	initPaths();
	addProfile("default","xpeccy.conf");

	initKeyMap();
	conf.scrShot.format = "png";
	addLayout("default",448,320,138,80,64,32,0,0,64);

	shotFormat["bmp"] = SCR_BMP;
	shotFormat["png"] = SCR_PNG;
	shotFormat["jpg"] = SCR_JPG;
	shotFormat["scr"] = SCR_SCR;
	shotFormat["hobeta"] = SCR_HOB;

	opt = new SetupWin(this);
	dbg = new DebugWin(this);

	initFileDialog(this);
	connect(opt,SIGNAL(closed()),this,SLOT(optApply()));
	connect(dbg,SIGNAL(closed()),this,SLOT(dbgReturn()));

	tapeWin = new TapeWin(this);
	connect(tapeWin,SIGNAL(stateChanged(int,int)),this,SLOT(tapStateChanged(int,int)));

	rzxWin = new RZXWin(this);
	connect(rzxWin,SIGNAL(stateChanged(int)),this,SLOT(rzxStateChanged(int)));

	initUserMenu();

	keywin = new QLabel;
	QPixmap pxm(":/images/keymap.png");
	keywin->setPixmap(pxm);
	keywin->setFixedSize(pxm.size());
	keywin->setWindowIcon(QIcon(":/images/keyboard.png"));
	keywin->setWindowTitle("ZX Keyboard");

	connect(&cmosTimer,SIGNAL(timeout()),this,SLOT(cmosTick()));
	cmosTimer.start(1000);
	connect(&timer,SIGNAL(timeout()),this,SLOT(onTimer()));
	timer.start(20);

	ethread.conf = &conf;
	connect(&ethread,SIGNAL(dbgRequest()),SLOT(doDebug()));
	connect(&ethread,SIGNAL(tapeSignal(int,int)),this,SLOT(tapStateChanged(int,int)));
	ethread.start();

	scrImg = QImage(100,100,QImage::Format_RGB888);
	connect(userMenu,SIGNAL(aboutToShow()),SLOT(menuShow()));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));

	loadConfig();
	fillUserMenu();
}

// scale screen

// 2:1 -> 4:4 = double each pixel & copy each row 3 times
void scrX4(unsigned char* src, int wid, int lines) {
	unsigned char* dst = screen;
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
	unsigned char* dst = screen;
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
	unsigned char* dst = screen;
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
	unsigned char* dst = screen;
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

// convert <size> bytes (by 3) on <ptr> from color-RGB to gray-RGB
void scrGray(unsigned char* ptr, int size) {
	int gray;
	while (size > 0) {
		gray = qGray(*ptr, *(ptr+1), *(ptr+2));
		*(ptr++) = gray & 0xff;
		*(ptr++) = gray & 0xff;
		*(ptr++) = gray & 0xff;
		size -= 3;
	}
}

// mix prev <size> bytes from <src> to <dst> 50/50 and copy unmixed <dst> to <src>
void scrMix(unsigned char* src, unsigned char* dst, int size) {
	unsigned char cur;
	while (size > 0) {
		cur = *dst;
		*dst = (*src + cur) >> 1;
		*src = cur;
		src++;
		dst++;
		size--;
	}
}

void MainWin::convImage() {
	memcpy(scrn, comp->vid->scrimg, comp->vid->vBytes);
	if (conf.vid.grayScale) scrGray(scrn, comp->vid->vBytes);
	if (conf.vid.noFlick) scrMix(prvScr, scrn, comp->vid->vBytes);
	switch(conf.vid.scale) {
		case 1:	scrX1(scrn, comp->vid->vsze.h, comp->vid->vsze.v);
			break;
		case 2:	scrX2(scrn, comp->vid->vsze.h, comp->vid->vsze.v);
			break;
		case 3:	scrX3(scrn, comp->vid->vsze.h, comp->vid->vsze.v);
			break;
		case 4: scrX4(scrn, comp->vid->vsze.h, comp->vid->vsze.v);
			break;
	}
}

void MainWin::onTimer() {
	if (opt->block) return;
	if (opt->prfChanged) {
		opt->prfChanged = 0;
		comp = conf.prof.cur->zx;
		ethread.comp = comp;
	}
	if (block) return;
// if not paused play sound buffer
	if (conf.snd.enabled && (conf.snd.mute || isActiveWindow())) sndPlay();
// if window is not active release keys & buttons
	if (!isActiveWindow()) {
		keyReleaseAll(comp->keyb);
		comp->mouse->buttons = 0xff;
	}

// take screenshot
	convImage();
	if (!conf.scrShot.noLeds) putLeds();
	if (scrCounter > 0) {
		if (scrInterval > 0) {
			scrInterval--;
		} else {
			screenShot();
			scrCounter--;
			scrInterval = conf.scrShot.interval;
		}
	}
	if (conf.scrShot.noLeds) putLeds();
// update window
	if (pauseFlags == 0) ethread.mtx.unlock();
	emuDraw();
}

void MainWin::menuShow() {
	pause(true,PR_MENU);
}

void MainWin::menuHide() {
	setFocus();
	pause(false,PR_MENU);
}

unsigned char toBCD(unsigned char val) {
	unsigned char rrt = val % 10;
	rrt |= ((val/10) << 4);
	return rrt;
}

void incBCDbyte(unsigned char& val) {
	val++;
	if ((val & 0x0f) < 0x0a) return;
	val += 0x10;
	val &= 0xf0;
}

void incTime(CMOS* cms) {
	incBCDbyte(cms->data[0]);		// sec
	if (cms->data[0] < 0x60) return;
	cms->data[0] = 0x00;
	incBCDbyte(cms->data[2]);		// min
	if (cms->data[2] < 0x60) return;
	cms->data[2] = 0x00;
	incBCDbyte(cms->data[4]);		// hour
	if (cms->data[4] < 0x24) return;
	cms->data[4] = 0x00;
}

void MainWin::cmosTick() {
	foreach(xProfile* prf, conf.prof.list) {
		if (prf->zx != NULL) {
			if (conf.sysclock) {
				time_t rtime;
				time(&rtime);
				tm* ctime = localtime(&rtime);
				prf->zx->cmos.data[0] = toBCD(ctime->tm_sec);
				prf->zx->cmos.data[2] = toBCD(ctime->tm_min);
				prf->zx->cmos.data[4] = toBCD(ctime->tm_hour);
				prf->zx->cmos.data[6] = toBCD(ctime->tm_wday);
				prf->zx->cmos.data[7] = toBCD(ctime->tm_mday);
				prf->zx->cmos.data[8] = toBCD(ctime->tm_mon);
				prf->zx->cmos.data[9] = toBCD(ctime->tm_year % 100);
			} else {
				incTime(&prf->zx->cmos);
			}
		}
	}
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
			comp->rzxPlay = false;
			rzxStop(comp);
			pause(false,PR_RZX);
			break;
		case RWS_OPEN:
			pause(true,PR_RZX);
			loadFile(comp,"",FT_RZX,0);
			if (comp->rzx.size != 0) {
				rzxWin->startPlay();
			}
			pause(false,PR_RZX);
			break;
	}
}

#ifdef DRAWQT
void MainWin::paintEvent(QPaintEvent*) {
	if (block) return;
	QPainter pnt;
	pnt.begin(this);
	pnt.drawImage(0,0,scrImg);
	pnt.end();
}
#endif

void MainWin::keyPressEvent(QKeyEvent *ev) {
	keyEntry kent = getKeyEntry(ev->nativeScanCode());
	if (pckAct->isChecked()) {
		keyPressXT(comp->keyb, kent.keyCode);
		if (!kent.zxKey.key2)			// don't press 2-key keys in PC-mode
			keyPress(comp->keyb,kent.zxKey,0);
		keyPress(comp->keyb, kent.extKey, 1);
		keyPress(comp->keyb, kent.msxKey, 2);
		if (ev->key() == Qt::Key_ScrollLock) {
			compReset(comp,RES_DEFAULT);
			rzxWin->stop();
		}
	} else if (ev->modifiers() & Qt::AltModifier) {
		switch(ev->key()) {
			case Qt::Key_0:
				switch (comp->vid->vmode) {
					case VID_NORMAL: vidSetMode(comp->vid,VID_ALCO); break;
					case VID_ALCO: vidSetMode(comp->vid,VID_ATM_EGA); break;
					case VID_ATM_EGA: vidSetMode(comp->vid,VID_ATM_TEXT); break;
					case VID_ATM_TEXT: vidSetMode(comp->vid,VID_ATM_HWM); break;
					case VID_ATM_HWM: vidSetMode(comp->vid,VID_NORMAL); break;
				}
				break;
			case Qt::Key_1:
				conf.vid.scale = 1;
				updateWindow();
				convImage();
				saveConfig();
				break;
			case Qt::Key_2:
				conf.vid.scale = 2;
				updateWindow();
				convImage();
				saveConfig();
				break;
			case Qt::Key_3:
				conf.vid.scale = 3;
				updateWindow();
				convImage();
				saveConfig();
				break;
			case Qt::Key_4:
				conf.vid.scale = 4;
				updateWindow();
				convImage();
				saveConfig();
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
				keywin->show();
				break;
			case Qt::Key_N:
				conf.vid.noFlick ^= 1;
				saveConfig();
				if (conf.vid.noFlick) memcpy(prvScr, scrn, comp->vid->frmsz * 6);
				break;
		}
	} else {
		keyPress(comp->keyb, kent.zxKey, 0);
		if (kent.msxKey.key1) keyPress(comp->keyb,kent.msxKey,2);
		switch(ev->key()) {
			case Qt::Key_Pause:
				pauseFlags ^= PR_PAUSE;
				pause(true,0);
				break;
			case Qt::Key_Escape:
				ethread.fast = 0;
				pause(true, PR_DEBUG);
				dbg->start(comp);
				break;
			case Qt::Key_Menu:
				userMenu->popup(pos() + QPoint(20,20));
				userMenu->setFocus();
				break;
			case Qt::Key_Insert:
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
				comp->nmiRequest = true;
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
		}
	}
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	keyEntry kent = getKeyEntry(ev->nativeScanCode());
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
}

void MainWin::mousePressEvent(QMouseEvent *ev){
	switch (ev->button()) {
		case Qt::LeftButton:
			if (!grabMice) break;
			comp->mouse->buttons &= (comp->mouse->swapButtons ? ~0x02 : ~0x01);
			break;
		case Qt::RightButton:
			if (grabMice) {
				comp->mouse->buttons &= (comp->mouse->swapButtons ? ~0x01 : ~0x02);
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
			comp->mouse->buttons |= (comp->mouse->swapButtons ? 0x02 : 0x01);
			break;
		case Qt::RightButton:
			if (!grabMice) break;
			comp->mouse->buttons |= (comp->mouse->swapButtons ? 0x01 : 0x02);
			break;
		case Qt::MidButton:
			grabMice = !grabMice;
			if (grabMice) {
				grabMouse(QCursor(Qt::BlankCursor));
			} else {
				releaseMouse();
			}
			break;
		default: break;
	}
}

void MainWin::wheelEvent(QWheelEvent* ev) {
	if (grabMice && comp->mouse->hasWheel) {
		mouseWheel(comp->mouse,(ev->delta() < 0) ? XM_WHEELDN : XM_WHEELUP);
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
//	unsigned int i;
	std::ofstream file;
	std::string fname;
	pause(true,PR_EXIT);
//	for (i = 0; i < plist.size(); i++) {
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
		ideCloseFiles(comp->ide);
		sdcCloseFile(comp->sdc);
		timer.stop();
		ethread.finish = 1;
		ethread.mtx.unlock();		// unlock emulation thread (it exit, cuz of FL_EXIT)
		ethread.wait();
		keywin->close();
		ev->accept();
	} else {
		ev->ignore();
		pause(false,PR_EXIT);
	}
}

void MainWin::checkState() {
	if (comp->rzxPlay) rzxWin->startPlay();
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
	fnams.append(QTime::currentTime().toString("HHmmss_zzz")).append(".").append(QString(fext.c_str()));
	std::string fnam(fnams.toUtf8().data());
	std::ofstream file;
	QImage img(screen, width(), height(), QImage::Format_RGB888);
	int x,y,dx,dy;
	char pageBuf[0x4000];
	memGetPage(comp->mem,MEM_RAM,comp->vid->curscr,pageBuf);
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
				x = (comp->vid->bord.h - comp->vid->lcut.h) * conf.vid.scale;
				y = (comp->vid->bord.v - comp->vid->lcut.v) * conf.vid.scale;
				dx = 256 * conf.vid.scale;
				dy = 192 * conf.vid.scale;
				img = img.copy(x, y, dx, dy);
			}
			img.save(QString(fnam.c_str()),fext.c_str());
			break;
	}
}

// video drawing

void drawLed(int idx, QPainter& pnt) {
	if (leds[idx].showTime > 0) {
		leds[idx].showTime--;
		pnt.drawImage(leds[idx].x, leds[idx].y, QImage(leds[idx].imgName));
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
	pnt.begin(&scrImg);
	if (conf.led.mouse) drawLed(0,pnt);
	if (conf.led.joy) drawLed(1,pnt);
	if (conf.led.tape && comp->tape->on)
		pnt.drawImage(3, 90, QImage(":/images/tape.png"));
	if (conf.led.keys) pnt.drawImage(3,3,kled);
	pnt.end();
}

void MainWin::emuDraw() {
	if (block) return;
// update rzx window
	if ((comp->rzxPlay) && rzxWin->isVisible()) {
		rzxWin->setProgress(comp->rzx.frame, comp->rzx.size);
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
// ...
	update();
}

#ifdef DRAWGL
static int int_log2(int val) {
	int log = 0;
	while ((val >>= 1) != 0)
		log++;
	return log;
}

void MainWin::resizeGL(int, int) {
	int w = comp->vid->wsze.h;
	int h = comp->vid->wsze.v;

	glMatrixMode(GL_PROJECTION);
	glDeleteTextures(1, &tex);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	// No borders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	// TODO: Make an option: GL_LINEAR or GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	int texsize = 2 << int_log2(w > h ? w : h);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	swapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat texw = ((GLfloat)(w)/(GLfloat)texsize);
	GLfloat texh = ((GLfloat)(h)/(GLfloat)texsize);

	glViewport(0,0,w,h);

	if (glIsList(displaylist)) glDeleteLists(displaylist, 1);
	displaylist = glGenLists(1);
	glNewList(displaylist, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	// lower left
	glTexCoord2f(0, texh); glVertex2f(-1,-1);
	// lower right
	glTexCoord2f(texw, texh); glVertex2f(1,-1);
	// upper right
	glTexCoord2f(texw, 0); glVertex2f(1,1);
	// upper left
	glTexCoord2f(0,0); glVertex2f(-1,1);
	glEnd();
	glEndList();
}

void MainWin::paintGL() {
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, comp->vid->wsze.h, comp->vid->wsze.v, GL_RGB, GL_UNSIGNED_BYTE, screen);
	glCallList(displaylist);
}
#endif

// USER MENU

void MainWin::initUserMenu() {
	userMenu = new QMenu(this);
// submenu
	fileMenu = userMenu->addMenu(QIcon(":/images/fileopen.png"),"Open...");
	bookmarkMenu = userMenu->addMenu(QIcon(":/images/star.png"),"Bookmarks");
	profileMenu = userMenu->addMenu(QIcon(":/images/profile.png"),"Profiles");
	layoutMenu = userMenu->addMenu(QIcon(":/images/display.png"),"Layout");
	vmodeMenu = userMenu->addMenu(QIcon(":/images/rulers.png"),"Video mode");
	resMenu = userMenu->addMenu(QIcon(":/images/shutdown.png"),"Reset...");
	userMenu->addSeparator();
	userMenu->addAction(QIcon(":/images/tape.png"),"Tape player",tapeWin,SLOT(show()));
	userMenu->addAction(QIcon(":/images/video.png"),"RZX player",rzxWin,SLOT(show()));
	userMenu->addSeparator();
	pckAct = userMenu->addAction(QIcon(":/images/keyboard.png"),"PC keyboard");
	pckAct->setCheckable(true);
	userMenu->addAction(QIcon(":/images/other.png"),"Options",this,SLOT(doOptions()));

	connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	connect(layoutMenu,SIGNAL(triggered(QAction*)),this,SLOT(chLayout(QAction*)));
	connect(vmodeMenu,SIGNAL(triggered(QAction*)),this,SLOT(chVMode(QAction*)));
	connect(resMenu,SIGNAL(triggered(QAction*)),this,SLOT(reset(QAction*)));
	connect(fileMenu,SIGNAL(triggered(QAction*)),this,SLOT(umOpen(QAction*)));

	fileMenu->addAction(QIcon(":/images/memory.png"),"Snapshot")->setData(FT_SNAP | FT_SPG);
	fileMenu->addAction(QIcon(":/images/tape.png"),"Tape")->setData(FT_TAPE);
	fileMenu->addAction(QIcon(":/images/floppy.png"),"Floppy")->setData(FT_DISK);
	fileMenu->addAction(QIcon(":/images/msx.png"),"Slot (MSX)")->setData(FT_SLOT);

	nsAct = vmodeMenu->addAction("No screen");
	nsAct->setData(-1);
	nsAct->setCheckable(true);
	nsAct->setChecked(false);
	vmodeMenu->addAction("ZX 256 x 192")->setData(VID_NORMAL);
	vmodeMenu->addAction("Alco 16c")->setData(VID_ALCO);
	vmodeMenu->addAction("HW multicolor")->setData(VID_HWMC);
	vmodeMenu->addSeparator();
	vmodeMenu->addAction("ATM2 EGA")->setData(VID_ATM_EGA);
	vmodeMenu->addAction("ATM2 HW multicolor")->setData(VID_ATM_HWM);
	vmodeMenu->addAction("ATM2 text")->setData(VID_ATM_TEXT);
	vmodeMenu->addAction("BaseConf text")->setData(VID_EVO_TEXT);
	vmodeMenu->addSeparator();
	vmodeMenu->addAction("TSConf 256 x 192")->setData(VID_TSL_NORMAL);
	vmodeMenu->addAction("TSConf 4bpp")->setData(VID_TSL_16);
	vmodeMenu->addAction("TSConf 8bpp")->setData(VID_TSL_256);
	vmodeMenu->addAction("TSConf text")->setData(VID_TSL_TEXT);

	resMenu->addAction("default")->setData(RES_DEFAULT);
	resMenu->addSeparator();
	resMenu->addAction("ROMpage0")->setData(RES_128);
	resMenu->addAction("ROMpage1")->setData(RES_48);
	resMenu->addAction("ROMpage2")->setData(RES_SHADOW);
	resMenu->addAction("ROMpage3")->setData(RES_DOS);

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
		profileMenu->addAction(prf->name.c_str());
	}
}

void MainWin::fillLayoutMenu() {
	layoutMenu->clear();
	foreach(xLayout lay, conf.layList) {
		layoutMenu->addAction(lay.name.c_str());
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
	nsAct->setChecked(comp->vid->noScreen);
	nsPerFrame = comp->nsPerFrame;
	sndCalibrate();
	updateWindow();
	if (comp->firstRun) {
		compReset(comp, RES_DEFAULT);
		comp->firstRun = 0;
	}
	saveConfig();
	ethread.block = 0;
}

void MainWin::profileSelected(QAction* act) {
	setProfile(std::string(act->text().toLocal8Bit().data()));
}

void MainWin::reset(QAction* act) {
	rzxWin->stop();
	compReset(comp,act->data().toInt());
}

void MainWin::chLayout(QAction* act) {
	prfSetLayout(NULL, std::string(act->text().toLocal8Bit().data()));
	prfSave("");
	updateWindow();
}

void MainWin::chVMode(QAction* act) {
	int mode = act->data().toInt();
	if (mode > 0) {
		vidSetMode(comp->vid, mode);
	} else if (mode == -1) {
		comp->vid->noScreen = act->isChecked() ? 1 : 0;
		vidSetMode(comp->vid, VID_CURRENT);
	}
}

void MainWin::umOpen(QAction* act) {
	loadFile(comp, NULL, act->data().toInt(), -1);
}

// labels

void MainWin::loadLabels(const char* nm) {
	QFile file(nm);
	if (!file.open(QFile::ReadOnly)) return;
	QString line;
	QStringList arr;
	xLabel lab;
	int bank;
	dbg->labels.clear();
	while(!file.atEnd()) {
		line = file.readLine();
		arr = line.split(QRegExp("[: \r\n]"),QString::SkipEmptyParts);
		if (arr.size() == 3) {
			bank = arr.at(0).toInt(NULL,16);
			lab.adr = arr.at(1).toInt(NULL,16) & 0x3fff;
			switch (bank) {
				case 0x05: lab.adr |= 0x4000; break;
				case 0x02: lab.adr |= 0x8000; break;
				default: lab.adr |= 0xc000; break;
			}
			lab.name = arr.at(2);
			dbg->labels.append(lab);
		}
	}
}

// emulation thread (non-GUI)

xThread::xThread() {
	sndNs = 0;
	fast = 0;
	finish = 0;
	mtx.lock();
}

void xThread::tapeCatch() {
	blk = comp->tape->block;
	if (blk >= comp->tape->blkCount) return;
	if (conf->tape.fast && comp->tape->blkData[blk].hasBytes) {
		de = comp->cpu->de;
		ix = comp->cpu->ix;
		TapeBlockInfo inf = tapGetBlockInfo(comp->tape,blk);
		blkData = (unsigned char*)realloc(blkData,inf.size + 2);
		tapGetBlockData(comp->tape,blk,blkData);
		if (inf.size == de) {
			for (int i = 0; i < de; i++) {
				memWr(comp->mem,ix,blkData[i + 1]);
				ix++;
			}
			comp->cpu->ix = ix;
			comp->cpu->de = 0;
			comp->cpu->hl = 0;
			tapNextBlock(comp->tape);
		} else {
			comp->cpu->hl = 0xff00;
		}
		comp->cpu->pc = 0x5df;
	} else {
		if (conf->tape.autostart)
			emit tapeSignal(TW_STATE,TWS_PLAY);
	}
}

void xThread::emuCycle() {
//	int endBuf = 0;
	comp->frmStrobe = 0;
	do {
		// exec 1 opcode (+ INT, NMI)
		sndNs += compExec(comp);
		// if need - request sound buffer update
		if (sndNs > nsPerSample) {
			//endBuf = sndSync(comp, fast);
			sndSync(comp, fast);
			sndNs -= nsPerSample;
		}
		// tape trap
		pc = comp->cpu->pc;
		if ((comp->mem->pt[0]->type == MEM_ROM) && (comp->mem->pt[0]->num == 1)) {
			if (pc == 0x56b) tapeCatch();
			if ((pc == 0x5e2) && conf->tape.autostart)
				emit tapeSignal(TW_STATE,TWS_STOP);
		}
	} while (!comp->brk && !comp->frmStrobe);
	comp->nmiRequest = 0;
}

void xThread::run() {
	do {
		if (!fast) mtx.lock();		// wait until unlocked (MainWin::onTimer() or at exit)
		if (finish) break;
		if (!block && !comp->brk) {
			emuCycle();
			if (comp->joy->used) {
				leds[1].showTime = 50;
				comp->joy->used = 0;
			}
			if (comp->mouse->used) {
				leds[0].showTime = 50;
				comp->mouse->used = 0;
			}
			if (comp->brk) {
				emit dbgRequest();
				//comp->brk = 0;
			}
		}
	} while (1);
	exit(0);
}
