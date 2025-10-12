// TODO: extend labels list to labels sets manager, add address column to labels list

#include "labelist.h"
#include "xgui.h"

#include <QInputDialog>
#include <QDebug>

// model

xLabelistModel::xLabelistModel(QObject* p):QAbstractListModel(p) {

}

int xLabelistModel::rowCount(const QModelIndex&) const {
	return list.size();
}

QStringList filter(QStringList lst, QString f) {
	QString str;
	QStringList res;
	foreach(str, lst) {
		if (str.contains(f))
			res.append(str);
	}
	return res;
}

void xLabelistModel::reset(QString f) {
	// list = conf.labels.keys().filter(f, Qt::CaseInsensitive);
	xLabelSet* set = conf.prof.cur->curlabset;
	if (set) {
		list = filter(set->list.keys(), f);
		list.sort();
	} else {
		list.clear();
	}
	emit dataChanged(index(0,0), index(0, list.size()));
}

QModelIndex xLabelistModel::index(int row, int col, const QModelIndex&) const {
	return createIndex(row, col, nullptr);
}

QVariant xLabelistModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	if (row < 0) return res;
	if (row >= rowCount()) return res;
	QString str;
	xAdr xadr;
	switch (role) {
		case Qt::DisplayRole:
			str = list.at(row);
			xadr = find_label(str);
			switch(xadr.type) {
				case MEM_RAM: str.append(" | RAM:").append(gethexint(xadr.abs)); break;
				case MEM_ROM: str.append(" | ROM:").append(gethexint(xadr.abs)); break;
				case MEM_SLOT: str.append(" | SLT:").append(gethexint(xadr.abs)); break;
				case MEM_EXT: str.append(" | EXT:").append(gethexint(xadr.abs)); break;
				case MEM_IO: str.append(" | CPU:").append(gethexint(xadr.adr)); break;
			}
			res = str;
			break;
	}
	return res;
}

// window

void setRFIndex(QComboBox*, QVariant, int);
QString getRFSData(QComboBox*);

xLabeList::xLabeList(QWidget* p):QDialog(p) {
	ui.setupUi(this);
	mod = new xLabelistModel();
	ui.list->setModel(mod);

	connect(ui.cbLabelSet, SIGNAL(currentIndexChanged(int)), this, SIGNAL(labSetChanged()));

	connect(ui.name, SIGNAL(textChanged(QString)), mod, SLOT(reset(QString)));
	connect(ui.list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(listDoubleClicked(QModelIndex)));
	connect(ui.cbLabelSet, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLabelSet()));

	connect(ui.tbAddGrp, SIGNAL(released()), this, SLOT(newGroup()));
	connect(ui.tbEditGrp, SIGNAL(released()), this, SLOT(editGroup()));
	connect(ui.tbDelGrp, SIGNAL(released()), this, SLOT(delGroup()));
}

void xLabeList::listDoubleClicked(QModelIndex idx) {
	if (!idx.isValid()) return;
	QString str = mod->list.at(idx.row());
	emit labSelected(str);
	close();
}

void xLabeList::changeLabelSet() {
	QString str = getRFSData(ui.cbLabelSet);
	setLabelSet(str);
	mod->reset(ui.name->text());
}

void xLabeList::newGroup() {
	QString str = QInputDialog::getText(this, "Input name", "Set name");
	if (str.isEmpty()) return;
	newLabelSet(str);
	setLabelSet(str);
	fillSetList();
	mod->reset(ui.name->text());
}

void xLabeList::editGroup() {
	xLabelSet* set = conf.prof.cur->curlabset;
	if (!set) return;
	QString str = QInputDialog::getText(this, "Input name", "Set name",QLineEdit::Normal,set->name);
	if (str.isEmpty()) return;
	set->name = str;
	fillSetList();
	mod->reset(ui.name->text());
}

void xLabeList::delGroup() {
	xLabelSet* set = conf.prof.cur->curlabset;
	if (!set) return;
	if (!areSure("Do you want to delete this labelset?")) return;
	delLabelSet(set->name);
	fillSetList();
}

void xLabeList::fillSetList() {
	ui.cbLabelSet->clear();
	foreach(xLabelSet* set, conf.prof.cur->labsets) {
		ui.cbLabelSet->addItem(set->name, set->name);
	}
	if (conf.prof.cur->curlabset) {
		setRFIndex(ui.cbLabelSet, conf.prof.cur->curlabset->name, 0);
	} else {
		ui.cbLabelSet->setCurrentIndex(-1);
	}
}

void xLabeList::show() {
	fillSetList();
	mod->reset(ui.name->text());
	QWidget::show();
}
