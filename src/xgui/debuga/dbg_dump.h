#pragma once

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QHeaderView>

#include "libxpeccy/spectrum.h"

enum {
	XCP_1251 = 1,
	XCP_866,
	XCP_KOI8R
};

enum {
	XVIEW_NONE = 0,
	XVIEW_CPU,
	XVIEW_RAM,
	XVIEW_ROM,
	XVIEW_SLT,
};

enum {
	XVIEW_DEF = 0,
	XVIEW_OCTWRD
};

class xDumpModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xDumpModel(QObject* = NULL);
		int codePage;
		int dmpadr;
		void setComp(Computer**);
		void setMode(int, int);
		void setRows(int);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		void setView(int);
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
		int view;
		int page;
		int row_count;
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
		void setView(int);
		void update();
		void setAdr(int);
		int getAdr();
		int mode;
		int view;
	signals:
		void rqRefill();
	private:
		xDumpModel* model;
		int markAdr;
		int row_count;
		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);
};
