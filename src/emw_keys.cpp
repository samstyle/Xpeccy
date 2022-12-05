#include "emulwin.h"
#include "filer.h"
#include "xcore/xcore.h"
#include "xcore/vscalers.h"
#include "xcore/sound.h"

#include <QMenu>
#include <QFileDialog>

void MainWin::kPress(QKeyEvent* ev) {
	keyPressEvent(ev);
}

void MainWin::kRelease(QKeyEvent* ev) {
	keyReleaseEvent(ev);
}

#ifdef __WIN32
static QMap<qint32, int> key_press_map;
#endif

// TODO: realtime autorepeat on xt keyboard (no runtime emulation)
// debug: no reaction
// keygrab, no autorep: get keyid, xkey_press
// keygrab, autorepeat: get keyid, xt_release, xt_press
// no grab, no autorep: get shortcut/keyid, xkey_press
// no grab, autorepeat: if !shortcut then get keyid, xt_release, xt_press

int ev_to_keyid(QKeyEvent* ev, bool kgrab) {
	int keyid = -1;
	if (!kgrab) {
		keyid = shortcut_check(SCG_MAIN, QKeySequence(ev->key() | ev->modifiers()));
		if (keyid < 0)
			keyid = shortcut_check(SCG_MAIN, QKeySequence(ev->key()));
	}
	if (keyid < 0) {
#if defined(__linux) || defined(__BSD)
			keyid = ev->nativeScanCode();
#elif defined(__WIN32)
			keyid = ev->nativeScanCode();
#if STICKY_KEY
			if (keyid == 0) {
				keyid = ev->nativeVirtualKey();
			} else if ((ev->key() == Qt::Key_Shift) || (ev->key() == Qt::Key_Control) || (ev->key() == Qt::Key_Alt)) {
				keyid = 0;
			}
#endif
			if (key_press_map[keyid] == 0) {	// catch false press events
				key_press_map[keyid] = 1;
			} else {
				keyid = 0;
			}
#else
			keyid = qKey2id(ev->key(), ev->modifiers());
#endif
	}
	return keyid;
}

#if USE_HOST_KEYBOARD

void MainWin::keyPressEvent(QKeyEvent* ev) {
	int keyid;
	keyEntry kent;
	if (comp->debug) {
		ev->ignore();
	} else if (pckAct->isChecked()) {
		keyid = ev_to_keyid(ev, true);
		kent = getKeyEntry(keyid);
		if (ev->isAutoRepeat()) {
			xt_release(comp->keyb, kent);
			xt_press(comp->keyb, kent);
		//} else {
		//	xkey_press(keyid);
		}
	} else {
		keyid = ev_to_keyid(ev, false);
		kent = getKeyEntry(keyid);
		if (ev->isAutoRepeat()) {
			if (keyid < 0x10000) {		// not a shortcut
				xt_release(comp->keyb, kent);
				xt_press(comp->keyb, kent);
			} else {
				xkey_press(keyid);
			}
		} else {
			xkey_press(keyid);
		}
	}
}

#else

void MainWin::keyPressEvent(QKeyEvent *ev) {
	if (ev->isAutoRepeat()) return;
//	qDebug() << "keyPressEvent" << ev->text() << ev->nativeScanCode() << ev->count();
	if (comp->debug) {
		ev->ignore();
	} else {
		int keyid = ev_to_keyid(ev, pckAct->isChecked());
//		printf("press: %i\n", keyid);
		xkey_press(keyid);
	}
}

#endif

void MainWin::xkey_press(int xkey) {
	keyEntry kent = getKeyEntry(xkey);
//	printf("xkey_press %s\n", kent.name);
	int x;
	int y;
	int err;
	QSize wsz;
	QString path;
	if (pckAct->isChecked()) {
		xt_press(comp->keyb, kent);
		if (comp->hw->keyp)
			comp->hw->keyp(comp, kent);
		if (kent.joyMask)
			joyPress(comp->joy, kent.joyMask);
		if (xkey == XKEY_F12) {
			compReset(comp,RES_DEFAULT);
			emit s_rzx_stop();
		}
	} else {
		switch (xkey) {
			case XCUT_FULLSCR:
				vid_set_fullscreen(!conf.vid.fullScreen);
				setMessage(conf.vid.fullScreen ? " fullscreen on " : " fullscreen off ");
				updateWindow();
				saveConfig();
				if (!conf.vid.fullScreen) {
					wsz = SCREENSIZE;
					x = (wsz.width() - width()) / 2;
					y = (wsz.height() - height()) / 2;
					move(x, y);
				}
				break;
			case XCUT_SIZEX1:
				vid_set_zoom(1);
				updateWindow();
				saveConfig();
				setMessage(" size x1 ");
				break;
			case XCUT_SIZEX2:
				vid_set_zoom(2);
				updateWindow();
				saveConfig();
				setMessage(" size x2 ");
				break;
			case XCUT_SIZEX3:
				vid_set_zoom(3);
				updateWindow();
				saveConfig();
				setMessage(" size x3 ");
				break;
			case XCUT_SIZEX4:
				vid_set_zoom(4);
				updateWindow();
				saveConfig();
				setMessage(" size x4 ");
				break;
			case XCUT_COMBOSHOT:
				scrCounter = conf.scrShot.count;
				scrInterval = 0;
				break;
			case XCUT_RES_DOS:
				compReset(comp,RES_DOS);
				emit s_rzx_stop();
				break;
			case XCUT_KEYBOARD:
				emit s_keywin_shide();
				break;
			case XCUT_FAST:
				if (conf.emu.pause) break;
				conf.emu.fast ^= 1;
				updateHead();
				break;
			case XCUT_TURBO:
				if (comp->frqMul < 2) {
					compSetTurbo(comp, 2.0);
					setMessage(" turbo on ");
				} else {
					compSetTurbo(comp, 1.0);
					setMessage(" turbo off ");
				}
				break;
			case XCUT_NOFLICK:
				if (noflic < 15)
					noflic = 25;
				else if (noflic < 35)
					noflic = 50;
				else noflic = 0;
				saveConfig();
				setMessage(QString(" noflick %0% ").arg(noflic * 2));
				break;
			case XCUT_TVLINES:
				scanlines = !scanlines;
				setMessage(scanlines ? " scanlines on " : " scanlines off ");
				saveConfig();
				break;
			case XCUT_RELOAD_SHD:
				loadShader();
				break;
			case XCUT_RATIO:
				vid_set_ratio(!conf.vid.keepRatio);
				updateWindow();
				setMessage(conf.vid.keepRatio ? " keep aspect ratio " : " ignore aspect ratio ");
				saveConfig();
				break;
			case XCUT_MOUSE:
				grabMice = !grabMice;
				if (grabMice) {
					grabMouse(QCursor(Qt::BlankCursor));
					setMessage(" grab mouse ");
				} else {
					releaseMouse();
					setMessage(" release mouse ");
				}
				break;
			case XCUT_GRABKBD:
				pckAct->setChecked(true);
				setMessage(" grab keyboard ");
				break;
			case XCUT_PAUSE:
				conf.emu.pause ^= PR_PAUSE;
				pause(true,0);
				break;
			case XCUT_DEBUG:
				conf.emu.fast = 0;
				pause(true, PR_DEBUG);
				// setUpdatesEnabled(true);
				emit s_debug(comp);
				break;
			case XCUT_MENU:
				fillUserMenu();
				userMenu->popup(pos() + QPoint(20,20));
				userMenu->setFocus();
				break;
			case XCUT_OPTIONS:
				pause(true, PR_OPTS);
				emit s_options(conf.prof.cur);
				break;
			case XCUT_SAVE:
				pause(true,PR_FILE);
				save_file(comp, NULL, FG_ALL, -1);
				pause(false,PR_FILE);
				break;
			case XCUT_LOAD:
				pause(true,PR_FILE);
				load_file(comp, NULL, FG_ALL, -1);
				pause(false,PR_FILE);
				checkState();
				emit s_tape_upd(comp->tape);
				break;
			case XCUT_TAPLAY:
				if (comp->tape->on) {
					tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					tapStateChanged(TW_STATE,TWS_PLAY);
				}
				break;
			case XCUT_TAPREC:
				if (comp->tape->on) {
					tapStateChanged(TW_STATE,TWS_STOP);
				} else {
					tapStateChanged(TW_STATE,TWS_REC);
				}
				break;
			case XCUT_SCRSHOT:
				if (scrCounter == 0) {
					scrCounter = 1;
					scrInterval = 0;
				} else {
					scrCounter = 0;
				}
				break;
			case XCUT_RZXWIN:
				emit s_rzx_show();
				break;
			case XCUT_FASTSAVE:
				pause(true,PR_FILE);
				saveChanged();
				pause(false,PR_FILE);
				break;
			case XCUT_NMI:
				if (comp->rzx.play) break;
				if (comp->cpu->type != CPU_Z80) break;
				comp->nmiRequest = 1;
				break;
			case XCUT_TAPWIN:
				emit s_tape_show();
				break;
			case XCUT_RESET:
				compReset(comp,RES_DEFAULT);
				emit s_rzx_stop();
				break;
			case XCUT_WAV_OUT:
				pause(true, PR_FILE);
				if (conf.snd.wavout) {
					snd_wav_close();
					setMessage("Stop WAV output");
				} else {
					path = QFileDialog::getSaveFileName(this, "Sound output to wav", "", "Wave files (*.wav)",nullptr,QFileDialog::DontUseNativeDialog);
					if (!path.isEmpty()) {
						if (!path.endsWith(".wav", Qt::CaseInsensitive))
							path.append(".wav");
						err = snd_wav_open(path.toLocal8Bit().data());
						if (err == ERR_OK) {
							setMessage("Start WAV output");
						}
					}
				}
				pause(false, PR_FILE);
				break;
			default:
				// printf("%s %c %c\n", kent.name, kent.zxKey.key1, kent.zxKey.key2);
				xt_press(comp->keyb, kent);
				if (comp->hw->keyp)
					comp->hw->keyp(comp, kent);
				if (kent.joyMask)
					joyPress(comp->joy, kent.joyMask);
				break;
		}
	}
	emit s_keywin_upd(comp->keyb);
}

void MainWin::keyReleaseEvent(QKeyEvent *ev) {
	if (ev->isAutoRepeat()) return;
//	if (relskip) {
//		relskip = 0;
//	} else {
//		qDebug() << "keyReleaseEvent" << ev->text() << ev->nativeScanCode();
		int keyid;
		if (comp->debug) {
			ev->ignore();
		} else {
			keyid = -1;
			if (!pckAct->isChecked()) {
				keyid = shortcut_check(SCG_MAIN, QKeySequence(ev->key() | ev->modifiers()));
				if (keyid < 0)
					keyid = shortcut_check(SCG_MAIN, QKeySequence(ev->key()));
			}
			if (keyid < 0) {	// not hotkeys
#if defined(__linux) || defined(__BSD)
				keyid = ev->nativeScanCode();
#elif defined(__WIN32)
				keyid = ev->nativeScanCode();
#if STICKY_KEY
				if (keyid == 0) {
					keyid = ev->nativeVirtualKey();
				} else if ((ev->key() == Qt::Key_Shift) || (ev->key() == Qt::Key_Control) || (ev->key() == Qt::Key_Alt)) {
					keyid = 0;
				}
#endif
				if (key_press_map[keyid] == 0) {
					keyid = 0;
				} else {
					key_press_map[keyid] = 0;
				}
#else
				keyid = qKey2id(ev->key());
#endif
				//		printf("release: %i\n", keyid);
				xkey_release(keyid);
			}
		}
//	}
}

void MainWin::xkey_release(int keyid) {
	keyEntry kent = getKeyEntry(keyid);
	xt_release(comp->keyb, kent);
	if (comp->hw->keyr)
		comp->hw->keyr(comp, kent);
	if (kent.joyMask)
		joyRelease(comp->joy, kent.joyMask);
	emit s_keywin_upd(comp->keyb);
}

void MainWin::calcCoords(QMouseEvent* ev) {
	int x = ((ev->x() - pixSkip) * 256 / xstep) + comp->vid->lcut.x - comp->vid->bord.x;
	int y = (ev->y() * 256 / ystep - topSkip) + comp->vid->lcut.y - comp->vid->bord.y;
#if 0
	setMessage(QString("%0 (%1) : %2 (%3)").arg(x).arg(pixSkip).arg(y).arg(topSkip));
#else
	if ((x >= 0) && (x < comp->vid->scrn.x) && (y >= 0) && (y < comp->vid->scrn.y)) {	// inside screen
		int adr = ((y & 0xc0) << 5) | ((y & 0x38) << 2) | ((y & 7) << 8) | ((x & 0xf8) >> 3) | 0x4000;
		int atr = ((y & 0xf8) << 2) | ((x & 0xf8) >> 3) | 0x5800;
		setMessage(QString(" %0:%1 | %2 ").arg(gethexword(adr)).arg(x & 7).arg(gethexword(atr)));
		emit s_scradr(adr, atr);
	}
#endif
}
