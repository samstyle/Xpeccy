#include <QDebug>

#include <QGamepadManager>

#include <SDL_events.h>
#include <SDL_joystick.h>

#include <stdio.h>

#include "gamepad.h"

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

void padLoadConfig(std::string name) {
	if (name.empty()) return;
	xJoyMapEntry jent;
	// char path[FILENAME_MAX];
	FILE* file;
	int num;
	int idx;
	char buf[1024];
	char* ptr;

	std::string path = conf.path.confDir + SLASH + name;
	file = fopen(path.c_str(), "rb");
	if (file) {
		conf.joy.map.clear();
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
						jent.state = padGetId(ptr[idx], hatChars);
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
						case JMAP_JOY:		// JU, JD, JF, J2, J4
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
						conf.joy.map.push_back(jent);
				}
			}
		}
		fclose(file);
	}
}

void padSaveConfig(std::string name) {
	if (name.size() == 0) return;
	std::string path = conf.path.confDir + SLASH + name;
	FILE* file;
	file = fopen(path.c_str(), "wb");
	if (file) {
		foreach(xJoyMapEntry jent, conf.joy.map) {
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
xGamepad::xGamepad(int t, QObject* p):QObject(p) {
	lasthat = 0;
	id = -1;
#if USE_QT_GAMEPAD
	qjptr = new QGamepad;
#endif
	setType(t);
}

xGamepad::~xGamepad() {
	close();
}

void xGamepad::open(int devid) {
	close();
	id = -1;
#if USE_QT_GAMEPAD
	QList<int> gpidlist;
#endif
	switch (type) {
#if USE_QT_GAMEPAD
		case GPBACKEND_QT:
			gpidlist = QGamepadManager::instance()->connectedGamepads();
			if (gpidlist.size() > devid) {
				id = devid;
				qjptr->setDeviceId(gpidlist.at(devid));
			}
			qDebug() << "Qt :" << conf.joy.gpad->name();
			break;
#endif
		case GPBACKEND_SDL:
			sjptr = SDL_JoystickOpen(devid);
			if (sjptr) {
				id = devid;
				startTimer(20);
			}
			qDebug() << "SDL :" << conf.joy.gpad->name();
			break;
	}
}

void xGamepad::close() {
	if (id < 0) return;
	switch(type) {
#if USE_QT_GAMEPAD
		case GPBACKEND_QT:
			disconnect(qjptr);
			qjptr->setDeviceId(0);
			break;
#endif
		case GPBACKEND_SDL:
			if (sjptr) {
				killTimer(stid);
				SDL_JoystickClose(sjptr);
			}
			break;
	}
	id = -1;
	qDebug() << "Gamepad closed";
}

// TODO: Axis 4,5 (triggers): Qt: 0->32767; SDL: -32768->32767 (allways catched as negative)
void xGamepad::timerEvent(QTimerEvent* e) {
	if ((type == GPBACKEND_SDL) && (id > -1)) {
		SDL_Event ev;
		SDL_JoystickUpdate();
		double v;
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_JOYAXISMOTION:
					v = ev.jaxis.value / 32768.0;
					if (abs(v) < conf.joy.deadf) v = 0;
					emit axisChanged(ev.jaxis.axis, v);
					break;
				case SDL_JOYBUTTONDOWN:
					emit buttonChanged(ev.jbutton.button, true);
					break;
				case SDL_JOYBUTTONUP:
					emit buttonChanged(ev.jbutton.button, false);
					break;
				case SDL_JOYHATMOTION:
					lasthat ^= ev.jhat.value;	// bit n = 1 -> changed
					if (lasthat & SDL_HAT_UP) emit buttonChanged(12, !!(ev.jhat.value & SDL_HAT_UP));
					if (lasthat & SDL_HAT_DOWN) emit buttonChanged(13, !!(ev.jhat.value & SDL_HAT_DOWN));
					if (lasthat & SDL_HAT_LEFT) emit buttonChanged(14, !!(ev.jhat.value & SDL_HAT_LEFT));
					if (lasthat & SDL_HAT_RIGHT) emit buttonChanged(15, !!(ev.jhat.value & SDL_HAT_RIGHT));
					lasthat = ev.jhat.value;
					break;
// SDL: gamepad connect/disconnect events
#if HAVESDL2
				// TODO: select device, if there is more than one
				case SDL_JOYDEVICEREMOVED:
				case SDL_JOYDEVICEADDED:
					if (ev.jdevice.which != 0) break;
					close();
					if (SDL_NumJoysticks() > 0) {
						open(id);
					}
					break;
#endif
			}
		}
	}
}

QString xGamepad::name(int devid) {
	QString nm;
	if (devid < 0) devid = id;
	QList<int> devlst;
	switch(type) {
#if USE_QT_GAMEPAD
		case GPBACKEND_QT:
			devlst = QGamepadManager::instance()->connectedGamepads();
			if (devlst.size() > devid) {
				nm = QGamepadManager::instance()->gamepadName(devlst.at(devid));
			}
			break;
#endif
		case GPBACKEND_SDL:
			nm = QString(SDL_JoystickNameForIndex(devid));
			break;
	}
	return nm;
}

QStringList xGamepad::getList() {
	QStringList lst;
	int id, cnt;
	QList<int> devlist;
	switch(type) {
#if USE_QT_GAMEPAD
		case GPBACKEND_QT:
			devlist = QGamepadManager::instance()->connectedGamepads();
			foreach(int id, devlist) {
				lst << QGamepadManager::instance()->gamepadName(id);
			}
			break;
#endif
		case GPBACKEND_SDL:
			cnt = SDL_NumJoysticks();
			for(id = 0; id < cnt; id++) {
				lst << SDL_JoystickNameForIndex(id);
			}
			break;
	}
	return lst;
}

void xGamepad::setType(int t) {
	close();
	switch(t) {
#if USE_QT_GAMEPAD
		case GPBACKEND_QT:
			type = t;
			connect(qjptr, &QGamepad::buttonAChanged, this, &xGamepad::BAChanged);
			connect(qjptr, &QGamepad::buttonBChanged, this, &xGamepad::BBChanged);
			connect(qjptr, &QGamepad::buttonXChanged, this, &xGamepad::BXChanged);
			connect(qjptr, &QGamepad::buttonYChanged, this, &xGamepad::BYChanged);
			connect(qjptr, &QGamepad::buttonL1Changed, this, &xGamepad::BL1Changed);
			connect(qjptr, &QGamepad::buttonL3Changed, this, &xGamepad::BL3Changed);
			connect(qjptr, &QGamepad::buttonR1Changed, this, &xGamepad::BR1Changed);
			connect(qjptr, &QGamepad::buttonR3Changed, this, &xGamepad::BR3Changed);
			connect(qjptr, &QGamepad::buttonUpChanged, this, &xGamepad::BUChanged);
			connect(qjptr, &QGamepad::buttonDownChanged, this, &xGamepad::BDChanged);
			connect(qjptr, &QGamepad::buttonLeftChanged, this, &xGamepad::BLChanged);
			connect(qjptr, &QGamepad::buttonRightChanged, this, &xGamepad::BRChanged);
			connect(qjptr, &QGamepad::buttonStartChanged, this, &xGamepad::BStChanged);
			connect(qjptr, &QGamepad::buttonSelectChanged, this, &xGamepad::BSeChanged);
			connect(qjptr, &QGamepad::buttonCenterChanged, this, &xGamepad::BCeChanged);
			connect(qjptr, &QGamepad::buttonGuideChanged, this, &xGamepad::BGuChanged);
			connect(qjptr, &QGamepad::axisLeftXChanged, this, &xGamepad::ALXChanged);
			connect(qjptr, &QGamepad::axisLeftYChanged, this, &xGamepad::ALYChanged);
			connect(qjptr, &QGamepad::axisRightXChanged, this, &xGamepad::ARXChanged);
			connect(qjptr, &QGamepad::axisRightYChanged, this, &xGamepad::ARYChanged);
			connect(qjptr, &QGamepad::buttonL2Changed, this, &xGamepad::AL2Changed);
			connect(qjptr, &QGamepad::buttonR2Changed, this, &xGamepad::AR2Changed);
			break;
#endif
		default:		// case GPBACKEND_SDL:
			type = GPBACKEND_SDL;
			break;
	}
}

int xGamepad::getType() {
	return type;
}

#if USE_QT_GAMEPAD
void xGamepad::BAChanged(bool b) {emit buttonChanged(0, b);}
void xGamepad::BBChanged(bool b) {emit buttonChanged(1, b);}
void xGamepad::BXChanged(bool b) {emit buttonChanged(2, b);}
void xGamepad::BYChanged(bool b) {emit buttonChanged(3, b);}
void xGamepad::BL1Changed(bool b) {emit buttonChanged(4, b);}
void xGamepad::BR1Changed(bool b) {emit buttonChanged(5, b);}
void xGamepad::BSeChanged(bool b) {emit buttonChanged(6, b);}
void xGamepad::BStChanged(bool b) {emit buttonChanged(7, b);}
void xGamepad::BCeChanged(bool b) {emit buttonChanged(8, b);}
void xGamepad::BL3Changed(bool b) {emit buttonChanged(9, b);}
void xGamepad::BR3Changed(bool b) {emit buttonChanged(10, b);}
void xGamepad::BGuChanged(bool b) {emit buttonChanged(11, b);}
void xGamepad::BUChanged(bool b) {emit buttonChanged(12, b);}
void xGamepad::BDChanged(bool b) {emit buttonChanged(13, b);}
void xGamepad::BLChanged(bool b) {emit buttonChanged(14, b);}
void xGamepad::BRChanged(bool b) {emit buttonChanged(15, b);}

void xGamepad::ALXChanged(double v) {emit axisChanged(0, (absd(v) < conf.joy.deadf) ? 0 : v);}
void xGamepad::ALYChanged(double v) {emit axisChanged(1, (absd(v) < conf.joy.deadf) ? 0 : v);}
void xGamepad::ARXChanged(double v) {emit axisChanged(2, (absd(v) < conf.joy.deadf) ? 0 : v);}
void xGamepad::ARYChanged(double v) {emit axisChanged(3, (absd(v) < conf.joy.deadf) ? 0 : v);}
void xGamepad::AL2Changed(double v) {emit axisChanged(4, (absd(v) < conf.joy.deadf) ? 0 : v);}
void xGamepad::AR2Changed(double v) {emit axisChanged(5, (absd(v) < conf.joy.deadf) ? 0 : v);}
#endif
