#pragma once

#include <QTableView>
#include <QDockWidget>

#include "../classes.h"

class xDiskDumpModel : public xTableModel {
	public:
		xDiskDumpModel(QObject* = NULL);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		void setDrive(int);
		void setTrack(int);
	private:
		int drv;
		int trk;
		int rcnt;
};

class xDiskDump : public QTableView {
	Q_OBJECT
	public:
		xDiskDump(QWidget* = NULL);
		void update();
	public slots:
		void setTrack(int);
		void setDrive(int);
		void toTarget();
	private:
		int drv;
		xDiskDumpModel* mod;
};

#include "ui_form_fdddump.h"

class xDiskDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xDiskDumpWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::FDDDump ui;
	private slots:
		void toTarget();
};
