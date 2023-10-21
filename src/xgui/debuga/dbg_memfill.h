#pragma once

#include <QDialog>

#include "../../xgui/xgui.h"
#include "../../libxpeccy/memory.h"
#include "ui_filler.h"

class xMemFiller : public QDialog {
	Q_OBJECT
	public:
		xMemFiller(QWidget* = NULL);
		void start(Memory*, int, int);
	signals:
		void rqRefill();
	private:
		Ui::Filler ui;
		Memory* mem;
		int mrd(int);
		void mwr(int, int);
	private slots:
		void metChange();
		void fill();
};
