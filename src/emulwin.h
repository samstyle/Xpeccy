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

// Qt nativeScanCode

typedef struct {
	int showTime;	// in 1/50 sec
	int x;
	int y;
	QString imgName;
} xLed;

#ifdef USEOPENGL
#include <QtOpenGL>
class MainWin : public QGLWidget {
#else
class MainWin : public QWidget {
#endif
	Q_OBJECT
	public:
		MainWin();
		~MainWin();
		Computer* comp;
		void checkState();
		void loadLabels(const char*);
		void fillUserMenu();
	signals:
		void s_options(xProfile*);
		void s_debug(Computer*);
		void s_debug_off();
		void s_prf_change(xProfile*);
		void s_gamepad_plug();
		void s_scradr(int, int);

		void s_tape_show();
		void s_tape_progress(Tape*);
		void s_tape_upd(Tape*);

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
		void setProfile(std::string);
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
		unsigned block:1;
//		int relskip;

		QIcon icon;
		int timid;
		int secid;
		int cmsid;

		QTimer frm_tmr;
		int frm_ns;

		int scrCounter;
		int scrInterval;
		int lineBytes;
		int frameBytes;

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

#ifdef USEOPENGL
		bool shd_support;
		unsigned curtex:2;
		GLuint texids[4];
		GLuint curtxid;
		QList<GLuint> queue;
		QGLFormat frmt;
		QGLContext* cont;
		void initializeGL();
		void resizeGL(int,int);
		QGLShaderProgram prg;
		QGLShader* vtx_shd;
		QGLShader* frg_shd;
#endif
};
