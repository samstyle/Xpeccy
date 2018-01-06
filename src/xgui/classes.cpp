#include "xgui.h"

QString gethexword(int);

xHexSpin::xHexSpin(QWidget* p):QLineEdit(p) {
	setInputMask("HHHH");
	setText("0000");
	connect(this, SIGNAL(valueChanged(int)), SLOT(onChange(int)));
	connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChange(QString)));
}

int xHexSpin::getValue() {
	return value & 0xffff;
}

void xHexSpin::setValue(int nval) {
	nval &= 0xffff;
	if (value == nval) return;
	value = nval & 0xffff;
	emit valueChanged(value & 0xffff);
}

void xHexSpin::onChange(int val) {
	val &= 0xffff;
	int pos = cursorPosition();
	setText(gethexword(val));
	setCursorPosition(pos);
}

void xHexSpin::onTextChange(QString txt) {
	unsigned short nval = txt.toInt(NULL, 16) & 0xffff;
	if (value != nval) {
		value = nval;
		emit valueChanged(value & 0xffff);
	}
}

void xHexSpin::keyPressEvent(QKeyEvent* ev) {
	switch(ev->key()) {
		case Qt::Key_Up:
			value++;
			emit valueChanged(value & 0xffff);
			break;
		case Qt::Key_Down:
			value--;
			emit valueChanged(value & 0xffff);
			break;
		case Qt::Key_PageUp:
			value += 0x100;
			emit valueChanged(value & 0xffff);
			break;
		case Qt::Key_PageDown:
			value -= 0x100;
			emit valueChanged(value & 0xffff);
			break;
		default:
			QLineEdit::keyPressEvent(ev);
			break;
	}
}

void xHexSpin::wheelEvent(QWheelEvent* ev) {
	if (ev->delta() < 0) {
		value++;
		emit valueChanged(value & 0xffff);
	} else if (ev->delta() > 0) {
		value--;
		emit valueChanged(value & 0xffff);
	}
}
