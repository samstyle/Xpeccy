#pragma once

#include <QTableView>

class xDiskDumpModel : public QAbstractTableModel {
	public:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		void setDrive(int);
		void setTrack(int);
		void update();
	private:
		int drv;
		int trk;
		QModelIndex index(int row, int col, const QModelIndex& = QModelIndex()) const;
};

class xDiskDump : public QTableView {
	Q_OBJECT
	public:
		xDiskDump(QWidget* = NULL);
		void update();
	public slots:
		void setTrack(int);
		void setDrive(int);
	private:
		xDiskDumpModel* mod;
};
