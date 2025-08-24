#pragma once

#include "../classes.h"

#include <QResizeEvent>
#include <QTableView>

class xCmosDumpModel : public xTableModel {
	Q_OBJECT
	public:
		xCmosDumpModel(QObject* p = nullptr);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		// QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
};

class xCmosDumpTable : public QTableView {
	Q_OBJECT
	public:
		xCmosDumpTable(QWidget* = nullptr);
	private:
		void resizeEvent(QResizeEvent*);
};

#include "ui_form_cmosdump.h"

class xCmosDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xCmosDumpWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::CmosDump ui;
};
