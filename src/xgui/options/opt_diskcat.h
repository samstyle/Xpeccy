#pragma once

#include <QAbstractTableModel>
#include <QTableView>
#include "../../libxpeccy/filetypes/filetypes.h"

class xDiskCatModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xDiskCatModel(QObject* p = NULL);
		void update();
		void setCatalog(QList<TRFile>);
	private:
		QList<TRFile> cat;

		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int) const;
};

class xDiskCatTable : public QTableView {
	Q_OBJECT
	public:
		xDiskCatTable(QWidget* p = NULL);
		void setCatalog(QList<TRFile>);
	private:
		xDiskCatModel* model;
};
