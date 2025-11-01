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
	cbGPName = new QComboBox;
	cbMapFile = new QComboBox;
	sldDeadZone = new QSlider(Qt::Horizontal);
	sldDeadZone->setMinimum(0);
	sldDeadZone->setMaximum(32768);
	tvMapView = new QTableView;
	padmodel = new xPadMapModel(gp);
	tvMapView->setModel(padmodel);
	tvMapView->horizontalHeader()->setVisible(false);
	tvMapView->verticalHeader()->setVisible(false);
	tvMapView->verticalHeader()->setDefaultSectionSize(17);
	tvMapView->horizontalHeader()->setStretchLastSection(true);
	tvMapView->setGridStyle(Qt::DotLine);
	tvMapView->setSelectionMode(QAbstractItemView::SingleSelection);
	tvMapView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbAddMap = new QToolButton;
	tbAddMap->setIcon(QIcon(":/images/add.png"));
	tbDelMap = new QToolButton;
	tbDelMap->setIcon(QIcon(":/images/cancel.png"));
	tbAddEntry = new QToolButton;
	tbAddEntry->setIcon(QIcon(":/images/add.png"));
	tbEditEntry = new QToolButton;
	tbEditEntry->setIcon(QIcon(":/images/edit.png"));
	tbDelEntry = new QToolButton;
	tbDelEntry->setIcon(QIcon(":/images/cancel.png"));

	QVBoxLayout* laya = new QVBoxLayout;
	laya->addWidget(tbAddEntry);
	laya->addWidget(tbEditEntry);
	laya->addStretch();
	laya->addWidget(tbDelEntry);
	QHBoxLayout* layb = new QHBoxLayout;
	layb->addLayout(laya);
	layb->addWidget(tvMapView);
	QHBoxLayout* layc = new QHBoxLayout;
	layc->addWidget(cbMapFile);
	layc->addWidget(tbAddMap);
	layc->addWidget(tbDelMap);
	QGridLayout* layd = new QGridLayout;
	layd->addWidget(new QLabel("Gamepad"), 0, 0);
	layd->addWidget(cbGPName, 0, 1);
	layd->addWidget(new QLabel("Dead zone"), 1, 0);
	layd->addWidget(sldDeadZone, 1, 1);
	layd->addWidget(new QLabel("Maping"), 2, 0);
	layd->addLayout(layc, 2, 1);
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(layd);
	lay->addLayout(layb);
	setLayout(lay);

	connect(cbGPName, SIGNAL(currentIndexChanged(int)), this, SLOT(devChanged(int)));
	connect(cbMapFile, SIGNAL(currentIndexChanged(int)), this, SLOT(mapChanged(int)));
	connect(tbAddMap, SIGNAL(released()), this, SLOT(addMap()));
	connect(tbDelMap, SIGNAL(released()), this, SLOT(delMap()));
	connect(tbAddEntry, SIGNAL(released()), this, SLOT(addEntry()));
	connect(tbEditEntry, SIGNAL(released()), this, SLOT(editEntry()));
	connect(tbDelEntry, SIGNAL(released()), this, SLOT(delEntry()));
	connect(tvMapView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editEntry()));
}

void xGamepadWidget::update(std::string mapname) {
	QStringList lst;
	QString str;
	cbGPName->blockSignals(true);
	cbGPName->clear();
	lst = gpad->getList();
	lst.prepend("none");
	cbGPName->addItems(lst);
    // After a reconnect, gpad->name() may be empty.
    // Use the stored name as the source of truth for selection.
    // TODO: investigate why name() returns empty after reconnect.
	str = gpad->getStoredName();
    // Fallback to index 0 ("none") if the name is not found.
	cbGPName->setCurrentIndex(std::max(0,cbGPName->findText(str)));
	cbGPName->blockSignals(false);
	// Disable dropdown if only "none" is available to avoid overwriting config
	// when gamepad is disconnected or sleeping.
	cbGPName->setEnabled(lst.size() > 1);

	sldDeadZone->setValue(gpad->deadZone());

	QDir dir(conf.path.confDir.c_str());
	cbMapFile->clear();
	lst = dir.entryList(QStringList() << "*.pad",QDir::Files,QDir::Name);
	lst.prepend("none");
	cbMapFile->addItems(lst);
	if (mapname.empty()) {
		cbMapFile->setCurrentIndex(0);
	} else {
		cbMapFile->setCurrentIndex(cbMapFile->findText(mapname.c_str()));
	}

	padmodel->update();
}

void xGamepadWidget::apply() {
	gpad->setDeadZone(sldDeadZone->value());
	if (cbGPName->currentIndex() < 1) {
		gpad->close();
        // If the list actually had devices, explicit "none" selection
        // should clear the stored name.
		if(cbGPName->count() > 1)
			gpad->setStoredName("");
	} else {
        // Persist the explicit selection and open the device.
		const QString gpname = cbGPName->currentText();
		gpad->setStoredName(gpname);
		gpad->open(gpname);
	}
}

std::string xGamepadWidget::getMapName() {
	std::string str;
	if (cbMapFile->currentIndex() == 0) return str;
	str = cbMapFile->currentText().toStdString();
	return str;
}

void xGamepadWidget::devChanged(int idx) {
	if (idx > 0) {			// 0 is 'none'
		gpad->open(idx-1);
	} else {
		gpad->close();
        // If the list actually had devices, explicit "none" selection
        // should clear the stored name.
		if(cbGPName->count() > 1)
			gpad->setStoredName("");
	}
}

void xGamepadWidget::mapChanged(int idx) {
	if (idx < 1) {
		gpad->mapClear();
	} else {
		gpad->loadMap(cbMapFile->currentText().toStdString());
	}
	padmodel->update();
}

void xGamepadWidget::addMap() {
	QString nam = QInputDialog::getText(this,"Enter...","New gamepad map name");
	if (nam.isEmpty()) return;
	nam.append(".pad");
	std::string name = nam.toStdString();
	if (padCreate(name)) {
		cbMapFile->addItem(nam, nam);
		cbMapFile->setCurrentIndex(cbMapFile->count() - 1);
	} else {
		showInfo("Map with that name already exists");
	}
}

void xGamepadWidget::delMap() {
	if (cbMapFile->currentIndex() == 0) return;
	QString name = cbMapFile->currentText();
	if (name.isEmpty()) return;
	if (!areSure("Delete this map?")) return;
	padDelete(name.toStdString());
	cbMapFile->removeItem(cbMapFile->currentIndex());
	cbMapFile->setCurrentIndex(0);
}

void xGamepadWidget::addEntry() {
	if (cbMapFile->currentIndex() == 0) return;
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
	gpad->saveMap(cbMapFile->currentText().toStdString());
	padmodel->update();
}

void xGamepadWidget::editEntry() {
	bindidx = tvMapView->currentIndex().row();
	if (bindidx < 0) return;
	emit s_edit_entry(gpad, gpad->mapItem(bindidx));
//	padial->start(conf.joy.gpad->mapItem(bindidx));
}

extern bool qmidx_greater(const QModelIndex, const QModelIndex);

void xGamepadWidget::delEntry() {
	QModelIndexList lst = tvMapView->selectionModel()->selectedRows();
	if (!lst.isEmpty()) {
		std::sort(lst.begin(), lst.end(), qmidx_greater);
		if (areSure("Delete this binding(s)?")) {
			int row;
			foreach(QModelIndex idx, lst) {
				row = idx.row();
				gpad->delItem(row); // map.erase(conf.joy.gpad->map.begin() + row);
			}
			padmodel->update();
			gpad->saveMap(cbMapFile->currentText().toStdString());
		}
	}
}
