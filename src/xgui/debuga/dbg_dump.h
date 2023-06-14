#pragma once

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QHeaderView>

#include "../../xcore/xcore.h"
#include "../classes.h"

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

class xDumpModel : public xTableModel {
	Q_OBJECT
	public:
		xDumpModel(QObject* = NULL);
		int codePage;
		unsigned int dmpadr;
		unsigned int maxadr;
		void setMode(int, int, int, int);
		void setRows(int);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		void setView(int);
		//void updateCell(int, int);
		//void updateRow(int);
		//void updateColumn(int);
	signals:
		void s_adrch(int);
		void rqRefill();
	//public slots:
		// void update();
	private:
		int mode;
		int view;
		int page;
		int pgbase;
		int pgsize;
		int row_count;
		int mrd(int) const;
		void mwr(int, unsigned char);
};

class xDumpTable:public QTableView {
	Q_OBJECT
	public:
		xDumpTable(QWidget* = NULL);
		int rows();
		void setCodePage(int);
		void setMode(int, int, int, int);
		void setView(int);
		void update();
		int getAdr();
		unsigned int limit();
		int mode;
		int view;
	public slots:
		void setAdr(int);
		void setLimit(unsigned int);
	signals:
		void s_adrch(int);
		void rqRefill();
	private:
		xDumpModel* model;
		unsigned int markAdr;
		int row_count;
		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);
};
