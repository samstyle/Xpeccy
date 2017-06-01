#ifndef _DBG_DISASM_H
#define _DBG_DISASM_H

#include <QTableWidget>
#include <QKeyEvent>
#include <QMouseEvent>

class xTableWidget : public QTableWidget {
	Q_OBJECT
	public:
		xTableWidget(QWidget*);
	private:
		int markAdr;
	signals:
		void rqRefill();
	protected:
		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
};

#endif
