#ifndef X_DBG_DUMP_H
#define X_DBG_DUMP_H

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QWheelEvent>

#include "libxpeccy/spectrum.h"

enum {
	XCP_1251 = 1,
	XCP_866,
	XCP_KOI8R
};

enum {
	XVIEW_CPU = 1,
	XVIEW_RAM,
	XVIEW_ROM,
	XVIEW_SLT
};

class xDumpModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xDumpModel(QObject* = NULL);
		int codePage;
		void setComp(Computer**);
		void setMode(int, int);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		void updateCell(int, int);
		void updateRow(int);
		void updateColumn(int);
	signals:
		void rqRefill();
	public slots:
		void update();
	private:
		Computer** cptr;
		int mode;
		int page;
		int mrd(int) const;
		void mwr(int, unsigned char);
};

class xDumpTable:public QTableView {
	Q_OBJECT
	public:
		xDumpTable(QWidget* = NULL);
		Computer** cptr;
		void setComp(Computer**);
		int rows();
		void setCodePage(int);
		void setMode(int, int);
		void update();
	signals:
		void rqRefill();
	private:
		xDumpModel* model;
		int markAdr;
		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
};

#endif
