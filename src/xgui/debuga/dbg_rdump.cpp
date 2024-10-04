#include "dbg_rdump.h"

// widget

xRDumpWidget::xRDumpWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("REG DUMP");
	model = new xRDumpModel;
	ui.tabRegDump->setModel(model);
	connect(this, &QDockWidget::visibilityChanged, this, &xRDumpWidget::draw);
	hwList << HWG_ZX << HWG_GB << HWG_MSX << HWG_SPCLST;

	ui.tabRegDump->setColumnWidth(0, 70);
}

void xRDumpWidget::draw() {
	model->refill();
//	ui.tabRegDump->update();
}

// model

xRDumpModel::xRDumpModel(QObject* p):xTableModel(p) {
}

int xRDumpModel::columnCount(const QModelIndex&) const {
	return 12;
}

int xRDumpModel::rowCount(const QModelIndex&) const {
	return regs.size();
}

void xRDumpModel::refill() {
	xRegBunch rz = cpuGetRegs(conf.prof.cur->zx->cpu);
	regs.clear();
	for (int i = 0; (i < 32) && (rz.regs[i].id != REG_NONE); i++) {
		if (rz.regs[i].type & REG_RDMP) {
			regs.append(rz.regs[i]);
		}
	}
	update();
}

QVariant xRDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row >= rowCount()) return res;
	if (col >= columnCount()) return res;
	Memory* mem = conf.prof.cur->zx->mem;
	MemPage* pg;
	int adr;
	int fadr;
	switch(role) {
		case Qt::DisplayRole:
			if (col == 0) {
				res = QString(regs.at(row).name);
			} else {
				adr = regs.at(row).value + col - 1;
				pg = mem_get_page(mem, adr);
				fadr = mem_get_phys_adr(mem, adr);	// = pg->num << 8) | (adr & 0xff);
				switch (pg->type) {
					case MEM_ROM: res = gethexbyte(mem->romData[fadr & mem->romMask]); break;
					case MEM_RAM: res = gethexbyte(mem->ramData[fadr & mem->ramMask]); break;
					case MEM_SLOT: res = gethexbyte(memRd(mem, adr));
					default: res = "--";
						break;
				}
			}
			break;
	}
	return res;
}
