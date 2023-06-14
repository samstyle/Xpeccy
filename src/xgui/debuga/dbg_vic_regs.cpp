#include "dbg_vic_regs.h"
#include "../../xcore/xcore.h"

xVicRegsModel::xVicRegsModel(QObject* p):xTableModel(p) {
}

int xVicRegsModel::rowCount(const QModelIndex&) const {return 0x31;}
int xVicRegsModel::columnCount(const QModelIndex&) const {return 3;}

static const char* vic_reg_hnames[3] = {"HEX","DEC","BIN"};

QVariant xVicRegsModel::headerData(int sec, Qt::Orientation ori, int role) const {
	QVariant res;
	if (role == Qt::DisplayRole) {
		if (ori == Qt::Horizontal) {
			if (sec < 3) res = vic_reg_hnames[sec];
		} else if (ori == Qt::Vertical) {
			if (sec < 0x31) res = QString("R#%0").arg(gethexbyte(sec));
		}
	}
	return res;
}

QVariant xVicRegsModel::data(const QModelIndex& idx, int role) const {
	int row = idx.row();
	int col = idx.column();
	QVariant res;
	switch(role) {
		case Qt::DisplayRole:
			switch (col) {
				case 0: res = gethexbyte(conf.prof.cur->zx->vid->reg[row]); break;
				case 1: res = QString::number(conf.prof.cur->zx->vid->reg[row], 10); break;
				case 2: res = getbinbyte(conf.prof.cur->zx->vid->reg[row]); break;
			}
			break;
	}
	return res;
}
