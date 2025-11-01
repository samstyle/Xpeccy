#pragma once

#include <string>
#include <QMap>
#include <QObject>
#include <QKeySequence>
#include <SDL_joystick.h>

// joystick

// use QKeySequence to bind gamepad to pc keyboard, instead of single key
#define USE_SEQ_BIND 1

// QGamepad -> Qt5.7+
#define USE_QT_GAMEPAD 0 && (QT_VERSION >= QT_VERSION_CHECK(5,7,0)) && (QT_VERSION <= QT_VERSION_CHECK(6,0,0))

#if USE_QT_GAMEPAD
#include <QGamepad>
#endif

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
		xGamepad(int = GPBACKEND_SDL, QObject* = nullptr);
		~xGamepad();
		void open(int);
		void open(QString);
		void close();
		void setType(int);
		int getType();
		void setDeadZone(int);
		int deadZone();
		QString name(int = -1);
		QStringList getList();
		static QString getButtonName(int);

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
		int type;
		int lasthat;
		int stid;			// timer id (for sdl events)
		int dead;
		double deadf;
		QString s_name;
		QList<xJoyMapEntry> map;			// gamepad map for gamepad
		QMap<int, QMap<int, int> > jState;	// buttons/axis/hats state: -1 -> 1 | 1 -> -1 = release old one
		union {
			SDL_Joystick* sjptr;
#if USE_QT_GAMEPAD
			QGamepad* qjptr;
#endif
		};
#if USE_QT_GAMEPAD
	private slots:
		void BAChanged(bool);
		void BBChanged(bool);
		void BXChanged(bool);
		void BYChanged(bool);
		void BL1Changed(bool);
		void BL3Changed(bool);
		void BR1Changed(bool);
		void BR3Changed(bool);
		void BUChanged(bool);
		void BDChanged(bool);
		void BRChanged(bool);
		void BLChanged(bool);
		void BStChanged(bool);
		void BSeChanged(bool);
		void BCeChanged(bool);
		void BGuChanged(bool);
		void ALXChanged(double);
		void ALYChanged(double);
		void ARXChanged(double);
		void ARYChanged(double);
		void AL2Changed(double);
		void AR2Changed(double);
		void gpListChanged();
#endif
	protected:
		void timerEvent(QTimerEvent*);
};

//void padLoadConfig(xGamepad*, std::string);
//void padSaveConfig(xGamepad*, std::string);
// operations with gamepad map files
int padExists(std::string);
int padCreate(std::string);
void padDelete(std::string);
