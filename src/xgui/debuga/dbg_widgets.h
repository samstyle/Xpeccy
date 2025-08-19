#pragma once

#include <QDockWidget>

#include "../classes.h"
#include "../../xcore/xcore.h"
#include "../../xgui/xgui.h"

#include "dbg_brkpoints.h"
#include "dbg_disasm.h"
#include "dbg_diskdump.h"
#include "dbg_dump.h"
#include "dbg_finder.h"
#include "dbg_memfill.h"
#include "dbg_sprscan.h"
#include "dbg_vmem_dump.h"
#include "dbg_rdump.h"

// ay

#include "ui_form_ay.h"

class xAYWidget : public xDockWidget {
	Q_OBJECT
	public:
		xAYWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::AYWidget ui;
};

// cia

#include "ui_form_cia.h"

class xCiaWidget : public xDockWidget {
	Q_OBJECT
	public:
		xCiaWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::CIAWidget ui;
};


// cmos

class xCmosDumpModel : public xTableModel {
	Q_OBJECT
	public:
		xCmosDumpModel(QObject* p = nullptr);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
};

#include "ui_form_cmosdump.h"

class xCmosDumpWidget : public xDockWidget {
	Q_OBJECT
	public:
		xCmosDumpWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::CmosDump ui;
};

// dma

class xDmaTableModel : public xTableModel {
	public:
		xDmaTableModel(QObject* = nullptr);
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
};

#include "ui_form_dma.h"

class xDmaWidget : public xDockWidget {
	Q_OBJECT
	public:
		xDmaWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::DMAWidget ui;
};

// fdc

#include "ui_form_fdd.h"

class xFDDWidget : public xDockWidget {
	Q_OBJECT
	public:
		xFDDWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::FDDWidget ui;
};

// gameboy

#include "ui_form_gameboy.h"
#include "ui_form_vga.h"

class xGameboyWidget : public xDockWidget {
	Q_OBJECT
	public:
		xGameboyWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::GBWidget ui;
};

class xGBVideoModel : public xTableModel {
	public:
		xGBVideoModel(QObject* = nullptr);
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
};

class xGBVideoWidget : public xDockWidget {
	Q_OBJECT
	public:
		xGBVideoWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::TableWidget ui;
};

// memmap

#include "ui_form_mem.h"

class xMMapWidget : public xDockWidget {
	Q_OBJECT
	public:
		xMMapWidget(QString, QString, QWidget* = nullptr);
		MemPage mem_map[256];
	signals:
		void s_restore();
		void s_remap(int, int, int);
	public slots:
		void draw();
	private slots:
		void remap_b0();
		void remap_b1();
		void remap_b2();
		void remap_b3();
	private:
		Ui::MMapWidget ui;
};

// nesapu

// nesppu

#include "ui_form_nesppu.h"

class xPPUWidget : public xDockWidget {
	Q_OBJECT
	public:
		xPPUWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::PPUWidget ui;
};

// pic

class xPicModel : public xTableModel {
	public:
		xPicModel(QObject* = NULL);
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
};

#include "ui_form_pic.h"

class xPicWidget : public xDockWidget {
	Q_OBJECT
	public:
		xPicWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::PICWidget ui;
};

// pit

class xPitModel : public xTableModel {
	public:
		xPitModel(QObject* = NULL);
	private:
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
};

#include "ui_form_pit.h"

class xPitWidget : public xDockWidget {
	Q_OBJECT
	public:
		xPitWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::PITWidget ui;
};

// tape

#include "ui_form_tape.h"

class xTapeWidget : public xDockWidget {
	Q_OBJECT
	public:
		xTapeWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::TapeWidget ui;
};

// vga

class xVgaRegModel : public xTableModel {
	public:
		xVgaRegModel(QObject* = nullptr);
	private:
		int rowCount(const QModelIndex&) const;
		int columnCount(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
};

#include "ui_form_vga.h"

class xVgaWidget : public xDockWidget {
	Q_OBJECT
	public:
		xVgaWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::TableWidget ui;
};

// vic-ii

class xVicRegsModel : public xTableModel {
	public:
		xVicRegsModel(QObject* = nullptr);
	private:
		int rowCount(const QModelIndex&) const;
		int columnCount(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
};

#include "ui_form_vic.h"

class xVicWidget : public xDockWidget {
	Q_OBJECT
	public:
		xVicWidget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::VICWidget ui;
};

// zxscr

#include "ui_form_zxscreen.h"

class xZXScrWidget : public xDockWidget {
	Q_OBJECT
	public:
		xZXScrWidget(QString, QString, QWidget* = nullptr);
		void setAddress(int, int);
	public slots:
		void draw();
		void setZoom(int);
	private:
		Ui::ZXScrWidget ui;
		QImage scrImg;
};

// ps/2

#include "ui_form_ps2.h"

class xPS2Widget : public xDockWidget {
	Q_OBJECT
	public:
		xPS2Widget(QString, QString, QWidget* = nullptr);
	public slots:
		void draw();
	private:
		Ui::PS2Widget ui;
};
