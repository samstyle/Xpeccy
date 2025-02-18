#pragma once

#include "libxpeccy/spectrum.h"
#include "xgui/xgui.h"
#include "xgui/labelist.h"

#include "ui_watcher.h"
// #include "ui_wch_new.h"

#include <QDialog>

enum {
	// addressation type
	wchAddress = 0x100,
	wchCell,
	// address source
	wchAbsolute,
};

class xWatchModel : public QAbstractItemModel {
	Q_OBJECT
	public:
		xWatchModel();
		Computer* comp;
		void update();
		QString getItem(int);
		void addItem(QString);
		void setItem(int, QString);
		void delItem(int);
	private:
		// QList<xAdr> list;
		QStringList explist;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent(const QModelIndex&) const;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int = Qt::DisplayRole) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		void insertRow(int, const QModelIndex& = QModelIndex());
		void removeRow(int, const QModelIndex& = QModelIndex());
};

class xWatcher : public QDialog {
	Q_OBJECT
	public:
		xWatcher(QWidget* = NULL);
	public slots:
		void fillFields(Computer*);
		void show();
	private:
		int curwch;
		Ui::Watcher ui;
		xWatchModel* model;
		xLabeList* listwin;
		QList<QLabel*> regLabels;
		QList<xHexSpin*> regValues;
		int getCurRow();
	private slots:
		void newWatcher();
		void delWatcher();
		void edtWatcher();
};
