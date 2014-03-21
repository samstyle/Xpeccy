#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "emulwin.h"
#include "settings.h"
#include "debuger.h"
#include "setupwin.h"
#include "sdkwin.h"
#include "filer.h"

#ifdef HAVESDL
	#include <SDL.h>
	#undef main
#endif
#ifdef _WIN32
#endif

ZXComp* zx;
extern MainWin* mainWin;
extern volatile int pauseFlags;

#ifdef __linux
	#include <pthread.h>
	#include <semaphore.h>
	extern sem_t emuSem;
	extern pthread_t emuThread;
	sem_t eventSem;
	void* emuThreadMain(void*);

Uint32 onTimer(Uint32 itv, void*) {
	if (~pauseFlags & PR_FILE) sem_post(&eventSem);
	return itv;
}
#elif __WIN32
	#include <windows.h>
//	#include <direct.h>
	extern HANDLE emuThread;
	extern HANDLE emuSem;
	HANDLE eventSem;
	extern DWORD WINAPI emuThreadMain(LPVOID);

Uint32 onTimer(Uint32, void *) {
	if (~pauseFlags & PR_FILE) {
		ReleaseSemaphore(eventSem,1,NULL);
	}
	return 20;
}
#endif

int main(int ac,char** av) {

#ifdef XQTPAINT
	printf("Using Qt painter\n");
#else
	printf("Using SDL surface\n");
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
		int i;
		bool dev = false;
		initPaths();
		addProfile("default","xpeccy.conf");
		setProfile("default");
		for (int i = 1; i < ac; i++) {
			if (strcmp(av[i],"-d") == 0) dev = true;
			if ((strcmp(av[i],"-p") == 0) && (i < (ac - 1))) {
				profName = av[i + 1];
				i++;
			}
		}
		if (dev) {
			devInit();
			devShow();
			return app.exec();
		} else {
#ifdef __linux
			sem_init(&emuSem,1,0);
			sem_init(&eventSem,1,0);
			pthread_create(&emuThread,NULL,&emuThreadMain,NULL);
#elif __WIN32
			DWORD thrid;
			emuSem = CreateSemaphore(NULL,0,1,NULL);
			eventSem = CreateSemaphore(NULL,0,1,NULL);
			printf("create thread\n");
			emuThread = CreateThread(NULL,0,&emuThreadMain,NULL,0,&thrid);
#endif
			sndInit();
			emulInit();
			dbgInit(NULL);// emulWidget());
			optInit(emulWidget());
			devInit();
			initFileDialog(emulWidget());
			loadProfiles();
			if (profName) {
				setProfile(std::string(profName));
			} else {
//				prfLoad("");
			}
			fillUserMenu();
			emulShow();
			emulUpdateWindow();
			zxReset(zx,RES_DEFAULT);

			for(i=1;i<ac;i++) {
				if (strcmp(av[i],"-p") == 0) {		// skip -p profname
					i++;
				} else {
					if (strlen(av[i]) > 0) loadFile(zx,av[i],FT_ALL,0);
				}
			}

#ifdef HAVESDL
			SDL_JoystickOpen(1);
#endif
			mainWin->checkState();

			emuStart();
			SDL_TimerID tid = SDL_AddTimer(20,&onTimer,NULL);
			while (~emulFlags & FL_EXIT) {
#if __linux
				sem_wait(&eventSem);
#elif __WIN32
				WaitForSingleObject(eventSem,INFINITE);
#endif
				app.processEvents();
				emuFrame();
			}
			SDL_RemoveTimer(tid);
#if __linux
			sem_post(&emuSem);
			pthread_join(emuThread,NULL);
#elif __WIN32
			ReleaseSemaphore(emuSem,1,NULL);
			WaitForSingleObject(emuThread,INFINITE);
			CloseHandle(eventSem);
			CloseHandle(emuSem);
			CloseHandle(emuThread);
#endif
//			app.exec();
			emuStop();
			sndClose();
#ifdef HAVESDL
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
