#include "dbg_diskdump.h"
#include "../../xcore/xcore.h"

xDiskDump::xDiskDump(QWidget*) {
	mod = new xDiskDumpModel();
	setModel(mod);
	setDrive(0);
}

void xDiskDump::setDrive(int d) {
	drv = d;
	mod->setDrive(drv);
}

void xDiskDump::setTrack(int tr) {
	if (tr < 172)
		mod->setTrack(tr);
}

void xDiskDump::update() {
	setDrive(drv);
}

void xDiskDump::toTarget() {
	scrollTo(mod->index(conf.prof.cur->zx->dif->fdc->flp->pos >> 3, 0));
}

void xDiskDump::resizeEvent(QResizeEvent* ev) {
	int w = ev->size().width();
	int h = ev->size().height();
	if (h < 1) return;
	horizontalHeader()->setDefaultSectionSize((w - 150) / 8);;
	horizontalHeader()->setStretchLastSection(true);
	setColumnWidth(0, 70);
}

// model

xDiskDumpModel::xDiskDumpModel(QObject* p):xTableModel(p) {
	rcnt = 0;
	drv = 0;
	trk = 0;
}

void xDiskDumpModel::setDrive(int dr) {
	drv = dr & 3;
	Floppy* flp = conf.prof.cur->zx->dif->fdc->flop[drv];
	int new_rcnt = (flp->trklen / 8) + ((flp->trklen & 7) ? 1 : 0);
	if (new_rcnt < rcnt) {
		emit beginRemoveRows(QModelIndex(), new_rcnt, rcnt - new_rcnt);
		rcnt = new_rcnt;
		emit endRemoveRows();
	} else if (new_rcnt > rcnt) {
		emit beginInsertRows(QModelIndex(), rcnt, new_rcnt - rcnt);
		rcnt = new_rcnt;
		emit endInsertRows();
	}
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

void xDiskDumpModel::setTrack(int tr) {
	trk = tr;
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

int xDiskDumpModel::rowCount(const QModelIndex&) const {
	return rcnt;
}

int xDiskDumpModel::columnCount(const QModelIndex&) const {
	return 10;
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
		case X_BackgroundRole:
			if (col == 0) break;
			if (col > 8) break;
			if (offset >= flp->trklen) break;
			if (!flp->insert) break;
			ch = flp->data[trk].field[offset];
			switch(ch & 0x0f) {
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
//			if (ch & 0x80) {
//				res = QColor(220,120,120);	// 'broken' A1
//			}
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
					if (adr < flp->trklen) {
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
			} else if (offset < flp->trklen) {
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

// widget

xDiskDumpWidget::xDiskDumpWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("FDDDUMPWIDGET");
//	ui.tabDiskDump->setColumnWidth(0, 70);
//	ui.tabDiskDump->horizontalHeader()->setStretchLastSection(true);
	connect(ui.cbDrive, SIGNAL(currentIndexChanged(int)), ui.tabDiskDump, SLOT(setDrive(int)));
	connect(ui.sbTrack, SIGNAL(valueChanged(int)), ui.tabDiskDump, SLOT(setTrack(int)));
	connect(ui.tbTarget, SIGNAL(released()),this,SLOT(toTarget()));
	hwList << HWG_ZX << HWG_PC << HWG_BK;
}

void xDiskDumpWidget::toTarget() {
	FDC* fdc = conf.prof.cur->zx->dif->fdc;
	Floppy* flp = fdc->flp;
	ui.cbDrive->setCurrentIndex(flp->id);
	ui.sbTrack->setValue((flp->trk << 1) | !!fdc->side);
	ui.tabDiskDump->toTarget();
}

void xDiskDumpWidget::draw() {
	ui.tabDiskDump->update();
}
