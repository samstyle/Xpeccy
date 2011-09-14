#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#include <sstream>

#include "spectrum.h"
#include "sound.h"

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
DebugWin *dbg;
SetupWin *swin;
DevelWin *dwin;

Settings *sets;

void setFlagBit(bool cond, int32_t* val, int32_t mask) {
	if (cond) {
		*val |= mask;
	} else {
		*val &= ~mask;
	}
}

void shithappens(std::string msg) {
	QMessageBox mbx(QMessageBox::Critical,"Shit happens",QString(msg.c_str()),QMessageBox::Ok);
	mbx.exec();
}

std::string int2str(int num) {std::stringstream str; str<<num; return str.str();}
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

void splitline(std::string line, std::string* pnam, std::string* pval) {
	size_t pos;
	do {pos = line.find("\r"); if (pos!=std::string::npos) line.erase(pos);} while (pos!=std::string::npos);
	do {pos = line.find("\n"); if (pos!=std::string::npos) line.erase(pos);} while (pos!=std::string::npos);
	*pnam = "";
	*pval = "";
	pos = line.find("=");
	if (pos!=std::string::npos) {
		*pnam = std::string(line,0,pos);
		*pval = std::string(line,pos+1);
		pos = pnam->find_last_not_of(" ");
		if (pos!=std::string::npos) *pnam = std::string(*pnam,0,pos+1);	// delete last spaces
		pos = pval->find_first_not_of(" ");
		if (pos!=std::string::npos) *pval = std::string(*pval,pos);	// delete first spaces
	} else {
		*pnam = line;
	}
}

void filltabs();

int main(int ac,char** av) {
	QApplication app(ac,av);
	try {
		int i; bool dev = false;
		sets = new Settings;
		addProfile("default","xpeccy.conf");
		setProfile("default");
		for (i=1; i<ac; i++) {
			if (std::string(av[i])=="-dev") dev=true;
		}
		if (dev) {
			dwin = new DevelWin;
			sets->load(true);
			dwin->show();
			return app.exec();
		} else {
			SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
			atexit(SDL_Quit);
			filltabs();
			sndInit();
			emulInit();
			mwin = new EmulWin();
			dbg = new DebugWin(emulWidget());
			dwin = new DevelWin();
			initFileDialog(emulWidget());
			sets->loadProfiles();
			fillProfileMenu();
			swin = new SetupWin(emulWidget());
			emulUpdateWindow();
			emulShow();
			sets->load(false);
			emulUpdateWindow();
			fillBookmarkMenu();
			zx->reset();

			for(i=1;i<ac;i++) loadFile(av[i],FT_ALL,0);

			QObject::connect(mwin,SIGNAL(wannasetup()),swin,SLOT(start()));
			QObject::connect(mwin,SIGNAL(wannadevelop()),dwin,SLOT(start()));

			mwin->tim1->start(20);
			mwin->tim2->start(20);
			app.exec();
			mwin->tim1->stop();
			mwin->tim2->stop();
			sndClose();
			SDL_Quit();
			return 0;
		}
	}
	catch (const char* s) {
		shithappens(s);
		SDL_Quit();
		return 1;
	}
	catch (int i) {
		SDL_Quit();
		return 1;
	}
}
