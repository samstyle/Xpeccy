#pragma once

#include <QDialog>
#include <QMouseEvent>

class xPalGrid : public QWidget {
	Q_OBJECT
	public:
		xPalGrid(QWidget* = nullptr);
		void setPal(QList<QColor>*);
	private:
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		QList<QColor>* pal;
};

#include "ui_paleditor.h"

class xPalEditor : public QDialog {
	Q_OBJECT
	public:
		xPalEditor(QWidget* = nullptr);
		void edit(QList<QColor>*);
	signals:
		void ready();
	private:
		Ui::PalEditor ui;
	private slots:
		void apply();
};
