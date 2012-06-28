#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <getopt.h>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "emulwin.h"
#include "settings.h"
#include "debuger.h"
#include "setupwin.h"
#include "develwin.h"
#include "filer.h"

#ifdef HAVESDL
	#include <SDL/SDL.h>
#endif
#ifdef WIN32
	#undef main
#include <direct.h>
#endif

ZXComp* zx;
EmulWin *mwin;
extern MainWin* mainWin;

int main(int ac,char** av) {
	Z80EX_VERSION* ver = z80ex_get_version();
	printf("Using z80ex ver %d.%d\n",ver->major, ver->minor);
#ifdef XQTPAINT
	printf("Using Qt painter\n");
#else
	printf("Using SDL surface\n");
#endif

#ifdef HAVESDL
	SDL_version sdlver;
	SDL_VERSION(&sdlver);
	printf("Using SDL ver %u.%u.%u\n", sdlver.major, sdlver.minor, sdlver.patch);
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK);
	atexit(SDL_Quit);
#endif
	QApplication app(ac,av,true);
	try {

		int i;
		int p=0;
		bool dev = false;
		initPaths();
		addProfile("default","xpeccy.conf");
		setProfile("default");
		while ((p = getopt(ac,av,"d")) != -1) {
			switch(p) {
				case 'd': dev=true; break;
			}
		}
		if (dev) {
			devInit();
			loadConfig(true);
			devShow();
			return app.exec();
		} else {
			initHardware();
			sndInit();
			emulInit();
			mwin = new EmulWin();
			dbgInit(emulWidget());
			optInit(emulWidget());
			devInit();
			initFileDialog(emulWidget());
			loadProfiles();
			loadConfig(false);
			fillProfileMenu();
			fillBookmarkMenu();
			emulShow();
			emulUpdateWindow();
			zxReset(zx,RES_DEFAULT);

			for(i=1;i<ac;i++) {
				loadFile(av[i],FT_ALL,0);
			}

#ifdef HAVESDL
			SDL_JoystickOpen(1);
#endif
			mainWin->checkState();
			mainWin->startTimer(20);
			app.exec();
			mainWin->stopTimer();
			sndClose();
#ifdef HAVESDLS
			SDL_Quit();
#endif
			return 0;
		}
	}
	catch (const char* s) {
		shitHappens(s);
#ifdef HAVESDL
		SDL_Quit();
#endif
		return 1;
	}
	catch (int i) {
#ifdef HAVESDL
		SDL_Quit();
#endif
		return 1;
	}
}
