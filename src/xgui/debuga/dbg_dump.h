#pragma once

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QHeaderView>
#include <QDockWidget>
#include <QMenu>

#include "../../xcore/xcore.h"
#include "../../xgui/xgui.h"

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
		int dmpsize;
		void setMode(int, int, int, int);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		void setView(int);
	signals:
		void s_adrch(int);
		void s_datach();
	private:
		int mode;
		int view;
		int page;
		int pgbase;
		int pgsize;
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
//		int getAdr();
		unsigned int limit();
		int mode;
	public slots:
		void setAdr(int);
		int getCurrentAdr();
		void setLimit(unsigned int);
	private slots:
		void curAdrChanged();
	signals:
		void s_datach();
		void s_blockch();
		void s_adrch(int);
		void s_curadrch(int);
	private:
		int view;
		int pagesize;
		int pagenum;
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

#include "ui_form_dump.h"

class xDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xDumpWidget(QString, QString, QWidget* = nullptr);
		void setLimit(int);
	signals:
		void s_adrch(int);
		void s_curadrch(int);
		void s_datach();
		void s_blockch();
		void s_brkrq(int, int, int);
	public slots:
		void draw();
		void setAdr(int);
		void setBase(int, int);
	private:
		Ui::MemDump ui;
		QMenu* cellMenu;
		int ramBase;
		int romBase;
		QItemDelegate* xid_addr;
		QItemDelegate* xid_byte;
		QItemDelegate* xid_octw;
		QItemDelegate* xid_none;
	private slots:
		void adr_changed(int);
		void modeChanged();
		void cp_changed();
		void refill();
		void customMenu();
		void customMenuAction(QAction*);
};
