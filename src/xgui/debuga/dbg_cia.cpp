#include "dbg_widgets.h"

xCiaWidget::xCiaWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("CIAWIDGET");
	hwList << HWG_COMMODORE;
}

void xCiaWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	ui.cia1timera->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia1->timerA.value)).arg(gethexword(comp->c64.cia1->timerA.inival)));
	ui.cia1timerb->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia1->timerB.value)).arg(gethexword(comp->c64.cia1->timerB.inival)));
	ui.cia1irq->setText(getbinbyte(comp->c64.cia1->intrq));
	ui.cia1inten->setText(getbinbyte(comp->c64.cia1->inten));
	ui.cia1cra->setText(getbinbyte(comp->c64.cia1->timerA.flags));
	ui.cia1crb->setText(getbinbyte(comp->c64.cia1->timerB.flags));
	ui.cia2timera->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia2->timerA.value)).arg(gethexword(comp->c64.cia2->timerA.inival)));
	ui.cia2timerb->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia2->timerB.value)).arg(gethexword(comp->c64.cia2->timerB.inival)));
	ui.cia2irq->setText(getbinbyte(comp->c64.cia2->intrq));
	ui.cia2inten->setText(getbinbyte(comp->c64.cia2->inten));
	ui.cia2cra->setText(getbinbyte(comp->c64.cia2->timerA.flags));
	ui.cia2crb->setText(getbinbyte(comp->c64.cia2->timerB.flags));
}
