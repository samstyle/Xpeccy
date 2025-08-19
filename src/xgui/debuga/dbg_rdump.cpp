#include "dbg_rdump.h"

// widget

xRDumpWidget::xRDumpWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("REG DUMP");
	connect(this, &QDockWidget::visibilityChanged, this, &xRDumpWidget::draw);
	hwList << HWG_ZX << HWG_GB << HWG_MSX << HWG_SPCLST;

	ui.tabRegDump->setColumnWidth(0, 70);
}

void xRDumpWidget::draw() {
	ui.tabRegDump->update();
}

// table

xRDumpTable::xRDumpTable(QWidget* p):QTableView(p) {
	model = new xRDumpModel;
	setModel(model);
}

void xRDumpTable::update() {
	model->refill();
}

void xRDumpTable::resizeEvent(QResizeEvent* e) {
// FIXME: columns added by 2
//	int w = e->size().width();
//	if (w < 80) return;
//	int wd = horizontalHeader()->defaultSectionSize();
//	int cnt = (w - 80) / wd;
//	model->setCols(cnt + 1);
//	setColumnWidth(0, 70);
//	model->refill();
}

// model

xRDumpModel::xRDumpModel(QObject* p):xTableModel(p) {
	row_count = 0;
	col_count = 12;
}

int xRDumpModel::columnCount(const QModelIndex&) const {
	return col_count;
}

int xRDumpModel::rowCount(const QModelIndex&) const {
	return row_count;
}

void xRDumpModel::refill() {
	xRegBunch rz = cpuGetRegs(conf.prof.cur->zx->cpu);
	regs.clear();
	for (int i = 0; (i < 32) && (rz.regs[i].id != REG_EOT); i++) {
		if (rz.regs[i].flag & REG_RDMP) {
			regs.append(rz.regs[i]);
		}
	}
	setRows(regs.size());
	update();
}

QVariant xRDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row >= row_count) return res;
	if (col >= col_count) return res;
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
