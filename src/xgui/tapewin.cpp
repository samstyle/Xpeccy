#include "xgui.h"
#include "xcore/xcore.h"

TapeWin::TapeWin(QWidget *par):QDialog(par) {
	ui.setupUi(this);
	setWindowFlags(Qt::Tool);
	ui.stopBut->setEnabled(false);
	ui.tapeList->setColumnWidth(0,25);
	ui.tapeList->setColumnWidth(1,25);
	ui.tapeList->setColumnWidth(2,50);
	ui.tapeList->hideColumn(3);
	ui.tapeList->hideColumn(4);
	state = TWS_STOP;
	connect(ui.playBut,SIGNAL(released()),this,SLOT(doPlay()));
	connect(ui.recBut,SIGNAL(released()),this,SLOT(doRec()));
	connect(ui.stopBut,SIGNAL(released()),this,SLOT(doStop()));
	connect(ui.loadBut,SIGNAL(released()),this,SLOT(doLoad()));
	connect(ui.tapeList,SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doDClick(QModelIndex)));
	connect(ui.tapeList,SIGNAL(clicked(QModelIndex)), this, SLOT(doClick(QModelIndex)));
//	connect(ui.tapeList,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(doRewind(int,int)));
//	connect(ui.tapeList,SIGNAL(cellClicked(int,int)),this,SLOT(doSwitchBreak(int,int)));
}

void TapeWin::setState(int st) {
	switch(st) {
		case TWS_STOP:
			ui.playBut->setEnabled(true);
			ui.recBut->setEnabled(true);
			ui.stopBut->setEnabled(false);
			ui.tapeBar->setValue(0);
			state = st;
			break;
		case TWS_REC:
		case TWS_PLAY:
			ui.playBut->setEnabled(false);
			ui.recBut->setEnabled(false);
			ui.stopBut->setEnabled(true);
			ui.tapeBar->setValue(0);
			state = st;
			break;
	}
}

void TapeWin::buildList(Tape *tape) {
	ui.tapeList->fill(tape);
/*
	TapeBlockInfo* tinf = new TapeBlockInfo[tape->blkCount];
	tapGetBlocksInfo(tape,tinf);
	ui.tapeList->setRowCount(tape->blkCount);
	QTableWidgetItem* itm;
	std::string tims;
	for (int i=0; i < tape->blkCount; i++) {
		itm = new QTableWidgetItem(QString(getTimeString(tinf[i].time).c_str()));
		ui.tapeList->setItem(i,2,itm);
		itm = new QTableWidgetItem(QDialog::trUtf8(tinf[i].name));
		ui.tapeList->setItem(i,3,itm);
	}
	drawStops(tape);
	delete[](tinf);
*/
}

void TapeWin::drawStops(Tape *tape) {
	ui.tapeList->fill(tape);
/*
	QTableWidgetItem* itm;
	for (int i = 0; i < ui.tapeList->rowCount(); i++) {
		itm = new QTableWidgetItem;
		if (tape->blkData[i].breakPoint) {
			itm->setIcon(QIcon(":/images/cancel.png"));
		}
		ui.tapeList->setItem(i,1,itm);
	}
*/
}

void TapeWin::setCheck(int blk) {
/*
	QTableWidgetItem* itm;
	for (int i = 0; i < ui.tapeList->rowCount(); i++) {
		itm = new QTableWidgetItem;
		if (i == blk) {
			itm->setIcon(QIcon(":/images/checkbox.png"));
		}
		ui.tapeList->setItem(i,0,itm);
	}
*/
}

void TapeWin::setProgress(int val, int max) {
	ui.tapeBar->setMaximum(max);
	ui.tapeBar->setValue(val);
}

// slots

void TapeWin::doPlay() {
	emit stateChanged(TW_STATE,TWS_PLAY);
}

void TapeWin::doStop() {
	emit stateChanged(TW_STATE,TWS_STOP);
}

void TapeWin::doRec() {
	emit stateChanged(TW_STATE,TWS_REC);
}

void TapeWin::doLoad() {
	emit stateChanged(TW_STATE,TWS_OPEN);
}

void TapeWin::doDClick(QModelIndex idx) {
	int row = idx.row();
	int col = idx.column();
	if (col == 1) return;
	emit stateChanged(TW_REWIND, row);
}

void TapeWin::doClick(QModelIndex idx) {
	int row = idx.row();
	int col = idx.column();
	if (col != 1) return;
	emit stateChanged(TW_BREAK, row);
}

/*
void TapeWin::doRewind(int row, int) {
	if (row < 0) return;
	setCheck(row);
	emit stateChanged(TW_REWIND,row);
}

void TapeWin::doSwitchBreak(int row, int col) {
	if ((row < 0) || (col != 1)) return;
	emit stateChanged(TW_BREAK,row);
}
*/
