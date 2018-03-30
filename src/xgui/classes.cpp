#include "xgui.h"

#include <QDebug>
#include <QPalette>

QString gethexword(int);
QString gethexbyte(uchar);

xHexSpin::xHexSpin(QWidget* p):QLineEdit(p) {
	setInputMask("Hhhh");
	setText("0000");
	min = 0x0000;
	max = 0xffff;
	value = 0x0000;
	base = 16;
	hsflag = XHS_DEC;
	connect(this, SIGNAL(valueChanged(int)), SLOT(onChange(int)));
	connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChange(QString)));
}

int xHexSpin::getValue() {
	return value;
}

void xHexSpin::setBase(int b) {
	int mx;
	QString msk;
	QPalette pal = palette();
	switch(b) {
		case 10:
			b = value;
			base = 10;
			msk = "N";
			mx = 10;
			while (mx < max) {
				msk.append("n");
				mx *= 10;
			}
			pal.setColor(QPalette::Text, QColor(0,0,255));
			break;
		default:
			b = value;
			base = 16;
			msk = "H";
			mx = 16;
			while (mx < max) {
				msk.append("h");
				mx *= 16;
			}
			pal.setColor(QPalette::Text, QColor(0,0,0));
			break;
	}
	setPalette(pal);
	setInputMask(msk);
	setValue(b);
}

void xHexSpin::setXFlag(int xf) {
	hsflag = xf;
}

int minMaxCorrect(int val, int min, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void xHexSpin::setMin(int v) {
	min = v;
	if (value < min) setValue(min);
}

void xHexSpin::setMax(int v) {
	max = v;
	if (value > max) setValue(max);
}

void xHexSpin::setValue(int nval) {
	nval = minMaxCorrect(nval, min, max);
	QPalette pal = palette();
	if (value == nval) {
		pal.setColor(QPalette::Base, QColor(255,255,255));
	} else {
		value = nval;
		if (hsflag & XHS_BGR) {
			pal.setColor(QPalette::Base, QColor(160,255,160));
		} else {
			pal.setColor(QPalette::Base, QColor(255,255,255));
		}
		emit valueChanged(value);
	}
	setPalette(pal);
}

QString xHexSpin::getText(int val) {
	QString res;
	int sz = inputMask().size();
	if (sz > 5) sz = 5;
	switch(base) {
		case 10: res = QString::number(val, 10); break;
		default:
			if (max < 0x100) {
				res = gethexbyte(val);
			} else {
				res = gethexword(val);
			}
			break;
	}
	return res;
}

void xHexSpin::onChange(int val) {
	int pos = cursorPosition();
	setText(getText(val));
	setCursorPosition(pos);
}

void xHexSpin::onTextChange(QString txt) {
	int nval = txt.toInt(NULL, base);
	int xval = minMaxCorrect(nval, min, max);
	if (value != xval)
		setValue(xval);
}

void xHexSpin::keyPressEvent(QKeyEvent* ev) {
	switch(ev->key()) {
		case Qt::Key_Up:
			setValue(minMaxCorrect(value + 1, min, max));
			break;
		case Qt::Key_Down:
			setValue(minMaxCorrect(value - 1, min, max));
			break;
		case Qt::Key_PageUp:
			setValue(minMaxCorrect(value + 0x100, min, max));
			break;
		case Qt::Key_PageDown:
			setValue(minMaxCorrect(value - 0x100, min, max));
			break;
		case Qt::Key_X:
			if (hsflag & XHS_DEC) {
				setBase((base == 10) ? 16 : 10);
			}
			break;
		default:
			QLineEdit::keyPressEvent(ev);
			break;
	}
}

void xHexSpin::wheelEvent(QWheelEvent* ev) {
	if (ev->delta() < 0) {
		setValue(minMaxCorrect(value + 1, min, max));
	} else if (ev->delta() > 0) {
		setValue(minMaxCorrect(value - 1, min, max));
	}
}

// xLabel

xLabel::xLabel(QWidget* p):QLabel(p) {}

void xLabel::mousePressEvent(QMouseEvent* ev) {
	emit clicked(ev);
}
