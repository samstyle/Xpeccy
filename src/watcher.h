#pragma once

#include "libxpeccy/spectrum.h"
#include "xgui/xgui.h"
#include "xgui/labelist.h"

#include "ui_watcher.h"
#include "ui_wch_new.h"

#include <QDialog>

enum {
	WUT_CPU = 0,
	WUT_RAM,
	WUT_ROM
};

typedef struct {
	int type;
	QString exp;
} xWatchItem;

class xWatchModel : public QAbstractItemModel {
	Q_OBJECT
	public:
		xWatchModel();
		Computer* comp;
		void update();
		int getItemCount();
		xWatchItem getItem(int);
		void addItem(int, QString);
		void setItem(int, int, QString);
		void delItem(int);
	private:
		// QList<xAdr> list;
		QList<xWatchItem> explist;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent(const QModelIndex&) const;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int = Qt::DisplayRole) const;
//		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
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

		QDialog* newWch;
		xLabeList* labswin;
		Ui::WatcherAdd nui;
	private slots:
		void newWatcher();
		void delWatcher();
		void edtWatcher();
		void confirmNew();
		void insertLabel(QString);
};
