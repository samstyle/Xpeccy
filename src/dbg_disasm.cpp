#include "dbg_disasm.h"

extern unsigned short blockStart;
extern unsigned short blockEnd;

xTableWidget::xTableWidget(QWidget* par):QTableWidget(par) {
	setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
	// verticalHeader()->setStretchLastSection(true);
	blockStart = -1;
	blockEnd = -1;
}

void xTableWidget::keyPressEvent(QKeyEvent *ev) {
	if (ev->modifiers() & Qt::ControlModifier) {
		ev->ignore();
	} else {
		switch (ev->key()) {
			case Qt::Key_Home:
			case Qt::Key_End:
			case Qt::Key_F2:
				ev->ignore();
				break;
			default:
				QTableWidget::keyPressEvent(ev);
		}
	}
}

void xTableWidget::mousePressEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	if ((row < 0) || (row >= rowCount())) return;
	int adr = item(row,0)->data(Qt::UserRole).toInt();
	switch (ev->button()) {
		case Qt::MiddleButton:
			blockStart = -1;
			blockEnd = -1;
			emit rqRefill();
			ev->ignore();
			break;
		case Qt::LeftButton:
			if (ev->modifiers() & Qt::ControlModifier) {
				blockStart = adr;
				if (blockEnd < blockStart) blockEnd = blockStart;
				emit rqRefill();
				ev->ignore();
			} else if (ev->modifiers() & Qt::ShiftModifier) {
				blockEnd = adr;
				if (blockStart > blockEnd) blockStart = blockEnd;
				if (blockStart < 0) blockStart = 0;
				emit rqRefill();
				ev->ignore();
			} else {
				markAdr = adr;
				QTableWidget::mousePressEvent(ev);
			}
			break;
		default:
			QTableWidget::mousePressEvent(ev);
			break;
	}
}

void xTableWidget::mouseReleaseEvent(QMouseEvent* ev) {
	if (ev->button() == Qt::LeftButton) {
		markAdr = -1;
	}
}

void xTableWidget::mouseMoveEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	if ((row < 0) || (row >= rowCount())) return;
	int adr = item(row,0)->data(Qt::UserRole).toInt();
	// int len;
	if ((ev->modifiers() == Qt::NoModifier) && (ev->buttons() & Qt::LeftButton) && (adr != blockStart) && (adr != blockEnd) && (markAdr >= 0)) {
		if (adr < blockStart) {
			blockStart = adr;
			blockEnd = markAdr;
		} else {
			blockStart = markAdr;
			blockEnd = adr;
		}
		// len = getCommandSize(comp, blockEnd) - 1;
		// blockEnd += len;
		emit rqRefill();
	}
	QTableWidget::mouseMoveEvent(ev);
}
