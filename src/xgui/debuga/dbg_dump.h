#ifndef _DBG_DUMP_H
#define _DBG_DUMP_H

#include <QTableView>
#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QWheelEvent>

#include "../../libxpeccy/spectrum.h"

enum {
	XCP_1251 = 1,
	XCP_866,
	XCP_KOI8R
};

typedef int(*dmpMrd)(unsigned short, void*);
typedef void(*dmpMwr)(unsigned short, unsigned char, void*);

class xDumpModel:public QAbstractItemModel {
	Q_OBJECT
	public:
		xDumpModel(void**, dmpMrd, dmpMwr, QObject* = NULL);
		int codePage;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		void updateCell(int, int);
		void updateRow(int);
	public slots:
		void update();
	private:
		void** pptr;
		dmpMrd mrd;
		dmpMwr mwr;
};

class xDumpTable:public QTableView {
	Q_OBJECT
	public:
		xDumpTable(QWidget* = NULL);
		Computer** cptr;
	signals:
		void rqRefill();
	private:
		int markAdr;
		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
};

#endif
