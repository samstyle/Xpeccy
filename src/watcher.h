#pragma once

#include "libxpeccy/spectrum.h"
#include "xgui/xgui.h"
#include "xgui/labelist.h"

#include "ui_watcher.h"
#include "ui_wch_new.h"

#include <QDialog>

enum {
	// addressation type
	wchAddress = 0,
	wchCell,
	// address source
	wchAbsolute,
	wchBC,
	wchDE,
	wchHL,
	wchBCa,
	wchDEa,
	wchHLa,
	wchSP,
	wchIX,
	wchIY
};

class xWatchModel : public QAbstractItemModel {
	Q_OBJECT
	public:
		xWatchModel();
		Computer* comp;
		void update();
		xAdr getItem(int);
		void addItem(xAdr);
		void updItem(int, xAdr);
		void delItem(int);
		int currow;
	private:
		QList<xAdr> list;
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
	private:
		int curwch;
		Ui::Watcher ui;
		Ui::WatcherAdd nui;
		QDialog* addial;
		xWatchModel* model;
		xLabeList* listwin;
		int getCurRow();
		void fillDial();
	private slots:
		void addWatcher();
		void newWatcher();
		void delWatcher();
		void edtWatcher();
		void dialChanged();
		void setLabel(QString);
};
