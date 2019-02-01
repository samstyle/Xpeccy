#include "labelist.h"
#include "../xcore/xcore.h"

#include <QDebug>

// model

xLabelistModel::xLabelistModel(QObject* p):QAbstractListModel(p) {

}

int xLabelistModel::rowCount(const QModelIndex&) const {
	return list.size();
}

QStringList filter(QStringList lst, QString f) {
	QStringList res;
	QString str;
	foreach(str, lst) {
		if (str.contains(f, Qt::CaseInsensitive))
			res.append(str);
	}
	return res;
}

void xLabelistModel::reset(QString f) {
	list = filter(conf.labels.keys(), f);
	list.sort();
	emit dataChanged(index(0,0), index(0, list.size() - 1));
}

QVariant xLabelistModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	if (row < 0) return res;
	if (row >= rowCount()) return res;
	switch (role) {
		case Qt::DisplayRole:
			res = list.at(row);
			break;
	}
	return res;
}

// window

xLabeList::xLabeList(QWidget* p):QDialog(p) {
	ui.setupUi(this);
	mod = new xLabelistModel();
	ui.list->setModel(mod);

	connect(ui.name, SIGNAL(textChanged(QString)), mod, SLOT(reset(QString)));
	connect(ui.list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(listDoubleClicked(QModelIndex)));
}

void xLabeList::listDoubleClicked(QModelIndex idx) {
	if (!idx.isValid()) return;
	QString str = mod->list.at(idx.row());
	emit labSelected(str);
	close();
}

void xLabeList::show() {
	mod->reset(ui.name->text());
	QWidget::show();
}
