#include <QUrl>
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
extern Sound* snd;
extern Settings* sets;
extern EmulWin* mwin;
extern DebugWin* dbg;
extern DevelWin* dwin;

std::vector<XBookmark> bookmarkList;
std::vector<XProfile> profileList;
XProfile* currentProfile;
QMenu* userMenu;
QMenu* bookmarkMenu;
QMenu* profileMenu;

#define	XPTITLE	"Xpeccy 0.4.993"

//== NEW SHIT IS HERE

// TODO: SOMETHING INSTEAD OLD SHIT

//== OLD SHIT IS HERE

EmulWin::EmulWin() {
	setcuricon(":/images/logo.png");
	setWindowTitle(XPTITLE);
	setMouseTracking(true);

	int i;
	for (i=0; i<16; i++) {
		zxpal[i].b = (i & 1)?((i & 8)?0xff:0xc0):0;
		zxpal[i].r = (i & 2)?((i & 8)?0xff:0xc0):0;
		zxpal[i].g = (i & 4)?((i & 8)?0xff:0xc0):0;
	}

	pal.clear(); pal.resize(256);
	for (i=0; i<256; i++) {
		pal[i] = qRgb(zxpal[i].r,zxpal[i].g,zxpal[i].b);
	}

	fast = false;
	ssnum = ssbcnt = ssbint = 0;
	flags = 0;
	
	initUserMenu((QWidget*)this);
	QObject::connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	QObject::connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	
	sets->opt.scrshotFormat = "png";
	
#ifndef WIN32

	SDL_SysWMinfo inf;
	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
	embedClient(inf.info.x11.wmwindow);

	sets->opt.sndOutputName = "ALSA";
	sets->opt.scrshotDir = std::string(getenv("HOME"));

#else

#if !SDLMAINWIN

	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
//	SetWindowLong(inf.window,GWL_STYLE,GetWindowLong(inf.window,GWL_STYLE) | WS_CHILD);
	SetParent(inf.window,winId());
	
#endif

	sets->opt.sndOutputName = "WaveOut";
	sets->opt.scrshotDir = std::string(getenv("HOMEPATH"));

#endif

	tim1 = new QTimer();
	tim2 = new QTimer();
	QObject::connect(tim1,SIGNAL(timeout()),this,SLOT(emulframe()));
//	QObject::connect(tim1,SIGNAL(timeout()),this,SLOT(updateframe()));
	QObject::connect(tim2,SIGNAL(timeout()),this,SLOT(SDLEventHandler()));
	repause(false,-1);
//	setAcceptDrops(true);
}

void EmulWin::closeEvent(QCloseEvent* ev) {
		repause(true,PR_QUIT);
		bool yep = zx->bdi->flop[0].savecha();
		yep &= zx->bdi->flop[1].savecha();
		yep &= zx->bdi->flop[2].savecha();
		yep &= zx->bdi->flop[3].savecha();
		if (yep) {
			ev->accept();
		} else {
			ev->ignore();
//			sys->vid->update();
			SDL_SysWMinfo inf;
			SDL_VERSION(&inf.version);
			SDL_GetWMInfo(&inf);
			embedClient(inf.info.x11.wmwindow);
			repause(false,PR_QUIT);
		}
}

void EmulWin::updateframe() {
/*
	if ((sys->vid->flags & VF_FULLSCREEN) && !(sys->vid->flags & VF_BLOCKFULLSCREEN)) {
		SDL_Surface* bsurf = SDL_CreateRGBSurface(SDL_HWSURFACE,sys->vid->wsze.h,sys->vid->wsze.v,8,0x03,0x0c,0x30,0xc0);
		SDL_Rect rect;
		rect.x = rect.y = 0;
		rect.w = sys->vid->wsze.h;
		rect.h = sys->vid->wsze.v;
		SDL_BlitSurface(sys->vid->surf,&rect,bsurf,&rect);
		double zoomx = sys->vid->surf->w / bsurf->w;
		double zoomy = sys->vid->surf->h / bsurf->h;
		SDL_Surface* csurf = zoomSurface(bsurf,zoomx,zoomy,1);
		SDL_BlitSurface(csurf,NULL,sys->vid->surf,NULL);
	}
*/
	SDL_UpdateRect(surf,0,0,0,0);		//SDL_UpdateRect(zx->vid->surf,0,0,0,0);
}

void EmulWin::emulframe() {
	SDL_UpdateRect(surf,0,0,0,0);		//SDL_UpdateRect(zx->vid->surf,0,0,0,0);
//	if ((size().width() != (int)sys->vid->wsze.h) || (size().height() != (int)sys->vid->wsze.v)) {
//		setFixedSize(sys->vid->wsze.h,sys->vid->wsze.v);
//	}
#if !SDLMAINWIN
	if (!isActiveWindow()) {
		zx->keyb->releaseall();		// в неактивном окне все кнопки отпущены и мышь не заграблена
		zx->mouse->buttons = 0xff;
//		SDL_WM_GrabInput(SDL_GRAB_OFF);
//		SDL_ShowCursor(SDL_ENABLE);
	}
#endif
//	bool lint = vid->intupt;
	if (paused==0) {
		if (!fast && (snd->outsys!=NULL) && snd->enabled && (snd->mute || isActiveWindow())) {
//			printf("%i\n",snd->sbptr - snd->sndbuf);
			snd->outsys->play();
		}
//		snd->sbptr = snd->sndbuf;
//		snd->t = zx->vid->t;
		snd->smpCount = 0;
		do {
			exec();
		} while (!zx->sys->cpu->err && !zx->sys->istrb);
		zx->sys->nmi = false;
//		if (flags & FL_RZX) {
//			rfnum++; rfpos = 0;
//			printf("=== Frame %i\n",rfnum);
//			if (rfnum >= rzx.size()) flags &= ~FL_RZX;
//		}

//		if (!(dbg->active || cpu->err)) cpu->interrupt();
	}

//	SDL_UpdateRect(sys->vid->surf,0,0,0,0);
//	SDLEventHandler();

	if (paused!=0) return;

	if (ssbcnt!=0) {
		if (ssbint==0) {
			flags |= FL_SHOT;
			ssbcnt--;
			ssbint = sets->ssint;
			if (ssbcnt==0) printf("stop combo shots\n");
		} else {
			ssbint--;
		}
	}
	if (flags & FL_SHOT) {
		std::string fnam = sets->opt.scrshotDir + "/sshot" + int2str(ssnum) + "." + sets->opt.scrshotFormat;
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
		flags &= ~FL_SHOT;
		ssnum++;
	}
}

void EmulWin::exec() {
	uint32_t ln = zx->vid->t;
	zx->exec();
	ln = zx->vid->t - ln;
	if (zx->vid->t > snd->t) {
		snd->sync();
	}
	if (!dbg->active) {
		// somehow catch CPoint
		if (dbg->findbp(BPoint((zx->sys->cpu->pc < 0x4000) ? zx->sys->mem->crom : zx->sys->mem->cram, zx->sys->cpu->pc)) != -1) {
			dbg->start();
			zx->sys->cpu->err = true;
		}
		if (!zx->sys->cpu->err && zx->sys->istrb) {
			zx->INTHandle();
//			zx->vid->sync(ln, zx->sys->cpu->frq);
		}
	}
}

void EmulWin::updateWin() {
	zx->vid->update();
	int szw = zx->vid->wsze.h;
	int szh = zx->vid->wsze.v;
	setFixedSize(szw,szh);
	int sdlflg = SDL_SWSURFACE;	// | SDL_FULLSCREEN;
	if ((zx->vid->flags & VF_FULLSCREEN) && !(zx->vid->flags & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
	}
#if SDLMAINWIN
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg);
	SDL_WM_SetCaption(XPTITLE,XPTITLE);
#else
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg | SDL_NOFRAME);
#ifdef WIN32
	SetWindowPos(inf.window,HWND_TOP,0,0,szw,szh,0);
#endif
#endif
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
	zx->vid->scrimg = (uint8_t*)surf->pixels;
	zx->vid->scrptr = zx->vid->scrimg;
}

void EmulWin::setcuricon(QString nam) {
	curicon = QIcon(nam);
	repause(true,0);
}

void EmulWin::repause(bool p, int msk) {
	if (p) {paused |= msk;} else {paused &= ~msk;}
	bool kk = flags & FL_GRAB;
	if (!kk || ((paused!=0) && kk)) {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
	}
	if ((paused==0) && kk) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(SDL_DISABLE);
	}
	if (paused==0) {
		setWindowIcon(curicon);
		if (snd->outsys != NULL) {
			if (snd->outsys->name == "SDL") SDL_PauseAudio(0);
		}
	} else {
		setWindowIcon(QIcon(":/images/pause.png"));
		if (snd->outsys != NULL) {
			if (snd->outsys->name == "SDL") SDL_PauseAudio(1);
		}
	}
	if (msk & PR_PAUSE) return;
	if ((paused & ~PR_PAUSE) == 0) {
		zx->vid->flags &= ~VF_BLOCKFULLSCREEN;
		if (zx->vid->flags & VF_FULLSCREEN) updateWin();
	} else {
		zx->vid->flags |= VF_BLOCKFULLSCREEN;
		if (zx->vid->flags & VF_FULLSCREEN) updateWin();
	}
}

void EmulWin::reset() {
	zx->reset();
//	snd->sc1->reset();
//	snd->sc2->reset();
//	snd->scc = snd->sc1;
//	ide->reset();
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
						case SDLK_1: zx->vid->flags &= ~VF_DOUBLE; updateWin(); sets->save(); break;
						case SDLK_2: zx->vid->flags |= VF_DOUBLE; updateWin(); sets->save(); break;
						case SDLK_3: fast = !fast;
							tim1->start(fast?0:20);
							break;
						case SDLK_F4: close(); break;
						case SDLK_F7: ssbcnt = sets->sscnt; ssbint=0; break;	// ALT+F7 combo
						case SDLK_RETURN: zx->vid->flags ^= VF_FULLSCREEN; updateWin(); sets->save(); break;
						case SDLK_c:
							flags ^= FL_GRAB;
							if (flags & FL_GRAB) {
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
						case SDLK_PAUSE: if (paused == 0) repause(true,PR_PAUSE); else repause(false,PR_PAUSE); break;
						case SDLK_ESCAPE: dbg->start(); break;
						case SDLK_MENU: repause(true,PR_MENU); userMenu->popup(pos() + QPoint(20,20)); break;
						case SDLK_F1: emit(wannasetup()); break;
						case SDLK_F2: repause(true,PR_FILE); saveFile("",FT_ALL,-1); repause(false,PR_FILE); break;
						case SDLK_F3: repause(true,PR_FILE);
							loadFile("",FT_ALL,-1);
							repause(false,PR_FILE);
							break;
						case SDLK_F4: if (zx->tape->flags & TAPE_ON) {
									zx->tape->stop();
									setcuricon(":/images/logo.png");
								} else {
									zx->tape->startplay();
									setcuricon(":/images/play.png");
								}
								break;
						case SDLK_F5: if (zx->tape->flags & TAPE_ON) {
									zx->tape->stop();
									setcuricon(":/images/logo.png");
								} else {
									zx->tape->startrec();
									setcuricon(":/images/rec.png");
								}
								break;
						case SDLK_F6: emit wannadevelop(); break;
						case SDLK_F7: if (ssbcnt==0) {flags |= FL_SHOT;} else {flags &= ~FL_SHOT;} break;
						case SDLK_F9: repause(true,PR_FILE);
							zx->bdi->flop[0].savecha();
							zx->bdi->flop[1].savecha();
							zx->bdi->flop[2].savecha();
							zx->bdi->flop[3].savecha();
							repause(false,PR_FILE);
							break;
						case SDLK_F10:
							zx->sys->nmi = true;
							break;
						case SDLK_F12: reset(); break;
						default: break;
					}
				}
				break;
			case SDL_KEYUP:
				zx->keyb->release(ev.key.keysym.scancode);
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (ev.button.button) {
					if (paused!=0) break;
					case SDL_BUTTON_LEFT: if (flags & FL_GRAB) zx->mouse->buttons &= ~0x01; break;
					case SDL_BUTTON_RIGHT: if (flags & FL_GRAB) {
							zx->mouse->buttons &= ~0x02;
						} else {
							repause(true,PR_MENU);
							userMenu->popup(pos() + QPoint(ev.button.x,ev.button.y+20));
							userMenu->setFocus();
						}
						break;	
					case SDL_BUTTON_MIDDLE:
						break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (paused!=0) break;
				switch (ev.button.button) {
					case SDL_BUTTON_LEFT:
						if (flags & FL_GRAB) {
							zx->mouse->buttons |= 0x01;
						}
						break;
					case SDL_BUTTON_RIGHT:
						if (flags & FL_GRAB) {
							zx->mouse->buttons |= 0x02;
						}
						break;
					case SDL_BUTTON_MIDDLE:
						flags ^= FL_GRAB;
						if (flags & FL_GRAB) {
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
				if (!(flags & FL_GRAB) || (paused!=0)) break;
				zx->mouse->xpos = (ev.motion.x - 1)&0xff;
				zx->mouse->ypos = (257 - ev.motion.y)&0xff;
				SDL_WarpMouse(zx->mouse->xpos + 1, 257 - zx->mouse->ypos);
				SDL_PeepEvents(&ev,1,SDL_GETEVENT,SDL_EVENTMASK(SDL_MOUSEMOTION));
				break;
		}
	}
	if (!userMenu->isVisible() && (paused & PR_MENU)) {
		setFocus();
		repause(false,PR_MENU);
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

//=====================
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
	setFocus();
}


void EmulWin::profileSelected(QAction* act) {
	repause(true,PR_EXTRA);
	setProfile(std::string(act->text().toUtf8().data()));
	sets->load(false);
	updateWin();
	sets->saveProfiles();
	setFocus();
	repause(false,PR_EXTRA);
}
