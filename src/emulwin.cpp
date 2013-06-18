#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTime>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "settings.h"
#include "emulwin.h"
#include "setupwin.h"
#include "debuger.h"
#include "develwin.h"
#include "filer.h"

#ifdef HAVESDL
	#include <SDL_timer.h>
	#include <SDL_syswm.h>
#endif

//#include "ui_tapewin.h"

#ifdef XQTPAINT
	#include <QPainter>
	QImage scrImg = QImage(100,100,QImage::Format_Indexed8);
#endif

#define	XPTITLE	"Xpeccy 0.5 (20130617)"

// main
MainWin* mainWin;
QIcon curicon;
#ifndef XQTPAINT
	SDL_Surface* surf = NULL;
	SDL_Color zxpal[256];
	SDL_SysWMinfo inf;
	SDL_Joystick* joy = NULL;
#endif
QVector<QRgb> qPal;
volatile int emulFlags = FL_BLOCK;
volatile int pauseFlags;
//int wantedWin;
unsigned int scrCounter;
unsigned int scrInterval;

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
// temp emulation
Z80EX_WORD pc,af,de,ix;
int blkDataSize = 0;
unsigned char* blkData = NULL;
int blk;

int prc;
int ns = 0;				// ns counter

pthread_t emuThread;
sem_t emuSem;
void* emuThreadMain(void*);

void emulInit() {
	initKeyMap();
	emulFlags = 0;
//	wantedWin = WW_NONE;

	scrCounter = 0;
	scrInterval = 0;
	optSet(OPT_SHOTFRM,SCR_PNG);

	addLayout("default",448,320,136,80,64,32,0,0,64);

	sem_init(&emuSem,0,0);

	mainWin = new MainWin;
}

// leds

unsigned char icoBlueDisk[256] = {
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

unsigned char icoRedDisk[256] = {
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
	{"1",SDLK_1,'1',0,0x16},{"2",SDLK_2,'2',0,0x1e},{"3",SDLK_3,'3',0,0x26},{"4",SDLK_4,'4',0,0x25},{"5",SDLK_5,'5',0,0x2e},
	{"6",SDLK_6,'6',0,0x36},{"7",SDLK_7,'7',0,0x3d},{"8",SDLK_8,'8',0,0x3e},{"9",SDLK_9,'9',0,0x46},{"0",SDLK_0,'0',0,0x45},
	{"Q",SDLK_q,'q',0,0x15},{"W",SDLK_w,'w',0,0x1d},{"E",SDLK_e,'e',0,0x24},{"R",SDLK_r,'r',0,0x2d},{"T",SDLK_t,'t',0,0x2c},
	{"Y",SDLK_y,'y',0,0x35},{"U",SDLK_u,'u',0,0x3c},{"I",SDLK_i,'i',0,0x43},{"O",SDLK_o,'o',0,0x44},{"P",SDLK_p,'p',0,0x4d},
	{"A",SDLK_a,'a',0,0x1c},{"S",SDLK_s,'s',0,0x1b},{"D",SDLK_d,'d',0,0x23},{"F",SDLK_f,'f',0,0x2b},{"G",SDLK_g,'g',0,0x34},
	{"H",SDLK_h,'h',0,0x33},{"J",SDLK_j,'j',0,0x3b},{"K",SDLK_k,'k',0,0x42},{"L",SDLK_l,'l',0,0x4b},{"ENT",SDLK_RETURN,'E',0,0x5a},
	{"LS",SDLK_LSHIFT,'C',0,0x00},{"Z",SDLK_z,'z',0,0x1a},{"X",SDLK_x,'x',0,0x22},{"C",SDLK_c,'c',0,0x21},{"V",SDLK_v,'v',0,0x2a},
	{"B",SDLK_b,'b',0,0x32},{"N",SDLK_n,'n',0,0x31},{"M",SDLK_m,'m',0,0x3a},{"LC",SDLK_LCTRL,'S',0,0x00},{"SPC",SDLK_SPACE,' ',0,0x29},

	{"RS",SDLK_RSHIFT,'C',0,0x59},{"RC",SDLK_RCTRL,'S',0,0x14e0},

	{"`",SDLK_BACKQUOTE,'C','S',0x0e},{"\\",SDLK_BACKSLASH,'C','S',0x5d},
	{";",SDLK_SEMICOLON,'S','o',0x4c},{"\"",SDLK_QUOTE,'S','p',0x52},
	{"TAB",SDLK_TAB,'C',' ',0x0d},{"CAPS",SDLK_CAPSLOCK,'C','2',0x58},
	{"PGDN",SDLK_PAGEUP,'C','3',0x7de0},{"PGUP",SDLK_PAGEDOWN,'C','4',0x7ae0},{"BSP",SDLK_BACKSPACE,'C','0',0x66},
	{"DEL",SDLK_DELETE,'C','9',0x71e0},{"INS",SDLK_INSERT,'S','w',0x70e0},{"HOME",SDLK_HOME,'S','q',0x6ce0},{"END",SDLK_END,'S','e',0x69e0},
	{"LEFT",SDLK_LEFT,'C','5',0x6be0},{"DOWN",SDLK_DOWN,'C','6',0x72e0},{"UP",SDLK_UP,'C','7',0x75e0},{"RIGHT",SDLK_RIGHT,'C','8',0x74e0},
	{"-",SDLK_MINUS,'S','j',0x4e},{"+",SDLK_PLUS,'S','k',0x00},{"=",SDLK_EQUALS,'S','l',0x55},
	{",",SDLK_COMMA,'S','n',0x41},{".",SDLK_PERIOD,'S','m',0x49},{"/",SDLK_SLASH,'S','c',0x4a},
	{"[",SDLK_LEFTBRACKET,'S','8',0x54},{"]",SDLK_RIGHTBRACKET,'S','9',0x5b},
	{"k/",SDLK_KP_DIVIDE,'S','v',0x4ae0},{"k*",SDLK_KP_MULTIPLY,'S','b',0x7c},{"k-",SDLK_KP_MINUS,'S','j',0x7b},
	{"k+",SDLK_KP_PLUS,'S','k',0x79},{"kENT",SDLK_KP_ENTER,'E',0,0x5ae0},{"k.",SDLK_KP_PERIOD,'S','m',0x71},
	{"",SDLK_LAST,0,0,0x00}
};

keyEntry getKeyEntry(SDLKey skey) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (keyMap[idx].key != skey)) {
		idx++;
	}
	return keyMap[idx];
}

#else

#define ENDKEY 0

keyEntry keyMapInit[] = {
	{"1",XKEY_1,'1',0,0x16},{"2",XKEY_2,'2',0,0x1e},{"3",XKEY_3,'3',0,0x26},{"4",XKEY_4,'4',0,0x25},{"5",XKEY_5,'5',0,0x2e},
	{"6",XKEY_6,'6',0,0x36},{"7",XKEY_7,'7',0,0x3d},{"8",XKEY_8,'8',0,0x3e},{"9",XKEY_9,'9',0,0x46},{"0",XKEY_0,'0',0,0x45},
	{"Q",XKEY_Q,'q',0,0x15},{"W",XKEY_W,'w',0,0x1d},{"E",XKEY_E,'e',0,0x24},{"R",XKEY_R,'r',0,0x2d},{"T",XKEY_T,'t',0,0x2c},
	{"Y",XKEY_Y,'y',0,0x35},{"U",XKEY_U,'u',0,0x3c},{"I",XKEY_I,'i',0,0x43},{"O",XKEY_O,'o',0,0x44},{"P",XKEY_P,'p',0,0x4d},
	{"A",XKEY_A,'a',0,0x1c},{"S",XKEY_S,'s',0,0x1b},{"D",XKEY_D,'d',0,0x23},{"F",XKEY_F,'f',0,0x2b},{"G",XKEY_G,'g',0,0x34},
	{"H",XKEY_H,'h',0,0x33},{"J",XKEY_J,'j',0,0x3b},{"K",XKEY_K,'k',0,0x42},{"L",XKEY_L,'l',0,0x4b},{"ENT",XKEY_ENTER,'E',0,0x5a},
	{"LS",XKEY_LSHIFT,'C',0,0x00},{"Z",XKEY_Z,'z',0,0x1a},{"X",XKEY_X,'x',0,0x22},{"C",XKEY_C,'c',0,0x21},{"V",XKEY_V,'v',0,0x2a},
	{"B",XKEY_B,'b',0,0x32},{"N",XKEY_N,'n',0,0x31},{"M",XKEY_M,'m',0,0x3a},{"LC",XKEY_LCTRL,'S',0,0x00},{"SPC",XKEY_SPACE,' ',0,0x29},

	{"`",XKEY_TILDA,'C','S',0x0e},{"\\",XKEY_SLASH,'C','S',0x5d},
	{";",XKEY_DOTCOM,'S','o',0x4c},{"\"",XKEY_QUOTE,'S','p',0x52},
	{"TAB",XKEY_TAB,'C',' ',0x0d},{"CAPS",XKEY_CAPS,'C','2',0x58},
	{"PGDN",XKEY_PGUP,'C','3',0x7de0},{"PGUP",XKEY_PGDN,'C','4',0x7ae0},{"BSP",XKEY_BSP,'C','0',0x66},
	{"DEL",XKEY_DEL,'C','9',0x71e0},{"INS",XKEY_INS,'S','w',0x70e0},{"HOME",XKEY_HOME,'S','q',0x6ce0},{"END",XKEY_END,'S','e',0x69e0},
	{"LEFT",XKEY_LEFT,'C','5',0x6be0},{"DOWN",XKEY_DOWN,'C','6',0x72e0},{"UP",XKEY_UP,'C','7',0x75e0},{"RIGHT",XKEY_RIGHT,'C','8',0x74e0},
	{"-",XKEY_MINUS,'S','j',0x4e},{"+",XKEY_PLUS,'S','k',0x00},
	{",",XKEY_PERIOD,'S','n',0x41},{".",XKEY_COMMA,'S','m',0x49},{"/",XKEY_BSLASH,'S','c',0x4a},
	{"[",XKEY_LBRACE,'S','8',0x54},{"]",XKEY_RBRACE,'S','9',0x5b},

	{"",ENDKEY,0,0,0x00}
};

keyEntry getKeyEntry(qint32 qkey) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (keyMap[idx].key != qkey)) {
		idx++;
	}
	return keyMap[idx];
}

#endif

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

QWidget* emulWidget() {
	return (QWidget*)mainWin;
}

unsigned char tslCoLevs[32] = {
	0,11,21,32,42,53,64,74,
	85,95,106,117,127,138,148,159,
	170,180,191,201,212,223,233,244,
	255,255,255,255,255,255,255,255
};

void emulSetPalette(ZXComp* comp,unsigned char lev) {
	int i;
	int fcol;
	unsigned char col;
	unsigned char r[256],g[256],b[256];	// common zx-colors
	if (lev == 0) lev = optGetInt(OPT_BRGLEV);
	qPal.clear();
	qPal.resize(256);
	if (comp->hw->type == HW_TSLAB) {
		for (i = 0; i < 256; i++) {
			fcol = (comp->vid->tsconf.tsAMem[(i << 1) + 1] << 8) | (comp->vid->tsconf.tsAMem[i << 1]);
			r[i] = tslCoLevs[(fcol >> 10) & 0x1f];
			g[i] = tslCoLevs[(fcol >> 5) & 0x1f];
			b[i] = tslCoLevs[fcol & 0x1f];
			qPal[i] = qRgb(r[i],g[i],b[i]);
		}
	} else {
		for(i = 0; i < 16; i++) {
			col = comp->colMap[i];
			b[i] = ((col & 0x10) ? (0xff - lev) : 0x00) + ((col & 0x01) ? lev : 0x00);
			r[i] = ((col & 0x20) ? (0xff - lev) : 0x00) + ((col & 0x02) ? lev : 0x00);
			g[i] = ((col & 0x40) ? (0xff - lev) : 0x00) + ((col & 0x04) ? lev : 0x00);
			if (vidFlag & VF_GREY) {
				col = 0.299 * r[i] + 0.587 * g[i] + 0.114 * b[i];
				r[i] = b[i] = g[i] = col;
			}
		}
		for(i = 0; i < 256; i++) {
			qPal[i] = qRgb((r[i & 0x0f] * 0.5) + (r[(i & 0xf0) >> 4] * 0.5),
				       (g[i & 0x0f] * 0.5) + (g[(i & 0xf0) >> 4] * 0.5),
				       (b[i & 0x0f] * 0.5) + (b[(i & 0xf0) >> 4] * 0.5));
		}
	}
#ifndef XQTPAINT
	for (i=0; i<256; i++) {
		zxpal[i].b = qBlue(qPal[i]);
		zxpal[i].r = qRed(qPal[i]);
		zxpal[i].g = qGreen(qPal[i]);
	}
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
#else
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
#ifdef XQTPAINT
	scrImg = scrImg.scaled(szw,szh);
	zx->vid->scrimg = scrImg.bits();
	zx->vid->scrptr = zx->vid->scrimg;
#else
	int sdlflg = SDL_SWSURFACE | SDL_NOFRAME;
	if ((vidFlag & VF_FULLSCREEN) && !(vidFlag & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
	}
	if (surf) SDL_FreeSurface(surf);
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg);
	zx->vid->scrptr = (unsigned char*)surf->pixels;
	zx->vid->scrimg = zx->vid->scrptr;
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
#endif
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

void emulSetIcon(const char* inam) {
	curicon = QIcon(QString(inam));
	emulPause(true, 0);
}

void emulPause(bool p, int msk) {
	if (p) {
		pauseFlags |= msk;
	} else {
		pauseFlags &= ~msk;
	}
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
	cmosTimer = new QTimer();
	connect(cmosTimer,SIGNAL(timeout()),this,SLOT(cmosTick()));
	cmosTimer->start(1000);

	connect(this,SIGNAL(extSignal(int,int)),SLOT(extSlot(int,int)));
	connect(userMenu,SIGNAL(aboutToHide()),SLOT(menuHide()));

}

void MainWin::menuHide() {
	setFocus();
	emulPause(false,PR_MENU);
}

// show wanted window if any
void MainWin::extSlot(int sig, int par) {
	switch (sig) {
		case EV_WINDOW:
			switch (par) {
				case WW_DEBUG:
					dbgShow();
					break;
				case WW_DEVEL:
					devShow();
					break;
				case WW_OPTIONS:
					optShow();
					break;
				case WW_FOPEN:
					sndPause(true);
					emulPause(true,PR_FILE);
					loadFile(zx,"",FT_ALL,-1);
					emulPause(false,PR_FILE);
					mainWin->checkState();
					sndPause(false);
					break;
				case WW_FSAVE:
					sndPause(true);
					emulPause(true,PR_FILE);
					saveFile(zx,"",FT_ALL,-1);
					emulPause(false,PR_FILE);
					sndPause(false);
					break;
				case WW_SAVECHA:
					sndPause(true);
					emulPause(true,PR_FILE);
					emulSaveChanged();
					emulPause(false,PR_FILE);
					sndPause(false);
					break;
				case WW_RZXPLAYER:
					if (rzxWin->isVisible()) {
						rzxWin->hide();
					} else {
						rzxWin->show();
					}
					break;
				case WW_TAPEPLAYER:
					if (tapeWin->isVisible()) {
						tapeWin->hide();
					} else {
						tapeWin->show();
					}
					break;
				case WW_MENU:
					emulPause(true,PR_MENU);
					userMenu->popup(mainWin->pos() + QPoint(20,20));
					userMenu->setFocus();
					break;
			}
			break;
		case EV_TAPE:
			if (zx->tape->flag & TAPE_ON) {
				mainWin->tapStateChanged(TW_STATE,TWS_STOP);
			} else {
				mainWin->tapStateChanged(TW_STATE,par);
			}
			break;
	}
}

void emuStart() {
	emulFlags |= FL_WORK;
	pthread_create(&emuThread,NULL,&emuThreadMain,NULL);		// emulation thread
//	mainWin->timer->start();
}

void emuStop() {
	emulFlags &= ~FL_WORK;
	sem_post(&emuSem);
	pthread_join(emuThread,NULL);
//	mainWin->timer->stop();
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
			loadFile(zx,"",FT_RZX,0);
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
				//mainWin->updateWindow();
				emulFlags |= FL_UPDATE;
				saveProfiles();
				//saveConfig();
				break;
			case Qt::Key_2:
				vidFlag |= VF_DOUBLE;
				// mainWin->updateWindow();
				emulFlags |= FL_UPDATE;
				saveProfiles();
				//saveConfig();
				break;
			case Qt::Key_3:
				emulFlags ^= FL_FAST_RQ;
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
			case Qt::Key_N:
				vidFlag ^= VF_NOFLIC;
				//saveConfig();
				saveProfiles();
				break;
		}
	} else {
		keyEntry kent = getKeyEntry(ev->nativeScanCode());
		keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
		switch(ev->key()) {
			case Qt::Key_Pause:
				pauseFlags ^= PR_PAUSE;
				emulPause(true,0);
				break;
			case Qt::Key_Escape:
				extSlot(EV_WINDOW,WW_DEBUG);
				break;
			case Qt::Key_Menu:
				emulPause(true,PR_MENU);
				userMenu->popup(mainWin->pos() + QPoint(20,20));
				userMenu->setFocus();
				break;
			case Qt::Key_F1:
				extSlot(EV_WINDOW,WW_OPTIONS);
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
				extSlot(EV_WINDOW,WW_DEVEL);
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
	keyRelease(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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

void MainWin::wheelEvent(QWheelEvent* ev) {
	if ((emulFlags & FL_GRAB) && (zx->mouse->flags & INF_WHEEL)) {
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

#else

#include <fstream>

void doSDLEvents() {
	keyEntry kent;
	int jdir;
	SDL_Event ev;
	intButton intb;
	extButton extb;
#ifdef ISDEBUG
	std::ofstream file;
#endif
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			// BAD NEWS, EVERYONE. SDL 1.2 Hasn't drop event, it appears in SDL 2.0
			case SDL_KEYDOWN:
				if (ev.key.keysym.mod & KMOD_ALT) {
					switch(ev.key.keysym.sym) {
						case SDLK_0:
							switch (zx->vid->vmode) {
								case VID_NORMAL: vidSetMode(zx->vid,VID_ALCO); break;
								case VID_ALCO: vidSetMode(zx->vid,VID_ATM_EGA); break;
								case VID_ATM_EGA: vidSetMode(zx->vid,VID_ATM_TEXT); break;
								case VID_ATM_TEXT: vidSetMode(zx->vid,VID_ATM_HWM); break;
								case VID_ATM_HWM: vidSetMode(zx->vid,VID_NORMAL); break;
							}
							break;
						case SDLK_1:
							vidFlag &= ~VF_DOUBLE;
							mainWin->updateWindow();
							saveProfiles();
							//saveConfig();
							break;
						case SDLK_2:
							vidFlag |= VF_DOUBLE;
							mainWin->updateWindow();
							saveProfiles();
							//saveConfig();
							break;
						case SDLK_3:
							emulFlags ^= FL_FAST;
							break;
						case SDLK_F4:
							mainWin->close();
							break;
						case SDLK_F7:
							scrCounter = optGetInt(OPT_SHOTCNT);
							scrInterval = 0;
							break;	// ALT+F7 combo
						case SDLK_F12:
							zxReset(zx,RES_DOS);
							rzxWin->stop();
							break;
						//case SDLK_RETURN:
						//	vidFlag ^= VF_FULLSCREEN;
						//	mainWin->updateWindow();
							//saveConfig();
						//	saveProfiles();
							break;
						case SDLK_n:
							vidFlag ^= VF_NOFLIC;
							//saveConfig();
							saveProfiles();
							break;
						default: break;
					}
				} else {
					kent = getKeyEntry(ev.key.keysym.sym);
					keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
					switch (ev.key.keysym.sym) {
						case SDLK_PAUSE:
							pauseFlags ^= PR_PAUSE;
							emulPause(true,0);
							break;
						case SDLK_ESCAPE:
							mainWin->sendSignal(EV_WINDOW,WW_DEBUG);
							break;
						case SDLK_MENU:
							mainWin->sendSignal(EV_WINDOW,WW_MENU);
							break;
						case SDLK_F1:
							mainWin->sendSignal(EV_WINDOW,WW_OPTIONS);
							break;
						case SDLK_F2:
							mainWin->sendSignal(EV_WINDOW,WW_FSAVE);
							//wantedWin = WW_FSAVE;
							//emulPause(true,PR_FILE);
							//saveFile(zx,"",FT_ALL,-1);
							//emulPause(false,PR_FILE);
							break;
						case SDLK_F3:
							mainWin->sendSignal(EV_WINDOW,WW_FOPEN);
							//wantedWin = WW_FOPEN;
							//emulPause(true,PR_FILE);
							//loadFile(zx,"",FT_ALL,-1);
							//emulPause(false,PR_FILE);
							//mainWin->checkState();
							break;
						case SDLK_F4:
							mainWin->sendSignal(EV_TAPE,TWS_PLAY);
							//if (zx->tape->flag & TAPE_ON) {
							//	mainWin->tapStateChanged(TW_STATE,TWS_STOP);
							//} else {
							//	mainWin->tapStateChanged(TW_STATE,TWS_PLAY);
							//}
							break;
						case SDLK_F5:
							mainWin->sendSignal(EV_TAPE,TWS_REC);
							//if (zx->tape->flag & TAPE_ON) {
							//	mainWin->tapStateChanged(TW_STATE,TWS_STOP);
							//} else {
							//	mainWin->tapStateChanged(TW_STATE,TWS_REC);
							//}
							break;
						case SDLK_F6:
							mainWin->sendSignal(EV_WINDOW,WW_DEVEL);
							//wantedWin = WW_DEVEL;
							break;
						case SDLK_F7:
							if (scrCounter == 0) {
								emulFlags |= FL_SHOT;
							} else {
								emulFlags &= ~FL_SHOT;
							} break;
						case SDLK_F8:
							mainWin->sendSignal(EV_WINDOW,WW_RZXPLAYER);
							//wantedWin = WW_RZXPLAYER;
						case SDLK_F9:
							mainWin->sendSignal(EV_WINDOW,WW_SAVECHA);
							//wantedWin = WW_SAVECHA;
							break;
						case SDLK_F10:
							zx->nmiRequest = true;
							break;
						case SDLK_F11:
							mainWin->sendSignal(EV_WINDOW,WW_TAPEPLAYER);
							//wantedWin = WW_TAPEPLAYER;
							break;
						case SDLK_F12:
							zxReset(zx,RES_DEFAULT);
							// rzxWin->stop();
							break;
						default: break;
					}
				}
				break;
			case SDL_KEYUP:
				kent = getKeyEntry(ev.key.keysym.sym);
				keyRelease(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
							mainWin->sendSignal(EV_WINDOW,WW_MENU);
							//wantedWin = WW_MENU;
//							emulPause(true,PR_MENU);
//							userMenu->popup(mainWin->pos() + QPoint(ev.button.x,ev.button.y+20));
//							userMenu->setFocus();
						}
						break;
					case SDL_BUTTON_MIDDLE:
						break;
					case SDL_BUTTON_WHEELUP:
						if ((emulFlags & FL_GRAB) && (zx->mouse->flags & INF_WHEEL))
							mouseWheel(zx->mouse,XM_WHEELUP);
						break;
					case SDL_BUTTON_WHEELDOWN:
						if ((emulFlags & FL_GRAB) && (zx->mouse->flags & INF_WHEEL))
							mouseWheel(zx->mouse,XM_WHEELDN);
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
						keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
						keyRelease(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
							keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
							keyRelease(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
							keyPress(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
							keyRelease(zx->keyb,kent.key1,kent.key2,kent.keyCode);
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
}

#endif

void MainWin::closeEvent(QCloseEvent* ev) {
	unsigned int i;
	std::string cmosFile;
	std::vector<XProfile> plist = getProfileList();
//	smpCount = 0;
	sndFillToEnd();
	sndPause(true);
	emuStop();
	for (i = 0; i < plist.size(); i++) {
		prfSave(plist[i].name);
		cmosFile = optGetString(OPT_WORKDIR) + std::string(SLASH) + plist[i].name + std::string(".cmos");
		std::ofstream file(cmosFile.c_str());
		if (file.good()) {
			file.write((const char*)plist[i].zx->cmos.data,256);
			file.close();
		}
	}
	if (emulSaveChanged()) {
		ideCloseFiles(zx->ide);
		sdcCloseFile(zx->sdc);
		ev->accept();
		emulFlags |= FL_EXIT;
	} else {
		ev->ignore();
#ifndef XQTPAINT
		SDL_VERSION(&inf.version);
		SDL_GetWMInfo(&inf);
		mainWin->embedClient(inf.info.x11.wmwindow);
#endif
		sndPause(false);
		emuStart();
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
}

void putIcon(Video* vid, int x, int y, unsigned char* data) {
	unsigned char* ptr = vid->scrimg + x + y * (vid->wsze.h);
	for (int i = 0; i < 16; i++) {
		memcpy(ptr,data,16);
		ptr += vid->wsze.h;
		data += 16;
	}
}

void MainWin::sendSignal(int sig, int par) {
	emit extSignal(sig,par);
}

//void MainWin::emulFrame() {
void emuFrame() {
	if (emulFlags & FL_BLOCK) return;
// if not paused play sound buffer
	if (sndEnabled && (sndMute || mainWin->isActiveWindow())) sndPlay();
// process SDL events
#ifndef XQTPAINT
	doSDLEvents();
#endif
	if (pauseFlags != 0) return;
// take screenshot
	if (emulFlags & FL_SHOT) doScreenShot();
// change palette
	if (zx->flag & ZX_PALCHAN) {
		zx->flag &= ~ZX_PALCHAN;
		emulSetPalette(zx,optGetInt(OPT_BRGLEV));
	}
// if window is not active release keys & buttons
	if (!mainWin->isActiveWindow()) {
		keyRelease(zx->keyb,0,0,0);
		zx->mouse->buttons = 0xff;
	}
// update window
	mainWin->emuDraw();

// set emulation semaphore
	if (~emulFlags & (FL_FAST | FL_EMUL)) sem_post(&emuSem);	// inc 'emulate frame' semaphore
}

// video drawing

void MainWin::emuDraw() {
	if (emulFlags & FL_BLOCK) return;
	emulFlags |= FL_DRAW;
// change palette if need
	if (zx->flag & ZX_PALCHAN) {
		zx->flag &= ~ZX_PALCHAN;
		emulSetPalette(zx,optGetInt(OPT_BRGLEV));
	}
// update rzx window
	if ((zx->rzxPlay) && rzxWin->isVisible()) {
		prc = 100 * zx->rzxFrame / zx->rzxSize;
		rzxWin->setProgress(prc);
	}
// update tape window
	if (tapeWin->isVisible()) {
		if ((zx->tape->flag & (TAPE_ON | TAPE_REC)) == TAPE_ON) {
			tapeWin->setProgress(tapGetBlockTime(zx->tape,zx->tape->block,zx->tape->pos),tapGetBlockTime(zx->tape,zx->tape->block,-1));
		}
		if (zx->tape->flag & TAPE_BLOCK_CHANGED) {
			if (!(zx->tape->flag & TAPE_ON)) {
				tapStateChanged(TW_STATE,TWS_STOP);
			}
			tapeWin->setCheck(zx->tape->block);
			zx->tape->flag &= ~TAPE_BLOCK_CHANGED;
		}
		if (zx->tape->flag & TAPE_NEW_BLOCK) {
			tapeWin->buildList(zx->tape);
			zx->tape->flag &= ~TAPE_NEW_BLOCK;
		}
	}
// put icons
	if (emulFlags & FL_LED_DISK) {
		int fst = zx->bdi->fdc->status;
		switch (fst) {
			case FDC_READ: putIcon(zx->vid,4,4,icoBlueDisk); break;
			case FDC_WRITE: putIcon(zx->vid,4,4,icoRedDisk); break;
		}
	}
//	if (vidFlag & (VF_CHANGED | VF_FRAMEDBG)) {
#ifdef XQTPAINT
		update();
#else
		SDL_UpdateRect(surf,0,0,0,0);
#endif
//	}
//	vidFlag &= ~VF_CHANGED;
//	vidFlag |= VF_CHECKCHA;
	emulFlags &= ~FL_DRAW;
}

void emulTapeCatch() {
	blk = zx->tape->block;
	if (blk >= zx->tape->blkCount) return;
	if (optGetFlag(OF_TAPEFAST) && (zx->tape->blkData[blk].flag & TBF_BYTES)) {
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
	vmodeMenu->addAction("ZX 256 x 192")->setData(VID_NORMAL);
	vmodeMenu->addAction("Alco 16c")->setData(VID_ALCO);
	vmodeMenu->addAction("HW multicolor")->setData(VID_HWMC);
	vmodeMenu->addAction("ATM EGA")->setData(VID_ATM_EGA);
	vmodeMenu->addAction("ATM HW multicolor")->setData(VID_ATM_HWM);
	vmodeMenu->addAction("ATM text")->setData(VID_ATM_TEXT);
	vmodeMenu->addAction("No screen")->setData(VID_NOSCREEN);

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

void MainWin::doOptions() {extSlot(EV_WINDOW,WW_OPTIONS);}

void MainWin::bookmarkSelected(QAction* act) {
	loadFile(zx,act->data().toString().toLocal8Bit().data(),FT_ALL,0);
	setFocus();
}

void MainWin::profileSelected(QAction* act) {
	emulPause(true,PR_EXTRA);
	setProfile(std::string(act->text().toLocal8Bit().data()));
	sndCalibrate();
	emulUpdateWindow();
	saveProfiles();
	setFocus();
	emulPause(false,PR_EXTRA);
}

void MainWin::reset(QAction* act) {
	zxReset(zx,act->data().toInt());
	rzxWin->stop();
}

void MainWin::chLayout(QAction* act) {
	emulPause(true,PR_EXTRA);
	emulSetLayout(zx,std::string(act->text().toLocal8Bit().data()));
	prfSave("");
	emulUpdateWindow();
	setFocus();
	emulPause(false,PR_EXTRA);
}

void MainWin::chVMode(QAction* act) {
	vidSetMode(zx->vid,act->data().toInt());
}

// emulation thread (non-GUI)

/*
// TODO : move here (sndPlay + sem_post) and send signals to main window
Uint32 onTimer(Uint32 itv,void*) {
	if (sndEnabled && (sndMute || mainWin->isActiveWindow())) sndPlay();
	mainWin->emuDraw();
	if (~emulFlags & (FL_FAST | FL_EMUL)) sem_post(&emuSem);	// inc 'emulate frame' semaphore
#ifdef XQTPAINT
//	mainWin->update();
#else
	doSDLEvents();
//	SDL_UpdateRect(surf,0,0,0,0);
#endif
	return itv;
}
*/

void* emuThreadMain(void *) {
//	SDL_TimerID tid = SDL_AddTimer(20,&onTimer,NULL);
	while (emulFlags & FL_WORK) {
		if (~emulFlags & FL_FAST) {
			sem_wait(&emuSem);
			if (~emulFlags & FL_WORK) break;
		}
		emulFlags |= FL_EMUL;
		if (pauseFlags == 0) {
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
				if ((zx->mem->pt0->type == MEM_ROM) && (zx->mem->pt0->num == 1)) {
					if (pc == 0x56b) emulTapeCatch();
					if ((pc == 0x5e2) && optGetFlag(OF_TAPEAUTO))
						mainWin->tapStateChanged(TW_STATE,TWS_STOP);
				}
			} while (!(zx->flag & ZX_BREAK) && (zx->frmStrobe == 0));		// exec until breakpoint or INT
			if (zx->flag & ZX_BREAK) {						// request debug window on breakpoint
				mainWin->sendSignal(EV_WINDOW,WW_DEBUG);
				//wantedWin = WW_DEBUG;
				zx->flag &= ~ZX_BREAK;
			}
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
		emulFlags &= ~FL_EMUL;
	}
//	SDL_RemoveTimer(tid);
	return NULL;
}
