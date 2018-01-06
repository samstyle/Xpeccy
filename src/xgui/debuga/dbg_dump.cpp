#include "dbg_dump.h"

#include <QDebug>

#include <QTextCodec>
#include <QColor>

extern QColor colBRK;
extern QColor colSEL;

extern unsigned short dumpAdr;
extern int blockStart;
extern int blockEnd;

QString gethexbyte(unsigned char);
QString gethexword(int);

// MODEL

QString getDumpString(QByteArray bts, int cp) {
	QString res;
	QTextCodec* codec = NULL;
	switch(cp) {
		case XCP_1251: codec = QTextCodec::codecForName("CP1251"); break;
		case XCP_866: codec = QTextCodec::codecForName("IBM866"); break;
		case XCP_KOI8R: codec = QTextCodec::codecForName("KOI8R"); break;
	}
	unsigned char bte;
	for (int i = 0; i < 8; i++) {
		bte = (unsigned char)bts.at(i);
		if (bte > 31) {
			res.append(QChar(bte));
		} else {
			res.append(".");
		}
	}
	if (codec == NULL) return res;
	QByteArray arr = res.toLatin1();
	res = codec->toUnicode(arr);
	return res;
}

int xDumpModel::mrd(int adr) const {
	int res = 0xff;
	switch(mode) {
		case XVIEW_CPU:
			res = memRd((*cptr)->mem, adr & 0xffff);
			res |= getBrk(*cptr, adr & 0xffff) << 8;
			break;
		case XVIEW_RAM:
			adr &= 0x3fff;
			adr |= (page << 14);
			res = (*cptr)->mem->ramData[adr & 0x3fffff];
			res |= (*cptr)->brkRamMap[adr & 0x3fffff] << 8;
			break;
		case XVIEW_ROM:
			adr &= 0x3fff;
			adr |= (page << 14);
			res = (*cptr)->mem->romData[adr & 0x7ffff];
			res |= (*cptr)->brkRomMap[adr & 0x7ffff] << 8;
			break;
	}
	return res;
}

void xDumpModel::mwr(int adr, unsigned char bt) {
	switch(mode) {
		case XVIEW_CPU:
			memWr((*cptr)->mem, adr & 0xffff, bt);
			break;
		case XVIEW_RAM:
			adr &= 0x3fff;
			adr |= (page << 14);
			(*cptr)->mem->ramData[adr & 0x3ffff] = bt;
			break;
// write to ROM?
		case XVIEW_ROM:
//			adr &= 0x3fff;
//			adr |= (page << 14);
//			(*cptr)->mem->romData[adr & 0x7fff] = bt;
			break;
	}
}

xDumpModel::xDumpModel(QObject* par):QAbstractTableModel(par) {
	codePage = XCP_1251;
	mode = XVIEW_CPU;
	page = 0;
}

void xDumpModel::setComp(Computer** ptr) {
	cptr = ptr;
}

void xDumpModel::setMode(int md, int pg) {
	mode = md;
	page = pg;
	update();
}

int xDumpModel::rowCount(const QModelIndex& idx) const {
	return 11;
}

int xDumpModel::columnCount(const QModelIndex& idx) const {
	return 10;
}

void xDumpModel::update() {
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void xDumpModel::updateCell(int row, int col) {
	emit dataChanged(index(row, col), index(row, col));
}

void xDumpModel::updateRow(int row) {
	emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void xDumpModel::updateColumn(int col) {
	emit dataChanged(index(0, col), index(rowCount() - 1, col));
}

QVariant xDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row < 0) return res;
	if (row >= rowCount()) return res;
	if (col < 0) return res;
	if (col >= columnCount()) return res;
	unsigned short adr = (dumpAdr + (row << 3)) & 0xffff;
	unsigned short cadr = (adr + col - 1) & 0xffff;
	int flg = mrd(cadr) >> 8;
	QByteArray arr;
	switch(role) {
		case Qt::BackgroundColorRole:
			if (col < 1) break;
			if (col > 8) break;
			if (flg & 0x0f) {								// breakpoint
				res = colBRK;
			} else if ((cadr >= blockStart) && (cadr <= blockEnd) && (mode == XVIEW_CPU)) {	// selection
				res = colSEL;
			}
			break;
		case Qt::TextAlignmentRole:
			switch (col) {
				case 0: break;
				case 9: res = Qt::AlignRight; break;
				default: res = Qt::AlignCenter; break;
			}
			break;
		case Qt::EditRole:
			switch(col) {
				case 0:
					if ((mode == XVIEW_RAM) || (mode == XVIEW_ROM))
						adr &= 0x3fff;
					res = gethexword(adr);
					break;
				case 9: break;
				default:
					res = gethexbyte(mrd((adr + col - 1) & 0xffff) & 0xff);
					break;
			}
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					if ((mode == XVIEW_RAM) || (mode == XVIEW_ROM))
						adr &= 0x3fff;
					res = gethexword(adr);
					break;
				case 9:
					for(int i = 0; i < 8; i++)
						arr.append(mrd((adr + i) & 0xffff) & 0xff);
					res = getDumpString(arr, codePage);
					break;
				default:
					res = gethexbyte(mrd((adr + col - 1) & 0xffff) & 0xff);
					break;
			}
			break;
	}
	return res;
}

Qt::ItemFlags xDumpModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags res = QAbstractItemModel::flags(idx);
	if (!idx.isValid()) return res;
	if (idx.column() < columnCount())
		res |= Qt::ItemIsEditable;
	return res;
}

bool xDumpModel::setData(const QModelIndex& idx, const QVariant& val, int role) {
	if (!idx.isValid()) return false;
	if (role != Qt::EditRole) return false;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return false;
	if ((col < 0) || (col >= columnCount())) return false;
	unsigned short adr = (dumpAdr + (row << 3)) & 0xffff;
	unsigned short nadr;
	unsigned char bt;
	if (col == 0) {
		nadr = val.toString().toInt(NULL, 16) & 0xffff;
		dumpAdr = (nadr - (row << 3)) & 0xffff;
		update();
		emit rqRefill();
	} else if (col < 9) {
		bt = val.toString().toInt(NULL, 16) & 0xff;
		nadr = (adr + col - 1) & 0xffff;
		mwr(nadr, bt);
		updateRow(row);
		emit rqRefill();
	}
	return true;
}

// TABLE

xDumpTable::xDumpTable(QWidget* p):QTableView(p) {
	markAdr = -1;
	model = new xDumpModel();
	setModel(model);

	connect(model, SIGNAL(rqRefill()), this, SIGNAL(rqRefill()));
}

void xDumpTable::setComp(Computer** ptr) {
	cptr = ptr;
	model->setComp(ptr);
}

void xDumpTable::setMode(int md, int pg) {
	mode = md;
	model->setMode(md, pg);
	emit rqRefill();
}

int xDumpTable::rows() {
	return model->rowCount();
}

void xDumpTable::setCodePage(int cp) {
	model->codePage = cp;
	model->updateColumn(9);
}

void xDumpTable::update() {
	model->update();
	QTableView::update();
}

void xDumpTable::keyPressEvent(QKeyEvent* ev) {
	QModelIndex idx = currentIndex();
	switch(ev->key()) {
		case Qt::Key_Up:
			if (idx.row() > 0) {
				QTableView::keyPressEvent(ev);
			} else {
				dumpAdr -= 8;
				emit rqRefill();
			}
			break;
		case Qt::Key_Down:
			if (idx.row() < model->rowCount() - 1) {
				QTableView::keyPressEvent(ev);
			} else {
				dumpAdr += 8;
				emit rqRefill();
			}
			break;
		case Qt::Key_Return:
			edit(currentIndex());
			ev->ignore();
			break;
		default:
			QTableView::keyPressEvent(ev);
			break;
	}
}

void xDumpTable::mousePressEvent(QMouseEvent* ev) {
	QTableView::mousePressEvent(ev);
	QModelIndex idx = indexAt(ev->pos());
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= model->rowCount())) return;
	if ((col < 0) || (col >= model->columnCount())) return;
	if (col > 8) return;
	int adr;
	if ((col == 0) || (col > 8)) {
		adr = dumpAdr + (row << 3);
	} else {
		adr = dumpAdr + (row << 3) + col - 1;
	}
	adr &= 0xffff;
	switch(ev->button()) {
		case Qt::LeftButton:
			if (ev->modifiers() & Qt::ControlModifier) {
				blockStart = adr;
				if (blockEnd < blockStart) blockEnd = blockStart;
				emit rqRefill();
			} else if (ev->modifiers() & Qt::ShiftModifier) {
				blockEnd = adr;
				if (blockStart > blockEnd) blockStart = blockEnd;
				if (blockStart < 0) blockStart = 0;
				emit rqRefill();
			} else {
				markAdr = adr;
			}
			emit rqRefill();
			break;
		case Qt::MidButton:
			blockStart = -1;
			blockEnd = -1;
			markAdr = -1;
			emit rqRefill();
			break;
		default:
			break;
	}
}

void xDumpTable::mouseReleaseEvent(QMouseEvent* ev) {
	QTableView::mouseReleaseEvent(ev);
	if (ev->button() == Qt::LeftButton)
		markAdr = -1;
}

void xDumpTable::mouseMoveEvent(QMouseEvent* ev) {
	QTableView::mouseMoveEvent(ev);
	QModelIndex idx = indexAt(ev->pos());
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= model->rowCount())) return;
	if ((col < 0) || (col >= model->columnCount())) return;
	int adr;
	if ((col == 0) || (col > 8)) {
		adr = dumpAdr + (row << 3);
	} else {
		adr = dumpAdr + (row << 3) + col - 1;
	}
	adr &= 0xffff;
	if ((ev->modifiers() == Qt::NoModifier) && (ev->buttons() & Qt::LeftButton) && (adr != blockStart) && (adr != blockEnd) && (adr != markAdr)) {
		if ((col == 0) || (col > 8))
			adr += 7;
		if (adr < blockStart) {
			blockStart = adr;
			blockEnd = markAdr;
		} else {
			blockStart = markAdr;
			blockEnd = adr;
		}
		emit rqRefill();
	}
	ev->accept();
}

void xDumpTable::wheelEvent(QWheelEvent* ev) {
	if (ev->delta() < 0) {
		dumpAdr += 8;
		emit rqRefill();
	} else if (ev->delta() > 0) {
		dumpAdr -= 8;
		emit rqRefill();
	}
}
