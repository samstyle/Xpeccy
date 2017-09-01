#include <opt_diskcat.h>

// model

xDiskCatModel::xDiskCatModel(QObject* p):QAbstractTableModel(p) {
	cat.clear();
}

int xDiskCatModel::rowCount(const QModelIndex&) const {
	return cat.size();
}

int xDiskCatModel::columnCount(const QModelIndex&) const {
	return 7;
}

static QVariant dchNames[] = {"Name","Ext","Start","Length","SecLen","Trk","Sec"};

QVariant xDiskCatModel::headerData(int sect, Qt::Orientation ori, int role) const {
	QVariant res;
	if (ori != Qt::Horizontal) return res;
	if (role != Qt::DisplayRole) return res;
	if ((sect < 0) || (sect > 7)) return res;
	res = dchNames[sect];
	return res;
}

QVariant xDiskCatModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row >= rowCount()) return res;
	if (col >= columnCount()) return res;
	if (role != Qt::DisplayRole) return res;
	TRFile dsc = cat[row];
	switch(col) {
		case 0: res = QString(QByteArray((char*)dsc.name, 8)); break;
		case 1: res = QChar(dsc.ext); break;
		case 2: res = dsc.lst + (dsc.hst << 8); break;
		case 3: res = dsc.llen + (dsc.hlen << 8); break;
		case 4: res = dsc.slen; break;
		case 5: res = dsc.trk; break;
		case 6: res = dsc.sec; break;
	}
	return res;
}

void xDiskCatModel::update() {
	emit dataChanged(index(0,0), index(columnCount() - 1, rowCount() - 1));
}

void xDiskCatModel::setCatalog(QList<TRFile> c) {
	cat = c;
	emit endResetModel();
}

// table

xDiskCatTable::xDiskCatTable(QWidget* p):QTableView(p) {
	model = new xDiskCatModel();
	setModel(model);
	setColumnWidth(0,100);
	setColumnWidth(1,30);
	setColumnWidth(2,70);
	setColumnWidth(3,70);
	setColumnWidth(4,50);
	setColumnWidth(5,50);
}

void xDiskCatTable::setCatalog(QList<TRFile> cat) {
	model->setCatalog(cat);
}
