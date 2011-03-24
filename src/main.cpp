#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#include <sstream>

//#include "video.h"
//#include "memory.h"
//#include "iosys.h"
//#include "z80.h"
//#include "keyboard.h"
#include "spectrum.h"
#include "sound.h"
//#include "bdi.h"
//#include "tape.h"
//#include "hdd.h"
//#include "gs.h"

#include "emulwin.h"
#include "settings.h"
#include "debuger.h"
#include "setupwin.h"
#include "develwin.h"
#include "filer.h"

#ifdef WIN32
	#include <direct.h>
#endif

//Video *vid;
//Memory *mem;
//IOSys *iosys;
HardWare *hw;
//Z80 *cpu;
ZXComp* zx;
//Spec* sys;
Sound *snd;
//Keyboard *keyb;
//BDI *bdi;
//Tape *tape;
//Mouse *mouse;
//IDE *ide;
//GS *gs;

EmulWin *mwin;
DebugWin *dbg;
SetupWin *swin;
DevelWin *dwin;
MFiler *filer;

Settings *sets;

void shithappens(const char* msg) {
	QMessageBox mbx(QMessageBox::Critical,"Shit happens",QString(msg),QMessageBox::Ok);
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

//uint8_t zx_in(int);
//void zx_out(int,uint8_t);
void filltabs();

int main(int ac,char** av) {
	QApplication app(ac,av);
	try {
#if SDLMAINWIN
		app.setQuitOnLastWindowClosed(true);
#endif
		int i; bool dev = false;
		sets = new Settings;
		for (i=1; i<ac; i++) {
			if (std::string(av[i])=="-dev") dev=true;
		}
		if (dev) {
			sets->load(true);
			dwin = new DevelWin;
			dwin->show();
			return app.exec();
		} else {
			SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
			atexit(SDL_Quit);
			
			filltabs();
//			cpu = new Z80();
//			iosys = new IOSys(&zx_in,&zx_out);
			zx = new ZXComp;
			zx->vid = new Video;
//			zx->sys = new ZXBase;
//			vid = new Video();
//			keyb = new Keyboard();
//			mouse = new Mouse();
			snd = new Sound();
//			bdi = new BDI();
//			tape = new Tape();
//			ide = new IDE;
//			gs = new GS(); gs->reset();

			mwin = new EmulWin();
			dbg = new DebugWin((QWidget*)mwin);
			dwin = new DevelWin();
			filer = new MFiler(NULL);
#ifndef WIN32
			sets->soutname = "ALSA";
#else
			sets->soutname = "WaveOut";
#endif
#if !SDLMAINWIN
			mwin->show();
#endif
			sets->load(false);
			swin = new SetupWin((QWidget*)mwin);
			mwin->reset();

			for(i=1;i<ac;i++) filer->loadsomefile(std::string(av[i]),0);

			QObject::connect(mwin,SIGNAL(wannasetup()),swin,SLOT(start()));
			QObject::connect(mwin,SIGNAL(wannadevelop()),dwin,SLOT(start()));
#if SDLMAINWIN
			QObject::connect(mwin,SIGNAL(icum()),&app,SLOT(quit()));
#else
//			mwin->show();
#endif
			mwin->tim1->start(20);
			mwin->tim2->start(20);
			app.exec();
			snd->outsys->close();
			SDL_Quit();
			return 0;
		}
	}
	catch (const char* s) {
		shithappens(s);
//		QMessageBox mbx;
//		mbx.setIcon(QMessageBox::Critical);
//		mbx.setWindowTitle("Shit happens");
//		mbx.setText(QString(s));
//		mbx.exec();
		SDL_Quit();
		return 1;
	}
	catch (int i) {
		QMessageBox mbx(QMessageBox::Warning,"Test",QString::number(i));
		mbx.exec();
	}
}
