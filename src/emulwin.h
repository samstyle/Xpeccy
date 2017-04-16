#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWidget>

#include "libxpeccy/spectrum.h"
#include "setupwin.h"
#include "debuger.h"
#include "watcher.h"
#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "ethread.h"

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

// Qt nativeScanCode

typedef struct {
	int showTime;	// in 1/50 sec
	int x;
	int y;
	QString imgName;
} xLed;

class keyWindow : public QLabel {
	Q_OBJECT
	public:
		keyWindow(QWidget* = NULL);
		Keyboard* kb;
	private:
		xKey xk;
	protected:
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
};

class MainWin : public QWidget {
	Q_OBJECT
	public:
		MainWin();
		Computer* comp;
		void checkState();
		void setProfile(std::string);
		void loadLabels(const char*);
	private:
		unsigned grabMice:1;
		unsigned block:1;

		SetupWin* opt;
		DebugWin* dbg;
		TapeWin* tapeWin;
		RZXWin* rzxWin;
		xWatcher* watcher;

		QIcon icon;
		QTimer cmosTimer;
		QTimer timer;
		xThread ethread;
		keyWindow* keywin;
		QImage scrImg;
		QByteArray font;

		int pauseFlags;
		int scrCounter;
		int scrInterval;
		int lineBytes;
		int frameBytes;

		int msgTimer;
		QString msg;
		void drawMessage();
		void setMessage(QString, double = 2.0);

		bool saveChanged();
		void updateHead();
		void screenShot();
		void putLeds();
		void convImage();

		QMenu* userMenu;
		QMenu* bookmarkMenu;
		QMenu* profileMenu;
		QMenu* layoutMenu;
		QMenu* vmodeMenu;
		QMenu* resMenu;
		QMenu* fileMenu;
#ifdef ISDEBUG
		QMenu* dbgMenu;
#endif
		QAction* pckAct;
		QAction* nsAct;

		void initUserMenu();
		void fillUserMenu();
		void fillProfileMenu();
		void fillBookmarkMenu();
		void fillLayoutMenu();
	public slots:
		void doOptions();
		void doDebug();
		void updateWindow();
		void pause(bool, int);
		void tapStateChanged(int,int);
	private slots:
		void onTimer();
		void emuDraw();
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
		void umOpen(QAction*);

		void saveVRAM();
		void saveGBVRAM();
		void saveGSRAM();
		void debugAction();
	protected:
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
};

#endif
