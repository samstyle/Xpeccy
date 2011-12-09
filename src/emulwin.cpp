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
	#undef main
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

#ifdef XQTPAINT
	#include <QPainter>
	QImage scrImg(100,100,QImage::Format_Indexed8);
#endif

#ifndef WIN32
	#include <SDL.h>
	#include <SDL_timer.h>
	#include <SDL_syswm.h>
#endif

#include <fstream>

#define	XPTITLE	"Xpeccy 0.4.999"

extern ZXComp* zx;
extern EmulWin* mwin;
// main
MainWin* mainWin;
QIcon curicon;
#ifndef XQTPAINT
	SDL_Surface* surf = NULL;
	SDL_Color zxpal[256];
	SDL_SysWMinfo inf;
	SDL_TimerID tid;
#endif
QVector<QRgb> qPal;
int emulFlags;
int pauseFlags;
int wantedWin;
uint32_t scrNumber;
uint32_t scrCounter;
uint32_t scrInterval;
bool breakFrame = false;
// tape window
Ui::TapeWin tapeUi;
QDialog* tapeWin;
int lastTapeFlags;
int lastTapeBlock;
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
	emulFlags = 0;
	wantedWin = WW_NONE;

	lastTapeFlags = 0;
	lastTapeBlock = 0;
	
	scrNumber = 0;
	scrCounter = 0;
	scrInterval = 0;
	optSet(OPT_SHOTFRM,SCR_PNG);

	int par[] = {448,320,138,80,64,32,64,0};
	addLayout("default",par);
	
	emulSetColor(0xc0);
	mainWin = new MainWin;
}

void setTapeCheck() {
	QTableWidgetItem* itm;
	for (int i=0; i < tapGet(zx->tape, TAPE_BLOCKS); i++) {
		itm = new QTableWidgetItem;
		if (tapGet(zx->tape,TAPE_BLOCK) == i) itm->setIcon(QIcon(":/images/checkbox.png"));
		tapeUi.tapeList->setItem(i,0,itm);
		itm = new QTableWidgetItem;
		if (tapGet(zx->tape,i,TAPE_BFLAG) & TBF_BREAK) itm->setIcon(QIcon(":/images/cancel.png"));
		tapeUi.tapeList->setItem(i,1,itm);
	}
}

void buildTapeList() {
	std::vector<TapeBlockInfo> tinf = tapGetBlocksInfo(zx->tape);
	tapeUi.tapeList->setRowCount(tinf.size());
	QTableWidgetItem* itm;
	std::string tims;
	for (uint i=0; i<tinf.size(); i++) {
		itm = new QTableWidgetItem;
		tims = getTimeString(tinf[i].time);
		itm = new QTableWidgetItem(QString(tims.c_str()));
		tapeUi.tapeList->setItem(i,2,itm);
		itm = new QTableWidgetItem(QDialog::trUtf8(tinf[i].name.c_str()));
		tapeUi.tapeList->setItem(i,3,itm);
	}
	setTapeCheck();
}

void emulShow() {
	mainWin->show();
}

QWidget* emulWidget() {
	return (QWidget*)mainWin;
}

void emulSetColor(int brl) {
	int i;
	qPal.clear(); qPal.resize(256);
#ifndef XQTPAINT
	for (i=0; i<16; i++) {
		zxpal[i].b = (i & 1) ? ((i & 8) ? 0xff : brl) : 0;
		zxpal[i].r = (i & 2) ? ((i & 8) ? 0xff : brl) : 0;
		zxpal[i].g = (i & 4) ? ((i & 8) ? 0xff : brl) : 0;
	}
	for (i=0; i<256; i++) {
		qPal[i] = qRgb(zxpal[i].r,zxpal[i].g,zxpal[i].b);
	}
#else
	uint8_t r,g,b;
	for (i=0; i<16; i++) {
		b = (i & 1) ? ((i & 8) ? 0xff : brl) : 0;
		r = (i & 2) ? ((i & 8) ? 0xff : brl) : 0;
		g = (i & 4) ? ((i & 8) ? 0xff : brl) : 0;
		qPal[i] = qRgb(r,g,b);
	}
	scrImg.setColorTable(qPal);
#endif
}

void emulUpdateWindow() {
	mainWin->updateWindow();
}

void MainWin::updateWindow() {
	emulFlags |= FL_BLOCK;
	vidUpdate(zx->vid);
	int szw = zx->vid->wsze.h;
	int szh = zx->vid->wsze.v;
#ifndef XQTPAINT
	int sdlflg = SDL_SWSURFACE;
	if ((zx->vid->flags & VF_FULLSCREEN) && !(zx->vid->flags & VF_BLOCKFULLSCREEN)) {
		sdlflg |= SDL_FULLSCREEN;
	}
#endif
//printf("szw.h.v = %i\t%i\n",szw,szh);
	setFixedSize(szw,szh);
#ifdef XQTPAINT
	scrImg = scrImg.scaled(szw,szh);
	zx->vid->scrimg = scrImg.bits();
#else
	surf = SDL_SetVideoMode(szw,szh,8,sdlflg | SDL_NOFRAME);
	SDL_SetPalette(surf,SDL_LOGPAL|SDL_PHYSPAL,zxpal,0,256);
//printf("surface size = %i\t%i\t%i\n",surf->w,surf->h,surf->pitch);
//printf("window size = %i\t%i\n",size().width(),size().height());
	zx->vid->scrimg = (uint8_t*)surf->pixels;
#endif
	zx->vid->scrptr = zx->vid->scrimg;
	emulFlags &= ~FL_BLOCK;
}

bool emulSaveChanged() {
	bool yep = saveChangedDisk(0);
	yep &= saveChangedDisk(1);
	yep &= saveChangedDisk(2);
	yep &= saveChangedDisk(3);
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

double tks = 0;

void emulExec() {
	tks += zx->exec();
	tks = sndSync(tks);
	if (!dbgIsActive()) {
		// somehow catch CPoint
		Z80EX_WORD pc = z80ex_get_reg(zx->cpu,regPC);
		if (dbgFindBreakpoint(BPoint(memGet(zx->mem,(pc < 0x4000) ? MEM_ROM : MEM_RAM), pc)) != -1) {
			wantedWin = WW_DEBUG;
			breakFrame = true;
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
#ifdef XQTPAINT
		mainWin->releaseMouse();
#else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
#endif
	}
	if ((pauseFlags == 0) && kk) {
#ifdef XQTPAINT
		mainWin->grabMouse(QCursor(Qt::BlankCursor));
#else
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(SDL_DISABLE);
#endif
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
#ifndef XQTPAINT
	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
	embedClient(inf.info.x11.wmwindow);
#endif
	initUserMenu((QWidget*)this);
	tapeWin = new QDialog((QWidget*)this,Qt::Tool);
	tapeUi.setupUi(tapeWin);
	tapeUi.stopBut->setEnabled(false);
	tapeUi.tapeList->setColumnWidth(0,20);
	tapeUi.tapeList->setColumnWidth(1,20);
	tapeUi.tapeList->setColumnWidth(2,50);
	timer = new QTimer();
	QObject::connect(timer,SIGNAL(timeout()),this,SLOT(emulFrame()));
}

#ifdef XQTPAINT
void MainWin::paintEvent(QPaintEvent *ev) {
	QPainter pnt;
	pnt.begin(this);
	pnt.drawImage(0,0,scrImg);
}

void MainWin::keyPressEvent(QKeyEvent *ev) {
	if (ev->modifiers() & Qt::AltModifier) {
		switch(ev->key()) {
			case Qt::Key_0: zx->vid->mode = (zx->vid->mode==VID_NORMAL)?VID_ALCO:VID_NORMAL; break;
			case Qt::Key_1:
				zx->vid->flags &= ~VF_DOUBLE;
				mainWin->updateWindow();
				saveConfig();
				break;
			case Qt::Key_2:
				zx->vid->flags |= VF_DOUBLE;
				mainWin->updateWindow();
				saveConfig();
				break;
			case Qt::Key_3: emulFlags ^= FL_FAST;
				mainWin->stopTimer();
				mainWin->startTimer((emulFlags & FL_FAST) ? 1 : 20);
				break;
			case Qt::Key_F4:
				mainWin->close();
				break;
			case Qt::Key_F7:
				scrCounter = optGetInt(OPT_SHOTCNT);
				scrInterval=0;
				break;	// ALT+F7 combo
			case Qt::Key_F12:
				zx->reset(RES_DOS);
				break;
		}
	} else {
		keyPress(zx->keyb,ev->nativeScanCode());
		switch(ev->key()) {
			case Qt::Key_Pause: pauseFlags ^= PR_PAUSE; emulPause(true,0); break;
			case Qt::Key_Escape: wantedWin = WW_DEBUG; break;
			case Qt::Key_Menu: emulPause(true,PR_MENU); userMenu->popup(mainWin->pos() + QPoint(20,20)); break;
			case Qt::Key_F1: optShow(); break;
			case Qt::Key_F2: emulPause(true,PR_FILE); saveFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
			case Qt::Key_F3: emulPause(true,PR_FILE); loadFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
			case Qt::Key_F4:
				if (tapGet(zx->tape,TAPE_FLAGS) & TAPE_ON) {
						mwin->tapeStop();
					} else {
						mwin->tapePlay();
					}
					break;
			case Qt::Key_F5:
					if (tapGet(zx->tape,TAPE_FLAGS) & TAPE_ON) {
						mwin->tapeStop();
					} else {
						mwin->tapeRec();
					}
					break;
			case Qt::Key_F6: devShow(); break;
			case Qt::Key_F7: if (scrCounter == 0) {emulFlags |= FL_SHOT;} else {emulFlags &= ~FL_SHOT;} break;
			case Qt::Key_F9: emulPause(true,PR_FILE);
				emulSaveChanged();
//				zx->bdi->flop[0].savecha();
//				zx->bdi->flop[1].savecha();
//				zx->bdi->flop[2].savecha();
//				zx->bdi->flop[3].savecha();
				emulPause(false,PR_FILE);
				break;
			case Qt::Key_F10:
				zx->nmiRequest = true;
				break;
			case Qt::Key_F11:			// TODO: when tapeWin will be working, move it to F4
				if (tapeWin->isVisible()) {
					tapeWin->hide();
				} else {
					buildTapeList();
					tapeWin->show();
				}
				break;
			case Qt::Key_F12: zx->reset(RES_DEFAULT); break;
		}
	}
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	keyRelease(zx->keyb,ev->nativeScanCode());
}

void MainWin::mousePressEvent(QMouseEvent *ev){
	switch (ev->button()) {
		case Qt::LeftButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons &= ~0x01;
			break;
		case Qt::RightButton:
			if (emulFlags & FL_GRAB) {
				zx->mouse->buttons &= ~0x02;
			} else {
				emulPause(true,PR_MENU);
				userMenu->popup(QPoint(ev->globalX(),ev->globalY()));
			}
			break;
		default: break;
	}
}

void MainWin::mouseReleaseEvent(QMouseEvent *ev) {
	if (pauseFlags != 0) return;
	switch (ev->button()) {
		case Qt::LeftButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons |= 0x01;
			break;
		case Qt::RightButton:
			if (emulFlags & FL_GRAB) zx->mouse->buttons |= 0x02;
			break;
		case Qt::MidButton:
			emulFlags ^= FL_GRAB;
			if (emulFlags & FL_GRAB) {
				grabMouse(QCursor(Qt::BlankCursor));
			} else {
				releaseMouse();
			}
			break;
		default: break;
	}
}

void MainWin::mouseMoveEvent(QMouseEvent *ev) {
	if (!(emulFlags & FL_GRAB) || (pauseFlags !=0 )) return;
	zx->mouse->xpos = ev->globalX() & 0xff;
	zx->mouse->ypos = 256 - (ev->globalY() & 0xff);
}

#endif

void MainWin::closeEvent(QCloseEvent* ev) {
	timer->stop();
	if (emulSaveChanged()) {
		ev->accept();
	} else {
		ev->ignore();
#ifndef XQTPAINT
		SDL_VERSION(&inf.version);
		SDL_GetWMInfo(&inf);
		mainWin->embedClient(inf.info.x11.wmwindow);
#endif
		timer->start(20);
	}
}

void MainWin::startTimer(int iv) {timer->start(iv);}
void MainWin::stopTimer() {timer->stop();}

// ...

char hobHead[] = {'s','c','r','e','e','e','n',' ','C',0,0,0,0x1b,0,0x1b,0xe7,0x81};	// last 2 bytes is crc

void MainWin::emulFrame() {
	if (emulFlags & FL_BLOCK) return;
	breakFrame = false;
	if (!mainWin->isActiveWindow()) {
		keyRelease(zx->keyb,0);
		zx->mouse->buttons = 0xff;
	}
	if ((wantedWin == WW_NONE) && (pauseFlags == 0)) {
		if (!(emulFlags & FL_FAST) && sndGet(SND_ENABLE) && (sndGet(SND_MUTE) || mainWin->isActiveWindow())) {
			sndPlay();
		}
		sndSet(SND_COUNT,0);
		do {
			emulExec();
		} while ((wantedWin == WW_NONE) && !zx->intStrobe);
//		if (zx->rzxPlay) {
//			zx->mem->rzxFrame++;
//			zx->mem->rzxPos = 0;
//		}
		zx->nmiRequest = false;
		if (scrCounter !=0) {
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
			int frm = optGetInt(OPT_SHOTFRM);
			std::string fext;
			switch (frm) {
				case SCR_BMP: fext = "bmp"; break;
				case SCR_JPG: fext = "jpg"; break;
				case SCR_PNG: fext = "png"; break;
				case SCR_SCR: fext = "scr"; break;
				case SCR_HOB: fext = "$C"; break;
			};
			std::string fnam = optGetString(OPT_SHOTDIR) + "/sshot" + int2str(scrNumber) + "." + fext;
			std::ofstream file;
#ifdef XQTPAINT
			QImage *img = new QImage(scrImg);
#else
			QImage *img = new QImage((uchar*)surf->pixels,surf->w,surf->h,QImage::Format_Indexed8);
#endif
			img->setColorTable(qPal);
			char* pageBuf = new char[0x4000];
			memGetPage(zx->mem,MEM_RAM,zx->vid->curscr ? 7 : 5,pageBuf);
			switch (frm) {
				case SCR_HOB:
					file.open(fnam.c_str(),std::ios::binary);
					file.write((char*)hobHead,17);
					file.write(pageBuf,0x1b00);
					file.close();
					break;
				case SCR_SCR:
					file.open(fnam.c_str(),std::ios::binary);
					file.write(pageBuf,0x1b00);
					file.close();
					break;
				case SCR_BMP:
				case SCR_JPG:
				case SCR_PNG:
					if (img != NULL) img->save(QString(fnam.c_str()),fext.c_str());
					break;
			}
			free(pageBuf);
			emulFlags &= ~FL_SHOT;
			scrNumber++;
		}
	}
	return;
}

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
	QObject::connect(tapeUi.tapeList,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(tapeRewind(int,int)));
	QObject::connect(tapeUi.tapeList,SIGNAL(cellClicked(int,int)),this,SLOT(setTapeStop(int,int)));
	emulPause(false,-1);
}

void EmulWin::tapePlay() {
	if (!tapPlay(zx->tape)) return;
	emulSetIcon(":/images/play.png");
	tapeUi.playBut->setEnabled(false);
	tapeUi.recBut->setEnabled(false);
	tapeUi.stopBut->setEnabled(true);
}

void EmulWin::tapeRec() {
	tapRec(zx->tape);
	emulSetIcon(":/images/rec.png");
	tapeUi.playBut->setEnabled(false);
	tapeUi.recBut->setEnabled(false);
	tapeUi.stopBut->setEnabled(true);
	tapeUi.tapeBar->setValue(0);
}

void EmulWin::tapeStop() {
	tapStop(zx->tape);
	emulSetIcon(":/images/logo.png");
	tapeUi.playBut->setEnabled(true);
	tapeUi.recBut->setEnabled(true);
	tapeUi.stopBut->setEnabled(false);
	tapeUi.tapeBar->setValue(0);
}

void EmulWin::tapeRewind(int row, int) {
	if (row < 0) return;
	tapRewind(zx->tape, row);
//	zx->tape->block = row;
//	zx->tape->pos = 0;
	setTapeCheck();
}

void EmulWin::setTapeStop(int row, int col) {
	if ((row < 0) || (col != 1)) return;
	int flg = tapGet(zx->tape,row,TAPE_BFLAG);
	flg ^= TBF_BREAK;
	tapSet(zx->tape,row,TAPE_BFLAG,flg);
	setTapeCheck();
}

void EmulWin::tapeLoad() {
	emulPause(true,PR_FILE);
	loadFile("",FT_TAPE,-1);
	buildTapeList();
	emulPause(false,PR_FILE);
}

void EmulWin::SDLEventHandler() {
#ifndef XQTPAINT
	SDL_UpdateRect(surf,0,0,0,0);
#else
	mainWin->update();
#endif
	if (emulFlags & FL_BLOCK) return;
	switch (wantedWin) {
		case WW_DEBUG: dbgShow(); wantedWin = WW_NONE; break;
	}
	int blk = tapGet(zx->tape,TAPE_BLOCK);
	int flg = tapGet(zx->tape,TAPE_FLAGS);
	if (lastTapeBlock != blk) {
		lastTapeBlock = blk;
		setTapeCheck();
	}
	if ((tapGet(zx->tape,TAPE_FLAGS) & (TAPE_ON | TAPE_REC)) == TAPE_ON) {
		tapeUi.tapeBar->setMaximum(tapGet(zx->tape,blk,TAPE_BFTIME));			// total
		tapeUi.tapeBar->setValue(tapGet(zx->tape,blk,TAPE_BCTIME));			// current
	}
	if (lastTapeFlags != flg) {
		lastTapeFlags = flg;
		if (lastTapeFlags & TAPE_ON) {
			if (lastTapeFlags & TAPE_REC) {
				tapeUi.tapeBar->setValue(0);
			}
		} else {
			tapeUi.playBut->setEnabled(true);
			tapeUi.recBut->setEnabled(true);
			tapeUi.stopBut->setEnabled(false);
			tapeUi.tapeBar->setValue(0);
		}
	}
#ifndef XQTPAINT
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				if (ev.key.keysym.mod & KMOD_ALT) {
					switch(ev.key.keysym.sym) {
						case SDLK_0: zx->vid->mode = (zx->vid->mode==VID_NORMAL)?VID_ALCO:VID_NORMAL; break;
						case SDLK_1:
 							zx->vid->flags &= ~VF_DOUBLE;
 							mainWin->updateWindow();
							saveConfig();
							break;
						case SDLK_2:
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
						case SDLK_F12:
							zx->reset(RES_DOS);
							break;
						case SDLK_RETURN:
							zx->vid->flags ^= VF_FULLSCREEN;
							mainWin->updateWindow();
							saveConfig();
							break;
						default: break;
					}
				} else {
					keyPress(zx->keyb,ev.key.keysym.scancode);
					switch (ev.key.keysym.sym) {
						case SDLK_PAUSE: pauseFlags ^= PR_PAUSE; emulPause(true,0); break;
						case SDLK_ESCAPE: wantedWin = WW_DEBUG; break;
						case SDLK_MENU: emulPause(true,PR_MENU); userMenu->popup(mainWin->pos() + QPoint(20,20)); break;
						case SDLK_F1: optShow(); break;
						case SDLK_F2: emulPause(true,PR_FILE); saveFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
						case SDLK_F3: emulPause(true,PR_FILE); loadFile("",FT_ALL,-1); emulPause(false,PR_FILE); break;
						case SDLK_F4:
								if (tapGet(zx->tape,TAPE_FLAGS) & TAPE_ON) {
									mwin->tapeStop();
								} else {
									mwin->tapePlay();
								}
								break;
						case SDLK_F5:
								if (tapGet(zx->tape,TAPE_FLAGS) & TAPE_ON) {
									mwin->tapeStop();
								} else {
									mwin->tapeRec();
								}
								break;
						case SDLK_F6: devShow(); break;
						case SDLK_F7: if (scrCounter == 0) {emulFlags |= FL_SHOT;} else {emulFlags &= ~FL_SHOT;} break;
						case SDLK_F9: emulPause(true,PR_FILE);
							emulSaveChanged();
//							zx->bdi->flop[0].savecha();
//							zx->bdi->flop[1].savecha();
//							zx->bdi->flop[2].savecha();
//							zx->bdi->flop[3].savecha();
							emulPause(false,PR_FILE);
							break;
						case SDLK_F10:
							zx->nmiRequest = true;
							break;
						case SDLK_F11:			// TODO: when tapeWin will be working, move it to F4
							if (tapeWin->isVisible()) {
								tapeWin->hide();
							} else {
								buildTapeList();
								tapeWin->show();
							}
							break;
						case SDLK_F12: zx->reset(RES_DEFAULT); break;
						default: break;
					}
				}
				break;
			case SDL_KEYUP:
				keyRelease(zx->keyb,ev.key.keysym.scancode);
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
#endif
	if (!userMenu->isVisible() && (pauseFlags & PR_MENU)) {
		mainWin->setFocus();
		emulPause(false,PR_MENU);
	}
}

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

#ifdef WIN32
#define	SLASHES "\\"
#else
#define	SLASHES "/"
#endif

// set and load memory romset. if rset is NULL, just load current
void emulSetRomset(Memory* mem, RomSet* rset) {
	int i,ad;
	std::string romDir = optGetString(OPT_ROMDIR);
	std::string fpath = "";
	std::ifstream file;
	char* pageBuf = new char[0x4000];
	int prts = 0;
	int profMask = 0;
	if (rset == NULL) {
		rset = memGetRomset(mem);
	} else {
		memSetRomset(mem,rset);
	}
	if (rset == NULL) {
		for (i=0; i<16; i++) {
			for (ad=0; ad<0x4000; ad++) pageBuf[i] = 0xff;
			memSetPage(mem,MEM_ROM,i,pageBuf);
		}
	} else {
		if (rset->file != "") {
			fpath = romDir + SLASHES + rset->file;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.seekg(0,std::ios_base::end);
				prts = file.tellg() / 0x4000;
				profMask = 3;
				if (prts < 9) profMask = 1;
				if (prts < 5) profMask = 0;
				if (prts > 16) prts = 16;
				file.seekg(0,std::ios_base::beg);
				memSet(mem,MEM_PROFMASK,profMask);
				for (i = 0; i < prts; i++) {
					file.read(pageBuf,0x4000);
					memSetPage(mem,MEM_ROM,i,pageBuf);
				}
				for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
				for (i=prts; i<16; i++) memSetPage(mem,MEM_ROM,i,pageBuf);
			} else {
				printf("Can't open single rom '%s'\n",rset->file.c_str());
				for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
				for (i = 0; i < 16; i++) memSetPage(mem,MEM_ROM,i,pageBuf);
			}
			file.close();
		} else {
			for (i = 0; i < 4; i++) {
				if (rset->roms[i].path == "") {
					for (ad = 0; ad < 0x4000; ad++) pageBuf[ad]=0xff;
				} else {
					fpath = romDir + SLASHES + rset->roms[i].path;
					file.open(fpath.c_str(),std::ios::binary);
					if (file.good()) {
						file.seekg(rset->roms[i].part<<14);
						file.read(pageBuf,0x4000);
					} else {
						printf("Can't open rom '%s:%i'\n",rset->roms[i].path.c_str(),rset->roms[i].part);
						for (ad=0;ad<0x4000;ad++) pageBuf[ad]=0xff;
					}
					file.close();
				}
				memSetPage(mem,MEM_ROM,i,pageBuf);
			}
		}
	}
	for (ad = 0; ad < 0x4000; ad++) pageBuf[ad] = 0xff;
	if (zx->opt.GSRom == "") {
		gsSetRom(zx->gs,0,pageBuf);
		gsSetRom(zx->gs,1,pageBuf);
	} else {
			fpath = romDir + SLASHES + zx->opt.GSRom;
			file.open(fpath.c_str(),std::ios::binary);
			if (file.good()) {
				file.read(pageBuf,0x4000);
				gsSetRom(zx->gs,0,pageBuf);
				file.read(pageBuf,0x4000);
				gsSetRom(zx->gs,1,pageBuf);
			} else {
				printf("Can't load gs rom '%s'\n",zx->opt.GSRom.c_str());
				gsSetRom(zx->gs,0,pageBuf);
				gsSetRom(zx->gs,1,pageBuf);
			}
			file.close();
	}
	free(pageBuf);
}

void setRomsetList(std::vector<RomSet> rsl) {
	rsList.clear();
	uint i;
	for (i=0; i<rsl.size(); i++) {
		addRomset(rsl[i]);
	}
	for (i=0; i<profileList.size(); i++) {
		emulSetRomset(profileList[i].zx->mem, findRomset(profileList[i].zx->opt.rsName));
//		profileList[i].zx->mem->loadromset(optGetString(OPT_ROMDIR));
	}
}

void setRomset(ZXComp* comp, std::string nm) {
	emulSetRomset(zx->mem, findRomset(nm));
}

std::vector<RomSet> getRomsetList() {
	return rsList;
}

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
			comp->hwFlags = comp->hw->flags;
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

// OVERALL
