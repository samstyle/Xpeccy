#pragma once

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QMouseEvent>
#include <QTableView>
#include <QKeyEvent>
#include <QVariant>
#include <QDialog>

#include "xgui/xgui.h"
#include "ui_brkmanager.h"
#include "xcore/xcore.h"

class xBreakListModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xBreakListModel(QObject* = NULL);
		void update();
		void updateCell(int, int);
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		void sort(int, Qt::SortOrder);
};

class xBreakTable : public QTableView {
	Q_OBJECT
	public:
		xBreakTable(QWidget* = NULL);
	public slots:
		void update();
	signals:
		void rqDisasm(int);
		void rqDasmDump();
	private:
		xBreakListModel* model;
	private slots:
		void onCellClick(QModelIndex);
		void onDoubleClick(QModelIndex);
	protected:
		void keyPressEvent(QKeyEvent*);
};

class xBrkManager : public QDialog {
	Q_OBJECT
	public:
		xBrkManager(QWidget* = NULL);
		void edit(xBrkPoint*);
	private:
		Ui::BrkManager ui;
		xBrkPoint obrk;
		void setElements(int);
	private slots:
		void confirm();
		void chaType(int);
		void bnkChanged(int);
		void startOffChanged(int);
		void startAbsChanged(int);
		void endOffChanged(int);
		void endAbsChanged(int);
	signals:
		void completed(xBrkPoint, xBrkPoint);
};
