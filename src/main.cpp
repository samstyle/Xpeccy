#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QFontDatabase>

#include "xcore/xcore.h"
#include "xcore/sound.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "emulwin.h"
#include "debuger.h"
#include "setupwin.h"
#include "filer.h"

#ifdef HAVESDL
	#include <SDL.h>
	#undef main
#endif
#ifdef _WIN32
	#include <mmsystem.h>
#endif

int main(int ac,char** av) {

#ifdef HAVESDL
	SDL_Init(SDL_INIT_AUDIO);
	atexit(SDL_Quit);
	SDL_version sdlver;
	SDL_VERSION(&sdlver);
	printf("Using SDL ver %u.%u.%u\n", sdlver.major, sdlver.minor, sdlver.patch);
#endif
#ifdef HAVEALSA
	printf("Using ALSA ver %s\n",SND_LIB_VERSION_STR);
#endif
#ifdef HAVEZLIB
	printf("Using ZLIB ver %s\n",ZLIB_VERSION);
#endif
	printf("Using Qt ver %s\n",qVersion());

	QApplication app(ac,av,true);


	vidInitAdrs();
	sndInit();
	initPaths();
	addProfile("default","xpeccy.conf");

	QFontDatabase::addApplicationFont("://DejaVuSansMono.ttf");

	int i;
	MainWin mwin;
	char* parg;
	unsigned char* ptr = NULL;
	int adr = 0x4000;
	i = 1;
	mwin.setProfile("");
	int dbg = 0;
	while (i < ac) {
		parg = av[i++];
		if (((strcmp(parg,"-p") == 0) || (strcmp(parg,"--profile") == 0)) && (i < ac)) {
			mwin.setProfile(std::string(av[i]));
			i++;
		} else if ((strcmp(parg,"-d") == 0) || (strcmp(parg,"--debug") == 0)) {
			dbg = 1;
		} else if (i < ac) {
			if (!strcmp(parg,"--pc")) {
				mwin.comp->cpu->pc = strtol(av[i],NULL,0);
				i++;
			} else if (!strcmp(parg,"--sp")) {
				mwin.comp->cpu->sp = strtol(av[i],NULL,0);
				i++;
			} else if (!strcmp(parg,"-b") || !strcmp(parg,"--bank")) {
				memSetBank(mwin.comp->mem, MEM_BANK3, MEM_RAM, strtol(av[i],NULL,0), NULL, NULL, NULL);
				i++;
			} else if (!strcmp(parg,"-a") || !strcmp(parg,"--adr")) {
				adr = strtol(av[i],NULL,0) & 0xffff;
				i++;
			} else if (!strcmp(parg,"-f") || !strcmp(parg,"--file")) {
				loadDUMP(mwin.comp, av[i], adr);
				i++;
			} else if (!strcmp(parg,"--bp")) {
				ptr = getBrkPtr(mwin.comp, strtol(av[i],NULL,0) & 0xffff);
				*ptr |= MEM_BRK_FETCH;
				i++;
			} else if (!strcmp(parg,"-l") || !strcmp(parg,"--labels")) {
				mwin.loadLabels(av[i]);
				i++;
			} else if (strlen(parg) > 0) {
				loadFile(mwin.comp, parg, FT_ALL, 0);
			}
		} else if (strlen(parg) > 0) {
			loadFile(mwin.comp, parg, FT_ALL, 0);
		}
	}
	if (dbg) mwin.doDebug();
	mwin.show();
	mwin.updateWindow();
	mwin.checkState();
	app.exec();
	sndClose();
	return 0;
}
