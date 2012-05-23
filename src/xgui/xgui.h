#ifndef _XGUI_H
#define _XGUI_H

#include <QDialog>

// tape player

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
