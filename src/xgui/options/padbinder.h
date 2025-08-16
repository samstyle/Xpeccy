#pragma once

#include <QAbstractItemModel>
#include <QDialog>
#include <QKeyEvent>
#include <QTimer>

#include "../../xcore/xcore.h"
#include "ui_padbinder.h"

class xPadBinder : public QDialog {
	Q_OBJECT
	public:
		xPadBinder(QWidget* = NULL);
		xJoyMapEntry ent;
	public slots:
		void start(xGamepad*, xJoyMapEntry);
	signals:
		void bindReady(xJoyMapEntry);
	private slots:
		void reject();
		void okPress();
		void startBindKey();
		void setJoyDir();
		void setMouseDir();
		void onRepSlider(int);
		void seqFinished();

		void gpButtonChanged(int, bool);
		void gpAxisChanged(int, double);
	private:
		Ui::PadBinder ui;
		xGamepad* gpad;
//		xPadMapModel* model;
		int mode;
//		void close();
		void setPadButtonText();
		void setKeyButtonText();
		void keyPressEvent(QKeyEvent*);
};
