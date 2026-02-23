#include "dbg_widgets.h"

xPitModel::xPitModel(QObject* p):xTableModel(p) {}

int xPitModel::columnCount(const QModelIndex&) const {
	return 3;
}

int xPitModel::rowCount(const QModelIndex&) const {
	return 8;
}

static const char* vhead[8] = {"Divider","Counter","Mode","ACMode","State","Out","wdiv","wgate"};
static const char* hhead[3] = {"CH0","CH1","CH2"};

QVariant xPitModel::headerData(int sect, Qt::Orientation ori, int role) const {
	QVariant res;
	if (sect < 0) return res;
	if (role != Qt::DisplayRole) return res;
	switch(ori) {
		case Qt::Horizontal:
			if (sect < columnCount())
				res = hhead[sect];
			break;
		case Qt::Vertical:
			if (sect < rowCount())
				res = vhead[sect];
			break;
	}
	return res;
}

QVariant xPitModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return res;
	if ((col < 0) || (col >= columnCount())) return res;
	pitChan* ch = NULL;
	switch (col) {
		case 0: ch = &conf.prof.cur->zx->pit->ch0; break;
		case 1: ch = &conf.prof.cur->zx->pit->ch1; break;
		case 2: ch = &conf.prof.cur->zx->pit->ch2; break;
	}
	if (!ch) return res;
	switch(role) {
		case Qt::DisplayRole:
			switch(row) {
				case 0: res = gethexword(ch->div); break;
				case 1: res = gethexword(ch->cnt); break;
				case 2: res = gethexbyte(ch->opmod); break;
				case 3: res = gethexbyte(ch->acmod); break;
				case 4: res = gethexbyte(ch->state); break;
				case 5: res = ch->out; break;
				case 6: res = ch->wdiv; break;
				case 7: res = ch->wgat; break;
			}
			break;
	}
	return res;
}

// widget

xPitWidget::xPitWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("PITWIDGET");
	ui.tabPit->setModel(new xPitModel());
	hwList << HWG_PC << HWG_PC98XX;
}

void xPitWidget::draw() {
	ui.tabPit->update();
}
