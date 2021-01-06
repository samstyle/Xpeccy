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

static QColor colPC(32,200,32);	// pc
static QColor colBRK(200,128,128);	// breakpoint
static QColor colBG0(255,255,255);	// background
static QColor colBG1(230,230,230);	// background alternative
static QColor colSEL(128,255,128);	// background selected

int blockStart = -1;
int blockEnd = -1;

int getRFIData(QComboBox*);
void setRFIndex(QComboBox* box, QVariant data);
int dasmSome(Computer*, unsigned short, dasmData&);

// trace type
enum {
	DBG_TRACE_ALL = 0x100,
	DBG_TRACE_INT,
	DBG_TRACE_KEY,
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

#define SETCOLOR(_n, _r) col = conf.pal[_n]; if (col.isValid()) pal.setColor(_r, col)

void DebugWin::chaPal() {
	QColor col;
	QColor cot;
	QPalette pal;
	SETCOLOR("dbg.window", QPalette::Window);
	SETCOLOR("dbg.window", QPalette::Button);
	SETCOLOR("dbg.text", QPalette::WindowText);
	SETCOLOR("dbg.input.bg", QPalette::Base);
	setPalette(pal);

	col = conf.pal["dbg.header.bg"];
	cot = conf.pal["dbg.header.txt"];
	QString str = QString("background-color:%0;color:%1").arg(col.name()).arg(cot.name());
	ui.labHeadCpu->setStyleSheet(str);
	ui.labHeadDump->setStyleSheet(str);
	ui.labHeadDisasm->setStyleSheet(str);
	ui.labHeadMem->setStyleSheet(str);
	ui.labHeadRay->setStyleSheet(str);
	ui.labHeadStack->setStyleSheet(str);
	ui.labHeadSignal->setStyleSheet(str);

	fillAll();
}

void DebugWin::save_mem_map() {
	for (int i = 0; i < 256; i++) {
		mem_map[i] = comp->mem->map[i];
	}
}

void DebugWin::rest_mem_map() {
	for (int i = 0; i < 256; i++) {
		 comp->mem->map[i] = mem_map[i];
	}
	fillAll();
}

void DebugWin::remapMem() {
	if (block) return;
	memSetBank(comp->mem, 0x00, getRFIData(ui.cbBank0), ui.numBank0->getValue(), MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x40, getRFIData(ui.cbBank1), ui.numBank1->getValue(), MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, getRFIData(ui.cbBank2), ui.numBank2->getValue(), MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xc0, getRFIData(ui.cbBank3), ui.numBank3->getValue(), MEM_16K, NULL, NULL, NULL);
	fillAll();
}

void DebugWin::start(Computer* c) {
	blockStart = -1;
	blockEnd = -1;
	save_mem_map();
	chLayout();
	if (!comp->vid->tail)
		vidDarkTail(comp->vid);

	this->move(winPos);
	comp->vid->debug = 1;
	comp->debug = 1;
	comp->brk = 0;

	if (comp->hw->grp != tabMode) {
		onPrfChange(conf.prof.cur);		// update tabs
	}

	chaPal();
	show();
	if (!fillAll()) {
		ui.dasmTable->setAdr(comp->cpu->pc);
		// fillDisasm();
	}
	updateScreen();

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
	// rest_mem_map();
	compExec(comp);		// to prevent double breakpoint catch
	comp->debug = 0;
	comp->vid->debug = 0;
	comp->maping = ui.actMaping->isChecked() ? 1 : 0;
	tCount = comp->tickCount;
	winPos = pos();
	stopTrace();

	memViewer->vis = memViewer->isVisible() ? 1 : 0;
	memViewer->winPos = memViewer->pos();

	memViewer->hide();
	hide();
	emit closed();
}

void DebugWin::onPrfChange(xProfile* prf) {
	if (!prf) prf = conf.prof.cur;
	if (!prf) return;
	//if (!isHidden())
	//	rest_mem_map();
	comp = prf->zx;
	save_mem_map();
	ui.tabsPanel->clear();
	QList<QPair<QIcon, QWidget*> > lst = tablist[prf->zx->hw->grp];
	QPair<QIcon, QWidget*> p;
	p.first = QIcon(":/images/stop.png");
	p.second = ui.brkTab;
	lst.append(p);
	p.first = QIcon(":/images/memory.png");
	p.second = ui.memTab;
	lst.append(p);
	while(lst.size() > 0) {
		ui.tabsPanel->addTab(lst.first().second, lst.first().first, "");
		lst.removeFirst();
	}
	ui.tabsPanel->setPalette(QPalette());
	tabMode = prf->zx->hw->grp;
	// set input line base
	for (int i = 0; i < 16; i++) {
		if (dbgRegEdit[i] != NULL) {
			dbgRegEdit[i]->setBase(conf.prof.cur->zx->hw->base);
		}
	}
	switch(conf.prof.cur->zx->hw->base) {
		case 8:
			ui.dumpTable->setItemDelegateForColumn(1, xid_octw);
			ui.dumpTable->setItemDelegateForColumn(3, xid_octw);
			ui.dumpTable->setItemDelegateForColumn(5, xid_octw);
			ui.dumpTable->setItemDelegateForColumn(7, xid_octw);
			ui.dumpTable->setView(XVIEW_OCTWRD);
			break;
		default:
			ui.dumpTable->setItemDelegateForColumn(1, xid_byte);
			ui.dumpTable->setItemDelegateForColumn(3, xid_byte);
			ui.dumpTable->setItemDelegateForColumn(5, xid_byte);
			ui.dumpTable->setItemDelegateForColumn(7, xid_byte);
			ui.dumpTable->setView(XVIEW_DEF);
			break;
	}
	fillAll();
}

void DebugWin::reject() {stop();}

xItemDelegate::xItemDelegate(int t) {
	type = t;
}

QWidget* xItemDelegate::createEditor(QWidget* par, const QStyleOptionViewItem&, const QModelIndex&) const {
	QLineEdit* edt = new QLineEdit(par);
	QString pat("[0-9A-Fa-f\\s]");
	int rpt = 0;
	switch (type) {
		case XTYPE_NONE: delete(edt); edt = NULL; break;
		case XTYPE_ADR: rpt = 4; break;
		case XTYPE_LABEL: break;
		case XTYPE_DUMP: rpt = 12; break;		// 6 bytes max
		case XTYPE_BYTE: rpt = 2; break;
		case XTYPE_OCTWRD: pat = "[0-7\\s]"; rpt = 6; break;
	}
	if (edt && (rpt > 0)) {
		edt->setInputMask(QString(rpt,'h'));
		edt->setMaxLength(rpt);
		edt->setValidator(new QRegExpValidator(QRegExp(QString("%0+").arg(pat))));
	}
	return edt;
}

/*
int dmpmrd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	MemPage* pg = &comp->mem->map[(adr >> 8) & 0xff];
	int fadr = (pg->num << 8) | (adr & 0xff);
	int res = 0xff;
	switch (pg->type) {
		case MEM_RAM:
			res = comp->mem->ramData[fadr & comp->mem->ramMask];
			break;
		case MEM_ROM:
			res = comp->mem->romData[fadr & comp->mem->romMask];
			break;
		case MEM_SLOT:
			if (!comp->slot) break;
			if (!comp->slot->data) break;
			res = comp->slot->data[fadr & comp->slot->memMask];
			break;
	}
	res |= getBrk(comp, adr) << 8;
	return res;
}

void dmpmwr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	memWr(comp->mem, adr, val);
}
*/

DebugWin::DebugWin(QWidget* par):QDialog(par) {
	int i;

	setFont(QFont("://DejaVuSansMono.ttf",10));
	ui.setupUi(this);

	dumpwin = new QDialog(this);

	tabMode = HW_NULL;

	QList<QPair<QIcon, QWidget*> > lst;
	QPair<QIcon, QWidget*> p;
	tablist.clear();

	p.first = QIcon(":/images/display.png"); p.second = ui.scrTab; lst.append(p);
	p.first = QIcon(":/images/speaker2.png"); p.second = ui.ayTab; lst.append(p);
	p.first = QIcon(":/images/tape.png"); p.second = ui.tapeTab; lst.append(p);
	p.first = QIcon(":/images/floppy.png"); p.second = ui.fdcTab; lst.append(p);
	tablist[HWG_ZX] = lst;
	lst.clear();
	p.first = QIcon(":/images/nespad.png"); p.second = ui.nesTab; lst.append(p);
	tablist[HWG_NES] = lst;
	lst.clear();
	p.first = QIcon(":/images/gameboy.png"); p.second = ui.gbTab; lst.append(p);
	tablist[HWG_GB] = lst;
	lst.clear();
	p.first = QIcon(":/images/tape.png"); p.second = ui.tapeTab; lst.append(p);
	p.first = QIcon(":/images/floppy.png"); p.second = ui.fdcTab; lst.append(p);
	tablist[HWG_BK] = lst;
	lst.clear();
	p.first = QIcon(":/images/commodore.png"); p.second = ui.ciaTab; lst.append(p);
	p.first = QIcon(":/images/tape.png"); p.second = ui.tapeTab; lst.append(p);
	tablist[HWG_COMMODORE] = lst;
	lst.clear();
	p.first = QIcon(":/images/speaker2.png"); p.second = ui.ayTab; lst.append(p);
	p.first = QIcon(":/images/tape.png"); p.second = ui.tapeTab; lst.append(p);
	tablist[HWG_MSX] = lst;
	lst.clear();
	p.first = QIcon(":/images/tape.png"); p.second = ui.tapeTab; lst.append(p);
	tablist[HWG_SPCLST] = lst;

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
			dbgRegEdit[i]->setXFlag(XHS_BGR | XHS_DEC | XHS_FILL);
		}
	}

	ui.dumpTable->setComp(&comp);
	ui.dasmTable->setComp(&comp);

	conf.dbg.labels = 1;
	conf.dbg.segment = 0;
	ui.actShowLabels->setChecked(conf.dbg.labels);
	ui.actShowSeg->setChecked(conf.dbg.segment);

	xid_none = new xItemDelegate(XTYPE_NONE);
	xid_byte = new xItemDelegate(XTYPE_BYTE);
	xid_labl = new xItemDelegate(XTYPE_LABEL);
	xid_octw = new xItemDelegate(XTYPE_OCTWRD);
	xid_dump = new xItemDelegate(XTYPE_DUMP);

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
	ui.dasmTable->setItemDelegateForColumn(0, xid_labl);
	ui.dasmTable->setItemDelegateForColumn(1, xid_dump);

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
//	ui.tbSaveDasm->addAction(ui.actLoadMap);
	ui.tbSaveDasm->addAction(ui.actLoadLabels);
	ui.tbSaveDasm->addAction(ui.actSaveDump);
//	ui.tbSaveDasm->addAction(ui.actSaveMap);
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
	ui.tbTool->addAction(ui.actWutcha);

	ui.tbDbgOpt->addAction(ui.actShowLabels);
	ui.tbDbgOpt->addAction(ui.actHideAddr);
	ui.tbDbgOpt->addAction(ui.actShowSeg);
	ui.tbDbgOpt->addAction(ui.actRomWr);
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

	connect(ui.bpList,SIGNAL(rqDisasm(int)),ui.dasmTable,SLOT(setAdr(int)));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDisasm()));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDump()));

	connect(ui.actSearch,SIGNAL(triggered(bool)),this,SLOT(doFind()));
	connect(ui.actFill,SIGNAL(triggered(bool)),this,SLOT(doFill()));
	connect(ui.actSprScan,SIGNAL(triggered(bool)),this,SLOT(doMemView()));
	connect(ui.actShowKeys,SIGNAL(triggered(bool)),this,SIGNAL(wannaKeys()));
	connect(ui.actWutcha,SIGNAL(triggered(bool)),this,SIGNAL(wannaWutch()));

	connect(ui.actShowLabels,SIGNAL(toggled(bool)),this,SLOT(setShowLabels(bool)));
	connect(ui.actHideAddr,SIGNAL(toggled(bool)),this,SLOT(fillDisasm()));
	connect(ui.actShowSeg,SIGNAL(toggled(bool)),this,SLOT(setShowSegment(bool)));
	connect(ui.actRomWr, SIGNAL(toggled(bool)),this,SLOT(setRomWriteable(bool)));

	connect(ui.tbView, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui.tbBreak, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui.tbTrace, SIGNAL(triggered(QAction*)),this,SLOT(doTrace(QAction*)));

	connect(ui.actLoadDump, SIGNAL(triggered(bool)),this,SLOT(doOpenDump()));
	connect(ui.actSaveDump, SIGNAL(triggered(bool)),dumpwin,SLOT(show()));
	connect(ui.actLoadLabels, SIGNAL(triggered(bool)),this,SLOT(dbgLLab()));
	connect(ui.actSaveLabels, SIGNAL(triggered(bool)),this,SLOT(dbgSLab()));
//	connect(ui.actLoadMap, SIGNAL(triggered(bool)),this,SLOT(loadMap()));
//	connect(ui.actSaveMap, SIGNAL(triggered(bool)),this,SLOT(saveMap()));
	connect(ui.actDisasm, SIGNAL(triggered(bool)),this,SLOT(saveDasm()));
	connect(ui.tbRefresh, SIGNAL(released()), this, SLOT(reload()));

// dump table

	ui.cbCodePage->addItem("WIN", XCP_1251);
	ui.cbCodePage->addItem("DOS", XCP_866);
	ui.cbCodePage->addItem("KOI8R", XCP_KOI8R);

	ui.cbDumpView->addItem("CPU", XVIEW_CPU);
	ui.cbDumpView->addItem("RAM", XVIEW_RAM);
	ui.cbDumpView->addItem("ROM", XVIEW_ROM);

	ui.dumpTable->setColumnWidth(0,60);
	ui.dumpTable->setItemDelegate(xid_byte);
	ui.dumpTable->setItemDelegateForColumn(0, xid_labl);
	ui.dumpTable->setItemDelegateForColumn(9, xid_none);

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

	connect(ui.tabsPanel, SIGNAL(currentChanged(int)), this, SLOT(fillAll()));

	// setFixedSize(size());
	setFixedHeight(size().height());
	block = 0;
	// ui.dasmTable->setAdr(0);
	// ui.dumpTable->setAdr(0);
	tCount = 0;
	trace = 0;
// nes tab
	ui.nesScrType->addItem("BG off", NES_SCR_OFF);
	ui.nesScrType->addItem("BG scr 0", NES_SCR_0);
	ui.nesScrType->addItem("BG scr 1", NES_SCR_1);
	ui.nesScrType->addItem("BG scr 2", NES_SCR_2);
	ui.nesScrType->addItem("BG scr 3", NES_SCR_3);
	ui.nesScrType->addItem("All in 1", NES_SCR_ALL);

	ui.nesBGTileset->addItem("Tiles #0000", NES_TILE_0000);
	ui.nesBGTileset->addItem("Tiles #1000", NES_TILE_1000);

	connect(ui.nesScrType,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));
	connect(ui.nesBGTileset,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));
// mem tab
	ui.cbBank0->addItem("ROM", MEM_ROM);
	ui.cbBank0->addItem("RAM", MEM_RAM);
	ui.cbBank1->addItem("ROM", MEM_ROM);
	ui.cbBank1->addItem("RAM", MEM_RAM);
	ui.cbBank2->addItem("ROM", MEM_ROM);
	ui.cbBank2->addItem("RAM", MEM_RAM);
	ui.cbBank3->addItem("ROM", MEM_ROM);
	ui.cbBank3->addItem("RAM", MEM_RAM);
	ui.numBank0->setMax(255);
	ui.numBank1->setMax(255);
	ui.numBank2->setMax(255);
	ui.numBank3->setMax(255);
	connect(ui.cbBank0, SIGNAL(currentIndexChanged(int)), this, SLOT(remapMem()));
	connect(ui.cbBank1, SIGNAL(currentIndexChanged(int)), this, SLOT(remapMem()));
	connect(ui.cbBank2, SIGNAL(currentIndexChanged(int)), this, SLOT(remapMem()));
	connect(ui.cbBank3, SIGNAL(currentIndexChanged(int)), this, SLOT(remapMem()));
	connect(ui.numBank0, SIGNAL(valueChanged(int)), this, SLOT(remapMem()));
	connect(ui.numBank1, SIGNAL(valueChanged(int)), this, SLOT(remapMem()));
	connect(ui.numBank2, SIGNAL(valueChanged(int)), this, SLOT(remapMem()));
	connect(ui.numBank3, SIGNAL(valueChanged(int)), this, SLOT(remapMem()));
	connect(ui.pbRestMemMap, SIGNAL(clicked()), this, SLOT(rest_mem_map()));

// subwindows
	dui.setupUi(dumpwin);
	dui.tbSave->addAction(dui.aSaveBin);
	dui.tbSave->addAction(dui.aSaveHobeta);
	dui.tbSave->addAction(dui.aSaveToA);
	dui.tbSave->addAction(dui.aSaveToB);
	dui.tbSave->addAction(dui.aSaveToC);
	dui.tbSave->addAction(dui.aSaveToD);
	dui.leStart->setMin(0);
	dui.leStart->setMax(0xffff);
	dui.leEnd->setMin(0);
	dui.leEnd->setMax(0xffff);
	dui.leLen->setMin(1);
	dui.leLen->setMax(0x10000);

	connect(dui.aSaveBin,SIGNAL(triggered()),this,SLOT(saveDumpBin()));
	connect(dui.aSaveHobeta,SIGNAL(triggered()),this,SLOT(saveDumpHobeta()));
	connect(dui.aSaveToA,SIGNAL(triggered()),this,SLOT(saveDumpToA()));
	connect(dui.aSaveToB,SIGNAL(triggered()),this,SLOT(saveDumpToB()));
	connect(dui.aSaveToC,SIGNAL(triggered()),this,SLOT(saveDumpToC()));
	connect(dui.aSaveToD,SIGNAL(triggered()),this,SLOT(saveDumpToD()));
	connect(dui.leStart,SIGNAL(valueChanged(int)),this,SLOT(dmpLimChanged()));
	connect(dui.leEnd,SIGNAL(valueChanged(int)),this,SLOT(dmpLimChanged()));
	connect(dui.leLen,SIGNAL(valueChanged(int)),this,SLOT(dmpLenChanged()));

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

void DebugWin::setRomWriteable(bool f) {
	conf.dbg.romwr = f ? 1 : 0;
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
//	int mode = getRFIData(ui.cbDasmMode);
//	int page = ui.sbDasmPage->value();
//	ui.sbDasmPage->setDisabled(mode == XVIEW_CPU);
//	ui.dasmTable->setMode(mode, page);
}

static QFile logfile;

void DebugWin::doStep() {
		tCount = comp->tickCount;
		compExec(comp);
		if (!fillAll()) {
			ui.dasmTable->setAdr(comp->cpu->pc);
			//fillDisasm();
		}
}

void DebugWin::doTraceHere() {
	doTrace(ui.actTraceHere);
}

void DebugWin::doTrace(QAction* act) {
	if (trace) return;

	traceType = act->data().toInt();

	if (traceType == DBG_TRACE_LOG) {
		QString path = QFileDialog::getSaveFileName(this, "Log file",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
		if (path.isEmpty()) return;
		logfile.setFileName(path);
		if (!logfile.open(QFile::WriteOnly)) return;
	}

	trace = 1;
	traceAdr = getAdr();
	ui.tbTrace->setEnabled(false);
	QApplication::postEvent(this, new QEvent((QEvent::Type)DBG_EVENT_STEP));
}

void DebugWin::stopTrace() {
	trace = 0;
	ui.tbTrace->setEnabled(true);
	if (logfile.isOpen()) logfile.close();
}

void DebugWin::reload() {
	if (comp->mem->snapath) {
		load_file(comp, comp->mem->snapath, FG_SNAPSHOT, 0);
		ui.dasmTable->setAdr(comp->cpu->pc);
		fillAll();
	}
	if (!conf.labpath.isEmpty())
		loadLabels(conf.labpath.toLocal8Bit().data());
}

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	if (trace && !ev->isAutoRepeat()) {
		stopTrace();
		return;
	}
	int i;
	int key = shortcut_check(SCG_DEBUGA, QKeySequence(ev->key() | ev->modifiers()));
	if (key < 0)
		key = ev->key() | ev->modifiers();
	unsigned char* ptr;
	int len;
	dasmData drow;
	QModelIndex idx;
	switch (key) {
		case XCUT_OPTIONS:
			emit wannaOptions(conf.prof.cur);
			break;
		case XCUT_LOAD:
			load_file(comp, NULL, FG_ALL, -1);
			ui.dasmTable->setAdr(comp->cpu->pc);
			//fillAll();
			activateWindow();
			break;
		case XCUT_SAVE:
			save_file(comp, NULL, FG_ALL, -1);
			activateWindow();
			break;
		case XCUT_STEPIN:
			if (!ev->isAutoRepeat()) {
				doStep();
			} else if (!trace) {
				doTrace(ui.actTrace);
			}
			break;
		case XCUT_STEPOVER:
			len = dasmSome(comp, comp->cpu->pc, drow);
			if (drow.oflag & OF_SKIPABLE) {
				ptr = getBrkPtr(comp, (comp->cpu->pc + len) & 0xffff);
				*ptr |= MEM_BRK_TFETCH;
				stop();
			} else {
				doStep();
			}
			break;
		case XCUT_FASTSTEP:
			for (i = 10; i > 0; i--)
				doStep();
			break;
		case XCUT_TMPBRK:
			if (!ui.dasmTable->hasFocus()) break;
			idx = ui.dasmTable->currentIndex();
			i = ui.dasmTable->getData(idx.row(), 0, Qt::UserRole).toInt();
			ptr = getBrkPtr(comp, i & 0xffff);
			*ptr |= MEM_BRK_TFETCH;
			stop();
			break;
		case XCUT_RESET:
			rzxStop(comp);
			compReset(comp, RES_DEFAULT);
			if (!fillAll()) {
				ui.dasmTable->setAdr(comp->cpu->pc);
				//fillDisasm();
			}
			break;
		case XCUT_TRACE:
			doTrace(ui.actTrace);
			break;
		case XCUT_LABELS:
			ui.actShowLabels->setChecked(!conf.dbg.labels);
			break;
		case XCUT_KEYBOARD:
			emit wannaKeys();
			break;
		case XCUT_OPEN_DUMP:
			doOpenDump();
			break;
		case XCUT_SAVE_DUMP:
			//doSaveDump();
			dumpwin->show();
			break;
		case XCUT_FINDER:
			doFind();
			break;
		case XCUT_DEBUG:
			if (!ev->isAutoRepeat())
				stop();
			break;
	}
}

void DebugWin::keyReleaseEvent(QKeyEvent* ev) {
	QKeySequence seq(ev->key() | ev->modifiers());
	if (!ev->isAutoRepeat() && (shortcut_match(SCG_DEBUGA, XCUT_STEPIN, seq) != QKeySequence::NoMatch)) {
		stopTrace();
	}
}

void DebugWin::customEvent(QEvent* ev) {
	switch(ev->type()) {
		case DBG_EVENT_STEP:
			doStep();
			switch(traceType) {
				case DBG_TRACE_INT:
					if (comp->cpu->intrq & comp->cpu->inten)
						stopTrace();
					break;
				case DBG_TRACE_HERE:
					if (comp->cpu->pc == traceAdr)
						stopTrace();
					break;
			}
			if (trace) {
				QApplication::postEvent(this, new QEvent((QEvent::Type)DBG_EVENT_STEP));
			}
			break;
		default:
			break;
	}
}

void setSignal(QLabel* lab, int on) {
	QFont fnt = lab->font();
	fnt.setBold(on);
	lab->setFont(fnt);
}

QString getAYmix(aymChan* ch) {
	QString res = ch->tdis ? "-" : "T";
	res += ch->ndis ? "-" : "N";
	res += ch->een ? "E" : "-";
	return res;
}

void drawBar(QLabel* lab, int lev, int max) {
	if (lev > max) lev = max;
	if (lev < 0) lev = 0;
	QPixmap pxm(100, lab->height() / 2);
	QPainter pnt;
	pxm.fill(Qt::black);
	pnt.begin(&pxm);
	pnt.fillRect(0,0,pxm.width() * lev / max, pxm.height(), Qt::green);
	pnt.setPen(Qt::red);
	pnt.drawLine(pxm.width() / 2, 0, pxm.width() / 2, pxm.height());
	pnt.end();
	lab->setPixmap(pxm);
}

void DebugWin::fillAY() {
	if (ui.tabsPanel->currentWidget() != ui.ayTab) return;
	aymChip* chp = comp->ts->chipA;
	ui.leToneA->setText(gethexword(((chp->reg[1] << 8) | chp->reg[0]) & 0x0fff));
	ui.leToneB->setText(gethexword(((chp->reg[3] << 8) | chp->reg[2]) & 0x0fff));
	ui.leToneC->setText(gethexword(((chp->reg[5] << 8) | chp->reg[4]) & 0x0fff));
	ui.leVolA->setText(gethexbyte(chp->reg[8] & 0x0f));
	ui.leVolB->setText(gethexbyte(chp->reg[9] & 0x0f));
	ui.leVolC->setText(gethexbyte(chp->reg[10] & 0x0f));
	ui.leMixA->setText(getAYmix(&chp->chanA));
	ui.leMixB->setText(getAYmix(&chp->chanB));
	ui.leMixC->setText(getAYmix(&chp->chanC));
	ui.leToneN->setText(gethexbyte(chp->reg[6]));
	ui.leEnvTone->setText(gethexword((chp->reg[12] << 8) | chp->reg[11]));
	ui.leEnvForm->setText(gethexbyte(chp->reg[13]));
	ui.leVolE->setText(gethexbyte(chp->chanE.vol));
	ui.labLevA->setText(chp->chanA.lev ? "1" : "0");
	ui.labLevB->setText(chp->chanB.lev ? "1" : "0");
	ui.labLevC->setText(chp->chanC.lev ? "1" : "0");
	ui.labLevN->setText(chp->chanN.lev ? "1" : "0");

	drawBar(ui.labBeep, comp->beep->val, 256);
}

#define TDSTEP 20	// mks/dot

void DebugWin::fillTape() {
	if (ui.tabsPanel->currentWidget() != ui.tapeTab) return;
	Tape* tape = comp->tape;
	drawBar(ui.labTapein, tape->volPlay, 256);
	drawBar(ui.labTapeout, tape->levRec, 1);
	ui.labSigLen->setText(tape->on ? QString("%0 mks").arg(tape->sigLen) : "");
	ui.labTapeState->setText(tape->on ? (tape->rec ? "rec" : "play") : "stop");
	ui.labTapePos->setText(tape->on ? QString::number(tape->pos - 1) : "--");
	// draw tape diagram
	int wid = 330;
	int hei = 100;
	int pos;
	int bnr;
	int amp;
	int oamp = -1;
	int x = wid / 2;
	int time = 0;
	TapeBlock* blk;
	QPixmap pxm(wid, hei);
	QPainter pnt;
	pxm.fill(Qt::black);
	pnt.begin(&pxm);
	pnt.setPen(Qt::red);
	pnt.drawLine(0, hei/2, wid, hei/2);
	pnt.drawLine(wid / 2, 0, wid/2, hei);
	pnt.setPen(Qt::green);
	if (tape->blkCount > 0) {
		bnr = tape->block;
		blk = &tape->blkData[bnr];
		pos = tape->pos;
		time = tape->sigLen + (wid / 2) * TDSTEP;
		while ((time >= 0) && (blk != NULL)) {
			pos--;
			if (pos < 0) {
				bnr--;
				if (bnr < 0) {
					blk = NULL;
				} else {
					blk = &tape->blkData[bnr];
					pos = blk->sigCount - 1;
				}
			} else {
				time -= blk->data[pos].size;
			}
		}
		if (blk == NULL) {
			bnr = 0;
			pos = 0;
			blk = &tape->blkData[bnr];
			x = time / TDSTEP;		// skip
			time = blk->data[pos].size;
		} else {
			x = 0;
			time += blk->data[pos].size;	// remaining time
		}
		while (x < wid) {
			if (pos < blk->sigCount) {
				amp = hei - blk->data[pos].vol * hei / 256;
				if (oamp < 0)
					oamp = amp;
				while ((time > 0) && (x < wid)) {
					pnt.drawLine(x, oamp, x + 1, amp);
					oamp = amp;
					time -= TDSTEP;
					x++;
				}
				pos++;
				if (pos < blk->sigCount)
					time += blk->data[pos].size;
			} else {
				bnr++;
				if (bnr < tape->blkCount) {
					blk = &tape->blkData[bnr];
					pos = 0;
					time += blk->data[pos].size;
				} else {
					x = wid;
				}
			}
		}
	}
	pnt.end();
	ui.labTapeDiag->setPixmap(pxm);
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
	fillTape();
	// cia
	if (ui.tabsPanel->currentWidget() == ui.ciaTab)	{
		ui.cia1timera->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia1.timerA.value)).arg(gethexword(comp->c64.cia1.timerA.inival)));
		ui.cia1timerb->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia1.timerB.value)).arg(gethexword(comp->c64.cia1.timerB.inival)));
		ui.cia1irq->setText(getbinbyte(comp->c64.cia1.intrq));
		ui.cia1inten->setText(getbinbyte(comp->c64.cia1.inten));
		ui.cia1cra->setText(getbinbyte(comp->c64.cia1.timerA.flags));
		ui.cia1crb->setText(getbinbyte(comp->c64.cia1.timerB.flags));
		ui.cia2timera->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia2.timerA.value)).arg(gethexword(comp->c64.cia2.timerA.inival)));
		ui.cia2timerb->setText(QString("%0 / %1").arg(gethexword(comp->c64.cia2.timerB.value)).arg(gethexword(comp->c64.cia2.timerB.inival)));
		ui.cia2irq->setText(getbinbyte(comp->c64.cia2.intrq));
		ui.cia2inten->setText(getbinbyte(comp->c64.cia2.inten));
		ui.cia2cra->setText(getbinbyte(comp->c64.cia2.timerA.flags));
		ui.cia2crb->setText(getbinbyte(comp->c64.cia2.timerB.flags));
	}

	updateScreen();
	if (ui.tabsPanel->currentWidget() == ui.brkTab)	{
		ui.brkTab->update();
	}

	ui.labRX->setNum(comp->vid->ray.x);
	setSignal(ui.labRX, comp->vid->hblank);

	ui.labRY->setNum(comp->vid->ray.y);
	setSignal(ui.labRY, comp->vid->vblank);

	// memory
	if (ui.tabsPanel->currentWidget() == ui.memTab) {
		ui.widBank->setVisible(comp->hw->grp == HWG_ZX);
		QPixmap img(256, 256);
		QPainter pnt;
		img.fill(Qt::black);
		pnt.begin(&img);
		int pg = 0;
		int x,y;
		QColor col;
		for (y = 0; y < 16; y++) {
			for (x = 0; x < 16; x++) {
				switch(comp->mem->map[pg & 0xff].type) {
					case MEM_RAM: col = Qt::darkGreen; break;
					case MEM_ROM: col = Qt::darkRed; break;
					case MEM_IO: col = Qt::darkBlue; break;
					case MEM_SLOT: col = Qt::darkCyan; break;
					default: col = Qt::darkGray; break;
				}
				pnt.fillRect(x << 4, y << 4, 15, 15, col);
				pg++;
			}
		}
		pnt.setPen(Qt::yellow);
		pnt.drawLine(0, 63, 256, 63);
		pnt.drawLine(0, 127, 256, 127);
		pnt.drawLine(0, 191, 256, 191);
		pnt.end();
		ui.labMemMap->setPixmap(img);
		block = 1;
		if (comp->hw->grp == HWG_ZX) {
			ui.numBank0->setValue(comp->mem->map[0x00].num >> 6);
			ui.numBank1->setValue(comp->mem->map[0x40].num >> 6);
			ui.numBank2->setValue(comp->mem->map[0x80].num >> 6);
			ui.numBank3->setValue(comp->mem->map[0xc0].num >> 6);
			setRFIndex(ui.cbBank0, comp->mem->map[0x00].type);
			setRFIndex(ui.cbBank1, comp->mem->map[0x40].type);
			setRFIndex(ui.cbBank2, comp->mem->map[0x80].type);
			setRFIndex(ui.cbBank3, comp->mem->map[0xc0].type);
		}
		block = 0;
	}

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
	if (ui.tabsPanel->currentWidget() != ui.gbTab) return;
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
	if (ui.tabsPanel->currentWidget() != ui.nesTab) return;
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
			ui.dumpTable->setAdr(val);
			fillDump();
			break;
		case Qt::LeftButton:
			ui.dasmTable->setAdr(val);
			// fillDisasm();
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
	if (ui.tabsPanel->currentWidget() != ui.fdcTab) return;
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
	ui.flpCRC->setText(gethexword(comp->dif->fdc->crc));

	ui.flpCurL->setText(QString('A' + comp->dif->fdc->flp->id));
	ui.flpRdyL->setText(comp->dif->fdc->flp->insert ? "1" : "0");
	ui.flpTrkL->setText(gethexbyte(comp->dif->fdc->flp->trk));
	ui.flpPosL->setText(QString::number(comp->dif->fdc->flp->pos));
	ui.flpIdxL->setText(comp->dif->fdc->flp->index ? "1" : "0");
	ui.flpDataL->setText(comp->dif->fdc->flp->insert ? gethexbyte(flpRd(comp->dif->fdc->flp, comp->dif->fdc->side)): "--"); comp->dif->fdc->flp->rd = 0;
	ui.flpMotL->setText(comp->dif->fdc->flp->motor ? "1" : "0");
}

// z80 regs section

void setCBFlag(QCheckBox* cb, int state) {
	if ((cb->isChecked() && !state) || (!cb->isChecked() && state)) {
		cb->setBackgroundRole(QPalette::Highlight);
	} else {
		cb->setBackgroundRole(QPalette::NoRole);
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
				// dbgRegEdit[i]->clear();
				break;
			default:
				dbgRegLabs[i]->setText(bunch.regs[i].name);
				dbgRegEdit[i]->setProperty("regid", bunch.regs[i].id);
				if (bunch.regs[i].byte) {
					dbgRegEdit[i]->setMax(0xff);
				} else {
					dbgRegEdit[i]->setMax(0xffff);
				}
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
			bunch.regs[i].value = dbgRegEdit[idx]->getValue() & 0xffff;
			i++;
		} else {
			bunch.regs[i].id = REG_NONE;
		}
		idx++;
	}
	cpuSetRegs(cpu, bunch);
	cpu->imode = ui.boxIM->value() & 0xff;
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

void DebugWin::dbgLLab() {
	if (!loadLabels(NULL)) {
		shitHappens("Can't open file");
	}
	fillDisasm();
}
void DebugWin::dbgSLab() {saveLabels(NULL);}

/*
void DebugWin::loadLabels(QString path) {
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(this, "Load SJASM labels");
	if (path.isEmpty())
		return;
	conf.labels.clear();
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
				conf.labels[name] = xadr;
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
		keys = conf.labels.keys();
		foreach(key, keys) {
			xadr = conf.labels[key];
			line = (xadr.type == MEM_RAM) ? gethexbyte(xadr.bank) : "FF";
			line.append(QString(":%0 %1\n").arg(gethexword(xadr.adr & 0x3fff)).arg(key));
			file.write(line.toUtf8());
		}
		file.close();
	} else {
		shitHappens("Can't open file for writing");
	}
}
*/

/*
QString findLabel(int adr, int type, int bank) {
	QString lab;
	if (!conf.dbg.labels)
		return lab;
	QString key;
	xAdr xadr;
	QStringList keys = conf.labels.keys();
	foreach(key, keys) {
		xadr = conf.labels[key];
		if (!((xadr.adr ^ adr) & 0x3fff) \
				&& ((type < 0) || (xadr.type < 0) || (type == xadr.type))\
				&& ((bank < 0) || (xadr.bank < 0) || (bank == xadr.bank))) {
			lab = key;
			break;
		}
	}
	return lab;
}
*/

// map

/*

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
	QStringList keys = conf.labels.keys();
	QString key;
	xAdr xadr;
	foreach(key, keys) {			// labels list
		xadr = conf.labels[key];
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
	conf.labels.clear();
	do {
		strm >> xadr.type;
		strm >> xadr.bank;
		strm >> xadr.adr;
		strm >> key;
		if (!key.isEmpty()) {
			conf.labels[key] = xadr;
		}
	} while (!key.isEmpty());
}

#define XDBGVER 0x00

void DebugWin::saveMap() {
	QString path = QFileDialog::getSaveFileName(this, "Save deBUGa project","","deBUGa project (*.xdbg)");
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
*/

// disasm table

int rdbyte(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	MemPage* pg = &comp->mem->map[(adr >> 8) & 0xff];
	int res = 0xff;
	int fadr = (pg->num << 8) | (adr & 0xff);
	switch (pg->type) {
		case MEM_RAM: res = comp->mem->ramData[fadr & comp->mem->ramMask]; break;
		case MEM_ROM: res = comp->mem->romData[fadr & comp->mem->romMask]; break;
		case MEM_SLOT:
			if (!comp->slot) break;
			if (!comp->slot->data) break;
			res = sltRead(comp->slot, SLT_PRG, adr & 0xffff); break;
			// res = comp->slot->data[fadr & comp->slot->memMask]; break;
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
			bt = rdbyte(adr, comp);
			res = 0;
			while (((fl & 0xc0) == DBG_VIEW_TEXT) && (bt > 31) && (bt < 128) && (res < 250)) {
				res++;
				adr++;
				bt = rdbyte(adr & 0xffff, comp);
				fl = getBrk(comp, adr & 0xffff);
			}
			if (res == 0)
				res++;
			break;
		case DBG_VIEW_CODE:
		case DBG_VIEW_EXEC:
			mn = cpuDisasm(comp->cpu, adr, buf, rdbyte, comp);
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

void DebugWin::saveDasm() {
	QString path = QFileDialog::getSaveFileName(this, "Save disasm",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QFile file(path);
	dasmData drow;
	QList<dasmData> list;
	if (file.open(QFile::WriteOnly)) {
		QTextStream strm(&file);
		unsigned short adr = (blockStart < 0) ? 0 : (blockStart & 0xffff);
		unsigned short end = (blockEnd < 0) ? 0 : (blockEnd & 0xffff);
		int work = 1;
		strm << "; Created by Xpeccy deBUGa\n\n";
		strm << "\tORG 0x" << gethexword(adr) << "\n\n";
		while ((adr <= end) && work) {
			// TODO: check equ $-e
			list = getDisasm(comp, adr);
			foreach (drow, list) {
				if (adr < drow.adr) work = 0;		// address overfill (FFFF+)
				if (drow.isequ) {
					strm << drow.aname << ":";
				} else if (drow.islab) {
					strm << drow.aname << ":\n";
				}
				strm << "\t" << drow.command << "\n";
			}
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
	int adr = ui.dumpTable->getAdr() + (idx.row() << 3);
	if ((col > 0) && (col < 9)) {
		 adr += (idx.column() - 1);
	}
	if (ui.dumpTable->mode != XVIEW_CPU)
		adr &= 0x3fff;
	ui.labHeadDump->setText(QString("Dump : %0").arg(gethexword(adr & 0xffff)));
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
		str.append(gethexbyte(rdbyte(adr+1, comp)));
		str.append(gethexbyte(rdbyte(adr, comp)));
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
	int col;
	QModelIndex idx;
	if (ui.dumpTable->hasFocus()) {
		idx = ui.dumpTable->currentIndex();
		col = idx.column();
		adr = (ui.dumpTable->getAdr() + (idx.row() << 3)) & 0xffff;
		if ((col > 0) && (col < 9)) {
			adr += idx.column() - 1;
		}
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
	unsigned char bt;
	int fadr;
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
		if (data & MEM_BRK_ANY) {
			bt = 0;
			if (ui.actFetch->isChecked()) bt |= MEM_BRK_FETCH;
			if (ui.actRead->isChecked()) bt |= MEM_BRK_RD;
			if (ui.actWrite->isChecked()) bt |= MEM_BRK_WR;
			switch (getRFIData(ui.cbDumpView)) {
				case XVIEW_RAM:
					fadr = (adr & 0x3fff) | (ui.sbDumpPage->value() << 14);
					brkSet(BRK_MEMCELL, bt | MEM_BRK_RAM, fadr, -1);
					break;
				case XVIEW_ROM:
					fadr = (adr & 0x3fff) | (ui.sbDumpPage->value() << 14);
					brkSet(BRK_MEMCELL, bt | MEM_BRK_ROM, fadr, -1);
					break;
				default:
					brkSet(BRK_CPUADR, bt, adr, -1);
					break;
			}
			ui.bpList->update();
		} else {
			ptr = getBrkPtr(comp, adr);
			*ptr &= 0x0f;
			if ((data & 0xf0) == DBG_VIEW_TEXT) {
				bt = rdbyte(adr, comp);
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

//void DebugWin::doSaveDump() {
//	dumpwin->show();
//}

void DebugWin::dmpLimChanged() {
	int start = dui.leStart->getValue();
	int end = dui.leEnd->getValue();
	if (end < start) end = start;
	int len = end - start + 1;
	start = dui.leEnd->cursorPosition();
	dui.leEnd->setValue(end);
	dui.leLen->setValue(len);
	dui.leEnd->setCursorPosition(start);
}

void DebugWin::dmpLenChanged() {
	int start = dui.leStart->getValue();
	int len = dui.leLen->getValue();
	if (start + len > 0x10000) {
		len = 0x10000 - start;
		dui.leLen->setValue(len);
	}
	int end = start + len - 1;
	start = dui.leLen->cursorPosition();
	dui.leEnd->setValue(end);
	dui.leLen->setCursorPosition(start);
}

QByteArray DebugWin::getDumpData() {
	//int bank = dui.leBank->text().toInt(NULL,16);
	int adr = dui.leStart->getValue();
	int len = dui.leLen->getValue();
	QByteArray res;
	while (len > 0) {
		//if (adr < 0xc000) {
		res.append(rdbyte(adr, comp));
		//} else {
		//	res.append(comp->mem->ramData[(bank << 14) | (adr & 0x3fff)]);
		//}
		adr++;
		len--;
	}
	return res;
}

void DebugWin::saveDumpBin() {
	QByteArray data = getDumpData();
	if (data.size() == 0) return;
	QString path = QFileDialog::getSaveFileName(this,"Save memory dump",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) file.write(data);
	dumpwin->hide();
}

void DebugWin::saveDumpHobeta() {
	QByteArray data = getDumpData();
	if (data.size() == 0) return;
	QString path = QFileDialog::getSaveFileName(this,"Save memory dump as hobeta","","Hobeta files (*.$C)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	TRFile dsc;
	QString name = dui.leStart->text();
	// name.append(".").append(dui.leBank->text());
	std::string nms = name.toStdString();
	nms.resize(8,' ');
	memcpy(dsc.name,nms.c_str(),8);
	dsc.ext = 'C';
	int start = dui.leStart->getValue();
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
	int start = dui.leStart->getValue();
	int len = dui.leLen->getValue();
	QString name = dui.leStart->text();
	// name.append(".").append(dui.leBank->text());
	Floppy* flp = comp->dif->fdc->flop[idx & 3];
	if (!flp->insert) {
		diskFormat(flp);
		flp->insert = 1;
	}
	TRFile dsc = diskMakeDescriptor(name.toStdString().c_str(), 'C', start, len);
	if (diskCreateFile(flp, dsc, (unsigned char*)data.data(), data.size()) == ERR_OK)
		dumpwin->hide();

}

// memfinder

void DebugWin::doFind() {
	memFinder->mem = comp->mem;
	memFinder->adr = (ui.dasmTable->getAdr() + 1) & 0xffff;
	memFinder->show();
}

void DebugWin::onFound(int adr) {
	ui.dasmTable->setAdr(adr & 0xffff);
	// fillDisasm();
}

// memfiller

void DebugWin::doFill() {
	memFiller->start(comp->mem, blockStart, blockEnd);
}

// spr scanner

void DebugWin::doMemView() {
	memViewer->mem = comp->mem;
	memViewer->ui.sbPage->setValue(comp->mem->map[0xc0].num >> 6);
	memViewer->fillImage();
	memViewer->show();
}

// open dump

void dbg_mem_wr(Computer* comp, int adr, unsigned char bt) {
	MemPage* pg = &comp->mem->map[(adr >> 8) & 0xff];
	int fadr = (pg->num << 8) | (adr & 0xff);
	switch (pg->type) {
		case MEM_RAM:
			comp->mem->ramData[fadr & comp->mem->ramMask] = bt;
			break;
		case MEM_ROM:
			if (conf.dbg.romwr)
				comp->mem->romData[fadr & comp->mem->romMask] = bt;
			break;
	}
}

int loadDUMP(Computer* comp, const char* name, int adr) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int bt;
	while (adr < 0x10000) {
		bt = fgetc(file);
		if (feof(file)) break;
		dbg_mem_wr(comp, adr & 0xffff, bt & 0xff);
		adr++;
	}
	return ERR_OK;
}

void DebugWin::doOpenDump() {
	dumpPath.clear();
	oui.laPath->clear();
	oui.leStart->setText("C000");
	openDumpDialog->show();
}

void DebugWin::chDumpFile() {
	QString path = QFileDialog::getOpenFileName(this,"Open dump",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
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
	int start = oui.leStart->getValue();
	int len = oui.leLen->getValue();
	int pos = oui.leStart->cursorPosition();
	if (start + len > 0xffff)
		start = 0x10000 - len;
	int end = start + len - 1;
	oui.leStart->setValue(start);
	oui.leEnd->setValue(end);
	oui.leStart->setCursorPosition(pos);
}

void DebugWin::loadDump() {
	if (dumpPath.isEmpty()) return;
	int res = loadDUMP(comp, dumpPath.toLocal8Bit().data(),oui.leStart->text().toInt(NULL,16));
	fillAll();
	if (res == ERR_OK) {
		openDumpDialog->hide();
	} else {
		shitHappens("Can't open file");
	}
}

// screen

void DebugWin::updateScreen() {
	if (ui.tabsPanel->currentWidget() != ui.scrTab) return;
	int flag = ui.cbScrAtr->isChecked() ? 1 : 0;
	flag |= ui.cbScrPix->isChecked() ? 2 : 0;
	flag |= ui.cbScrGrid->isChecked() ? 4 : 0;
	vidGetScreen(comp->vid, scrImg.bits(), ui.sbScrBank->value(), ui.leScrAdr->getValue(), flag);
	xColor bcol;// = comp->vid->pal[comp->vid->nextbrd];
	bcol.b = (comp->vid->nextbrd & 1) ? ((comp->vid->nextbrd & 8) ? 0xff : 0xa0) : 0x00;
	bcol.r = (comp->vid->nextbrd & 2) ? ((comp->vid->nextbrd & 8) ? 0xff : 0xa0) : 0x00;
	bcol.g = (comp->vid->nextbrd & 4) ? ((comp->vid->nextbrd & 8) ? 0xff : 0xa0) : 0x00;
	QPainter pnt;
	QPixmap xpxm(276, 212);
	pnt.begin(&xpxm);
	pnt.fillRect(0,0,276,212,qRgb(bcol.r, bcol.g, bcol.b));
	pnt.drawImage(10,10,scrImg);
	pnt.end();
	ui.scrLabel->setPixmap(xpxm);
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
	std::sort(idxl.begin(), idxl.end(), qGreater<QModelIndex>());
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
	ui.dasmTable->setAdr(adr & 0xffff);
	// fillDisasm();
}

void DebugWin::saveBrk(QString path) {
	if (path.isEmpty())
		path = QFileDialog::getSaveFileName(this, "Save breakpoints", "", "deBUGa breakpoints (*.xbrk)",nullptr,QFileDialog::DontUseNativeDialog);
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
			//if (!brk.off) {
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
					case BRK_IRQ:
						nm = "IRQ";
						ar1.clear();
						ar2.clear();
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
					if (brk.off) flag.append("0");
					file.write(QString("%0:%1:%2:%3\n").arg(nm).arg(ar1).arg(ar2).arg(flag).toUtf8());
				}
			//}
		}
		file.close();
	} else {
		shitHappens("Can't open file for writing");
	}
}

void DebugWin::openBrk() {
	QString path = QFileDialog::getOpenFileName(this, "Open breakpoints list", "", "deBUGa breakpoints (*.xbrk)",nullptr,QFileDialog::DontUseNativeDialog);
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
				brk.off = list.at(3).contains("0") ? 1 : 0;
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
				} else if (list.at(0) == "IRQ") {
					brk.type = BRK_IRQ;
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
