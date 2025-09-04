#include <QFileDialog>

#include "xcore/xcore.h"
#include "dbg_brkpoints.h"

// Model

xBreakListModel::xBreakListModel(QObject* par):xTableModel(par) {

}

int xBreakListModel::rowCount(const QModelIndex&) const {
	if (!conf.prof.cur) return 0;
	return conf.prof.cur->brkList.size();
}

int xBreakListModel::columnCount(const QModelIndex&) const {
	return 6;
}

QString brkGetString(xBrkPoint brk) {
	QString res;
	switch(brk.type) {
		case BRK_CPUADR:
			res = QString("CPU:%0").arg(gethexword(brk.adr));
			if (brk.eadr > brk.adr) {
				res.append(QString("-%0").arg(gethexword(brk.eadr)));
			}
			break;
		case BRK_IOPORT:
			res = QString("IO:%0 mask %1").arg(gethexword(brk.adr)).arg(gethexword(brk.mask));
			break;
		case BRK_MEMRAM:
			if (brk.eadr > brk.adr) {
				res = QString("RAM:%0-%1 [%2:%3-%4:%5]").arg(gethex6(brk.adr)).arg(gethex6(brk.eadr)).arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff)).arg(gethexbyte(brk.eadr >> 14)).arg(gethexword(brk.eadr & 0x3fff));
			} else {
				res = QString("RAM:%0 [%1:%2]").arg(gethex6(brk.adr)).arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			}
			break;
		case BRK_MEMROM:
			if (brk.eadr > brk.adr) {
				res = QString("ROM:%0-%1 [%2:%3-%4:%5]").arg(gethex6(brk.adr)).arg(gethex6(brk.eadr)).arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff)).arg(gethexbyte(brk.eadr >> 14)).arg(gethexword(brk.eadr & 0x3fff));
			} else {
				res = QString("ROM:%0 [%1:%2]").arg(gethex6(brk.adr)).arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			}
			break;
		case BRK_MEMSLT:
			res = QString("SLT:%0 [%1:%2]").arg(gethex6(brk.adr)).arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			break;
		case BRK_MEMEXT:
			res = QString("EXT:%0 [%1:%2]").arg(gethex6(brk.adr)).arg(gethexbyte(brk.adr >> 14)).arg(gethexword(brk.adr & 0x3fff));
			break;
		case BRK_IRQ:
			res = QString("IRQ");
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
				case 1: if ((brk.type != BRK_IRQ) && (brk.type != BRK_IOPORT)) res = brk.fetch ? Qt::Checked : Qt::Unchecked; break;
				case 2: if (brk.type != BRK_IRQ) res = brk.read ? Qt::Checked : Qt::Unchecked; break;
				case 3: if (brk.type != BRK_IRQ) res = brk.write ? Qt::Checked : Qt::Unchecked; break;
			}
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 4: res = brkGetString(brk); break;
				case 5: res = brk.count; break;
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
						case 5: res = "Cnt"; break;
					}
					break;
			}
			break;
		case Qt::Vertical:
			break;
	}
	return res;
}

bool xbsOff(const xBrkPoint bpa, const xBrkPoint bpb) {return (bpa.off && !bpb.off);}
bool xbsFe(const xBrkPoint bpa, const xBrkPoint bpb) {return (bpa.fetch && !bpb.fetch);}
bool xbsRd(const xBrkPoint bpa, const xBrkPoint bpb) {return (bpa.read && !bpb.read);}
bool xbsWr(const xBrkPoint bpa, const xBrkPoint bpb) {return (bpa.write && !bpb.write);}
bool xbsCnt(const xBrkPoint bpa, const xBrkPoint bpb) {return (bpa.count < bpb.count);}
bool xbsName(const xBrkPoint bpa, const xBrkPoint bpb) {
	return brkGetString(bpa) < brkGetString(bpb);
}

void xBreakListModel::sort(int col, Qt::SortOrder ord) {
	if (!conf.prof.cur) return;
	switch(col) {
		case 0: std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsOff); break;
		case 1: std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsFe); break;
		case 2: std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsRd); break;
		case 3: std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsWr); break;
		case 4: std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsName); break;
		case 5: std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), xbsCnt); break;
	}
	emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
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
	setColumnWidth(4, 200);
	setColumnWidth(5, 40);
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
	// brkInstall(prf->brkList[row], 0);
	brkInstallAll();
	model->updateCell(row, col);
	emit rqDasmDump();
}

void xBreakTable::onDoubleClick(QModelIndex idx) {
	if (!idx.isValid()) return;
	int row = idx.row();
	xBrkPoint bp = conf.prof.cur->brkList[row];
	int adr = -1;
	switch(bp.type) {
		case BRK_CPUADR: adr = bp.adr; break;
		case BRK_MEMRAM: adr = memFindAdr(conf.prof.cur->zx->mem, MEM_RAM, bp.adr); break;
		case BRK_MEMROM: adr = memFindAdr(conf.prof.cur->zx->mem, MEM_ROM, bp.adr); break;
	}
	if (adr < 0) return;
	emit rqDisasm(adr);
}

// Dialog

xBrkManager::xBrkManager(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	ui.brkType->addItem("ADR bus (MEM)", BRK_CPUADR);
	ui.brkType->addItem("ADR bus (IO)", BRK_IOPORT);
	ui.brkType->addItem("RAM cell", BRK_MEMRAM);
	ui.brkType->addItem("ROM cell", BRK_MEMROM);
	ui.brkType->addItem("SLT cell", BRK_MEMSLT);
	ui.brkType->addItem("IRQ", BRK_IRQ);

	ui.brkAdrEnd->setMin(0x0000);
	ui.brkAdrEnd->setMax(0xffffff);
	ui.brkAdrHex->setMin(0x0000);
	ui.brkAdrHex->setMax(0xffffff);

	ui.leStartOffset->setMin(0);
	ui.leStartOffset->setMax(0x3fff);
	ui.leEndOffset->setMin(0);
	ui.leEndOffset->setMax(0x3fff);
	ui.leValue->setMin(0);
	ui.leValue->setMax(0xff);
	ui.leValMask->setMin(0);
	ui.leValMask->setMax(0xff);
	ui.leValMask->setValue(0xff);

	ui.brkAction->addItem("Debuger", BRK_ACT_DBG);
	ui.brkAction->addItem("Counter", BRK_ACT_COUNT);
	ui.brkAction->addItem("Screen dump (ZX only)", BRK_ACT_SCR);

	connect(ui.brkBank, SIGNAL(valueChanged(int)), this, SLOT(bnkChanged(int)));
	connect(ui.leStartOffset,SIGNAL(valueChanged(int)),this,SLOT(startOffChanged(int)));
	connect(ui.brkAdrHex,SIGNAL(valueChanged(int)),this,SLOT(startAbsChanged(int)));
	connect(ui.leEndOffset,SIGNAL(valueChanged(int)),this,SLOT(endOffChanged(int)));
	connect(ui.brkAdrEnd,SIGNAL(valueChanged(int)),this,SLOT(endAbsChanged(int)));

	connect(ui.brkType, SIGNAL(currentIndexChanged(int)), this, SLOT(chaType(int)));
//	connect(ui.brkAdrHex, SIGNAL(valueChanged(int)), ui.brkAdrEnd, SLOT(setMin(int)));
	connect(ui.pbOK, SIGNAL(clicked()), this, SLOT(confirm()));
}

void xBrkManager::bnkChanged(int v) {
//	ui.brkAdrHex->blockSignals(true);
//	ui.brkAdrEnd->blockSignals(true);
	ui.brkAdrHex->setValue((v << 14) | (ui.leStartOffset->getValue() & 0x3fff));
	ui.brkAdrEnd->setMin((v << 14) | (ui.leStartOffset->getValue() & 0x3fff));
	ui.brkAdrEnd->setValue((v << 14) | (ui.leEndOffset->getValue() & 0x3fff));
//	ui.brkAdrHex->blockSignals(false);
//	ui.brkAdrEnd->blockSignals(false);
}

void xBrkManager::startOffChanged(int v) {
//	ui.brkAdrHex->blockSignals(true);
	ui.leEndOffset->setMin(v);
	v = (ui.brkBank->value() << 14) | (v & 0x3fff);
	ui.brkAdrHex->setValue(v);
	ui.brkAdrEnd->setMin(v);
//	ui.brkAdrEnd->setValue(v);
//	ui.brkAdrHex->blockSignals(false);
}

void xBrkManager::startAbsChanged(int v) {
//	ui.brkBank->blockSignals(true);
//	ui.leStartOffset->blockSignals(true);
	ui.brkBank->setValue(v >> 14);
	ui.leStartOffset->setValue(v & 0x3fff);
//	ui.brkBank->blockSignals(false);
//	ui.leStartOffset->blockSignals(false);
	ui.brkAdrEnd->setMin(v);
	ui.leEndOffset->setMin(v & 0x3fff);
//	ui.brkAdrEnd->setValue(v);
}

void xBrkManager::endOffChanged(int v) {
//	ui.brkAdrEnd->blockSignals(true);
	ui.brkAdrEnd->setValue((ui.brkBank->value() << 14) | (v & 0x3fff));
//	ui.brkAdrEnd->blockSignals(false);
}

void xBrkManager::endAbsChanged(int v) {
//	ui.leEndOffset->blockSignals(true);
	ui.leEndOffset->setValue(v & 0x3fff);
//	ui.leEndOffset->blockSignals(true);
}

#define EL_VAL 512
#define EL_FE 256
#define EL_RD 128
#define EL_WR 64
#define EL_BNK 32
#define EL_SOF 16
#define EL_SAD 8
#define EL_EOF 4
#define EL_EAD 2
#define EL_MSK 1

void xBrkManager::setElements(int mask) {
	ui.brkFetch->setVisible(mask & EL_FE);
	ui.brkRead->setVisible(mask & EL_RD);
	ui.brkWrite->setVisible(mask & EL_WR);
	ui.labFlags->setVisible(mask & (EL_FE | EL_RD | EL_WR));
	ui.brkBank->setVisible(mask & EL_BNK);
	ui.labBank->setVisible(mask & EL_BNK);
	ui.leStartOffset->setVisible(mask & EL_SOF);
	ui.labStartOff->setVisible(mask & EL_SOF);
	ui.brkAdrHex->setVisible(mask & EL_SAD);
	ui.labStartAbs->setVisible(mask & EL_SAD);
	ui.leEndOffset->setVisible(mask & EL_EOF);
	ui.labEndOff->setVisible(mask & EL_EOF);
	ui.brkAdrEnd->setVisible(mask & EL_EAD);
	ui.labEndAbs->setVisible(mask & EL_EAD);
	ui.brkMaskHex->setVisible(mask & EL_MSK);
	ui.labMask->setVisible(mask & EL_MSK);

	ui.labValue->setVisible(mask & EL_VAL);
	ui.labValMask->setVisible(mask & EL_VAL);
	ui.leValue->setVisible(mask & EL_VAL);
	ui.leValMask->setVisible(mask & EL_VAL);
}

void xBrkManager::chaType(int i) {
	int t = ui.brkType->itemData(i).toInt();
	switch (t) {
		case BRK_IRQ:
			setElements(0);
			break;
		case BRK_IOPORT:
			setElements(EL_RD | EL_WR | EL_SAD | EL_MSK);
			break;
		case BRK_CPUADR:
			setElements(EL_FE | EL_RD | EL_WR | EL_SAD | EL_EAD);
			break;
		default:
			setElements(EL_FE | EL_RD | EL_WR | EL_BNK | EL_SOF | EL_SAD | EL_EOF | EL_EAD);
			break;
	}
}

void xBrkManager::edit(xBrkPoint* sbrk) {
	if (sbrk) {
		obrk = *sbrk;
		obrk.off = 0;
	} else {
		obrk.type = BRK_MEMRAM;
		obrk.adr = 0;
		obrk.mask = 0;
		obrk.eadr = 0;
		obrk.off = 0;
		obrk.fetch = 1;
		obrk.read = 0;
		obrk.write = 0;
		obrk.off = 0;
		obrk.count = 0;
		obrk.action = BRK_ACT_DBG;
	}
	ui.brkAction->setCurrentIndex(ui.brkAction->findData(obrk.action));
	ui.brkType->setCurrentIndex(ui.brkType->findData(obrk.type));
	ui.brkFetch->setChecked(obrk.fetch);
	ui.brkRead->setChecked(obrk.read);
	ui.brkWrite->setChecked(obrk.write);
	ui.brkAdrHex->setMax(conf.prof.cur->zx->mem->busmask);
	ui.brkAdrEnd->setMax(conf.prof.cur->zx->mem->busmask);
	switch(obrk.type) {
		case BRK_IOPORT:
			ui.brkBank->setValue(0);
			ui.brkAdrHex->setValue(obrk.adr);
			ui.brkMaskHex->setValue(obrk.mask);
			break;
		case BRK_CPUADR:
			ui.brkBank->setValue(0);
			ui.brkAdrHex->setValue(obrk.adr);
			ui.brkAdrEnd->setValue(obrk.eadr);
			ui.brkMaskHex->setText("FFFF");
			break;
		default:
			ui.brkBank->setValue(obrk.adr >> 14);
			ui.brkAdrHex->setValue(obrk.adr);	// &0x3fff ?
			ui.brkAdrEnd->setValue(obrk.eadr);
			ui.brkMaskHex->setText("FFFF");
			break;
	}
	chaType(ui.brkType->currentIndex());
	show();
}

void xBrkManager::confirm() {
	xBrkPoint brk;
	brk.off = 0;
	brk.type = ui.brkType->itemData(ui.brkType->currentIndex()).toInt();
	brk.fetch = ui.brkFetch->isChecked() ? 1 : 0;
	brk.read = ui.brkRead->isChecked() ? 1 : 0;
	brk.write = ui.brkWrite->isChecked() ? 1 : 0;
	brk.action = ui.brkAction->itemData(ui.brkAction->currentIndex()).toInt();
	brk.count = obrk.count;
	int bnk = ui.brkBank->value();
	switch (brk.type) {
		case BRK_CPUADR:
			brk.adr = ui.brkAdrHex->getValue();
			brk.eadr = ui.brkAdrEnd->getValue();
			break;
		case BRK_IOPORT:
			brk.adr = ui.brkAdrHex->getValue();
			brk.eadr = brk.adr;
			break;
		default:
			brk.adr = ui.brkAdrHex->getValue() | (bnk << 14);
			brk.eadr = ui.brkAdrEnd->getValue() | (bnk << 14);
			break;
	}
	brk.mask = ui.brkMaskHex->getValue();
	emit completed(obrk, brk);
	hide();
}

// widget

xBreakWidget::xBreakWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("BRKWIDGET");

	ui.bpList->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui.bpList->addAction(ui.brkActReset);

	brkManager = new xBrkManager(this);
	connect(brkManager, &xBrkManager::completed, this, &xBreakWidget::confirmBrk);

	connect(ui.tbAddBrk, &QToolButton::clicked, this, &xBreakWidget::addBrk);
	connect(ui.tbEditBrk, &QToolButton::clicked, this, &xBreakWidget::editBrk);
	connect(ui.tbDelBrk, &QToolButton::clicked, this, &xBreakWidget::delBrk);
	connect(ui.tbBrkOpen, &QToolButton::clicked, this, &xBreakWidget::openBrk);
	connect(ui.tbBrkSave, &QToolButton::clicked, this, &xBreakWidget::saveBrk);
	connect(ui.bpList, &xBreakTable::rqDisasm, this, &xBreakWidget::rqDisasm);
	connect(ui.bpList, &xBreakTable::rqDasmDump, this, &xBreakWidget::updated);
	connect(ui.brkActReset, &QAction::triggered, this, &xBreakWidget::resetBrk);
}

void xBreakWidget::draw() {
	ui.bpList->update();
}

void xBreakWidget::addBrk() {
	brkManager->edit(NULL);
}

void xBreakWidget::editBrk() {
	QModelIndexList idxl = ui.bpList->selectionModel()->selectedRows();
	if (idxl.size() < 1) return;
	int row = idxl.first().row();
	xBrkPoint* brk = &conf.prof.cur->brkList[row];
	brkManager->edit(brk);
}

void xBreakWidget::confirmBrk(xBrkPoint obrk, xBrkPoint brk) {
	brkDelete(obrk);
	brkAdd(brk);
	emit updated();
	ui.bpList->update();
}

bool qmidx_greater(const QModelIndex idx1, const QModelIndex idx2) {
	return (idx1.row() > idx2.row());
}

void xBreakWidget::delBrk() {
	QModelIndexList idxl = ui.bpList->selectionModel()->selectedRows();
	std::sort(idxl.begin(), idxl.end(), qmidx_greater);
	QModelIndex idx;
	xBrkPoint brk;
	foreach(idx, idxl) {
		brk = conf.prof.cur->brkList[idx.row()];
		brkDelete(brk);
	}
	ui.bpList->update();
	emit updated();		// fill disasm/dump
}

void xBreakWidget::resetBrk() {
	QModelIndexList idxl = ui.bpList->selectionModel()->selectedRows();
	QModelIndex idx;
	foreach(idx, idxl) {
		conf.prof.cur->brkList[idx.row()].count = 0;
	}
	ui.bpList->update();
	emit updated();		// fill disasm/dump
}

bool parseRange(QString str, xBrkPoint* p) {
	bool r;
	int badr;
	int eadr;
	int tadr;
	QStringList splt;
	splt = str.split('-', X_SkipEmptyParts);
	badr = splt.first().toInt(&r, 16);
	if (r) {
		if (splt.size() > 1) {
			eadr = splt[1].toInt(&r, 16);
			if (eadr < badr) {
				tadr = badr;
				badr = eadr;
				eadr = tadr;
			}
		} else {
			eadr = badr;
		}
		if (r) {
			p->adr = badr;
			p->eadr = eadr;
		}
	}
	return r;
}

void xBreakWidget::openBrk() {
	QString path = QFileDialog::getOpenFileName(this, "Open breakpoints list", "", "deBUGa breakpoints (*.xbrk)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QFile file(path);
	QString line;
	QStringList splt;
	QStringList list;
	xBrkPoint brk;
	int off;
	bool b0,b1;
	if (file.open(QFile::ReadOnly)) {
		conf.prof.cur->brkList.clear();
		while(!file.atEnd()) {
			line = tr(file.readLine());
			if (!line.startsWith(";")) {
				b0 = true;
				b1 = true;
				list = line.trimmed().split(":", X_KeepEmptyParts);
				while(list.size() < 5)
					list.append(QString());
				brk.fetch = list.at(3).contains("F") ? 1 : 0;
				brk.read = list.at(3).contains("R") ? 1 : 0;
				brk.write = list.at(3).contains("W") ? 1 : 0;
				brk.off = list.at(3).contains("0") ? 1 : 0;
				if (list.at(0) == "IO") {
					brk.type = BRK_IOPORT;
					brk.adr = list.at(1).toInt(&b0, 16) & 0xffff;
					brk.mask = list.at(2).toInt(&b1, 16) & 0xffff;
				} else if (list.at(0) == "CPU") {
					brk.type = BRK_CPUADR;
					b0 = parseRange(list.at(1), &brk);
					brk.adr &= 0xffff;
					brk.eadr &= 0xffff;
//					if (list.at(1).contains("-")) {		// 1234-ABCD
//						splt = list.at(1).split(QLatin1Char('-'), X_SkipEmptyParts);
//						list[1] = splt.first();
//						list[2] = splt.last();
//					}
//					brk.adr = list.at(1).toInt(&b0, 16) & 0xffff;
//					if (list.at(2).isEmpty()) {
//						brk.eadr = brk.adr;
//					} else {
//						brk.eadr = list.at(2).toInt(&b1, 16) & 0xffff;
//						if (brk.eadr < brk.adr)
//							brk.eadr = brk.adr;
//					}
				} else if (list.at(0) == "ROM") {
					brk.type = BRK_MEMROM;
					b0 = parseRange(list.at(2), &brk);
					off = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
					brk.adr = (brk.adr & 0x3fff) | off;
					brk.eadr = (brk.eadr & 0x3fff) | off;
//					brk.adr = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
//					brk.adr |= (list.at(2).toInt(&b1, 16) & 0x3fff);
//					brk.eadr = brk.adr;
				} else if (list.at(0) == "RAM") {
					brk.type = BRK_MEMRAM;
					b0 = parseRange(list.at(2), &brk);
					off = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
					brk.adr = (brk.adr & 0x3fff) | off;
					brk.eadr = (brk.eadr & 0x3fff) | off;
//					brk.adr = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
//					brk.adr |= (list.at(2).toInt(&b1, 16) & 0x3fff);
//					brk.eadr = brk.adr;
				} else if (list.at(0) == "SLT") {
					brk.type = BRK_MEMSLT;
					b0 = parseRange(list.at(2), &brk);
					off = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
					brk.adr = (brk.adr & 0x3fff) | off;
					brk.eadr = (brk.eadr & 0x3fff) | off;
//					brk.adr = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
//					brk.adr |= (list.at(2).toInt(&b1, 16) & 0x3fff);
//					brk.eadr = brk.adr;
				} else if (list.at(0) == "IRQ") {
					brk.type = BRK_IRQ;
				} else {
					b0 = false;
				}
				if (list.at(4) == "SCR") {
					brk.action = BRK_ACT_SCR;
				} else if (list.at(4) == "CNT") {
					brk.action = BRK_ACT_COUNT;
				} else {
					brk.action = BRK_ACT_DBG;
				}
				if (b0 && b1) {
					brk.count = 0;
					conf.prof.cur->brkList.push_back(brk);
				}
			}
		}
		file.close();
		brkInstallAll();
		ui.bpList->update();
		emit updated();
	} else {
		shitHappens("Can't open file for reading");
	}
}

void xBreakWidget::saveBrk() {
	QString path = QFileDialog::getSaveFileName(this, "Save breakpoints", "", "deBUGa breakpoints (*.xbrk)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty())
		return;
	if (!path.endsWith(".xbrk", Qt::CaseInsensitive))
		path.append(".xbrk");
	xBrkPoint brk;
	QFile file(path);
	QString nm,ar1,ar2,flag,act;
	if (file.open(QFile::WriteOnly)) {
		file.write("; Xpeccy deBUGa breakpoints list\n");
		foreach(brk, conf.prof.cur->brkList) {
			switch(brk.type) {
				case BRK_IOPORT:
					nm = "IO";
					ar1 = gethexword(brk.adr & 0xffff);
					ar2 = gethexword(brk.mask & 0xffff);
					break;
				case BRK_CPUADR:
					nm = "CPU";
					ar1 = gethexword(brk.adr & 0xffff);
					ar2.clear();
					if (brk.eadr > brk.adr) {
						ar1.append("-");
						ar1.append(gethexword(brk.eadr & 0xffff));
					}
					break;
				case BRK_MEMRAM:
					nm = "RAM";
					ar1 = gethexbyte((brk.adr >> 14) & 0xff);	// 16K page
					ar2 = gethexword(brk.adr & 0x3fff);		// adr in page
					if (brk.eadr > brk.adr) {
						ar2.append("-");
						ar2.append(gethexword(brk.eadr & 0x3fff));
					}
					break;
				case BRK_MEMROM:
					nm = "ROM";
					ar1 = gethexbyte((brk.adr >> 14) & 0xff);
					ar2 = gethexword(brk.adr & 0x3fff);
					if (brk.eadr > brk.adr) {
						ar2.append("-");
						ar2.append(gethexword(brk.eadr & 0x3fff));
					}
					break;
				case BRK_MEMSLT:
					nm = "SLT";
					ar1 = gethexbyte((brk.adr >> 14) & 0xff);
					ar2 = gethexword(brk.adr & 0x3fff);
					if (brk.eadr > brk.adr) {
						ar2.append("-");
						ar2.append(gethexword(brk.eadr & 0x3fff));
					}
					break;
				case BRK_IRQ:
					nm = "IRQ";
					ar1.clear();
					ar2.clear();
					break;
				default:
					nm.clear();
					break;
			}
			switch(brk.action) {
				case BRK_ACT_DBG: act = "DBG"; break;
				case BRK_ACT_SCR: act = "SCR"; break;
				case BRK_ACT_COUNT: act = "CNT"; break;
				default: act.clear(); break;
			}
			if (!nm.isEmpty()) {
				flag.clear();
				if (brk.fetch) flag.append("F");
				if (brk.read) flag.append("R");
				if (brk.write) flag.append("W");
				if (brk.off) flag.append("0");
				file.write(QString("%0:%1:%2:%3:%4\n").arg(nm).arg(ar1).arg(ar2).arg(flag).arg(act).toUtf8());
			}
		}
		file.close();
	} else {
		shitHappens("Can't open file for writing");
	}
}
