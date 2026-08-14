#include "dbg_palette.h"

#include <QPainter>

xPalWidget::xPalWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("PAL");
	hwList << HWG_ZX;
}

void xPalWidget::draw() {
	xProfile* prf = conf.prof.cur;
	if (prf) {
		Computer* comp = prf->zx;
		if (comp) {
			Video* vid = comp->vid;
			if (vid) {
				QPixmap pxm(256,256);
				pxm.fill(QColor(8,8,8));
				QColor col;
				int idx;
				int coli;
				QPainter pnt;
				pnt.begin(&pxm);
				for (idx = 0; idx < 256; idx++) {
					coli = vid->pal[idx];
					col.setRed(coli & 0xff);
					col.setGreen((coli >> 8) & 0xff);
					col.setBlue((coli >> 16) & 0xff);
					pnt.fillRect((idx & 0x0f) << 4, idx & 0xf0, 15, 15, col);
				}
				pnt.end();
				ui.labPalette->setPixmap(pxm);
				widget()->setEnabled(true);
			} else {
				widget()->setEnabled(false);
			}
		} else {
			widget()->setEnabled(false);
		}
	} else {
		widget()->setEnabled(false);
	}
}

void xPalWidget::mousePressEvent(QMouseEvent* ev) {
	if (!widget()->isEnabled()) return;
	QPoint pos = ui.labPalette->mapFrom(this, ev->pos());
	int x = pos.x();
	int y = pos.y();
	if ((x < 0) || (x > 255)) return;
	if ((y < 0) || (y > 255)) return;
	int idx = ((x >> 4) & 0x0f) | (y & 0xf0);
	uint32_t coli = conf.prof.cur->zx->vid->pal[idx];
	unsigned char r = coli & 0xff;
	unsigned char g = (coli >> 8) & 0xff;
	unsigned char b = (coli >> 16) & 0xff;
	ui.leIndex->setText(QString::number(idx));
	ui.leRed->setText(QString::number(r));
	ui.leGreen->setText(QString::number(g));
	ui.leBlue->setText(QString::number(b));
}
