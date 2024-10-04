#pragma once

#include <QAbstractTableModel>
#include "../classes.h"
#include "../../xcore/xcore.h"
#include "ui_form_regdump.h"

class xRDumpModel : public xTableModel {
	public:
		xRDumpModel(QObject* = nullptr);
		void refill();
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
//		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;

		QList<xRegister> regs;
};

class xRDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xRDumpWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		xRDumpModel* model;
		Ui::RDumpWidget ui;
};
