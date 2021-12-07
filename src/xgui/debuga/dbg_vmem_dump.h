#pragma once

#include <QTableView>

class xVMemDumpModel : public QAbstractTableModel {
	public:
		xVMemDumpModel(unsigned char* ptr, QObject* p = nullptr);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		void update();
		void setVMem(unsigned char*);
	private:
		QModelIndex index(int row, int col, const QModelIndex& = QModelIndex()) const;
		unsigned char* vmem;
};

class xVMemDump : public QTableView {
	public:
		xVMemDump(QWidget* = NULL);
		void update();
		void setVMem(unsigned char*);
	private:
		xVMemDumpModel* mod;
};
