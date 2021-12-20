#include "dbg_vmem_dump.h"
#include "../../xcore/xcore.h"

// model

xVMemDumpModel::xVMemDumpModel(unsigned char* ptr, QObject *p):QAbstractTableModel(p) {
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
							str.append(vmem[radr + i]);
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
	}
	return res;
}

QModelIndex xVMemDumpModel::index(int row, int col, const QModelIndex&) const {
	return createIndex(row, col, nullptr);
}

void xVMemDumpModel::update() {
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

void xVMemDumpModel::setVMem(unsigned char* ptr) {
	vmem = ptr;
}

// view

xVMemDump::xVMemDump(QWidget *p):QTableView(p) {
	mod = new xVMemDumpModel(nullptr);
}

void xVMemDump::update() {
	mod->update();
}

void xVMemDump::setVMem(unsigned char* ptr) {
	mod->setVMem(ptr);
	setModel(mod);
	setColumnWidth(0, 70);
}
