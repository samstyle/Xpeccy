#include "dbg_widgets.h"

#include <QPainter>

xAYWidget::xAYWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	hwList << HWG_ZX << HWG_MSX << HWG_ALF;
	setObjectName("AYWIDGET");

	connect(ui.sbChanNum, SIGNAL(valueChanged(int)), this, SLOT(draw()));
	connect(ui.sbOpNum, SIGNAL(valueChanged(int)), this, SLOT(draw()));
}

QString getAYmix(aymChan* ch) {
	QString res = ch->tdis ? "-" : "T";
	res += ch->ndis ? "-" : "N";
	res += ch->een ? "E" : "-";
	return res;
}

// dir = 1/0 = horiz/vert
void drawBar(QLabel* lab, int lev, int max, int dir) {
	if (lev > max) lev = max;
	if (lev < 0) lev = 0;
	QPixmap pxm(100, lab->height() / 2);
	QPainter pnt;
	pxm.fill(Qt::black);
	pnt.begin(&pxm);
	if (dir) {
		pnt.fillRect(0, 0, pxm.width() * lev / max, pxm.height(), Qt::green);
		pnt.setPen(Qt::red);
		pnt.drawLine(pxm.width() / 2, 0, pxm.width() / 2, pxm.height());
	} else {
		pnt.fillRect(0, pxm.height() * lev / max, pxm.width(), pxm.height(), Qt::green);
		pnt.setPen(Qt::red);
		pnt.drawLine(0, pxm.height() / 2, pxm.width(), pxm.height() / 2);
	}
	pnt.end();
	lab->setPixmap(pxm);
}

void drawHBar(QLabel* lab, int lev, int max) {drawBar(lab, lev, max, 1);}
void drawVBar(QLabel* lab, int lev, int max) {drawBar(lab, lev, max, 0);}

struct {
	int id;
	QString str;
} stNameTab[] = {
	{OPST_OFF, "OFF"},
	{OPST_ATK, "ATK"},
	{OPST_DEC, "DEC"},
	{OPST_SUS, "SUS"},
	{OPST_REL, "REL"},
	{-1, "?"}
};

QString getOpStatusName(int id) {
	int i = 0;
	while ((stNameTab[i].id != -1) && (stNameTab[i].id != id))
		i++;
	return stNameTab[i].str;
}

void xAYWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	tsGetVolume(comp->ts);		// to update FM output value
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
	// fm
	int chn = ui.sbChanNum->value() & 3;
	int opn = ui.sbOpNum->value() & 3;
	fmChan* ch = &chp->chanFM[chn];
	fmOper* op = &ch->op[opn];
	ui.leFmChanFrq->setText(gethexword(ch->freq));
	ui.leFmChanBase->setText(gethexbyte(ch->block));
	ui.leFmChanStep->setText(gethexint(ch->freq << ch->block));
	ui.leFmChanAlg->setText(gethexbyte(ch->algo));
	ui.leFmChanOut->setText(QString::number(ch->out));
	ui.leFmOpStatus->setText(getOpStatusName(op->state));
	ui.leFmOpAR->setText(gethexbyte(op->atkrate));
	ui.leFmOpDR->setText(gethexbyte(op->decrate));
	ui.leFmOpSR->setText(gethexbyte(op->susrate));
	ui.leFmOpSL->setText(gethexword(op->suslev));
	ui.leFmOpRR->setText(gethexbyte(op->relrate));
	ui.leFmOpTL->setText(gethexword(op->tlev));
	ui.leFmOpKS->setText(gethexbyte(op->ks));
	ui.leFmOpEGAmp->setText(gethexword(op->amp));
	ui.leFmOpPhase->setText(gethexint(op->phase));
	ui.leFmOpOut->setText(QString::number(op->out));	// signed

	drawHBar(ui.labBeep, comp->beep->val, 256);
}
