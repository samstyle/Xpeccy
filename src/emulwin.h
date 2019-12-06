#ifndef X_EMULWIN_H
#define X_EMULWIN_H

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QString>

#ifdef USENETWORK
#include <QTcpServer>
#include <QTcpSocket>
#endif

#ifdef HAVESDL2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

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
		void s_prf_change(xProfile*);

		void s_tape_show();
		void s_tape_progress(Tape*);
		void s_tape_upd(Tape*);

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
	private slots:
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
		void connected();
		void disconnected();
		void socketRead();
		void saveVRAM();
		void saveGBVRAM();
		void saveNESPPU();
		void debugAction();
	private:
		unsigned grabMice:1;
		unsigned block:1;

		QIcon icon;
		int timid;
		int secid;

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

		void mapJoystick(Computer*, int, int, int);
		void mapPress(Computer*, xJoyMapEntry);
		void mapRelease(Computer*, xJoyMapEntry);

		QMenu* userMenu;
		QMenu* bookmarkMenu;
		QMenu* profileMenu;
		QMenu* layoutMenu;
		QMenu* resMenu;
		QMenu* fileMenu;
#ifdef ISDEBUG
		QMenu* dbgMenu;
#endif
		QAction* pckAct;

		void initUserMenu();
		void fillUserMenu();
		void fillProfileMenu();
		void fillBookmarkMenu();
		void fillLayoutMenu();

		void xkey_press(int, Qt::KeyboardModifiers = Qt::NoModifier);
		void xkey_release(int, Qt::KeyboardModifiers = Qt::NoModifier);

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
		void timerEvent(QTimerEvent*);
};

#endif
