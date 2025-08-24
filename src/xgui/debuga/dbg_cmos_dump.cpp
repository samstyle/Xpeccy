//#include "dbg_widgets.h"
#include "dbg_cmos_dump.h"
#include "../../xcore/xcore.h"

xCmosDumpModel::xCmosDumpModel(QObject *p):xTableModel(p) {
}

int xCmosDumpModel::rowCount(const QModelIndex&) const {
	return 32;
}

int xCmosDumpModel::columnCount(const QModelIndex&) const {
	return 9;
}

QVariant xCmosDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	int row = idx.row();
	int col = idx.column();
	switch(role) {
		case Qt::TextAlignmentRole:
			if (col != 0) res = Qt::AlignCenter;
			break;
		case Qt::DisplayRole:
			if (col == 0) {
				res = gethexbyte(row << 3).prepend("#");		// 00..F8 = address
			} else {
				res = gethexbyte(conf.prof.cur->zx->cmos.data[(row << 3) + col - 1] & 0xff);
			}
			break;
		case Qt::EditRole:
			if (col == 0) break;
			res = gethexbyte(conf.prof.cur->zx->cmos.data[(row << 3) + col - 1] & 0xff);
			break;
	}
	return res;
}

bool xCmosDumpModel::setData(const QModelIndex& idx, const QVariant& val, int role) {
	int row = idx.row();
	int col = idx.column();
	if (col == 0) return false;
	int adr = (row << 3) + col - 1;
	if (adr > 0xff) return false;
	bool flag;
	int d;
	switch(role) {
		case Qt::EditRole:
			d = val.toString().toInt(&flag,16) & 0xff;
			if (flag) {
				conf.prof.cur->zx->cmos.data[adr] = d;
			}
			break;
	}
	return true;
}

Qt::ItemFlags xCmosDumpModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags f = QAbstractTableModel::flags(idx);
	if (idx.column() > 0)
		f |= Qt::ItemIsEditable;
	return f;
}

// table

xCmosDumpTable::xCmosDumpTable(QWidget* p):QTableView(p) {

}

void xCmosDumpTable::resizeEvent(QResizeEvent* e) {
	int w = e->size().width();
	int sz = w / 9;
	horizontalHeader()->setDefaultSectionSize(sz);
}

// widget

xCmosDumpWidget::xCmosDumpWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("CMOSDUMPWIDGET");
	ui.tabCmos->setModel(new xCmosDumpModel());
	hwList << HWG_ZX << HWG_PC;
}

void xCmosDumpWidget::draw() {
	ui.tabCmos->update();
}
