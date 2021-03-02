#pragma once

#include <QTableView>

class xDiskDumpModel : public QAbstractTableModel {
	public:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		void setDrive(int);
		void setTrack(int);
	private:
		int drv;
		int trk;
};

class xDiskDump : public QTableView {
	Q_OBJECT
	public:
		xDiskDump(QWidget* = NULL);
	public slots:
		void setTrack(int);
		void setDrive(int);
	private:
		xDiskDumpModel* mod;
};
