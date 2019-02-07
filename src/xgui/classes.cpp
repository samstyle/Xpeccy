#include "xgui.h"

#include <QDebug>
#include <QPalette>

QString gethexword(int);
QString gethexbyte(uchar);

xHexSpin::xHexSpin(QWidget* p):QLineEdit(p) {
	setInputMask("Hhhh");
	setText("0000");
	setMinimumWidth(60);
	setMinimumHeight(22);
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
			while (mx <= max) {
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
			while (mx <= max) {
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
	setBase(base);
	if (value < min) setValue(min);
}

void xHexSpin::setMax(int v) {
	max = v;
	setBase(base);
	if (value > max) setValue(max);
}

void xHexSpin::setValue(int nval) {
	nval = minMaxCorrect(nval, min, max);
//	QPalette pal = palette();
	if (value == nval) {
		//pal.setColor(QPalette::Base, QWidget::palette().color(QPalette::Base));
		setStyleSheet("");
	} else {
		value = nval;
		if (hsflag & XHS_BGR) {
			//pal.setColor(QPalette::Base, QColor(160,255,160));
			setStyleSheet("font:bold;");
		} else {
			setStyleSheet("");
			//pal.setColor(QPalette::Base, QColor(255,255,255));
		}
		emit valueChanged(value);
	}
	//setPalette(pal);
}

void xHexSpin::onChange(int val) {
	int pos = cursorPosition();
	QString res = QString::number(val, base).toUpper();
	res = res.rightJustified(inputMask().size(), '0');
	if (text() != res) {
		setText(res);
		setCursorPosition(pos);
	}
}

void xHexSpin::onTextChange(QString txt) {
	if (txt.size() < inputMask().size()) {
		txt = txt.leftJustified(inputMask().size(), '0');
	}
	int nval = txt.toInt(NULL, base);
	int xval = minMaxCorrect(nval, min, max);
	if (value != xval)
		setValue(xval);
	onChange(value);
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
	ev->accept();
}

// xLabel

xLabel::xLabel(QWidget* p):QLabel(p) {}

void xLabel::mousePressEvent(QMouseEvent* ev) {
	emit clicked(ev);
}
