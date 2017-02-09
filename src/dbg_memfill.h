#ifndef _DBG_MEMFILL_H
#define _DBG_MEMFILL_H

#include <QDialog>
#include "ui_filler.h"
#include "libxpeccy/memory.h"

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
	private slots:
		void adrChange();
		void hexChange();
		void fill();
};

#endif
