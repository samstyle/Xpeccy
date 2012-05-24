#ifndef _XGUI_H
#define _XGUI_H

#include <QDialog>

// common

void shitHappens(const char*);
bool areSure(const char*);
void showInfo(const char*);

// tape player

#include "ui_tapewin.h"
#include "../libxpeccy/tape.h"

#define	TW_STATE	0
#define	TW_REWIND	1
#define	TW_BREAK	2

#define	TWS_PLAY	1
#define	TWS_REC		2
#define	TWS_STOP	3
#define	TWS_OPEN	4
#define	TWS_REWIND	5

class TapeWin : public QDialog {
	Q_OBJECT
	public:
		TapeWin(QWidget*);
		void setState(int);
		void buildList(Tape*);
		void drawStops(Tape*);
		void setCheck(int);
		void setProgress(int,int);
	signals:
		void stateChanged(int,int);
	private:
		Ui::TapeWin ui;
		int state;
	private slots:
		void doPlay();
		void doRec();
		void doStop();
		void doLoad();
		void doRewind(int,int);
		void doSwitchBreak(int,int);
};

// rzx player

#include "ui_rzxplayer.h"

#define	RWS_PLAY	1
#define	RWS_STOP	2
#define	RWS_PAUSE	3
#define	RWS_OPEN	4

class RZXWin : public QDialog {
	Q_OBJECT
	public:
		RZXWin(QWidget*);
		void startPlay();
		void setProgress(int);
	signals:
		void stateChanged(int);
	private:
		Ui::rzxPlayer ui;
		int state;
	public slots:
		void stop();
	private slots:
		void playPause();
		void open();
};

#endif
