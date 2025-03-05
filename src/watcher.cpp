#include "watcher.h"
#include "xcore/xcore.h"

#include <QInputDialog>
#include <QDebug>

// expression evaluation

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
				while (isLetter(c) || (c == 0x27)) {			// 0x27 = '
					if ((c >= 'a') && (c <= 'z')) c = c - 'a' + 'A';		// to capital
					*bptr = c;
					bptr++;
					res.ptr++;
					c = *res.ptr;
				}
				*bptr = 0;
				res.value = cpu_get_reg(conf.prof.cur->zx->cpu, buf, &err);
				res.err = err;			// 1 if 'no such reg'
				break;
		}
	}
	free(buf);
	return res;
}

// flag:
// b0: stop on ) with pointer increment
// b1: stop on ] with pointer increment
// b2: stop on ),],+,- and don't increment pointer

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
				} else if (f & 4) {
					exit = 1;
				} else {
					res.err = 1;	// ) without (
				}
				break;
			case ']':
				if (f & 2) {
					res.ptr++;
					exit = 1;
				} else if (f & 4) {
					exit = 1;
				} else {
					res.err = 1;	// ] without [
				}
				break;
			case '+':
				if (f & 4) {
					exit = 1;
				} else {
					// op = getOperand(res.ptr + 1);
					op = xEval(res.ptr + 1, 4);
					if (!op.err) {
						res.value += op.value;
						res.ptr = op.ptr;
					} else {
						res.err = 1;
					}
				}
				break;
			case '-':
				if (f & 4) {
					exit = 1;
				} else {
					// op = getOperand(res.ptr + 1);
					op = xEval(res.ptr + 1, 4);
					if (!op.err) {
						res.value -= op.value;
						res.ptr = op.ptr;
					} else {
						res.err = 1;
					}
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
	int i;
	ui.setupUi(this);
	model = new xWatchModel;
	ui.wchMemTab->setModel(model);
	ui.wchMemTab->addAction(ui.actAddWatcher);
	ui.wchMemTab->addAction(ui.actDelWatcher);

// like in deBUGa: pairs regName/regValue
	QLabel* lp;
	xHexSpin* hp;
	for(i = 0; i < 32; i++) {
		lp = new QLabel;
		hp = new xHexSpin;
		hp->setMaximumWidth(50);
		regLabels.append(lp);
		regValues.append(hp);
		ui.regGrid->addWidget(lp, i >> 1, (i & 1) << 1);
		ui.regGrid->addWidget(hp, i >> 1, ((i & 1) << 1) | 1);
	}
//	ui.regGrid->setRowStretch(32, 10);

	newWch = new QDialog(this);
	nui.setupUi(newWch);
	nui.cbType->addItem("CPU addr", WUT_CPU);
	nui.cbType->addItem("RAM addr", WUT_RAM);
	nui.cbType->addItem("ROM addr", WUT_ROM);

	for(i = 0; i < 14; i++) ui.wchMemTab->setColumnWidth(i, 30);

	connect(ui.actAddWatcher, SIGNAL(triggered(bool)), this, SLOT(newWatcher()));
	connect(ui.actDelWatcher, SIGNAL(triggered(bool)), this, SLOT(delWatcher()));
	connect(ui.wchMemTab, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(edtWatcher()));

	connect(nui.pbOK, SIGNAL(released()), this, SLOT(confirmNew()));
}

void xWatcher::show() {

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

int xWatcher::getCurRow() {
	int res = -1;
	QModelIndexList lst = ui.wchMemTab->selectionModel()->selectedRows();
	if (lst.size() == 1)
		res = lst.first().row();
	return res;
}

void xWatcher::newWatcher() {
	curwch = -1;
	newWch->show();
}

void xWatcher::confirmNew() {
	int type = nui.cbType->currentData().toInt();
	QString str = nui.leExpression->text();
	if (str.isEmpty()) return;
	xResult res = xEval(str.toLocal8Bit().data());		// check syntax
	if (res.err) return;
	newWch->close();
	if (curwch < 0) {
		model->addItem(type, str);
		ui.wchMemTab->setSpan((model->getItemCount() - 1) * 2, 0, 1, 14);
	} else {
		model->setItem(curwch, type, str);
	}
}

void xWatcher::delWatcher() {
	int row = getCurRow();
	if (row < 0) return;
	model->delItem(row);
}

void xWatcher::edtWatcher() {
	curwch = getCurRow();
	if (curwch < 0) return;
	xWatchItem itm = model->getItem(curwch);
	nui.cbType->setCurrentIndex(nui.cbType->findData(itm.type));
	nui.leExpression->setText(itm.exp);
	newWch->show();
}

// watcher view model

xWatchModel::xWatchModel() {
}

void xWatchModel::update() {
	emit QAbstractItemModel::dataChanged(index(0, 2), index(explist.size() - 1, 3));
}

QModelIndex xWatchModel::index(int row, int col, const QModelIndex&) const {
	QModelIndex res = createIndex(row, col, (void*)this);
	return res;
}

QModelIndex xWatchModel::parent(const QModelIndex&) const {
	return QModelIndex();
}

int xWatchModel::rowCount(const QModelIndex&) const {
	return explist.size() << 1;
}

int xWatchModel::columnCount(const QModelIndex&) const {
	return 13;
}

void xWatchModel::insertRow(int row, const QModelIndex& idx) {
	emit beginInsertRows(idx,row,row);
	emit endInsertRows();
}

void xWatchModel::removeRow(int row, const QModelIndex& idx) {
	emit beginRemoveRows(idx,row,row);
	emit endRemoveRows();
}

int xWatchModel::getItemCount() {
	return explist.size();
}

xWatchItem xWatchModel::getItem(int row) {
	return explist.at(row);
}

void xWatchModel::addItem(int type, QString exp) {
	xWatchItem itm;
	itm.type = type;
	itm.exp = exp;
	explist.append(itm);
	insertRow(explist.size() - 1);
	insertRow(explist.size() - 1);
}

void xWatchModel::setItem(int idx, int type, QString exp) {
	if (idx < 0) return;
	if (idx >= explist.size()) return;
	xWatchItem itm;
	itm.type = type;
	itm.exp = exp;
	explist[idx] = itm;
	emit QAbstractItemModel::dataChanged(index(idx, 0), index(idx, columnCount()));
}

void xWatchModel::delItem(int idx) {
	if (idx < explist.size()) {
		explist.removeAt(idx);
		removeRow(idx * 2);
		removeRow(idx * 2 + 1);
	}
}

QVariant xWatchModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount(idx))) return res;
	if ((col < 0) || (col >= columnCount(idx))) return res;
	xWatchItem itm;
	switch (role) {
		case Qt::DisplayRole:
			if (row & 1) {
				itm = explist.at(row >> 1);
				xResult xr = xEval(itm.exp.toLocal8Bit().data());
				if (xr.err) {
					res = "??";
				} else {
					res = gethexbyte(memRd(comp->mem, (xr.value + col) & comp->mem->busmask));
				}
			} else if (col == 0) {
				itm = explist.at(row >> 1);
				switch(itm.type) {
					case WUT_CPU: res = "CPU: "+itm.exp; break;
					case WUT_RAM: res = "RAM: "+itm.exp; break;
					case WUT_ROM: res = "ROM: "+itm.exp; break;
					default: res = "Error"; break;
				}
			}
	}
	return res;
}

/*
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
*/
