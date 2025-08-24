#pragma once

#include <QTableView>
#include <QDockWidget>
#include <QResizeEvent>

#include "../classes.h"

class xVMemDumpModel : public xTableModel {
	Q_OBJECT
	public:
		xVMemDumpModel(unsigned char* ptr, QObject* p = nullptr);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		Qt::ItemFlags flags(const QModelIndex&) const;
		// void update();
		void setVMem(unsigned char*);
	signals:
		void adr_ch(QModelIndex);
	private:
		unsigned char* vmem;
};

class xVMemDump : public QTableView {
	Q_OBJECT
	public:
		xVMemDump(QWidget* = NULL);
		void update();
		void setVMem(unsigned char*);
	private:
		xVMemDumpModel* mod;
		void resizeEvent(QResizeEvent*);
	private slots:
		void jumpToIdx(QModelIndex);
};

#include "ui_form_vmemdump.h"

class xVMemDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xVMemDumpWidget(QString, QString, QWidget* = nullptr);
		void setVMem(unsigned char*);
	public slots:
		void draw();
	private:
		Ui::VMemDump ui;
	private slots:
		void save_vram();

};
