#pragma once

#include <QAbstractTableModel>

class xTableModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xTableModel(QObject* = NULL);
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
	public slots:
		void update();
		void updateRow(int);
		void updateColumn(int);
		void updateCell(int, int);
};
