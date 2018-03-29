#include "xgui.h"

QString gethexword(int);

xHexSpin::xHexSpin(QWidget* p):QLineEdit(p) {
	setInputMask("Hhhh");
	setText("0000");
	min = 0x0000;
	max = 0xffff;
	connect(this, SIGNAL(valueChanged(int)), SLOT(onChange(int)));
	connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChange(QString)));
}

int xHexSpin::getValue() {
	return value;
}

int minMaxCorrect(int val, int min, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void xHexSpin::setValue(int nval) {
	nval = minMaxCorrect(nval, min, max);
	if (value == nval) return;
	value = nval;
	emit valueChanged(value);
}

void xHexSpin::onChange(int val) {
	int pos = cursorPosition();
	setText(gethexword(val));
	setCursorPosition(pos);
}

void xHexSpin::onTextChange(QString txt) {
	unsigned short nval = txt.toInt(NULL, 16);
	nval = minMaxCorrect(nval, min, max);
	if (value != nval) {
		value = nval;
		emit valueChanged(value);
	}
}

void xHexSpin::keyPressEvent(QKeyEvent* ev) {
	switch(ev->key()) {
		case Qt::Key_Up:
			value = minMaxCorrect(value + 1, min, max);
			emit valueChanged(value);
			break;
		case Qt::Key_Down:
			value = minMaxCorrect(value - 1, min, max);
			emit valueChanged(value);
			break;
		case Qt::Key_PageUp:
			value = minMaxCorrect(value + 0x100, min, max);
			emit valueChanged(value);
			break;
		case Qt::Key_PageDown:
			value = minMaxCorrect(value - 0x100, min, max);
			emit valueChanged(value);
			break;
		default:
			QLineEdit::keyPressEvent(ev);
			break;
	}
}

void xHexSpin::wheelEvent(QWheelEvent* ev) {
	if (ev->delta() < 0) {
		value = minMaxCorrect(value + 1, min, max);
		emit valueChanged(value);
	} else if (ev->delta() > 0) {
		value = minMaxCorrect(value - 1, min, max);
		emit valueChanged(value);
	}
}

// xLabel

xLabel::xLabel(QWidget* p):QLabel(p) {}

void xLabel::mousePressEvent(QMouseEvent* ev) {
	emit clicked(ev);
}
