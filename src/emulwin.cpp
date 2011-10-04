#include <QDebug>
#include <QMessageBox>
#include <QProgressBar>
#include <QTableWidget>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
	#include <SDL.h>
	#include <SDL_timer.h>
	#include <SDL_syswm.h>
	#include <windows.h>
	#undef main
	extern HWND wid;
#endif
#include "common.h"
#include "sound.h"
#include "spectrum.h"
#include "settings.h"
#include "setupwin.h"
#include "emulwin.h"
#include "debuger.h"
#include "develwin.h"
#include "filer.h"

#include "ui_tapewin.h"

#ifndef WIN32
	#include <SDL.h>
	#include <SDL_timer.h>
	#include <SDL_syswm.h>
#endif

#include <fstream>

#define	XPTITLE	"Xpeccy 0.4.995"

#define	RSZ_NONE	0
#define	RSZ_SINGLE	1
#define	RSZ_DOUBLE	2
#define	RSZ_FULLSCR	3

extern ZXComp* zx;
extern EmulWin* mwin;
extern MainWin* mainWin;
// main
QIcon curicon;
SDL_Surface* surf = NULL;
SDL_Color zxpal[256];
SDL_SysWMinfo inf;
SDL_TimerID tid;
QVector<QRgb> qPal;
int emulFlags;
int pauseFlags;
int wantedWin;
uint32_t resizeMode; 
uint32_t scrNumber;
uint32_t scrCounter;
uint32_t scrInterval;
// tape window
Ui::TapeWin tapeUi;
QDialog* tapeWin;
// hardwares
std::vector<HardWare> hwList;
// romsets
std::vector<RomSet> rsList;
// for user menu
std::vector<XBookmark> bookmarkList;
std::vector<XProfile> profileList;
XProfile* currentProfile;
QMenu* userMenu;
QMenu* bookmarkMenu;
QMenu* profileMenu;

void emulInit() {
	int i;
	for (i=0; i<16; i++) {
		zxpal[i].b = (i & 1) ? ((i & 8) ? 0xff : 0xc0) : 0;
		zxpal[i].r = (i & 2) ? ((i & 8) ? 0xff : 0xc0) : 0;
		zxpal[i].g = (i & 4) ? ((i & 8) ? 0xff : 0xc0) : 0;
	}
	qPal.clear(); qPal.resize(256);
	for (i=0; i<256; i++) {
		qPal[i] = qRgb(zxpal[i].r,zxpal[i].g,zxpal[i].b);
	}
	emulFlags = 0;
	wantedWin = WW_NONE;
	resizeMode = RSZ_NONE;

	scrNumber = 0;
	scrCounter = 0;
	scrInterval = 0;
	optSet(OPT_SHOTEXT,std::string("png"));

	int par[] = {448,320,138,80,64,32,64,0};
	addLayout("default",par);
	
#ifndef WIN32
	optSet(OPT_SHOTDIR,std::string(getenv("HOME")));
#else
	optSet(OPT_SHOTDIR,std::string(getenv("HOMEPATH")));
#endif
}

void setTapeCheck() {
	QTableWidgetItem* itm;
	for (uint i=0; i<zx->tape->data.size(); i++) {
		itm = new QTableWidgetItem;
		if (zx->tape->block == i) {
			itm->setIcon(QIcon(":/images/checkbox.png"));
		}
		tapeUi.tapeList->setItem(i,0,itm);
	}
}

void buildTapeList() {
	std::vector<TapeBlockInfo> inf = zx->tape->getInfo();
	tapeUi.tapeList->setRowCount(inf.size());
	QTableWidgetItem* itm;
	for (uint i=0; i<inf.size(); i++) {
		itm = new QTableWidgetItem(QString::number(inf[i].size));
		tapeUi.tapeList->setItem(i,1,itm);
		itm = new QTableWidgetItem(QDialog::trUtf8(inf[i].name.c_str()));
		tapeUi.tapeList->setItem(i,2,itm);
	}
	setTapeCheck();
}

void emulShow() {
	mainWin->show();
}

QWidget* emulWidget() {
	return (QWidget*)mainWin;
}

void emulUpdateWindow() {
	mainWin->updateWindow();
}

void MainWin::updateWindow() {
	emulFlags |= FL_BLOCK;
	zx->vid->update();
	int szw = zx->vid->wsze.h;
	int szh = zx->vid->wsze.v;
	int sdlflg = SDL_SWSURFACE;
	if ((zx->vid->flags & VF_FULLSCREEN) && !(zx->vid->flags & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
	}
	setFixedSize(szw,szh);
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg | SDL_NOFRAME);
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
	zx->vid->scrimg = (uint8_t*)surf->pixels;
	zx->vid->scrptr = zx->vid->scrimg;
	emulFlags &= ~FL_BLOCK;
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

void emulExec() {
	zx->exec();
	sndSync(zx->vid->t);
	if (!dbgIsActive()) {
		// somehow catch CPoint
		if (dbgFindBreakpoint(BPoint((zx->sys->cpu->pc < 0x4000) ? zx->sys->mem->crom : zx->sys->mem->cram, zx->sys->cpu->pc)) != -1) {
			wantedWin = WW_DEBUG;
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

void emulPause(bool p, int msk) {
	if (p) {
		pauseFlags |= msk;
	} else {
		pauseFlags &= ~msk;
	}
	bool kk = ((emulFlags & FL_GRAB) != 0);
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

// Main window
// It's full shit, but i need it because of closeEvent

MainWin::MainWin() {
	setWindowTitle(XPTITLE);
	setMouseTracking(true);
	curicon = QIcon(":/images/logo.png");
	setWindowIcon(curicon);
	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
#ifndef WIN32
	embedClient(inf.info.x11.wmwindow);
#else
	SetParent(inf.window,winId());
#endif
	initUserMenu((QWidget*)this);
	tapeWin = new QDialog((QWidget*)this,Qt::Tool);
	tapeUi.setupUi(tapeWin);
	tapeUi.tapeList->setColumnWidth(0,20);
	tapeUi.tapeList->setColumnWidth(1,50);
	timer = new QTimer();
	QObject::connect(timer,SIGNAL(timeout()),this,SLOT(emulFrame()));
}

void MainWin::closeEvent(QCloseEvent* ev) {
	timer->stop();
	if (emulSaveChanged()) {
		ev->accept();
	} else {
		ev->ignore();
		SDL_VERSION(&inf.version);
		SDL_GetWMInfo(&inf);
		mainWin->embedClient(inf.info.x11.wmwindow);
		timer->start(20);
	}
}

void MainWin::startTimer(int iv) {timer->start(iv);}
void MainWin::stopTimer() {timer->stop();}

// ...

void MainWin::emulFrame() {
	if (emulFlags & FL_BLOCK) return;
//	if (~emulFlags & FL_FAST) SDL_UpdateRect(surf,0,0,0,0);
	if (!mainWin->isActiveWindow()) {
		zx->keyb->releaseall();
		zx->mouse->buttons = 0xff;
	}
	if ((wantedWin == WW_NONE) && (pauseFlags == 0)) {
		if (!(emulFlags & FL_FAST) && sndGet(SND_ENABLE) && (sndGet(SND_MUTE) || mainWin->isActiveWindow())) {
			sndPlay();
		}
		sndSet(SND_COUNT,0);
		do {
			emulExec();
		} while ((wantedWin == WW_NONE) && !zx->sys->istrb);
		zx->sys->nmi = false;
		if (scrCounter !=0 ) {
			if (scrInterval == 0) {
				emulFlags |= FL_SHOT;
				scrCounter--;
				scrInterval = optGetInt(OPT_SHOTINT);
				if (scrCounter == 0) printf("stop combo shots\n");
			} else {
				scrInterval--;
			}
		}
		if (emulFlags & FL_SHOT) {
			std::string fext = optGetString(OPT_SHOTEXT);
			std::string fnam = optGetString(OPT_SHOTDIR) + "/sshot" + int2str(scrNumber) + "." + fext;
			if (fext == "scr") {
				std::ofstream file(fnam.c_str(),std::ios::binary);
				file.write((char*)&zx->sys->mem->ram[zx->vid->curscr ? 7 : 5][0],0x1b00);
			} else {
				QImage *img = new QImage((uchar*)surf->pixels,surf->w,surf->h,QImage::Format_Indexed8);
				img->setColorTable(qPal);
				if (img==NULL) {
					printf("NULL image\n");
				} else {
					img->save(QString(fnam.c_str()),fext.c_str());
				}
			}
			emulFlags &= ~FL_SHOT;
			scrNumber++;
		}
	}
/*	switch (resizeMode) {
		case RSZ_SINGLE:
			zx->vid->flags &= ~VF_DOUBLE;
			emulUpdateWindow();
			resizeMode = RSZ_NONE;
			break;
		case RSZ_DOUBLE:
			zx->vid->flags |= VF_DOUBLE;
			emulUpdateWindow();
			resizeMode = RSZ_NONE;
			break;
		case RSZ_FULLSCR:
			zx->vid->flags ^= VF_FULLSCREEN;
			emulUpdateWindow();
			resizeMode = RSZ_NONE;
			break;
	} */
	return;
}

//Uint32 onTimer(Uint32 iv,void*) {
//	if (emulFlags & FL_BLOCK) return iv;
//	SDL_UpdateRect(surf,0,0,0,0);
//	return iv;
//}

//void emulStartTimer(int) {
//	tid = SDL_AddTimer(iv,&emulFrame,NULL);
//	if (emulFlags & FL_FAST) sid = SDL_AddTimer(100,&onTimer,NULL);
//}

//void emulStopTimer() {
//	SDL_RemoveTimer(tid);
//	SDL_RemoveTimer(sid);
//}

// OBJECT

EmulWin::EmulWin() {
	QObject::connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(bookmarkSelected(QAction*)));
	QObject::connect(profileMenu,SIGNAL(triggered(QAction*)),this,SLOT(profileSelected(QAction*)));
	timer = new QTimer;
	QObject::connect(timer,SIGNAL(timeout()),this,SLOT(SDLEventHandler()));
	timer->start(20);
	QObject::connect(tapeUi.playBut,SIGNAL(released()),this,SLOT(tapePlay()));
	QObject::connect(tapeUi.recBut,SIGNAL(released()),this,SLOT(tapeRec()));
	QObject::connect(tapeUi.stopBut,SIGNAL(released()),this,SLOT(tapeStop()));
	QObject::connect(tapeUi.loadBut,SIGNAL(released()),this,SLOT(tapeLoad()));
	QObject::connect(tapeUi.tapeList,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(tapeRewind(QModelIndex)));
	emulPause(false,-1);
}

void EmulWin::tapePlay() {
	if (!zx->tape->startplay()) return;
	emulSetIcon(":/images/play.png");
	tapeUi.playBut->setEnabled(false);
	tapeUi.recBut->setEnabled(false);
	tapeUi.stopBut->setEnabled(true);
}

void EmulWin::tapeRec() {
	zx->tape->startrec();
	emulSetIcon(":/images/rec.png");
	tapeUi.playBut->setEnabled(false);
	tapeUi.recBut->setEnabled(false);
	tapeUi.stopBut->setEnabled(true);
	tapeUi.tapeBar->setValue(0);
}

void EmulWin::tapeStop() {
	zx->tape->stop(zx->vid->t);
	emulSetIcon(":/images/logo.png");
	tapeUi.playBut->setEnabled(true);
	tapeUi.recBut->setEnabled(true);
	tapeUi.stopBut->setEnabled(false);
	tapeUi.tapeBar->setValue(0);
}

void EmulWin::tapeRewind(QModelIndex idx) {
	if (!idx.isValid()) return;
	zx->tape->block = idx.row();
	zx->tape->pos = 0;
	buildTapeList();
}

void EmulWin::tapeLoad() {
	emulPause(true,PR_FILE);
	loadFile("",FT_TAPE,-1);
	buildTapeList();
	emulPause(false,PR_FILE);
}

void EmulWin::SDLEventHandler() {
	SDL_UpdateRect(surf,0,0,0,0);
	if (emulFlags & FL_BLOCK) return;
	switch (wantedWin) {
		case WW_DEBUG: dbgShow(); wantedWin = WW_NONE; break;
	}
	if (zx->tape->flags & TAPE_ON) {
		if (~zx->tape->flags & TAPE_REC) {
			tapeUi.tapeBar->setMaximum(zx->tape->data[zx->tape->block].gettime(-1));			// total
			tapeUi.tapeBar->setValue(zx->tape->data[zx->tape->block].gettime(zx->tape->pos));		// current
		}
		setTapeCheck();
	} else {
		emulSetIcon(":/images/logo.png");
		tapeUi.playBut->setEnabled(true);
		tapeUi.recBut->setEnabled(true);
		tapeUi.stopBut->setEnabled(false);
		tapeUi.tapeBar->setValue(0);
	}
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				if (ev.key.keysym.mod & KMOD_ALT) {
					switch(ev.key.keysym.sym) {
						case SDLK_0: zx->vid->mode = (zx->vid->mode==VID_NORMAL)?VID_ALCO:VID_NORMAL; break;
						case SDLK_1:
//							resizeMode = RSZ_SINGLE;
 							zx->vid->flags &= ~VF_DOUBLE;
 							mainWin->updateWindow();
							saveConfig();
							break;
						case SDLK_2:
//							resizeMode = RSZ_DOUBLE;
							zx->vid->flags |= VF_DOUBLE;
							mainWin->updateWindow();
							saveConfig();
							break;
						case SDLK_3: emulFlags ^= FL_FAST;
							mainWin->stopTimer();
							mainWin->startTimer((emulFlags & FL_FAST) ? 1 : 20);
							break;
						case SDLK_F4:
							mainWin->close();
							break;
						case SDLK_F7:
							scrCounter = optGetInt(OPT_SHOTCNT);
							scrInterval=0;
							break;	// ALT+F7 combo
						case SDLK_RETURN:
//							resizeMode = RSZ_FULLSCR;
							zx->vid->flags ^= VF_FULLSCREEN;
							mainWin->updateWindow();
							saveConfig();
							break;
//						case SDLK_c:
//							emulFlags ^= FL_GRAB;
//							if (emulFlags & FL_GRAB) {
//								SDL_WM_GrabInput(SDL_GRAB_ON);
//								SDL_ShowCursor(SDL_DISABLE);
//							} else {
//								SDL_WM_GrabInput(SDL_GRAB_OFF);
//								SDL_ShowCursor(SDL_ENABLE);
//							}
							break;
						default: break;
					}
				} else {
					zx->keyb->press(ev.key.keysym.scancode);
					switch (ev.key.keysym.sym) {
						case SDLK_PAUSE: pauseFlags ^= PR_PAUSE; emulPause(true,0); break;
						case SDLK_ESCAPE: wantedWin = WW_DEBUG; break;
						case SDLK_MENU: emulPause(true,PR_MENU); userMenu->popup(mainWin->pos() + QPoint(20,20)); break;
						case SDLK_F1: optShow(); break;
						case SDLK_F2: emulPause(true,PR_FILE); saveFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
						case SDLK_F3: emulPause(true,PR_FILE); loadFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
						case SDLK_F4:
								if (zx->tape->flags & TAPE_ON) {
									zx->tape->stop(zx->vid->t);
									emulSetIcon(":/images/logo.png");
								} else {
									if (!zx->tape->startplay()) break;
									emulSetIcon(":/images/play.png");
								}
								break;
						case SDLK_F5:
								if (zx->tape->flags & TAPE_ON) {
									zx->tape->stop(zx->vid->t);
									emulSetIcon(":/images/logo.png");
								} else {
									zx->tape->startrec();
									emulSetIcon(":/images/rec.png");
								}
								break;
						case SDLK_F6: devShow(); break;
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
						case SDLK_F11:			// TODO: when tapeWin will be working, move it to F4
							if (tapeWin->isVisible()) {
								tapeWin->hide();
							} else {
								buildTapeList();
								tapeWin->show();
							}
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
				if (!(emulFlags & FL_GRAB) || (pauseFlags !=0 )) break;
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
// ROMSETS

RomSet* findRomset(std::string nm) {
	RomSet* res = NULL;
	for (uint i=0; i<rsList.size(); i++) {
		if (rsList[i].name == nm) {
			res = &rsList[i];
		}
	}
	return res;
}

bool addRomset(RomSet rs) {
	if (findRomset(rs.name) != NULL) return false;
	rsList.push_back(rs);
	return true;
}

void setRomsetList(std::vector<RomSet> rsl) {
	rsList.clear();
	uint i;
	for (i=0; i<rsl.size(); i++) {
		addRomset(rsl[i]);
	}
	for (i=0; i<profileList.size(); i++) {
		profileList[i].zx->sys->mem->romset = findRomset(profileList[i].zx->opt.rsName);
		profileList[i].zx->sys->mem->loadromset(optGetString(OPT_ROMDIR));
	}
}

void setRomset(ZXComp* comp, std::string nm) {
	zx->sys->mem->romset = findRomset(nm);
}

std::vector<RomSet> getRomsetList() {
	return rsList;
}

//=====================
// HARDWARE

void addHardware(std::string nam, int typ, int msk, int flg) {
	HardWare nhw;
	nhw.name = nam;
	nhw.type = typ;
	nhw.mask = msk;
	nhw.flags = flg;
	hwList.push_back(nhw);
}

void setHardware(ZXComp* comp, std::string nam) {
	for (uint i = 0; i < hwList.size(); i++) {
		if (hwList[i].name == nam) {
			comp->hw = &hwList[i];
			comp->sys->hwflags = comp->hw->flags;
			break;
		}
	}
}

std::vector<std::string> getHardwareNames() {
	std::vector<std::string> res;
	for (uint i=0; i<hwList.size(); i++) {
		res.push_back(hwList[i].name);
	}
	return res;
}

std::vector<HardWare> getHardwareList() {
	return hwList;
}

void initHardware() {
	addHardware("ZX48K",HW_ZX48,0x00,0);
	addHardware("Pentagon",HW_PENT,0x05,0);
	addHardware("Pentagon1024SL",HW_P1024,0x08,0);
	addHardware("Scorpion",HW_SCORP,0x0a,IO_WAIT);
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
	loadConfig(false);
	emulUpdateWindow();
	saveProfiles();
	mainWin->setFocus();
	emulPause(false,PR_EXTRA);
}
