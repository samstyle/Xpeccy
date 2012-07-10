#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QTableWidget>
#include <sys/stat.h>
#include <sys/types.h>
#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "settings.h"
#include "setupwin.h"
#include "emulwin.h"
#include "debuger.h"
#include "develwin.h"
#include "filer.h"

#ifdef HAVESDL
	#include <SDL/SDL.h>
	#include <SDL/SDL_timer.h>
	#include <SDL/SDL_syswm.h>
#endif

//#include "ui_tapewin.h"

#ifdef XQTPAINT
	#include <QPainter>
	QImage scrImg = QImage(100,100,QImage::Format_Indexed8);
#endif

#include <fstream>

#define	XPTITLE	"Xpeccy 0.4.999"

extern EmulWin* mwin;
// main
MainWin* mainWin;
QIcon curicon;
#ifndef XQTPAINT
	SDL_Surface* surf = NULL;
	SDL_Color zxpal[256];
	SDL_SysWMinfo inf;
	SDL_TimerID tid;
	SDL_Joystick* joy = NULL;
#endif
QVector<QRgb> qPal;
int emulFlags;
int pauseFlags;
int wantedWin;
uint32_t scrNumber;
uint32_t scrCounter;
uint32_t scrInterval;
bool breakFrame = false;

// tape player
TapeWin* tapeWin;
// rzx player
RZXWin* rzxWin;
// for user menu
QMenu* userMenu;
QMenu* bookmarkMenu;
QMenu* profileMenu;
// temp emulation
Z80EX_WORD pc,af,de,ix;
int blkDataSize = 0;
uint8_t* blkData = NULL;
int blk;

void emulInit() {
	initKeyMap();
	emulFlags = 0;
	wantedWin = WW_NONE;

	scrNumber = 0;
	scrCounter = 0;
	scrInterval = 0;
	optSet(OPT_SHOTFRM,SCR_PNG);

	addLayout("default",448,320,136,80,64,32,0,64,0);

	emulSetColor(0xc0);
	mainWin = new MainWin;
}

// leds

uint8_t icoBlueDisk[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

uint8_t icoRedDisk[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// KEYMAPS

keyEntry keyMap[256];	// current keymap (init at start from keyMapInit[]

#ifndef XQTPAINT

// nums & small letters = keys
// capital letters:
// E = enter	C = CapsShift	S = SymShift	' ' = Space

#define ENDKEY	SDLK_LAST

keyEntry keyMapInit[] = {
	{"1",SDLK_1,'1',0},{"2",SDLK_2,'2',0},{"3",SDLK_3,'3',0},{"4",SDLK_4,'4',0},{"5",SDLK_5,'5',0},
	{"6",SDLK_6,'6',0},{"7",SDLK_7,'7',0},{"8",SDLK_8,'8',0},{"9",SDLK_9,'9',0},{"0",SDLK_0,'0',0},
	{"Q",SDLK_q,'q',0},{"W",SDLK_w,'w',0},{"E",SDLK_e,'e',0},{"R",SDLK_r,'r',0},{"T",SDLK_t,'t',0},
	{"Y",SDLK_y,'y',0},{"U",SDLK_u,'u',0},{"I",SDLK_i,'i',0},{"O",SDLK_o,'o',0},{"P",SDLK_p,'p',0},
	{"A",SDLK_a,'a',0},{"S",SDLK_s,'s',0},{"D",SDLK_d,'d',0},{"F",SDLK_f,'f',0},{"G",SDLK_g,'g',0},
	{"H",SDLK_h,'h',0},{"J",SDLK_j,'j',0},{"K",SDLK_k,'k',0},{"L",SDLK_l,'l',0},{"ENT",SDLK_RETURN,'E',0},
	{"LS",SDLK_LSHIFT,'C',0},{"Z",SDLK_z,'z',0},{"X",SDLK_x,'x',0},{"C",SDLK_c,'c',0},{"V",SDLK_v,'v',0},
	{"B",SDLK_b,'b',0},{"N",SDLK_n,'n',0},{"M",SDLK_m,'m',0},{"LC",SDLK_LCTRL,'S',0},{"SPC",SDLK_SPACE,' ',0},

	{"RS",SDLK_RSHIFT,'C',0},{"RC",SDLK_RCTRL,'S',0},

	{"`",SDLK_BACKQUOTE,'C','S'},{"\\",SDLK_BACKSLASH,'C','S'},
	{";",SDLK_SEMICOLON,'S','o'},{"\"",SDLK_QUOTE,'S','p'},
	{"TAB",SDLK_TAB,'C',' '},{"CAPS",SDLK_CAPSLOCK,'C','2'},
	{"PGDN",SDLK_PAGEDOWN,'C','3'},{"PGUP",SDLK_PAGEUP,'C','4'},{"BSP",SDLK_BACKSPACE,'C','0'},
	{"DEL",SDLK_DELETE,'C','9'},{"INS",SDLK_INSERT,'S','w'},{"HOME",SDLK_HOME,'S','q'},{"END",SDLK_END,'S','e'},
	{"LEFT",SDLK_LEFT,'C','5'},{"DOWN",SDLK_DOWN,'C','6'},{"UP",SDLK_UP,'C','7'},{"RIGHT",SDLK_RIGHT,'C','8'},
	{"-",SDLK_MINUS,'S','j'},{"+",SDLK_PLUS,'S','k'},{"=",SDLK_EQUALS,'S','l'},
	{",",SDLK_COMMA,'S','n'},{".",SDLK_PERIOD,'S','m'},{"/",SDLK_SLASH,'S','c'},
	{"[",SDLK_LEFTBRACKET,'S','8'},{"]",SDLK_RIGHTBRACKET,'S','9'},
	{"k/",SDLK_KP_DIVIDE,'S','v'},{"k*",SDLK_KP_MULTIPLY,'S','b'},{"k-",SDLK_KP_MINUS,'S','j'},
	{"k+",SDLK_KP_PLUS,'S','k'},{"kENT",SDLK_KP_ENTER,'E',0},{"k.",SDLK_KP_PERIOD,'S','m'},
	{"",ENDKEY,0,0}
};

keyEntry getKeyEntry(SDLKey skey) {
	int idx = 0;
	while ((keyMap[idx].key != SDLK_LAST) && (keyMap[idx].key != skey)) {
		idx++;
	}
	return keyMap[idx];
}

#else

#define ENDKEY 0

keyEntry keyMapInit[] = {
	{"1",XKEY_1,'1',0},{"2",XKEY_2,'2',0},{"3",XKEY_3,'3',0},{"4",XKEY_4,'4',0},{"5",XKEY_5,'5',0},
	{"6",XKEY_6,'6',0},{"7",XKEY_7,'7',0},{"8",XKEY_8,'8',0},{"9",XKEY_9,'9',0},{"0",XKEY_0,'0',0},
	{"Q",XKEY_Q,'q',0},{"W",XKEY_W,'w',0},{"E",XKEY_E,'e',0},{"R",XKEY_R,'r',0},{"T",XKEY_T,'t',0},
	{"Y",XKEY_Y,'y',0},{"U",XKEY_U,'u',0},{"I",XKEY_I,'i',0},{"O",XKEY_O,'o',0},{"P",XKEY_P,'p',0},
	{"A",XKEY_A,'a',0},{"S",XKEY_S,'s',0},{"D",XKEY_D,'d',0},{"F",XKEY_F,'f',0},{"G",XKEY_G,'g',0},
	{"H",XKEY_H,'h',0},{"J",XKEY_J,'j',0},{"K",XKEY_K,'k',0},{"L",XKEY_L,'l',0},{"ENT",XKEY_ENTER,'E',0},
	{"LS",XKEY_LSHIFT,'C',0},{"Z",XKEY_Z,'z',0},{"X",XKEY_X,'x',0},{"C",XKEY_C,'c',0},{"V",XKEY_V,'v',0},
	{"B",XKEY_B,'b',0},{"N",XKEY_N,'n',0},{"M",XKEY_M,'m',0},{"LC",XKEY_LCTRL,'S',0},{"SPC",XKEY_SPACE,' ',0},

	{"`",XKEY_TILDA,'C','S'},{"\\",XKEY_SLASH,'C','S'},
	{";",XKEY_DOTCOM,'S','o'},{"\"",XKEY_QUOTE,'S','p'},
	{"TAB",XKEY_TAB,'C',' '},{"CAPS",XKEY_CAPS,'C','2'},
	{"PGDN",XKEY_PGDN,'C','3'},{"PGUP",XKEY_PGUP,'C','4'},{"BSP",XKEY_BSP,'C','0'},
	{"DEL",XKEY_DEL,'C','9'},{"INS",XKEY_INS,'S','w'},{"HOME",XKEY_HOME,'S','q'},{"END",XKEY_END,'S','e'},
	{"LEFT",XKEY_LEFT,'C','5'},{"DOWN",XKEY_DOWN,'C','6'},{"UP",XKEY_UP,'C','7'},{"RIGHT",XKEY_RIGHT,'C','8'},
	{"-",XKEY_MINUS,'S','j'},{"+",XKEY_PLUS,'S','k'},
	{",",XKEY_PERIOD,'S','n'},{".",XKEY_COMMA,'S','m'},{"/",XKEY_BSLASH,'S','c'},
	{"[",XKEY_LBRACE,'S','8'},{"]",XKEY_RBRACE,'S','9'},

	{"",ENDKEY,0,0}
};

keyEntry getKeyEntry(qint32 qkey) {
	int idx = 0;
	while ((keyMap[idx].key != 0) && (keyMap[idx].key != qkey)) {
		idx++;
	}
	return keyMap[idx];
}

#endif

void setKey(const char* key,const char key1,const char key2) {
	printf("set key\n");
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
	int idx = 0;
	while (keyMapInit[idx].key != ENDKEY) {
		keyMap[idx] = keyMapInit[idx];
		idx++;
	}
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

QWidget* emulWidget() {
	return (QWidget*)mainWin;
}

void emulSetColor(int brl) {
	int i;
	qPal.clear(); qPal.resize(256);
#ifndef XQTPAINT
	for (i=0; i<16; i++) {
		zxpal[i].b = (i & 1) ? ((i & 8) ? 0xff : brl) : 0;
		zxpal[i].r = (i & 2) ? ((i & 8) ? 0xff : brl) : 0;
		zxpal[i].g = (i & 4) ? ((i & 8) ? 0xff : brl) : 0;
	}
	for (i=0; i<256; i++) {
		qPal[i] = qRgb(zxpal[i].r,zxpal[i].g,zxpal[i].b);
	}
#else
	uint8_t r,g,b;
	for (i=0; i<16; i++) {
		b = (i & 1) ? ((i & 8) ? 0xff : brl) : 0;
		r = (i & 2) ? ((i & 8) ? 0xff : brl) : 0;
		g = (i & 4) ? ((i & 8) ? 0xff : brl) : 0;
		qPal[i] = qRgb(r,g,b);
	}
	scrImg.setColorTable(qPal);
#endif
}

void emulUpdateWindow() {
	mainWin->updateWindow();
}

void MainWin::updateHead() {
	QString title(XPTITLE);
	XProfile* curProf = getCurrentProfile();
	if (curProf != NULL) {
		title.append(" | ").append(QString::fromLocal8Bit(curProf->name.c_str()));
	}
	if (emulFlags & FL_FAST) {
		title.append(" | fast");
	}
	setWindowTitle(title);
}

void MainWin::updateWindow() {
	emulFlags |= FL_BLOCK;
	vidUpdate(zx->vid);
	int szw = zx->vid->wsze.h;
	int szh = zx->vid->wsze.v;
	setFixedSize(szw,szh);
#ifdef XQTPAINT
	scrImg = scrImg.scaled(szw,szh);
	zx->vid->scrimg = scrImg.bits();
	zx->vid->scrptr = scrImg.bits();
#else
	int sdlflg = SDL_SWSURFACE;
	if ((vidFlag & VF_FULLSCREEN) && !(vidFlag & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
	}
	if (surf != NULL) {
		surf->pixels = NULL;		// else creating new surface will destroy screenBuf
	}
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg | SDL_NOFRAME);
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
	surf->pixels = vidGetScreen();
	zx->vid->scrimg = (uint8_t*)surf->pixels;
	zx->vid->scrptr = zx->vid->scrimg;
#endif
	zx->vid->firstFrame = true;
	updateHead();
	emulFlags &= ~FL_BLOCK;
}

bool emulSaveChanged() {
	bool yep = saveChangedDisk(0);
	yep &= saveChangedDisk(1);
	yep &= saveChangedDisk(2);
	yep &= saveChangedDisk(3);
	return yep;
}

int emulGetFlags() {
	return emulFlags;
}

void emulSetFlag(int msk,bool cnd) {
	if (cnd) {
		emulFlags |= msk;
	} else {
		emulFlags &= ~msk;
	}
}

double tks = 0;

void emulExec() {
	tks += zxExec(zx);
	tks = sndSync(tks,emulFlags & FL_FAST);
}

void emulSetIcon(const char* inam) {
	curicon = QIcon(QString(inam));
	emulPause(true, 0);
}

void emulPause(bool p, int msk) {
	setFlagBit(p,&pauseFlags,msk);
	bool kk = ((emulFlags & FL_GRAB) != 0);
	if (!kk || ((pauseFlags != 0) && kk)) {
#ifdef XQTPAINT
		mainWin->releaseMouse();
#else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
#endif
	}
	if ((pauseFlags == 0) && kk) {
#ifdef XQTPAINT
		mainWin->grabMouse(QCursor(Qt::BlankCursor));
#else
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(SDL_DISABLE);
#endif
	}
	if (pauseFlags == 0) {
		mainWin->setWindowIcon(curicon);
		sndPause(false);
	} else {
		mainWin->setWindowIcon(QIcon(":/images/pause.png"));
		sndPause(true);
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
	curicon = QIcon(":/images/logo.png");
	setWindowIcon(curicon);
	setAcceptDrops(true);
#ifndef XQTPAINT
	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
	embedClient(inf.info.x11.wmwindow);
#endif
	tapeWin = new TapeWin(this);
	connect(tapeWin,SIGNAL(stateChanged(int,int)),this,SLOT(tapStateChanged(int,int)));

	rzxWin = new RZXWin(this);
	connect(rzxWin,SIGNAL(stateChanged(int)),this,SLOT(rzxStateChanged(int)));

	initUserMenu((QWidget*)this);
	timer = new QTimer();
	connect(timer,SIGNAL(timeout()),this,SLOT(emulFrame()));
}

void MainWin::tapStateChanged(int wut, int val) {
	switch(wut) {
		case TW_STATE:
			switch(val) {
				case TWS_PLAY:
					if (tapPlay(zx->tape)) {
						emulSetIcon(":/images/play.png");
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
					loadFile("",FT_TAPE,-1);
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
			zx->tape->blkData[val].flag ^= TBF_BREAK;
			tapeWin->drawStops(zx->tape);
			break;
	}
}

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
			loadFile("",FT_RZX,0);
			if (zx->rzxSize != 0) {
				rzxWin->startPlay();
			}
			emulPause(false,PR_RZX);
			break;
	}
}

#ifdef XQTPAINT
void MainWin::paintEvent(QPaintEvent *ev) {
	if (emulFlags & FL_BLOCK) return;
	QPainter pnt(this);
	pnt.drawImage(0,0,scrImg);
	pnt.end();
}

void MainWin::keyPressEvent(QKeyEvent *ev) {
	if (ev->modifiers() & Qt::AltModifier) {
		switch(ev->key()) {
			case Qt::Key_0:
				zx->vid->mode = (zx->vid->mode == VID_NORMAL) ? VID_ALCO : VID_NORMAL;
				break;
			case Qt::Key_1:
				vidFlag &= ~VF_DOUBLE;
				mainWin->updateWindow();
				saveConfig();
				break;
			case Qt::Key_2:
				vidFlag |= VF_DOUBLE;
				mainWin->updateWindow();
				saveConfig();
				break;
			case Qt::Key_3: emulFlags ^= FL_FAST;
				mainWin->stopTimer();
				mainWin->startTimer((emulFlags & FL_FAST) ? 1 : 20);
				mainWin->updateHead();
				sndPause((emulFlags & FL_FAST) ? true : false);
				break;
			case Qt::Key_F4:
				mainWin->close();
				break;
			case Qt::Key_F7:
				scrCounter = optGetInt(OPT_SHOTCNT);
				scrInterval=0;
				break;	// ALT+F7 combo
			case Qt::Key_F12:
				zxReset(zx,RES_DOS);
				rzxWin->stop();
				break;
		}
	} else {
		keyEntry kent = getKeyEntry(ev->nativeScanCode());
		keyPress(zx->keyb,kent.key1,kent.key2);
		switch(ev->key()) {
			case Qt::Key_Pause:
				pauseFlags ^= PR_PAUSE;
				emulPause(true,0);
				break;
			case Qt::Key_Escape:
				wantedWin = WW_DEBUG;
				break;
			case Qt::Key_Menu:
				emulPause(true,PR_MENU);
				userMenu->popup(mainWin->pos() + QPoint(20,20));
				break;
			case Qt::Key_F1:
				wantedWin = WW_OPTIONS;
				break;
			case Qt::Key_F2:
				emulPause(true,PR_FILE);
				saveFile("",FT_ALL,-1);
				emulPause(false,PR_FILE);
				break;
			case Qt::Key_F3:
				emulPause(true,PR_FILE);
				loadFile("",FT_ALL,-1);
				emulPause(false,PR_FILE);
				checkState();
				break;
			case Qt::Key_F4:
				if (zx->tape->flag & TAPE_ON) {
					mainWin->tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					mainWin->tapStateChanged(TW_STATE,TWS_PLAY);
				}
				break;
			case Qt::Key_F5:
				if (zx->tape->flag & TAPE_ON) {
					mainWin->tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					mainWin->tapStateChanged(TW_STATE,TWS_REC);
				}
				break;
			case Qt::Key_F6:
				wantedWin = WW_DEVEL;
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
			case Qt::Key_F11:			// TODO: when tapeWin will be working, move it to F4
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
	keyRelease(zx->keyb,kent.key1,kent.key2);
}

void MainWin::mousePressEvent(QMouseEvent *ev){
	switch (ev->button()) {
		case Qt::LeftButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons &= ~0x01;
			break;
		case Qt::RightButton:
			if (emulFlags & FL_GRAB) {
				zx->mouse->buttons &= ~0x02;
			} else {
				emulPause(true,PR_MENU);
				userMenu->popup(QPoint(ev->globalX(),ev->globalY()));
			}
			break;
		default: break;
	}
}

void MainWin::mouseReleaseEvent(QMouseEvent *ev) {
	if (pauseFlags != 0) return;
	switch (ev->button()) {
		case Qt::LeftButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons |= 0x01;
			break;
		case Qt::RightButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons |= 0x02;
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
#ifdef WIN32
		fpath.remove(0,1);	// by some reason path will start with /
#endif
		loadFile(fpath.toUtf8().data(),FT_ALL,0);
	}
}

#endif

void MainWin::closeEvent(QCloseEvent* ev) {
	timer->stop();
	if (emulSaveChanged()) {
		ev->accept();
	} else {
		ev->ignore();
#ifndef XQTPAINT
		SDL_VERSION(&inf.version);
		SDL_GetWMInfo(&inf);
		mainWin->embedClient(inf.info.x11.wmwindow);
#endif
		timer->start(20);
	}
}

void MainWin::startTimer(int iv) {timer->start(iv);}
void MainWin::stopTimer() {timer->stop();}
void MainWin::doOptions() {wantedWin = WW_OPTIONS;}

void MainWin::checkState() {
	if (zx->rzxPlay) rzxWin->startPlay();
	tapeWin->buildList(zx->tape);
	tapeWin->setCheck(zx->tape->block);
}

// ...

uchar hobHead[] = {'s','c','r','e','e','n',' ',' ','C',0,0,0,0x1b,0,0x1b,0xe7,0x81};	// last 2 bytes is crc

void MainWin::emulFrame() {
	if (emulFlags & FL_BLOCK) return;
	breakFrame = false;
	if (!mainWin->isActiveWindow()) {
		keyRelease(zx->keyb,0,0);
		zx->mouse->buttons = 0xff;
	}
	zx->flags = 0;
	if ((wantedWin == WW_NONE) && (pauseFlags == 0)) {
		if (!(emulFlags & FL_FAST) && sndGet(SND_ENABLE) && (sndGet(SND_MUTE) || mainWin->isActiveWindow())) {
			sndPlay();
		}
		sndSet(SND_COUNT,0);
		do {
			emulExec();
			pc = z80ex_get_reg(zx->cpu,regPC);
			if ((pc == 0x56b) && (zx->mem->crom == 1)) {
				blk = zx->tape->block;
				if (blk < zx->tape->blkCount) {
					if (optGetFlag(OF_TAPEFAST) && (zx->tape->blkData[blk].flag & TBF_BYTES)) {
						de = z80ex_get_reg(zx->cpu,regDE);
						ix = z80ex_get_reg(zx->cpu,regIX);
						TapeBlockInfo inf = tapGetBlockInfo(zx->tape,blk);
						blkData = (uint8_t*)realloc(blkData,inf.size + 2);
						tapGetBlockData(zx->tape,blk,blkData);
						if (inf.size == de) {
							for (int i = 0; i < de; i++) {
								memWr(zx->mem,ix,blkData[i + 1]);
								ix++;
							}
							z80ex_set_reg(zx->cpu,regIX,ix);
							z80ex_set_reg(zx->cpu,regDE,0);
							z80ex_set_reg(zx->cpu,regHL,0);
							tapNextBlock(zx->tape);
						} else {
							z80ex_set_reg(zx->cpu,regHL,0xff00);
						}
						z80ex_set_reg(zx->cpu,regPC,0x5df);	// to exit
					} else {
						if (optGetFlag(OF_TAPEAUTO))
							mainWin->tapStateChanged(TW_STATE,TWS_PLAY);
					}
				}
			}
			if ((pc == 0x5e2) && (zx->mem->crom == 1) && (optGetFlag(OF_TAPEAUTO)))
				mainWin->tapStateChanged(TW_STATE,TWS_STOP);
			if (zx->flags & ZX_BREAK) {
				wantedWin = WW_DEBUG;
				zx->flags = 0;
			}
		} while ((wantedWin == WW_NONE) && !zx->intStrobe);
		zx->nmiRequest = false;
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
		if (emulFlags & FL_SHOT) {
			int frm = optGetInt(OPT_SHOTFRM);
			std::string fext;
			switch (frm) {
				case SCR_BMP: fext = "bmp"; break;
				case SCR_JPG: fext = "jpg"; break;
				case SCR_PNG: fext = "png"; break;
				case SCR_SCR: fext = "scr"; break;
				case SCR_HOB: fext = "$C"; break;
			};
			std::string fnam = optGetString(OPT_SHOTDIR) + "/sshot" + int2str(scrNumber) + "." + fext;
			std::ofstream file;
#ifdef XQTPAINT
			QImage *img = new QImage(scrImg);
#else
			QImage *img = new QImage((uchar*)surf->pixels,surf->w,surf->h,QImage::Format_Indexed8);
#endif
			img->setColorTable(qPal);
			char* pageBuf = new char[0x4000];
			memGetPage(zx->mem,MEM_RAM,(zx->vid->curscr == 0) ? 5 : 7,pageBuf);
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
			scrNumber++;
		}
	}
	return;
}

// OBJECT

EmulWin::EmulWin() {
	QObject::connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	QObject::connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	timer = new QTimer;
	QObject::connect(timer,SIGNAL(timeout()),this,SLOT(SDLEventHandler()));
	timer->start(20);
	emulPause(false,-1);
}

int prc;

void putIcon(Video* vid, int x, int y, uint8_t* data) {
	uint8_t* ptr = vid->scrimg + x + y * (vid->wsze.h);
	for (int i = 0; i < 16; i++) {
		memcpy(ptr,data,16);
		ptr += vid->wsze.h;
		data += 16;
	}
}

void EmulWin::SDLEventHandler() {
	// leds
	if (emulFlags & FL_LED_DISK) {
		int fst = zx->bdi->fdc->status;
		switch (fst) {
			case FDC_READ: putIcon(zx->vid,4,4,icoBlueDisk); break;
			case FDC_WRITE: putIcon(zx->vid,4,4,icoRedDisk); break;
		}
	}
#ifndef XQTPAINT
	if (vidFlag & VF_CHANGED) {
		SDL_UpdateRect(surf,0,0,0,0);
		vidFlag &= ~VF_CHANGED;
	} else {
		SDL_UpdateRect(surf,3,3,18,18);
	}
#else
	if (vidFlag & VF_CHANGED) {
		mainWin->update();
		vidFlag &= ~VF_CHANGED;
	} else {
		mainWin->update(3,3,18,18);
	}
#endif
	if (emulFlags & FL_BLOCK) return;
	// wanted windows
	switch (wantedWin) {
		case WW_DEBUG:
			dbgShow();
			wantedWin = WW_NONE;
			break;
		case WW_OPTIONS:
			optShow();
			wantedWin = WW_NONE;
			break;
		case WW_DEVEL:
			devShow();
			wantedWin = WW_NONE;
			break;
	}
	// rzx window
	if (zx->rzxPlay) {
		prc = 100 * zx->rzxFrame / zx->rzxSize;
		rzxWin->setProgress(prc);
	}
	// tape window
	if ((zx->tape->flag & (TAPE_ON | TAPE_REC)) == TAPE_ON) {
		tapeWin->setProgress(tapGetBlockTime(zx->tape,zx->tape->block,zx->tape->pos),tapGetBlockTime(zx->tape,zx->tape->block,-1));
	}
	if (zx->tape->flag & TAPE_BLOCK_CHANGED) {
		if (!(zx->tape->flag & TAPE_ON)) {
			mainWin->tapStateChanged(TW_STATE,TWS_STOP);
		}
		tapeWin->setCheck(zx->tape->block);
		zx->tape->flag &= ~TAPE_BLOCK_CHANGED;
	}
	if (zx->tape->flag & TAPE_NEW_BLOCK) {
		tapeWin->buildList(zx->tape);
		zx->tape->flag &= ~TAPE_NEW_BLOCK;
	}
#ifndef XQTPAINT
	// sdl events
	keyEntry kent;
	int jdir;
	SDL_Event ev;
	intButton intb;
	extButton extb;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			/*
		// BAD NEWS, EVERYONE. SDL 1.2 Hasn't drop event, it appears in SDL 2.0
			case SDL_DROPFILE:
				break;
			*/
			case SDL_KEYDOWN:
				if (ev.key.keysym.mod & KMOD_ALT) {
					switch(ev.key.keysym.sym) {
						case SDLK_0: zx->vid->mode = (zx->vid->mode==VID_NORMAL)?VID_ALCO:VID_NORMAL; break;
						case SDLK_1:
							vidFlag &= ~VF_DOUBLE;
							mainWin->updateWindow();
							saveConfig();
							break;
						case SDLK_2:
							vidFlag |= VF_DOUBLE;
							mainWin->updateWindow();
							saveConfig();
							break;
						case SDLK_3: emulFlags ^= FL_FAST;
							mainWin->stopTimer();
							mainWin->startTimer((emulFlags & FL_FAST) ? 1 : 20);
							mainWin->updateHead();
							sndPause((emulFlags & FL_FAST) ? true : false);
							break;
						case SDLK_F4:
							mainWin->close();
							break;
						case SDLK_F7:
							scrCounter = optGetInt(OPT_SHOTCNT);
							scrInterval=0;
							break;	// ALT+F7 combo
						case SDLK_F12:
							zxReset(zx,RES_DOS);
							rzxWin->stop();
							break;
						case SDLK_RETURN:
							vidFlag ^= VF_FULLSCREEN;
							mainWin->updateWindow();
							saveConfig();
							break;
						default: break;
					}
				} else {
					kent = getKeyEntry(ev.key.keysym.sym);
					keyPress(zx->keyb,kent.key1,kent.key2);
					//keyPress(zx->keyb,ev.key.keysym.scancode);
					switch (ev.key.keysym.sym) {
						case SDLK_PAUSE:
							pauseFlags ^= PR_PAUSE;
							emulPause(true,0);
							break;
						case SDLK_ESCAPE:
							wantedWin = WW_DEBUG;
							break;
						case SDLK_MENU:
							emulPause(true,PR_MENU);
							userMenu->popup(mainWin->pos() + QPoint(20,20));
							break;
						case SDLK_F1:
							wantedWin = WW_OPTIONS;
							break;
						case SDLK_F2:
							emulPause(true,PR_FILE);
							saveFile("",FT_ALL,-1);
							emulPause(false,PR_FILE);
							break;
						case SDLK_F3:
							emulPause(true,PR_FILE);
							loadFile("",FT_ALL,-1);
							emulPause(false,PR_FILE);
							mainWin->checkState();
							break;
						case SDLK_F4:
							if (zx->tape->flag & TAPE_ON) {
								mainWin->tapStateChanged(TW_STATE,TWS_STOP);
							} else {
								mainWin->tapStateChanged(TW_STATE,TWS_PLAY);
							}
							break;
						case SDLK_F5:
							if (zx->tape->flag & TAPE_ON) {
								mainWin->tapStateChanged(TW_STATE,TWS_STOP);
							} else {
								mainWin->tapStateChanged(TW_STATE,TWS_REC);
							}
							break;
						case SDLK_F6:
							wantedWin = WW_DEVEL;
							break;
						case SDLK_F7:
							if (scrCounter == 0) {
								emulFlags |= FL_SHOT;
							} else {
								emulFlags &= ~FL_SHOT;
							} break;
						case SDLK_F8:
							if (rzxWin->isVisible()) {
								rzxWin->hide();
							} else {
								rzxWin->show();
							}
						case SDLK_F9:
							emulPause(true,PR_FILE);
							emulSaveChanged();
							emulPause(false,PR_FILE);
							break;
						case SDLK_F10:
							zx->nmiRequest = true;
							break;
						case SDLK_F11:			// TODO: when tapeWin will be working, move it to F4
							if (tapeWin->isVisible()) {
								tapeWin->hide();
							} else {
								tapeWin->show();
							}
							break;
						case SDLK_F12:
							zxReset(zx,RES_DEFAULT);
							rzxWin->stop();
							break;
						default: break;
					}
				}
				break;
			case SDL_KEYUP:
				kent = getKeyEntry(ev.key.keysym.sym);
				keyRelease(zx->keyb,kent.key1,kent.key2);
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (ev.button.button) {
					if (pauseFlags != 0) break;
					case SDL_BUTTON_LEFT:
						if (emulFlags & FL_GRAB) {
							zx->mouse->buttons &= ~0x01;
						}
						break;
					case SDL_BUTTON_RIGHT:
						if (emulFlags & FL_GRAB) {
							zx->mouse->buttons &= ~0x02;
						} else {
							emulPause(true,PR_MENU);
							userMenu->popup(mainWin->pos() + QPoint(ev.button.x,ev.button.y+20));
						}
						break;
					case SDL_BUTTON_MIDDLE:
						break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (pauseFlags != 0) break;
				switch (ev.button.button) {
					case SDL_BUTTON_LEFT:
						if (emulFlags & FL_GRAB) {
							zx->mouse->buttons |= 0x01;
						}
						break;
					case SDL_BUTTON_RIGHT:
						if (emulFlags & FL_GRAB) {
							zx->mouse->buttons |= 0x02;
						}
						break;
					case SDL_BUTTON_MIDDLE:
						emulFlags ^= FL_GRAB;
						if (emulFlags & FL_GRAB) {
							SDL_WM_GrabInput(SDL_GRAB_ON);
							SDL_ShowCursor(SDL_DISABLE);
						} else {
							SDL_WM_GrabInput(SDL_GRAB_OFF);
							SDL_ShowCursor(SDL_ENABLE);
						}
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				if (!(emulFlags & FL_GRAB) || (pauseFlags !=0 )) break;
				zx->mouse->xpos = (ev.motion.x - 1)&0xff;
				zx->mouse->ypos = (257 - ev.motion.y)&0xff;
				SDL_WarpMouse(zx->mouse->xpos + 1, 257 - zx->mouse->ypos);
				SDL_PeepEvents(&ev,1,SDL_GETEVENT,SDL_EVENTMASK(SDL_MOUSEMOTION));
				break;
			case SDL_JOYBUTTONDOWN:
				extb.type = XJ_BUTTON;
				extb.num = ev.jbutton.button;
				extb.dir = true;
				intb = optGetJMap(extb);
				switch (intb.dev) {
					case XJ_KEY:
						kent = getKeyEntry(intb.name);
						keyPress(zx->keyb,kent.key1,kent.key2);
						break;
					case XJ_JOY:
						jdir = optGetId(OPT_JOYDIRS,intb.name);
						joyPress(zx->joy,jdir);
						break;
				}
				break;
			case SDL_JOYBUTTONUP:
				extb.type = XJ_BUTTON;
				extb.num = ev.jbutton.button;
				extb.dir = true;
				intb = optGetJMap(extb);
				switch (intb.dev) {
					case XJ_KEY:
						kent = getKeyEntry(intb.name);
						keyRelease(zx->keyb,kent.key1,kent.key2);
						break;
					case XJ_JOY:
						jdir = optGetId(OPT_JOYDIRS,intb.name);
						joyRelease(zx->joy,jdir);
						break;
				}
				break;
			case SDL_JOYAXISMOTION:
				extb.type = XJ_AXIS;
				extb.num = ev.jaxis.axis;
				extb.dir = false;
				intb = optGetJMap(extb);
				if (ev.jaxis.value < -5000) {
					switch (intb.dev) {
						case XJ_KEY:
							kent = getKeyEntry(intb.name);
							keyPress(zx->keyb,kent.key1,kent.key2);
							break;
						case XJ_JOY:
							jdir = optGetId(OPT_JOYDIRS,intb.name);
							joyPress(zx->joy,jdir);
							break;
					}
				} else {
					switch (intb.dev) {
						case XJ_KEY:
							kent = getKeyEntry(intb.name);
							keyRelease(zx->keyb,kent.key1,kent.key2);
							break;
						case XJ_JOY:
							jdir = optGetId(OPT_JOYDIRS,intb.name);
							joyRelease(zx->joy,jdir);
							break;
					}
				}
				extb.dir = true;
				intb = optGetJMap(extb);
				if (ev.jaxis.value > 5000) {
					switch (intb.dev) {
						case XJ_KEY:
							kent = getKeyEntry(intb.name);
							keyPress(zx->keyb,kent.key1,kent.key2);
							break;
						case XJ_JOY:
							jdir = optGetId(OPT_JOYDIRS,intb.name);
							joyPress(zx->joy,jdir);
							break;
					}
				} else {
					switch (intb.dev) {
						case XJ_KEY:
							kent = getKeyEntry(intb.name);
							keyRelease(zx->keyb,kent.key1,kent.key2);
							break;
						case XJ_JOY:
							jdir = optGetId(OPT_JOYDIRS,intb.name);
							joyRelease(zx->joy,jdir);
							break;
					}
				}
				break;
		}
	}
#endif
	if (!userMenu->isVisible() && (pauseFlags & PR_MENU)) {
		mainWin->setFocus();
		emulPause(false,PR_MENU);
	}
}

// JOYSTICK

bool emulIsJoystickOpened() {
#ifdef XQTPAINT
	return false;
#else
	return (joy != NULL);
#endif
}

void emulOpenJoystick(std::string name) {
#ifndef XQTPAINT
	emulCloseJoystick();
	int jnums = SDL_NumJoysticks();
	for (int i=0; i<jnums; i++) {
		if (std::string(SDL_JoystickName(i)) == name) {
			joy = SDL_JoystickOpen(i);
			break;
		}
	}
#endif
}

void emulCloseJoystick() {
#ifndef XQTPAINT
	if (joy == NULL) return;
	SDL_JoystickClose(joy);
	joy = NULL;
#endif
}

// USER MENU

void initUserMenu(QWidget* par) {
	userMenu = new QMenu(par);
	bookmarkMenu = userMenu->addMenu(QIcon(":/images/star.png"),"Bookmarks");
	profileMenu = userMenu->addMenu(QIcon(":/images/profile.png"),"Profiles");
	userMenu->addSeparator();
	userMenu->addAction(QIcon(":/images/tape.png"),"Tape window",tapeWin,SLOT(show()));
	userMenu->addAction(QIcon(":/images/video.png"),"RZX player",rzxWin,SLOT(show()));
	userMenu->addSeparator();
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

// SLOTS

void EmulWin::bookmarkSelected(QAction* act) {
	loadFile(act->data().toString().toLocal8Bit().data(),FT_ALL,0);
	mainWin->setFocus();
}

void EmulWin::profileSelected(QAction* act) {
	emulPause(true,PR_EXTRA);
	setProfile(std::string(act->text().toLocal8Bit().data()));
	loadConfig(false);
	emulUpdateWindow();
	saveProfiles();
	if (zx->flags & ZX_JUSTBORN) {
		zxReset(zx,RES_DEFAULT);
		zx->flags &= ~ZX_JUSTBORN;
	}
	mainWin->setFocus();
	emulPause(false,PR_EXTRA);
}
