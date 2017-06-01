#include "dbg_dump.h"

#include <QDebug>

#include <QTextCodec>
#include <QColor>

extern unsigned short dumpAdr;
extern unsigned short blockStart;
extern unsigned short blockEnd;

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

xDumpModel::xDumpModel(void** pp, dmpMrd cr, dmpMwr cw, QObject* par):QAbstractItemModel(par) {
	pptr = pp;
	mrd = cr;
	mwr = cw;
	codePage = XCP_1251;
}

QModelIndex xDumpModel::index(int row, int col, const QModelIndex& par) const {
	return createIndex(row, col);
}

QModelIndex xDumpModel::parent(const QModelIndex& idx) const {
	return QModelIndex();
}

int xDumpModel::rowCount(const QModelIndex& idx) const {
	return 13;
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

QVariant xDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row < 0) return res;
	if (row >= rowCount()) return res;
	if (col < 0) return res;
	if (col >= columnCount()) return res;
	unsigned short adr = dumpAdr + (row << 3);
	unsigned short cadr = adr + col - 1;
	int flg = mrd(cadr, *pptr) >> 8;
	QByteArray arr;
	switch(role) {
		case Qt::BackgroundColorRole:
			if (col < 1) break;
			if (col > 8) break;
			if (flg & 0x0f) {						// breakpoint
				res = QColor(200,128,128);
			} else if ((cadr >= blockStart) && (cadr <= blockEnd)) {	// selection
				res = QColor(128,255,128);
			}
			break;
		case Qt::TextAlignmentRole:
			switch (col) {
				case 0: break;
				case 9: res = Qt::AlignRight; break;
				default: res = Qt::AlignCenter; break;
			}
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 0: res = gethexword(adr);
					break;
				case 9:
					for(int i = 0; i < 8; i++)
						arr.append(mrd(adr + i, *pptr) & 0xff);
					res = getDumpString(arr, codePage);
					break;
				default:
					res = gethexbyte(mrd(adr + col - 1, *pptr) & 0xff);
					break;
			}
			break;
	}
	return res;
}

Qt::ItemFlags xDumpModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags res = QAbstractItemModel::flags(idx);
	if (!idx.isValid()) return res;
	if (idx.row() < 9)
		res |= Qt::ItemIsEditable;
	return res;
}

bool xDumpModel::setData(const QModelIndex& idx, const QVariant& val, int role) {
	if (!idx.isValid()) return false;
	if (role != Qt::EditRole) return false;
	int row = idx.row();
	int col = idx.column();
	unsigned short adr = dumpAdr + (row << 3);
	unsigned short nadr;
	unsigned char bt;
	if (col == 0) {
		nadr = val.toString().toInt(NULL, 16);
		dumpAdr = nadr - (row << 3);
		update();
	} else if (col < 9) {
		bt = val.toString().toInt(NULL, 16);
		nadr = adr + col - 1;
		mwr(nadr, bt, *pptr);
		updateRow(row);
	}
	return true;
}

// TABLE

xDumpTable::xDumpTable(QWidget* p):QTableView(p) {

}
