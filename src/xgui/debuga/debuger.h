#pragma once

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
#include <QRegExpValidator>

#include "../xgui.h"
#include "../labelist.h"

#include "dbg_dump.h"
#include "dbg_disasm.h"

#include "libxpeccy/spectrum.h"
#include "dbg_sprscan.h"
#include "dbg_memfill.h"
#include "dbg_finder.h"
#include "dbg_brkpoints.h"
#include "dbg_diskdump.h"
#include "dbg_vmem_dump.h"
#include "dbg_pit.h"
#include "dbg_vga_regs.h"

#include "ui_dumpdial.h"
#include "ui_openDump.h"
#include "ui_debuger.h"

enum {
	XTYPE_NONE = -1,
	XTYPE_ADR = 0,
	XTYPE_LABEL,
	XTYPE_DUMP,
	XTYPE_BYTE,
	XTYPE_OCTWRD,
};

enum {
	DMP_MEM = 1,
	DMP_REG
};

enum {
	DBG_EVENT_STEP = QEvent::User
};

typedef struct {
	QIcon icon;
	QString name;
	QWidget* wid;
} tabDSC;

class xItemDelegate : public QItemDelegate {
	public:
		xItemDelegate(int);
		int type;
		// QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
	private:
		QRegExpValidator vld;
		QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
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
		bool fillNotCPU();
		void fillTabs();
		void onPrfChange(xProfile*);
		void chaPal();
		void doStep();
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

		QMap<int, QList<tabDSC> > tablist;

		Computer* comp;
		long tCount;

		QList<xLabel*> dbgRegLabs;
		QList<xHexSpin*> dbgRegEdit;

		QList<QLabel*> dbgFlagLabs;
		QList<QCheckBox*> dbgFlagBox;
		QButtonGroup* flagrp;

		MemPage mem_map[256];

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
		xLabeList* labswin;

		QMenu* cellMenu;
		unsigned short bpAdr;
		void doBreakPoint(unsigned short);
		int getAdr();

		xItemDelegate* xid_none;
		xItemDelegate* xid_byte;
		xItemDelegate* xid_labl;
		xItemDelegate* xid_octw;
		xItemDelegate* xid_dump;

		void fillCPU();
		void fillFlags(const char*);
		void fillMem();
		void fillStack();
		void fillFDC();
		void fillAY();
		void fillTape();

		void setFlagNames(const char*);
		void chLayout();

	private slots:
		void setShowLabels(bool);
		void setShowSegment(bool);
		void setRomWriteable(bool);
		void chDumpView();
		void setDumpCP();
		void resetTCount();

		void saveDasm();
		void remapMem();
		void save_mem_map();
		void rest_mem_map();

		void dbgLLab();
		void dbgSLab();
		void jumpToLabel(QString);

		void saveMap();
		void loadMap();
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
		void dumpChadr(int);

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
		void stopTrace();

		void doOpenDump();
//		void doSaveDump();
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
		void customEvent(QEvent*);
};

int loadDUMP(Computer*, const char*, int);
