#ifndef _VKEYBOARD_H
#define _VKEYBOARD_H

#include <QWidget>
#include <QDialog>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "libxpeccy/input.h"

class keyWindow : public QDialog {
	Q_OBJECT
	public:
		keyWindow(QWidget* = NULL);
	signals:
		void s_key_press(QKeyEvent*);
		void s_key_release(QKeyEvent*);
	public slots:
		void switcher();
		void upd(Keyboard*);
		void rall(Keyboard*);
	private:
		Keyboard* kb;
		keyEntry xent;
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
};

#endif
