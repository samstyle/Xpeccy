#pragma once

#include <QTableView>
#include "../../libxpeccy/filetypes/filetypes.h"
#include "../xgui.h"

class xDiskCatModel : public xTableModel {
	Q_OBJECT
	public:
		xDiskCatModel(QObject* p = NULL);
		// void update();
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
