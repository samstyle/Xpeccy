#pragma once

#include <QDialog>
#include "../../xgui/xgui.h"
#include "../../libxpeccy/memory.h"
#include "ui_dbgfinder.h"

class xMemFinder : public QDialog {
	Q_OBJECT
	public:
		xMemFinder(QWidget* = NULL);
		Memory* mem;
		int adr;
	signals:
		void patFound(int);
	private:
		Ui::xFinder ui;
	private slots:
		void onBytesEdit();
		void onTextEdit();
		void doFind();
};
