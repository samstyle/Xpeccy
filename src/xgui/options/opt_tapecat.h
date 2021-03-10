#pragma once

#include <QAbstractTableModel>
#include <QTableView>

#include "libxpeccy/tape.h"

class xTapeCatModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xTapeCatModel(QObject* p = NULL);
		void fill(Tape*);
	private:
		int rcnt;
		int rcur;
		TapeBlockInfo* inf;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int) const;
		void update();
};

class xTapeCatTable : public QTableView {
	Q_OBJECT
	public:
		xTapeCatTable(QWidget* = NULL);
		void fill(Tape*);
	private:
		xTapeCatModel* model;
};
