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
MainWin* mainWin;
QIcon curicon;
volatile int emulFlags = FL_BLOCK;
volatile int pauseFlags = 0;
//int wantedWin;
volatile unsigned int scrCounter;
volatile unsigned int scrInterval;

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

void emulInit() {
	initKeyMap();
	emulFlags = 0;

	scrCounter = 0;
	scrInterval = 0;
	optSet(OPT_SHOTFRM,SCR_PNG);

	addLayout("default",448,320,138,80,64,32,0,0,64);

	mainWin = new MainWin;

	optInit(mainWin);
	initFileDialog(mainWin);
}

// LEDS

static xLed leds[] = {
	{FL_LED_MOUSE, 0, 3, 3, ":/images/mouse.png"},
	{FL_LED_JOY, 0, 35, 3, ":/images/joystick.png"},
	{0, -1, -1, -1, ""}
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

void emulShow() {
	mainWin->show();
}

void emulUpdateWindow() {
	mainWin->updateWindow();
}

void MainWin::updateHead() {
	QString title(XPTITLE);
	XProfile* curProf = getCurrentProfile();
	if (curProf != NULL) {
		title.append(" | ").append(QString::fromLocal8Bit(curProf->name.c_str()));
		title.append(" | ").append(QString::fromLocal8Bit(curProf->layName.c_str()));
	}
	if (emulFlags & FL_FAST) {
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
	vidUpdate(zx->vid);
	int szw = zx->vid->wsze.h;
	int szh = zx->vid->wsze.v;
	setFixedSize(szw,szh);
	scrImg = QImage(screen,szw,szh,QImage::Format_RGB888);
	updateHead();
	emulFlags &= ~FL_BLOCK;
}

bool emulSaveChanged() {
	bool yep = saveChangedDisk(zx,0);
	yep &= saveChangedDisk(zx,1);
	yep &= saveChangedDisk(zx,2);
	yep &= saveChangedDisk(zx,3);
	return yep;
}

void emulSetFlag(int msk,bool cnd) {
	if (cnd) {
		emulFlags |= msk;
	} else {
		emulFlags &= ~msk;
	}
}

void emulPause(bool p, int msk) {
	if (p) {
		pauseFlags |= msk;
	} else {
		pauseFlags &= ~msk;
	}
	sndPause(pauseFlags != 0);
	bool kk = ((emulFlags & FL_GRAB) != 0);
	if (!kk || ((pauseFlags != 0) && kk)) {
		mainWin->releaseMouse();
	}
	if (pauseFlags == 0) {
		mainWin->setWindowIcon(curicon);
		if (kk) mainWin->grabMouse(QCursor(Qt::BlankCursor));

	} else {
		mainWin->setWindowIcon(QIcon(":/images/pause.png"));
	}
	if (msk & PR_PAUSE) return;
	if ((pauseFlags & ~PR_PAUSE) == 0) {
		vidFlag &= ~VF_BLOCKFULLSCREEN;
		if (vidFlag & VF_FULLSCREEN) emulUpdateWindow();
	} else {
		vidFlag |= VF_BLOCKFULLSCREEN;
		if (vidFlag & VF_FULLSCREEN) emulUpdateWindow();
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

	ethread.start();

	scrImg = QImage(100,100,QImage::Format_RGB888);
	connect(userMenu,SIGNAL(aboutToShow()),SLOT(menuShow()));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));
	connect(&ethread,SIGNAL(dbgRequest()),SLOT(doDebug()));

}

void doScreenShot();

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
		scrDouble(zx->vid->scrimg, zx->vid->lineBytes, zx->vid->vsze.v);
	} else {
		scrNormal(zx->vid->scrimg, zx->vid->lineBytes, zx->vid->vsze.v);
	}
}

void MainWin::onTimer() {
	if (emulFlags & FL_BLOCK) return;
// if not paused play sound buffer
	if (sndEnabled && (sndMute || mainWin->isActiveWindow())) sndPlay();
	if (pauseFlags != 0) return;
// take screenshot
	if (emulFlags & FL_SHOT) doScreenShot();
// if window is not active release keys & buttons
	if (!mainWin->isActiveWindow()) {
		keyRelease(zx->keyb,0,0,0);
		zx->mouse->buttons = 0xff;
	}
// update window
	emuDraw();
	if (emulFlags & FL_WORK) ethread.mtx.unlock();
}

void MainWin::menuShow() {
	emulPause(true,PR_MENU);
}

void MainWin::menuHide() {
	setFocus();
	emulPause(false,PR_MENU);
}

void emuStart() {
	emulFlags |= FL_WORK;
}

void emuStop() {
	emulFlags &= ~FL_WORK;
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
			if (emulFlags & FL_SYSCLOCK) {
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
					if (tapPlay(zx->tape)) {
						tapeWin->setState(TWS_PLAY);
					} else {
						tapeWin->setState(TWS_STOP);
					}
					break;
				case TWS_STOP:
					tapStop(zx->tape);
					tapeWin->setState(TWS_STOP);
					break;
				case TWS_REC:
					tapRec(zx->tape);
					tapeWin->setState(TWS_REC);
					break;
				case TWS_OPEN:
					emulPause(true,PR_FILE);
					loadFile(zx,"",FT_TAPE,-1);
					tapeWin->buildList(zx->tape);
					tapeWin->setCheck(zx->tape->block);
					emulPause(false,PR_FILE);
					break;
			}
			break;
		case TW_REWIND:
			tapRewind(zx->tape,val);
			break;
		case TW_BREAK:
			zx->tape->blkData[val].breakPoint ^= 1;
			tapeWin->drawStops(zx->tape);
			break;
	}
}

// connection between rzx player and emulation state
void MainWin::rzxStateChanged(int state) {
	switch(state) {
		case RWS_PLAY:
			emulPause(false,PR_RZX);
			break;
		case RWS_PAUSE:
			emulPause(true,PR_RZX);
			break;
		case RWS_STOP:
			zx->rzxPlay = false;
			rzxClear(zx);
			emulPause(false,PR_RZX);
			break;
		case RWS_OPEN:
			emulPause(true,PR_RZX);
			loadFile(zx,"",FT_RZX,0);
			if (zx->rzxSize != 0) {
				rzxWin->startPlay();
			}
			emulPause(false,PR_RZX);
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
		keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
		if (ev->key() == Qt::Key_F12) {
			zxReset(zx,RES_DEFAULT);
			rzxWin->stop();
		}
	} else if (ev->modifiers() & Qt::AltModifier) {
		switch(ev->key()) {
			case Qt::Key_0:
				switch (zx->vid->vmode) {
					case VID_NORMAL: vidSetMode(zx->vid,VID_ALCO); break;
					case VID_ALCO: vidSetMode(zx->vid,VID_ATM_EGA); break;
					case VID_ATM_EGA: vidSetMode(zx->vid,VID_ATM_TEXT); break;
					case VID_ATM_TEXT: vidSetMode(zx->vid,VID_ATM_HWM); break;
					case VID_ATM_HWM: vidSetMode(zx->vid,VID_NORMAL); break;
				}
				break;
			case Qt::Key_1:
				vidFlag &= ~VF_DOUBLE;
				updateWindow();
				saveProfiles();
				break;
			case Qt::Key_2:
				vidFlag |= VF_DOUBLE;
				updateWindow();
				saveProfiles();
				break;
			case Qt::Key_3:
				emulFlags ^= FL_FAST;
				updateHead();
				break;
			case Qt::Key_F4:
				mainWin->close();
				break;
			case Qt::Key_F7:
				scrCounter = optGetInt(OPT_SHOTCNT);
				scrInterval = 0;
				break;	// ALT+F7 combo
			case Qt::Key_F12:
				zxReset(zx,RES_DOS);
				rzxWin->stop();
				break;
			case Qt::Key_K:
				keywin->show();
				break;
			case Qt::Key_N:
				vidFlag ^= VF_NOFLIC;
				saveProfiles();
				if (vidFlag & VF_NOFLIC) memcpy(prevscr,screen,zx->vid->frameBytes);
				break;
		}
	} else {
		kent = getKeyEntry(ev->nativeScanCode());
		if ((kent.key1 || kent.key2) || pckAct->isChecked())
			keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
		switch(ev->key()) {
			case Qt::Key_Pause:
				pauseFlags ^= PR_PAUSE;
				emulPause(true,0);
				break;
			case Qt::Key_Escape:
				dbgShow();
				break;
			case Qt::Key_Menu:
//				emulPause(true,PR_MENU);
				userMenu->popup(mainWin->pos() + QPoint(20,20));
				userMenu->setFocus();
				break;
			case Qt::Key_F1:
				optShow();
				break;
			case Qt::Key_F2:
				emulPause(true,PR_FILE);
				saveFile(zx,"",FT_ALL,-1);
				emulPause(false,PR_FILE);
				break;
			case Qt::Key_F3:
				emulPause(true,PR_FILE);
				loadFile(zx,"",FT_ALL,-1);
				emulPause(false,PR_FILE);
				checkState();
				break;
			case Qt::Key_F4:
				if (zx->tape->on) {
					mainWin->tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					mainWin->tapStateChanged(TW_STATE,TWS_PLAY);
				}
				break;
			case Qt::Key_F5:
				if (zx->tape->on) {
					mainWin->tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					mainWin->tapStateChanged(TW_STATE,TWS_REC);
				}
				break;
			case Qt::Key_F7:
				if (scrCounter == 0) {
					emulFlags |= FL_SHOT;
				} else {
					emulFlags &= ~FL_SHOT;
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
				emulPause(true,PR_FILE);
				emulSaveChanged();
				emulPause(false,PR_FILE);
				break;
			case Qt::Key_F10:
				zx->nmiRequest = true;
				break;
			case Qt::Key_F11:
				if (tapeWin->isVisible()) {
					tapeWin->hide();
				} else {
					tapeWin->buildList(zx->tape);
					tapeWin->show();
				}
				break;
			case Qt::Key_F12:
				zxReset(zx,RES_DEFAULT);
				rzxWin->stop();
				break;
		}
	}
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	keyEntry kent = getKeyEntry(ev->nativeScanCode());
	if (kent.key1 || kent.key2 || pckAct->isChecked())
		keyRelease(zx->keyb,kent.key1,kent.key2,kent.keyCode);
}

void MainWin::mousePressEvent(QMouseEvent *ev){
	switch (ev->button()) {
		case Qt::LeftButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons &= (zx->mouse->swapButtons ? ~0x02 : ~0x01);
			break;
		case Qt::RightButton:
			if (emulFlags & FL_GRAB) {
				zx->mouse->buttons &= (zx->mouse->swapButtons ? ~0x01 : ~0x02);
			} else {
//				emulPause(true,PR_MENU);
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
			if (emulFlags & FL_GRAB) zx->mouse->buttons |= (zx->mouse->swapButtons ? 0x02 : 0x01);
			break;
		case Qt::RightButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons |= (zx->mouse->swapButtons ? 0x01 : 0x02);
			break;
		case Qt::MidButton:
			emulFlags ^= FL_GRAB;
			if (emulFlags & FL_GRAB) {
				grabMouse(QCursor(Qt::BlankCursor));
			} else {
				releaseMouse();
			}
			break;
		default: break;
	}
}

void MainWin::wheelEvent(QWheelEvent* ev) {
	if ((emulFlags & FL_GRAB) && zx->mouse->hasWheel) {
		mouseWheel(zx->mouse,(ev->delta() < 0) ? XM_WHEELDN : XM_WHEELUP);
	}
}

void MainWin::mouseMoveEvent(QMouseEvent *ev) {
	if (!(emulFlags & FL_GRAB) || (pauseFlags !=0 )) return;
	zx->mouse->xpos = ev->globalX() & 0xff;
	zx->mouse->ypos = 256 - (ev->globalY() & 0xff);
}

void MainWin::dragEnterEvent(QDragEnterEvent* ev) {
	if (ev->mimeData()->hasUrls()) {
		ev->acceptProposedAction();
	}
}

void MainWin::dropEvent(QDropEvent* ev) {
	QList<QUrl> urls = ev->mimeData()->urls();
	QString fpath;
	mainWin->raise();
	mainWin->activateWindow();
	for (int i = 0; i < urls.size(); i++) {
		fpath = urls.at(i).path();
#ifdef _WIN32
		fpath.remove(0,1);	// by some reason path will start with /
#endif
		loadFile(zx,fpath.toUtf8().data(),FT_ALL,0);
	}
}


void MainWin::closeEvent(QCloseEvent* ev) {
	unsigned int i;
	std::ofstream file;
	std::string fname;
	std::vector<XProfile> plist = getProfileList();
	emulPause(true,PR_EXIT);
	for (i = 0; i < plist.size(); i++) {
		prfSave(plist[i].name);
		fname = optGetString(OPT_WORKDIR) + std::string(SLASH) + plist[i].name + std::string(".cmos");
		file.open(fname.c_str());
		if (file.good()) {
			file.write((const char*)plist[i].zx->cmos.data,256);
			file.close();
		}
		if (plist[i].zx->ide->type == IDE_SMUC) {
			fname = optGetString(OPT_WORKDIR) + std::string(SLASH) + plist[i].name + std::string(".nvram");
			file.open(fname.c_str());
			if (file.good()) {
				file.write((const char*)plist[i].zx->ide->smuc.nv->mem,0x800);
				file.close();
			}
		}
	}
	if (emulSaveChanged()) {
		ideCloseFiles(zx->ide);
		sdcCloseFile(zx->sdc);
		emulFlags |= FL_EXIT;
		timer.stop();
		ethread.mtx.unlock();		// unlock emulation thread (it exit, cuz of FL_EXIT)
		ethread.wait();
		keywin->close();
		ev->accept();
	} else {
		ev->ignore();
		emulPause(false,PR_EXIT);
	}
}

void MainWin::checkState() {
	if (zx->rzxPlay) rzxWin->startPlay();
	tapeWin->buildList(zx->tape);
	tapeWin->setCheck(zx->tape->block);
}

// ...

uchar hobHead[] = {'s','c','r','e','e','n',' ',' ','C',0,0,0,0x1b,0,0x1b,0xe7,0x81};	// last 2 bytes is crc

void doScreenShot() {
	int frm = optGetInt(OPT_SHOTFRM);
	std::string fext;
	switch (frm) {
		case SCR_BMP: fext = "bmp"; break;
		case SCR_JPG: fext = "jpg"; break;
		case SCR_PNG: fext = "png"; break;
		case SCR_SCR: fext = "scr"; break;
		case SCR_HOB: fext = "$C"; break;
	};
	QString fnams = QString(optGetString(OPT_SHOTDIR).c_str()).append(SLASH);
	fnams.append(QTime::currentTime().toString("HHmmss_zzz")).append(".").append(QString(fext.c_str()));
	std::string fnam(fnams.toUtf8().data());
	std::ofstream file;
	QImage *img = new QImage(zx->vid->scrimg,zx->vid->wsze.h, zx->vid->wsze.v,QImage::Format_RGB888);
	char* pageBuf = new char[0x4000];
	memGetPage(zx->mem,MEM_RAM,zx->vid->curscr,pageBuf);
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
	free(pageBuf);
	emulFlags &= ~FL_SHOT;
}

// video drawing

void MainWin::emuDraw() {
	if (emulFlags & FL_BLOCK) return;
	emulFlags |= FL_DRAW;
// update rzx window
	if ((zx->rzxPlay) && rzxWin->isVisible()) {
		prc = 100 * zx->rzxFrame / zx->rzxSize;
		rzxWin->setProgress(prc);
	}
// update tape window
	if (tapeWin->isVisible()) {
		if (zx->tape->on && !zx->tape->rec) {
			tapeWin->setProgress(tapGetBlockTime(zx->tape,zx->tape->block,zx->tape->pos),tapGetBlockTime(zx->tape,zx->tape->block,-1));
		}
		if (zx->tape->blkChange) {
			if (!zx->tape->on) {
				tapStateChanged(TW_STATE,TWS_STOP);
			}
			tapeWin->setCheck(zx->tape->block);
			zx->tape->blkChange = 0;
		}
		if (zx->tape->newBlock) {
			tapeWin->buildList(zx->tape);
			zx->tape->newBlock = 0;
		}
	}
// leds
	int idx = 0;
	QPainter pnt;
	pnt.begin(&scrImg);
	while (leds[idx].showTime > -1) {
		if ((emulFlags & leds[idx].flag) && (leds[idx].showTime > 0)) {
			pnt.drawImage(leds[idx].x, leds[idx].y, QImage(leds[idx].imgName));
			leds[idx].showTime--;
		} else {
			leds[idx].showTime = 0;
		}
		idx++;
	}
	pnt.end();
// ...
	if (zx->debug) convImage(zx);
	int winDots = zx->vid->wsze.h * zx->vid->wsze.v;
	if (vidFlag & VF_GREY) scrGray(screen, winDots);
	if (vidFlag & VF_NOFLIC) scrMix(prevscr, screen, winDots * 3);
	update();
	emulFlags &= ~FL_DRAW;
}

#ifdef DRAWGL
static int int_log2(int val) {
	int log = 0;
	while ((val >>= 1) != 0)
		log++;
	return log;
}

void MainWin::resizeGL(int, int) {
	int w = zx->vid->wsze.h;
	int h = zx->vid->wsze.v;

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
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, zx->vid->wsze.h, zx->vid->wsze.v, GL_RGB, GL_UNSIGNED_BYTE, screen);
	glCallList(displaylist);
}
#endif

void emulTapeCatch() {
	blk = zx->tape->block;
	if (blk >= zx->tape->blkCount) return;
	if (optGetFlag(OF_TAPEFAST) && zx->tape->blkData[blk].hasBytes) {
		de = GETDE(zx->cpu);	//z80ex_get_reg(zx->cpu,regDE);
		ix = GETIX(zx->cpu);	//z80ex_get_reg(zx->cpu,regIX);
		TapeBlockInfo inf = tapGetBlockInfo(zx->tape,blk);
		blkData = (unsigned char*)realloc(blkData,inf.size + 2);
		tapGetBlockData(zx->tape,blk,blkData);
		if (inf.size == de) {
			for (int i = 0; i < de; i++) {
				memWr(zx->mem,ix,blkData[i + 1]);
				ix++;
			}
			SETIX(zx->cpu,ix);	// z80ex_set_reg(zx->cpu,regIX,ix);
			SETDE(zx->cpu,0);	//z80ex_set_reg(zx->cpu,regDE,0);
			SETHL(zx->cpu,0);	//z80ex_set_reg(zx->cpu,regHL,0);
			tapNextBlock(zx->tape);
		} else {
			SETHL(zx->cpu,0xff00);	//z80ex_set_reg(zx->cpu,regHL,0xff00);
		}
		SETPC(zx->cpu,0x5df);	// z80ex_set_reg(zx->cpu,regPC,0x5df);	// to exit
	} else {
		if (optGetFlag(OF_TAPEAUTO))
			mainWin->tapStateChanged(TW_STATE,TWS_PLAY);
	}
}

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

void MainWin::doOptions() {optShow();}
void MainWin::doDebug() {dbgShow();}

void MainWin::bookmarkSelected(QAction* act) {
	loadFile(zx,act->data().toString().toLocal8Bit().data(),FT_ALL,0);
	setFocus();
}

void MainWin::profileSelected(QAction* act) {
//	emulPause(true,PR_EXTRA);
	setProfile(std::string(act->text().toLocal8Bit().data()));
	sndCalibrate();
	emulUpdateWindow();
	saveProfiles();
//	setFocus();
//	emulPause(false,PR_EXTRA);
}

void MainWin::reset(QAction* act) {
	zxReset(zx,act->data().toInt());
	rzxWin->stop();
}

void MainWin::chLayout(QAction* act) {
//	emulPause(true,PR_EXTRA);
	emulSetLayout(zx,std::string(act->text().toLocal8Bit().data()));
	prfSave("");
	emulUpdateWindow();
//	setFocus();
//	emulPause(false,PR_EXTRA);
}

void MainWin::chVMode(QAction* act) {
	int mode = act->data().toInt();
	if (mode > 0) {
		vidSetMode(zx->vid, mode);
	} else if (mode == -1) {
		vidFlag ^= VF_NOSCREEN;
		act->setChecked(vidFlag & VF_NOSCREEN);
		vidSetMode(zx->vid, VID_CURRENT);
	}
}

// emulation thread (non-GUI)

void emuCycle() {
	zx->frmStrobe = 0;
	do {
		// exec 1 opcode (+ INT, NMI)
		ns += zxExec(zx);
		// if need - request sound buffer update
		if (ns > nsPerSample) {
			sndSync(emulFlags & FL_FAST);
			ns -= nsPerSample;
		}
		// tape trap
		pc = GETPC(zx->cpu);	// z80ex_get_reg(zx->cpu,regPC);
		if ((zx->mem->pt[0]->type == MEM_ROM) && (zx->mem->pt[0]->num == 1)) {
			if (pc == 0x56b) emulTapeCatch();
			if ((pc == 0x5e2) && optGetFlag(OF_TAPEAUTO))
				mainWin->tapStateChanged(TW_STATE,TWS_STOP);
		}
	} while (!zx->brk && (zx->frmStrobe == 0));		// exec until breakpoint or INT

	if (!zx->debug) convImage(zx);

	zx->nmiRequest = 0;
	// decrease frames & screenshot counter (if any), request screenshot (if needed)
	if (scrCounter != 0) {
		if (scrInterval == 0) {
			emulFlags |= FL_SHOT;
			scrCounter--;
			scrInterval = optGetInt(OPT_SHOTINT);
			if (scrCounter == 0) printf("stop combo shots\n");
		} else {
			scrInterval--;
		}
	}
}

void xThread::run() {
	do {
		if (~emulFlags & FL_FAST) mtx.lock();		// wait until unlocked (MainWin::onTimer() or at exit)
		if (emulFlags & FL_EXIT) break;
		if (pauseFlags == 0) {
			emuCycle();
			if (zx->joy->used) {
				leds[1].showTime = 50;
				zx->joy->used = 0;
			}
			if (zx->mouse->used) {
				leds[0].showTime = 50;
				zx->mouse->used = 0;
			}
		}
		if (zx->brk) {
			emit dbgRequest();
			zx->brk = 0;
		}
	} while (1);
	exit(0);
}
