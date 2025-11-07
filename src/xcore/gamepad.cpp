#include <QDebug>

#if USE_QT_GAMEPAD
#include <QGamepadManager>
#endif

#include <SDL_events.h>
#include <SDL_joystick.h>

#include <stdio.h>

#include "xcore.h"
#include "gamepad.h"

#define VIRTKEYBASE 12

typedef struct {
	char ch;
	int val;
} xCharDir;

const xCharDir kjoyChars[] = {
	{'U', XJ_UP},
	{'D', XJ_DOWN},
	{'L', XJ_LEFT},
	{'R', XJ_RIGHT},
	{'F', XJ_FIRE},
	{'2', XJ_BUT2},
	{'3', XJ_BUT3},
	{'4', XJ_BUT4},
	{'A', XJ_FIRE},	// nes buttons: a,b,start,select
	{'B', XJ_BUT2},
	{'S', XJ_BUT3},
	{'O', XJ_BUT4},
	{'-', XJ_NONE}
};

const xCharDir kmouChars[] = {
	{'U', XM_UP},
	{'D', XM_DOWN},
	{'L', XM_LEFT},
	{'R', XM_RIGHT},
	{'[', XM_LMB},
	{'|', XM_MMB},
	{']', XM_RMB},
	{'^', XM_WHEELUP},
	{'v', XM_WHEELDN},
	{'-', XM_NONE}
};

const xCharDir hatChars[] = {
	{'U', SDL_HAT_UP},
	{'D', SDL_HAT_DOWN},
	{'L', SDL_HAT_LEFT},
	{'R', SDL_HAT_RIGHT},
	{'-', 0}
};

const xCharDir pabhChars[] = {
	{'A', JOY_AXIS},
	{'B', JOY_BUTTON},
	{'H', JOY_HAT},
	{'-', JOY_NONE}
};

const xCharDir devChars[] = {
	{'K', JMAP_KEY},
	{'J', JMAP_JOY},
	{'B', JMAP_JOYB},
	{'M', JMAP_MOUSE},
	{'-', JMAP_NONE}
};

char padGetChar(int val, const xCharDir* tab) {
	int idx = 0;
	while ((tab[idx].val > 0) && (tab[idx].val != val))
		idx++;
	return tab[idx].ch;
}

int padGetId(char ch, const xCharDir* tab) {
	int idx = 0;
	while ((tab[idx].val > 0) && (tab[idx].ch != ch))
		idx++;
	return tab[idx].val;
}

int xGamepad::mapSize() {
	return map.size();
}

void xGamepad::mapClear() {
	map.clear();
}

xJoyMapEntry xGamepad::mapItem(int i) {
	return map[i];
}

void xGamepad::setItem(int i, xJoyMapEntry xjm) {
	if ((i < 0) || (i >= map.size())) {
		map.append(xjm);
	} else {
		map[i] = xjm;
	}
}

void xGamepad::delItem(int i) {
	if (i < 0) return;
	if (i >= map.size()) return;
	map.erase(map.begin() + i);
}

void xGamepad::loadMap(std::string mapname) {
	if (mapname.empty()) return;
	xJoyMapEntry jent;
	FILE* file;
	int num;
	int idx;
	char buf[1024];
	char* ptr;

	std::string path = conf.path.confDir + SLASH + mapname;
	file = fopen(path.c_str(), "rb");
	if (file) {
		map.clear();
		while(!feof(file)) {
			memset(buf, 0x00, 1024);
			fgets(buf, 1023, file);
			ptr = strtok(buf, ":\n");
			if (ptr) {
				jent.type = padGetId(ptr[0], pabhChars);
				jent.num = atoi(&ptr[1]);
				idx = 1;
				num = 0;
				while ((ptr[idx] >= '0') && (ptr[idx] <= '9')) {
					num = num * 10 + ptr[idx] - '0';
					idx++;
				}
				jent.num = num;
				// get direction for axis & hat
				switch(jent.type) {
					case JOY_AXIS:		// A0+ A0-
						jent.state = (ptr[idx] == '-') ? -1 : +1;
						idx++;
						break;
					case JOY_BUTTON:
						break;
					case JOY_HAT:		// HU HD HR HL
						jent.type = JOY_BUTTON;				// convert hat->button for xGamepad
						switch(ptr[idx]) {
							case 'U': jent.num = VIRTKEYBASE; break;
							case 'D': jent.num = VIRTKEYBASE+1; break;
							case 'L': jent.num = VIRTKEYBASE+2; break;
							case 'R': jent.num = VIRTKEYBASE+3; break;
							default: jent.type = JOY_HAT; jent.state = padGetId(ptr[idx], hatChars); break;
						}
						idx++;
						break;
				}
				ptr = strtok(NULL, ":\n");
				if (ptr) {		// there was 1st :
					jent.dev = padGetId(ptr[0], devChars);
					idx = 1;
					switch (jent.dev) {
						case JMAP_KEY:		// KUP, KLEFT, KQ, KA
#if USE_SEQ_BIND
							jent.seq = QKeySequence::fromString(QString(&ptr[1]));
							if (jent.seq.isEmpty())
								jent.dev = JMAP_NONE;
#else
							jent.key = getKeyIdByName(&ptr[1]);
							if (jent.key == ENDKEY)
								jent.dev = JMAP_NONE;
#endif
							break;
						case JMAP_JOY:		// JU, JD, JL, JR, JF, J2, J3, J4
						case JMAP_JOYB:
							jent.dir = padGetId(ptr[1], kjoyChars);
							break;
						case JMAP_MOUSE:	// MD, ML, M[ M| M] M^ Mv
							jent.dir = padGetId(ptr[1], kmouChars);
							break;
						default:
							jent.dev = JMAP_NONE;	// ignore it
							break;
					}
					jent.rpt = 0;
					jent.cnt = 0;
					ptr = strtok(NULL, ":\n");
					if (ptr)
						jent.rpt = atoi(ptr);
					if (jent.dev != JMAP_NONE)
						map.push_back(jent);
				}
			}
		}
		fclose(file);
	}
}

void xGamepad::saveMap(std::string mapname) {
	if (mapname.empty()) return;
	std::string path = conf.path.confDir + SLASH + mapname;
	FILE* file;
	file = fopen(path.c_str(), "wb");
	if (file) {
		foreach(xJoyMapEntry jent, map) {
			fprintf(file, "%c%i", padGetChar(jent.type, pabhChars), jent.num);
			switch (jent.type) {
				case JOY_AXIS:
					fputc((jent.state < 0) ? '-' : '+', file);
					break;
				case JOY_HAT:
					fputc(padGetChar(jent.state, hatChars), file);
					break;
			}
			fprintf(file, ":%c", padGetChar(jent.dev, devChars));
			switch(jent.dev) {
				case JMAP_KEY:
#if USE_SEQ_BIND
					fprintf(file, "%s", jent.seq.toString().toUtf8().data());
#else
					fprintf(file, "%s", getKeyNameById(jent.key));
#endif
					break;
				case JMAP_JOY:
				case JMAP_JOYB:
					fputc(padGetChar(jent.dir, kjoyChars), file);
					break;
				case JMAP_MOUSE:
					fputc(padGetChar(jent.dir, kmouChars), file);
					break;
				default:
					fprintf(file, "?");
			}
			if (jent.rpt > 0)
				fprintf(file, ":%i", jent.rpt);
			fputc('\n', file);
		}
		fclose(file);
	}
}

int padExists(std::string name) {
	std::string path = conf.path.confDir + SLASH + name;
	FILE* file = fopen(path.c_str(), "rb");
	if (!file) return 0;
	fclose(file);
	return 1;
}

int padCreate(std::string name) {
	if (padExists(name)) return 0;
	std::string path = conf.path.confDir + SLASH + name;
	FILE* file = fopen(path.c_str(), "wb");
	if (!file) return 0;
	fclose(file);
	return 1;
}

void padDelete(std::string name) {
	std::string path = conf.path.confDir + SLASH + name;
	remove(path.c_str());
}

// xGamepad
xGamepad::xGamepad(QObject* p):QObject(p) {
	lasthat = 0;
	id = -1;
	dead = 8192;
	deadf = 8192 / 32768.0;
}

xGamepad::~xGamepad() {
	close();
}

void xGamepad::open(int devid) {
	close();
	sjptr = SDL_JoystickOpen(devid);
	if (sjptr) {
		id = SDL_JoystickInstanceID(sjptr);
	}
}

void xGamepad::open(QString name) {
	QStringList lst = getList();
	int idx = lst.indexOf(name);
	if (idx >= 0) {
		open(idx);
		s_name = name;
	}
}

void xGamepad::open() {
	open(s_name);
}

void xGamepad::close() {
	if (id < 0) return;
	if (sjptr) {
		SDL_JoystickClose(sjptr);
	}
	id = -1;
}

int xGamepad::isOpened() {
	return !(id < 0);
}

int xGamepad::getId() {
	return id;
}

int sign(int v) {
	if (v < 0) return -1;
	if (v > 0) return 1;
	return 0;
}

QList<xJoyMapEntry> xGamepad::scanMap(int type, int num, int st) {
	QList<xJoyMapEntry> presslist;
	int state;
	int hst;
	if (type == JOY_HAT) {
		state = st;
		hst = jState[type][num] ^ st;		// changed only
	} else {
		state = sign(st);
		hst = 0;
	}
	if (jState[type][num] == state) return presslist;
	jState[type][num] = state;
	for (int i = 0; i < map.size(); i++) {
		xJoyMapEntry& xjm = map[i];
		if ((type == xjm.type) && (num == xjm.num)) {
			if ((state == 0) && (type != JOY_HAT)) {
				xjm.cnt = 0;
				xjm.rps = 0;
				presslist.append(xjm);
			} else {
				switch(type) {
					case JOY_AXIS:
						if (sign(state) == sign(xjm.state)) {
							xjm.state = st;
							xjm.cnt = xjm.rpt;
							xjm.rps = 1;
						} else {
							xjm.cnt = 0;
							xjm.rps = 0;
						}
						presslist.append(xjm);
						break;
					case JOY_HAT:
						if (hst & xjm.state) {			// state changed
							if (state & xjm.state) {	// pressed
								xjm.cnt = xjm.rpt;
								xjm.rps = 1;
							} else {			// released
								xjm.cnt = 0;
								xjm.rps = 0;
							}
							presslist.append(xjm);
						}
						break;
					case JOY_BUTTON:
						xjm.cnt = xjm.rpt;
						xjm.rps = 1;
						presslist.append(xjm);
						break;
				}
			}
		}
	}
	return presslist;
}

QList<xJoyMapEntry> xGamepad::repTick() {
	QList<xJoyMapEntry> presslist;
	for (int i = 0; i < map.size(); i++) {
		xJoyMapEntry& xjm = map[i];
		if (xjm.cnt > 0) {
			xjm.cnt--;
			if (xjm.cnt == 0) {
				xjm.cnt = xjm.rpt;
				xjm.rps = !xjm.rps;
				presslist.append(xjm);
			}
		}
	}
	return presslist;
}

#define GP_USESDLEVENTS 0 || !HAVESDL2

// TODO: Axis 4,5 (triggers): Qt: 0->32767; SDL: -32768->32767 (allways catched as negative)
// TODO: Don't poll events, check buttons/axis/hats state, on changes map immediately and signal(xJoyMapEntry)
// update(): check buttons/axis/hats changing and emit signals
void xGamepad::update() {
	if (id > -1) {
#if GP_USESDLEVENTS
		SDL_Event ev;
		SDL_JoystickUpdate();
		double v;
		// WARNING: SDL_PollEvent gets ALL events (for ALL gamepads). Need to filter for current gamepad (!)
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_JOYAXISMOTION:
					//if (ev.jaxis.which != id) break;	// need return ev to queue ???
					v = ev.jaxis.value / 32768.0;
					if (abs(v) < deadf) v = 0;
					emit axisChanged(ev.jaxis.axis, v);
					break;
				case SDL_JOYBUTTONDOWN:
					//if (ev.jbutton.which != id) break;
					emit buttonChanged(ev.jbutton.button, true);
					break;
				case SDL_JOYBUTTONUP:
					//if (ev.jbutton.which != id) break;
					emit buttonChanged(ev.jbutton.button, false);
					break;
				case SDL_JOYHATMOTION:
					//if (ev.jhat.which != id) break;
					lasthat ^= ev.jhat.value;	// bit n = 1 -> changed
					if (lasthat & SDL_HAT_UP) emit buttonChanged(VIRTKEYBASE + ev.jhat.hat * 4, !!(ev.jhat.value & SDL_HAT_UP));
					if (lasthat & SDL_HAT_DOWN) emit buttonChanged(VIRTKEYBASE + 1 + ev.jhat.hat * 4, !!(ev.jhat.value & SDL_HAT_DOWN));
					if (lasthat & SDL_HAT_LEFT) emit buttonChanged(VIRTKEYBASE + 2 + ev.jhat.hat * 4, !!(ev.jhat.value & SDL_HAT_LEFT));
					if (lasthat & SDL_HAT_RIGHT) emit buttonChanged(VIRTKEYBASE + 3 + ev.jhat.hat * 4, !!(ev.jhat.value & SDL_HAT_RIGHT));
					lasthat = ev.jhat.value;
					break;
					// SDL: gamepad connect/disconnect events
#if HAVESDL2
					// TODO: select device, if there is more than one
				case SDL_JOYDEVICEREMOVED:
					emit deviceRemoved(ev.jdevice.which);
					break;
				case SDL_JOYDEVICEADDED:
					emit deviceAdded(QString(SDL_JoystickNameForIndex(ev.jdevice.which)));
					break;
#endif
			}
		}
#else
		// hats (init counter here, logic is below)
		int h = SDL_JoystickNumHats(sjptr);
		// buttons
		int n = SDL_JoystickNumButtons(sjptr);
		// clamp if HAT is present: skip virtual D-Pad buttons (>=VIRTKEYBASE)
		if (h > 0 && n > VIRTKEYBASE) n = VIRTKEYBASE;
		int state;
		while (n > 0) {
			n--;
			state = SDL_JoystickGetButton(sjptr, n);
			if (jState[JOY_BUTTON][n] != state) {
				emit buttonChanged(n, state);
			}
		}
		// axis
		n = SDL_JoystickNumAxes(sjptr);
		while (n > 0) {
			n--;
			state = SDL_JoystickGetAxis(sjptr, n);
			if (abs(state) < dead)
				state = 0;
			state = sign(state);
			if (jState[JOY_AXIS][n] != state) {
				emit axisChanged(n, state);
			}
		}
		// hats
		while (h > 0) {
			h--;
			state = SDL_JoystickGetHat(sjptr, h);
			lasthat ^= state;	// bit n = 1 -> changed
			if (lasthat & SDL_HAT_UP) emit buttonChanged(VIRTKEYBASE + h * 4, !!(state & SDL_HAT_UP));
			if (lasthat & SDL_HAT_DOWN) emit buttonChanged(VIRTKEYBASE + 1 + h * 4, !!(state & SDL_HAT_DOWN));
			if (lasthat & SDL_HAT_LEFT) emit buttonChanged(VIRTKEYBASE + 2 + h * 4, !!(state & SDL_HAT_LEFT));
			if (lasthat & SDL_HAT_RIGHT) emit buttonChanged(VIRTKEYBASE + 3 + h * 4, !!(state & SDL_HAT_RIGHT));
			lasthat = state;
		}
	}
#endif
}

QString xGamepad::name(int devid) {
	QString nm;
	if (devid < 0) devid = id;
#if HAVESDL2
	nm = QString(SDL_JoystickNameForIndex(devid));
#else
	nm = QString(SDL_JoystickName(sjptr));
#endif
	return nm;
}

QString xGamepad::lastName() {
	return s_name;
}

void xGamepad::setName(QString nm) {
	s_name = nm;
}

QStringList xGamepad::getList() {
	QStringList lst;
	int id, cnt;
	cnt = SDL_NumJoysticks();
	for(id = 0; id < cnt; id++) {
		lst << SDL_JoystickNameForIndex(id);
	}
	return lst;
}

QString xGamepad::getButtonName(int n) {
	QString nm;
	if (n < VIRTKEYBASE) {
		nm = QString("Button %0").arg(n);
	} else {
		n -= VIRTKEYBASE;
		switch(n & 3) {
			case 0: nm = QString("Hat %0 up").arg(n >> 2); break;
			case 1: nm = QString("Hat %0 down").arg(n >> 2); break;
			case 2: nm = QString("Hat %0 left").arg(n >> 2); break;
			case 3: nm = QString("Hat %0 right").arg(n >> 2); break;
		}
	}
	return nm;
}

void xGamepad::setDeadZone(int v) {
	if (v < 0) return;
	if (v > 32768) return;
	dead = v;
	deadf = v / 32768.0;
}

int xGamepad::deadZone() {return dead;}

// controller

xGamepadController::xGamepadController(QObject* p):QObject(p) {
	gpada = new xGamepad;
	gpadb = new xGamepad;
	startTimer(20);
}

void xGamepadController::timerEvent(QTimerEvent* e) {
	gpada->update();
	gpadb->update();
#ifdef HAVESDL2
	SDL_Event ev;
	QString nm;
	int idx;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_JOYDEVICEREMOVED:
				//emit deviceRemoved(ev.jdevice.which);
				idx = ev.jdevice.which;
				if ((idx == gpada->getId()) && gpada->isOpened()) {
					gpada->close();
				} else if ((idx == gpadb->getId()) && gpadb->isOpened()) {
					gpadb->close();
				}
				break;
			case SDL_JOYDEVICEADDED:
				// emit deviceAdded(QString(SDL_JoystickNameForIndex(ev.jdevice.which)));
				nm = SDL_JoystickNameForIndex(ev.jdevice.which);
				if (nm == gpada->lastName() && !gpada->isOpened()) {
					gpada->open();
				} else if (nm == gpadb->lastName() && !gpadb->isOpened()) {
					gpadb->open();
				}
				break;
		}
	}
#endif
}
