#include "xgui.h"
#include "../xcore/xcore.h"

#include <QDebug>
#include <QPalette>

QString gethexword(int);
QString gethexbyte(uchar);

xHexSpin::xHexSpin(QWidget* p):QLineEdit(p) {
	setMinimumWidth(60);
	setAutoFillBackground(true);
	setUpdatesEnabled(true);
	vldtr.setRegExp(QRegExp(""));
	min = 0x0000;
	max = 0xffff;
	value = 0x0000;
	hsflag = XHS_DEC;
	len = 6;
	vtxt = "0000";
	// setValidator(&vldtr);
	setBase(16);
	setText(vtxt);
	connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChange(QString)));
}

int xHexSpin::getValue() {
	return value;
}

void xHexSpin::setBase(int b) {
	int mx;
	int tmp = value;
	QString rxp;
	QString digxp;
	switch(b) {
		case 8:
			base = 8;
			digxp = "[0-7]";
			//setStyleSheet("border:1px solid red;");
			break;
		case 10:
			base = 10;
			digxp = "[0-9]";
			//setStyleSheet("border:1px solid black;");
			break;
		default:
			base = 16;
			digxp = "[A-Fa-f0-9]";
			//setStyleSheet("border:1px solid green;");
			break;
	}
//	if (conf.prof.cur) {
//		if (base == conf.prof.cur->zx->hw->base) {
//			setStyleSheet("border:1px solid white;");
//		}
//	}
	len = 1;
	rxp = digxp;
	mx = base;
	while (mx <= max) {
		mx *= base;
		len++;
	}
	rxp.append(QString("{%0}").arg(len));	// 'len' times this char

	// setMaxLength(len);
	setInputMask(QString(len, 'h'));	// to enter overwrite cursor mode. TODO:is there some legit method?
	vldtr.setRegExp(QRegExp(rxp));		// set available chars
	hsflag |= XHS_UPD;			// update even if value doesn't changed
	setValue(tmp);
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

int xHexSpin::getMax() {
	return max;
}

void xHexSpin::updatePal() {
	QPalette pal;
	if (changed) {
		pal.setColor(QPalette::Base, conf.pal["dbg.changed.bg"].isValid() ? conf.pal["dbg.changed.bg"] : pal.toolTipBase().color());
		pal.setColor(QPalette::Text, conf.pal["dbg.changed.txt"].isValid() ? conf.pal["dbg.changed.txt"] : pal.toolTipText().color());
	} else {
		pal.setColor(QPalette::Base, conf.pal["dbg.input.bg"].isValid() ? conf.pal["dbg.input.bg"] : pal.base().color());
		pal.setColor(QPalette::Text, conf.pal["dbg.input.txt"].isValid() ? conf.pal["dbg.input.txt"] : pal.text().color());
	}
	setPalette(pal);
}

void xHexSpin::setValue(int nval) {
	nval = minMaxCorrect(nval, min, max);
	if ((value == nval) && !(hsflag & XHS_UPD)) {
		changed = 0;
	} else {
		value = nval;
		changed = (hsflag & XHS_BGR) ? 1 : 0;
		emit valueChanged(nval);
		onChange(value);
	}
	updatePal();
}

void xHexSpin::onChange(int val) {
	int pos = cursorPosition();
	QString res = QString::number(val, base).toUpper();
	res = res.rightJustified(len, '0');
	if ((text() != res) || (hsflag & XHS_UPD)) {
		hsflag &= ~XHS_UPD;
		setText(res);
		setCursorPosition(pos);
	}
}

void xHexSpin::onTextChange(QString txt) {
	if (txt.size() < len) {
		txt = txt.leftJustified(len, '0');
	} else {
		txt = txt.left(len);
	}
	int pos = 0;
	if (vldtr.validate(txt, pos) == QValidator::Acceptable) {
		vtxt = txt;
		int nval = txt.toInt(NULL, base);
		int xval = minMaxCorrect(nval, min, max);
		if (value != xval)
			setValue(xval);
		else
			onChange(value);
	} else {
		pos = cursorPosition();		// Qt moves cursor at end of field after setText
		setText(vtxt);
		setCursorPosition(pos-1);
	}
}

void xHexSpin::keyPressEvent(QKeyEvent* ev) {
	if (isReadOnly()) return;
	QString txt;
	int pos;
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
		case Qt::Key_Insert:
			pos = cursorPosition();
			txt = vtxt;
			if (inputMask().isEmpty()) {
				setInputMask(QString(len,'H'));
			} else {
				setInputMask(QString());
			}
			setText(txt);
			setCursorPosition(pos);
			break;
		case Qt::Key_X:
			if (hsflag & XHS_DEC) {
				if (base == 8) {
					setBase(10);
				} else if (base == 10) {
					setBase(16);
				} else {
					setBase(8);
				}
			}
			break;
		default:
			QLineEdit::keyPressEvent(ev);
			break;
	}
}

void xHexSpin::wheelEvent(QWheelEvent* ev) {
	if (isReadOnly()) return;
	if (ev->yDelta > 0) {
		setValue(minMaxCorrect(value + 1, min, max));
	} else if (ev->yDelta < 0) {
		setValue(minMaxCorrect(value - 1, min, max));
	}
	ev->accept();
}

// xLabel

xLabel::xLabel(QWidget* p):QLabel(p) {}

void xLabel::mousePressEvent(QMouseEvent* ev) {
	emit clicked(ev);
}
