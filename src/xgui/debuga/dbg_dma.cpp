#include "dbg_dma.h"
#include "../../xcore/xcore.h"

xDmaTableModel::xDmaTableModel(QObject* p):QAbstractTableModel(p) {}

int xDmaTableModel::columnCount(const QModelIndex &) const {
	return 8;
}

int xDmaTableModel::rowCount(const QModelIndex &) const {
	return 7;
}

QModelIndex xDmaTableModel::index(int row, int col, const QModelIndex &) const {
	return createIndex(row, col);
}

void xDmaTableModel::update() {
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

static const char* dmaColName[8] = {"CH0","CH1","CH2","CH3","CH4","CH5","CH6","CH7"};
static const char* dmaRowName[7] = {"Masked","Mode","BAR","CAR","PAR","BWR","CWR"};

QVariant xDmaTableModel::headerData(int sec, Qt::Orientation ori, int role) const {
	QVariant res;
	if (role != Qt::DisplayRole) return res;
	if (ori == Qt::Horizontal) {
		if (sec < columnCount())
			res = dmaColName[sec];
	} else if (ori == Qt::Vertical) {
		if (sec < rowCount())
			res = dmaRowName[sec];
	}
	return res;
}

QVariant xDmaTableModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (role != Qt::DisplayRole) return res;
	int row = idx.row();
	int col = idx.column();
	i8237DMA* dma = (col & 4) ? conf.prof.cur->zx->dma16 : conf.prof.cur->zx->dma8;
	DMAChan* ch = &dma->ch[col & 3];
	switch(row) {
		case 0: res = gethexbyte(ch->masked); break;
		case 1: res = gethexbyte(ch->mode); break;
		case 2: res = gethexword(ch->bar); break;
		case 3: res = gethexword(ch->car); break;
		case 4: res = gethexbyte(ch->par); break;
		case 5: res = gethexword(ch->bwr); break;
		case 6: res = gethexword(ch->cwr); break;
	}
	return res;
}
