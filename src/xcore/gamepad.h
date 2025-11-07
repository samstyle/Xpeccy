#pragma once

#include <string>
#include <QMap>
#include <QObject>
#include <QKeySequence>
#include <SDL_joystick.h>

// joystick

// use QKeySequence to bind gamepad to pc keyboard, instead of single key
#define USE_SEQ_BIND 1

enum {
	JOY_NONE = 0,
	JOY_AXIS,
	JOY_BUTTON,
	JOY_HAT
};

enum {
	JMAP_NONE = 0,
	JMAP_KEY,
	JMAP_JOY,
	JMAP_JOYB,
	JMAP_MOUSE
};

typedef struct {
	int type;		// axis/button
	int num;		// number of axis/button
	int state;		// -x/+x for axis, 0/x for button
	int dev;		// device for action JMAP_*
#if USE_SEQ_BIND
	QKeySequence seq;	// key sequence to activate
#else
	int key;		// key XKEY_* for keyboard
#endif
	int dir;		// XJ_* for kempston
	int rps;		// repeat state (0:released, !0:pressed)
	int rpt;		// repeat period (0 = no repeat)
	int cnt;		// repeat counter
} xJoyMapEntry;

enum {
	GPBACKEND_NONE = 0,
	GPBACKEND_SDL,
	GPBACKEND_QT
};

class xGamepad : public QObject {
	Q_OBJECT
	public:
		xGamepad(QObject* = nullptr);
		~xGamepad();
		void open(int);		// by index/id
		void open(QString);	// by name
		void open();		// by last name
		void close();
		int isOpened();
		int getId();
		void setDeadZone(int);
		int deadZone();
		QString name(int = -1);		// real name by index (name() is empty if gamepad not opened)
		QString lastName();		// name from config / last successfuly opened gamepad
		void setName(QString);
		QStringList getList();
		static QString getButtonName(int);
		void update();

		int mapSize();
		void mapClear();
		xJoyMapEntry mapItem(int);
		void setItem(int, xJoyMapEntry);
		void delItem(int);

		void loadMap(std::string);
		void saveMap(std::string);
		QList<xJoyMapEntry> scanMap(int, int, int);
		QList<xJoyMapEntry> repTick();
	signals:
		void buttonChanged(int, bool);
		void axisChanged(int, double);
	private:
		int id;
		int lasthat;
		int dead;
		double deadf;
		QString s_name;
		QList<xJoyMapEntry> map;			// gamepad map for gamepad
		QMap<int, QMap<int, int> > jState;	// buttons/axis/hats state: -1 -> 1 | 1 -> -1 = release old one
		SDL_Joystick* sjptr;
};

class xGamepadController : public QObject {
	Q_OBJECT
	public:
		xGamepadController(QObject* = nullptr);
		xGamepad* gpada;
		xGamepad* gpadb;
	protected:
		void timerEvent(QTimerEvent*);
};

// operations with gamepad map files
int padExists(std::string);
int padCreate(std::string);
void padDelete(std::string);
