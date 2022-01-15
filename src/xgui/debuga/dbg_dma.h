#pragma once

#include <QAbstractTableModel>

class xDmaTableModel : public QAbstractTableModel {
	public:
		xDmaTableModel(QObject* = nullptr);
		void update();
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
};
