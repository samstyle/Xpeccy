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
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "settings.h"
#include "emulwin.h"
#include "setupwin.h"
#include "debuger.h"
#include "filer.h"

#ifdef HAVESDL
//	#include <SDL_timer.h>
	#include <SDL_syswm.h>
#endif

#ifdef DRAWQT
	#include <QPainter>
#endif

#define STR_EXPAND(tok) #tok
#define	STR(tok) STR_EXPAND(tok)

#define	XPTITLE	STR(Xpeccy VERSION)

// main
// MainWin* mainWin;
QIcon curicon;
volatile int emulFlags = FL_BLOCK;

// tape player
TapeWin* tapeWin;
// rzx player
RZXWin* rzxWin;
// for user menu
QMenu* userMenu;
QMenu* bookmarkMenu;
QMenu* profileMenu;
QMenu* layoutMenu;
QMenu* vmodeMenu;
QAction* pckAct;
// temp emulation
Z80EX_WORD pc,af,de,ix;
int blkDataSize = 0;
unsigned char* blkData = NULL;
int blk;

int prc;
int ns = 0;				// ns counter

static unsigned char screen[1024 * 1024 * 3];
static unsigned char prevscr[1024 * 1024 * 3];

//void emulInit() {
//	mainWin = new MainWin;
//}

//void emulPause(bool state, int mask) {
//	mainWin->pause(state, mask);
//}

//void emulUpdateWindow() {
//	mainWin->updateWindow();
//}

// LEDS

static xLed leds[] = {
	{0, 68, 3, ":/images/mouse.png"},
	{0, 35, 3, ":/images/joystick.png"},
	{-1, -1, -1, ""}
};

// KEYMAPS

keyEntry keyMap[256];	// current keymap (init at start from keyMapInit[]

#define ENDKEY 0

keyEntry keyMapInit[] = {
	{"1",XKEY_1,'1',0,0x16},{"2",XKEY_2,'2',0,0x1e},{"3",XKEY_3,'3',0,0x26},{"4",XKEY_4,'4',0,0x25},{"5",XKEY_5,'5',0,0x2e},
	{"6",XKEY_6,'6',0,0x36},{"7",XKEY_7,'7',0,0x3d},{"8",XKEY_8,'8',0,0x3e},{"9",XKEY_9,'9',0,0x46},{"0",XKEY_0,'0',0,0x45},
	{"Q",XKEY_Q,'q',0,0x15},{"W",XKEY_W,'w',0,0x1d},{"E",XKEY_E,'e',0,0x24},{"R",XKEY_R,'r',0,0x2d},{"T",XKEY_T,'t',0,0x2c},
	{"Y",XKEY_Y,'y',0,0x35},{"U",XKEY_U,'u',0,0x3c},{"I",XKEY_I,'i',0,0x43},{"O",XKEY_O,'o',0,0x44},{"P",XKEY_P,'p',0,0x4d},
	{"A",XKEY_A,'a',0,0x1c},{"S",XKEY_S,'s',0,0x1b},{"D",XKEY_D,'d',0,0x23},{"F",XKEY_F,'f',0,0x2b},{"G",XKEY_G,'g',0,0x34},
	{"H",XKEY_H,'h',0,0x33},{"J",XKEY_J,'j',0,0x3b},{"K",XKEY_K,'k',0,0x42},{"L",XKEY_L,'l',0,0x4b},{"ENT",XKEY_ENTER,'E',0,0x5a},
	{"LS",XKEY_LSHIFT,'C',0,0x12},{"Z",XKEY_Z,'z',0,0x1a},{"X",XKEY_X,'x',0,0x22},{"C",XKEY_C,'c',0,0x21},{"V",XKEY_V,'v',0,0x2a},
	{"B",XKEY_B,'b',0,0x32},{"N",XKEY_N,'n',0,0x31},{"M",XKEY_M,'m',0,0x3a},{"LC",XKEY_LCTRL,'S',0,0x14},{"SPC",XKEY_SPACE,' ',0,0x29},

	{"RS",XKEY_RSHIFT,'C',0,0x59},{"RC",XKEY_RCTRL,'S',0,0x14e0},

	{"`",XKEY_TILDA,'C','S',0x0e},{"\\",XKEY_SLASH,'C','S',0x5d},
	{";",XKEY_DOTCOM,'S','o',0x4c},{"\"",XKEY_QUOTE,'S','p',0x52},
	{"TAB",XKEY_TAB,'C',' ',0x0d},{"CAPS",XKEY_CAPS,'C','2',0x58},
	{"PGDN",XKEY_PGUP,'C','3',0x7de0},{"PGUP",XKEY_PGDN,'C','4',0x7ae0},{"BSP",XKEY_BSP,'C','0',0x66},
	{"DEL",XKEY_DEL,'C','9',0x71e0},{"INS",XKEY_INS,'S','w',0x70e0},{"HOME",XKEY_HOME,'S','q',0x6ce0},{"END",XKEY_END,'S','e',0x69e0},
	{"LEFT",XKEY_LEFT,'C','5',0x6be0},{"DOWN",XKEY_DOWN,'C','6',0x72e0},{"UP",XKEY_UP,'C','7',0x75e0},{"RIGHT",XKEY_RIGHT,'C','8',0x74e0},
	{"-",XKEY_MINUS,'S','j',0x4e},{"+",XKEY_PLUS,'S','k',0x00},
	{",",XKEY_PERIOD,'S','n',0x41},{".",XKEY_COMMA,'S','m',0x49},{"/",XKEY_BSLASH,'S','c',0x4a},
	{"[",XKEY_LBRACE,'S','8',0x54},{"]",XKEY_RBRACE,'S','9',0x5b},

	{"ESC",XKEY_ESC,0,0,0x76},
	{"F1",XKEY_F1,0,0,0x05},{"F2",XKEY_F2,0,0,0x06},{"F3",XKEY_F3,0,0,0x04},{"F4",XKEY_F4,0,0,0x0C},
	{"F5",XKEY_F5,0,0,0x03},{"F6",XKEY_F6,0,0,0x0B},{"F7",XKEY_F7,0,0,0x83},{"F8",XKEY_F8,0,0,0x0A},
	{"F9",XKEY_F9,0,0,0x01},{"F10",XKEY_F10,0,0,0x09},{"F11",XKEY_F11,0,0,0x78},

	{"LA",XKEY_LALT,0,0,0x11},{"RA",XKEY_RALT,0,0,0x11e0},

	{"",ENDKEY,0,0,0x00}
};

keyEntry getKeyEntry(qint32 qkey) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (keyMap[idx].key != qkey)) {
		idx++;
	}
	return keyMap[idx];
}

void setKey(const char* key,const char key1,const char key2) {
	int idx = 0;
	while (keyMap[idx].key != ENDKEY) {
		if (strcmp(key,keyMap[idx].name) == 0) {
			keyMap[idx].key1 = key1;
			keyMap[idx].key2 = key2;
		}
		idx++;
	}
}

void initKeyMap() {
	int idx = -1;
	do {
		idx++;
		keyMap[idx] = keyMapInit[idx];
	} while (keyMapInit[idx].key != ENDKEY);
}

keyEntry getKeyEntry(const char* name) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (strcmp(keyMap[idx].name,name) != 0)) {
		idx++;
	}
	return keyMap[idx];
}

void MainWin::updateHead() {
	QString title(XPTITLE);
	XProfile* curProf = getCurrentProfile();
	if (curProf != NULL) {
		title.append(" | ").append(QString::fromLocal8Bit(curProf->name.c_str()));
		title.append(" | ").append(QString::fromLocal8Bit(curProf->layName.c_str()));
	}
	if (ethread.fast) {
		title.append(" | fast");
	}
	setWindowTitle(title);
}

#ifdef WORDS_BIG_ENDIAN
	#define RMASK 0xff000000
	#define GMASK 0x00ff0000
	#define BMASK 0x0000ff00
	#define AMASK 0x000000ff
#else
	#define RMASK 0x000000ff
	#define GMASK 0x0000ff00
	#define BMASK 0x00ff0000
	#define AMASK 0xff000000
#endif

void MainWin::updateWindow() {
	emulFlags |= FL_BLOCK;
	vidUpdate(comp->vid);
	int szw = comp->vid->wsze.h;
	int szh = comp->vid->wsze.v;
	setFixedSize(szw,szh);
	scrImg = QImage(screen,szw,szh,QImage::Format_RGB888);
	updateHead();
	emulFlags &= ~FL_BLOCK;
}

bool MainWin::saveChanged() {
	bool yep = saveChangedDisk(comp,0);
	yep &= saveChangedDisk(comp,1);
	yep &= saveChangedDisk(comp,2);
	yep &= saveChangedDisk(comp,3);
	return yep;
}

void MainWin::pause(bool p, int msk) {
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
		setWindowIcon(curicon);
		if (grabMice) grabMouse(QCursor(Qt::BlankCursor));
		ethread.block = 0;
	} else {
		setWindowIcon(QIcon(":/images/pause.png"));
		ethread.block = 1;
	}
	updateHead();
	if (msk & PR_PAUSE) return;
	if ((pauseFlags & ~PR_PAUSE) == 0) {
		vidFlag &= ~VF_BLOCKFULLSCREEN;
		if (vidFlag & VF_FULLSCREEN) updateWindow();
	} else {
		vidFlag |= VF_BLOCKFULLSCREEN;
		if (vidFlag & VF_FULLSCREEN) updateWindow();
	}
}

// Main window

MainWin::MainWin() {
	setWindowTitle(XPTITLE);
	setMouseTracking(true);
	curicon = QIcon(":/images/xpeccy.png");
	setWindowIcon(curicon);
	setAcceptDrops(true);
	setAutoFillBackground(false);
	pauseFlags = 0;
	scrCounter = 0;
	scrInterval = 0;
	grabMice = 0;

	emulFlags = 0;

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

	initUserMenu((QWidget*)this);

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

	ethread.fast = 0;
	ethread.mtx.lock();
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

// 2:1 -> 2:2 = double each line
void scrDouble (unsigned char* src, int wid, int lines) {
	unsigned char* dst = screen;
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

void scrNormal (unsigned char* src, int wid, int lines) {
	unsigned char* dst = screen;
	int cnt;
	while (lines > 0) {
		for (cnt = 0; cnt < wid; cnt += 3) {
			*dst = *src;
			dst++; src++;
			*dst = *src;
			dst++; src++;
			*dst = *src;
			dst++;
			src += 4;
		}
		lines--;
	}
}

void convImage(ZXComp* comp) {
	if (vidFlag & VF_DOUBLE) {
		scrDouble(comp->vid->scrimg, comp->vid->lineBytes, comp->vid->vsze.v);
	} else {
		scrNormal(comp->vid->scrimg, comp->vid->lineBytes, comp->vid->vsze.v);
	}
}

void MainWin::onTimer() {
	if (emulFlags & FL_BLOCK) return;
// if not paused play sound buffer
	if (sndEnabled && (sndMute || isActiveWindow())) sndPlay();
// if window is not active release keys & buttons
	if (!isActiveWindow()) {
		keyRelease(comp->keyb,0,0,0);
		comp->mouse->buttons = 0xff;
	}
// take screenshot
	if (scrCounter > 0) {
		if (scrInterval > 0) {
			scrInterval--;
		} else {
			screenShot();
			scrCounter--;
			scrInterval = conf.scrShot.interval;
		}
	}
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
	unsigned int i;
	ZXComp* comp;
	std::vector<XProfile> plist = getProfileList();
	for (i = 0; i < plist.size(); i++) {
		comp = plist[i].zx;
		if (comp != NULL) {
			if (conf.sysclock) {
				time_t rtime;
				time(&rtime);
				tm* ctime = localtime(&rtime);
				comp->cmos.data[0] = toBCD(ctime->tm_sec);
				comp->cmos.data[2] = toBCD(ctime->tm_min);
				comp->cmos.data[4] = toBCD(ctime->tm_hour);
				comp->cmos.data[6] = toBCD(ctime->tm_wday);
				comp->cmos.data[7] = toBCD(ctime->tm_mday);
				comp->cmos.data[8] = toBCD(ctime->tm_mon);
				comp->cmos.data[9] = toBCD(ctime->tm_year % 100);
			} else {
				incTime(&comp->cmos);
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
			rzxClear(comp);
			pause(false,PR_RZX);
			break;
		case RWS_OPEN:
			pause(true,PR_RZX);
			loadFile(comp,"",FT_RZX,0);
			if (comp->rzxSize != 0) {
				rzxWin->startPlay();
			}
			pause(false,PR_RZX);
			break;
	}
}

// convert <size> dots on <ptr> from color-RGB to gray-RGB
void scrGray(unsigned char* ptr, int size) {
	int gray;
	while (size > 0) {
		gray = qGray(*ptr, *(ptr+1), *(ptr+2));
		*(ptr++) = gray & 0xff;
		*(ptr++) = gray & 0xff;
		*(ptr++) = gray & 0xff;
		size--;
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

#ifdef DRAWQT
void MainWin::paintEvent(QPaintEvent*) {
	if (emulFlags & FL_BLOCK) return;
	QPainter pnt;
	pnt.begin(this);
	pnt.drawImage(0,0,scrImg);
	pnt.end();
}
#endif

void MainWin::keyPressEvent(QKeyEvent *ev) {
	keyEntry kent;
	if (pckAct->isChecked()) {
		kent = getKeyEntry(ev->nativeScanCode());
		keyPress(comp->keyb,kent.key1,kent.key2,kent.keyCode);
		if (ev->key() == Qt::Key_F12) {
			zxReset(comp,RES_DEFAULT);
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
				vidFlag &= ~VF_DOUBLE;
				updateWindow();
				saveConfig();
				break;
			case Qt::Key_2:
				vidFlag |= VF_DOUBLE;
				updateWindow();
				saveConfig();
				break;
			case Qt::Key_3:
				ethread.fast ^= 1;
				updateHead();
				break;
			case Qt::Key_F4:
				close();
				break;
			case Qt::Key_F7:
				scrCounter = conf.scrShot.count;
				scrInterval = 0;
				break;	// ALT+F7 combo
			case Qt::Key_F12:
				zxReset(comp,RES_DOS);
				rzxWin->stop();
				break;
			case Qt::Key_K:
				keywin->show();
				break;
			case Qt::Key_N:
				vidFlag ^= VF_NOFLIC;
				saveConfig();
				if (vidFlag & VF_NOFLIC) memcpy(prevscr,screen,comp->vid->frameBytes);
				break;
		}
	} else {
		kent = getKeyEntry(ev->nativeScanCode());
		if ((kent.key1 || kent.key2) || pckAct->isChecked())
			keyPress(comp->keyb,kent.key1,kent.key2,kent.keyCode);
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
			case Qt::Key_F1:
				pause(true, PR_OPTS);
				opt->start(comp);
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
				zxReset(comp,RES_DEFAULT);
				rzxWin->stop();
				break;
		}
	}
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	keyEntry kent = getKeyEntry(ev->nativeScanCode());
	if (kent.key1 || kent.key2 || pckAct->isChecked())
		keyRelease(comp->keyb,kent.key1,kent.key2,kent.keyCode);
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
	unsigned int i;
	std::ofstream file;
	std::string fname;
	std::vector<XProfile> plist = getProfileList();
	pause(true,PR_EXIT);
	for (i = 0; i < plist.size(); i++) {
		prfSave(plist[i].name);
		fname = conf.path.confDir + SLASH + plist[i].name + ".cmos";
		file.open(fname.c_str());
		if (file.good()) {
			file.write((const char*)plist[i].zx->cmos.data,256);
			file.close();
		}
		if (plist[i].zx->ide->type == IDE_SMUC) {
			fname = conf.path.confDir + SLASH + plist[i].name + ".nvram";
			file.open(fname.c_str());
			if (file.good()) {
				file.write((const char*)plist[i].zx->ide->smuc.nv->mem,0x800);
				file.close();
			}
		}
	}
	if (saveChanged()) {
		ideCloseFiles(comp->ide);
		sdcCloseFile(comp->sdc);
		emulFlags |= FL_EXIT;
		timer.stop();
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
	QImage *img = new QImage(screen,comp->vid->wsze.h, comp->vid->wsze.v,QImage::Format_RGB888);
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
			if (img != NULL) img->save(QString(fnam.c_str()),fext.c_str());
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

void MainWin::emuDraw() {
	if (emulFlags & FL_BLOCK) return;
// update rzx window
	if ((comp->rzxPlay) && rzxWin->isVisible()) {
		prc = 100 * comp->rzxFrame / comp->rzxSize;
		rzxWin->setProgress(prc);
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
// form image...
	if (comp->debug || ethread.fast) convImage(comp);
	int winDots = width() * height();
	if (vidFlag & VF_GREY) scrGray(screen, winDots);
	if (vidFlag & VF_NOFLIC) scrMix(prevscr, screen, winDots * 3);
// leds
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
	if (conf.led.keys) pnt.drawImage(3,3,kled);
	pnt.end();
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

void initUserMenu(QWidget* par) {
	QAction* act;
	QMenu* resMenu;
	userMenu = new QMenu(par);

	bookmarkMenu = userMenu->addMenu(QIcon(":/images/star.png"),"Bookmarks");
	QObject::connect(bookmarkMenu,SIGNAL(triggered(QAction*)),par,SLOT(bookmarkSelected(QAction*)));

	profileMenu = userMenu->addMenu(QIcon(":/images/profile.png"),"Profiles");
	QObject::connect(profileMenu,SIGNAL(triggered(QAction*)),par,SLOT(profileSelected(QAction*)));

	layoutMenu = userMenu->addMenu(QIcon(":/images/display.png"),"Layout");
	QObject::connect(layoutMenu,SIGNAL(triggered(QAction*)),par,SLOT(chLayout(QAction*)));

	vmodeMenu = userMenu->addMenu("Video mode");
	QObject::connect(vmodeMenu,SIGNAL(triggered(QAction*)),par,SLOT(chVMode(QAction*)));
	act = vmodeMenu->addAction("No screen");
	act->setData(-1);
	act->setCheckable(true);
	act->setChecked(vidFlag & VF_NOSCREEN);
//	vmodeMenu->addAction("No screen")->setData(VID_NOSCREEN);
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

	resMenu = userMenu->addMenu(QIcon(":/images/shutdown.png"),"Reset...");
	QObject::connect(resMenu,SIGNAL(triggered(QAction*)),par,SLOT(reset(QAction*)));
	resMenu->addAction("default")->setData(RES_DEFAULT);
	resMenu->addSeparator();
	resMenu->addAction("ROMpage0")->setData(RES_128);
	resMenu->addAction("ROMpage1")->setData(RES_48);
	resMenu->addAction("ROMpage2")->setData(RES_SHADOW);
	resMenu->addAction("ROMpage3")->setData(RES_DOS);

	userMenu->addSeparator();
	userMenu->addAction(QIcon(":/images/tape.png"),"Tape window",tapeWin,SLOT(show()));
	userMenu->addAction(QIcon(":/images/video.png"),"RZX player",rzxWin,SLOT(show()));
	userMenu->addSeparator();
	pckAct = userMenu->addAction(QIcon(":/images/keyboard.png"),"PC keyboard");
	pckAct->setCheckable(true);
	userMenu->addAction(QIcon(":/images/other.png"),"Options",par,SLOT(doOptions()));
}

void fillBookmarkMenu() {
	bookmarkMenu->clear();
	QAction* act;
	std::vector<XBookmark> bookmarkList = getBookmarkList();
	if (bookmarkList.size() == 0) {
		bookmarkMenu->addAction("None")->setEnabled(false);
	} else {
		for(uint i=0; i<bookmarkList.size(); i++) {
			act = bookmarkMenu->addAction(QString::fromLocal8Bit(bookmarkList[i].name.c_str()));
			act->setData(QVariant(QString::fromLocal8Bit(bookmarkList[i].path.c_str())));
		}
	}
}

void fillProfileMenu() {
	profileMenu->clear();
	std::vector<XProfile> profileList = getProfileList();
	for(uint i=0; i < profileList.size(); i++) {
		profileMenu->addAction(profileList[i].name.c_str());
	}
}

void fillLayoutMenu() {
	layoutMenu->clear();
	std::vector<VidLayout> layoutList = getLayoutList();
	for (uint i = 0; i < layoutList.size(); i++) {
		layoutMenu->addAction(layoutList[i].name.c_str());
	}
}

void fillUserMenu() {
	fillBookmarkMenu();
	fillProfileMenu();
	fillLayoutMenu();
}

// SLOTS

void MainWin::doOptions() {
	pause(true, PR_OPTS);
	opt->start(comp);
}

void MainWin::optApply() {
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
	selProfile(nm);
	comp = getCurrentProfile()->zx;
	ethread.comp = comp;
	nsPerFrame = comp->nsPerFrame;
	sndCalibrate();
	updateWindow();
	if (comp->firstRun) {
		zxReset(comp, RES_DEFAULT);
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
	zxReset(comp,act->data().toInt());
}

void MainWin::chLayout(QAction* act) {
//	emulPause(true,PR_EXTRA);
	emulSetLayout(comp,std::string(act->text().toLocal8Bit().data()));
	prfSave("");
	updateWindow();
//	setFocus();
//	emulPause(false,PR_EXTRA);
}

void MainWin::chVMode(QAction* act) {
	int mode = act->data().toInt();
	if (mode > 0) {
		vidSetMode(comp->vid, mode);
	} else if (mode == -1) {
		vidFlag ^= VF_NOSCREEN;
		act->setChecked(vidFlag & VF_NOSCREEN);
		vidSetMode(comp->vid, VID_CURRENT);
	}
}

// emulation thread (non-GUI)

void xThread::tapeCatch() {
	blk = comp->tape->block;
	if (blk >= comp->tape->blkCount) return;
	if (conf->tape.fast && comp->tape->blkData[blk].hasBytes) {
		de = GETDE(comp->cpu);	//z80ex_get_reg(comp->cpu,regDE);
		ix = GETIX(comp->cpu);	//z80ex_get_reg(comp->cpu,regIX);
		TapeBlockInfo inf = tapGetBlockInfo(comp->tape,blk);
		blkData = (unsigned char*)realloc(blkData,inf.size + 2);
		tapGetBlockData(comp->tape,blk,blkData);
		if (inf.size == de) {
			for (int i = 0; i < de; i++) {
				memWr(comp->mem,ix,blkData[i + 1]);
				ix++;
			}
			SETIX(comp->cpu,ix);	// z80ex_set_reg(comp->cpu,regIX,ix);
			SETDE(comp->cpu,0);	//z80ex_set_reg(comp->cpu,regDE,0);
			SETHL(comp->cpu,0);	//z80ex_set_reg(comp->cpu,regHL,0);
			tapNextBlock(comp->tape);
		} else {
			SETHL(comp->cpu,0xff00);	//z80ex_set_reg(comp->cpu,regHL,0xff00);
		}
		SETPC(comp->cpu,0x5df);	// z80ex_set_reg(comp->cpu,regPC,0x5df);	// to exit
	} else {
		if (conf->tape.autostart)
			emit tapeSignal(TW_STATE,TWS_PLAY);
	}
}

void xThread::emuCycle() {
	comp->frmStrobe = 0;
	do {
		// exec 1 opcode (+ INT, NMI)
		ns += zxExec(comp);
		// if need - request sound buffer update
		if (ns > nsPerSample) {
			sndSync(comp, fast);
			ns -= nsPerSample;
		}
		// tape trap
		pc = GETPC(comp->cpu);	// z80ex_get_reg(comp->cpu,regPC);
		if ((comp->mem->pt[0]->type == MEM_ROM) && (comp->mem->pt[0]->num == 1)) {
			if (pc == 0x56b) tapeCatch();
			if ((pc == 0x5e2) && conf->tape.autostart)
				emit tapeSignal(TW_STATE,TWS_STOP);
		}
	} while (!comp->brk && (comp->frmStrobe == 0));		// exec until breakpoint or INT

	if (!(comp->debug || fast)) convImage(comp);

	comp->nmiRequest = 0;
	// decrease frames & screenshot counter (if any), request screenshot (if needed)
}

void xThread::run() {
	do {
		if (!fast) mtx.lock();		// wait until unlocked (MainWin::onTimer() or at exit)
		if (emulFlags & FL_EXIT) break;
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
				comp->brk = 0;
			}
		}
	} while (1);
	exit(0);
}
