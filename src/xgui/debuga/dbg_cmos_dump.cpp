#include "dbg_widgets.h"

xCmosDumpModel::xCmosDumpModel(QObject *p):xTableModel(p) {
}

int xCmosDumpModel::rowCount(const QModelIndex &) const {
	return 256/8;
}

int xCmosDumpModel::columnCount(const QModelIndex &) const {
	return 8;
}

QVariant xCmosDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	int row = idx.row();
	int col = idx.column();
	switch(role) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			res = QString::number(conf.prof.cur->zx->cmos.data[(row << 3) + col] & 0xff, 16).toUpper().rightJustified(2,'0');
			break;
	}
	return res;
}

bool xCmosDumpModel::setData(const QModelIndex& idx, const QVariant& val, int role) {
	int row = idx.row();
	int col = idx.column();
	int adr = (row << 3) + col;
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
	f |= Qt::ItemIsEditable;
	return f;
}

QVariant xCmosDumpModel::headerData(int sect, Qt::Orientation ori, int role) const {
	QVariant res;
	if (ori != Qt::Vertical) return res;
	if (role != Qt::DisplayRole) return res;
	return QString::number(sect << 3, 16).toUpper().rightJustified(3,'0');
}

// table

// TODO

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
