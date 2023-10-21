#include <QPainter>

#include "dbg_widgets.h"

xZXScrWidget::xZXScrWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("ZXSCRWIDGET");

	scrImg = QImage(256, 192, QImage::Format_RGB888);
	connect(ui.sbScrBank, SIGNAL(valueChanged(int)), this, SLOT(draw()));
	connect(ui.leScrAdr, &xHexSpin::textChanged, this, &xZXScrWidget::draw);
	connect(ui.cbScrAtr, &QCheckBox::stateChanged, this, &xZXScrWidget::draw);
	connect(ui.cbScrPix, &QCheckBox::stateChanged, this, &xZXScrWidget::draw);
	connect(ui.cbScrGrid, &QCheckBox::stateChanged, this, &xZXScrWidget::draw);

	hwList << HWG_ZX;
}

void xZXScrWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	int flag = ui.cbScrAtr->isChecked() ? 1 : 0;
	flag |= ui.cbScrPix->isChecked() ? 2 : 0;
	flag |= ui.cbScrGrid->isChecked() ? 4 : 0;
	vid_get_screen(comp->vid, scrImg.bits(), ui.sbScrBank->value(), ui.leScrAdr->getValue(), flag);
	xColor bcol;// = comp->vid->pal[comp->vid->nextbrd];
	bcol.b = (comp->vid->nextbrd & 1) ? ((comp->vid->nextbrd & 8) ? 0xff : 0xa0) : 0x00;
	bcol.r = (comp->vid->nextbrd & 2) ? ((comp->vid->nextbrd & 8) ? 0xff : 0xa0) : 0x00;
	bcol.g = (comp->vid->nextbrd & 4) ? ((comp->vid->nextbrd & 8) ? 0xff : 0xa0) : 0x00;
	QPainter pnt;
	QPixmap xpxm(276, 212);
	pnt.begin(&xpxm);
	pnt.fillRect(0,0,276,212,qRgb(bcol.r, bcol.g, bcol.b));
	pnt.drawImage(10,10,scrImg);
	pnt.end();
	ui.scrLabel->setPixmap(xpxm);
	ui.labCurScr->setText(QString::number(comp->vid->curscr, 16).rightJustified(2, '0'));
}

void xZXScrWidget::setAddress(int adr, int atr) {
	ui.leScr->setValue(adr);
	ui.leAtr->setValue(atr);
}
