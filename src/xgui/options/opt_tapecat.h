#pragma once

#include <QAbstractTableModel>
#include <QTableView>

#include "libxpeccy/tape.h"
#include "../classes.h"

class xTapeCatModel : public xTableModel {
	Q_OBJECT
	public:
		xTapeCatModel(QObject* p = NULL);
		void fill(Tape*);
	private:
		int rcur;
		TapeBlockInfo* inf;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int) const;
};

class xTapeCatTable : public QTableView {
	Q_OBJECT
	public:
		xTapeCatTable(QWidget* = NULL);
		void fill(Tape*);
	private:
		xTapeCatModel* model;
};
