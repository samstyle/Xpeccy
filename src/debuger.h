#ifndef _DEBUGER_H
#define _DEBUGER_H

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include <QItemDelegate>

#include "ui_dumpdial.h"
#include "ui_openDump.h"
#include "ui_bpeditor.h"
#include "ui_debuger.h"
#include "libxpeccy/spectrum.h"

#define DASMROW 26
#define DMPSIZE 16

struct DasmRow {
	ushort adr;
	QString bytes;
	QString com;
};

#define XTYPE_ADR 0
#define XTYPE_DUMP 1
#define XTYPE_BYTE 2

class xItemDelegate : public QItemDelegate {
	public:
		xItemDelegate(int);
		int type;
		QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;

};

class DebugWin : public QDialog {
	Q_OBJECT
	public:
		DebugWin(QWidget*);
		bool active;
		void reject();
		void start(ZXComp*);
		void stop();
	signals:
		void closed();
	private:
		ZXComp* comp;
		bool block;
		long tCount;
//		unsigned long lastDbgTicks = 0;
//		QString logFileName;
//		QFile logFile;

		Ui::Debuger ui;

		QDialog* dumpwin;
		Ui::DumpDial dui;
		QByteArray getDumpData();

		QDialog* openDumpDialog;
		Ui::oDumpDial oui;
		QString dumpPath;

		QDialog* bpEditor;
		Z80EX_WORD bpAdr;
		Ui::BPDialog bui;

		Z80EX_WORD disasmAdr;
		Z80EX_WORD dumpAdr;

		bool fillAll();
		void fillZ80();
		void fillFlags();
		void fillMem();
		void fillDump();
		bool fillDisasm();
		void fillStack();

		Z80EX_WORD getPrevAdr(Z80EX_WORD);
		void doBreakPoint(Z80EX_WORD);
		void scrollDown();
		void scrollUp();

	private slots:
		void setZ80();
		void setFlags();

		void dasmEdited(int, int);
		void dumpEdited(int, int);

		void putBreakPoint();
		void chaBreakPoint();

		void doOpenDump();
		void chDumpFile();
		void dmpStartOpen();
		void loadDump();

		void doSaveDump();
		void dmpLimChanged();
		void dmpLenChanged();
		void saveDumpBin();
		void saveDumpHobeta();
		void saveDumpToDisk(int);
		void saveDumpToA();
		void saveDumpToB();
		void saveDumpToC();
		void saveDumpToD();
	protected:
		void keyPressEvent(QKeyEvent*);
		void wheelEvent(QWheelEvent*);
};

#endif
