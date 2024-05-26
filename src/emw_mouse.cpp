#include "emulwin.h"

#include <QMenu>

void MainWin::mousePressEvent(QMouseEvent *ev){
	Computer* comp = conf.prof.cur->zx;
	if (comp->debug) {
		if ((ev->button() == Qt::LeftButton) && (comp->hw->grp == HWG_ZX)) {
			calcCoords(ev);
		}
		ev->ignore();
	} else {
		switch (ev->button()) {
			case Qt::LeftButton:
				if (grabMice) {
					comp->mouse->lmb = 1;
					mouse_interrupt(comp->mouse);
				} else if (comp->hw->grp == HWG_ZX) {	// zx: print dot address
					if (ev->modifiers() & Qt::ControlModifier)
						calcCoords(ev);
				}
				break;
			case Qt::RightButton:
				if (grabMice) {
					comp->mouse->rmb = 1;
					mouse_interrupt(comp->mouse);
				} else {
					fillUserMenu();
					userMenu->popup(QPoint(ev->xGlobalX,ev->xGlobalY));
					userMenu->setFocus();
				}
				break;
			default: break;
		}
	}
}

void MainWin::mouseReleaseEvent(QMouseEvent *ev) {
	if (conf.emu.pause) return;
	Computer* comp = conf.prof.cur->zx;
	if (comp->debug) {
		ev->ignore();
	} else {
		switch (ev->button()) {
			case Qt::LeftButton:
				if (grabMice) {
					comp->mouse->lmb = 0;
					mouse_interrupt(comp->mouse);
#ifdef __APPLE__
				} else if (comp->mouse->enable) {
					grabMice = 1;
					grabMouse(QCursor(Qt::BlankCursor));
					setMessage(" grab mouse ");
#endif
				}
				break;
			case Qt::RightButton:
				if (grabMice) {
					comp->mouse->rmb = 0;
					mouse_interrupt(comp->mouse);
				}
				break;
			case X_MidButton:
				grabMice = !grabMice;
				if (grabMice) {
					grabMouse(QCursor(Qt::BlankCursor));
					setMessage(" grab mouse ");
				} else {
					releaseMouse();
					setMessage(" release mouse ");
					cursor().setPos(pos().x() + width() / 2, pos().y() + height() / 2);
				}
				break;
			default: break;
		}
	}
}

void MainWin::wheelEvent(QWheelEvent* ev) {
	Computer* comp = conf.prof.cur->zx;
	if (comp->debug) {
		ev->ignore();
	} else if (grabMice) {
		if (comp->mouse->hasWheel)
			mousePress(comp->mouse, (ev->yDelta < 0) ? XM_WHEELDN : XM_WHEELUP, 0);
	} else {
		if (ev->yDelta < 0) {
			conf.snd.vol.master -= 5;
			if (conf.snd.vol.master < 0)
				conf.snd.vol.master = 0;
		} else {
			conf.snd.vol.master += 5;
			if (conf.snd.vol.master > 100)
				conf.snd.vol.master = 100;
		}
		setMessage(QString(" volume %0% ").arg(conf.snd.vol.master));
	}
}

static int dumove = 0;

void MainWin::mouseMoveEvent(QMouseEvent *ev) {
	Computer* comp = conf.prof.cur->zx;
	if (!grabMice || conf.emu.pause) {
		if ((ev->buttons() & Qt::LeftButton) && (comp->hw->grp == HWG_ZX)) {
			calcCoords(ev);
		}
	} else if (dumove) {			// it was dummy move to center of screen
		dumove = 0;
	} else {
		QPoint dpos = pos() + QPoint(width()/2, height()/2);
		// TODO: apply sensitivity on xdelta/ydelta - ps/2 interrupt works with delta
		comp->mouse->xdelta = ev->xGlobalX - dpos.x();
		comp->mouse->ydelta = dpos.y() - ev->xGlobalY;		// revert sign
		comp->mouse->xpos += comp->mouse->xdelta;
		comp->mouse->ypos += comp->mouse->ydelta;
		mouse_interrupt(comp->mouse);
		dumove = 1;
		cursor().setPos(dpos);
	}
}
