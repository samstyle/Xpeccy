#include "opt_gamepad.h"

#include "../xgui.h"
#include "../../xcore/xcore.h"

#include <QInputDialog>
#include <QLabel>
#include <QLayout>

// Gamepad map table model

xPadMapModel::xPadMapModel(xGamepad* gp, QObject* p):QAbstractItemModel(p) {
	gpad = gp;
}

QModelIndex xPadMapModel::index(int row, int col, const QModelIndex& idx) const {
	return createIndex(row, col);
}

QModelIndex xPadMapModel::parent(const QModelIndex& idx) const {
	return QModelIndex();
}

int xPadMapModel::rowCount(const QModelIndex& idx) const {
	if (idx.isValid()) return 0;
	return gpad->mapSize();
}

int xPadMapModel::columnCount(const QModelIndex& par) const {
	if (par.isValid()) return 0;
	return 3;
}

static QString hatDirs[4] = {"Up","Down","Left","Right"};

QVariant xPadMapModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return res;
	if ((col < 0) || (col >= columnCount())) return res;
	xJoyMapEntry jent = gpad->mapItem(row);
	QString str;
	switch (role) {
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					if (jent.type == JOY_AXIS) str = "Axis";
					else if (jent.type == JOY_BUTTON) str = "Button";
					else if (jent.type == JOY_HAT) str = "Hat";
					else str = "???";
					str.append(QString(" %0").arg(jent.num));
					switch (jent.type) {
						case JOY_AXIS:
							str.append((jent.state < 0) ? " -" : " +");
							break;
						case JOY_HAT:
							switch(jent.state) {
								case SDL_HAT_UP: str.append(" up"); break;
								case SDL_HAT_LEFT: str.append(" left"); break;
								case SDL_HAT_DOWN: str.append(" down"); break;
								case SDL_HAT_RIGHT: str.append(" right"); break;
								default: str.append(" ??"); break;
							}
							break;
					}
					if ((jent.type == JOY_BUTTON) && (jent.num > 11)) {
						str = xGamepad::getButtonName(jent.num); // QString("Hat ") + hatDirs[jent.num & 3];
					}
					res = str;
					break;
				case 1:
					switch(jent.dev) {
						case JMAP_KEY:
#if USE_SEQ_BIND
							str = QString("Key: %0").arg(jent.seq.toString());
#else
							str = QString("Key %0").arg(getKeyNameById(jent.key));
#endif
							break;
						case JMAP_JOY:
							str = "Joystick ";
							switch (jent.dir) {
								case XJ_UP: str.append("up"); break;
								case XJ_DOWN: str.append("down"); break;
								case XJ_RIGHT: str.append("right"); break;
								case XJ_LEFT: str.append("left"); break;
								case XJ_FIRE: str.append("fire"); break;
								case XJ_BUT2: str.append("button2"); break;
								case XJ_BUT3: str.append("button3"); break;
								case XJ_BUT4: str.append("button4"); break;
								default: str.append("??"); break;
							}
							break;
						case JMAP_MOUSE:
							str = "Mouse ";
							switch (jent.dir) {
								case XM_UP: str.append("up"); break;
								case XM_DOWN: str.append("down"); break;
								case XM_LEFT: str.append("left"); break;
								case XM_RIGHT: str.append("right"); break;
								case XM_LMB: str.append("LB"); break;
								case XM_MMB: str.append("MB"); break;
								case XM_RMB: str.append("RB"); break;
								case XM_WHEELUP: str.append("wheel up"); break;
								case XM_WHEELDN: str.append("wheel down"); break;
								default: str.append("??"); break;
							}
					}
					res = str;
					break;
				case 2:
					if (jent.rpt > 0)
						res = QString("%0 sec").arg(jent.rpt / 50.0);
					break;
			}
			break;
	}
	return res;
}

void xPadMapModel::update() {
	endResetModel();
}

// Gamepad setings widget

xGamepadWidget::xGamepadWidget(xGamepad* gp, QWidget* p):QWidget(p) {
	gpad = gp;
	ui.setupUi(this);
	padmodel = new xPadMapModel(gp);
	ui.tvMapView->setModel(padmodel);
	ui.tvMapView->horizontalHeader()->resizeSection(2, 30);
	connect(ui.cbGPName, SIGNAL(currentIndexChanged(int)), this, SLOT(devChanged(int)));
	connect(ui.cbMapFile, SIGNAL(currentIndexChanged(int)), this, SLOT(mapChanged(int)));
	connect(ui.tbAddMap, SIGNAL(released()), this, SLOT(addMap()));
	connect(ui.tbDelMap, SIGNAL(released()), this, SLOT(delMap()));
	connect(ui.tbAddEntry, SIGNAL(released()), this, SLOT(addEntry()));
	connect(ui.tbEditEntry, SIGNAL(released()), this, SLOT(editEntry()));
	connect(ui.tbDelEntry, SIGNAL(released()), this, SLOT(delEntry()));
	connect(ui.tvMapView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editEntry()));
}

void xGamepadWidget::updateList() {
	QStringList lst;
	QString str;
	int i;
	lst = gpad->getList();
	lst.prepend("none");
	ui.cbGPName->blockSignals(true);			// don't call devChanged automaticly
	ui.cbGPName->clear();
	ui.cbGPName->addItems(lst);
	ui.cbGPName->setEnabled(lst.size() > 1);
	str = gpad->lastName();
	if (str.isEmpty()) {
		ui.cbGPName->setCurrentIndex(0);
	} else {
		i = ui.cbGPName->findText(str);
		if (i < 0) i = 0;			// no such gamepad, reset to 'none'
		ui.cbGPName->setCurrentIndex(i);
	}
	// devChanged(cbGPName->currentIndex());		// update current gamepad
	ui.cbGPName->blockSignals(false);
}

void xGamepadWidget::update(std::string mapname) {
	QStringList lst;
	int i;
	updateList();
	ui.sldDeadZone->setValue(gpad->deadZone());
	QDir dir(conf.path.confDir.c_str());
	ui.cbMapFile->clear();
	lst = dir.entryList(QStringList() << "*.pad",QDir::Files,QDir::Name);
	lst.prepend("none");
	ui.cbMapFile->addItems(lst);
	if (mapname.empty()) {
		ui.cbMapFile->setCurrentIndex(0);
	} else {
		i = ui.cbMapFile->findText(mapname.c_str());
		if (i < 0) i = 0;
		ui.cbMapFile->setCurrentIndex(i);
	}

	padmodel->update();
}

void xGamepadWidget::apply() {
	gpad->setDeadZone(ui.sldDeadZone->value());
	if (ui.cbGPName->currentIndex() < 1) {
		gpad->close();
	} else {
		gpad->open(ui.cbGPName->currentText());
	}
}

std::string xGamepadWidget::getMapName() {
	std::string str;
	if (ui.cbMapFile->currentIndex() == 0) return str;
	str = ui.cbMapFile->currentText().toStdString();
	return str;
}

void xGamepadWidget::devChanged(int idx) {
	if (idx > 0) {			// 0 is 'none'
		gpad->open(idx-1);
	} else {
		gpad->close();
		gpad->setName("");
	}
}

void xGamepadWidget::mapChanged(int idx) {
	if (idx < 1) {
		gpad->mapClear();
	} else {
		gpad->loadMap(ui.cbMapFile->currentText().toStdString());
	}
	padmodel->update();
}

void xGamepadWidget::addMap() {
	QString nam = QInputDialog::getText(this,"Enter...","New gamepad map name");
	if (nam.isEmpty()) return;
	nam.append(".pad");
	std::string name = nam.toStdString();
	if (padCreate(name)) {
		ui.cbMapFile->addItem(nam, nam);
		ui.cbMapFile->setCurrentIndex(ui.cbMapFile->count() - 1);
	} else {
		showInfo("Map with that name already exists");
	}
}

void xGamepadWidget::delMap() {
	if (ui.cbMapFile->currentIndex() == 0) return;
	QString name = ui.cbMapFile->currentText();
	if (name.isEmpty()) return;
	if (!areSure("Delete this map?")) return;
	padDelete(name.toStdString());
	ui.cbMapFile->removeItem(ui.cbMapFile->currentIndex());
	ui.cbMapFile->setCurrentIndex(0);
}

void xGamepadWidget::addEntry() {
	if (ui.cbMapFile->currentIndex() == 0) return;
	bindidx = -1;
	xJoyMapEntry jent;
	jent.dev = JOY_NONE;
	jent.dev = JMAP_JOY;
#if USE_SEQ_BIND
	jent.seq = QKeySequence();
#else
	jent.key = ENDKEY;
#endif
	jent.dir = XJ_NONE;
	jent.rpt = 0;
	emit s_edit_entry(gpad, jent);
//	padial->start(jent);
}

void xGamepadWidget::entryReady(xJoyMapEntry ent) {
	if (ent.type == JOY_NONE) return;
	if (ent.dev == JMAP_NONE) return;
	gpad->setItem(bindidx, ent);
	gpad->saveMap(ui.cbMapFile->currentText().toStdString());
	padmodel->update();
}

void xGamepadWidget::editEntry() {
	bindidx = ui.tvMapView->currentIndex().row();
	if (bindidx < 0) return;
	emit s_edit_entry(gpad, gpad->mapItem(bindidx));
//	padial->start(conf.joy.gpad->mapItem(bindidx));
}

extern bool qmidx_greater(const QModelIndex, const QModelIndex);

void xGamepadWidget::delEntry() {
	QModelIndexList lst = ui.tvMapView->selectionModel()->selectedRows();
	if (!lst.isEmpty()) {
		std::sort(lst.begin(), lst.end(), qmidx_greater);
		if (areSure("Delete this binding(s)?")) {
			int row;
			foreach(QModelIndex idx, lst) {
				row = idx.row();
				gpad->delItem(row); // map.erase(conf.joy.gpad->map.begin() + row);
			}
			padmodel->update();
			gpad->saveMap(ui.cbMapFile->currentText().toStdString());
		}
	}
}
