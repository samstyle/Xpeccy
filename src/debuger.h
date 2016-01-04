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
#include <QMenu>
#include <QTableWidget>

class xTableWidget : public QTableWidget {
	Q_OBJECT
	public:
		xTableWidget(QWidget*);
	protected:
		void keyPressEvent(QKeyEvent*);
};

#include "ui_dumpdial.h"
#include "ui_openDump.h"
#include "ui_debuger.h"
#include "libxpeccy/spectrum.h"

struct xLabel {
	int bank;
	int adr;
	QString name;
};

#define XTYPE_NONE -1
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
		void start(Computer*);
		void stop();
		bool fillAll();

		QList<xLabel> labels;
	signals:
		void closed();
	private:
		unsigned trace:1;
		unsigned showLabels:1;

		Ui::Debuger ui;
		QPoint winPos;
		QImage scrImg;

		Computer* comp;
		bool block;
		long tCount;

		QDialog* dumpwin;
		Ui::DumpDial dui;
		QByteArray getDumpData();

		QDialog* openDumpDialog;
		Ui::oDumpDial oui;
		QString dumpPath;

		QMenu* bpMenu;
		unsigned short bpAdr;
		void doBreakPoint(unsigned short);
		int getAdr();
		void switchBP(unsigned char);

		xLabel* findLabel(int);

		unsigned short disasmAdr;
		unsigned short dumpAdr;

		void fillZ80();
		void fillFlags();
		void fillMem();
		void fillDump();
		int fillDisasm();
		void fillStack();
		void fillFDC();
		void fillRZX();

		void fillBrkTable();

		unsigned short getPrevAdr(unsigned short);
		void scrollDown();
		void scrollUp();

	private slots:
		void setZ80();
		void setFlags();
		void updateScreen();

		void dasmEdited(int, int);
		void dumpEdited(int, int);

		void putBreakPoint();
		void chaBreakPoint();
		void goToBrk(QModelIndex);

		void doOpenDump();
		void chDumpFile();
		void dmpStartOpen();
		void loadDump();

		void doStep();

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
