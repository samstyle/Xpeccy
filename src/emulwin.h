#ifndef X_EMULWIN_H
#define X_EMULWIN_H

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QString>

#include <SDL.h>

#include "xcore.h"
#include "xgui.h"
#include "spectrum.h"
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
		void loadLabels(const char*);
	signals:
		void s_options(xProfile*);
		void s_debug(Computer*);
//		void s_labels(QString);

		void s_tape_show();
		void s_tape_progress(Tape*);
		void s_tape_upd(Tape*);

		void s_rzx_start();
		void s_rzx_stop();
		void s_rzx_upd(Computer*);
		void s_rzx_show();
		void s_watch_upd(Computer*);
		void s_watch_show();
		void s_keywin_upd(Keyboard*);
		void s_keywin_shide();
		void s_keywin_close();
	public slots:
		void d_frame();
		void doOptions();
		void doDebug();
		void updateWindow();
		void pause(bool, int);
		void tapStateChanged(int,int);
		void setProfile(std::string);
	private slots:
		void onTimer();
		void updateSatellites();
		void menuHide();
		void menuShow();
		void optApply();
		void dbgReturn();
		void rzxStateChanged(int);
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
		void reset(QAction*);
		void chLayout(QAction*);
		void umOpen(QAction*);

		void saveVRAM();
		void saveGBVRAM();
		void saveNESPPU();
		void debugAction();
	private:
		unsigned grabMice:1;
		unsigned block:1;

		QIcon icon;
		QTimer timer;

		int scrCounter;
		int scrInterval;
		int lineBytes;
		int frameBytes;

		int msgTimer;
		QString msg;
		void setMessage(QString, double = 2.0);

		bool saveChanged();
		void updateHead();
		void screenShot();
//		void putLeds();

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
		void focusOutEvent(QFocusEvent*);
};

#endif
