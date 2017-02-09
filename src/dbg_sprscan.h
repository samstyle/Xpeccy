#ifndef _DBG_SPRSCAN_H
#define _DBG_SPRSCAN_H

#include <QDialog>
#include <QWheelEvent>

#include "ui_memviewer.h"
#include "libxpeccy/spectrum.h"

class MemViewer : public QDialog {
	Q_OBJECT
	public:
		MemViewer(QWidget* = NULL);
		Memory* mem;
		Ui::MemView ui;
		QPoint winPos;
		unsigned vis:1;
	private:
		unsigned char rdMem(int);
	public slots:
		void fillImage();
	private slots:
		void adrChanged(int);
		void hexChanged();
		void memScroll(int);
		void saveSprite();
	protected:
		void wheelEvent(QWheelEvent*);
};

#endif
