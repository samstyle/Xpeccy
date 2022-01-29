#include "dbg_pic.h"
#include "../../xcore/xcore.h"

xPicModel::xPicModel(QObject* p):QAbstractTableModel(p) {
}

int xPicModel::columnCount(const QModelIndex &) const {
	return 2;
}

int xPicModel::rowCount(const QModelIndex &) const {
	return 9;
}

static const char* picVHead[9] = {"IRR","IMR","ISR","ICW1","ICW2","ICW3","ICW4","OCW2","OCW3"};
static const char* picHHead[2] = {"PIC1","PIC2"};

QVariant xPicModel::headerData(int sec, Qt::Orientation ori, int role) const {
	QVariant res;
	if (role != Qt::DisplayRole) return res;
	switch(ori) {
		case Qt::Vertical:
			if (sec < rowCount())
				res = picVHead[sec];
			break;
		case Qt::Horizontal:
			if (sec < columnCount())
				res = picHHead[sec];
			break;
	}
	return res;
}

QModelIndex xPicModel::index(int r, int c, const QModelIndex &) const {
	return createIndex(r, c, nullptr);
}

QVariant xPicModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	int row = idx.row();
	int col = idx.column();
	PIC* pic = col ? &conf.prof.cur->zx->spic : &conf.prof.cur->zx->mpic;
	switch (role) {
		case Qt::DisplayRole:
			switch (row) {
				case 0: res = getbinbyte(pic->irr); break;
				case 1: res = getbinbyte(pic->imr); break;
				case 2: res = getbinbyte(pic->isr); break;
				case 3: res = getbinbyte(pic->icw1); break;
				case 4: res = getbinbyte(pic->icw2); break;
				case 5: res = getbinbyte(pic->icw3); break;
				case 6: res = getbinbyte(pic->icw4); break;
				case 7: res = getbinbyte(pic->ocw2); break;
				case 8: res = getbinbyte(pic->ocw3); break;
			}
			break;
	}
	return res;
}
