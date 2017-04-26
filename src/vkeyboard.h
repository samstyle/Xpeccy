#ifndef _VKEYBOARD_H
#define _VKEYBOARD_H

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "libxpeccy/input.h"

class keyWindow : public QLabel {
	Q_OBJECT
	public:
		keyWindow(QWidget* = NULL);
		Keyboard* kb;
	private:
		xKey xk;
	protected:
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
};

#endif
