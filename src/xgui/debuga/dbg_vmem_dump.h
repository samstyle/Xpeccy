#pragma once

#include <QTableView>
#include "../classes.h"

class xVMemDumpModel : public xTableModel {
	Q_OBJECT
	public:
		xVMemDumpModel(unsigned char* ptr, QObject* p = nullptr);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		Qt::ItemFlags flags(const QModelIndex&) const;
		// void update();
		void setVMem(unsigned char*);
	signals:
		void adr_ch(QModelIndex);
	private:
		// QModelIndex index(int row, int col, const QModelIndex& = QModelIndex()) const;
		unsigned char* vmem;
};

class xVMemDump : public QTableView {
	Q_OBJECT
	public:
		xVMemDump(QWidget* = NULL);
		void update();
		void setVMem(unsigned char*);
	private:
		xVMemDumpModel* mod;
	private slots:
		void jumpToIdx(QModelIndex);
};
