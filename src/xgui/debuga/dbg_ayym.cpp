#include "dbg_widgets.h"

#include <QPainter>

xAYWidget::xAYWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	hwList << HWG_ZX << HWG_MSX << HWG_ALF;
	setObjectName("AYWIDGET");

	ui.cbChipSelect->addItem("Sound chip A", 0);
	ui.cbChipSelect->addItem("Sound chip B", 1);
	ui.cbChipSelect->addItem("Sound chip C", 2);
	// ui.cbChipSelect->addItem("Sound chip D", 3);

	connect(ui.sbChanNum, SIGNAL(valueChanged(int)), this, SLOT(draw()));
	connect(ui.sbOpNum, SIGNAL(valueChanged(int)), this, SLOT(draw()));
	connect(ui.fmChanOff, SIGNAL(stateChanged(int)), this, SLOT(offChan(int)));
	connect(ui.cbChipSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(draw()));
}

void xAYWidget::offChan(int st) {
	Computer* comp = conf.prof.cur->zx;
	aymChip* chp = comp->ts->chipA;
	int chn = ui.sbChanNum->value();
	chp->chanFM[chn].off = (st == Qt::Checked);
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
	QPixmap pxm;
	if (dir) {
		pxm = QPixmap(100, 10); // lab->height() / 2);
	} else {
		pxm = QPixmap(10, 100); // lab->height() / 2);
	}
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
	aymChip* chp;
	switch (ui.cbChipSelect->currentIndex()) {
		case 1: chp = comp->ts->chipB; break;
		case 2: chp = comp->ts->chipC; break;
		case 3: chp = comp->ts->chipD; break;
		default: chp = comp->ts->chipA; break;
	}
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
	ui.fmChanOff->setChecked(ch->off);
	ui.leFmChanFrq->setText(gethexword(op->pg.freq));
	ui.leFmChanBase->setText(gethexbyte(op->pg.block));
	ui.leFmChanStep->setText(gethexint(op->pg.pstep));
	ui.leFmChanAlg->setText(gethexbyte(ch->algo));
	ui.leFmChanOut->setText(QString::number(ch->out));
	ui.leFmOpStatus->setText(getOpStatusName(op->eg.state));
	ui.leFmOpAR->setText(gethexbyte(op->eg.atkrate));
	ui.leFmOpDR->setText(gethexbyte(op->eg.decrate));
	ui.leFmOpSR->setText(gethexbyte(op->eg.susrate));
	ui.leFmOpSL->setText(gethexword(op->eg.suslev));
	ui.leFmOpRR->setText(gethexbyte(op->eg.relrate));
	ui.leFmOpTL->setText(gethexword(op->tlev));
	ui.leFmOpKS->setText(gethexbyte(op->eg.ks));
	ui.leFmOpEGAmp->setText(gethexword(op->eg.att));
	ui.leFmOpPhase->setText(gethexint(op->pg.phase).right(5));
	ui.leFmOpOut->setText(QString::number(op->out));	// signed

	drawHBar(ui.labBeep, comp->beep->val, 256);
}
