// virtual keyboard

#include "vkeyboard.h"
#include "xcore/xcore.h"

#include <QPainter>

static unsigned char kwMap[4][10] = {
	{'1','2','3','4','5','6','7','8','9','0'},
	{'q','w','e','r','t','y','u','i','o','p'},
	{'a','s','d','f','g','h','j','k','l','E'},
	{'C','z','x','c','v','b','n','m','S',' '}
};

keyWindow::keyWindow(QWidget* p):QLabel(p) {
	kb = NULL;
	xent.key = ENDKEY;
	xent.zxKey.key1 = 0;
	xent.zxKey.key2 = 0;
	// setWindowModality(Qt::WindowModal);
}

void keyWindow::paintEvent(QPaintEvent*) {
	QPainter pnt;
	int wid = width() / 10 + 1;
	int hig = (height() - 10) / 4;
	unsigned char val;
	int row, pos;
	pnt.begin(this);
	pnt.fillRect(QRectF(0,0,1,1), qRgba(0,0,0,0));
//	pnt.setPen(qRgb(0, 200, 255));
//	pnt.setBrush(QBrush(qRgb(0, 180, 235)));
	for(int i = 0; i < 8; i++) {
		pos = (i & 4) ? 0 : 9;
		row = (i & 4) ? (i & 3) : (~i & 3);
		val = ~kb->map[i] & 0x1f;
		while(val) {
			if (val & 1) {
				pnt.fillRect(pos * wid, 10 + row * hig, wid, hig, qRgb(0, 200, 255));
				//pnt.drawRoundRect(pos * wid, row * hig, wid, hig);
			}
			val >>= 1;
			pos += (i & 4) ? 1 : -1;
		}
	}
	pnt.drawPixmap(0, 0, QPixmap(":/images/keymap.png"));
	pnt.end();
}

void keyWindow::mousePressEvent(QMouseEvent* ev) {
	if (!kb) return;
	int row;
	int col;
	row = ev->y() * 4 / height();
	col = ev->x() * 10 / width();
	xent.zxKey.key1 = kwMap[row][col];
	switch(ev->button()) {
		case Qt::LeftButton:
			kbdPress(kb, xent);
			update();
			break;
		case Qt::RightButton:
			kbdTrigger(kb, xent);
			update();
			break;
		case Qt::MiddleButton:
			kbdReleaseAll(kb);
			xent.zxKey.key1 = 0;
			update();
			break;
		default:
			break;
	}
}

void keyWindow::mouseReleaseEvent(QMouseEvent* ev) {
	if (!kb) return;
	if (ev->button() == Qt::LeftButton) {
		kbdRelease(kb, xent);
	}
	update();
}

void keyWindow::keyPressEvent(QKeyEvent* ev) {
	int keyid = qKey2id(ev->key());
	keyEntry kent = getKeyEntry(keyid);
	if (ev->modifiers() & Qt::AltModifier) {
		if (ev->key() == Qt::Key_K)
			close();
	} else {
		kbdPress(kb, kent);
		update();
	}
}

void keyWindow::keyReleaseEvent(QKeyEvent* ev) {
	int keyid = qKey2id(ev->key());
	keyEntry kent = getKeyEntry(keyid);
	kbdRelease(kb, kent);
	update();
}
