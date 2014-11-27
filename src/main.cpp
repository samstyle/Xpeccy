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
	std::vector<char*> files;
	int i;
	MainWin mwin;
	char* parg;
	i = 1;
	while (i < ac) {
		parg = av[i++];
		if (((strcmp(parg,"-p") == 0) || (strcmp(parg,"--profile") == 0)) && (i < ac)) profName = av[i++];
		else files.push_back(parg);
	}
	if (profName) {
		mwin.setProfile(std::string(profName));
	} else {
		mwin.setProfile("");
	}
	mwin.show();
	mwin.updateWindow();
	zxReset(mwin.comp,RES_DEFAULT);

	for (unsigned idx = 0; idx < files.size(); idx++) {
		if (strlen(files[idx]) > 0) loadFile(mwin.comp,files[idx],FT_ALL,0);
	}

	mwin.checkState();
	app.exec();
	sndClose();
	return 0;
}
