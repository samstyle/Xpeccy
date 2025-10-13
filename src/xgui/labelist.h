#pragma once

#include <QAbstractListModel>
#include <QDialog>
#include "../xcore/xcore.h"

#include "ui_labelist.h"

class xLabelistModel : public QAbstractListModel {
	Q_OBJECT
	public:
		xLabelistModel(QObject* = NULL);
		QStringList list;
	public slots:
		void reset(QString = QString());
	protected:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
};

class xLabeList : public QDialog {
	Q_OBJECT
	public:
		xLabeList(QWidget* = NULL);
	public slots:
		void show();
	signals:
		void labSelected(QString);
		void labSetChanged();
	private slots:
		void listDoubleClicked(QModelIndex);
		void changeLabelSet();
		void newGroup();
		void editGroup();
		void delGroup();
	private:
		Ui::LabList ui;
		xLabelistModel* mod;
		void fillSetList();
};
