#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <vector>
#include <QIcon>
#include <QMenu>
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
#define PR_MENU		0x01
#define PR_FILE		0x02
#define PR_OPTS		0x04
#define PR_DEBUG	0x08
#define PR_QUIT		0x10
#define PR_PAUSE	0x20

// flags - emul mode / events
#define FL_GRAB	0x00000001
#define FL_RZX	0x00000002
#define FL_SHOT	0x00000004

struct RZXFrame {
	int fetches;		// fetches till next int
	std::vector<uint8_t> in;
};

struct EmulProfile {
	std::string name;		// name of profile
	std::string configFileName;	// config file name
	ZXComp* zx;			// zx for emulation
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
		std::vector<RZXFrame> rzx;
		std::vector<EmulProfile> profs;
		SDL_Surface* surf;
		EmulProfile* prof;
		struct {
			std::string sndOutputName;
			std::string scrshotDir,scrshotFormat;
			std::string workDir,romDir,optPath;
		} opt;
		void repause(bool,int);
		void load(std::string,int);
//		void shithappens(const char*);
		void makeBookmarkMenu();
		void addProfile(std::string,std::string);
		bool setProfile(std::string);
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
		QMenu *mainMenu,*bookmarkMenu,*profileMenu;
		QVector<QRgb> pal;
		QIcon curicon;
		void rmksize();
		void paintmain();
		void emitSDLevents();
		int paused;
		int ssbcnt,ssbint,ssnum;
	private slots:
		void actmenu(QAction*);
		void SDLEventHandler();
		void emulframe();
		void updateframe();
	protected:
		void closeEvent(QCloseEvent*);
		
};

// extern EmulWin *mwin;

#endif
