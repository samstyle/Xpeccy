#pragma once

#include <QDialog>
#include <QMainWindow>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include <QItemDelegate>
#include <QMenu>
#include <QTableWidget>
#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
#include <QRegularExpressionValidator>
#else
#include <QRegExpValidator>
#endif

#include "../labelist.h"
#include "libxpeccy/spectrum.h"
#include "dbg_widgets.h"

#include "ui_dumpdial.h"
#include "ui_openDump.h"
// #include "ui_debuger.h"

#include "ui_form_cpu.h"
#include "ui_form_disasm.h"
#include "ui_form_misc.h"

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

class DebugWin : public QMainWindow {
	Q_OBJECT
	public:
		DebugWin(QWidget* = NULL);
		~DebugWin();

//		void reject();
		void stop();

	signals:
		void closed();
		void wannaKeys();
		void wannaWutch();
		void wannaOptions();
		void needStep();
	public slots:
		void start();
		void onPrfChange();
		void setScrAtr(int, int);
		void updateStyle();
	private:
		unsigned block:1;
		int tabMode;
		// tracer
		unsigned trace:1;
		int traceType;
		int traceAdr;
		long tCount;

		QImage scrImg;
		QMap<int, QList<tabDSC> > tablist;

		// Ui::Debuger ui;
		QWidget* cw;
		// widgets
		Ui::CPUWidget ui_cpu;
		Ui::DisasmWidget ui_asm;
		Ui::FormDbgMisc ui_misc;
		xDumpWidget* wid_dump;
		xRDumpWidget* wid_rdump;
		xDiskDumpWidget* wid_disk_dump;
		xCmosDumpWidget* wid_cmos_dump;
		xVMemDumpWidget* wid_vmem_dump;
		xZXScrWidget* wid_zxscr;
		xDmaWidget* wid_dma;
		xPitWidget* wid_pit;
		xPicWidget* wid_pic;
		xVgaWidget* wid_vga;
		xPS2Widget* wid_ps2;
		xAYWidget* wid_ay;
		xTapeWidget* wid_tape;
		xFDDWidget* wid_fdd;
		xBreakWidget* wid_brk;
		xGameboyWidget* wid_gb;
		xPPUWidget* wid_ppu;
		// apu (future)
		xCiaWidget* wid_cia;
		xVicWidget* wid_vic;
		xMMapWidget* wid_mmap;
		QList<void*> dockWidgets;

		QList<xLabel*> dbgRegLabs;
		QList<xHexSpin*> dbgRegEdit;
		QList<QCheckBox*> dbgRegBits;

		QList<QLabel*> dbgFlagLabs;
		QList<QCheckBox*> dbgFlagBox;
		QButtonGroup* flagrp;

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
		void fillPorts();

		void setFlagNames(const char*);
		void chLayout();

	private slots:
		void setShowLabels(bool);
		void setShowSegment(bool);
		void setRomWriteable(bool);
		void resetTCount();

		bool fillAll();
		void fillNotCPU();
		void doStep();

		void saveDasm();
		void d_remap(int, int, int);
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
		void regClick(QMouseEvent*);
		void reload();

		void setCPU();
		void setFlags();

		void brkRequest(int, int, int);
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
		void saveVRam();
	protected:
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
		void resizeEvent(QResizeEvent*);
		void moveEvent(QMoveEvent*);
		void closeEvent(QCloseEvent*);
		void customEvent(QEvent*);
};

int loadDUMP(Computer*, const char*, int);
