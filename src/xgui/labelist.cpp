// TODO: extend labels list to labels sets manager, add address column to labels list

#include "labelist.h"
#include "xgui.h"

#include <QInputDialog>
#include <QDebug>

// model

xLabelistModel::xLabelistModel(QObject* p):QAbstractListModel(p) {
	cpuMode = false;
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
	fstr = f;
	xLabelSet* set = conf.prof.cur->curlabset;
	if (set) {
		list = filter(set->list.keys(), f);
//		list.sort();		// useless
	} else {
		list.clear();
	}
	emit dataChanged(index(0,0), index(0, list.size()));
}

void xLabelistModel::setCpuMode(bool f) {
	cpuMode = f;
	reset(fstr);
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
	QString adrstr;
	bool pref = false;
	int zadr;
	xAdr xadr;
	switch (role) {
		case Qt::DisplayRole:
			str = list.at(row);
			xadr = find_label(str);
			if (cpuMode) {
				zadr = memFindAdr(conf.prof.cur->zx->mem, xadr.type, xadr.abs);
				if (zadr < 0) {
					adrstr = QString("%0:%1").arg(gethexbyte(xadr.abs >> 14), gethexword(xadr.abs & 0x3fff));
					pref = true;
				} else {
					adrstr = QString("CPU:%0").arg(gethexword(zadr));
				}
			} else {
				adrstr = gethexint(xadr.abs);
				pref = true;
			}
			if (pref) {
				switch(xadr.type) {
					case MEM_RAM: adrstr.prepend("RAM:"); break;
					case MEM_ROM: adrstr.prepend("ROM:"); break;
					case MEM_SLOT: adrstr.prepend("SLT:"); break;
					case MEM_EXT: adrstr.prepend("EXT:"); break;
					case MEM_IO: adrstr = QString("CPU:%0").arg(gethexword(xadr.adr));
				}
			}
			str.prepend(" | ").prepend(adrstr);
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

	connect(ui.name, SIGNAL(textChanged(QString)), mod, SLOT(reset(QString)));
	connect(ui.list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(listDoubleClicked(QModelIndex)));
	connect(ui.list, SIGNAL(activated(QModelIndex)), this, SLOT(listDoubleClicked(QModelIndex)));		// maybe this is better than doubleClicked ? os-depending
	connect(ui.cbLabelSet, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLabelSet()));

	connect(ui.tbAddGrp, SIGNAL(released()), this, SLOT(newGroup()));
	connect(ui.tbEditGrp, SIGNAL(released()), this, SLOT(editGroup()));
	connect(ui.tbDelGrp, SIGNAL(released()), this, SLOT(delGroup()));
	connect(ui.cbAdrCpu, SIGNAL(clicked(bool)), mod, SLOT(setCpuMode(bool)));
}

void xLabeList::listDoubleClicked(QModelIndex idx) {
	if (!idx.isValid()) return;
	QString str = mod->list.at(idx.row());
	emit labSelected(str);
//	close();
}

void xLabeList::changeLabelSet() {
	QString str = getRFSData(ui.cbLabelSet);
	setLabelSet(str);
	mod->reset(ui.name->text());
	emit labSetChanged();
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
	ui.cbLabelSet->blockSignals(true);		// prevent index changing -> slot calling -> current labset changing
	ui.cbLabelSet->clear();
	foreach(xLabelSet* set, conf.prof.cur->labsets) {
		ui.cbLabelSet->addItem(set->name, set->name);
	}
	if (conf.prof.cur->curlabset) {
		setRFIndex(ui.cbLabelSet, conf.prof.cur->curlabset->name, 0);
	} else {
		ui.cbLabelSet->setCurrentIndex(-1);
	}
	ui.cbLabelSet->blockSignals(false);
	emit labSetChanged();
}

void xLabeList::show() {
	fillSetList();
	mod->reset(ui.name->text());
	QWidget::show();
}
