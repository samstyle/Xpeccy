#include "dbg_widgets.h"

#include <QPainter>

xMMapWidget::xMMapWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("MEMMAPWIDGET");

	ui.cbBank0->addItem("ROM", MEM_ROM);
	ui.cbBank0->addItem("RAM", MEM_RAM);
	ui.cbBank1->addItem("ROM", MEM_ROM);
	ui.cbBank1->addItem("RAM", MEM_RAM);
	ui.cbBank2->addItem("ROM", MEM_ROM);
	ui.cbBank2->addItem("RAM", MEM_RAM);
	ui.cbBank3->addItem("ROM", MEM_ROM);
	ui.cbBank3->addItem("RAM", MEM_RAM);
	ui.numBank0->setMax(255);
	ui.numBank1->setMax(255);
	ui.numBank2->setMax(255);
	ui.numBank3->setMax(255);
	connect(ui.cbBank0, SIGNAL(currentIndexChanged(int)), this, SLOT(remap_b0()));
	connect(ui.cbBank1, SIGNAL(currentIndexChanged(int)), this, SLOT(remap_b1()));
	connect(ui.cbBank2, SIGNAL(currentIndexChanged(int)), this, SLOT(remap_b2()));
	connect(ui.cbBank3, SIGNAL(currentIndexChanged(int)), this, SLOT(remap_b3()));
	connect(ui.numBank0, &xHexSpin::valueChanged, this, &xMMapWidget::remap_b0);
	connect(ui.numBank1, &xHexSpin::valueChanged, this, &xMMapWidget::remap_b1);
	connect(ui.numBank2, &xHexSpin::valueChanged, this, &xMMapWidget::remap_b2);
	connect(ui.numBank3, &xHexSpin::valueChanged, this, &xMMapWidget::remap_b3);
	connect(ui.pbRestMemMap, &QPushButton::clicked, this, &xMMapWidget::s_restore);
}

void xMMapWidget::remap_b0() {emit s_remap(0x00, getRFIData(ui.cbBank0), ui.numBank0->getValue());}
void xMMapWidget::remap_b1() {emit s_remap(0x40, getRFIData(ui.cbBank1), ui.numBank1->getValue());}
void xMMapWidget::remap_b2() {emit s_remap(0x80, getRFIData(ui.cbBank2), ui.numBank2->getValue());}
void xMMapWidget::remap_b3() {emit s_remap(0xc0, getRFIData(ui.cbBank3), ui.numBank3->getValue());}

void xMMapWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	ui.widBank->setVisible(comp->hw->grp == HWG_ZX);
	QPixmap img(256, 192);
	QPainter pnt;
	img.fill(Qt::black);
	pnt.begin(&img);
	int pg = 0;
	int x,y;
	QColor col;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			switch(comp->mem->map[pg & 0xff].type) {
				case MEM_RAM: col = Qt::darkGreen; break;
				case MEM_ROM: col = Qt::darkRed; break;
				case MEM_IO: col = Qt::darkBlue; break;
				case MEM_SLOT: col = Qt::darkCyan; break;
				default: col = Qt::darkGray; break;
			}
			pnt.fillRect(x * 12, y * 12, 11, 11, col);
			pg++;
		}
	}
	pnt.setPen(Qt::yellow);
	pnt.drawLine(0, 47, 256, 47);
	pnt.drawLine(0, 95, 256, 95);
	pnt.drawLine(0, 143, 256, 143);
	pnt.end();
	ui.labMemMap->setPixmap(img);
	blockSignals(true);
	if (comp->hw->grp == HWG_ZX) {
		ui.numBank0->setValue(comp->mem->map[0x00].num >> 6);
		ui.numBank1->setValue(comp->mem->map[0x40].num >> 6);
		ui.numBank2->setValue(comp->mem->map[0x80].num >> 6);
		ui.numBank3->setValue(comp->mem->map[0xc0].num >> 6);
		setRFIndex(ui.cbBank0, comp->mem->map[0x00].type);
		setRFIndex(ui.cbBank1, comp->mem->map[0x40].type);
		setRFIndex(ui.cbBank2, comp->mem->map[0x80].type);
		setRFIndex(ui.cbBank3, comp->mem->map[0xc0].type);
	}
	blockSignals(false);
}
