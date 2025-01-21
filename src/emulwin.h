#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QString>

#ifdef USENETWORK
#include <QTcpServer>
#include <QTcpSocket>
#endif

#include <SDL.h>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "watcher.h"
#include "vkeyboard.h"
#include "ethread.h"

#if USE_QT_GAMEPAD
#include <QGamepad>
#include <QGamepadManager>
#endif

// for windows
#define STICKY_KEY 1

enum {
	led_kbd = 0,
	led_joy,
	led_mouse,
	led_tap_red,
	led_tap_yellow,
	led_disk_green,
	led_disk_red,
	led_wav,
	leds_count
};

typedef struct {
	int showTime;	// in 1/50 sec
	int x;
	int y;
	QString imgName;
} xLed;

// QOpenGLWidget since Qt5.4

#define BLOCKGL 0
#define USELEGACYGL 0
#define ISLEGACYGL ((QT_VERSION < QT_VERSION_CHECK(5,4,0)) || (USELEGACYGL && (QT_VERSION < QT_VERSION_CHECK(6,0,0))))

#ifdef USEOPENGL
	#include <QtOpenGL>

	#if ISLEGACYGL
		class MainWin : public QGLWidget {
	#else
		#include <QOpenGLWidget>
		class MainWin : public QOpenGLWidget, protected QOpenGLFunctions {
	#endif
#else
	class MainWin : public QWidget {
#endif
	Q_OBJECT
	public:
		MainWin();
		~MainWin();
//		Computer* comp;
		void checkState();
		void loadLabels(const char*);
		void fillUserMenu();
	signals:
		void s_options();
		void s_debug();
		void s_debug_off();
		// void s_prf_change(xProfile*);
		void s_gamepad_plug();
		void s_scradr(int, int);

		void s_tape_show();
		void s_tape_progress(Tape*);
		void s_tape_upd(Tape*);
		void s_tape_blk(Tape*);

		void s_step();

		void s_rzx_start();
		void s_rzx_stop();
		void s_rzx_upd(Computer*);
		void s_rzx_show();
		void s_watch_upd(Computer*);
		void s_watch_show();
		void s_keywin_rall(Keyboard*);
		void s_keywin_upd(Keyboard*);
		void s_keywin_shide();
		void s_keywin_close();
		void s_emulwin_close();
	public slots:
		void d_frame();
		void doOptions();
		void doDebug();
		void updateWindow();
		void pause(bool, int);
		void tapStateChanged(int,int);
		void onPrfChange();
		void kPress(QKeyEvent*);
		void kRelease(QKeyEvent*);
		void loadShader();
	private slots:
		void updateSatellites();
		void menuHide();
		void menuShow();
		void optApply();
		void dbgReturn();
		void rzxStateChanged(int);
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
		void shdSelected(QAction*);
		void keySelected(QAction*);
		void reset(QAction*);
		void chLayout(QAction*);
		void umOpen(QAction*);
		void gpButtonChanged(int, bool);	// need xGamepad on SDL
		void gpAxisChanged(int, double);
		void connected();
		void disconnected();
		void socketRead();

		void saveVRAM();
		void saveGBVRAM();
		void saveNESPPU();
		void debugAction();
		void frame_timer();
	private:
		unsigned grabMice:1;
		unsigned dumove:1;
		unsigned block:1;

		QIcon icon;
		int timid;
		int secid;
		int cmsid;
		QImage leds[leds_count];

		QTimer frm_tmr;
		int frm_ns;

		int scrCounter;
		int scrInterval;

		QImage alphabet;
		void drawText(QPainter*, int, int, const char*);

		#ifdef USENETWORK
		QTcpServer srv;
		QList<QTcpSocket*> clients;
		#endif

		int msgTimer;
		QString msg;
		void setMessage(QString, double = 2.0);

		bool saveChanged();
		void updateHead();
		void screenShot();
		void drawIcons(QPainter&);

#if USE_QT_GAMEPAD
		QGamepadManager* gpadmgr;
#endif
		void mapJoystick(Computer*, int, int, int);
		void mapPress(Computer*, xJoyMapEntry);
		void mapRelease(Computer*, xJoyMapEntry);
#ifdef USENETWORK
		void openServer();
		void closeServer();
#endif
		QMenu* userMenu;
		QMenu* bookmarkMenu;
		QMenu* profileMenu;
		QMenu* layoutMenu;
		QMenu* resMenu;
		QMenu* fileMenu;
		QMenu* shdMenu;
		QMenu* keyMenu;
#ifdef ISDEBUG
		QMenu* dbgMenu;
#endif
		QAction* pckAct;

		void initUserMenu();
		void calcCoords(QMouseEvent*);

		void xkey_press(int);
		void xkey_release(int);

		void closeEvent(QCloseEvent*);
		void dragEnterEvent(QDragEnterEvent*);
		void dropEvent(QDropEvent*);
		void paintEvent(QPaintEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void focusOutEvent(QFocusEvent*);
		void focusInEvent(QFocusEvent*);
		void timerEvent(QTimerEvent*);
		void moveEvent(QMoveEvent*);
#if defined(USEOPENGL) && !BLOCKGL
		unsigned curtex:2;
		GLuint texids[4];
		GLuint curtxid;
		QList<GLuint> queue;
		void initializeGL();
		void resizeGL(int,int);
		void paintGL();
#if ISLEGACYGL
		QGLContext* cont;
		QGLShaderProgram prg;
		QGLShader* vtx_shd;
		QGLShader* frg_shd;
#else
		QOpenGLShaderProgram prg;
		QOpenGLShader* vtx_shd;
		QOpenGLShader* frg_shd;
#endif
#endif
};
