#pragma once

#include "dbg_widgets.h"
#include "ui_form_palette.h"

class xPalWidget : public xDockWidget {
	Q_OBJECT
	public:
		xPalWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::Palette ui;
		void mousePressEvent(QMouseEvent*);
};
