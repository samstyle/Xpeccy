#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <QLabel>
#include <QTimer>
#include <QWidget>

#include <SDL.h>

#include "xcore.h"
#include "xgui.h"
#include "spectrum.h"
#include "setupwin.h"
#include "debuger.h"
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
		QTimer timer;
		xThread ethread;
		keyWindow* keywin;
		QImage scrImg;
		QByteArray font;

//		int pauseFlags;
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

		void mapJoystick(Computer*, int, int, int);
		void mapPress(Computer*, xJoyMapEntry);
		void mapRelease(Computer*, xJoyMapEntry);

		QMenu* userMenu;
		QMenu* bookmarkMenu;
		QMenu* profileMenu;
		QMenu* layoutMenu;
//		QMenu* vmodeMenu;
		QMenu* resMenu;
		QMenu* fileMenu;
#ifdef ISDEBUG
		QMenu* dbgMenu;
#endif
		QAction* pckAct;
//		QAction* nsAct;

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
		void convImage();
		void updateSatellites();
//		void cmosTick();
		void menuHide();
		void menuShow();
		void optApply();
		void dbgReturn();
		void rzxStateChanged(int);
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
		void reset(QAction*);
		void chLayout(QAction*);
		// void chVMode(QAction*);
		void umOpen(QAction*);

		void saveVRAM();
		void saveGBVRAM();
		//void saveGSRAM();
		void saveNESPPU();
		void debugAction();
	private:
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
};

#endif
