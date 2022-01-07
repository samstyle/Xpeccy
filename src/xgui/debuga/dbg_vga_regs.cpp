#include "dbg_vga_regs.h"
#include "../../xcore/xcore.h"
#include "../../libxpeccy/video/vga.h"

xVgaRegModel::xVgaRegModel(QObject* p):QAbstractTableModel(p) {}

int xVgaRegModel::rowCount(const QModelIndex&) const {
	return 0x18;
}

int xVgaRegModel::columnCount(const QModelIndex&) const {
	return 1;
}

static const char* vga_reg_hnames[1] = {"CRT"};

QVariant xVgaRegModel::headerData(int sec, Qt::Orientation ori, int role) const {
	QVariant res;
	if (role == Qt::DisplayRole) {
		if (ori == Qt::Horizontal) {
			res = vga_reg_hnames[sec];
		} else if (ori == Qt::Vertical) {
			res = QString("R#%0").arg(gethexbyte(sec));
		}
	}
	return res;
}

QVariant xVgaRegModel::data(const QModelIndex& idx, int role) const {
	int row = idx.row();
	int col = idx.column();
	QVariant res;
	switch(role) {
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					if (row < VGA_CRB)
						res = gethexbyte(conf.prof.cur->zx->CRT_REG(row));
					break;
			}
			break;
		case Qt::ForegroundRole:
			if (row == conf.prof.cur->zx->CRT_IDX) {
				res = QColor(0xe0, 0x20, 0x20);
			}
			break;
	}
	return res;
}
