#include "dbg_cmos_dump.h"
#include "../../xcore/xcore.h"

xCmosDumpModel::xCmosDumpModel(QObject *p):QAbstractTableModel(p) {
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
			res = QString::number(conf.prof.cur->zx->cmos.data[(row << 3) + col] & 0xff, 16).toUpper().rightJustified(2,'0');
			break;
	}
	return res;
}

QVariant xCmosDumpModel::headerData(int sect, Qt::Orientation ori, int role) const {
	QVariant res;
	if (ori != Qt::Vertical) return res;
	if (role != Qt::DisplayRole) return res;
	return QString::number(sect << 3, 16).toUpper().rightJustified(3,'0');
}

QModelIndex xCmosDumpModel::index(int row, int col, const QModelIndex &) const {
	return createIndex(row, col, nullptr);
}
