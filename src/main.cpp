#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "emulwin.h"
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
/*
	char buf[8];
	int len = cpuAsm("call z,15", buf, 0);
	printf("%i : ",len);
	char* xptr = buf;
	while (len > 0) {
		printf("%.2X ", *xptr & 0xff);
		xptr++;
		len--;
	}
	printf("\n");
*/
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
	int i;
	MainWin mwin;
	char* parg;
	unsigned char* ptr = NULL;
	int adr = 0x4000;
	i = 1;
	mwin.setProfile("");
	while (i < ac) {
		parg = av[i++];
		if (((strcmp(parg,"-p") == 0) || (strcmp(parg,"--profile") == 0)) && (i < ac)) {
			mwin.setProfile(std::string(av[i]));
			i++;
		} else if ((strcmp(parg,"-d") == 0) || (strcmp(parg,"--debug") == 0)) {
			mwin.doDebug();
		} else if ((strcmp(parg,"--pc") == 0) && (i < ac)) {
			SETPC(mwin.comp->cpu, strtol(av[i],NULL,0));
			i++;
		} else if ((strcmp(parg,"--sp") == 0) && (i < ac)) {
			SETSP(mwin.comp->cpu, strtol(av[i],NULL,0));
			i++;
		} else if (((strcmp(parg,"-b") == 0) || (strcmp(parg,"--bank") == 0)) && (i < ac)) {
			memSetBank(mwin.comp->mem, MEM_BANK3, MEM_RAM, strtol(av[i],NULL,0));
			i++;
		} else if (((strcmp(parg,"-a") == 0) || (strcmp(parg,"--adr") == 0)) && (i < ac)) {
			adr = strtol(av[i],NULL,0) & 0xffff;
			i++;
		} else if (((strcmp(parg,"-f") == 0) || (strcmp(parg,"--file") == 0)) && (i < ac)) {
			loadDUMP(mwin.comp, av[i], adr);
			i++;
		} else if ((strcmp(parg,"--bp") == 0) && (i < ac)) {
			ptr = memGetFptr(mwin.comp->mem, strtol(av[i],NULL,0) & 0xffff);
			*ptr |= MEM_BRK_FETCH;
			i++;
		} else if (((strcmp(parg,"-l") == 0) || (strcmp(parg,"--labels") == 0)) && (i < ac)) {
			mwin.loadLabels(av[i]);
			i++;
		} else if (strlen(parg) > 0) {
			loadFile(mwin.comp, parg, FT_ALL, 0);
		}
	}
	mwin.show();
	mwin.updateWindow();
	mwin.checkState();
	app.exec();
	sndClose();
	return 0;
}
