#pragma once

#include <QAbstractTableModel>

class xPicModel : public QAbstractTableModel {
	public:
		xPicModel(QObject* = NULL);
		void update();
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
};
