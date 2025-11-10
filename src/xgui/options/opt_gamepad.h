#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QTableView>
#include <QToolButton>

#include "../../xcore/gamepad.h"
#include "ui_opt_gpadwidget.h"

class xPadMapModel : public QAbstractItemModel {
	Q_OBJECT
	public:
		xPadMapModel(xGamepad*, QObject* = nullptr);
		void update();
	private:
		xGamepad* gpad;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent(const QModelIndex& = QModelIndex()) const;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
};

class xGamepadWidget : public QWidget {
	Q_OBJECT
	public:
		xGamepadWidget(xGamepad*, QWidget* = nullptr);
		std::string getMapName();
		void update(std::string);
		void apply();
	public slots:
		void entryReady(xJoyMapEntry);
	private slots:
		void updateList();
	signals:
		void s_edit_entry(xGamepad*, xJoyMapEntry);
	private:
		Ui::GPWidget ui;
		int bindidx;
		xGamepad* gpad;
		xPadMapModel* padmodel;
	private slots:
		void devChanged(int);
		void mapChanged(int);
		void addMap();
		void delMap();
		void addEntry();
		void editEntry();
		void delEntry();
};
