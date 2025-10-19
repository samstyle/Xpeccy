#pragma once

#include <QAbstractTableModel>
#include <QResizeEvent>
#include <QTableView>

#include "../classes.h"
#include "../../xcore/xcore.h"

class xRDumpModel : public xTableModel {
	public:
		xRDumpModel(QObject* = nullptr);
		void refill();
	private:
//		int rowCount(const QModelIndex& = QModelIndex()) const;
//		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QList<xRegister> regs;
};

class xRDumpTable : public QTableView {
	Q_OBJECT
	public:
		xRDumpTable(QWidget* = nullptr);
		void update();
	private:
		void resizeEvent(QResizeEvent*);
		xRDumpModel* model;
};

#include "ui_form_regdump.h"

class xRDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xRDumpWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
//		xRDumpModel* model;
		Ui::RDumpWidget ui;
};
