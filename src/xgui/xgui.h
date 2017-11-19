#ifndef X_GUI_H
#define X_GUI_H

#include <QDialog>

// common

void shitHappens(const char*);
bool areSure(const char*);
int askYNC(const char*);
void showInfo(const char*);

// tape player

#include "xgui/options/opt_tapecat.h"
#include "ui_tapewin.h"
#include "libxpeccy/tape.h"

enum {
	TW_STATE = 0,
	TW_REWIND,
	TW_BREAK
};

enum {
	TWS_PLAY = 1,
	TWS_REC,
	TWS_STOP,
	TWS_OPEN,
	TWS_REWIND
};

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
		void doDClick(QModelIndex);
		void doClick(QModelIndex);
};

// rzx player

#include "ui_rzxplayer.h"

enum {
	RWS_PLAY = 1,
	RWS_STOP,
	RWS_PAUSE,
	RWS_OPEN
};

class RZXWin : public QDialog {
	Q_OBJECT
	public:
		RZXWin(QWidget*);
		void startPlay();
		void setProgress(int,int);
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
