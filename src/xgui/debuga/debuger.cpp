#include "../../xcore/xcore.h"

#include <stdio.h>

#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTextCodec>

#include "debuger.h"
#include "dbg_sprscan.h"
#include "../../filer.h"
#include "../xgui.h"

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

// trace type
enum {
	DBG_TRACE_ALL = 0x100,
	DBG_TRACE_INT,
	DBG_TRACE_HERE
};

typedef struct {
	QLabel* name;
	QLineEdit* edit;
} dbgRegPlace;

void DebugWin::start(Computer* c) {
	comp = c;
	blockStart = -1;
	blockEnd = -1;
	chLayout();
	disasmAdr = comp->cpu->pc;
	fillAll();
//	fillBrkTable();
	updateScreen();
	if (!comp->vid->tail)
		vidDarkTail(comp->vid);
	ui.tabsPanel->setTabEnabled(ui.tabsPanel->indexOf(ui.gbTab), comp->hw->id == HW_GBC);

//	ui.dasmTable->comp = comp;
	move(winPos);
	ui.dasmTable->setFocus();
	comp->vid->debug = 1;
	comp->debug = 1;
	comp->brk = 0;

	show();

	//int wd = 17;
	//ui.dasmTable->setRowCount(ui.dasmTable->height() / wd);
	int wd = ui.dasmTable->height() / ui.dasmTable->rows();
	ui.dasmTable->verticalHeader()->setDefaultSectionSize(wd);

	//ui.dumpTable->setRowCount(ui.dumpTable->height() / wd);
	wd = ui.dumpTable->height() / ui.dumpTable->model()->rowCount();
	ui.dumpTable->verticalHeader()->setDefaultSectionSize(wd);

	if (memViewer->vis) {
		memViewer->move(memViewer->winPos);
		memViewer->show();
	}
	activateWindow();
}

void DebugWin::stop() {
	comp->debug = 0;
	comp->vid->debug = 0;
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
		case XTYPE_DUMP: edt->setInputMask("Hh:hh:hh:hh:hh"); break;
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

QLabel* dbgRegLabs[16];
QLineEdit* dbgRegEdit[16];

DebugWin::DebugWin(QWidget* par):QDialog(par) {
	ui.setupUi(this);

	QLabel* arrl[16] = {
		ui.labReg00, ui.labReg01, ui.labReg02, ui.labReg03, ui.labReg04,
		ui.labReg05, ui.labReg06, ui.labReg07, ui.labReg08, ui.labReg09,
		ui.labReg10, ui.labReg11, ui.labReg12, ui.labReg13, ui.labReg14,
		NULL
	};

	QLineEdit* arre[16] = {
		ui.editReg00, ui.editReg01, ui.editReg02, ui.editReg03, ui.editReg04,
		ui.editReg05, ui.editReg06, ui.editReg07, ui.editReg08, ui.editReg09,
		ui.editReg10, ui.editReg11, ui.editReg12, ui.editReg13, ui.editReg14,
		NULL
	};
	for (int i = 0; i < 16; i++) {
		dbgRegLabs[i] = arrl[i];
		dbgRegEdit[i] = arre[i];
	}

	dumpodel = new xDumpModel((void**)(&comp), dmpmrd, dmpmwr);
	ui.dumpTable->setModel(dumpodel);
	ui.dumpTable->cptr = &comp;

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

	ui.actViewOpcode->setData(DBG_VIEW_CODE);
	ui.actViewByte->setData(DBG_VIEW_BYTE);
	ui.actViewWord->setData(DBG_VIEW_WORD);
	ui.actViewAddr->setData(DBG_VIEW_ADDR);
	ui.actViewText->setData(DBG_VIEW_TEXT);

	ui.actTrace->setData(DBG_TRACE_ALL);
	ui.actTraceHere->setData(DBG_TRACE_HERE);
	ui.actTraceINT->setData(DBG_TRACE_INT);

// disasm table
	ui.dasmTable->setColumnWidth(0,100);
	ui.dasmTable->setColumnWidth(1,85);
	ui.dasmTable->setColumnWidth(2,130);
	ui.dasmTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_LABEL));
	ui.dasmTable->setItemDelegateForColumn(1, new xItemDelegate(XTYPE_DUMP));
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

	ui.tbTool->addAction(ui.actSearch);
	ui.tbTool->addAction(ui.actFill);
	ui.tbTool->addAction(ui.actSprScan);

	ui.tbDbgOpt->addAction(ui.actShowLabels);
	ui.tbDbgOpt->addAction(ui.actHideAddr);
	ui.tbDbgOpt->addAction(ui.actShowSeg);

// connections
	connect(ui.dasmTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDump()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),ui.bpList,SLOT(update()));
	connect(ui.dasmTable,SIGNAL(rqRefillAll()),this,SLOT(fillAll()));

	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(fillDump()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),ui.bpList,SLOT(update()));

	connect(ui.bpList,SIGNAL(rqDisasm()),this,SLOT(fillDisasm()));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDisasm()));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDump()));

	connect(ui.actSearch,SIGNAL(triggered(bool)),this,SLOT(doFind()));
	connect(ui.actFill,SIGNAL(triggered(bool)),this,SLOT(doFill()));
	connect(ui.actSprScan,SIGNAL(triggered(bool)),this,SLOT(doMemView()));

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

	ui.act1251->setData(XCP_1251);
	ui.act866->setData(XCP_866);
	ui.actKoi8r->setData(XCP_KOI8R);
	ui.tbCodepage->addAction(ui.act1251);
	ui.tbCodepage->addAction(ui.act866);
	ui.tbCodepage->addAction(ui.actKoi8r);

	ui.actDumpMem->setData(DMP_MEM);
	ui.actDumpReg->setData(DMP_REG);
	ui.tbDumpView->addAction(ui.actDumpMem);
	ui.tbDumpView->addAction(ui.actDumpReg);

	ui.dumpTable->setColumnWidth(0,60);
	ui.dumpTable->setItemDelegate(new xItemDelegate(XTYPE_BYTE));
	ui.dumpTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_ADR));
	ui.dumpTable->setItemDelegateForColumn(9, new xItemDelegate(XTYPE_NONE));

//	connect(ui.dumpTable,SIGNAL(cellChanged(int,int)),this,SLOT(dumpEdited(int,int)));
	connect(ui.dumpTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));

	connect(ui.tbCodepage, SIGNAL(triggered(QAction*)), this, SLOT(setDumpCP(QAction*)));
	// connect(ui.tbDumpView, SIGNAL(triggered(QAction*)), this, SLOT(setDumpView(QAction*)));

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
	connect(ui.sbScrAdr,SIGNAL(valueChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrAtr,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrPix,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrGrid,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));

	connect(ui.tbAddBrk, SIGNAL(clicked()), this, SLOT(addBrk()));
	connect(ui.tbEditBrk, SIGNAL(clicked()), this, SLOT(editBrk()));
	connect(ui.tbDelBrk, SIGNAL(clicked()), this, SLOT(delBrk()));
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

/*
void DebugWin::wheelEvent(QWheelEvent* ev) {
	if (ev->modifiers() & Qt::ControlModifier) {
		if (ev->delta() < 0) {
			disasmAdr++;
			fillDisasm();
		} else if (ev->delta() > 0) {
			disasmAdr--;
			fillDisasm();
		}
	} else {
		if (ev->delta() < 0) {
			scrollDown();
		} else if (ev->delta() > 0) {
			scrollUp();
		}
	}
}
*/

void DebugWin::setDumpCP(QAction* act) {
	dumpodel->codePage = act->data().toInt();
	switch(dumpodel->codePage) {
		case XCP_1251: ui.tbCodepage->setText("WIN"); break;
		case XCP_866: ui.tbCodepage->setText("DOS"); break;
		case XCP_KOI8R: ui.tbCodepage->setText("KOI"); break;
		default: ui.tbCodepage->setText("???"); break;
	}
	fillDump();
}

/*
void DebugWin::setDumpView(QAction* act) {
	dumpMode = act->data().toInt();
	switch(dumpMode) {
		case DMP_MEM: ui.tbDumpView->setText("MEM"); break;
		case DMP_REG: ui.tbDumpView->setText("REG"); break;
		default: ui.tbDumpView->setText("???"); break;
	}
	fillDump();
}
*/

void DebugWin::doStep() {
	tCount = comp->tickCount;
	compExec(comp);
	if (!fillAll()) {
		disasmAdr = comp->cpu->pc;
		fillDisasm();
	}
	if ((traceType == DBG_TRACE_INT) && (comp->cpu->intrq & comp->cpu->inten))
		trace = 0;
	if ((traceType == DBG_TRACE_HERE) && (comp->cpu->pc == traceAdr))
		trace = 0;
	if (trace) {
		QTimer::singleShot(1,this,SLOT(doStep()));
	} else {
		ui.tbTrace->setEnabled(true);
	}
}

void DebugWin::doTraceHere() {
	doTrace(ui.actTraceHere);
}

void DebugWin::doTrace(QAction* act) {
	if (trace) return;
	trace = 1;
	traceAdr = getAdr();
	traceType = act->data().toInt();
	ui.tbTrace->setEnabled(false);
	doStep();
}

/*
void DebugWin::switchBP(unsigned char mask) {
	if (!ui.dasmTable->hasFocus()) return;
	int adr = getAdr();
	unsigned char* ptr = getBrkPtr(comp, adr);
	if (mask == 0) {
		*ptr = 0;
	} else {
		*ptr ^= mask;
	}
	fillDisasm();
	fillDump();
//	fillBrkTable();
}
*/

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	if (trace) {
		trace = 0;
		return;
	}
	int i;
	unsigned short pc = comp->cpu->pc;
	unsigned char* ptr;
	int offset = (ui.dumpTable->model()->rowCount() - 1) << 3;
	QString com;
	int row;
	int pos;
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
				case Qt::Key_Space:
					//switchBP(0);
					break;
				case Qt::Key_Up:
					disasmAdr--;
					fillDisasm();
					break;
				case Qt::Key_Down:
					disasmAdr++;
					fillDisasm();
					break;
			}
			break;
/*
		case Qt::AltModifier:
			switch (ev->key()) {
				case Qt::Key_F:
					//switchBP(MEM_BRK_FETCH);
					break;
				case Qt::Key_R:
					//switchBP(MEM_BRK_RD);
					break;
				case Qt::Key_W:
					//switchBP(MEM_BRK_WR);
					break;
			}
			break;
*/
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
					comp->cpu->pc = getAdr();
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
//				case Qt::Key_Up:
//					scrollUp();
//					break;
//				case Qt::Key_Down:
//					scrollDown();
//					break;
				case Qt::Key_F3:
					loadFile(comp,"",FT_ALL,-1);
					disasmAdr = comp->cpu->pc;
					fillAll();
					break;
				case Qt::Key_F7:
					doStep();
					break;
				case Qt::Key_F8:
					// if (!ui.dasmTable->hasFocus()) break;
					i = memRd(comp->mem, pc);
					if (((i & 0xc7) == 0xc4) || (i == 0xcd)) {		// call
						ptr = getBrkPtr(comp, pc + 3);
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else if (((i & 0xc7) == 0xc7) || (i == 0x76)) {	// rst, halt
						ptr = getBrkPtr(comp, pc + 1);
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else if (i == 0x10) {
						ptr = getBrkPtr(comp, pc + 2);			// djnz
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else if (i == 0xed) {
						i = memRd(comp->mem, pc + 1);
						if ((i & 0xf4) == 0xb0) {			// block cmds
							ptr = getBrkPtr(comp, pc + 2);
							*ptr ^= MEM_BRK_TFETCH;
							stop();
						} else {
							doStep();
						}
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
	ui.labTcount->setText(QString::number(comp->tickCount - tCount));
	fillCPU();
	fillMem();
	fillDump();
	fillFDC();
	fillGBoy();
	fillAY();
	ui.brkTab->update();
	if (ui.scrLabel->isVisible())
		updateScreen();
	if (comp->rzx.play) fillRZX();
	if (comp->vid->ray.x < comp->vid->ssze.x)
		ui.labRX->setNum(comp->vid->ray.x);
	else
		ui.labRX->setText("HB");
	if (comp->vid->ray.y < comp->vid->ssze.y)
		ui.labRY->setNum(comp->vid->ray.y);
	else
		ui.labRY->setText("VB");
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
void drawGBTile(QImage& img, GBCVid* gbv, int x, int y, int adr) {
	int row, bit;
	int data;
	unsigned char col;
	xColor xcol;
	for (row = 0; row < 8; row++) {
		data = gbv->ram[adr & 0x3fff] & 0xff;
		adr++;
		data |= (gbv->ram[adr & 0x3fff] & 0xff) << 8;
		adr++;
		for (bit = 0; bit < 8; bit++) {
			col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);
			xcol = iniCol[col];
			img.setPixel(x + bit, y + row, qRgb(xcol.r, xcol.g, xcol.b));
			data <<= 1;
		}
	}
}

QImage getGBTiles(GBCVid* gbv, int tset) {
	int tadr = (tset & 1) ? 0x800 : 0;
	if (tset & 2) tadr |= 0x2000;
	int x,y;
	QImage img(128, 128, QImage::Format_RGB888);
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			drawGBTile(img, gbv, x << 3, y << 3, tadr);
			tadr += 16;
		}
	}
	return img.scaled(256,256);
}

QImage getGBMap(GBCVid* gbv, int tmap, int tset) {
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
			tile = gbv->ram[adr & 0x1fff];
			adr++;
			tadr = badr;
			if (tset & 1) {
				tadr += (tile ^ 0x80) << 4;
			} else {
				tadr += tile << 4;
			}
			drawGBTile(img, gbv, x << 3, y << 3, tadr);
		}
	}
	return img;
}

QImage getGBPal(GBCVid* gbv) {
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
		img = getGBTiles(comp->vid->gbc, tset);
	} else if (ui.rbTilemapView->isChecked()) {
		img = getGBMap(comp->vid->gbc, tmap, tset);
	} else {
		img = getGBPal(comp->vid->gbc);
	}
	ui.gbImage->setPixmap(QPixmap::fromImage(img));
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
//			setFlagNames("SZ5H3PNC");
/*
			ui.cpuGrid->setEnabled(true);
			ui.editAFa->setEnabled(true);
			ui.editBCa->setEnabled(true);
			ui.editDEa->setEnabled(true);
			ui.editHLa->setEnabled(true);
			ui.editIX->setEnabled(true);
			ui.editIY->setEnabled(true);
			ui.editIR->setEnabled(true);
*/
			ui.boxIM->setEnabled(true);
			break;
		case CPU_LR35902:
/*
			ui.cpuGrid->setEnabled(true);
			ui.editAFa->setEnabled(false);
			ui.editBCa->setEnabled(false);
			ui.editDEa->setEnabled(false);
			ui.editHLa->setEnabled(false);
			ui.editIX->setEnabled(false);
			ui.editIY->setEnabled(false);
			ui.editIR->setEnabled(false);
*/
			ui.boxIM->setEnabled(false);
//			setFlagNames("ZNHC----");
			break;
		case CPU_6502:
			ui.boxIM->setEnabled(false);
//			setFlagNames("MV5BDIZC");
			break;
		default:
//			ui.cpuGrid->setEnabled(false);
//			setFlagNames("--------");
			break;
	}
}

// rzx

void dbgSetRzxIO(QLabel* lab, Computer* comp, int pos) {
	if (pos < comp->rzx.frm.size) {
		lab->setText(gethexbyte(comp->rzx.frm.data[pos]));
	} else {
		lab->setText("--");
	}
}

void DebugWin::fillRZX() {
/*
	ui.rzxFrm->setText(QString::number(comp->rzx.frame).append(" / ").append(QString::number(comp->rzx.size)));
	ui.rzxFetch->setText(QString::number(comp->rzx.fetches));
	ui.rzxFSize->setText(QString::number(comp->rzx.data[comp->rzx.frame].frmSize));
	int pos = comp->rzx.pos;
	dbgSetRzxIO(ui.rzxIO1, comp, pos++);
	dbgSetRzxIO(ui.rzxIO2, comp, pos++);
	dbgSetRzxIO(ui.rzxIO3, comp, pos++);
	dbgSetRzxIO(ui.rzxIO4, comp, pos++);
	dbgSetRzxIO(ui.rzxIO5, comp, pos);
*/
}

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
				dbgRegEdit[i]->setEnabled(false);
				dbgRegEdit[i]->clear();
				break;
			default:
				dbgRegLabs[i]->setText(bunch.regs[i].name);
				dbgRegEdit[i]->setProperty("regid", bunch.regs[i].id);
				if (bunch.regs[i].byte) {
					dbgRegEdit[i]->setInputMask("HH");
					dbgRegEdit[i]->setText(gethexbyte(bunch.regs[i].value & 0xff));
				} else {
					dbgRegEdit[i]->setInputMask("HHHH");
					dbgRegEdit[i]->setText(gethexword(bunch.regs[i].value));
				}
				dbgRegEdit[i]->setEnabled(true);
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
			bunch.regs[i].value = dbgRegEdit[idx]->text().toInt(NULL, 16);
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
	res.append(gethexbyte(pg.num));
	return res;
}

void DebugWin::fillMem() {
	ui.labPG0->setText(getPageName(comp->mem->map[0]));
	ui.labPG1->setText(getPageName(comp->mem->map[1]));
	ui.labPG2->setText(getPageName(comp->mem->map[2]));
	ui.labPG3->setText(getPageName(comp->mem->map[3]));
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
		int bit = 0b011;			// b0:ram cells, b1:rom cells, b2:slt cells
		if (comp->slot->brkMap) bit |= 0b100;
		strm << bit;
		fwritepack(strm, comp->brkRamMap, 0x400000);
		fwritepack(strm, comp->brkRomMap, 0x80000);
		if (bit & 0b100) fwritepack(strm, comp->slot->brkMap, comp->slot->memMask + 1);
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
				if (bt & 1) freadpack(strm, comp->brkRamMap, 0x400000);
				if (bt & 2) freadpack(strm, comp->brkRomMap, 0x80000);
				if ((bt & 4) && comp->slot->brkMap) freadpack(strm, comp->slot->brkMap, comp->slot->memMask + 1);
			}

		} else {
			shitHappens("Wrong signature");
		}
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

void DebugWin::dasmEdited(int row, int col) {
/*
	if (block) return;
	int adr = getAdr();
	char buf[8];
	int len;
	int idx;
	unsigned char cbyte;
	unsigned char* ptr;
	bool flag;
	QString str;
	QString lab;
	QStringList lst;
	xAdr xadr = memGetXAdr(comp->mem, adr);
	switch (col) {
		case 0:
			str = ui.dasmTable->item(row, 0)->text();
			if (str.isEmpty()) {
				str = findLabel(xadr.adr, xadr.type, xadr.bank);
				if (!str.isEmpty()) {
					labels.remove(str);
				}
			} else {
				idx = str.toInt(&flag, 16);
				if (flag) {				// is number
					adr = idx;
				} else if (labels.contains(str)) {	// is existing label
					adr = labels[str].adr;
				} else {				// else create a new label
					// xadr = memGetXAdr(comp->mem, adr);
					lab = findLabel(xadr.adr, xadr.type, xadr.bank);	// old label was here (if any)
					if (!lab.isEmpty())
						labels.remove(lab);
					labels[str] = xadr;
				}
				while (row > 0) {
					adr = getPrevAdr(adr);
					row--;
				}
				disasmAdr = adr;
			}
			break;
		case 1:
			lst = ui.dasmTable->item(row, col)->text().split(":", QString::SkipEmptyParts);
			foreach(str, lst) {
				cbyte = str.toInt(NULL,16);
				memWr(comp->mem, adr, cbyte);
				adr++;
			}
			break;
		case 2:
			str = ui.dasmTable->item(row, col)->text();
			if (!str.startsWith("db \"", Qt::CaseInsensitive))		// #NUM -> 0xNUM if not db "..."
				str.replace("#", "0x");
			if (str.startsWith("db ", Qt::CaseInsensitive)) {		// db
				if ((str.at(3) == '"') && str.endsWith("\"")) {		// db "text"
					str = str.mid(4, str.size() - 5);
					idx = 0;
					cbyte = str.at(idx).cell();
					while ((idx < 250) && (idx < str.size()) && (cbyte > 31) && (cbyte < 128)) {
						buf[idx] = cbyte;
						ptr = getBrkPtr(comp, (adr + idx) & 0xffff);
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_TEXT;
						idx++;
						cbyte = str.at(idx).cell();
					}
					len = idx;
				} else {						// db n
					str = str.mid(3);
					idx = str.toInt(&flag, 0);
					if (flag) {
						len = 1;
						buf[0] = idx & 0xff;
						chaCellProperty(ui.actViewByte);
					} else {
						len = 0;
					}
				}
			} else if (str.startsWith("dw ", Qt::CaseInsensitive)) {	// word/addr
				str = str.mid(3);
				if (labels.contains(str)) {				// check label
					idx = labels[str].adr;
					chaCellProperty(ui.actViewAddr);
					len = 2;
				} else {
					idx = str.toInt(&flag, 0);
					if (flag) {
						len = 2;
						chaCellProperty(ui.actViewWord);
					} else {
						len = 0;
					}
				}
				if (len > 0) {
					buf[0] = idx & 0xff;
					buf[1] = (idx >> 8) & 0xff;
				}
			} else {			// byte
				len = cpuAsm(comp->cpu, str.toLocal8Bit().data(), buf, adr);
				if (len > 0)
					chaCellProperty(ui.actViewOpcode);
			}
			idx = 0;
			while (idx < len) {
				memWr(comp->mem, (adr + idx) & 0xffff, buf[idx]);
				idx++;
			}
			break;
	}
	fillDump();
	fillDisasm();
	updateScreen();
*/
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
	dumpodel->update();

/*
	unsigned short adr = dumpAdr;
	int row,col;
	QColor bgcol, ccol;
	for (row = 0; row < ui.dumpTable->rowCount(); row++) {
		ui.dumpTable->item(row,0)->setText(gethexword(adr));
		ui.dumpTable->item(row,9)->setText(getDumpString(comp->mem, adr, codePage));
		bgcol = (row & 1) ? colBG0 : colBG1;
		ui.dumpTable->item(row, 0)->setBackground(bgcol);
		ui.dumpTable->item(row, 9)->setBackground(bgcol);
		for (col = 1; col < 9; col++) {
			if (getBrk(comp, adr) & MEM_BRK_ANY) {
				ccol = colBRK;
			} else if ((adr >= ui.dasmTable->blockStart) && (adr <= ui.dasmTable->blockEnd)) {
				ccol = colSEL;
			} else {
				ccol = bgcol;
			}
			ui.dumpTable->item(row,col)->setBackgroundColor(ccol);
			ui.dumpTable->item(row,col)->setText(gethexbyte(memRd(comp->mem, adr)));
			adr++;
		}
	}
*/
	fillStack();
	block = 0;
}

void DebugWin::dumpEdited(int row, int col) {
/*
	if (block) return;
	if (col == 0) {
		dumpAdr = ui.dumpTable->item(row, 0)->text().toInt(NULL,16) - row * 8;
	} else if (col < 9) {
		unsigned short adr = (dumpAdr + (col - 1) + row * 8) & 0xffff;
		memWr(comp->mem, adr, ui.dumpTable->item(row, col)->text().toInt(NULL,16) & 0xff);
//		fillDisasm();

		col++;
		if (col > 8) {
			col = 1;
			row++;
			if (row >= ui.dumpTable->rowCount()) {
				row--;
				dumpAdr += 8;
			}
		}
		// ui.dumpTable->selectionModel()->select(ui.dumpTable->model()->index(row,col), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
		ui.dumpTable->setCurrentCell(row,col);
	}
	fillDump();
	fillDisasm();
	updateScreen();
*/
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
	MemPage curBank = comp->mem->map[MEM_BANK3];
	int bank = dui.leBank->text().toInt(NULL,16);
	int adr = dui.leStart->text().toInt(NULL,16);
	int len = dui.leLen->text().toInt(NULL,16);
	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, bank, NULL, NULL, NULL);
	QByteArray res;
	while (len > 0) {
		res.append(memRd(comp->mem,adr));
		adr++;
		len--;
	}
	comp->mem->map[3] = curBank;
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
	memViewer->ui.sbPage->setValue(comp->mem->map[3].num);
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
	vidGetScreen(comp->vid, scrImg.bits(), ui.sbScrBank->value(), ui.sbScrAdr->value(), flag);
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
