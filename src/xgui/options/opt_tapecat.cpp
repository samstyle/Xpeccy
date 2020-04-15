#include "opt_tapecat.h"
#include "xcore/xcore.h"

//#include <QDebug>

// model

xTapeCatModel::xTapeCatModel(QObject* p):QAbstractTableModel(p) {
	rcnt = 0;
	inf = NULL;
}

int xTapeCatModel::rowCount(const QModelIndex&) const {
	return  rcnt;
}

int xTapeCatModel::columnCount(const QModelIndex&) const {
	return 6;
}

void xTapeCatModel::fill(Tape* tap) {
	emit beginResetModel();
	rcnt = tap->blkCount;
	rcur = tap->block;
	int fsz = rcnt * sizeof(TapeBlockInfo);
	inf = (TapeBlockInfo*)realloc(inf, fsz);
	if (rcnt == 0) {
		inf = NULL;
	} else {
		switch(conf.prof.cur->zx->hw->grp) {
			case HWG_ZX:
				tapGetBlocksInfo(tap, inf, TFRM_ZX);
				break;
			case HWG_BK:
				tapGetBlocksInfo(tap, inf, TFRM_BK);
				break;
			default:
				memset(inf, 0, fsz);
				break;
		}

	}
	emit endResetModel();
}

void xTapeCatModel::update() {
	emit dataChanged(index(0,0), index(columnCount() - 1, rowCount() - 1));
}

static QVariant tcmName[6] = {"Cur","Brk","Dur","Time","Bytes","Name"};

QVariant xTapeCatModel::headerData(int sec, Qt::Orientation ori, int role) const {
	QVariant res;
	if (ori != Qt::Horizontal) return res;
	if (role != Qt::DisplayRole) return res;
	if ((sec < 0) || (sec >= columnCount())) return res;
	res = tcmName[sec];
	return res;
}

QVariant xTapeCatModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return res;
	if ((col < 0) || (col >= columnCount())) return res;
	if (inf == NULL) return res;
	switch (role) {
		case Qt::CheckStateRole:
			switch(col) {
				case 0:	if (row == rcur)
						res = Qt::Checked;
					break;
				case 1: if (inf[row].breakPoint)
						res = Qt::Checked;
					break;
			}
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 2: res = QString(getTimeString(inf[row].time).c_str());
					break;
				case 3: if (row == rcur)
						res = QString(getTimeString(inf[row].curtime).c_str());
					break;
				case 4:	if (inf[row].size > 0)
						res = inf[row].size;
					break;
				case 5: res = QString::fromLocal8Bit(inf[row].name);
					break;
			}
			break;
	}
	return  res;
}

// table

xTapeCatTable::xTapeCatTable(QWidget* p):QTableView(p) {
	model = new xTapeCatModel();
	setModel(model);
}

// tape player window will reset scroll on update
void xTapeCatTable::fill(Tape* tape) {
	int row = currentIndex().row();
	model->fill(tape);
	selectRow(row);
	setEnabled(tape->blkCount > 0);
}
