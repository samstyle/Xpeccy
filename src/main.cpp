#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <SDL/SDL.h>
#ifdef WIN32
	#undef main
#endif
#include <sstream>
#include <getopt.h>

#include "libxpeccy/spectrum.h"
#include "sound.h"
#include "common.h"
#include "emulwin.h"
#include "settings.h"
#include "debuger.h"
#include "setupwin.h"
#include "develwin.h"
#include "filer.h"

#ifdef WIN32
	#include <direct.h>
#endif

ZXComp* zx;
EmulWin *mwin;
extern MainWin* mainWin;

std::string getTimeString(int32_t tsec) {
	int32_t tmin = tsec / 60;
	tsec -= tmin * 60;
	std::string res = int2str(tmin);
	res += ":";
	if (tsec < 10) res += "0";
	res += int2str(tsec);
	return res;
}

void setFlagBit(bool cond, int32_t* val, int32_t mask) {
	if (cond) {
		*val |= mask;
	} else {
		*val &= ~mask;
	}
}

void shitHappens(const char* msg) {
	QMessageBox mbx(QMessageBox::Critical,"Shit happens",QDialog::trUtf8(msg),QMessageBox::Ok);
	mbx.exec();
}

bool areSure(const char* msg) {
	QMessageBox mbx(QMessageBox::Question,"R U Sure?",QDialog::trUtf8(msg),QMessageBox::Yes | QMessageBox::No);
	int res = mbx.exec();
	return (res == QMessageBox::Yes);
}

void showInfo(const char* msg) {
	QMessageBox mbx(QMessageBox::Information,"Message",QDialog::trUtf8(msg),QMessageBox::Ok);
	mbx.exec();
}

std::string int2str(int num) {
	std::stringstream str;
	str<<num;
	return str.str();
}

bool str2bool(std::string v) {
	return (v=="y" || v=="Y" || v=="1" || v=="yes" || v=="YES" || v=="true" || v=="TRUE");
}

std::vector<std::string> splitstr(std::string str,const char* spl) {
	size_t pos;
	std::vector<std::string> res;
	pos = str.find_first_of(spl);
	while (pos != std::string::npos) {
		res.push_back(str.substr(0,pos));
		str = str.substr(pos+1);
		pos = str.find_first_of(spl);
	}
	res.push_back(str);
	return res;
	
}

std::pair<std::string,std::string> splitline(std::string line) {
	size_t pos;
	std::pair<std::string,std::string> res;
	do {pos = line.find("\r"); if (pos!=std::string::npos) line.erase(pos);} while (pos!=std::string::npos);
	do {pos = line.find("\n"); if (pos!=std::string::npos) line.erase(pos);} while (pos!=std::string::npos);
	res.first = "";
	res.second = "";
	pos = line.find("=");
	if (pos!=std::string::npos) {
		res.first = std::string(line,0,pos);
		res.second = std::string(line,pos+1);
		pos = res.first.find_last_not_of(" ");
		if (pos != std::string::npos) res.first = std::string(res.first,0,pos+1);	// delete last spaces
		pos = res.second.find_first_not_of(" ");
		if (pos != std::string::npos) res.second = std::string(res.second,pos);		// delete first spaces
	} else {
		res.first = line;
		res.second = "";
	}
	return res;
}

int main(int ac,char** av) {
	SDL_version sdlver;
	SDL_VERSION(&sdlver);
	printf("Using SDL ver %u.%u.%u\n", sdlver.major, sdlver.minor, sdlver.patch);
	Z80EX_VERSION* ver = z80ex_get_version();
	printf("Using z80ex ver %d.%d\n",ver->major, ver->minor);
#ifdef XQTPAINT
	printf("Using Qt painter\n");
#else
	printf("Using SDL surface\n");
#endif
	
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK);
	atexit(SDL_Quit);
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

			SDL_JoystickOpen(1);
			mainWin->startTimer(20);
			app.exec();
			mainWin->stopTimer();
			sndClose();
			SDL_Quit();
			return 0;
		}
	}
	catch (const char* s) {
		shitHappens(s);
		SDL_Quit();
		return 1;
	}
	catch (int i) {
		SDL_Quit();
		return 1;
	}
}
