#include "xcore/xcore.h"

#include <stdio.h>

#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTextCodec>

#include "debuger.h"
#include "dbg_sprscan.h"
#include "filer.h"
#include "xgui.h"

QColor colPC(32,200,32);	// pc
QColor colBRK(200,128,128);	// breakpoint
QColor colBG0(255,255,255);	// background
QColor colBG1(230,230,230);	// background alternative
QColor colSEL(128,255,128);	// background selected

unsigned short dumpAdr = 0;
unsigned short disasmAdr = 0;

int blockStart = -1;
int blockEnd = -1;

QMap<QString, xAdr> labels;

int getRFIData(QComboBox*);
int dasmSome(Computer*, unsigned short, dasmData&);

// trace type
enum {
	DBG_TRACE_ALL = 0x100,
	DBG_TRACE_INT,
	DBG_TRACE_HERE,
	DBG_TRACE_LOG
};

enum {
	NES_SCR_OFF = 0x00,
	NES_SCR_0,
	NES_SCR_1,
	NES_SCR_2,
	NES_SCR_3,
	NES_SCR_ALL
};

enum {
	NES_TILE_0000 = 0x0000,
	NES_TILE_1000 = 0x1000
};

typedef struct {
	QLabel* name;
	QLineEdit* edit;
} dbgRegPlace;

void DebugWin::start(Computer* c) {
	// onStart = 1;
	comp = c;
	blockStart = -1;
	blockEnd = -1;
	chLayout();
	if (getRFIData(ui.cbDasmMode) == XVIEW_CPU)
		disasmAdr = comp->cpu->pc;
	fillAll();
	updateScreen();
	if (!comp->vid->tail)
		vidDarkTail(comp->vid);
	ui.tabsPanel->setTabEnabled(ui.tabsPanel->indexOf(ui.gbTab), comp->hw->id == HW_GBC);
	ui.tabsPanel->setTabEnabled(ui.tabsPanel->indexOf(ui.nesTab), comp->hw->id == HW_NES);

	this->move(winPos);
	// ui.dasmTable->setFocus();
	comp->vid->debug = 1;
	comp->debug = 1;
	comp->brk = 0;

	show();

	int wd = (ui.dasmTable->height() - 2) / ui.dasmTable->rows();
	ui.dasmTable->verticalHeader()->setDefaultSectionSize(wd);

	wd = (ui.dumpTable->height() - 2) / ui.dumpTable->rows();
	ui.dumpTable->verticalHeader()->setDefaultSectionSize(wd);

	if (memViewer->vis) {
		memViewer->move(memViewer->winPos);
		memViewer->show();
		memViewer->fillImage();
	}

	chDumpView();
	activateWindow();
}

void DebugWin::stop() {
	comp->debug = 0;
	comp->vid->debug = 0;
	comp->maping = ui.actMaping->isChecked() ? 1 : 0;
	tCount = comp->tickCount;
	winPos = pos();
	trace = 0;

	memViewer->vis = memViewer->isVisible() ? 1 : 0;
	memViewer->winPos = memViewer->pos();

	memViewer->hide();
	hide();
	emit closed();
}

void DebugWin::reject() {stop();}

xItemDelegate::xItemDelegate(int t) {type = t;}

QWidget* xItemDelegate::createEditor(QWidget* par, const QStyleOptionViewItem&, const QModelIndex&) const {
	QLineEdit* edt = new QLineEdit(par);
	switch (type) {
		case XTYPE_NONE: delete(edt); edt = NULL; break;
		case XTYPE_ADR: edt->setInputMask("Hhhh"); break;
		case XTYPE_LABEL: break;
		case XTYPE_DUMP: edt->setInputMask("Hhhhhhhhhh"); break;
		case XTYPE_BYTE: edt->setInputMask("Hh"); break;
	}
	return edt;
}

int dmpmrd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = memRd(comp->mem, adr) & 0xff;
	res |= getBrk(comp, adr) << 8;
	return res;
}

void dmpmwr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	memWr(comp->mem, adr, val);
}

static xLabel* dbgRegLabs[16];
static xHexSpin* dbgRegEdit[16];

DebugWin::DebugWin(QWidget* par):QDialog(par) {
	int i;

	ui.setupUi(this);

	xLabel* arrl[16] = {
		ui.labReg00, ui.labReg01, ui.labReg02, ui.labReg03, ui.labReg04,
		ui.labReg05, ui.labReg06, ui.labReg07, ui.labReg08, ui.labReg09,
		ui.labReg10, ui.labReg11, ui.labReg12, ui.labReg13, ui.labReg14,
		NULL
	};

	xHexSpin* arre[16] = {
		ui.editReg00, ui.editReg01, ui.editReg02, ui.editReg03, ui.editReg04,
		ui.editReg05, ui.editReg06, ui.editReg07, ui.editReg08, ui.editReg09,
		ui.editReg10, ui.editReg11, ui.editReg12, ui.editReg13, ui.editReg14,
		NULL
	};
	for (i = 0; i < 16; i++) {
		dbgRegLabs[i] = arrl[i];
		dbgRegEdit[i] = arre[i];
		if (arrl[i]) {
			arrl[i]->id = i;
			connect(arrl[i], SIGNAL(clicked(QMouseEvent*)), this, SLOT(regClick(QMouseEvent*)));
			dbgRegEdit[i]->setXFlag(XHS_BGR | XHS_DEC);
		}
	}

	ui.dumpTable->setComp(&comp);
	ui.dasmTable->setComp(&comp);

	setFont(QFont("://DejaVuSansMono.ttf",10));

	conf.dbg.labels = 1;
	conf.dbg.segment = 0;
	ui.actShowLabels->setChecked(conf.dbg.labels);
	ui.actShowSeg->setChecked(conf.dbg.segment);
// actions data
	ui.actFetch->setData(MEM_BRK_FETCH);
	ui.actRead->setData(MEM_BRK_RD);
	ui.actWrite->setData(MEM_BRK_WR);

	ui.actViewOpcode->setData(DBG_VIEW_EXEC);
	ui.actViewByte->setData(DBG_VIEW_BYTE);
	ui.actViewWord->setData(DBG_VIEW_WORD);
	ui.actViewAddr->setData(DBG_VIEW_ADDR);
	ui.actViewText->setData(DBG_VIEW_TEXT);

	ui.actTrace->setData(DBG_TRACE_ALL);
	ui.actTraceHere->setData(DBG_TRACE_HERE);
	ui.actTraceINT->setData(DBG_TRACE_INT);
	ui.actTraceLog->setData(DBG_TRACE_LOG);

	ui.dasmTable->setFocus();

// disasm table
	ui.dasmTable->setColumnWidth(0,100);
	ui.dasmTable->setColumnWidth(1,85);
	ui.dasmTable->setColumnWidth(2,130);
	ui.dasmTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_LABEL));
	ui.dasmTable->setItemDelegateForColumn(1, new xItemDelegate(XTYPE_DUMP));

	ui.cbDasmMode->addItem("CPU", XVIEW_CPU);
	ui.cbDasmMode->addItem("RAM", XVIEW_RAM);
	ui.cbDasmMode->addItem("ROM", XVIEW_ROM);

	connect(ui.cbDasmMode, SIGNAL(currentIndexChanged(int)),this,SLOT(setDasmMode()));
	connect(ui.sbDasmPage, SIGNAL(valueChanged(int)),this,SLOT(setDasmMode()));

// actions
	ui.tbBreak->addAction(ui.actFetch);
	ui.tbBreak->addAction(ui.actRead);
	ui.tbBreak->addAction(ui.actWrite);

	ui.tbView->addAction(ui.actViewOpcode);
	ui.tbView->addAction(ui.actViewByte);
	ui.tbView->addAction(ui.actViewText);
	ui.tbView->addAction(ui.actViewWord);
	ui.tbView->addAction(ui.actViewAddr);

	ui.tbSaveDasm->addAction(ui.actDisasm);
	ui.tbSaveDasm->addAction(ui.actLoadDump);
	ui.tbSaveDasm->addAction(ui.actLoadMap);
	ui.tbSaveDasm->addAction(ui.actLoadLabels);
	ui.tbSaveDasm->addAction(ui.actSaveDump);
	ui.tbSaveDasm->addAction(ui.actSaveMap);
	ui.tbSaveDasm->addAction(ui.actSaveLabels);

	ui.tbTrace->addAction(ui.actTrace);
	ui.tbTrace->addAction(ui.actTraceHere);
	ui.tbTrace->addAction(ui.actTraceINT);
#ifdef ISDEBUG
	ui.tbTrace->addAction(ui.actTraceLog);
#endif

	ui.tbTool->addAction(ui.actSearch);
	ui.tbTool->addAction(ui.actFill);
	ui.tbTool->addAction(ui.actSprScan);
	ui.tbTool->addAction(ui.actShowKeys);

	ui.tbDbgOpt->addAction(ui.actShowLabels);
	ui.tbDbgOpt->addAction(ui.actHideAddr);
	ui.tbDbgOpt->addAction(ui.actShowSeg);
	ui.tbDbgOpt->addAction(ui.actMaping);
	ui.tbDbgOpt->addAction(ui.actMapingClear);

// connections
	connect(this,SIGNAL(needStep()),this,SLOT(doStep()));

	connect(ui.actMapingClear,SIGNAL(triggered()),this,SLOT(mapClear()));

	connect(ui.dasmTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDump()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(updateScreen()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),ui.bpList,SLOT(update()));
	connect(ui.dasmTable,SIGNAL(rqRefillAll()),this,SLOT(fillAll()));

	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(fillDump()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),ui.bpList,SLOT(update()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(updateScreen()));
	connect(ui.dumpTable->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(dumpChadr(QModelIndex)));

	connect(ui.bpList,SIGNAL(rqDisasm()),this,SLOT(fillDisasm()));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDisasm()));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDump()));

	connect(ui.actSearch,SIGNAL(triggered(bool)),this,SLOT(doFind()));
	connect(ui.actFill,SIGNAL(triggered(bool)),this,SLOT(doFill()));
	connect(ui.actSprScan,SIGNAL(triggered(bool)),this,SLOT(doMemView()));
	connect(ui.actShowKeys,SIGNAL(triggered(bool)),this,SIGNAL(wannaKeys()));

	connect(ui.actShowLabels,SIGNAL(toggled(bool)),this,SLOT(setShowLabels(bool)));
	connect(ui.actHideAddr,SIGNAL(toggled(bool)),this,SLOT(fillDisasm()));
	connect(ui.actShowSeg,SIGNAL(toggled(bool)),this,SLOT(setShowSegment(bool)));

	connect(ui.tbView, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui.tbBreak, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui.tbTrace, SIGNAL(triggered(QAction*)),this,SLOT(doTrace(QAction*)));

	connect(ui.actLoadDump, SIGNAL(triggered(bool)),this,SLOT(doOpenDump()));
	connect(ui.actSaveDump, SIGNAL(triggered(bool)),this,SLOT(doSaveDump()));
	connect(ui.actLoadLabels, SIGNAL(triggered(bool)),this,SLOT(loadLabels()));
	connect(ui.actSaveLabels, SIGNAL(triggered(bool)),this,SLOT(saveLabels()));
	connect(ui.actLoadMap, SIGNAL(triggered(bool)),this,SLOT(loadMap()));
	connect(ui.actSaveMap, SIGNAL(triggered(bool)),this,SLOT(saveMap()));
	connect(ui.actDisasm, SIGNAL(triggered(bool)),this,SLOT(saveDasm()));

// dump table

	ui.cbCodePage->addItem("WIN", XCP_1251);
	ui.cbCodePage->addItem("DOS", XCP_866);
	ui.cbCodePage->addItem("KOI8R", XCP_KOI8R);

	ui.cbDumpView->addItem("CPU", XVIEW_CPU);
	ui.cbDumpView->addItem("RAM", XVIEW_RAM);
	ui.cbDumpView->addItem("ROM", XVIEW_ROM);

	ui.dumpTable->setColumnWidth(0,60);
	ui.dumpTable->setItemDelegate(new xItemDelegate(XTYPE_BYTE));
	ui.dumpTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_ADR));
	ui.dumpTable->setItemDelegateForColumn(9, new xItemDelegate(XTYPE_NONE));

	connect(ui.dumpTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	connect(ui.cbCodePage, SIGNAL(currentIndexChanged(int)), this, SLOT(setDumpCP()));
	connect(ui.cbDumpView, SIGNAL(currentIndexChanged(int)), this, SLOT(chDumpView()));
	connect(ui.sbDumpPage, SIGNAL(valueChanged(int)), this, SLOT(chDumpView()));

	ui.cbDumpView->setCurrentIndex(0);

// registers
	connect(ui.editReg00, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg01, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg02, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg03, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg04, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg05, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg06, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg07, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg08, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg09, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg10, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg11, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg12, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg13, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	connect(ui.editReg14, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));

	connect(ui.boxIM,SIGNAL(valueChanged(int)),this,SLOT(setCPU()));
	connect(ui.flagIFF1,SIGNAL(stateChanged(int)),this,SLOT(setCPU()));
	connect(ui.flagIFF2,SIGNAL(stateChanged(int)),this,SLOT(setCPU()));
	connect(ui.flagGroup,SIGNAL(buttonClicked(int)),this,SLOT(setFlags()));
// infoslots
	scrImg = QImage(256, 192, QImage::Format_RGB888);
	// ui.scrLabel->setFixedSize(256,192);
	connect(ui.sbScrBank,SIGNAL(valueChanged(int)),this,SLOT(updateScreen()));
	connect(ui.leScrAdr,SIGNAL(textChanged(QString)),this,SLOT(updateScreen()));
	connect(ui.cbScrAtr,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrPix,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrGrid,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));

	connect(ui.tbAddBrk, SIGNAL(clicked()), this, SLOT(addBrk()));
	connect(ui.tbEditBrk, SIGNAL(clicked()), this, SLOT(editBrk()));
	connect(ui.tbDelBrk, SIGNAL(clicked()), this, SLOT(delBrk()));
	connect(ui.tbBrkOpen, SIGNAL(clicked()), this, SLOT(openBrk()));
	connect(ui.tbBrkSave, SIGNAL(clicked()), this, SLOT(saveBrk()));
	connect(ui.bpList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(goToBrk(QModelIndex)));
// gb tab
	connect(ui.gbModeGroup, SIGNAL(buttonClicked(int)), this, SLOT(fillGBoy()));
	connect(ui.sbTileset, SIGNAL(valueChanged(int)), this, SLOT(fillGBoy()));
	connect(ui.sbTilemap, SIGNAL(valueChanged(int)), this, SLOT(fillGBoy()));

	// setFixedSize(size());
	setFixedHeight(size().height());
	block = 0;
	disasmAdr = 0;
	dumpAdr = 0;
	tCount = 0;
	trace = 0;
// nes tab
	ui.nesScrType->addItem(trUtf8("BG off"), NES_SCR_OFF);
	ui.nesScrType->addItem(trUtf8("BG scr 0"), NES_SCR_0);
	ui.nesScrType->addItem(trUtf8("BG scr 1"), NES_SCR_1);
	ui.nesScrType->addItem(trUtf8("BG scr 2"), NES_SCR_2);
	ui.nesScrType->addItem(trUtf8("BG scr 3"), NES_SCR_3);
	ui.nesScrType->addItem(trUtf8("All in 1"), NES_SCR_ALL);

	ui.nesBGTileset->addItem(trUtf8("Tiles #0000"), NES_TILE_0000);
	ui.nesBGTileset->addItem(trUtf8("Tiles #1000"), NES_TILE_1000);

	connect(ui.nesScrType,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));
	connect(ui.nesBGTileset,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));

// subwindows
	dumpwin = new QDialog(this);
	dui.setupUi(dumpwin);
	dui.tbSave->addAction(dui.aSaveBin);
	dui.tbSave->addAction(dui.aSaveHobeta);
	dui.tbSave->addAction(dui.aSaveToA);
	dui.tbSave->addAction(dui.aSaveToB);
	dui.tbSave->addAction(dui.aSaveToC);
	dui.tbSave->addAction(dui.aSaveToD);

	connect(dui.aSaveBin,SIGNAL(triggered()),this,SLOT(saveDumpBin()));
	connect(dui.aSaveHobeta,SIGNAL(triggered()),this,SLOT(saveDumpHobeta()));
	connect(dui.aSaveToA,SIGNAL(triggered()),this,SLOT(saveDumpToA()));
	connect(dui.aSaveToB,SIGNAL(triggered()),this,SLOT(saveDumpToB()));
	connect(dui.aSaveToC,SIGNAL(triggered()),this,SLOT(saveDumpToC()));
	connect(dui.aSaveToD,SIGNAL(triggered()),this,SLOT(saveDumpToD()));
	connect(dui.leStart,SIGNAL(textChanged(QString)),this,SLOT(dmpLimChanged()));
	connect(dui.leEnd,SIGNAL(textChanged(QString)),this,SLOT(dmpLimChanged()));
	connect(dui.leLen,SIGNAL(textChanged(QString)),this,SLOT(dmpLenChanged()));

	openDumpDialog = new QDialog(this);
	oui.setupUi(openDumpDialog);
	connect(oui.tbFile,SIGNAL(clicked()),this,SLOT(chDumpFile()));
	connect(oui.leStart,SIGNAL(textChanged(QString)),this,SLOT(dmpStartOpen()));
	connect(oui.butOk,SIGNAL(clicked()), this, SLOT(loadDump()));

	memViewer = new MemViewer(this);

	memFinder = new xMemFinder(this);
	connect(memFinder, SIGNAL(patFound(int)), this, SLOT(onFound(int)));

	memFiller = new xMemFiller(this);
	connect(memFiller, SIGNAL(rqRefill()),this,SLOT(fillAll()));

	brkManager = new xBrkManager(this);
	connect(brkManager, SIGNAL(completed(xBrkPoint, xBrkPoint)), this, SLOT(confirmBrk(xBrkPoint, xBrkPoint)));

// context menu
	cellMenu = new QMenu(this);
	QMenu* bpMenu = new QMenu("Breakpoints");
	bpMenu->setIcon(QIcon(":/images/stop.png"));
	cellMenu->addMenu(bpMenu);
	bpMenu->addAction(ui.actFetch);
	bpMenu->addAction(ui.actRead);
	bpMenu->addAction(ui.actWrite);
	QMenu* viewMenu = new QMenu("View");
	viewMenu->setIcon(QIcon(":/images/bars.png"));
	cellMenu->addMenu(viewMenu);
	viewMenu->addAction(ui.actViewOpcode);
	viewMenu->addAction(ui.actViewByte);
	viewMenu->addAction(ui.actViewText);
	viewMenu->addAction(ui.actViewWord);
	viewMenu->addAction(ui.actViewAddr);
	cellMenu->addSeparator();
	cellMenu->addAction(ui.actTraceHere);
	cellMenu->addAction(ui.actShowLabels);
	// NOTE: actions already connected to slots by main menu. no need to double it here
}

DebugWin::~DebugWin() {
	delete(dumpwin);
	delete(openDumpDialog);
	delete(memViewer);
	delete(memFiller);
	delete(memFinder);
}

void DebugWin::setShowLabels(bool f) {
	conf.dbg.labels = f ? 1 : 0;
	fillDisasm();
}

void DebugWin::setShowSegment(bool f) {
	conf.dbg.segment = f ? 1 : 0;
	fillDisasm();
}

void DebugWin::setDumpCP() {
	int cp = getRFIData(ui.cbCodePage);
	ui.dumpTable->setCodePage(cp);
	fillDump();
}

void DebugWin::chDumpView() {
	int mode = getRFIData(ui.cbDumpView);
	int page = ui.sbDumpPage->value();
	ui.sbDumpPage->setDisabled(mode == XVIEW_CPU);
	ui.dumpTable->setMode(mode, page);
}

void DebugWin::setDasmMode() {
	int mode = getRFIData(ui.cbDasmMode);
	int page = ui.sbDasmPage->value();
	ui.sbDasmPage->setDisabled(mode == XVIEW_CPU);
	ui.dasmTable->setMode(mode, page);
}

static QFile logfile;

void DebugWin::doStep() {
#ifdef ISDEBUG
	QString str;
	if (traceType == DBG_TRACE_LOG) {
		str = gethexword(comp->cpu->pc).append(" ");
		str.append(QString("A:%0 ").arg(gethexbyte(comp->cpu->a)));
		str.append(QString("X:%0 ").arg(gethexbyte(comp->cpu->lx)));
		str.append(QString("Y:%0 ").arg(gethexbyte(comp->cpu->ly)));
		str.append(QString("P:%0 ").arg(gethexbyte(comp->cpu->f)));
		str.append(QString("SP:%0 ").arg(gethexbyte(comp->cpu->lsp)));
		str.append(QString("CYC:%0").arg(comp->vid->ray.x));
		logfile.write(str.toUtf8());
		logfile.write("\r\n");
		if (comp->cpu->pc == 0xc66e)
			trace = 0;
	}
#endif
	do {
		tCount = comp->tickCount;
		compExec(comp);
		if (!fillAll()) {
			disasmAdr = comp->cpu->pc;
			fillDisasm();
		}
		switch(traceType) {
			case DBG_TRACE_INT:
				if (comp->cpu->intrq & comp->cpu->inten)
					trace = 0;
				break;
			case DBG_TRACE_HERE:
				if (comp->cpu->pc == traceAdr)
					trace = 0;
				break;
		}
		QApplication::processEvents();
	} while(trace);
//	if (trace) {
		//emit needStep();
		//QTimer::singleShot(1,this,SLOT(doStep()));
		//QApplication::processEvents();
//	} else {
	ui.tbTrace->setEnabled(true);
	if (logfile.isOpen()) logfile.close();
//	}
}

void DebugWin::doTraceHere() {
	doTrace(ui.actTraceHere);
}

void DebugWin::doTrace(QAction* act) {
	if (trace) return;

	traceType = act->data().toInt();

	if (traceType == DBG_TRACE_LOG) {
		QString path = QFileDialog::getSaveFileName(this, "Log file");
		if (path.isEmpty()) return;
		logfile.setFileName(path);
		if (!logfile.open(QFile::WriteOnly)) return;
	}

	trace = 1;
	traceAdr = getAdr();
	ui.tbTrace->setEnabled(false);
	doStep();
}

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	if (trace) {
		trace = 0;
		return;
	}

//	if (ev->isAutoRepeat() && onStart) return;
//	onStart = 0;

	int i;
	unsigned short pc = comp->cpu->pc;
	unsigned char* ptr;
	int offset = (ui.dumpTable->rows() - 1) << 3;
	// QString com;
	int row;
	int pos;
	int adr;
	int len;
	dasmData drow;
	QModelIndex idx;
	switch(ev->modifiers()) {
		case Qt::ControlModifier:
			switch(ev->key()) {
				case Qt::Key_F:
					doFind();
					break;
				case Qt::Key_S:
					doSaveDump();
					break;
				case Qt::Key_O:
					doOpenDump();
					break;
				case Qt::Key_T:
					doTrace(ui.actTrace);
					break;
				case Qt::Key_L:
					ui.actShowLabels->setChecked(!conf.dbg.labels);
					break;
			}
			break;
		case Qt::AltModifier:
			switch (ev->key()) {
				case Qt::Key_K:
					emit wannaKeys();
					break;
			}
			break;
		default:
			switch(ev->key()) {
				case Qt::Key_Escape:
					if (!ev->isAutoRepeat()) stop();
					break;
				case Qt::Key_Home:
					disasmAdr = pc;
					fillDisasm();
					break;
				case Qt::Key_End:
					// if (!ui.dasmTable->hasFocus()) break;
					adr = getAdr();
					if (adr < 0) break;
					comp->cpu->pc = adr & 0xffff;
					fillCPU();
					fillDisasm();
					break;
				case Qt::Key_F2:
					// switchBP(MEM_BRK_FETCH);
					break;
				case Qt::Key_F4:
					idx = ui.dasmTable->currentIndex();
					if (!idx.isValid()) break;
					row = idx.row();
					if (row < 0) break;
					pos = ui.dasmTable->getData(row, 2, Qt::UserRole).toInt();
					if (pos < 0) break;
					jumpHistory.append(disasmAdr);
					if (jumpHistory.size() > 64)
						jumpHistory.takeFirst();
					disasmAdr = pos;
					fillDisasm();
					break;
				case Qt::Key_F5:
					if (jumpHistory.size() == 0) break;
					disasmAdr = jumpHistory.takeLast();
					fillDisasm();
					break;
				case Qt::Key_PageUp:
					if (ui.dumpTable->hasFocus()) {
						dumpAdr = (dumpAdr - offset) & 0xffff;
						fillDump();
					} else if (ui.dasmTable->hasFocus()) {
						for (i = 0; i < ui.dasmTable->rows() - 1; i++) {
							disasmAdr = getPrevAdr(disasmAdr);
						}
						fillDisasm();
					}
					break;
				case Qt::Key_PageDown:
					if (ui.dumpTable->hasFocus()) {
						dumpAdr = (dumpAdr + offset) & 0xffff;
						fillDump();
					} else if (ui.dasmTable->hasFocus()) {
						disasmAdr = ui.dasmTable->getData(ui.dasmTable->rows() - 1, 0, Qt::UserRole).toInt();
						fillDisasm();
					}
					break;
				case Qt::Key_F3:
					//loadFile(comp,"",FT_ALL,-1);
					load_file(comp, NULL, FG_ALL, -1);
					disasmAdr = comp->cpu->pc;
					fillAll();
					break;
				case Qt::Key_F7:
					doStep();
					break;
				case Qt::Key_F8:
					// if (!ui.dasmTable->hasFocus()) break;
					len = dasmSome(comp, comp->cpu->pc, drow);
					if (drow.oflag & OF_SKIPABLE) {
						ptr = getBrkPtr(comp, (comp->cpu->pc + len) & 0xffff);
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else {
						doStep();
					}
					break;
				case Qt::Key_F9:
					if (!ui.dasmTable->hasFocus()) break;
					idx = ui.dasmTable->currentIndex();
					i = ui.dasmTable->getData(idx.row(), 0, Qt::UserRole).toInt() & 0xffff;
					// qDebug() << idx << i;
					ptr = getBrkPtr(comp, i);
					*ptr |= MEM_BRK_TFETCH;
					stop();
					break;
				case Qt::Key_F12:
					rzxStop(comp);
					compReset(comp, RES_DEFAULT);
					if (!fillAll()) {
						disasmAdr = comp->cpu->pc;
						fillDisasm();
					}
					break;
			}
		break;
	}
}

void setSignal(QLabel* lab, int on) {
	QString col(on ? "160,255,160" : "255,160,160");
	// QString bld(on ? "font-weight:bold" : "");
	lab->setStyleSheet(QString("background-color: rgb(%0)").arg(col));
}

QString getAYmix(aymChan* ch) {
	QString res = ch->ten ? "T" : "-";
	res += ch->nen ? "N" : "-";
	res += ch->een ? "E" : "-";
	return res;
}

void DebugWin::fillAY() {
	aymChip* chp = comp->ts->chipA;
	ui.leToneA->setText(gethexword(((chp->reg[0] << 8) | chp->reg[1]) & 0x0fff));
	ui.leToneB->setText(gethexword(((chp->reg[2] << 8) | chp->reg[3]) & 0x0fff));
	ui.leToneC->setText(gethexword(((chp->reg[4] << 8) | chp->reg[5]) & 0x0fff));
	ui.leVolA->setText(gethexbyte(chp->chanA.vol));
	ui.leVolB->setText(gethexbyte(chp->chanB.vol));
	ui.leVolC->setText(gethexbyte(chp->chanC.vol));
	ui.leMixA->setText(getAYmix(&chp->chanA));
	ui.leMixB->setText(getAYmix(&chp->chanB));
	ui.leMixC->setText(getAYmix(&chp->chanC));
	ui.leToneN->setText(gethexbyte(chp->reg[6]));
	ui.leEnvTone->setText(gethexword((chp->reg[11] << 8) | chp->reg[12]));
	ui.leEnvForm->setText(gethexbyte(chp->reg[13]));
	ui.leVolE->setText(gethexbyte(chp->chanE.vol));
	ui.labLevA->setText(chp->chanA.lev ? "1" : "0");
	ui.labLevB->setText(chp->chanB.lev ? "1" : "0");
	ui.labLevC->setText(chp->chanC.lev ? "1" : "0");
	ui.labLevN->setText(chp->chanN.lev ? "1" : "0");
}

bool DebugWin::fillAll() {
	ui.labTcount->setText(QString("%0 / %1").arg(comp->tickCount - tCount).arg(comp->frmtCount));
	fillCPU();
	fillMem();
	fillDump();
	fillFDC();
	fillGBoy();
	drawNes();
	fillAY();
	ui.brkTab->update();
	if (ui.scrLabel->isVisible())
		updateScreen();

	ui.labRX->setNum(comp->vid->ray.x);
	if (comp->vid->hblank)
		ui.labRX->setStyleSheet("background-color: rgb(255,160,160);");
	else
		ui.labRX->setStyleSheet("");

	ui.labRY->setNum(comp->vid->ray.y);
	if (comp->vid->vblank)
		ui.labRY->setStyleSheet("background-color: rgb(255,160,160);");
	else
		ui.labRY->setStyleSheet("");

	setSignal(ui.labDOS, comp->dos);
	setSignal(ui.labROM, comp->rom);
	setSignal(ui.labCPM, comp->cpm);
	setSignal(ui.labINT, comp->cpu->intrq & comp->cpu->inten);
	if (memViewer->isVisible())
		memViewer->fillImage();
	return fillDisasm();
}

// gameboy

extern xColor iniCol[4];
void drawGBTile(QImage& img, Video* vid, int x, int y, int adr) {
	int row, bit;
	int data;
	unsigned char col;
	xColor xcol;
	for (row = 0; row < 8; row++) {
		data = vid->ram[adr & 0x3fff] & 0xff;
		adr++;
		data |= (vid->ram[adr & 0x3fff] & 0xff) << 8;
		adr++;
		for (bit = 0; bit < 8; bit++) {
			col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);
			xcol = iniCol[col];
			img.setPixel(x + bit, y + row, qRgb(xcol.r, xcol.g, xcol.b));
			data <<= 1;
		}
	}
}

QImage getGBTiles(Video* vid, int tset) {
	int tadr = (tset & 1) ? 0x800 : 0;
	if (tset & 2) tadr |= 0x2000;
	int x,y;
	QImage img(128, 128, QImage::Format_RGB888);
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			drawGBTile(img, vid, x << 3, y << 3, tadr);
			tadr += 16;
		}
	}
	return img.scaled(256,256);
}

QImage getGBMap(Video* vid, int tmap, int tset) {
	QImage img(256, 256, QImage::Format_RGB888);
	img.fill(qRgb(0,0,0));
	int adr = tmap ? 0x1c00 : 0x1800;
	int badr = (tset & 1) ? 0x800 : 0;
	if (tset & 2) badr |= 0x2000;
	int tadr;
	unsigned char tile;
	int x,y;
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			tile = vid->ram[adr & 0x1fff];
			adr++;
			tadr = badr;
			if (tset & 1) {
				tadr += (tile ^ 0x80) << 4;
			} else {
				tadr += tile << 4;
			}
			drawGBTile(img, vid, x << 3, y << 3, tadr);
		}
	}
	return img;
}

QImage getGBPal(Video* gbv) {
	QImage img(256,256,QImage::Format_RGB888);
	img.fill(Qt::black);
	int x,y;
	int idx = 0;
	xColor col;
	QPainter pnt;
	pnt.begin(&img);
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 4; x++) {
			col = gbv->pal[idx++];
			pnt.fillRect((x << 6) + 1, (y << 4) + 1, 62, 14, QColor(col.r, col.g, col.b));
			if (idx == 32) idx += 32;
		}
	}
	pnt.end();
	return img;
}

void DebugWin::fillGBoy() {
	QImage img;
	int tset = ui.sbTileset->value();
	int tmap = ui.sbTilemap->value();
	if (ui.rbTilesetView->isChecked()) {
		img = getGBTiles(comp->vid, tset);
	} else if (ui.rbTilemapView->isChecked()) {
		img = getGBMap(comp->vid, tmap, tset);
	} else {
		img = getGBPal(comp->vid);
	}
	ui.gbImage->setPixmap(QPixmap::fromImage(img));
}

// nes

extern xColor nesPal[64];

void dbgNesConvertColors(Video* vid, unsigned char* buf, QImage& img, int trn) {
	int x, y;
	unsigned char col;
	xColor xcol;
	int adr = 0;
	for (y = 0; y < 240; y++) {
		for (x = 0; x < 256; x++) {
			col = buf[adr];
			if (!(col & 3)) col = 0;
			col = vid->ram[0x3f00 | (col & 0x3f)];
			xcol = nesPal[col & 0x3f];
			if (!trn || (col & 3)) {
				img.setPixel(x, y, qRgba(xcol.r, xcol.g, xcol.b, 0xff));
			} else {
				img.setPixel(x, y, Qt::transparent);
			}
			adr++;
		}
	}
}

QImage dbgNesScreenImg(Video* vid, unsigned short adr, unsigned short tadr) {
	QImage img(256, 240, QImage::Format_ARGB32);
	img.fill(Qt::black);
	unsigned char scrmap[256 * 240];
	memset(scrmap, 0x00, 256 * 240);
	if (adr != 0) {
		for(int y = 0; y < 240; y++) {
			ppuRenderBGLine(vid, scrmap + (y << 8), adr, 0, tadr);
			adr = ppuYinc(adr);
		}
	}
	dbgNesConvertColors(vid, scrmap, img, 0);
	return img;
}

QImage dbgNesSpriteImg(Video* vid, unsigned short tadr) {
	QImage img(256, 240, QImage::Format_ARGB32);
	img.fill(Qt::transparent);
	unsigned char scrmap[256 * 240];
	memset(scrmap, 0x00, 256 * 240);
	for (int y = 0; y < 240; y++) {
		ppuRenderSpriteLine(vid, y, scrmap + (y << 8), NULL, tadr, 64);
	}
	dbgNesConvertColors(vid, scrmap, img, 1);
	return img;
}

void DebugWin::drawNes() {
	unsigned short adr = 0;
	unsigned short tadr = 0;
	QImage img;
	QPixmap pic;

	ui.labVAdr->setText(gethexword(comp->vid->vadr & 0x7fff));
	ui.labTAdr->setText(gethexword(comp->vid->tadr & 0x7fff));

	switch(ui.nesScrType->itemData(ui.nesScrType->currentIndex()).toInt()) {
		case NES_SCR_OFF: adr = 0; break;
		case NES_SCR_0:	adr = 0x2000; break;
		case NES_SCR_1: adr = 0x2400; break;
		case NES_SCR_2: adr = 0x2800; break;
		case NES_SCR_3: adr = 0x2c00; break;
		case NES_SCR_ALL: adr = 0xffff; break;
	}
	tadr = getRFIData(ui.nesBGTileset) & 0xffff;
	QPainter pnt;
	pic = QPixmap(256, 240);
	if (adr != 0xffff) {
		img = dbgNesScreenImg(comp->vid, adr, tadr);
		pnt.begin(&pic);
		pnt.drawImage(0,0,img);
		pnt.end();
		ui.nesScreen->setPixmap(pic);
	} else {
		pnt.begin(&pic);
		pnt.drawImage(0, 0, dbgNesScreenImg(comp->vid, 0x2000, tadr).scaled(128, 120));
		pnt.drawImage(128, 0, dbgNesScreenImg(comp->vid, 0x2400, tadr).scaled(128, 120));
		pnt.drawImage(0, 120, dbgNesScreenImg(comp->vid, 0x2800, tadr).scaled(128, 120));
		pnt.drawImage(128, 120, dbgNesScreenImg(comp->vid, 0x2c00, tadr).scaled(128, 120));
		pnt.end();
		ui.nesScreen->setPixmap(pic);
	}
}

// ...

void DebugWin::setFlagNames(const char name[8]) {
	ui.labF7->setText(QString(name[0]));
	ui.labF6->setText(QString(name[1]));
	ui.labF5->setText(QString(name[2]));
	ui.labF4->setText(QString(name[3]));
	ui.labF3->setText(QString(name[4]));
	ui.labF2->setText(QString(name[5]));
	ui.labF1->setText(QString(name[6]));
	ui.labF0->setText(QString(name[7]));
}

void DebugWin::chLayout() {
	switch (comp->cpu->type) {
		case CPU_Z80:
			ui.boxIM->setEnabled(true);
			break;
		case CPU_LR35902:
			ui.boxIM->setEnabled(false);
			break;
		case CPU_6502:
			ui.boxIM->setEnabled(false);
			break;
		default:
			break;
	}
}

void DebugWin::regClick(QMouseEvent* ev) {
	xLabel* lab = (xLabel*)sender();
	int id = lab->id;
	if (id < 0) return;
	if (id > 15) return;
	CPU* cpu = comp->cpu;
	xRegBunch bunch = cpuGetRegs(cpu);
	xRegister reg = bunch.regs[id];
	unsigned short val = reg.value;
	switch (ev->button()) {
		case Qt::RightButton:
			dumpAdr = val;
			fillDump();
			break;
		case Qt::LeftButton:
			disasmAdr = val;
			fillDisasm();
			break;
		default:
			break;
	}
}

// rzx

/*
void dbgSetRzxIO(QLabel* lab, Computer* comp, int pos) {
#ifdef HAVEZLIB
	if (pos < comp->rzx.frm.size) {
		lab->setText(gethexbyte(comp->rzx.frm.data[pos]));
	} else {
		lab->setText("--");
	}
#endif
}

void DebugWin::fillRZX() {
	ui.rzxFrm->setText(QString::number(comp->rzx.frame).append(" / ").append(QString::number(comp->rzx.size)));
	ui.rzxFetch->setText(QString::number(comp->rzx.fetches));
	ui.rzxFSize->setText(QString::number(comp->rzx.data[comp->rzx.frame].frmSize));
	int pos = comp->rzx.pos;
	dbgSetRzxIO(ui.rzxIO1, comp, pos++);
	dbgSetRzxIO(ui.rzxIO2, comp, pos++);
	dbgSetRzxIO(ui.rzxIO3, comp, pos++);
	dbgSetRzxIO(ui.rzxIO4, comp, pos++);
	dbgSetRzxIO(ui.rzxIO5, comp, pos);
}
*/

// fdc

void DebugWin::fillFDC() {
	ui.fdcBusyL->setText(comp->dif->fdc->idle ? "0" : "1");
	ui.fdcComL->setText(comp->dif->fdc->idle ? "--" : gethexbyte(comp->dif->fdc->com));
	ui.fdcIrqL->setText(comp->dif->fdc->irq ? "1" : "0");
	ui.fdcDrqL->setText(comp->dif->fdc->drq ? "1" : "0");
	ui.fdcTrkL->setText(gethexbyte(comp->dif->fdc->trk));
	ui.fdcSecL->setText(gethexbyte(comp->dif->fdc->sec));
	ui.fdcHeadL->setText(comp->dif->fdc->side ? "1" : "0");
	ui.fdcDataL->setText(gethexbyte(comp->dif->fdc->data));
	ui.fdcStateL->setText(gethexbyte(comp->dif->fdc->state));
	ui.fdcSr0L->setText(gethexbyte(comp->dif->fdc->sr0));
	ui.fdcSr1L->setText(gethexbyte(comp->dif->fdc->sr1));
	ui.fdcSr2L->setText(gethexbyte(comp->dif->fdc->sr2));

	ui.flpCurL->setText(QString('A' + comp->dif->fdc->flp->id));
	ui.flpRdyL->setText(comp->dif->fdc->flp->insert ? "1" : "0");
	ui.flpTrkL->setText(gethexbyte(comp->dif->fdc->flp->trk));
	ui.flpPosL->setText(QString::number(comp->dif->fdc->flp->pos));
	ui.flpIdxL->setText(comp->dif->fdc->flp->index ? "1" : "0");
	ui.flpDataL->setText(comp->dif->fdc->flp->insert ? gethexbyte(flpRd(comp->dif->fdc->flp)): "--"); comp->dif->fdc->flp->rd = 0;
	ui.flpMotL->setText(comp->dif->fdc->flp->motor ? "1" : "0");
}

// z80 regs section

void setCBFlag(QCheckBox* cb, int state) {
	if ((cb->isChecked() && !state) || (!cb->isChecked() && state)) {
		cb->setStyleSheet("background-color: rgb(160,255,160);");
	} else {
		cb->setStyleSheet("");
	}
	cb->setChecked(state);
}

void DebugWin::fillFlags() {
	unsigned char flg = comp->cpu->f;
	setCBFlag(ui.cbF7, flg & 0x80);
	setCBFlag(ui.cbF6, flg & 0x40);
	setCBFlag(ui.cbF5, flg & 0x20);
	setCBFlag(ui.cbF4, flg & 0x10);
	setCBFlag(ui.cbF3, flg & 0x08);
	setCBFlag(ui.cbF2, flg & 0x04);
	setCBFlag(ui.cbF1, flg & 0x02);
	setCBFlag(ui.cbF0, flg & 0x01);
}

void setLEReg(QLineEdit* le, int num) {
	QString txt = gethexword(num);
	if (le->text() == txt) {
		le->setStyleSheet("");
	} else {
		le->setStyleSheet("background-color: rgb(160,255,160);");
	}
	le->setText(txt);
}

void DebugWin::fillCPU() {
	block = 1;
	CPU* cpu = comp->cpu;
	xRegBunch bunch = cpuGetRegs(cpu);
	int i = 0;
	while(dbgRegLabs[i] != NULL) {
		switch (bunch.regs[i].id) {
			case REG_EMPTY:
			case REG_NONE:
				dbgRegLabs[i]->clear();
				dbgRegEdit[i]->setVisible(false);
				dbgRegEdit[i]->clear();
				break;
			default:
				dbgRegLabs[i]->setText(bunch.regs[i].name);
				dbgRegEdit[i]->setProperty("regid", bunch.regs[i].id);
				dbgRegEdit[i]->setMax(bunch.regs[i].byte ? 0xff : 0xffff);
				dbgRegEdit[i]->setValue(bunch.regs[i].value);
				dbgRegEdit[i]->setVisible(true);
				break;
		}
		i++;
	}
	setFlagNames(bunch.flags);
	ui.boxIM->setValue(cpu->imode);
	ui.flagIFF1->setChecked(cpu->iff1);
	ui.flagIFF2->setChecked(cpu->iff2);
	fillFlags();
	block = 0;
	fillStack();
}

void DebugWin::setFlags() {
	if (block) return;
	unsigned char f = 0;
	if (ui.cbF7->isChecked()) f |= 0x80;
	if (ui.cbF6->isChecked()) f |= 0x40;
	if (ui.cbF5->isChecked()) f |= 0x20;
	if (ui.cbF4->isChecked()) f |= 0x10;
	if (ui.cbF3->isChecked()) f |= 0x08;
	if (ui.cbF2->isChecked()) f |= 0x04;
	if (ui.cbF1->isChecked()) f |= 0x02;
	if (ui.cbF0->isChecked()) f |= 0x01;
	comp->cpu->f = f;
	fillCPU();
	fillDisasm();
}

void DebugWin::setCPU() {
	if (block) return;
	CPU* cpu = comp->cpu;
	int i = 0;
	int idx = 0;
	xRegBunch bunch;
	while (dbgRegEdit[idx] != NULL) {
		if (dbgRegEdit[idx]->isEnabled()) {
			bunch.regs[i].id = dbgRegEdit[idx]->property("regid").toInt();
			bunch.regs[i].value = dbgRegEdit[idx]->getValue();
			i++;
		} else {
			bunch.regs[i].id = REG_NONE;
		}
		idx++;
	}
	cpuSetRegs(cpu, bunch);
	cpu->imode = ui.boxIM->value();
	cpu->iff1 = ui.flagIFF1->isChecked() ? 1 : 0;
	cpu->iff2 = ui.flagIFF2->isChecked() ? 1 : 0;
	fillFlags();
	fillStack();
	fillDisasm();
}

// memory map section

QString getPageName(MemPage& pg) {
	QString res;
	switch(pg.type) {
		case MEM_RAM: res = "RAM:"; break;
		case MEM_ROM: res = "ROM:"; break;
		case MEM_EXT: res = "EXT:"; break;
		case MEM_SLOT: res = "SLT:"; break;
		default: res = "---:"; break;
	}
	res.append(gethexbyte(pg.num >> 6));
	return res;
}

void DebugWin::fillMem() {
	ui.labPG0->setText(getPageName(comp->mem->map[0x00]));
	ui.labPG1->setText(getPageName(comp->mem->map[0x40]));
	ui.labPG2->setText(getPageName(comp->mem->map[0x80]));
	ui.labPG3->setText(getPageName(comp->mem->map[0xc0]));
}

// labels

void DebugWin::loadLabels(QString path) {
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(this, "Load SJASM labels");
	if (path.isEmpty())
		return;
	labels.clear();
	QString line;
	QString name;
	QStringList arr;
	QFile file(path);
	xAdr xadr;
	if (file.open(QFile::ReadOnly)) {
		while(!file.atEnd()) {
			line = file.readLine();
			arr = line.split(QRegExp("[: \r\n]"),QString::SkipEmptyParts);
			if (arr.size() > 2) {
				xadr.type = MEM_RAM;
				xadr.bank = arr.at(0).toInt(NULL,16);
				xadr.adr = arr.at(1).toInt(NULL,16) & 0x3fff;
                xadr.abs = (xadr.bank << 14) | xadr.adr;
				name = arr.at(2);
				switch (xadr.bank) {
					case 0xff:
						xadr.type = MEM_ROM;
						xadr.bank = -1;
						break;
					case 0x05:
						xadr.adr |= 0x4000;
						break;
					case 0x02:
						xadr.adr |= 0x8000;
						break;
					default:
						xadr.bank = -1;
						xadr.adr |= 0xc000;
						break;
				}
				labels[name] = xadr;
			}
		}
	} else {
		shitHappens("Can't open file");
	}
}

void DebugWin::saveLabels() {
	QString path = QFileDialog::getSaveFileName(this, "save SJASM labels");
	if (path.isEmpty()) return;
	QStringList keys;
	QString key;
	xAdr xadr;
	QString line;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		keys = labels.keys();
		foreach(key, keys) {
			xadr = labels[key];
			line = (xadr.type == MEM_RAM) ? gethexbyte(xadr.bank) : "FF";
			line.append(QString(":%0 %1\n").arg(gethexword(xadr.adr & 0x3fff)).arg(key));
			file.write(line.toUtf8());
		}
		file.close();
	} else {
		shitHappens("Can't open file for writing");
	}
}

QString findLabel(int adr, int type, int bank) {
	QString lab;
	if (!conf.dbg.labels)
		return lab;
	QString key;
	xAdr xadr;
	QStringList keys = labels.keys();
	foreach(key, keys) {
		xadr = labels[key];
		if ((xadr.adr == adr)\
				&& ((type < 0) || (xadr.type < 0) || (type == xadr.type))\
				&& ((bank < 0) || (xadr.bank < 0) || (bank == xadr.bank))) {
			lab = key;
			break;
		}
	}
	return lab;
}


// map

void fwritepack(QDataStream& strm, unsigned char* data, int size) {
	QByteArray pack = qCompress(data, size);
	strm << pack;
}

void freadpack(QDataStream& strm, unsigned char* data, int maxsize) {
	QByteArray pack;
	strm >> pack;
	QByteArray unpk = qUncompress(pack);
	int size = unpk.size();
	if (size > maxsize) size = maxsize;
	memcpy(data, unpk.data(), size);
}

void strmLabels(QDataStream& strm) {
	QStringList keys = labels.keys();
	QString key;
	xAdr xadr;
	foreach(key, keys) {			// labels list
		xadr = labels[key];
		strm << xadr.type;
		strm << xadr.bank;
		strm << xadr.adr;
		strm << key;
	}
	strm << 0x00 << 0x00 << 0x00;		// end of labels list
	strm << QString();
}

void strdLabels(QDataStream& strm) {
	xAdr xadr;
	QString key;
	labels.clear();
	do {
		strm >> xadr.type;
		strm >> xadr.bank;
		strm >> xadr.adr;
		strm >> key;
		if (!key.isEmpty()) {
			labels[key] = xadr;
		}
	} while (!key.isEmpty());
}

#define XDBGVER 0x00

void DebugWin::saveMap() {
	QString path = QFileDialog::getSaveFileName(this, "Save deBUGa project","","deBUGa project (*.xdbg)",NULL,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	if (!path.endsWith(".xdbg",Qt::CaseInsensitive))
		path.append(".xdbg");
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		QDataStream strm(&file);
#if 1
		strm << QString("XDBG");		// [new] signature
		strm << XDBGVER;			// version
		strmLabels(strm);
		int bit = 3;				// b0:ram cells, b1:rom cells, b2:slt cells
		if (comp->slot->brkMap)
			bit |= 4;
		strm << bit;
		fwritepack(strm, comp->brkRamMap, 0x400000);
		fwritepack(strm, comp->brkRomMap, 0x80000);
		if (bit & 4)
			fwritepack(strm, comp->slot->brkMap, comp->slot->memMask + 1);
#else

		strm << QString("deBUGa");		// signature
		strmLabels(strm);
		fwritepack(strm, comp->brkRamMap, 0x400000);
		fwritepack(strm, comp->brkRomMap, 0x80000);
#endif
		file.close();
	}
}

void DebugWin::loadMap() {
	QString path = QFileDialog::getOpenFileName(this, "Open deBUGa project","","deBUGa project (*.xdbg)");
	if (path.isEmpty()) return;
	QFile file(path);
	QString key;
	int bt;
	if (file.open(QFile::ReadOnly)) {
		QDataStream strm(&file);
		strm >> key;
		if (key == QString("deBUGa")) {		// old data
			strdLabels(strm);
			freadpack(strm, comp->brkRamMap, 0x400000);
			freadpack(strm, comp->brkRomMap, 0x400000);
		} else if (key == QString("XDBG")) {	// new data
			strm >> bt;
			if (bt > XDBGVER) {
				shitHappens("Version mismatch");
			} else {
				strdLabels(strm);
				strm >> bt;
				memset(comp->brkRamMap, 0x00, 0x400000);
				memset(comp->brkRomMap, 0x00, 0x80000);
				if (comp->slot->brkMap) memset(comp->slot->brkMap, 0x00, comp->slot->memMask + 1);
				if (bt & 1)
					freadpack(strm, comp->brkRamMap, 0x400000);
				if (bt & 2)
					freadpack(strm, comp->brkRomMap, 0x80000);
				if ((bt & 4) && comp->slot->brkMap)
					freadpack(strm, comp->slot->brkMap, comp->slot->memMask + 1);
			}
		} else {
			shitHappens("Wrong signature");
		}
		file.close();
		brkInstallAll();
		fillAll();
	}
}

// disasm table

unsigned char rdbyte(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return memRd(comp->mem, adr);
}

int checkCond(Computer* comp, int num) {
	int res = 0;
	unsigned char flg = comp->cpu->f;
	switch (num) {
		case 0: res = (flg & FZ) ? 0 : 1; break;	// NZ
		case 1: res = (flg & FZ) ? 1 : 0; break;	// Z
		case 2: res = (flg & FC) ? 0 : 1; break;	// NC
		case 3: res = (flg & FC) ? 1 : 0; break;	// C
		case 4: res = (flg & FP) ? 0 : 1; break;	// PO
		case 5: res = (flg & FP) ? 1 : 0; break;	// PE
		case 6: res = (flg & FS) ? 0 : 1; break;	// N
		case 7: res = (flg & FS) ? 1 : 0; break;	// M
	}
	return res;
}

int getCommandSize(Computer* comp, unsigned short adr) {
	int type = getBrk(comp, adr) & 0xf0;
	unsigned char fl;
	unsigned char bt;
	char buf[256];
	xMnem mn;
	int res;
	switch (type) {
		case DBG_VIEW_BYTE:
			res = 1;
			break;
		case DBG_VIEW_ADDR:
		case DBG_VIEW_WORD:
			res = 2;
			break;
		case DBG_VIEW_TEXT:
			fl = getBrk(comp, adr);
			bt = memRd(comp->mem, adr);
			res = 0;
			while (((fl & 0xc0) == DBG_VIEW_TEXT) && (bt > 31) && (bt < 128) && (res < 250)) {
				res++;
				adr++;
				bt = memRd(comp->mem, adr & 0xffff);
				fl = getBrk(comp, adr & 0xffff);
			}
			if (res == 0)
				res++;
			break;
		case DBG_VIEW_CODE:
		case DBG_VIEW_EXEC:
			mn = cpuDisasm(comp->cpu, adr, buf, &rdbyte, comp);
			res = mn.len;
			break;
		default:
			res = 1;
			break;
	}
	return res;
}

int DebugWin::fillDisasm() {
	conf.dbg.hideadr = ui.actHideAddr->isChecked() ? 1 : 0;
	return ui.dasmTable->updContent();
}

unsigned short DebugWin::getPrevAdr(unsigned short adr) {
	for (int i = 16; i > 0; i--) {
		if (getCommandSize(comp, adr - i) == i)
			return adr - i;
	}
	return (adr - 1);
}

void DebugWin::saveDasm() {
	QString path = QFileDialog::getSaveFileName(this, "Save disasm");
	if (path.isEmpty()) return;
	QFile file(path);
	dasmData drow;
	if (file.open(QFile::WriteOnly)) {
		QTextStream strm(&file);
		unsigned short adr = (blockStart < 0) ? 0 : (blockStart & 0xffff);
		unsigned short end = (blockEnd < 0) ? 0 : (blockEnd & 0xffff);
		int work = 1;
		strm << "; Created by Xpeccy deBUGa\n\n";
		strm << "\tORG 0x" << gethexword(adr) << "\n\n";
		while ((adr <= end) && work) {
			drow = getDisasm(comp, adr);
			if (adr < drow.adr) work = 0;		// address overfill (FFFF+)
			if (drow.islab)
				strm << drow.aname << ":\n";
			strm << "\t" << drow.command << "\n";
		}
		file.close();
	} else {
		shitHappens("Can't write to file");
	}
}

// memory dump

void DebugWin::fillDump() {
	block = 1;
	ui.dumpTable->update();
	fillStack();
	dumpChadr(ui.dumpTable->selectionModel()->currentIndex());
	block = 0;
}

void DebugWin::dumpChadr(QModelIndex idx) {
	int col = idx.column();
	int adr = dumpAdr + (idx.row() << 3);
	if ((col > 0) && (col < 9)) {
		 adr += (idx.column() - 1);
	}
	if (ui.dumpTable->mode != XVIEW_CPU)
		adr &= 0x3fff;
	ui.labDump->setText(QString("Dump : %0").arg(gethexword(adr & 0xffff)));
}

// maping

void DebugWin::mapClear() {
	if (!areSure("Clear memory mapping?")) return;
	int adr;
	for (adr = 0; adr < 0x400000; adr++) {
		comp->brkRamMap[adr] &= 0x0f;
		if (adr < 0x80000) comp->brkRomMap[adr] &= 0x0f;
		if (comp->slot->data && (adr <= comp->slot->memMask))
			comp->slot->brkMap[adr] &= 0x0f;
	}
	fillDisasm();
}

void DebugWin::mapAuto() {

}

// stack

void DebugWin::fillStack() {
	int adr = comp->cpu->sp;
	QString str;
	for (int i = 0; i < 4; i++) {
		str.append(gethexbyte(memRd(comp->mem, adr+1)));
		str.append(gethexbyte(memRd(comp->mem, adr)));
		adr += 2;
	}
	ui.labSP->setText(str.left(4));
	ui.labSP2->setText(str.mid(4,4));
	ui.labSP4->setText(str.mid(8,4));
	ui.labSP6->setText(str.mid(12,4));
}

// breakpoint

int DebugWin::getAdr() {
	int adr = -1;
	QModelIndex idx;
	if (ui.dumpTable->hasFocus()) {
		idx = ui.dumpTable->currentIndex();
		adr = (dumpAdr + idx.column() - 1 + (idx.row() << 3)) & 0xffff;
	} else {
		idx = ui.dasmTable->currentIndex();
		adr = ui.dasmTable->getData(idx.row(), 0, Qt::UserRole).toInt();
	}
	return adr;
}

void DebugWin::putBreakPoint() {
	int adr = getAdr();
	if (adr < 0) return;
	doBreakPoint(adr);
	cellMenu->move(QCursor::pos());
	cellMenu->show();
}

void DebugWin::doBreakPoint(unsigned short adr) {
	bpAdr = adr;
	unsigned char flag = getBrk(comp, adr);
	ui.actFetch->setChecked(flag & MEM_BRK_FETCH);
	ui.actRead->setChecked(flag & MEM_BRK_RD);
	ui.actWrite->setChecked(flag & MEM_BRK_WR);
}

void DebugWin::chaCellProperty(QAction* act) {
	int data = act->data().toInt();
	int adr = getAdr();
	int bgn, end;
//	unsigned short eadr;
	unsigned char bt;
	unsigned char* ptr;
	if ((adr < blockStart) || (adr > blockEnd)) {	// pointer outside block : process 1 cell
		bgn = adr;
		end = adr;
	} else {									// pointer inside block : process all block
		bgn = blockStart;
		end = blockEnd;
	}
	// eadr = end & 0xffff; getDisasm(comp, eadr); end = (eadr-1) & 0xffff;		// move end to last byte of block
	if (end < bgn)
		end += 0x10000;
	adr = bgn;
	while (adr <= end) {
		ptr = getBrkPtr(comp, adr);
		if (data & MEM_BRK_ANY) {
			bt = 0;
			if (ui.actFetch->isChecked()) bt |= MEM_BRK_FETCH;
			if (ui.actRead->isChecked()) bt |= MEM_BRK_RD;
			if (ui.actWrite->isChecked()) bt |= MEM_BRK_WR;
			brkSet(BRK_MEMCELL, bt, adr, -1);
			ui.bpList->update();
		} else {
			*ptr &= 0x0f;
			if ((data & 0xf0) == DBG_VIEW_TEXT) {
				bt = memRd(comp->mem, adr);
				if ((bt < 32) || (bt > 127)) {
					*ptr |= DBG_VIEW_BYTE;
				} else {
					*ptr |= DBG_VIEW_TEXT;
				}
			} else {
				*ptr |= (data & 0xf0);
			}
		}
		adr++;
	}
	fillDisasm();
	fillDump();
//	fillBrkTable();
}

// memDump

void DebugWin::doSaveDump() {
	dui.leBank->setText(QString::number(comp->mem->map[3].num,16));
	dumpwin->show();
}

void DebugWin::dmpLimChanged() {
	int start = dui.leStart->text().toInt(NULL,16);
	int end = dui.leEnd->text().toInt(NULL,16);
	if (end < start) end = start;
	int len = end-start+1;
	start = dui.leEnd->cursorPosition();
	dui.leEnd->setText(QString::number(end,16));
	dui.leLen->setText(QString::number(len,16));
	dui.leEnd->setCursorPosition(start);
}

void DebugWin::dmpLenChanged() {
	int start = dui.leStart->text().toInt(NULL,16);
	int len = dui.leLen->text().toInt(NULL,16);
	if (start + len > 0xffff) {
		len = 0x10000 - start;
		dui.leLen->setText(QString::number(len,16));
	}
	int end = start + len - 1;
	start = dui.leLen->cursorPosition();
	dui.leEnd->setText(QString::number(end,16));
	dui.leLen->setCursorPosition(start);
}

QByteArray DebugWin::getDumpData() {
//	MemPage curBank = comp->mem->map[MEM_BANK3];
//	int bank = dui.leBank->text().toInt(NULL,16);
	int adr = dui.leStart->text().toInt(NULL,16);
	int len = dui.leLen->text().toInt(NULL,16);
//	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, bank, NULL, NULL, NULL);
	QByteArray res;
	while (len > 0) {
		res.append(memRd(comp->mem,adr));
		adr++;
		len--;
	}
//	comp->mem->map[3] = curBank;
	return res;
}

void DebugWin::saveDumpBin() {
	QByteArray data = getDumpData();
	if (data.size() == 0) return;
	QString path = QFileDialog::getSaveFileName(this,"Save memory dump");
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) file.write(data);
	dumpwin->hide();
}

void DebugWin::saveDumpHobeta() {
	QByteArray data = getDumpData();
	if (data.size() == 0) return;
	QString path = QFileDialog::getSaveFileName(this,"Save memory dump as hobeta","","Hobeta files (*.$C)");
	if (path.isEmpty()) return;
	TRFile dsc;
	QString name = dui.leStart->text();
	name.append(".").append(dui.leBank->text());
	std::string nms = name.toStdString();
	nms.resize(8,' ');
	memcpy(dsc.name,nms.c_str(),8);
	dsc.ext = 'C';
	int start = dui.leStart->text().toInt(NULL,16);
	int len = data.size();
	dsc.hst = (start >> 8) & 0xff;
	dsc.lst = start & 0xff;
	dsc.hlen = (len >> 8) & 0xff;
	dsc.llen = len & 0xff;
	dsc.slen = dsc.hlen + ((len & 0xff) ? 1 : 0);
	saveHobeta(dsc, data.data(), path.toStdString().c_str());
	dumpwin->hide();
}

void DebugWin::saveDumpToA() {saveDumpToDisk(0);}
void DebugWin::saveDumpToB() {saveDumpToDisk(1);}
void DebugWin::saveDumpToC() {saveDumpToDisk(2);}
void DebugWin::saveDumpToD() {saveDumpToDisk(3);}

void DebugWin::saveDumpToDisk(int idx) {
	QByteArray data = getDumpData();
	if (data.size() == 0) return;
	if (data.size() > 0xff00) return;
	int start = dui.leStart->text().toInt(NULL,16);
	int len = dui.leLen->text().toInt(NULL,16);
	QString name = dui.leStart->text();
	name.append(".").append(dui.leBank->text());
	Floppy* flp = comp->dif->fdc->flop[idx & 3];
	if (!flp->insert) {
		diskFormat(flp);
		flp->insert = 1;
	}
	TRFile dsc = diskMakeDescriptor(name.toStdString().c_str(), 'C', start, len);
	if (diskCreateFile(flp, dsc, (unsigned char*)data.data(), data.size()) == ERR_OK) dumpwin->hide();

}

// memfinder

void DebugWin::doFind() {
	memFinder->mem = comp->mem;
	memFinder->adr = (disasmAdr + 1) & 0xffff;
	memFinder->show();
}

void DebugWin::onFound(int adr) {
	disasmAdr = adr & 0xffff;
	fillDisasm();
}

// memfiller

void DebugWin::doFill() {
	memFiller->start(comp->mem, blockStart, blockEnd);
}

// spr scanner

void DebugWin::doMemView() {
	memViewer->mem = comp->mem;
	memViewer->ui.sbPage->setValue(comp->mem->map[0xc0].num >> 14);
	memViewer->fillImage();
	memViewer->show();
}

// open dump

void DebugWin::doOpenDump() {
	dumpPath.clear();
	oui.laPath->clear();
	oui.leBank->setText(QString::number(comp->mem->map[3].num,16));
	oui.leStart->setText("4000");
	openDumpDialog->show();
}

void DebugWin::chDumpFile() {
	QString path = QFileDialog::getOpenFileName(this,"Open dump");
	if (path.isEmpty()) return;
	QFileInfo inf(path);
	if ((inf.size() == 0) || (inf.size() > 0xff00)) {
		shitHappens("File is too long");
	} else {
		dumpPath = path;
		oui.laPath->setText(path);
		oui.leLen->setText(QString::number(inf.size(),16));
		dmpStartOpen();
	}
}

void DebugWin::dmpStartOpen() {
	int start = oui.leStart->text().toInt(NULL,16);
	int len = oui.leLen->text().toInt(NULL,16);
	int pos = oui.leStart->cursorPosition();
	if (start + len > 0xffff) start = 0x10000 - len;
	int end = start + len - 1;
	oui.leStart->setText(QString::number(start,16));
	oui.leEnd->setText(QString::number(end,16));
	oui.leStart->setCursorPosition(pos);
}

void DebugWin::loadDump() {
	if (dumpPath.isEmpty()) return;
	int res = loadDUMP(comp, dumpPath.toStdString().c_str(),oui.leStart->text().toInt(NULL,16));
	fillAll();
	if (res == ERR_OK) {
		openDumpDialog->hide();
	} else {
		shitHappens("Can't open file");
	}
}

// screen

void DebugWin::updateScreen() {
	int flag = ui.cbScrAtr->isChecked() ? 1 : 0;
	flag |= ui.cbScrPix->isChecked() ? 2 : 0;
	flag |= ui.cbScrGrid->isChecked() ? 4 : 0;
	vidGetScreen(comp->vid, scrImg.bits(), ui.sbScrBank->value(), ui.leScrAdr->getValue(), flag);
	ui.scrLabel->setPixmap(QPixmap::fromImage(scrImg));
}

// breakpoints

void DebugWin::addBrk() {
	brkManager->edit(NULL);
}

void DebugWin::editBrk() {
	QModelIndexList idxl = ui.bpList->selectionModel()->selectedRows();
	if (idxl.size() < 1) return;
	int row = idxl.first().row();
	xBrkPoint* brk = &conf.prof.cur->brkList[row];
	brkManager->edit(brk);
}

void DebugWin::delBrk() {
	QModelIndexList idxl = ui.bpList->selectionModel()->selectedRows();
	qSort(idxl.begin(), idxl.end(), qGreater<QModelIndex>());
	QModelIndex idx;
	xBrkPoint brk;
	foreach(idx, idxl) {
		brk = conf.prof.cur->brkList[idx.row()];
		brkDelete(brk);
	}
	ui.bpList->update();
	fillDisasm();
	fillDump();
}

void DebugWin::confirmBrk(xBrkPoint obrk, xBrkPoint brk) {
	brkDelete(obrk);
	brkAdd(brk);
	fillDisasm();
	fillDump();
	ui.bpList->update();
}

void DebugWin::goToBrk(QModelIndex idx) {
	if (!idx.isValid()) return;
	int row = idx.row();
	xBrkPoint brk = conf.prof.cur->brkList[row];
	int adr;
	int mtype = MEM_EXT;
	switch(brk.type) {
		case BRK_CPUADR:
			adr = brk.adr & 0xffff;
			break;
		default:
			switch(brk.type) {
				case BRK_MEMRAM: mtype = MEM_RAM; break;
				case BRK_MEMROM: mtype = MEM_ROM; break;
				case BRK_MEMSLT: mtype = MEM_SLOT; break;
			}
			adr = memFindAdr(comp->mem, mtype, brk.adr);
			break;
	}
	if (adr < 0) return;
	disasmAdr = adr & 0xffff;
	fillDisasm();
}

void DebugWin::saveBrk(QString path) {
	if (path.isEmpty())
		path = QFileDialog::getSaveFileName(this, "Save breakpoints", "", "deBUGa breakpoints (*.xbrk)");
	if (path.isEmpty())
		return;
	if (!path.endsWith(".xbrk"))
		path.append(".xbrk");
	xBrkPoint brk;
	QFile file(path);
	QString nm,ar1,ar2,flag;
	if (file.open(QFile::WriteOnly)) {
		file.write("; Xpeccy deBUGa breakpoints list\n");
		foreach(brk, conf.prof.cur->brkList) {
			if (!brk.off) {
				switch(brk.type) {
					case BRK_IOPORT:
						nm = "IO";
						ar1 = gethexword(brk.adr & 0xffff);
						ar2 = gethexword(brk.mask & 0xffff);
						break;
					case BRK_CPUADR:
						nm = "CPU";
						ar1 = gethexword(brk.adr & 0xffff);
						ar2.clear();
						break;
					case BRK_MEMRAM:
						nm = "RAM";
						ar1 = gethexbyte((brk.adr >> 14) & 0xff);	// 16K page
						ar2 = gethexword(brk.adr & 0x3fff);		// adr in page
						break;
					case BRK_MEMROM:
						nm = "ROM";
						ar1 = gethexbyte((brk.adr >> 14) & 0xff);
						ar2 = gethexword(brk.adr & 0x3fff);
						break;
					case BRK_MEMSLT:
						nm = "SLT";
						ar1 = gethexbyte((brk.adr >> 14) & 0xff);
						ar2 = gethexword(brk.adr & 0x3fff);
						break;
					default:
						nm.clear();
						break;
				}
				if (!nm.isEmpty()) {
					flag.clear();
					if (brk.fetch) flag.append("F");
					if (brk.read) flag.append("R");
					if (brk.write) flag.append("W");
					file.write(QString("%0:%1:%2:%3\n").arg(nm).arg(ar1).arg(ar2).arg(flag).toUtf8());
				}
			}
		}
		file.close();
	} else {
		shitHappens("Can't open file for writing");
	}
}

void DebugWin::openBrk() {
	QString path = QFileDialog::getOpenFileName(this, "Open breakpoints list", "", "deBUGa breakpoints (*.xbrk)");
	if (path.isEmpty()) return;
	QFile file(path);
	QString line;
	QStringList list;
	xBrkPoint brk;
	bool b0,b1;
	if (file.open(QFile::ReadOnly)) {
		conf.prof.cur->brkList.clear();
		while(!file.atEnd()) {
			line = trUtf8(file.readLine());
			if (!line.startsWith(";")) {
				b0 = true;
				b1 = true;
				list = line.split(":", QString::KeepEmptyParts);
				while(list.size() < 4)
					list.append(QString());
				brk.fetch = list.at(3).contains("F") ? 1 : 0;
				brk.read = list.at(3).contains("R") ? 1 : 0;
				brk.write = list.at(3).contains("W") ? 1 : 0;
				if (list.at(0) == "IO") {
					brk.type = BRK_IOPORT;
					brk.adr = list.at(1).toInt(&b0, 16) & 0xffff;
					brk.mask = list.at(2).toInt(&b1, 16) & 0xffff;
				} else if (list.at(0) == "CPU") {
					brk.type = BRK_CPUADR;
					brk.adr = list.at(1).toInt(&b0, 16) & 0xffff;
				} else if (list.at(0) == "ROM") {
					brk.type = BRK_MEMROM;
					brk.adr = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
					brk.adr |= (list.at(2).toInt(&b1, 16) & 0x3fff);
				} else if (list.at(0) == "RAM") {
					brk.type = BRK_MEMRAM;
					brk.adr = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
					brk.adr |= (list.at(2).toInt(&b1, 16) & 0x3fff);
				} else if (list.at(0) == "SLT") {
					brk.type = BRK_MEMSLT;
					brk.adr = (list.at(1).toInt(&b0, 16) & 0xff) << 14;
					brk.adr |= (list.at(2).toInt(&b1, 16) & 0x3fff);
				} else {
					b0 = false;
				}
				if (b0 && b1) {
					conf.prof.cur->brkList.push_back(brk);
				}
			}
		}
		file.close();
		brkInstallAll();
		ui.bpList->update();
		fillDisasm();
		fillDump();
	} else {
		shitHappens("Can't open file for reading");
	}
}
