#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "emulwin.h"
#include "settings.h"
#include "debuger.h"
#include "setupwin.h"
#include "filer.h"

#ifdef HAVESDL
	#include <SDL.h>
	#undef main
#endif
#ifdef _WIN32
#endif

ZXComp* zx;
extern MainWin* mainWin;
//extern volatile int pauseFlags;

int main(int ac,char** av) {

#ifdef DRAWGL
	printf("Using OpenGL painter\n");
#endif
#ifdef DRAWQT
	printf("Using Qt painter\n");
#endif

#ifdef HAVESDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK);
	atexit(SDL_Quit);
	SDL_version sdlver;
	SDL_VERSION(&sdlver);
	printf("Using SDL ver %u.%u.%u\n", sdlver.major, sdlver.minor, sdlver.patch);
#endif
	printf("Using Qt ver %s\n",qVersion());
#ifdef SELFZ80
	printf("Using my Z80\n");
#else
	Z80EX_VERSION* ver = z80ex_get_version();
	printf("Using z80ex ver %d.%d\n",ver->major, ver->minor);
#endif
	QApplication app(ac,av,true);
	char* profName = NULL;
	try {
		std::vector<char*> files;
		int i;
		initPaths();
		addProfile("default","xpeccy.conf");
		setProfile("default");
		char* parg;
		i = 1;
		while (i < ac) {
			parg = av[i++];
			if (((strcmp(parg,"-p") == 0) || (strcmp(parg,"--profile") == 0)) && (i < ac)) profName = av[i++];
			else files.push_back(parg);
		}
		sndInit();
		emulInit();
		dbgInit(NULL);
		loadProfiles();
		if (profName) {
			setProfile(std::string(profName));
		}
		fillUserMenu();
		emulShow();
		emulUpdateWindow();
		zxReset(zx,RES_DEFAULT);

		for (unsigned idx = 0; idx < files.size(); idx++) {
			if (strlen(files[idx]) > 0) loadFile(zx,files[idx],FT_ALL,0);
		}

		mainWin->checkState();

		emuStart();
		app.exec();
		emuStop();
		sndClose();
		return 0;
	}
	catch (const char* s) {
		shitHappens(s);
		return 1;
	}
	catch (int i) {
		return 1;
	}
}
