#include "xgui.h"
#include "xcore/xcore.h"
#include "../filer.h"

TapeWin::TapeWin(QWidget *par):QDialog(par) {
	ui.setupUi(this);
	setWindowFlags(Qt::Tool);
	ui.stopBut->setEnabled(false);
	//ui.tapeList->setColumnWidth(0,25);
	ui.tapeList->setColumnWidth(0,25);
	ui.tapeList->setColumnWidth(1,50);
	ui.tapeList->hideColumn(2);
	ui.tapeList->hideColumn(3);
	connect(ui.playBut,SIGNAL(released()),this,SLOT(doPlay()));
	connect(ui.recBut,SIGNAL(released()),this,SLOT(doRec()));
	connect(ui.stopBut,SIGNAL(released()),this,SLOT(doStop()));
	connect(ui.loadBut,SIGNAL(released()),this,SLOT(doLoad()));
	connect(ui.tbRewind,SIGNAL(released()),this,SLOT(doRewind()));
	connect(ui.tapeList,SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doDClick(QModelIndex)));
	connect(ui.tapeList,SIGNAL(clicked(QModelIndex)), this, SLOT(doClick(QModelIndex)));
}

void TapeWin::show() {
	QDialog::show();
	upd(conf.prof.cur->zx->tape);
	updList(conf.prof.cur->zx->tape);
}

// on timer
void TapeWin::updProgress(Tape* tape) {
	if (!isVisible()) return;
	if (!tape->on || tape->rec) {
		ui.tapeBar->setValue(0);
	} else {
		ui.tapeBar->setMaximum(tape->blkData[tape->block].sigCount);
		ui.tapeBar->setValue(tape->pos);
	}
}

// TODO: on play state changed
void TapeWin::upd(Tape* tape) {
	if (isVisible()) {
		if (tape->blkCount > 0) {
			ui.playBut->setEnabled(!tape->on);
			ui.recBut->setEnabled(!tape->on);
			ui.stopBut->setEnabled(tape->on);
			ui.tapeList->setEnabled(true);
			ui.tbRewind->setEnabled(!tape->on);
			// ui.tapeList->fill(tape);
		} else {
			ui.playBut->setEnabled(false);
			ui.recBut->setEnabled(false);
			ui.stopBut->setEnabled(false);
			ui.tapeList->setEnabled(false);
			ui.tbRewind->setEnabled(false);
		}
	}
}

// on block changed
void TapeWin::updList(Tape* tape) {
	ui.tapeList->fill(tape);
}

// slots

void TapeWin::doPlay() {
	Tape* tap = conf.prof.cur->zx->tape;
	tap->rec = 0;
	tap->on = 1;
	upd(tap);
}

void TapeWin::doStop() {
	Tape* tap = conf.prof.cur->zx->tape;
	tap->on = 0;
	tap->rec = 0;
	upd(tap);
}

void TapeWin::doRec() {
	Tape* tap = conf.prof.cur->zx->tape;
	tap->rec = 1;
	tap->on = 1;
	upd(tap);
}

void TapeWin::doRewind() {
	Tape* tap = conf.prof.cur->zx->tape;
	if (!tap->on) {
		tapRewind(tap, 0);
		upd(tap);
	}
}

void TapeWin::doLoad() {
	conf.emu.pause |= PR_FILE;
	load_file(conf.prof.cur->zx, nullptr, FG_TAPE, -1);
	updList(conf.prof.cur->zx->tape);
	// ui.tapeList->fill(conf.prof.cur->zx->tape);
	conf.emu.pause &= ~PR_FILE;
}

void TapeWin::doDClick(QModelIndex idx) {
	int row = idx.row();
	int col = idx.column();
	if (col == 0) return;
	tapRewind(conf.prof.cur->zx->tape, row);
	updList(conf.prof.cur->zx->tape);
	//ui.tapeList->fill(conf.prof.cur->zx->tape);
}

void TapeWin::doClick(QModelIndex idx) {
	int row = idx.row();
	int col = idx.column();
	if (col != 0) return;
	conf.prof.cur->zx->tape->blkData[row].breakPoint ^= 1;
	updList(conf.prof.cur->zx->tape);
	// ui.tapeList->fill(conf.prof.cur->zx->tape);
}
