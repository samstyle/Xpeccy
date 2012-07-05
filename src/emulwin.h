#ifndef _EMULWIN_H
#define _EMULWIN_H

//#include <vector>
//#include <QIcon>
#include <QTimer>
#include <stdint.h>
#ifdef HAVESDL
	#include <SDL/SDL.h>
#endif

#ifndef XQTPAINT
	#include <QX11EmbedContainer>
#else
	#include <QWidget>
#endif

#include "libxpeccy/spectrum.h"

// wanted windows
#define	WW_NONE		0
#define	WW_DEBUG	1
#define WW_OPTIONS	2
#define	WW_DEVEL	3

// pause reasons
#define	PR_MENU		1
#define	PR_FILE		(1<<1)
#define	PR_OPTS		(1<<2)
#define	PR_DEBUG	(1<<3)
#define	PR_QUIT		(1<<4)
#define	PR_PAUSE	(1<<5)
#define	PR_EXTRA	(1<<6)
#define PR_RZX		(1<<7)

// flags - emul mode / events
#define	FL_GRAB		1
#define	FL_RZX		(1<<1)
#define	FL_SHOT		(1<<2)
#define	FL_RESET	(1<<3)
#define	FL_FAST		(1<<4)
#define	FL_BLOCK	(1<<5)
#define	FL_EXIT		(1<<6)
#define	FL_LED_DISK	(1<<7)
#define	FL_LED_SHOT	(1<<8)

typedef struct {
	const char* name;
#ifdef XQTPAINT
	Qt::Key key;
#else
	SDLKey key;
#endif
	char key1;
	char key2;
} keyEntry;

// TODO: kill EmulWin class?

class EmulWin : public QObject {
	Q_OBJECT
	public:
		EmulWin();
	private:
		QTimer *timer;
	public slots:
	private slots:
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
	public slots:
		void SDLEventHandler();
};

#ifdef XQTPAINT
class MainWin : public QWidget {
#else
class MainWin : public QX11EmbedContainer {
#endif
	Q_OBJECT
	public:
		MainWin();
		void updateWindow();
		void startTimer(int);
		void stopTimer();
		void checkState();
	private:
		QTimer* timer;
	public slots:
		void tapStateChanged(int,int);
	private slots:
		void emulFrame();
		void rzxStateChanged(int);
	protected:
		void closeEvent(QCloseEvent*);
#ifdef XQTPAINT
		void dragEnterEvent(QDragEnterEvent*);
		void dropEvent(QDropEvent*);
		void paintEvent(QPaintEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
#endif
};

// main
void emulInit();
void emulShow();
void emulUpdateWindow();
void emulSetIcon(const char*);
void emulPause(bool, int);
int emulGetFlags();
void emulSetFlag(int,bool);
bool emulSaveChanged();
void emulExec();
QWidget* emulWidget();
void emulSetColor(int);

// keys
void initKeyMap();
void setKey(const char*,const char,const char);
#ifndef XQTPAINT
	keyEntry getKeyEntry(SDLKey);
#endif
// USER MENU
void initUserMenu(QWidget*);
void fillBookmarkMenu();
void fillProfileMenu();
// bookmarks : moved to xcore/bookmarks
// profiles : moved to xcore/profiles
// hardware : moved to xcore/hardwares
// romset : moved to xcore/romsets
// layouts : moved to xcore/layouts
// tape window : moved to xgui/tapewin
// joystick
void emulOpenJoystick(std::string);
void emulCloseJoystick();
bool emulIsJoystickOpened();

#endif
