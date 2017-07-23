#include "../../xcore/xcore.h"
#include "dbg_brkpoints.h"

extern unsigned short disasmAdr;

// Model

xBreakListModel::xBreakListModel(QObject* par):QAbstractTableModel(par) {

}

int xBreakListModel::rowCount(const QModelIndex& idx) const {
	if (!conf.prof.cur) return 0;
	return conf.prof.cur->brkList.size();
}

int xBreakListModel::columnCount(const QModelIndex& idx) const {
	return 5;
}

QString brkGetString(xBrkPoint brk) {
	QString res;
	switch(brk.type) {
		case BRK_CPUADR:
			res = QString("CPU:%0").arg(gethexword(brk.adr));
			break;
		case BRK_IOPORT:
			res = QString("IO:%0 mask %0").arg(gethexword(brk.adr)).arg(gethexword(brk.mask));
			break;
		case BRK_MEMRAM:
			res = QString("RAM:%0:%1").arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			break;
		case BRK_MEMROM:
			res = QString("ROM:%0:%1").arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			break;
		case BRK_MEMSLT:
			res = QString("SLT:%0:%1").arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			break;
		case BRK_MEMEXT:
			res = QString("EXT:%0:%1").arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			break;
	}
	return res;
}

QVariant xBreakListModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((col < 0) || (col >= columnCount())) return res;
	if ((row < 0) || (row >= rowCount())) return res;
	xBrkPoint brk = conf.prof.cur->brkList[row];
	switch (role) {
		case Qt::CheckStateRole:
			switch(col) {
				case 0: res = brk.off ? Qt::Unchecked : Qt::Checked; break;
				case 1: res = brk.fetch ? Qt::Checked : Qt::Unchecked; break;
				case 2: res = brk.read ? Qt::Checked : Qt::Unchecked; break;
				case 3: res = brk.write ? Qt::Checked : Qt::Unchecked; break;
			}
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 4: res = brkGetString(brk); break;
			}
			break;
	}
	return res;
}

QVariant xBreakListModel::headerData(int sect, Qt::Orientation ornt, int role) const {
	QVariant res;
	switch(ornt) {
		case Qt::Horizontal:
			if (sect < 0) break;
			if (sect >= columnCount()) break;
			switch(role) {
				case Qt::DisplayRole:
					switch(sect) {
						case 0: res = "On"; break;
						case 1: res = "F"; break;
						case 2: res = "R"; break;
						case 3: res = "W"; break;
						case 4: res = "Addr"; break;
					}
					break;
			}
			break;
		case Qt::Vertical:
			break;
	}
	return res;
}

bool xbsOff(xBrkPoint& bpa, xBrkPoint& bpb) {return (bpa.off && !bpb.off);}
bool xbsFe(xBrkPoint& bpa, xBrkPoint& bpb) {return (bpa.fetch && !bpb.fetch);}
bool xbsRd(xBrkPoint& bpa, xBrkPoint& bpb) {return (bpa.read && !bpb.read);}
bool xbsWr(xBrkPoint& bpa, xBrkPoint& bpb) {return (bpa.write && !bpb.write);}
bool xbsName(xBrkPoint& bpa, xBrkPoint& bpb) {
	return brkGetString(bpa) < brkGetString(bpb);
}

void xBreakListModel::sort(int col, Qt::SortOrder ord) {
	if (!conf.prof.cur) return;
	switch(col) {
		case 0: qSort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsOff); break;
		case 1: qSort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsFe); break;
		case 2: qSort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsRd); break;
		case 3: qSort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsWr); break;
		case 4: qSort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsName); break;
	}
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
}

void xBreakListModel::updateCell(int row, int col) {
	emit dataChanged(index(row, col), index(row, col));
}

void xBreakListModel::update() {
	emit endResetModel();
}

// Widget

xBreakTable::xBreakTable(QWidget* p):QTableView(p) {
	model = new xBreakListModel();
	setModel(model);
	setColumnWidth(0, 30);
	setColumnWidth(1, 30);
	setColumnWidth(2, 30);
	setColumnWidth(3, 30);
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onCellClick(QModelIndex)));
	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClick(QModelIndex)));
}

void xBreakTable::update() {
	model->update();
	QTableView::update();
}

void xBreakTable::keyPressEvent(QKeyEvent* ev) {
	ev->ignore();
}

void xBreakTable::onCellClick(QModelIndex idx) {
	if (!idx.isValid()) return;
	int row = idx.row();
	int col = idx.column();
	xProfile* prf = conf.prof.cur;
	xBrkPoint* brk = &prf->brkList[row];
	switch(col) {
		case 0: brk->off ^= 1; break;
		case 1: brk->fetch ^= 1; break;
		case 2: brk->read ^= 1; break;
		case 3: brk->write ^= 1; break;
	}
	if (brk->fetch || brk->read || brk->write) {
		model->updateCell(row, col);
		brkInstall(prf->brkList[row]);
	} else {
		brkInstall(prf->brkList[row]);		// when row removed
		update();
	}
	emit rqDasmDump();
}

void xBreakTable::onDoubleClick(QModelIndex idx) {
	if (!idx.isValid()) return;
	int row = idx.row();
	xBrkPoint bp = conf.prof.cur->brkList[row];
	int adr = -1;
	switch(bp.type) {
		case BRK_CPUADR: adr = bp.adr & 0xffff; break;
		case BRK_MEMRAM: adr = memFindAdr(conf.prof.cur->zx->mem, MEM_RAM, bp.adr); break;
		case BRK_MEMROM: adr = memFindAdr(conf.prof.cur->zx->mem, MEM_ROM, bp.adr); break;
	}
	if (adr < 0) return;
	disasmAdr = adr & 0xffff;
	emit rqDisasm();
}

// Dialog

xBrkManager::xBrkManager(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	ui.brkType->addItem("CPU ADR bus", BRK_CPUADR);
//	ui.brkType->addItem("IO address", BRK_IOPORT);
	ui.brkType->addItem("RAM cell", BRK_MEMRAM);
	ui.brkType->addItem("ROM cell", BRK_MEMROM);
	ui.brkType->addItem("SLT cell", BRK_MEMCELL);

	connect(ui.brkAdrDec, SIGNAL(valueChanged(int)), this, SLOT(adrDec2hex(int)));
	connect(ui.brkAdrHex, SIGNAL(textChanged(QString)), this, SLOT(adrHex2dec(QString)));
	connect(ui.pbOK, SIGNAL(clicked()), this, SLOT(confirm()));
}

void xBrkManager::adrDec2hex(int val) {
	QString hex = gethexword(val).toUpper();
	int pos = ui.brkAdrHex->cursorPosition();
	ui.brkAdrHex->setText(hex);
	ui.brkAdrHex->setCursorPosition(pos);
}

void xBrkManager::adrHex2dec(QString hex) {
	int val = hex.toInt(NULL, 16);
	ui.brkAdrDec->setValue(val);
}

void xBrkManager::edit(xBrkPoint* sbrk) {
	if (sbrk) {
		obrk = *sbrk;
		obrk.off = 0;
	} else {
		obrk.type = BRK_MEMRAM;
		obrk.adr = 0;
		obrk.mask = 0;
		obrk.size = 1;
		obrk.off = 0;
		obrk.fetch = 1;
		obrk.read = 0;
		obrk.write = 0;
		obrk.off = 1;
	}
	ui.brkType->setCurrentIndex(ui.brkType->findData(obrk.type));
	ui.brkFetch->setChecked(obrk.fetch);
	ui.brkRead->setChecked(obrk.read);
	ui.brkWrite->setChecked(obrk.write);
	switch(obrk.type) {
		case BRK_CPUADR:
		case BRK_IOPORT:
			ui.brkBank->setValue(0);
			ui.brkAdrDec->setValue(obrk.adr);
			break;
		default:
			ui.brkBank->setValue(obrk.adr >> 14);
			ui.brkAdrDec->setValue(obrk.adr & 0x3fff);
			break;
	}
	ui.brkMaskHex->setText("FFFF");
	adrDec2hex(ui.brkAdrDec->value());
	show();
}

void xBrkManager::confirm() {
	xBrkPoint brk;
	brk.off = 0;
	brk.size = 1;
	brk.type = ui.brkType->itemData(ui.brkType->currentIndex()).toInt();
	brk.fetch = ui.brkFetch->isChecked() ? 1 : 0;
	brk.read = ui.brkRead->isChecked() ? 1 : 0;
	brk.write = ui.brkWrite->isChecked() ? 1 : 0;
	switch (brk.type) {
		case BRK_CPUADR:
		case BRK_IOPORT:
			brk.adr = ui.brkAdrDec->value();
			break;
		default:
			brk.adr = (ui.brkBank->value() << 14) | (ui.brkAdrDec->value() & 0x3fff);
			break;
	}
	brk.mask = ui.brkMaskHex->text().toInt(NULL, 16);
	emit completed(obrk, brk);
	hide();
}
