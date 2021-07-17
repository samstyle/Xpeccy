#include "dbg_diskdump.h"
#include "../../xcore/xcore.h"

xDiskDump::xDiskDump(QWidget*) {
	mod = new xDiskDumpModel();
	setModel(mod);
}

void xDiskDump::setDrive(int drv) {
	mod->setDrive(drv);
}

void xDiskDump::setTrack(int tr) {
	if (tr < 166)
		mod->setTrack(tr);
}

void xDiskDump::update() {
	mod->update();
	QTableView::update();
}

// model

void xDiskDumpModel::setDrive(int dr) {
	drv = dr & 3;
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

void xDiskDumpModel::setTrack(int tr) {
	trk = tr;
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

int xDiskDumpModel::rowCount(const QModelIndex&) const {
	return (TRACKLEN / 8) + ((TRACKLEN & 7) ? 1 : 0);
}

int xDiskDumpModel::columnCount(const QModelIndex&) const {
	return 10;
}

void xDiskDumpModel::update() {
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

QModelIndex xDiskDumpModel::index(int row, int col, const QModelIndex& par) const {
	return createIndex(row, col, nullptr);
}

QVariant xDiskDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	int row = idx.row();
	int col = idx.column();
	if (row >= rowCount() || (row < 0)) return res;
	if (col >= columnCount() || (col < 0)) return res;
	int offset = (row << 3) | ((col - 1) & 7);
	int adr = offset & ~7;
	unsigned char ch;
	int pos;
	char buf[256];
	Floppy* flp = conf.prof.cur->zx->dif->fdc->flop[drv];
	QFont fnt;
	switch (role) {
		case Qt::BackgroundColorRole:
			if (col == 0) break;
			if (col > 8) break;
			if (offset >= TRACKLEN) break;
			if (!flp->insert) break;
			switch(flp->data[trk].field[offset]) {
				case 1:			// id
					res = QColor(220, 220, 255);
					break;
				case 2:			// data
				case 3:
					res = QColor(220, 255, 220);
					break;
				case 4:			// crc
					res = QColor(255, 220, 255);
					break;
			}
			break;
		case Qt::FontRole:
			if (col == 0) break;
			if (col > 8) break;
			if (trk != ((flp->trk << 1) | (conf.prof.cur->zx->dif->fdc->side ? 1 : 0))) break;
			if (offset != flp->pos) break;
			fnt.setBold(true);
			res = fnt;
			break;
		case Qt::DisplayRole:
			if (col == 0) {
				sprintf(buf, "%.2X:%.4X", trk, offset & ~7);
			} else if (col == 9) {
				pos = 0;
				while (pos < 8) {
					if (adr < TRACKLEN) {
						ch = flp->data[trk].byte[adr];
						if ((ch < 32) || (ch > 127))
							ch = '.';
					} else {
						ch = '.';
					}
					buf[pos++] = ch;
					adr++;
				}
				buf[pos] = 0;
			} else if (offset < TRACKLEN) {
				if (flp->insert) {
					sprintf(buf, "%.2X", flp->data[trk].byte[offset]);
				} else {
					strcpy(buf, "FF");
				}
			} else {
				buf[0] = 0;
			}
			res = QString(buf);
			break;
	}
	return res;
}
