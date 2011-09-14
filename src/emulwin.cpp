#include <QDebug>
#include <QMessageBox>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
	#include <SDL.h>
	#include <SDL_syswm.h>
	#include <windows.h>
	#undef main
	extern HWND wid;
#endif
#include "sound.h"
#include "spectrum.h"
#include "settings.h"
#include "emulwin.h"
#include "debuger.h"
#include "develwin.h"
#include "filer.h"

#ifndef WIN32
	#include <SDL.h>
	#include <SDL_syswm.h>
#endif

#include <fstream>

std::string int2str(int);
void shithappens(std::string);

extern ZXComp* zx;
extern Settings* sets;
extern EmulWin* mwin;
extern DebugWin* dbg;
extern DevelWin* dwin;
// main
QX11EmbedContainer* mainWin;
QIcon curicon;
SDL_Surface* surf;
SDL_Color zxpal[256];
SDL_SysWMinfo inf;
uint32_t emulFlags;
int pauseFlags;
uint32_t scrNumber;
uint32_t scrCounter;
uint32_t scrInterval;
// for user menu
std::vector<XBookmark> bookmarkList;
std::vector<XProfile> profileList;
XProfile* currentProfile;
QMenu* userMenu;
QMenu* bookmarkMenu;
QMenu* profileMenu;

#define	XPTITLE	"Xpeccy 0.4.994"

void emulInit() {
	int i;
	for (i=0; i<16; i++) {
		zxpal[i].b = (i & 1) ? ((i & 8) ? 0xff : 0xc0) : 0;
		zxpal[i].r = (i & 2) ? ((i & 8) ? 0xff : 0xc0) : 0;
		zxpal[i].g = (i & 4) ? ((i & 8) ? 0xff : 0xc0) : 0;
	}
	emulFlags = 0;

	scrNumber = 0;
	scrCounter = 0;
	scrInterval = 0;
	sets->opt.scrshotFormat = "png";

	int par[] = {448,320,138,80,64,32,64,0};
	addLayout("default",par);

	mainWin = new QX11EmbedContainer;
	mainWin->setWindowTitle(XPTITLE);
	mainWin->setMouseTracking(true);

	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
#ifndef WIN32
	mainWin->embedClient(inf.info.x11.wmwindow);
	sets->opt.scrshotDir = std::string(getenv("HOME"));
#else
	SetParent(inf.window,winId());
	sets->opt.scrshotDir = std::string(getenv("HOMEPATH"));
#endif
	initUserMenu((QWidget*)mainWin);
}

void emulShow() {
	mainWin->show();
}

QWidget* emulWidget() {
	return (QWidget*)mainWin;
}

void emulUpdateWindow() {
	zx->vid->update();
	int szw = zx->vid->wsze.h;
	int szh = zx->vid->wsze.v;
	mainWin->setFixedSize(szw,szh);
	int sdlflg = SDL_SWSURFACE;
	if ((zx->vid->flags & VF_FULLSCREEN) && !(zx->vid->flags & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
	}
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg | SDL_NOFRAME);
#ifdef WIN32
	SetWindowPos(inf.window,HWND_TOP,0,0,szw,szh,0);
#endif
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
	zx->vid->scrimg = (uint8_t*)surf->pixels;
	zx->vid->scrptr = zx->vid->scrimg;
}

// FIXME: it doesn't work!
void emulRestore() {
	emulUpdateWindow();
	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
	mainWin->embedClient(inf.info.x11.wmwindow);
	mainWin->show();
}

bool emulSaveChanged() {
	bool yep = zx->bdi->flop[0].savecha();
	yep &= zx->bdi->flop[1].savecha();
	yep &= zx->bdi->flop[2].savecha();
	yep &= zx->bdi->flop[3].savecha();
	return yep;
}

int emulGetFlags() {return emulFlags;}
void emulSetFlag(int msk,bool cnd) {
	if (cnd) {
		emulFlags |= msk;
	} else {
		emulFlags &= ~msk;
	}
}

EmulWin::EmulWin() {
	emulSetIcon(":/images/logo.png");
	pal.clear(); pal.resize(256);
	int i;
	for (i=0; i<256; i++) {
		pal[i] = qRgb(zxpal[i].r,zxpal[i].g,zxpal[i].b);
	}
	
	QObject::connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	QObject::connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));

	tim1 = new QTimer();
	tim2 = new QTimer();
	QObject::connect(tim1,SIGNAL(timeout()),this,SLOT(emulframe()));
	QObject::connect(tim2,SIGNAL(timeout()),this,SLOT(SDLEventHandler()));

	emulPause(false,-1);
}

/*
void EmulWin::closeEvent(QCloseEvent* ev) {
		emulPause(true,PR_QUIT);
		bool yep = zx->bdi->flop[0].savecha();
		yep &= zx->bdi->flop[1].savecha();
		yep &= zx->bdi->flop[2].savecha();
		yep &= zx->bdi->flop[3].savecha();
		if (yep) {
			ev->accept();
		} else {
			ev->ignore();
			SDL_SysWMinfo inf;
			SDL_VERSION(&inf.version);
			SDL_GetWMInfo(&inf);
			mainWin->embedClient(inf.info.x11.wmwindow);
			emulPause(false,PR_QUIT);
		}
}
*/

void EmulWin::emulframe() {
	SDL_UpdateRect(surf,0,0,0,0);
#if !SDLMAINWIN
	if (!mainWin->isActiveWindow()) {
		zx->keyb->releaseall();
		zx->mouse->buttons = 0xff;
	}
#endif
	if (pauseFlags != 0) return;

	if (!(emulFlags & FL_FAST) && sndGet(SND_ENABLE) && (sndGet(SND_MUTE) || mainWin->isActiveWindow())) {
		sndPlay();
	}
	sndSet(SND_COUNT,0);
	do {
		exec();
	} while (!zx->sys->cpu->err && !zx->sys->istrb);
	zx->sys->nmi = false;

	if (scrCounter !=0 ) {
		if (scrInterval == 0) {
			emulFlags |= FL_SHOT;
			scrCounter--;
			scrInterval = sets->ssint;
			if (scrCounter == 0) printf("stop combo shots\n");
		} else {
			scrInterval--;
		}
	}
	if (emulFlags & FL_SHOT) {
		std::string fnam = sets->opt.scrshotDir + "/sshot" + int2str(scrNumber) + "." + sets->opt.scrshotFormat;
		if (sets->opt.scrshotFormat == "scr") {
			std::ofstream file(fnam.c_str(),std::ios::binary);
			file.write((char*)&zx->sys->mem->ram[zx->vid->curscr ? 7 : 5][0],0x1b00);
		} else {
			QImage *img = new QImage((uchar*)surf->pixels,surf->w,surf->h,QImage::Format_Indexed8);
			img->setColorTable(pal);
			if (img==NULL) {
				printf("NULL image\n");
			} else {
				img->save(QString(fnam.c_str()),sets->opt.scrshotFormat.c_str());
			}
		}
		emulFlags &= ~FL_SHOT;
		scrNumber++;
	}
}

void EmulWin::exec() {
	zx->exec();
	sndSync(zx->vid->t);
	if (!dbg->active) {
		// somehow catch CPoint
		if (dbg->findbp(BPoint((zx->sys->cpu->pc < 0x4000) ? zx->sys->mem->crom : zx->sys->mem->cram, zx->sys->cpu->pc)) != -1) {
			dbg->start();
			zx->sys->cpu->err = true;
		}
		if (!zx->sys->cpu->err && zx->sys->istrb) {
			zx->INTHandle();
		}
	}
}

void emulSetIcon(const char* inam) {
	curicon = QIcon(QString(inam));
	emulPause(true, 0);
}

//void EmulWin::setcuricon(QString nam) {
//	curicon = QIcon(nam);
//	emulPause(true,0);
//}

void emulPause(bool p, int msk) {
	if (p) {
		pauseFlags |= msk;
	} else {
		pauseFlags &= ~msk;
	}
	bool kk = emulFlags & FL_GRAB;
	if (!kk || ((pauseFlags != 0) && kk)) {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
	}
	if ((pauseFlags == 0) && kk) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(SDL_DISABLE);
	}
	if (pauseFlags == 0) {
		mainWin->setWindowIcon(curicon);
		sndPause(false);
	} else {
		mainWin->setWindowIcon(QIcon(":/images/pause.png"));
		sndPause(true);
	}
	if (msk & PR_PAUSE) return;
	if ((pauseFlags & ~PR_PAUSE) == 0) {
		zx->vid->flags &= ~VF_BLOCKFULLSCREEN;
		if (zx->vid->flags & VF_FULLSCREEN) emulUpdateWindow();
	} else {
		zx->vid->flags |= VF_BLOCKFULLSCREEN;
		if (zx->vid->flags & VF_FULLSCREEN) emulUpdateWindow();
	}
}

// keys

void EmulWin::SDLEventHandler() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				if (ev.key.keysym.mod & KMOD_ALT) {
					switch(ev.key.keysym.sym) {
						case SDLK_0: zx->vid->mode = (zx->vid->mode==VID_NORMAL)?VID_ALCO:VID_NORMAL; break;
						case SDLK_1: zx->vid->flags &= ~VF_DOUBLE; emulUpdateWindow(); sets->save(); break;
						case SDLK_2: zx->vid->flags |= VF_DOUBLE; emulUpdateWindow(); sets->save(); break;
						case SDLK_3: emulFlags ^= FL_FAST;
							tim1->start((emulFlags & FL_FAST) ? 0 : 20);
							break;
						case SDLK_F4: mainWin->close(); break;
						case SDLK_F7: scrCounter = sets->sscnt; scrInterval=0; break;	// ALT+F7 combo
						case SDLK_RETURN: zx->vid->flags ^= VF_FULLSCREEN; emulUpdateWindow(); sets->save(); break;
						case SDLK_c:
							emulFlags ^= FL_GRAB;
							if (emulFlags & FL_GRAB) {
								SDL_WM_GrabInput(SDL_GRAB_ON);
								SDL_ShowCursor(SDL_DISABLE);
							} else {
								SDL_WM_GrabInput(SDL_GRAB_OFF);
								SDL_ShowCursor(SDL_ENABLE);
							}
							break;
						default: break;
					}
				} else {
					zx->keyb->press(ev.key.keysym.scancode);
					switch (ev.key.keysym.sym) {
						case SDLK_PAUSE: pauseFlags ^= PR_PAUSE; emulPause(true,0); break;
						case SDLK_ESCAPE: dbg->start(); break;
						case SDLK_MENU: emulPause(true,PR_MENU); userMenu->popup(mainWin->pos() + QPoint(20,20)); break;
						case SDLK_F1: emit(wannasetup()); break;
						case SDLK_F2: emulPause(true,PR_FILE); saveFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
						case SDLK_F3: emulPause(true,PR_FILE);
							loadFile("",FT_ALL,-1);
							emulPause(false,PR_FILE);
							break;
						case SDLK_F4: if (zx->tape->flags & TAPE_ON) {
									zx->tape->stop(zx->vid->t);
									emulSetIcon(":/images/logo.png");
								} else {
									zx->tape->startplay();
									emulSetIcon(":/images/play.png");
								}
								break;
						case SDLK_F5: if (zx->tape->flags & TAPE_ON) {
									zx->tape->stop(zx->vid->t);
									emulSetIcon(":/images/logo.png");
								} else {
									zx->tape->startrec();
									emulSetIcon(":/images/rec.png");
								}
								break;
						case SDLK_F6: emit wannadevelop(); break;
						case SDLK_F7: if (scrCounter == 0) {emulFlags |= FL_SHOT;} else {emulFlags &= ~FL_SHOT;} break;
						case SDLK_F9: emulPause(true,PR_FILE);
							zx->bdi->flop[0].savecha();
							zx->bdi->flop[1].savecha();
							zx->bdi->flop[2].savecha();
							zx->bdi->flop[3].savecha();
							emulPause(false,PR_FILE);
							break;
						case SDLK_F10:
							zx->sys->nmi = true;
							break;
						case SDLK_F12: zx->reset(); break;
						default: break;
					}
				}
				break;
			case SDL_KEYUP:
				zx->keyb->release(ev.key.keysym.scancode);
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (ev.button.button) {
					if (pauseFlags != 0) break;
					case SDL_BUTTON_LEFT: if (emulFlags & FL_GRAB) zx->mouse->buttons &= ~0x01; break;
					case SDL_BUTTON_RIGHT: if (emulFlags & FL_GRAB) {
							zx->mouse->buttons &= ~0x02;
						} else {
							emulPause(true,PR_MENU);
							userMenu->popup(mainWin->pos() + QPoint(ev.button.x,ev.button.y+20));
						}
						break;	
					case SDL_BUTTON_MIDDLE:
						break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (pauseFlags != 0) break;
				switch (ev.button.button) {
					case SDL_BUTTON_LEFT:
						if (emulFlags & FL_GRAB) {
							zx->mouse->buttons |= 0x01;
						}
						break;
					case SDL_BUTTON_RIGHT:
						if (emulFlags & FL_GRAB) {
							zx->mouse->buttons |= 0x02;
						}
						break;
					case SDL_BUTTON_MIDDLE:
						emulFlags ^= FL_GRAB;
						if (emulFlags & FL_GRAB) {
							SDL_WM_GrabInput(SDL_GRAB_ON);
							SDL_ShowCursor(SDL_DISABLE);
						} else {
							SDL_WM_GrabInput(SDL_GRAB_OFF);
							SDL_ShowCursor(SDL_ENABLE);
						}
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				if ((~emulFlags & FL_GRAB) || (pauseFlags !=0 )) break;
				zx->mouse->xpos = (ev.motion.x - 1)&0xff;
				zx->mouse->ypos = (257 - ev.motion.y)&0xff;
				SDL_WarpMouse(zx->mouse->xpos + 1, 257 - zx->mouse->ypos);
				SDL_PeepEvents(&ev,1,SDL_GETEVENT,SDL_EVENTMASK(SDL_MOUSEMOTION));
				break;
		}
	}
	if (!userMenu->isVisible() && (pauseFlags & PR_MENU)) {
		mainWin->setFocus();
		emulPause(false,PR_MENU);
	}
}

//=====================
// PROFILES

void fillProfileMenu() {
	profileMenu->clear();
	for(uint i=0; i < profileList.size(); i++) {
		profileMenu->addAction(profileList[i].name.c_str());
	}
}

void addProfile(std::string nm, std::string fp) {
	XProfile nprof;
	nprof.name = nm;
	nprof.file = fp;
	nprof.zx = new ZXComp;
	profileList.push_back(nprof);
}

bool setProfile(std::string nm) {
	for (uint i=0; i<profileList.size(); i++) {
		if (profileList[i].name == nm) {
			currentProfile = &profileList[i];
			zx = currentProfile->zx;
			return true;
		}
	}
	return false;
}

void clearProfiles() {
	XProfile defprof = profileList[0];
	profileList.clear();
	profileList.push_back(defprof);
}

std::vector<XProfile> getProfileList() {
	return profileList;
}

XProfile* getCurrentProfile() {
	return currentProfile;
}

// USER MENU

void initUserMenu(QWidget* par) {
	userMenu = new QMenu(par);
	bookmarkMenu = userMenu->addMenu("Bookmarks");
	profileMenu = userMenu->addMenu("Profiles");
}

void addBookmark(std::string nm, std::string fp) {
	XBookmark nbm;
	nbm.name = nm;
	nbm.path = fp;
	bookmarkList.push_back(nbm);
}

void swapBookmarks(int p1, int p2) {
	XBookmark bm = bookmarkList[p1];
	bookmarkList[p1] = bookmarkList[p2];
	bookmarkList[p2] = bm;
}

void setBookmark(int idx,std::string nm, std::string fp) {
	bookmarkList[idx].name = nm;
	bookmarkList[idx].path = fp;
}

void delBookmark(int idx) {
	bookmarkList.erase(bookmarkList.begin() + idx);
}

void clearBookmarks() {
	bookmarkList.clear();
}

int getBookmarksCount() {
	return bookmarkList.size();
}

void fillBookmarkMenu() {
	bookmarkMenu->clear();
	QAction* act;
	if (bookmarkList.size() == 0) {
		bookmarkMenu->addAction("None")->setEnabled(false);
	} else {
		for(uint i=0; i<bookmarkList.size(); i++) {
			act = bookmarkMenu->addAction(bookmarkList[i].name.c_str());
			act->setData(QVariant(bookmarkList[i].path.c_str()));
		}
	}
}

std::vector<XBookmark> getBookmarkList() {
	return bookmarkList;
}

// SLOTS

void EmulWin::bookmarkSelected(QAction* act) {
	loadFile(act->data().toString().toUtf8().data(),FT_ALL,0);
	mainWin->setFocus();
}


void EmulWin::profileSelected(QAction* act) {
	emulPause(true,PR_EXTRA);
	setProfile(std::string(act->text().toUtf8().data()));
	sets->load(false);
	emulUpdateWindow();
	sets->saveProfiles();
	mainWin->setFocus();
	emulPause(false,PR_EXTRA);
}
