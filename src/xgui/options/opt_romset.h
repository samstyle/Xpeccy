#ifndef X_OPT_ROMSET_H
#define X_OPT_ROMSET_H

#include <QAbstractTableModel>
#include <QDialog>

#include "xcore.h"

#include "ui_rsedit.h"

class xRomsetModel : public QAbstractTableModel {
	public:
		xRomsetModel(QObject* = NULL);
		void update(xRomset);
	private:
		xRomset rset;
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
};

class xRomsetEditor : public QDialog {
	Q_OBJECT
	public:
		xRomsetEditor(QWidget* = NULL);
		void edit(int = -1);
	signals:
		void complete(int, QString);
	private:
		Ui::RSEdialog ui;
		int idx;
		xRomset nrs;
	private slots:
		void store();
		void check();
		void grpSingle(bool);
		void grpSeparate(bool);
};

#endif
