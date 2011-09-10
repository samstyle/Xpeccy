#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <vector>
#include <QIcon>
//#include <QMenu>
#include <QTimer>
#include <stdint.h>

#ifndef WIN32
	#include <QX11EmbedContainer>
#else
	#include <QDialog>
	#include <SDL_syswm.h>
#endif

#include "spectrum.h"

// pause reasons
#define PR_MENU		1
#define PR_FILE		(1<<1)
#define PR_OPTS		(1<<2)
#define PR_DEBUG	(1<<3)
#define PR_QUIT		(1<<4)
#define PR_PAUSE	(1<<5)
#define PR_EXTRA	(1<<6)

// flags - emul mode / events
#define FL_GRAB		1
#define FL_RZX		(1<<1)
#define FL_SHOT		(1<<2)
#define FL_RESET	(1<<3)

struct XBookmark {
	std::string name;
	std::string path;
};

struct XProfile {
	std::string name;
	std::string file;
	ZXComp* zx;
};

#ifndef WIN32
	class EmulWin : public QX11EmbedContainer {
#else
	class EmulWin : public QDialog {
#endif
	Q_OBJECT
	public:
		EmulWin();
		QTimer *tim1,*tim2;
		bool fast;
		int flags;
		uint32_t rfnum;
		uint32_t rfpos;
		SDL_Surface* surf;
		SDL_Color zxpal[256];
		void repause(bool,int);
		void updateWin();
//		void makeBookmarkMenu();
//		void makeProfileMenu();
		void setcuricon(QString);
		void reset();
		void exec();
#ifdef WIN32
		SDL_SysWMinfo inf;
#endif
	signals:
		void icum();
		void onerror();
		void wannasetup();
		void wannadevelop();
	private:
		QVector<QRgb> pal;
		QIcon curicon;
		void rmksize();
		void paintmain();
		void emitSDLevents();
		int paused;
		int ssbcnt,ssbint,ssnum;
	private slots:
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
		void SDLEventHandler();
		void emulframe();
		void updateframe();
	protected:
		void closeEvent(QCloseEvent*);
		
};

void initUserMenu(QWidget*);

void fillBookmarkMenu();
void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void clearBookmarks();
void swapBookmarks(int,int);
std::vector<XBookmark> getBookmarkList();
int getBookmarksCount();

void fillProfileMenu();
void addProfile(std::string,std::string);
bool setProfile(std::string);
void clearProfiles();
std::vector<XProfile> getProfileList();
XProfile* getCurrentProfile();

#endif
