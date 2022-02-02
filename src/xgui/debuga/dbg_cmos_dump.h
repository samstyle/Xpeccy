#pragma once

#include <QTableView>

class xCmosDumpModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xCmosDumpModel(QObject* p = nullptr);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
	private:
		QModelIndex index(int row, int col, const QModelIndex& = QModelIndex()) const;
};
