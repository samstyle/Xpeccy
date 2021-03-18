#include "xgui.h"

RZXWin::RZXWin(QWidget *par):QDialog(par) {
	ui.setupUi(this);
	setWindowFlags(Qt::Tool);
	state = RWS_STOP;
	ui.ppButton->setEnabled(false);
	ui.stopButton->setEnabled(false);
	connect(ui.ppButton,SIGNAL(released()),this,SLOT(playPause()));
	connect(ui.stopButton,SIGNAL(released()),this,SLOT(stop()));
	connect(ui.openButton,SIGNAL(released()),this,SLOT(open()));
}

void RZXWin::startPlay() {
	ui.ppButton->setEnabled(true);
	ui.stopButton->setEnabled(true);
	ui.ppButton->setIcon(QIcon(":/images/pause.png"));
	ui.progress->setValue(0);
	state = RWS_PLAY;
}

void RZXWin::setProgress(int val, int max) {
	ui.progress->setMaximum(max / 50);
	ui.progress->setValue(val / 50);
}

// slots

void RZXWin::upd(Computer* comp) {
#ifdef HAVEZLIB
	if (comp->rzx.play && isVisible()) {
		setProgress(comp->rzx.fCurrent, comp->rzx.fTotal);
	}
#endif
}

void RZXWin::playPause() {
	switch(state) {
		case RWS_PLAY:
			state = RWS_PAUSE;
			ui.ppButton->setIcon(QIcon(":/images/play.png"));
			emit stateChanged(RWS_PAUSE);
			break;
		case RWS_PAUSE:
			state = RWS_PLAY;
			ui.ppButton->setIcon(QIcon(":/images/pause.png"));
			emit stateChanged(RWS_PLAY);
			break;
	}
}

void RZXWin::stop() {
	state = RWS_STOP;
	ui.ppButton->setEnabled(false);
	ui.stopButton->setEnabled(false);
	ui.ppButton->setIcon(QIcon(":/images/play.png"));
	ui.progress->setValue(0);
	emit stateChanged(RWS_STOP);
}

void RZXWin::open() {
	emit stateChanged(RWS_OPEN);
}
