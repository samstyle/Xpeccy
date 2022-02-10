#pragma once

#include <QAbstractTableModel>
#include <QDialog>

#include "../xgui.h"
#include "../../xcore/xcore.h"

#include "ui_rsedit.h"

class xRomsetModel : public QAbstractTableModel {
	public:
		xRomsetModel(QObject* = NULL);
		void update(xRomset*);
	private:
		xRomset* rset;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int) const;
};

class xRomsetEditor : public QDialog {
	Q_OBJECT
	public:
		xRomsetEditor(QWidget* = NULL);
		void edit(xRomFile);
	signals:
		void complete(xRomFile);
	private:
		Ui::RSEdialog ui;
		xRomFile xrf;
	private slots:
		void store();
};
