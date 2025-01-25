// virtual keyboard

#include "vkeyboard.h"
#include "xcore/xcore.h"

#include <QIcon>
#include <QPainter>

typedef struct {
	int x;
	int y;
	int dx;
	int dy;
} xRect;

typedef struct {
	unsigned char ch;
	xRect rect;
	xRect rect2;
} xVKeyMap;

/*
static unsigned char kwMap[4][10] = {
	{'1','2','3','4','5','6','7','8','9','0'},
	{'q','w','e','r','t','y','u','i','o','p'},
	{'a','s','d','f','g','h','j','k','l','E'},
	{'C','z','x','c','v','b','n','m','S',' '}
};
*/

static const xVKeyMap vkZxMap[] = {
	{'1',{3,10,49,60},{0,0,0,0}},{'2',{3 + 49,10,49,60},{0,0,0,0}},{'3',{3 + 49*2,10,49,60},{0,0,0,0}},{'4',{3 + 49*3,10,49,60},{0,0,0,0}},{'5',{3 + 49*4,10,49,60},{0,0,0,0}},
	{'6',{3 + 49*5,10,49,60},{0,0,0,0}},{'7',{3 + 49*6,10,49,60},{0,0,0,0}},{'8',{3 + 49*7,10,49,60},{0,0,0,0}},{'9',{3 + 49*8,10,49,60},{0,0,0,0}},{'0',{3 + 49*9,10,49,60},{0,0,0,0}},
	{'q',{3,70,49,60},{0,0,0,0}},{'w',{3 + 49,70,49,60},{0,0,0,0}},{'e',{3 + 49*2,70,49,60},{0,0,0,0}},{'r',{3 + 49*3,70,49,60},{0,0,0,0}},{'t',{3 + 49*4,70,49,60},{0,0,0,0}},
	{'y',{3 + 49*5,70,49,60},{0,0,0,0}},{'u',{3 + 49*6,70,49,60},{0,0,0,0}},{'i',{3 + 49*7,70,49,60},{0,0,0,0}},{'o',{3 + 49*8,70,49,60},{0,0,0,0}},{'p',{3 + 49*9,70,49,60},{0,0,0,0}},
	{'a',{3,130,49,60},{0,0,0,0}},{'s',{3 + 49,130,49,60},{0,0,0,0}},{'d',{3 + 49*2,130,49,60},{0,0,0,0}},{'f',{3 + 49*3,130,49,60},{0,0,0,0}},{'g',{3 + 49*4,130,49,60},{0,0,0,0}},
	{'h',{3 + 49*5,130,49,60},{0,0,0,0}},{'j',{3 + 49*6,130,49,60},{0,0,0,0}},{'k',{3 + 49*7,130,49,60},{0,0,0,0}},{'l',{3 + 49*8,130,49,60},{0,0,0,0}},{'E',{3 + 49*9,130,49,60},{0,0,0,0}},
	{'C',{3,190,49,60},{0,0,0,0}},{'z',{3 + 49,190,49,60},{0,0,0,0}},{'x',{3 + 49*2,190,49,60},{0,0,0,0}},{'c',{3 + 49*3,190,49,60},{0,0,0,0}},{'v',{3 + 49*4,190,49,60},{0,0,0,0}},
	{'b',{3 + 49*5,190,49,60},{0,0,0,0}},{'n',{3 + 49*6,190,49,60},{0,0,0,0}},{'m',{3 + 49*7,190,49,60},{0,0,0,0}},{'S',{3 + 49*8,190,49,60},{0,0,0,0}},{' ',{3 + 49*9,190,49,60},{0,0,0,0}},
	{0,{0,0,0,0},{0,0,0,0}}
};

keyWindow::keyWindow(QWidget* p):QDialog(p) {
	kb = NULL;
	xent.key = ENDKEY;
	memset(xent.zxKey, 0, 8);
	QPixmap pxm(":/images/keymap_volutar.png");
	setModal(false);
	setWindowModality(Qt::NonModal);
	setFixedSize(pxm.size());
	setWindowIcon(QIcon(":/images/keyboard.png"));
	setWindowTitle("ZX Keyboard");
}

void keyWindow::switcher() {
	if (isVisible())
		hide();
	else
		show();
}

void keyWindow::upd(Keyboard* k) {
	kb = k;
	if (isVisible())
		repaint();
}

void keyWindow::rall(Keyboard* k) {
	if (!isVisible()) {
		kbdReleaseAll(k);
	}
}

// TODO: untide from ZX-keyboard (row,pos calculation)
void keyWindow::paintEvent(QPaintEvent*) {
	QPainter pnt;
/*
//	int wid = width() / 10 + 1;
//	int hig = (height() - 10) / 4;
	int wid = (width() - 6) / 10 + 1;	// 49
	int hig = (height() - 10) / 4;		// 60
*/
	const xRect* prct;
	unsigned char val;
	int row, pos;
	pnt.begin(this);
	pnt.fillRect(0, 0, width(), height(), qRgba(0,0,0,0));
	if (kb) {
		for(int i = 0; i < 8; i++) {
			pos = (i & 4) ? 0 : 9;
			row = (i & 4) ? (i & 3) : (~i & 3);
			val = ~kb->map[i] & 0x1f;
			while(val) {
				if (val & 1) {
					//pnt.fillRect(pos * wid, 10 + row * hig, wid, hig, qRgb(0, 200, 255));
					//pnt.fillRect(3 + pos * wid, 10 + row * hig, wid, hig, qRgb(0, 200, 255));
					prct = &vkZxMap[row * 10 + pos].rect;
					pnt.fillRect(prct->x, prct->y, prct->dx, prct->dy, qRgb(0,200,255));
				}
				val >>= 1;
				pos += (i & 4) ? 1 : -1;
			}
		}
	}
	pnt.drawPixmap(0, 0, QPixmap(":/images/keymap_volutar.png"));
	pnt.end();
}

void keyWindow::mousePressEvent(QMouseEvent* ev) {
	if (!kb) return;

	const xRect* prct = NULL;
	const xRect* rctp;
	int idx = 0;
	int x = ev->xEventX;
	int y = ev->xEventY;
	int dx,dy;
	while(vkZxMap[idx].ch != 0) {
		rctp = &vkZxMap[idx].rect;
		dx = x - rctp->x;
		dy = y - rctp->y;
		if ((dx >= 0) && (dy >= 0) && (dx < rctp->dx) && (dy < rctp->dy)) {
			prct = rctp;
			xent.zxKey[0] = vkZxMap[idx].ch;
		} else {
			rctp = &vkZxMap[idx].rect2;
			dx = x - rctp->x;
			dy = y - rctp->y;
			if ((dx >= 0) && (dy >= 0) && (dx < rctp->dx) && (dy < rctp->dy)) {
				prct = rctp;
				xent.zxKey[0] = vkZxMap[idx].ch;
			}
		}
		idx++;
	}
	if (prct == NULL) return;		// no hit
/*
	int row;
	int col;
	row = (ev->xEventY - 10) * 4 / (height() - 10);
	if (row < 0) row = 0;
	if (row > 3) row = 3;
	col = (ev->xEventX - 3) * 10 / (width() - 3);
	if (col < 0) col = 0;
	if (col > 9) col = 9;
	xent.zxKey[0] = kwMap[row][col];
*/
	xent.zxKey[1] = 0;
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
			xent.zxKey[0] = 0;
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
	if (ev->key() == Qt::Key_Escape) {
		hide();
	} else {
		emit s_key_press(ev);
	}
}

void keyWindow::keyReleaseEvent(QKeyEvent* ev) {
	if (ev->key() != Qt::Key_Escape)
		emit s_key_release(ev);
}
