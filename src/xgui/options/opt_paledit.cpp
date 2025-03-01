#include <QPainter>
#include <QColorDialog>

#include "opt_paledit.h"

xPalEditor::xPalEditor(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	connect(ui.pbCancel, SIGNAL(released()), this, SLOT(close()));
	connect(ui.pbOk, SIGNAL(released()), this, SLOT(apply()));
}

void xPalEditor::edit(QList<QColor>* ptr) {
	ui.labPalette->setPal(ptr);
	show();
}

void xPalEditor::apply() {
	close();
	emit ready();
}

// ...

xPalGrid::xPalGrid(QWidget* p):QWidget(p) {
	setFixedSize(256, 256);
	pal = nullptr;
}

void xPalGrid::setPal(QList<QColor>* p) {
	pal = p;
}

void xPalGrid::paintEvent(QPaintEvent*) {
	QPainter pnt;
	pnt.begin(this);
	if (pal == nullptr) {
		pnt.fillRect(0, 0, width(), height(), Qt::black);
	} else {
		for (int i = 0; i < 16; i++) {
			pnt.fillRect((i & 3) << 6, (i & 0x0c) << 4, 64, 64, pal->at(i));
		}
	}
	pnt.end();
}

void xPalGrid::mousePressEvent(QMouseEvent* ev) {
	int x = ev->pos().x();
	int y = ev->pos().y();
	int i = ((y & 0xc0) >> 4) | ((x & 0xc0) >> 6);
	QColor col = QColorDialog::getColor(pal->at(i));
	if (col.isValid()) {
		*(pal->begin() + i) = col;
		repaint();
	}
}
