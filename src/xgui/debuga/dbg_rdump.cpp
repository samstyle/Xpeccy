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
}

void xRDumpWidget::draw() {
	ui.tabRegDump->update();
}

// table

xRDumpTable::xRDumpTable(QWidget* p):QTableView(p) {
	model = new xRDumpModel;
	setModel(model);
	setColumnWidth(0, 70);
}

void xRDumpTable::update() {
	model->refill();
}

void xRDumpTable::resizeEvent(QResizeEvent* e) {
	int w = width();
	int w0 = horizontalHeader()->sectionSize(0);
	if (w >= w0) {
		int wd = horizontalHeader()->defaultSectionSize();
		int cnt = (w - w0) / wd;
		model->setCols(cnt + 1);
	}
	QTableView::resizeEvent(e);
}

// model

xRDumpModel::xRDumpModel(QObject* p):xTableModel(p) {
	row_count = 1;
	col_count = 9;
}

void xRDumpModel::refill() {
	xRegBunch rz = cpuGetRegs(conf.prof.cur->zx->cpu);
	regs.clear();
	int i = 0;
	while (rz.regs[i].id != REG_EOT) {
		if (rz.regs[i].flag & REG_RDMP) {
			regs.append(rz.regs[i]);
		}
		i++;
	}
	setRows(regs.size());
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
					case MEM_SLOT: res = gethexbyte(memRd(mem, adr)); break;
					default: res = "--";
						break;
				}
			}
			break;
	}
	return res;
}
