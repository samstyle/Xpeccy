#pragma once

#include <QDialog>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QKeySequence>
#include <QKeyEvent>
#include <QAbstractTableModel>

class xKeyEditor : public QDialog {
	Q_OBJECT
	public:
		xKeyEditor(QWidget* p = NULL);
		void edit(int);
	signals:
		void s_done(int, QKeySequence);
	private:
		int foo;
		QLabel lab;
		QPushButton but;
		QKeySequence kseq;
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
	private slots:
		void okay();
		void reject();
};

class xHotkeyModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xHotkeyModel(QObject* p = nullptr);
		int rowCount(const QModelIndex& idx = QModelIndex()) const;
		int columnCount(const QModelIndex& idx = QModelIndex()) const;
		QVariant data(const QModelIndex& idx, int role) const;
		void updateCell(int, int);
	private:
		int rows;
};

class xHotkeyTable : public QTableView {
	Q_OBJECT
	public:
		xHotkeyTable(QWidget* p = nullptr);
	public slots:
		void set_seq(int, QKeySequence);
		void dbl_click(QModelIndex);
	private:
		xHotkeyModel* model;
		xKeyEditor* edt;
};
