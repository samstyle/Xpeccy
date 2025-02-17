#include "watcher.h"
#include "xcore/xcore.h"

#include <QDebug>

// expression evaluation
// current problem: operation priority. 2+2*2=8, but 2+(2*2)=6

typedef struct {
	int err;
	int value;
	const char* ptr;
} xResult;

bool isDigit(char c, int base) {
	if (c < '0') return 0;
	if (c <= '9') {
		return ((c - '0') < base);
	} else if (c < 'A') {
		return 0;
	} else if (c - 'A' + 10 < base) {
		return 1;
	} else if (c < 'a') {
		return 0;
	} else {
		return (c - 'a' + 10 < base);
	}
}

bool isLetter(char c) {
	if (c < 'A') return 0;
	if (c <= 'Z') return 1;
	if (c < 'a') return 0;
	return (c <= 'z');
}

xResult xEval(const char*, int);

// return:
//	res.value = value of number/register/label
//	res.err = 1 if error
//	res.ptr = next char pointer
xResult getOperand(const char* ptr) {
	xResult res;
	res.err = 0;
	res.value = 0;
	while (*ptr == ' ') ptr++;		// skip leading spaces
	res.ptr = ptr;
	char c = *ptr;
	char* buf = (char*)malloc(strlen(ptr) + 1);
	char* bptr;
	bool err;
	memset(buf, 0, strlen(ptr) + 1);
	int val;
	int base = conf.prof.cur->zx->hw->base;

	if (isDigit(c, base)) {	// number
		if ((c == '0') && (*(res.ptr + 1) == 'x')) {		// 0x... - hex
			res.ptr += 2;
			base = 16;
			c = *res.ptr;
		}
		do {
			res.value *= base;
			if (base > 10) {
				if ((c >= 'A') && (c < 'A' + base - 10)) {
					res.value += 10 + (c - 'A');
				} else if ((c >= 'a') && (c < 'a' + base - 10)) {
					res.value += 10 + (c - 'a');
				} else {
					res.value += (c - '0');
				}
			} else if (c - '0' < base) {
				res.value += (c - '0');
			} else {
				res.err = 1;
			}
			res.ptr++;
			c = *res.ptr;
		} while (isDigit(c, base));
	} else if (isLetter(c)) {	// label: collect letters|digits|_|. in buf, search label
		bptr = buf;
		do {
			*bptr = c;
			bptr++;
			res.ptr++;
			c = *res.ptr;
		} while (isLetter(c) || isDigit(c, base) || (c == '_') || (c == '.'));
		*bptr = 0;
		if (conf.prof.cur->labels.contains(buf)) {
			res.value = conf.prof.cur->labels[buf].adr;
		} else {
			res.err = 1;
		}
	} else {
		switch(c) {
			case '(':		// sub-expression (...)
				res = xEval(res.ptr + 1, 1);	// stop on )
				break;
			case '[':		// [exp] = word(L-H) on address exp
				res = xEval(res.ptr + 1, 2);	// stop on ]
				if (!res.err) {
					val = memRd(conf.prof.cur->zx->mem, res.value);
					val |= (memRd(conf.prof.cur->zx->mem, res.value + 1) << 8);
					res.value = val;
				}
				break;
			case '.':		// register: collect letters in buf, ask cpu for value
				res.ptr++;
				c = *res.ptr;
				bptr = buf;
				while (isLetter(c)) {
					if ((c >= 'a') && (c <= 'z')) c = c - 'a' + 'A';		// to capital
					*bptr = c;
					bptr++;
					res.ptr++;
					c = *res.ptr;
				}
				*bptr = 0;
				qDebug() << buf;
				res.value = cpu_get_reg(conf.prof.cur->zx->cpu, buf, &err);
				res.err = err;			// 1 if 'no such reg'
				break;
		}
	}
	free(buf);
	return res;
}

// flag:
// b0: stop on )
// b1: stop on ]

xResult xEval(const char* ptr, int f = 0) {
	xResult res = getOperand(ptr);
	xResult op;
	int exit = 0;
	while (!res.err && *res.ptr && !exit) {
		switch(*res.ptr) {
			case ' ':			// skip spaces
				res.ptr++;
				break;
			case ')':
				if (f & 1) {
					res.ptr++;
					exit = 1;
				} else {
					res.err = 1;	// ) without (
				}
				break;
			case ']':
				if (f & 2) {
					res.ptr++;
					exit = 1;
				} else {
					res.err = 1;	// ] without [
				}
				break;
			case '+':
				op = getOperand(res.ptr + 1);
				if (!op.err) {
					res.value += op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			case '-':
				op = getOperand(res.ptr + 1);
				if (!op.err) {
					res.value -= op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			case '*':
				op = getOperand(res.ptr + 1);
				if (!op.err) {
					res.value *= op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			case '/':
				op = getOperand(res.ptr + 1);
				if (!op.err && op.value) {
					res.value /= op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			case '&':
				op = getOperand(res.ptr + 1);
				if (!op.err) {
					res.value &= op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			case '|':
				op = getOperand(res.ptr + 1);
				if (!op.err) {
					res.value |= op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			case '^':
				op = getOperand(res.ptr + 1);
				if (!op.err) {
					res.value ^= op.value;
					res.ptr = op.ptr;
				} else {
					res.err = 1;
				}
				break;
			default:
				res.err = 1;
				break;
		}
	}
	return res;
}

// wutcha

xWatcher::xWatcher(QWidget* p):QDialog(p) {
	addial = new QDialog(this);
	nui.setupUi(addial);
	nui.cbType->addItem("Address", wchAddress);
	nui.cbType->addItem("Memory cell", wchCell);
	nui.cbMemType->addItem("RAM", MEM_RAM);
	nui.cbMemType->addItem("ROM", MEM_ROM);
// fill registers
	fillRegs();

	ui.setupUi(this);
	model = new xWatchModel;
	ui.wchMemTab->setModel(model);
	ui.wchMemTab->addAction(ui.actAddWatcher);
	ui.wchMemTab->addAction(ui.actDelWatcher);

// like in deBUGa: pairs regName/regValue
	QLabel* lp;
	xHexSpin* hp;
	for(int i = 0; i < 32; i++) {
		lp = new QLabel;
		hp = new xHexSpin;
		regLabels.append(lp);
		regValues.append(hp);
		ui.regGrid->addWidget(lp, i >> 1, (i & 1) << 1);
		ui.regGrid->addWidget(hp, i >> 1, ((i & 1) << 1) | 1);
	}
	ui.regGrid->setRowStretch(32, 10);

	listwin = new xLabeList(addial);

	connect(ui.actAddWatcher, SIGNAL(triggered(bool)), this, SLOT(newWatcher()));
	connect(ui.actDelWatcher, SIGNAL(triggered(bool)), this, SLOT(delWatcher()));
	connect(ui.wchMemTab, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(edtWatcher()));

	connect(nui.tbLabel, SIGNAL(clicked()), listwin, SLOT(show()));
	connect(nui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(dialChanged()));
	connect(nui.cbSrcReg, SIGNAL(currentIndexChanged(int)), this, SLOT(dialChanged()));
	connect(nui.pbOK, SIGNAL(clicked(bool)),this,SLOT(addWatcher()));

	connect(listwin, SIGNAL(labSelected(QString)), this, SLOT(setLabel(QString)));
}

void xWatcher::show() {
	fillRegs();

	xRegBunch regs = cpuGetRegs(conf.prof.cur->zx->cpu);
	int work = 1;
	for(int i = 0; i < 32; i++) {
		if (work) {
			if (regs.regs[i].id == REG_NONE) {
				work = 0;
				regLabels[i]->setVisible(false);
				regValues[i]->setVisible(false);
			} else {
				regLabels[i]->setVisible(true);
				regValues[i]->setVisible(true);
				regLabels[i]->setText(regs.regs[i].name);
				regValues[i]->setValue(regs.regs[i].value);
			}
		} else {
			regLabels[i]->setVisible(false);
			regValues[i]->setVisible(false);
		}
	}

	QDialog::show();
}

void xWatcher::setLabel(QString str) {
	if (conf.prof.cur->labels.contains(str)) {
		xAdr xadr = conf.prof.cur->labels[str];
		nui.leAdrHex->setValue(xadr.adr);
	}
}

QString getBankType(int type) {
	QString res;
	switch(type) {
		case MEM_ROM: res = "ROM"; break;
		case MEM_RAM: res = "RAM"; break;
		case MEM_SLOT: res = "SLT"; break;
		default: res = "EXT"; break;
	}
	return res;
}

QString getBankName(MemPage pg) {
	return QString("%0:%1").arg(getBankType(pg.type), gethexbyte(pg.num >> 6));
}

void xWatcher::fillFields(Computer* comp) {
	if (!isVisible()) return;
	if (comp == NULL) return;
	model->comp = comp;
// TODO: remake for universal CPU
	xRegBunch regs = cpuGetRegs(comp->cpu);
	int i = 0;
	while ((i < 32) && (regs.regs[i].id != REG_NONE)) {
		// regLabels[i]->setText(regs.regs[i].name);
		regValues[i]->setValue(regs.regs[i].value);
		i++;
	}
	ui.wchBank0->setText(getBankName(comp->mem->map[0x00]));
	ui.wchBank1->setText(getBankName(comp->mem->map[0x40]));
	ui.wchBank2->setText(getBankName(comp->mem->map[0x80]));
	ui.wchBank3->setText(getBankName(comp->mem->map[0xc0]));

	model->update();
}

void xWatcher::fillRegs() {
	nui.cbSrcReg->clear();
	nui.cbSrcReg->addItem("Absolute", wchAbsolute);
	xRegBunch regs = cpuGetRegs(conf.prof.cur->zx->cpu);
	int i = 0;
	while (regs.regs[i].type != REG_NONE) {
		if (regs.regs[i].type & REG_RDMP) {
			nui.cbSrcReg->addItem(regs.regs[i].name, regs.regs[i].id);
		}
		i++;
	}
}

void xWatcher::addWatcher() {
	addial->hide();
	int type = nui.cbType->itemData(nui.cbType->currentIndex()).toInt();
	xAdr xadr = {MEM_ROM, 0, 0, 0};
	switch (type) {
		case wchAddress:
			xadr.type = -1;
			xadr.bank = -1;
			xadr.adr = 0;
			xadr.abs = nui.cbSrcReg->itemData(nui.cbSrcReg->currentIndex()).toInt();	// addressation type
			if (xadr.abs == wchAbsolute) {
				xadr.adr = nui.leAdrHex->getValue();
			} else {
				xadr.adr = nui.sbShift->value();
			}
			break;
		case wchCell:
			xadr.type = nui.cbMemType->itemData(nui.cbMemType->currentIndex()).toInt();	// ram/rom
			xadr.bank = nui.sbMemPage->value();
			xadr.adr = nui.leAdrHex->getValue();
			xadr.abs = wchCell;
			break;
	}
	if (curwch < 0) {
		model->addItem(xadr);
	} else {
		model->updItem(curwch, xadr);
	}
}

int xWatcher::getCurRow() {
	int res = -1;
	QModelIndexList lst = ui.wchMemTab->selectionModel()->selectedRows();
	if (lst.size() == 1)
		res = lst.first().row();
	return res;
}

void xWatcher::newWatcher() {
	curwch = -1;
	fillDial();
	addial->show();
}

void xWatcher::delWatcher() {
	int row = getCurRow();
	if (row < 0) return;
	model->delItem(row);
}

void xWatcher::edtWatcher() {
	int row = getCurRow();
	if (row < 0) return;
	curwch = row;
	fillDial();
	addial->show();
}

void xWatcher::fillDial() {
	if (curwch < 0) {
		nui.cbMemType->setCurrentIndex(0);
		nui.cbSrcReg->setCurrentIndex(0);
		nui.cbType->setCurrentIndex(0);
		nui.leAdrHex->setValue(0);
		nui.sbMemPage->setValue(0);
		nui.sbShift->setValue(0);
	} else {
		xAdr xadr = model->getItem(curwch);
		if (xadr.abs == wchCell) {
			nui.cbType->setCurrentIndex(nui.cbType->findData(wchCell));
			nui.cbMemType->setCurrentIndex(nui.cbMemType->findData(xadr.type));
			nui.sbMemPage->setValue(xadr.bank);
			nui.leAdrHex->setValue(xadr.adr);
		} else {
			nui.cbType->setCurrentIndex(nui.cbType->findData(wchAddress));
			nui.cbSrcReg->setCurrentIndex(nui.cbSrcReg->findData(xadr.abs));
			if (xadr.abs == wchAbsolute) {
				nui.leAdrHex->setValue(xadr.adr);
			} else {
				nui.sbShift->setValue(xadr.adr);
			}
		}
	}
}

void xWatcher::dialChanged() {
	int type = nui.cbType->itemData(nui.cbType->currentIndex()).toInt();
	if (type == wchCell) {
		nui.cbMemType->setEnabled(true);
		nui.sbMemPage->setEnabled(true);
//		nui.sbAdrDec->setEnabled(true);
		nui.leAdrHex->setEnabled(true);
		nui.cbSrcReg->setEnabled(false);
		nui.sbShift->setEnabled(false);
	} else {
		nui.cbMemType->setEnabled(false);
		nui.sbMemPage->setEnabled(false);
		nui.cbSrcReg->setEnabled(true);
		type = nui.cbSrcReg->itemData(nui.cbSrcReg->currentIndex()).toInt();
		nui.sbShift->setDisabled(type == wchAbsolute);
		nui.leAdrHex->setEnabled(type == wchAbsolute);
	}
}

// watcher view model

xWatchModel::xWatchModel() {
}

void xWatchModel::update() {
	QModelIndex sti = index(0, 2);
	QModelIndex ste = index(list.size() - 1, 3);
	emit QAbstractItemModel::dataChanged(sti, ste);
}

QModelIndex xWatchModel::index(int row, int col, const QModelIndex&) const {
	QModelIndex res = createIndex(row, col, (void*)this);
	return res;
}

QModelIndex xWatchModel::parent(const QModelIndex&) const {
	return QModelIndex();
}

int xWatchModel::rowCount(const QModelIndex&) const {
	return list.size();
}

int xWatchModel::columnCount(const QModelIndex&) const {
	return 2;
}

void xWatchModel::insertRow(int row, const QModelIndex& idx) {
	emit beginInsertRows(idx,row,row);
	emit endInsertRows();
}

void xWatchModel::removeRow(int row, const QModelIndex& idx) {
	emit beginRemoveRows(idx,row,row);
	emit endRemoveRows();
}

xAdr xWatchModel::getItem(int idx) {
	xAdr xadr = {MEM_ROM, 0, 0, 0};
	if (idx < 0) return xadr;
	if (idx >= list.size()) return xadr;
	return list[idx];
}

void xWatchModel::addItem(xAdr xadr) {
	list.append(xadr);
	insertRow(list.size() - 1);
}

void xWatchModel::updItem(int idx, xAdr xadr) {
	if (idx < 0) return;
	if (idx >= list.size()) return;
	list[idx] = xadr;
	QModelIndex sti = index(idx, 0);
	QModelIndex ste = index(idx, 3);
	emit QAbstractItemModel::dataChanged(sti, ste);
}

void xWatchModel::delItem(int idx) {
	if (idx >= list.size()) return;
	list.removeAt(idx);
	removeRow(idx);
}

QVariant xWatchModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount(idx))) return res;
	if ((col < 0) || (col >= columnCount(idx))) return res;
	QString adrs;
	QString bytz;
	int i;
	switch (role) {
		/*
		case Qt::BackgroundColorRole:
			if (row == currow) {
				res = QColor(32,200,32);
			} else {
				res = QColor(255,255,255);
			}
			break;
		*/
		case Qt::DisplayRole:
			xAdr xadr = list[row];
			int adr;
			int msk = 0x3fffff;
			unsigned char* dat = NULL;
			if (xadr.abs == wchCell) {
				adr = (xadr.bank << 14) | (xadr.adr & 0x3fff);
				switch(xadr.type) {
					case MEM_ROM:
						msk = comp->mem->romMask;
						dat = comp->mem->romData;
						adrs = QString("ROM:%0:%1").arg(gethexbyte(xadr.bank)).arg(gethexword(xadr.adr & 0x3fff));
						break;
					case MEM_RAM:
						msk = comp->mem->ramMask;
						dat = comp->mem->ramData;
						adrs = QString("RAM:%0:%1").arg(gethexbyte(xadr.bank)).arg(gethexword(xadr.adr & 0x3fff));
						break;
				}
				bytz.clear();
				for (i = 0; i < 8; i++) {
					if (!bytz.isEmpty())
						bytz.append(":");
					bytz.append(gethexbyte(dat ? dat[(adr + i) & msk] : 0xff));
				}
			} else {
				adr = -1;
				switch(xadr.abs) {
					case wchAbsolute:
						adr = xadr.adr;
						adrs = find_label(xadr);
						if (adrs.isEmpty()) {
							adrs = gethexword(adr);
						}
						break;
					default:
						xRegister reg = cpuGetReg(comp->cpu, xadr.abs);
						adr = reg.value + xadr.adr;
						adrs = QString(reg.name).toUpper();
						break;
				}
				if (xadr.abs != wchAbsolute) {
					adrs.append(getdecshift(xadr.adr & 0xff));
				}
				bytz.clear();
				for (i = 0; i < 8; i++) {
					if (!bytz.isEmpty())
						bytz.append(":");
					bytz.append(gethexbyte(memRd(comp->mem, (adr + i) & 0xffff)));
				}
			}
			switch (col) {
				case 0:	res = adrs; break;
				case 1: res = bytz; break;
			}
	}
	return res;
}

QString xwhdname[5] = {"Addr","Bytes"};

QVariant xWatchModel::headerData(int col, Qt::Orientation orien, int role) const {
	QVariant res;
	switch (role) {
		case Qt::DisplayRole:
			switch (orien) {
				case Qt::Vertical:
					break;
				case Qt::Horizontal:
					if (col < 2) res = xwhdname[col];
					break;
			}
			break;
	}
	return res;
}
