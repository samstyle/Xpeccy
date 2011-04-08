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
//#include "video.h"
#include "sound.h"
//#include "keyboard.h"
//#include "bdi.h"
//#include "tape.h"
//#include "hdd.h"
//#include "gs.h"

#include "spectrum.h"
//#include "z80.h"
//#include "iosys.h"
//#include "memory.h"

#include "settings.h"
#include "emulwin.h"
#include "debuger.h"
#include "develwin.h"
#include "filer.h"

#ifndef WIN32
	#include <SDL.h>
	#include <SDL_syswm.h>
#endif

#include <zlib.h>
#include <fstream>

std::string int2str(int);
bool str2bool(std::string);
void splitline(std::string,std::string*,std::string*);
std::vector<std::string> splitstr(std::string,const char*);
uint8_t zx_in(int);
void zx_out(int,uint8_t);
void shithappens(const char*);

extern ZXComp* zx;
//extern BDI* bdi;
//extern Keyboard* keyb;
//extern Mouse* mouse;
extern Sound* snd;
extern Settings* sets;
//extern HardWare* hw;
//extern IDE* ide;
//extern GS* gs;
//extern Tape* tape;
extern EmulWin* mwin;
extern DebugWin* dbg;
extern DevelWin* dwin;
extern MFiler* filer;

EmulWin::EmulWin() {
	setcuricon(":/images/logo.png");
	setWindowTitle("Xpeccy 0.4.3");
	setMouseTracking(true);

	pal.clear(); pal.resize(256);
	int i; for(i=0;i<256;i++) {pal[i] = qRgb(zx->vid->pal[i].r,zx->vid->pal[i].g,zx->vid->pal[i].b);}

	fast = false;
	ssnum = ssbcnt = ssbint = 0;
	flags = 0;
	
	mainMenu = new QMenu("Main menu",this);
	bookmarkMenu = new QMenu("Bookmarks",this);
	profileMenu = new QMenu("Profiles",this);
	mainMenu->addMenu(bookmarkMenu);
	mainMenu->addMenu(profileMenu);

	setFixedSize(zx->vid->wsze.h,zx->vid->wsze.v);

	opt.scrshotFormat = "png";
	
#ifndef WIN32

	SDL_SysWMinfo inf;
	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
	embedClient(inf.info.x11.wmwindow);

	opt.sndOutputName = "ALSA";
	opt.scrshotDir = std::string(getenv("HOME"));
	opt.workDir = opt.scrshotDir + "/.config/samstyle/xpeccy";
	opt.romDir = opt.workDir + "/roms";
	opt.optPath = opt.workDir + "/xpeccy.conf";
	mkdir(opt.workDir.c_str(),0777);
	mkdir(opt.romDir.c_str(),0777);

#else

#if !SDLMAINWIN

	SDL_VERSION(&inf.version);
	SDL_GetWMInfo(&inf);
//	SetWindowLong(inf.window,GWL_STYLE,GetWindowLong(inf.window,GWL_STYLE) | WS_CHILD);
	SetParent(inf.window,winId());
	
#endif

	opt.sndOutputName = "WaveOut";
	opt.scrshotDir = std::string(getenv("HOMEPATH"));
	opt.workDir = std::string(".\\config");
	opt.romDir = opt.workDir + "\\roms";
	opt.optPath = opt.workDir + "\\xpeccy.conf";
	mkdir(opt.workDir.c_str());
	mkdir(opt.romDir.c_str());

#endif

	tim1 = new QTimer();
	tim2 = new QTimer();
	QObject::connect(tim1,SIGNAL(timeout()),this,SLOT(emulframe()));
	QObject::connect(tim2,SIGNAL(timeout()),this,SLOT(updateframe()));
	QObject::connect(tim1,SIGNAL(timeout()),this,SLOT(SDLEventHandler()));
	repause(false,-1);
//	setAcceptDrops(true);
}

/*
void EmulWin::dropEvent(QDropEvent* ev) {
printf("Drop\n");
	const QMimeData *mime = ev->mimeData();
	if (!mime->hasUrls()) return;
	QList<QUrl> urls = mime->urls();
	int i; for(i=0;i<urls.size();i++) {
		if (urls[i].isValid()) filer->loadsomefile(urls[i].path().toStdString(),0);
	}
}
*/

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
	SDL_UpdateRect(zx->vid->surf,0,0,0,0);
}

void EmulWin::emulframe() {
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
		snd->sbptr = snd->sndbuf;
		snd->t = zx->vid->t;
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
		std::string fnam = opt.scrshotDir + "/sshot" + int2str(ssnum) + "." + opt.scrshotFormat;
		if (opt.scrshotFormat == "scr") {
			std::ofstream file(fnam.c_str(),std::ios::binary);
			file.write((char*)&zx->sys->mem->ram[zx->vid->curscr ? 7 : 5][0],0x1b00);
		} else {
			QImage *img = new QImage((uchar*)zx->vid->surf->pixels,zx->vid->wsze.h,zx->vid->wsze.v,QImage::Format_Indexed8);
			img->setColorTable(pal);
			if (img==NULL) {
				printf("NULL image\n");
			} else {
				img->save(QString(fnam.c_str()),opt.scrshotFormat.c_str());
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
	} else {
		setWindowIcon(QIcon(":/images/pause.png"));
	}
	if (msk & PR_PAUSE) return;
	if ((paused & ~PR_PAUSE) == 0) {
		zx->vid->flags &= ~VF_BLOCKFULLSCREEN;
		if (zx->vid->flags & VF_FULLSCREEN) zx->vid->update();
	} else {
		zx->vid->flags |= VF_BLOCKFULLSCREEN;
		if (zx->vid->flags & VF_FULLSCREEN) zx->vid->update();
	}
}

void EmulWin::reset() {
	zx->reset();
//	snd->sc1->reset();
//	snd->sc2->reset();
//	snd->scc = snd->sc1;
//	ide->reset();
}

void EmulWin::SDLEventHandler() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				if (ev.key.keysym.mod & KMOD_ALT) {
					switch(ev.key.keysym.sym) {
						case SDLK_0: zx->vid->mode = (zx->vid->mode==VID_NORMAL)?VID_ALCO:VID_NORMAL; break;
						case SDLK_1: zx->vid->flags &= ~VF_DOUBLE; zx->vid->update(); sets->save(); break;
						case SDLK_2: zx->vid->flags |= VF_DOUBLE; zx->vid->update(); sets->save(); break;
						case SDLK_3: fast = !fast;
							tim1->start(fast?0:20);
							break;
						case SDLK_F4: close(); break;
						case SDLK_F7: ssbcnt = sets->sscnt; ssbint=0; break;	// ALT+F7 combo
						case SDLK_RETURN: zx->vid->flags ^= VF_FULLSCREEN; zx->vid->update(); sets->save(); break;
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
						case SDLK_MENU: repause(true,PR_MENU); mainMenu->popup(pos() + QPoint(20,20)); break;
						case SDLK_F1: emit(wannasetup()); break;
						case SDLK_F2: filer->saveonf2(); break;
						case SDLK_F3: filer->opensomewhat(); break;
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
						case SDLK_F9: zx->bdi->flop[0].savecha();
							zx->bdi->flop[1].savecha();
							zx->bdi->flop[2].savecha();
							zx->bdi->flop[3].savecha();
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
					case SDL_BUTTON_RIGHT: if (flags & FL_GRAB) zx->mouse->buttons &= ~0x02; break;
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
	if (!mainMenu->isVisible() && (paused & PR_MENU)) repause(false,PR_MENU);
}

// load rzx

uint32_t getint(std::ifstream* file) {
	uint32_t wrd = file->get();
	wrd += (file->get() << 8);
	wrd += (file->get() << 16);
	wrd += (file->get() << 24);
	return wrd;
}

int zlib_uncompress(uint8_t* in, int ilen, uint8_t* out, int olen) {
	int ret;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = inflateInit(&strm);
	if (ret!= Z_OK) {
		printf("Inflate init error\n");
		return 0;
	}
	strm.avail_in = ilen;		// full len
	strm.next_in = in;
	strm.avail_out = olen;
	strm.next_out = out;
	ret = inflate(&strm,Z_FINISH);
	switch (ret) {
		case Z_NEED_DICT:
//			ret = Z_DATA_ERROR;     /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&strm);
			return 0;
	}
	inflateEnd(&strm);
	return (olen - strm.avail_out);
}

/*
void EmulWin::shithappens(const char* msg) {
	QMessageBox mbx(QMessageBox::Critical,"Shit happens",QString(msg),QMessageBox::Ok);
	mbx.exec();
}
*/

void EmulWin::load(std::string fnam,int typ) {
	std::ifstream file(fnam.c_str());
	std::string onam = opt.workDir + "/lain.tmp";
	std::string extn;
	std::ofstream ofle(onam.c_str());
	if (!file.good()) {
		shithappens("Can't open file");
		return;
	}
	uint8_t* buf = new uint8_t[0x1000000];
	uint8_t* zbuf = new uint8_t[0x100000];
//	uint8_t* bptr;
//	uint32_t frm;
	uint32_t len,wrd,flg;
	uint8_t tmp=0,typz;
	RZXFrame rzxfrm;
	switch (typ) {
		case TYP_RZX:
			rzx.clear();
			file.seekg(0,std::ios_base::end);
			size_t sz = file.tellg();
			file.seekg(0);
			file.read((char*)buf,4);
			if (std::string((char*)buf,4) != "RZX!") {
				shithappens("Wrong signature");
				return;
			}
			file.seekg(2,std::ios_base::cur);	// skip version
			flg = getint(&file);
			bool kk = true;
			do {
				typz = file.get();		// block id
				len = getint(&file);		// block len
				switch(typz) {
					case 0x30:
						flg = getint(&file);
						file.read((char*)buf,4);
						tmp = -1;
						extn = std::string((char*)buf,3);
						if ((extn == "z80") || (extn == "Z80")) tmp = TYP_Z80;
						if ((extn == "sna") || (extn == "SNA")) tmp = TYP_SNA;
						wrd = getint(&file);
						if (flg & 1) {
							printf("External snapshot\n");
							wrd = getint(&file);
							file.read((char*)zbuf,len-21);
							fnam = std::string((char*)zbuf,len-21);
						} else {
							if (flg & 2) {
								file.read((char*)zbuf,len-17);
								wrd = zlib_uncompress(zbuf,len-17,buf,wrd);
							} else {
								file.read((char*)buf,len-17);
								wrd = len-17;
							}
							if (wrd == 0) {
								shithappens("Snapshot unpack error");
								kk = false;
							} else {
								ofle.write((char*)buf,wrd);
								ofle.close();
								zx->sys->mem->load(onam,tmp);		// parse (unpacked) snapshot
							}
						}
						break;
/*					case 0x80:

						kk = false;
						frm = getint(&file);
				printf("%i frames\n",frm);
						tmp = file.get();
						wrd = getint(&file);	// T-states @ begin
						flg = getint(&file);
						if (flg & 1) {
							shithappens("Xpeccy cannot into<br>encrypted RZX frames");
						} else {
							if (flg & 2) {
								file.read((char*)zbuf,len-18);
								wrd = zlib_uncompress(zbuf,len-18,buf,0x1000000);
							} else {
								file.read((char*)buf,len-18);
								wrd = len-18;
							}
							if (wrd == 0) {
								shithappens("Frames unpack error");
							} else {
						//		ofle.open(onam.c_str());
						//		ofle.write((char*)buf,wrd);
						//		ofle.close();
								bptr = buf;
								while (frm > 0) {
									wrd = *bptr + (*(bptr+1) << 8); bptr += 2;
									rzxfrm.fetches = wrd;
									wrd = *bptr + (*(bptr+1) << 8); bptr += 2;	// in count
									if (wrd != 0xffff) {
										rzxfrm.in.clear();
										while (wrd > 0) {
											rzxfrm.in.push_back(*bptr++);
											wrd--;
										}
									}
									rzx.push_back(rzxfrm);
									frm--;
								}
							}
						}
						break; */
					default: file.seekg(len-5,std::ios_base::cur); break;		// skip other blocks
				}
			} while ((file.tellg() < sz) && kk);
			break;
	}
}

//=====================
// MENU

void EmulWin::makeBookmarkMenu() {
	bookmarkMenu->clear();
	uint i;
	QAction *act;
	for (i=0; i<sets->umenu.data.size(); i++) {
		act = bookmarkMenu->addAction(QString(sets->umenu.data[i].name.c_str()));
		act->setToolTip(QString(sets->umenu.data[i].path.c_str()));
	}
	if (bookmarkMenu->isEmpty()) {
		act = bookmarkMenu->addAction("None"); act->setEnabled(false);
	}
	QObject::connect(bookmarkMenu,SIGNAL(triggered(QAction*)),this,SLOT(actmenu(QAction*)));
}

void EmulWin::actmenu(QAction* act) {
	filer->loadsomefile(std::string(act->toolTip().toUtf8().data()),0);
	setFocus();
}

void BookmarkMenu::add(std::string name,std::string path) {
	BookmarkEntry ment;
	ment.name = name;
	ment.path = path;
	data.push_back(ment);
}

void BookmarkMenu::set(int idx,std::string name,std::string path) {
	data[idx].name = name;
	data[idx].path = path;
}

void BookmarkMenu::del(int pos) {
	data.erase(data.begin()+pos);
}

void BookmarkMenu::swap(int ps1, int ps2) {
	BookmarkEntry ment = data[ps1];
	data[ps1] = data[ps2];
	data[ps2] = ment;
}

//=====================
// SETTINGS
//=====================

Settings::Settings() {
#ifndef WIN32
// move config dir to new place
	QDir dir;
	QString newpath = QDir::homePath() + "/.config/samstyle/xpeccy";
	if (!dir.exists(newpath)) {
		QFile file;
		QString oldpath = QDir::homePath() + "/.samstyle/samulator";
		QString oldfile = oldpath + "/samulator.conf";
		QString newfile = newpath + "/xpeccy.conf";
		dir.mkpath(newpath);
		file.rename(oldfile,newfile);
		dir.rename(oldpath + "/roms",newpath + "/roms");
		dir.rmdir(oldpath);
	}
// set new paths
//	ssdir = std::string(getenv("HOME"));
//	std::string mydir = ssdir + "/.config/samstyle";
#endif
}

void Settings::save() {
	std::ofstream sfile(mwin->opt.optPath.c_str());
	if (!sfile.good()) {
		shithappens("Can't open settings file");
		throw(0);
	}
	uint32_t i,j;

	sfile<<"[CPU]\n\n";
	sfile<<"# real cpu freq in MHz = this value / 2; correct range is 2 to 14 (1 to 7 MHz)\n";
	sfile<<"cpu.frq = "<<int2str((int)(zx->sys->cpu->frq * 2.0)).c_str()<<"\n";
	
	sfile<<"\n[VIDEO]\n\n";
	sfile<<"doublesize = "<<((zx->vid->flags & VF_DOUBLE)?"y":"n")<<"\n";
	sfile<<"fullscreen = "<<((zx->vid->flags & VF_FULLSCREEN)?"y":"n")<<"\n";
	sfile<<"bordersize = "<<int2str((int)(zx->vid->brdsize * 100)).c_str()<<"\n";
	for (i=1; i < zx->vid->layout.size(); i++) {
		sfile << "layout = ";
		sfile << zx->vid->layout[i].name.c_str() << ":";
		sfile << int2str(zx->vid->layout[i].full.h) << ":" << int2str(zx->vid->layout[i].full.v) << ":";
		sfile << int2str(zx->vid->layout[i].bord.h) << ":" << int2str(zx->vid->layout[i].bord.v) << ":";
		sfile << int2str(zx->vid->layout[i].sync.h) << ":" << int2str(zx->vid->layout[i].sync.v) << ":";
		sfile << int2str(zx->vid->layout[i].intsz) << ":" << int2str(zx->vid->layout[i].intpos) << "\n";
	}
	sfile<<"geometry = "<<zx->vid->curlay.c_str()<<"\n";

	sfile << "\n[SCREENSHOTS]\n\n";
	sfile << "folder = " << mwin->opt.scrshotDir.c_str() << "\n";
	sfile << "format = " << mwin->opt.scrshotFormat.c_str() << "\n";
	sfile << "combo.count = " << int2str(sscnt).c_str() << "\n";
	sfile << "combo.interval = " << int2str(ssint).c_str() << "\n";

	sfile << "\n[SOUND]\n\n";
	sfile << "enabled = " << (snd->enabled ? "y" : "n") << "\n";
	sfile<<"# possible sound systems are:";
	for (i=0;i<snd->outsyslist.size();i++) {
		if (i!=0) sfile<<", ";
		sfile<<snd->outsyslist[i].name.c_str();
	}
	sfile<<"\n";
	sfile<<"soundsys = "<<snd->outsys->name.c_str()<<"\n";
	sfile<<"dontmute = "<<(snd->mute?"y":"n")<<"\n";
	sfile<<"rate = "<<int2str(snd->rate).c_str()<<"\n";
	sfile<<"volume.beep = "<<int2str(snd->beepvol).c_str()<<"\n";
	sfile<<"volume.tape = "<<int2str(snd->tapevol).c_str()<<"\n";
	sfile<<"volume.ay = "<<int2str(snd->ayvol).c_str()<<"\n";
	sfile<<"volume.gs = "<<int2str(snd->gsvol).c_str()<<"\n";
	sfile<<"chip1 = "<<int2str(zx->aym->sc1->type)<<"\n";
	sfile<<"chip2 = "<<int2str(zx->aym->sc2->type)<<"\n";
	sfile<<"chip1.stereo = "<<int2str(zx->aym->sc1->stereo)<<"\n";
	sfile<<"chip2.stereo = "<<int2str(zx->aym->sc2->stereo)<<"\n";
	sfile<<"ts.type = "<<int2str(zx->aym->tstype)<<"\n";
	sfile<<"gs = "<<((zx->gs->flags & GS_ENABLE)?"y":"n")<<"\n";
	sfile<<"gs.reset = "<<((zx->gs->flags & GS_RESET)?"y":"n")<<"\n";
	sfile<<"gs.stereo = "<<int2str(zx->gs->stereo)<<"\n";

	sfile<<"\n[BETADISK]\n\n";
	sfile<<"enabled = "<<(zx->bdi->enable?"y":"n")<<"\n";
	sfile<<"fast = "<<(zx->bdi->vg93.turbo?"y":"n")<<"\n";

	sfile<<"\n[IDE]\n\n";
	sfile<<"iface = "<<int2str(zx->ide->iface)<<"\n";
	sfile<<"master.type = "<<int2str(zx->ide->master.iface)<<"\n";
	sfile<<"master.model = "<<zx->ide->master.pass.model.c_str()<<"\n";
	sfile<<"master.serial = "<<zx->ide->master.pass.serial.c_str()<<"\n";
	sfile<<"master.image = "<<zx->ide->master.image.c_str()<<"\n";
	sfile<<"master.lba = "<<(zx->ide->master.canlba?"y":"n")<<"\n";
	sfile<<"master.chs = "<<int2str(zx->ide->master.pass.spt)<<"/"<<int2str(zx->ide->master.pass.hds)<<"/"<<int2str(zx->ide->master.pass.cyls)<<"\n";
	sfile<<"slave.type = "<<int2str(zx->ide->slave.iface)<<"\n";
	sfile<<"slave.model = "<<zx->ide->slave.pass.model.c_str()<<"\n";
	sfile<<"slave.serial = "<<zx->ide->slave.pass.serial.c_str()<<"\n";
	sfile<<"slave.image = "<<zx->ide->slave.image.c_str()<<"\n";
	sfile<<"slave.lba = "<<(zx->ide->slave.canlba?"y":"n")<<"\n";
	sfile<<"slave.chs = "<<int2str(zx->ide->slave.pass.spt)<<"/"<<int2str(zx->ide->slave.pass.hds)<<"/"<<int2str(zx->ide->slave.pass.cyls)<<"\n";

	sfile<<"\n[MACHINE]\n\n";
	sfile<<"# possible values:";
	for (i=0; i < zx->hwlist.size(); i++) {if (i!=0) sfile<<", "; sfile << zx->hwlist[i].name.c_str();}
	sfile<<"\n";
	sfile<<"current = "<<zx->hw->name.c_str()<<"\n";
	sfile<<"memory = ";
	switch (zx->sys->mem->mask) {
		case 0x07: sfile<<"128\n"; break;
		case 0x0f: sfile<<"256\n"; break;
		case 0x1f: sfile<<"512\n"; break;
		case 0x3f: sfile<<"1024\n"; break;
		default: sfile<<"48\n"; break;
	}
	sfile<<"restart = "<<(zx->sys->io->resafter?"y":"n")<<"\n";
	sfile << "scrp.wait = "<< ((zx->sys->hwflags & WAIT_ON) ? "y" : "n") << "\n";

	sfile<<"\n[ROMSETS]\n\n";
	std::vector<std::string> rmnam;
	rmnam.push_back("basic128"); rmnam.push_back("basic48"); rmnam.push_back("shadow"); rmnam.push_back("trdos");
	rmnam.push_back("ext4"); rmnam.push_back("ext5"); rmnam.push_back("ext6"); rmnam.push_back("ext7");
	for (i=0;i < zx->sys->mem->rsetlist.size();i++) {
		sfile<<"name = " << zx->sys->mem->rsetlist[i].name.c_str()<<"\n";
		for (j=0;j<8;j++) {
			if (zx->sys->mem->rsetlist[i].roms[j].path!="") {
				sfile<<rmnam[j].c_str()<<" = " << zx->sys->mem->rsetlist[i].roms[j].path.c_str();
				if (zx->sys->mem->rsetlist[i].roms[j].part!=0) sfile<<":"<<int2str(zx->sys->mem->rsetlist[i].roms[j].part).c_str();
				sfile<<"\n";
			}
		}
		sfile<<"\n";
	}
	sfile << "gs = "	<< zx->opt.GSRom.c_str()		<< "\n";
	sfile << "current = "	<< zx->sys->mem->romset->name.c_str()	<< "\n";
	sfile << "reset = "	<< rmnam[zx->sys->mem->res].c_str()	<< "\n";

	sfile << "\n[TOOLS]\n\n";
	sfile << "sjasm = "		<< dwin->opt.asmPath.c_str()	<< "\n";
	sfile << "projectsdir = "	<< dwin->opt.projectsDir.c_str()<< "\n";

	sfile<<"\n[MENU]\n\n";
	for (i=0; i<umenu.data.size(); i++) {
		sfile<<umenu.data[i].name.c_str()<<" = "<<umenu.data[i].path.c_str()<<"\n";
	}
}

void Settings::load(bool dev) {
	std::ifstream file(mwin->opt.optPath.c_str());
	std::string line,pnam,pval;
	std::vector<std::string> vect;
	size_t pos;
	char* buf = new char[0x4000];
	int tmask = 0xff;
	if (!dev) zx->sys->mem->mask = 0;
	if (!file.good()) {
		shithappens("Can't find config file<br><b>~/.config/samstyle/xpeccy/xpeccy.conf</b><br>Default one will be created.");
		std::ofstream ofile(mwin->opt.optPath.c_str());
		ofile << "[MACHINE]\n\ncurrent = ZX48K\nmemory = 48\n\n";
		ofile << "[BETADISK]\n\nenabled = n\n\n";
		ofile << "[ROMSETS]\n\nname = ZX48\nbasic48 = 1982.rom\n\ncurrent = ZX48\nreset = basic48\n";
		ofile.close();
		QFile fle(":/rom/1982.rom");
		fle.copy(QString(std::string(mwin->opt.romDir + "/1982.rom").c_str()));
		file.open(mwin->opt.optPath.c_str(),std::ifstream::in);
	}
	if (!file.good()) {
		shithappens("Damn! I can't open config file<br>Zetsuboushita!");
		throw(0);
	} else {
		RomSet newrs;
		VidLayout vlay;
		int tmp2=0;
		int test;
		std::string fnam,tms;
		int fprt;
		while (!file.eof()) {
			file.getline(buf,2048);
			line = std::string(buf);
			splitline(line,&pnam,&pval);
			if (pval=="") {
				if (pnam=="[ROMSETS]") tmp2=1;
				if (pnam=="[VIDEO]") tmp2=2;
				if (pnam=="[SCREENSHOTS]") tmp2=3;
				if (pnam=="[SOUND]") tmp2=4;
				if (pnam=="[BETADISK]") tmp2=5;
				if (pnam=="[MACHINE]") tmp2=6;
				if (pnam=="[TOOLS]") tmp2=7;
				if (pnam=="[MENU]") tmp2=8;
				if (pnam=="[IDE]") tmp2=9;
				if (pnam=="[GENERAL]") tmp2=10;
				if (dev && (tmp2 != 7)) tmp2 = 0;
			} else {
				switch (tmp2) {
					case 1:
						pos = pval.find_last_of(":");
						if (pos != std::string::npos) {
							fnam = std::string(pval,0,pos);
							tms = std::string(pval,pos+1);
							if (tms=="") {fprt = 0;} else {fprt = atoi(tms.c_str());}
						} else {fnam = pval; fprt = 0;}
						if (pnam=="name") {
							newrs.name = pval;
							zx->sys->mem->rsetlist.push_back(newrs);
						}
						if ((pnam=="basic128") || (pnam=="0")) {
							zx->sys->mem->rsetlist.back().roms[0].path=fnam;
							zx->sys->mem->rsetlist.back().roms[0].part=fprt;}
						if ((pnam=="basic48") || (pnam=="1")) {
							zx->sys->mem->rsetlist.back().roms[1].path=fnam;
							zx->sys->mem->rsetlist.back().roms[1].part=fprt;}
						if ((pnam=="shadow") || (pnam=="2")) {
							zx->sys->mem->rsetlist.back().roms[2].path=fnam;
							zx->sys->mem->rsetlist.back().roms[2].part=fprt;}
						if ((pnam=="trdos") || (pnam=="3")) {
							zx->sys->mem->rsetlist.back().roms[3].path=fnam;
							zx->sys->mem->rsetlist.back().roms[3].part=fprt;}
						if (pnam=="reset") {
							if ((pval=="basic128") || (pval=="0")) zx->sys->mem->res = 0;
							if ((pval=="basic48") || (pval=="1")) zx->sys->mem->res = 1;
							if ((pval=="shadow") || (pval=="2")) zx->sys->mem->res = 2;
							if ((pval=="trdos") || (pval=="3")) zx->sys->mem->res = 3;
						}
						if (pnam=="current") zx->opt.romsetName = pval;
						if (pnam=="gs") zx->opt.GSRom = pval;
						break;
					case 2: if (pnam=="doublesize") {
							if (str2bool(pval))
								zx->vid->flags |= VF_DOUBLE;
							else
								zx->vid->flags &= ~VF_DOUBLE;
						}
						if (pnam=="fullscreen") {
							if (str2bool(pval))
								zx->vid->flags |= VF_FULLSCREEN;
							else
								zx->vid->flags &= ~VF_FULLSCREEN;
						}
						if (pnam=="bordersize") {test = atoi(pval.c_str()); if ((test>-1) && (test<101)) zx->vid->brdsize = test/100.0;}
						if (pnam=="layout") {
							vect = splitstr(pval,":");
//for(uint i=0;i<vect.size();i++) printf("%s\t",vect[i].c_str());
//printf("\n");
							if (vect.size() == 9) {
//printf("oga 8\n");
								vlay.name = vect[0];
								vlay.full.h = atoi(vect[1].c_str()); vlay.full.v = atoi(vect[2].c_str());
								vlay.bord.h = atoi(vect[3].c_str()); vlay.bord.v = atoi(vect[4].c_str());
								vlay.sync.h = atoi(vect[5].c_str()); vlay.sync.v = atoi(vect[6].c_str());
								vlay.intsz = atoi(vect[7].c_str()); vlay.intpos = atoi(vect[8].c_str());
//printf("%s\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n",vlay.name.c_str(),vlay.full.h,vlay.full.v,vlay.bord.h,vlay.bord.v,vlay.sync.h,vlay.sync.v,vlay.intsz);
								if ((vlay.full.h > vlay.bord.h + 256) && (vlay.bord.h > vlay.sync.h) && (vlay.full.v > vlay.bord.v + 192) && (vlay.bord.v > vlay.sync.v)) {
									zx->vid->layout.push_back(vlay);
								}
							}
						}
						if (pnam=="geometry") zx->vid->curlay = pval;
						break;
					case 3: if (pnam=="folder") mwin->opt.scrshotDir = pval;
						if (pnam=="format") mwin->opt.scrshotFormat = pval;
						if (pnam=="combo.count") sscnt=atoi(pval.c_str());
						if (pnam=="combo.interval") ssint=atoi(pval.c_str());
						break;
					case 4: if (pnam=="enabled") snd->enabled=str2bool(pval);
						if (pnam=="dontmute") snd->mute=str2bool(pval);
						if (pnam=="soundsys") mwin->opt.sndOutputName = pval;
						if (pnam=="rate") snd->rate = atoi(pval.c_str());
						if (pnam=="volume.beep") {snd->beepvol=atoi(pval.c_str()); if (snd->beepvol > 16) snd->beepvol = 16;}
						if (pnam=="volume.tape") {snd->tapevol = atoi(pval.c_str()); if (snd->tapevol > 16) snd->tapevol = 16;}
						if (pnam=="volume.ay") {snd->ayvol = atoi(pval.c_str()); if (snd->ayvol > 16) snd->ayvol = 16;}
						if (pnam=="volume.gs") {snd->gsvol = atoi(pval.c_str()); if (snd->gsvol > 16) snd->gsvol = 16;}
						if (pnam=="chip1") {test = atoi(pval.c_str()); if (test < SND_END) zx->aym->sc1->settype(test);}
						if (pnam=="chip2") {test = atoi(pval.c_str()); if (test < SND_END) zx->aym->sc2->settype(test);}
						if (pnam=="chip1.stereo") zx->aym->sc1->stereo = atoi(pval.c_str());
						if (pnam=="chip2.stereo") zx->aym->sc2->stereo = atoi(pval.c_str());
						if (pnam=="ts.type") zx->aym->tstype = atoi(pval.c_str());
						if (pnam=="gs") {
							if (str2bool(pval)) zx->gs->flags |= GS_ENABLE; else zx->gs->flags &= ~GS_ENABLE;
						}
						if (pnam=="gs.reset") {
							if (str2bool(pval)) zx->gs->flags |= GS_RESET; else zx->gs->flags &= ~GS_RESET;
						}
						if (pnam=="gs.stereo") zx->gs->stereo = atoi(pval.c_str());
						break;
					case 5: if (pnam=="enabled") zx->bdi->enable=str2bool(pval);
						if (pnam=="fast") zx->bdi->vg93.turbo=str2bool(pval);
						break;
					case 6: if (pnam=="current") zx->opt.hwName = pval;
						if (pnam=="memory") {
							if (pval=="48") {zx->sys->mem->mask = 0x00; tmask = 0;}
							if (pval=="128") {zx->sys->mem->mask = 0x07; tmask = 1;}
							if (pval=="256") {zx->sys->mem->mask = 0x0f; tmask = 2;}
							if (pval=="512") {zx->sys->mem->mask = 0x1f; tmask = 4;}
							if (pval=="1024") {zx->sys->mem->mask = 0x3f; tmask = 8;}
						}
						if (pnam=="restart") zx->sys->io->resafter = str2bool(pval);
						if (pnam=="scrp.wait") {
							if (str2bool(pval)) zx->sys->hwflags |= WAIT_ON; else zx->sys->hwflags &= ~WAIT_ON;
						}
						break;
					case 7: if (pnam=="sjasm") dwin->opt.asmPath = pval;
						if (pnam=="projectsdir") dwin->opt.projectsDir = pval;
						break;
					case 8: umenu.add(pnam,pval);
						break;
					case 9:
						if (pnam=="iface") zx->ide->iface = atoi(pval.c_str());
						if (pnam=="master.type") zx->ide->master.iface = atoi(pval.c_str());
						if (pnam=="master.model") zx->ide->master.pass.model = std::string(pval,0,40);
						if (pnam=="master.serial") zx->ide->master.pass.serial = std::string(pval,0,20);
						if (pnam=="master.lba") zx->ide->master.canlba = str2bool(pval);
						if (pnam=="master.image") zx->ide->master.image = pval;
						if (pnam=="master.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								zx->ide->master.pass.spt = atoi(vect.at(0).c_str());
								zx->ide->master.pass.hds = atoi(vect.at(1).c_str());
								zx->ide->master.pass.cyls = atoi(vect.at(2).c_str());
							}
						}
						if (pnam=="slave.type") zx->ide->slave.iface = atoi(pval.c_str());
						if (pnam=="slave.model") zx->ide->slave.pass.model = std::string(pval,0,40);
						if (pnam=="slave.serial") zx->ide->slave.pass.serial = std::string(pval,0,20);
						if (pnam=="slave.lba") zx->ide->slave.canlba = str2bool(pval);
						if (pnam=="slave.image") zx->ide->slave.image = pval;
						if (pnam=="slave.chs") {
							vect = splitstr(pval,"/");
							if (vect.size() > 2) {
								zx->ide->slave.pass.spt = atoi(vect.at(0).c_str());
								zx->ide->slave.pass.hds = atoi(vect.at(1).c_str());
								zx->ide->slave.pass.cyls = atoi(vect.at(2).c_str());
							}
						}
						break;
					case 10:
						if (pnam=="cpu.frq") {
							fprt = atoi(pval.c_str());
							if ((fprt > 0) && (fprt < 14)) zx->sys->cpu->frq = fprt / 2.0;
						}
						break;
				}
			}
		}
	}
	if (dev) return;
	snd->defpars();
	zx->setHardware(zx->opt.hwName);
	zx->sys->mem->setromptr(zx->opt.romsetName);
	snd->setoutptr(mwin->opt.sndOutputName);
	if (zx->hw==NULL) throw("Can't found current machine");
	if (zx->sys->mem->romset==NULL) throw("Can't found current romset");
	if (~zx->hw->mask & tmask) throw("Incorrect memory size for this machine");
	if (!zx->vid->setlayout(zx->vid->curlay)) zx->vid->setlayout("default");
	zx->vid->update();
	zx->sys->mem->loadromset();
	mwin->makeBookmarkMenu();
}
