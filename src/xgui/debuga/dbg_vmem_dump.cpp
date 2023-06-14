#include "dbg_vmem_dump.h"
#include "../../xcore/xcore.h"

// model

xVMemDumpModel::xVMemDumpModel(unsigned char* ptr, QObject *p):xTableModel(p) {
	vmem = ptr;
}

int xVMemDumpModel::rowCount(const QModelIndex&) const {
	return MEM_256K / 8;
}

int xVMemDumpModel::columnCount(const QModelIndex&) const {
	return 10;
}

QVariant xVMemDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	QString str;
	int i;
	int row = idx.row();
	int col = idx.column();
	int radr = row << 3;
	int adr = radr + col - 1;
	switch(role) {
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					res = QString::number(radr, 16).rightJustified(5, '0').toUpper();
					break;
				case 9:
					for (i = 0; i < 8; i++) {
						if (vmem[radr + i] < 32) {
							str.append(".");
						} else {
							str.append(QChar(vmem[radr + i]));
						}
					}
					res = str;
					break;
				default:
					if (vmem == nullptr) break;
					res = QString::number(vmem[adr], 16).rightJustified(2, '0').toUpper();
					break;
			}
			break;
		case Qt::TextAlignmentRole:
			switch(col) {
				case 0: res = Qt::AlignLeft; break;
				case 9: res = Qt::AlignRight; break;
				default: res = Qt::AlignCenter; break;
			}
			break;
		case Qt::EditRole:
			if (col == 0) {
				res = QString::number(radr, 16).rightJustified(5, '0').toUpper();
			} else if ((col < 9) && vmem) {
				res = QString::number(vmem[adr], 16).rightJustified(2, '0').toUpper();
			}
			break;
	}
	return res;
}

bool xVMemDumpModel::setData(const QModelIndex& idx, const QVariant& val, int role) {
	bool res = false;
	if (role != Qt::EditRole) return res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	int b;
	int adr;
	if (col == 0) {
		adr = val.toString().toInt(&res, 16) & (MEM_256K - 1);
		if (res)
			emit adr_ch(index(adr >> 3, 0));
	} else if (col < 9) {
		adr = ((row << 3) | (col - 1)) & (MEM_256K - 1);
		if (vmem) {
			b = val.toString().toInt(&res, 16) & 0xff;
			if (res) {
				vmem[adr] = b;
				emit dataChanged(idx, idx);
			}
		}
	}
	return res;
}

Qt::ItemFlags xVMemDumpModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags res = QAbstractItemModel::flags(idx);
	if (idx.column() < 9) {
		res |= Qt::ItemIsEditable;
	}
	return res;
}

/*
QModelIndex xVMemDumpModel::index(int row, int col, const QModelIndex&) const {
	return createIndex(row, col, nullptr);
}

void xVMemDumpModel::update() {
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}
*/

void xVMemDumpModel::setVMem(unsigned char* ptr) {
	vmem = ptr;
}

// view

xVMemDump::xVMemDump(QWidget *p):QTableView(p) {
	mod = new xVMemDumpModel(nullptr);
	connect(mod, SIGNAL(adr_ch(QModelIndex)), this, SLOT(jumpToIdx(QModelIndex)));
}

void xVMemDump::update() {
	mod->update();
}

void xVMemDump::setVMem(unsigned char* ptr) {
	mod->setVMem(ptr);
	setModel(mod);
	setColumnWidth(0, 70);
}

void xVMemDump::jumpToIdx(QModelIndex idx) {
	setCurrentIndex(idx);
	scrollTo(idx, QAbstractItemView::PositionAtCenter);
}
