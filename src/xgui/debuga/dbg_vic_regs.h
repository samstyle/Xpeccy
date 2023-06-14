#pragma once

#include "../classes.h"

class xVicRegsModel : public xTableModel {
	public:
		xVicRegsModel(QObject* = nullptr);
	private:
		int rowCount(const QModelIndex&) const;
		int columnCount(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
};
