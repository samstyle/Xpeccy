#include "dbg_widgets.h"

#include <QPainter>

xTapeWidget::xTapeWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("TAPEWIDGET");
	hwList << HWG_ZX << HWG_BK << HWG_COMMODORE << HWG_MSX << HWG_SPCLST;
}

#define XTDSTEP 20	// mks/dot

void xTapeWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	Tape* tape = comp->tape;
	drawHBar(ui.labTapein, tape->volPlay, 256);
	drawHBar(ui.labTapeout, tape->levRec, 1);
	ui.labSigLen->setText(tape->on ? QString("%0 mks").arg(tape->sigLen) : "");
	ui.labTapeState->setText(tape->on ? (tape->rec ? "rec" : "play") : "stop");
	ui.labTapePos->setText(tape->on ? QString::number(tape->pos - 1) : "--");
	// draw tape diagram
	int wid = 330;
	int hei = 100;
	int pos;
	int bnr;
	int amp;
	int oamp = -1;
	int x = wid / 2;
	int time = 0;
	TapeBlock* blk;
	QPixmap pxm(wid, hei);
	QPainter pnt;
	pxm.fill(Qt::black);
	pnt.begin(&pxm);
	pnt.setPen(Qt::red);
	pnt.drawLine(0, hei/2, wid, hei/2);
	pnt.drawLine(wid / 2, 0, wid/2, hei);
	pnt.setPen(Qt::green);
	if (tape->blkCount > 0) {
		bnr = tape->block;
		blk = &tape->blkData[bnr];
		pos = tape->pos;
		time = tape->sigLen + (wid / 2) * XTDSTEP;
		while ((time >= 0) && (blk != NULL)) {
			pos--;
			if (pos < 0) {
				bnr--;
				if (bnr < 0) {
					blk = NULL;
				} else {
					blk = &tape->blkData[bnr];
					pos = blk->sigCount - 1;
				}
			} else {
				time -= blk->data[pos].size;
			}
		}
		if (blk == NULL) {
			bnr = 0;
			pos = 0;
			blk = &tape->blkData[bnr];
			x = time / XTDSTEP;		// skip
			time = blk->data[pos].size;
		} else {
			x = 0;
			time += blk->data[pos].size;	// remaining time
		}
		while (x < wid) {
			if (pos < blk->sigCount) {
				amp = hei - blk->data[pos].vol * hei / 256;
				if (oamp < 0)
					oamp = amp;
				while ((time > 0) && (x < wid)) {
					pnt.drawLine(x, oamp, x + 1, amp);
					oamp = amp;
					time -= XTDSTEP;
					x++;
				}
				pos++;
				if (pos < blk->sigCount)
					time += blk->data[pos].size;
			} else {
				bnr++;
				if (bnr < tape->blkCount) {
					blk = &tape->blkData[bnr];
					pos = 0;
					time += blk->data[pos].size;
				} else {
					x = wid;
				}
			}
		}
	}
	pnt.end();
	ui.labTapeDiag->setPixmap(pxm);
}
