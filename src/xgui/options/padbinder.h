#pragma once

#include <QAbstractItemModel>
#include <QDialog>
#include <QKeyEvent>
#include <QTimer>

#include "../../xcore/xcore.h"
#include "ui_padbinder.h"

class xPadMapModel : public QAbstractItemModel {
	Q_OBJECT
	public:
		xPadMapModel(QObject* = NULL);
		void update();
	private:
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent(const QModelIndex& = QModelIndex()) const;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
};

class xPadBinder : public QDialog {
	Q_OBJECT
	public:
		xPadBinder(QWidget* = NULL);
		xJoyMapEntry ent;
		void start(xJoyMapEntry);
	signals:
		void bindReady(xJoyMapEntry);
	private slots:
		void onTimer();
		void okPress();
		void startBindPad();
		void startBindKey();
		void setJoyDir();
		void setMouseDir();
		void onRepSlider(int);

		void gpButtonChanged(int, bool);
		void gpAxisChanged(int, double);
	private:
		Ui::PadBinder ui;
		xPadMapModel* model;
		QTimer timer;
		int mode;
		void close();
		void setPadButtonText();
		void setKeyButtonText();
		void keyPressEvent(QKeyEvent*);
};
