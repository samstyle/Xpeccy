#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <vector>
#include <QIcon>
#include <QTimer>
//#include <QModelIndex>
#include <stdint.h>
#include <SDL.h>

#ifndef XQTPAINT
	#include <QX11EmbedContainer>
#else
	#include <QWidget>
#endif

#include "libxpeccy/spectrum.h"

// wanted windows
#define	WW_NONE		0
#define	WW_DEBUG	1

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
	std::string name;
	std::string path;
} XBookmark;

typedef struct {
	std::string name;
	std::string file;	// set when romfile is single file
	struct {
		std::string path;
		uint8_t part;
	} roms[32];
} RomSet;

typedef struct {
	std::string name;
	std::string file;
	std::string layName;
	ZXComp* zx;
	RomSet* rset;
} XProfile;

typedef struct {
	std::string name;
	VSize full;
	VSize sync;
	VSize bord;
	VSize intpos;
	int intsz;
} VidLayout;

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
		void tapePlay();
		void tapeRec();
		void tapeStop();
	private slots:
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
		void tapeLoad();
		void tapeRewind(int,int);
		void setTapeStop(int,int);
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
	private:
		QTimer* timer;
	private slots:
		void emulFrame();
		void rzxPlayPause();
		void rzxStop();
		void rzxOpen();
	protected:
		void closeEvent(QCloseEvent*);
#ifdef XQTPAINT
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
// bookmarks
void fillBookmarkMenu();
void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void clearBookmarks();
void swapBookmarks(int,int);
std::vector<XBookmark> getBookmarkList();
int getBookmarksCount();
// profiles
void fillProfileMenu();
void addProfile(std::string,std::string);
bool setProfile(std::string);
void clearProfiles();
std::vector<XProfile> getProfileList();
XProfile* getCurrentProfile();
// hardware
void initHardware();
void setHardware(ZXComp*, std::string);
std::vector<std::string> getHardwareNames();
std::vector<HardWare> getHardwareList();
// romset
bool addRomset(RomSet);
void setRomsetList(std::vector<RomSet>);
void setRomset(ZXComp*, std::string);
std::vector<RomSet> getRomsetList();
// layouts
bool addLayout(std::string,int,int,int,int,int,int,int,int,int);
bool addLayout(VidLayout);
std::vector<VidLayout> getLayoutList();
bool emulSetLayout(Video*, std::string);
// tape window
void buildTapeList();
// joystick
void emulOpenJoystick(std::string);
void emulCloseJoystick();
bool emulIsJoystickOpened();

#endif
