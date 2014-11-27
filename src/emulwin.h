#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QMutex>

#ifdef DRAWGL
	#include <QGLWidget>
#endif
#ifdef DRAWQT
	#include <QWidget>
#endif

#include "setupwin.h"
#include "debuger.h"
#include "xcore/xcore.h"
#include "libxpeccy/spectrum.h"

// pause reasons
#define	PR_MENU		1
#define	PR_FILE		(1<<1)
#define	PR_OPTS		(1<<2)
#define	PR_DEBUG	(1<<3)
#define	PR_QUIT		(1<<4)
#define	PR_PAUSE	(1<<5)
#define	PR_EXTRA	(1<<6)
#define PR_RZX		(1<<7)
#define	PR_EXIT		(1<<8)

// flags - emul mode
#define FL_PAUSE	1
#define	FL_BLOCK	(1<<1)
#define	FL_EXIT		(1<<2)
//#define	FL_LED_MOUSE	(1<<3)
//#define	FL_LED_JOY	(1<<4)
//#define	FL_WORK		(1<<5)
//#define	FL_SYSCLOCK	(1<<6)

// Qt nativeScanCode

#if __linux__

#define	XKEY_1	10
#define	XKEY_2	11
#define	XKEY_3	12
#define	XKEY_4	13
#define	XKEY_5	14
#define	XKEY_6	15
#define	XKEY_7	16
#define	XKEY_8	17
#define	XKEY_9	18
#define	XKEY_0	19
#define	XKEY_MINUS	20
#define	XKEY_PLUS	21
#define	XKEY_BSP	22
#define	XKEY_TAB	23
#define	XKEY_Q	24
#define	XKEY_W	25
#define	XKEY_E	26
#define	XKEY_R	27
#define	XKEY_T	28
#define	XKEY_Y	29
#define	XKEY_U	30
#define	XKEY_I	31
#define	XKEY_O	32
#define	XKEY_P	33
#define	XKEY_LBRACE	34
#define	XKEY_RBRACE	35
#define	XKEY_ENTER	36
#define	XKEY_LCTRL	37
#define	XKEY_A	38
#define	XKEY_S	39
#define	XKEY_D	40
#define	XKEY_F	41
#define	XKEY_G	42
#define	XKEY_H	43
#define	XKEY_J	44
#define	XKEY_K	45
#define	XKEY_L	46
#define	XKEY_DOTCOM	47	// ;
#define	XKEY_QUOTE	48	// "
#define	XKEY_TILDA	49	// ~
#define	XKEY_LSHIFT	50
#define	XKEY_SLASH	51
#define	XKEY_Z	52
#define	XKEY_X	53
#define	XKEY_C	54
#define	XKEY_V	55
#define	XKEY_B	56
#define	XKEY_N	57
#define	XKEY_M	58
#define	XKEY_PERIOD	59
#define	XKEY_COMMA	60
#define	XKEY_BSLASH	61	// /
#define	XKEY_SPACE	65
#define	XKEY_CAPS	66
#define	XKEY_RSHIFT	62
#define	XKEY_RCTRL	105
#define XKEY_LALT	64
#define	XKEY_RALT	108
#define	XKEY_HOME	110
#define	XKEY_UP		111
#define	XKEY_PGUP	112
#define	XKEY_LEFT	113
#define	XKEY_RIGHT	114
#define	XKEY_END	115
#define	XKEY_DOWN	116
#define	XKEY_PGDN	117
#define	XKEY_INS	118
#define	XKEY_DEL	119
#define	XKEY_MENU	135
#define XKEY_ESC	9
#define XKEY_F1		67
#define XKEY_F2		68
#define XKEY_F3		69
#define XKEY_F4		70
#define XKEY_F5		71
#define XKEY_F6		72
#define XKEY_F7		73
#define XKEY_F8		74
#define XKEY_F9		75
#define XKEY_F10	76
#define XKEY_F11	95

#elif _WIN32

#define	XKEY_1	2
#define	XKEY_2	3
#define	XKEY_3	4
#define	XKEY_4	5
#define	XKEY_5	6
#define	XKEY_6	7
#define	XKEY_7	8
#define	XKEY_8	9
#define	XKEY_9	10
#define	XKEY_0	11
#define	XKEY_MINUS	12
#define	XKEY_PLUS	13
#define	XKEY_BSP	14
#define	XKEY_TAB	15
#define	XKEY_Q	16
#define	XKEY_W	17
#define	XKEY_E	18
#define	XKEY_R	19
#define	XKEY_T	20
#define	XKEY_Y	21
#define	XKEY_U	22
#define	XKEY_I	23
#define	XKEY_O	24
#define	XKEY_P	25
#define	XKEY_LBRACE	26
#define	XKEY_RBRACE	27
#define	XKEY_ENTER	28
#define	XKEY_LCTRL	29
#define	XKEY_A	30
#define	XKEY_S	31
#define	XKEY_D	32
#define	XKEY_F	33
#define	XKEY_G	34
#define	XKEY_H	35
#define	XKEY_J	36
#define	XKEY_K	37
#define	XKEY_L	38
#define	XKEY_DOTCOM	39	// ;
#define	XKEY_QUOTE	40	// "
#define	XKEY_TILDA	41	// ~
#define	XKEY_LSHIFT	42
#define	XKEY_SLASH	43
#define	XKEY_Z	44
#define	XKEY_X	45
#define	XKEY_C	46
#define	XKEY_V	47
#define	XKEY_B	48
#define	XKEY_N	49
#define	XKEY_M	50
#define	XKEY_PERIOD	51
#define	XKEY_COMMA	52
#define	XKEY_BSLASH	53	// /
#define	XKEY_RSHIFT	54
#define	XKEY_SPACE	57
#define	XKEY_CAPS	58
#define XKEY_RCTRL	XKEY_LCTRL
#define	XKEY_RALT	312
#define XKEY_LALT	56
#define	XKEY_HOME	327
#define	XKEY_UP		328
#define	XKEY_PGUP	329
#define	XKEY_LEFT	331
#define	XKEY_RIGHT	333
#define	XKEY_END	335
#define	XKEY_DOWN	336
#define	XKEY_PGDN	337
#define	XKEY_INS	338
#define	XKEY_DEL	339
#define	XKEY_MENU	349
#define XKEY_ESC	1
#define XKEY_F1		59
#define XKEY_F2		60
#define XKEY_F3		61
#define XKEY_F4		62
#define XKEY_F5		63
#define XKEY_F6		64
#define XKEY_F7		65
#define XKEY_F8		66
#define XKEY_F9		67
#define XKEY_F10	68
#define XKEY_F11	87

#endif

typedef struct {
	const char* name;
	qint32 key;	// nativeScanCode()
	char key1;
	char key2;
	int keyCode;		// 0xXXYYZZ = ZZ,YY,XX in buffer (ZZ,YY,0xf0,XX if released)
} keyEntry;

typedef struct {
	int showTime;	// in 1/50 sec
	int x;
	int y;
	QString imgName;
} xLed;

class xThread : public QThread {
	Q_OBJECT
	public:
		unsigned fast:1;
		unsigned block:1;
		xConfig* conf;
		ZXComp* comp;
		QMutex mtx;
		void run();
	private:
		void emuCycle();
		void tapeCatch();
	signals:
		void dbgRequest();
		void tapeSignal(int,int);
};

#ifdef DRAWGL
class MainWin : public QGLWidget {
#endif
#ifdef DRAWQT
class MainWin : public QWidget {
#endif
	Q_OBJECT
	public:
		MainWin();
		ZXComp* comp;
		void checkState();
		void setProfile(std::string);
	private:
		unsigned grabMice:1;

		SetupWin* opt;
		DebugWin* dbg;

		QTimer cmosTimer;
		QTimer timer;
		xThread ethread;
		QLabel* keywin;
		QImage scrImg;
#ifdef DRAWGL
		GLuint tex;
		GLuint displaylist;
#endif
		int pauseFlags;
		int scrCounter;
		int scrInterval;

		bool saveChanged();
		void updateHead();
		void emuDraw();
		void screenShot();
	public slots:
		void doOptions();
		void doDebug();
		void updateWindow();
		void pause(bool, int);
		void tapStateChanged(int,int);
	private slots:
		void onTimer();
		void cmosTick();
		void menuHide();
		void menuShow();
		void optApply();
		void dbgReturn();
		void rzxStateChanged(int);
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
		void reset(QAction*);
		void chLayout(QAction*);
		void chVMode(QAction*);
	protected:
		void closeEvent(QCloseEvent*);
#ifdef DRAWGL
		void paintGL();
		void resizeGL(int,int);
#endif
		void dragEnterEvent(QDragEnterEvent*);
		void dropEvent(QDropEvent*);
#ifdef DRAWQT
		void paintEvent(QPaintEvent*);
#endif
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
};

// main
//void emulInit();
//void emuStart();
//void emuStop();
//void emulShow();
//void emulUpdateWindow();
//void emulPause(bool, int);
//extern volatile int emulFlags;
//void emulSetFlag(int,bool);

// keys
void initKeyMap();
void setKey(const char*,const char,const char);
// USER MENU
void initUserMenu(QWidget*);
void fillUserMenu();
void fillProfileMenu();
// joystick
//void emulOpenJoystick(std::string);
//void emulCloseJoystick();
//bool emulIsJoystickOpened();

#endif
