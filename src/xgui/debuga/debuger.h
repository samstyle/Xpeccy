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

#include "xgui.h"

#include "dbg_dump.h"
#include "dbg_disasm.h"

#include "libxpeccy/spectrum.h"
#include "dbg_sprscan.h"
#include "dbg_memfill.h"
#include "dbg_finder.h"
#include "dbg_brkpoints.h"

#include "ui_dumpdial.h"
#include "ui_openDump.h"
#include "ui_debuger.h"

enum {
	XTYPE_NONE = -1,
	XTYPE_ADR = 0,
	XTYPE_LABEL,
	XTYPE_DUMP,
	XTYPE_BYTE,
};

enum {
	DMP_MEM = 1,
	DMP_REG
};

class xItemDelegate : public QItemDelegate {
	public:
		xItemDelegate(int);
		int type;
		QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;

};

class DebugWin : public QDialog {
	Q_OBJECT
	public:
		DebugWin(QWidget* = NULL);
		~DebugWin();

		void reject();
		void stop();

	signals:
		void closed();
		void wannaKeys();
		void wannaWutch();
		void wannaOptions(xProfile*);
		void needStep();
	public slots:
		void start(Computer*);
		bool fillAll();
		void onPrfChange(xProfile*);
		void chaPal();
	private:
		unsigned block:1;
		int tabMode;
		// tracer
		unsigned trace:1;
		int traceType;
		int traceAdr;

		Ui::Debuger ui;
		QPoint winPos;
		QImage scrImg;

		QMap<int, QList<QPair<QIcon, QWidget*> > > tablist;

		Computer* comp;
		long tCount;

		QDialog* dumpwin;
		Ui::DumpDial dui;
		QByteArray getDumpData();

		QDialog* openDumpDialog;
		Ui::oDumpDial oui;
		QString dumpPath;

		xMemFiller* memFiller;
		xMemFinder* memFinder;
		MemViewer* memViewer;
		xBrkManager* brkManager;

		QMenu* cellMenu;
		unsigned short bpAdr;
		void doBreakPoint(unsigned short);
		int getAdr();

		void fillCPU();
		void fillFlags();
		void fillMem();
		void fillStack();
		void fillFDC();
		void fillAY();
		void fillTape();

		void setFlagNames(const char*);
		void chLayout();

		unsigned short getPrevAdr(unsigned short);

	private slots:
		void setShowLabels(bool);
		void setShowSegment(bool);
		void chDumpView();
		void setDasmMode();
		void setDumpCP();

//		void loadMap();
//		void saveMap();
		void saveDasm();

		void dbgLLab();
		void dbgSLab();

		void mapClear();
		void mapAuto();

		int fillDisasm();
		void fillDump();
		void fillGBoy();
		void drawNes();
		void regClick(QMouseEvent*);
		void reload();

		void setCPU();
		void setFlags();
		void updateScreen();
		void dumpChadr(QModelIndex);

		void addBrk();
		void editBrk();
		void delBrk();
		void confirmBrk(xBrkPoint, xBrkPoint);
		void goToBrk(QModelIndex);
		void openBrk();
		void saveBrk(QString = QString());

		void putBreakPoint();
		void chaCellProperty(QAction*);

		void doMemView();
		void doFill();

		void doFind();
		void onFound(int);

		void doTrace(QAction*);
		void doTraceHere();
		void doStep();

		void doOpenDump();
		void doSaveDump();
		void loadDump();
		void chDumpFile();
		void dmpStartOpen();
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
		void keyReleaseEvent(QKeyEvent*);
};

#endif
