#include <SDL.h>

#include "padbinder.h"

enum {
	PBMODE_FREE = 0,
	PBMODE_KEY = 1,
	PBMODE_PAD = 2
};

xPadBinder::xPadBinder(QWidget* p):QDialog(p) {
	ui.setupUi(this);
	mode = PBMODE_FREE;

	ui.cbJoyList->addItem("Up", XJ_UP);
	ui.cbJoyList->addItem("Down", XJ_DOWN);
	ui.cbJoyList->addItem("Left", XJ_LEFT);
	ui.cbJoyList->addItem("Right", XJ_RIGHT);
	ui.cbJoyList->addItem("Fire", XJ_FIRE);
	ui.cbJoyList->addItem("Button 2", XJ_BUT2);
	ui.cbJoyList->addItem("Button 3", XJ_BUT3);
	ui.cbJoyList->addItem("Button 4", XJ_BUT4);

	ui.cbMouseList->addItem("Up", XM_UP);
	ui.cbMouseList->addItem("Down", XM_DOWN);
	ui.cbMouseList->addItem("Left", XM_LEFT);
	ui.cbMouseList->addItem("Right", XM_RIGHT);
	ui.cbMouseList->addItem("Left button", XM_LMB);
	ui.cbMouseList->addItem("Mid button", XM_MMB);
	ui.cbMouseList->addItem("Right button", XM_RMB);
	ui.cbMouseList->addItem("Wheel up", XM_WHEELUP);
	ui.cbMouseList->addItem("Wheel down", XM_WHEELDN);

	connect(&timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	connect(ui.pbPadBind, SIGNAL(clicked(bool)), this, SLOT(startBindPad()));
	connect(ui.pbKeyBind, SIGNAL(clicked(bool)), this, SLOT(startBindKey()));
	connect(ui.cbJoyList, SIGNAL(currentIndexChanged(int)), this, SLOT(setJoyDir()));
	connect(ui.cbMouseList, SIGNAL(currentIndexChanged(int)), this, SLOT(setMouseDir()));
	connect(ui.pbOk, SIGNAL(clicked(bool)), this, SLOT(okPress()));
	connect(ui.pbRepSlider, SIGNAL(valueChanged(int)), this, SLOT(onRepSlider(int)));

}

void xPadBinder::start(xJoyMapEntry e) {
	ent = e;
	setPadButtonText();
	setKeyButtonText();
	ui.pbRepSlider->setValue(ent.rpt);
	timer.start(20);
	show();
}

void xPadBinder::close() {
	timer.stop();
	QDialog::close();
}

void xPadBinder::onRepSlider(int val) {
	QString num = QString::number(val / 50.0);
	ui.pbRepLabel->setText(QString("%0 sec").arg(num));
}

void xPadBinder::setKeyButtonText() {
	switch(ent.dev) {
		case JMAP_KEY:
			ui.rbKey->setChecked(true);
			ui.pbKeyBind->setText(QString("Key %0").arg(getKeyNameById(ent.key)));
			break;
		case JMAP_JOY:
			ui.pbKeyBind->setText("Push to bind");
			ui.rbJoy->setChecked(true);
			ui.cbJoyList->setCurrentIndex(ui.cbJoyList->findData(ent.dir));
			break;
		case JMAP_MOUSE:
			ui.pbKeyBind->setText("Push to bind");
			ui.rbMouse->setChecked(true);
			ui.cbMouseList->setCurrentIndex(ui.cbMouseList->findData(ent.dir));
			break;
		default:
			ui.pbKeyBind->setText("Push to bind");
			ent.dev = JMAP_NONE;
			break;
	}
	ui.pbOk->setEnabled((ent.type != JOY_NONE) && (ent.dev != JMAP_NONE));
}

void xPadBinder::setPadButtonText() {
	QString dir;
	switch (ent.type) {
		case JOY_HAT:
			switch(ent.state) {
				case SDL_HAT_UP: dir = "up"; break;
				case SDL_HAT_DOWN: dir = "down"; break;
				case SDL_HAT_LEFT: dir = "left"; break;
				case SDL_HAT_RIGHT: dir = "right"; break;
			}
			ui.pbPadBind->setText(QString("Hat %0 %1").arg(ent.num).arg(dir));
			break;
		case JOY_BUTTON:
			ui.pbPadBind->setText(QString("Button %0").arg(ent.num));
			break;
		case JOY_AXIS:
			ui.pbPadBind->setText(QString("Axis %0 %1").arg(ent.num).arg((ent.state < 0) ? "-" : "+"));
			break;
		default:
			ent.type = JOY_NONE;
			ui.pbPadBind->setText("Push to bind");
			break;
	}
	ui.pbOk->setEnabled((ent.type != JOY_NONE) && (ent.dev != JMAP_NONE));
}

// set kempston bind

void xPadBinder::setJoyDir() {
	ui.rbJoy->setChecked(true);
	ent.dev = JMAP_JOY;
	ent.key = ENDKEY;
	ent.dir = ui.cbJoyList->itemData(ui.cbJoyList->currentIndex()).toInt();
}

void xPadBinder::setMouseDir() {
	ui.rbMouse->setChecked(true);
	ent.dev = JMAP_MOUSE;
	ent.key = ENDKEY;
	ent.dir = ui.cbMouseList->itemData(ui.cbMouseList->currentIndex()).toInt();
}

// set key bind

void xPadBinder::startBindKey() {
	mode = PBMODE_KEY;
	ui.pbKeyBind->setText("...scan...");
	ui.rbKey->setChecked(true);
	grabKeyboard();
}

void xPadBinder::keyPressEvent(QKeyEvent* ev) {
	ev->ignore();
	if (ev->key() == Qt::Key_Escape) {
		releaseKeyboard();
		if (mode == PBMODE_FREE) {
			close();
		} else {
			mode = PBMODE_FREE;
			setPadButtonText();
			setKeyButtonText();
		}
	} else if (mode == PBMODE_KEY) {
		mode = PBMODE_FREE;
		int id = qKey2id(ev->key());
		if (id == ENDKEY) return;
		ent.dev = JMAP_KEY;
		ent.key = id;
		ent.dir = XJ_NONE;
		setKeyButtonText();
		releaseKeyboard();
	}
}

// OK pressed

void xPadBinder::okPress() {
	if (ent.type == JOY_NONE) return;
	ent.rpt = ui.pbRepSlider->value();
	if (ui.rbKey->isChecked()) {
		ent.dev = JMAP_KEY;
		ent.dir = XJ_NONE;
		if (ent.key == ENDKEY) return;
	}
	if (ui.rbJoy->isChecked()) {
		ent.dev = JMAP_JOY;
		ent.dir = ui.cbJoyList->itemData(ui.cbJoyList->currentIndex()).toInt();
		ent.key = ENDKEY;
		if (ent.dir == XJ_NONE) return;
	}
	close();
	emit bindReady(ent);
}

// scan gamepad

void xPadBinder::startBindPad() {
	if (!conf.joy.joy) {
		ent.type = JOY_NONE;
		ent.num = 0;
		ent.state = 0;
	} else {
		ui.pbPadBind->setText("...scan...");
		mode = PBMODE_PAD;
	}
}

void xPadBinder::onTimer() {
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		if (mode == PBMODE_PAD) {
			switch(ev.type) {
				case SDL_JOYAXISMOTION:
					if (abs(ev.jaxis.value) < conf.joy.dead) break;
					ent.type = JOY_AXIS;
					ent.num = ev.jaxis.axis;
					ent.state = (ev.jaxis.value < 0) ? -1 : +1;
					mode = PBMODE_FREE;
					break;
				case SDL_JOYBUTTONDOWN:
					ent.type = JOY_BUTTON;
					ent.num = ev.jbutton.button;
					mode = PBMODE_FREE;
					break;
				case SDL_JOYHATMOTION:
					ent.type = JOY_HAT;
					ent.num = ev.jhat.hat;
					ent.state = ev.jhat.value;
					mode = PBMODE_FREE;
					break;
			}
		}
	}
	if (mode != PBMODE_PAD) {
		setPadButtonText();
	}
}

// pad map table model

// PAD MAP MODEL

xPadMapModel::xPadMapModel(QObject* p):QAbstractItemModel(p) {
}

QModelIndex xPadMapModel::index(int row, int col, const QModelIndex& idx) const {
	return createIndex(row, col);
}

QModelIndex xPadMapModel::parent(const QModelIndex& idx) const {
	return QModelIndex();
}

int xPadMapModel::rowCount(const QModelIndex& par) const {
	if (par.isValid()) return 0;
	return (int)conf.joy.map.size();
}

int xPadMapModel::columnCount(const QModelIndex& par) const {
	if (par.isValid()) return 0;
	return 3;
}

QVariant xPadMapModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return res;
	if ((col < 0) || (col >= columnCount())) return res;
	xJoyMapEntry jent = conf.joy.map[row];
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
					res = str;
					break;
				case 1:
					switch(jent.dev) {
						case JMAP_KEY:
							str = QString("Key %0").arg(getKeyNameById(jent.key));
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
