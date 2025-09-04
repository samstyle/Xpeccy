#include "dbg_widgets.h"

#include <QPainter>

xAYWidget::xAYWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	hwList << HWG_ZX << HWG_MSX << HWG_ALF;
	setObjectName("AYWIDGET");
}

QString getAYmix(aymChan* ch) {
	QString res = ch->tdis ? "-" : "T";
	res += ch->ndis ? "-" : "N";
	res += ch->een ? "E" : "-";
	return res;
}

void drawBar(QLabel* lab, int lev, int max) {
	if (lev > max) lev = max;
	if (lev < 0) lev = 0;
	QPixmap pxm(100, lab->height() / 2);
	QPainter pnt;
	pxm.fill(Qt::black);
	pnt.begin(&pxm);
	pnt.fillRect(0,0,pxm.width() * lev / max, pxm.height(), Qt::green);
	pnt.setPen(Qt::red);
	pnt.drawLine(pxm.width() / 2, 0, pxm.width() / 2, pxm.height());
	pnt.end();
	lab->setPixmap(pxm);
}

void xAYWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	aymChip* chp = comp->ts->chipA;
	ui.leToneA->setText(gethexword(((chp->reg[1] << 8) | chp->reg[0]) & 0x0fff));
	ui.leToneB->setText(gethexword(((chp->reg[3] << 8) | chp->reg[2]) & 0x0fff));
	ui.leToneC->setText(gethexword(((chp->reg[5] << 8) | chp->reg[4]) & 0x0fff));
	ui.leVolA->setText(gethexbyte(chp->reg[8] & 0x0f));
	ui.leVolB->setText(gethexbyte(chp->reg[9] & 0x0f));
	ui.leVolC->setText(gethexbyte(chp->reg[10] & 0x0f));
	ui.leMixA->setText(getAYmix(&chp->chanA));
	ui.leMixB->setText(getAYmix(&chp->chanB));
	ui.leMixC->setText(getAYmix(&chp->chanC));
	ui.leToneN->setText(gethexbyte(chp->reg[6]));
	ui.leEnvTone->setText(gethexword((chp->reg[12] << 8) | chp->reg[11]));
	ui.leEnvForm->setText(gethexbyte(chp->reg[13]));
	ui.leVolE->setText(gethexbyte(chp->chanE.vol));
	ui.labLevA->setText(chp->chanA.lev ? "1" : "0");
	ui.labLevB->setText(chp->chanB.lev ? "1" : "0");
	ui.labLevC->setText(chp->chanC.lev ? "1" : "0");
	ui.labLevN->setText(chp->chanN.lev ? "1" : "0");

	drawBar(ui.labBeep, comp->beep->val, 256);
}
