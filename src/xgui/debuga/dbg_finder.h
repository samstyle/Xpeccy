#ifndef _DBG_FINDER_H
#define _DBG_FINDER_H

#include <QDialog>
#include "ui_dbgfinder.h"
#include "../../libxpeccy/memory.h"

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

#endif
