#include "xcore/xcore.h"

#include <stdio.h>

#include <QIcon>
#include <QDebug>
#include <QBuffer>
#include <QPainter>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTextCodec>

#include "debuger.h"
#include "dbg_sprscan.h"
#include "filer.h"
#include "../xgui.h"

unsigned int blockStart = UINT_MAX;
unsigned int blockEnd = UINT_MAX;

int tmpcnt = 0;

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
	NES_SCR_ALL,
	NES_SCR_TILES
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
	//ui.labHeadDump->setStyleSheet(str);
	ui.labHeadDisasm->setStyleSheet(str);
	ui.labHeadMem->setStyleSheet(str);
	ui.labHeadRay->setStyleSheet(str);
	ui.labHeadStack->setStyleSheet(str);
	ui.labHeadSignal->setStyleSheet(str);
	setFont(conf.dbg.font);
	foreach(xHexSpin* xhs, dbgRegEdit) {
		xhs->updatePal();
	}
	ui.dasmTable->update();
	ui.dumpTable->update();
	ui.tabDiskDump->update();
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
	if (isVisible()) {
		activateWindow();
		return;
	}
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

	brk_clear_tmp(comp);		// clear temp breakpoints

	chaPal();		// this will call fillAll
	show();
	if (!fillAll()) {
		ui.dasmTable->setAdr(comp->cpu->pc);
		// fillDisasm();
	}
	updateScreen();

	if (memViewer->vis) {
		memViewer->move(memViewer->winPos);
		memViewer->show();
		memViewer->fillImage();
	}
	chDumpView();
	activateWindow();
}

void DebugWin::stop() {
	if (!ui.cbAccT->isChecked())
		tCount = comp->tickCount;	// before compExec to add current opcode T
	compExec(comp);			// to prevent double breakpoint catch
	comp->debug = 0;		// back to normal work, turn breakpoints on
	comp->vid->debug = 0;
	comp->maping = ui.actMaping->isChecked() ? 1 : 0;
	winPos = pos();
	stopTrace();

	memViewer->vis = memViewer->isVisible() ? 1 : 0;
	memViewer->winPos = memViewer->pos();

	memViewer->hide();
	hide();
	emit closed();
}

void DebugWin::resetTCount() {
	if (ui.cbAccT->isChecked()) {
		tCount = comp->tickCount;
		ui.labTcount->setText(QString("%0 / %1").arg(comp->tickCount - tCount).arg(comp->frmtCount));
	}
}

void DebugWin::onPrfChange(xProfile* prf) {
	if (!prf) prf = conf.prof.cur;
	if (!prf) return;
	comp = prf->zx;
	save_mem_map();
	ui.tabsPanel->clear();
	QList<QPair<QIcon, QWidget*> > lst = tablist[prf->zx->hw->grp];
	QPair<QIcon, QWidget*> p;
	p.first = QIcon(":/images/stop.png");
	p.second = ui.brkTab;
	lst.append(p);
	while(lst.size() > 0) {
		ui.tabsPanel->addTab(lst.first().second, lst.first().first, "");
		lst.removeFirst();
	}
	ui.tabsPanel->setPalette(QPalette());
	tabMode = prf->zx->hw->grp;
	// set input line base
	foreach(xHexSpin* xhs, dbgRegEdit) {
		xhs->setBase(conf.prof.cur->zx->hw->base);
	}
	unsigned int lim = (comp->hw->id == HW_IBM_PC) ? MEM_4M : MEM_64K;
	ui.dumpTable->setLimit(lim);
	ui.dumpScroll->setMaximum(lim - 1);
	ui.tabVidMem->setVMem(conf.prof.cur->zx->vid->ram);
	switch(comp->hw->base) {
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
	bool z80like = (comp->cpu->type == CPU_Z80);
	z80like |= (comp->cpu->type == CPU_LR35902);
	z80like |= (comp->cpu->type == CPU_I8080);
	ui.labIMM->setVisible(z80like); ui.boxIM->setVisible(z80like);
	ui.labIFF1->setVisible(z80like); ui.flagIFF1->setVisible(z80like);
	ui.labIFF2->setVisible(z80like); ui.flagIFF2->setVisible(z80like);
	if (comp->hw->id == HW_IBM_PC) {
		ui.cbDumpView->setCurrentIndex(0);
		ui.cbDumpView->setEnabled(false);
	} else {
		ui.cbDumpView->setEnabled(true);
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

DebugWin::DebugWin(QWidget* par):QDialog(par) {
	int i;

	ui.setupUi(this);
	dumpwin = new QDialog(this);
	labswin = new xLabeList(this);

	tabMode = HW_NULL;

	QList<QPair<QIcon, QWidget*> > lst;
	QPair<QIcon, QWidget*> p;
	tablist.clear();

	p.first = QIcon(":/images/display.png"); p.second = ui.scrTab; lst.append(p);
	p.first = QIcon(":/images/speaker2.png"); p.second = ui.ayTab; lst.append(p);
	p.first = QIcon(":/images/tape.png"); p.second = ui.tapeTab; lst.append(p);
	p.first = QIcon(":/images/floppy.png"); p.second = ui.fdcTab; lst.append(p);
	p.first = QIcon(":/images/memory.png"); p.second = ui.memTab; lst.append(p);
	tablist[HWG_ZX] = lst;
	lst.clear();
	p.first = QIcon(":/images/nespad.png"); p.second = ui.nesTab; lst.append(p);
#if ISDEBUG
	p.first = QIcon(":/images/speaker2.png"); p.second = ui.nesApuTab; lst.append(p);
#endif
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
	lst.clear();
	p.first = QIcon(); p.second = ui.tabPit; lst.append(p);
	tablist[HWG_PC] = lst;
// create registers group
	xLabel* lab;
	xHexSpin* xhs;
	QLabel* qlb;
	QCheckBox* qcb;
	for(i = 0; i < 20; i++) {		// set max registers here
		lab = new xLabel;
		lab->id = i;
		lab->setVisible(false);
		xhs = new xHexSpin;
		xhs->setXFlag(XHS_BGR | XHS_DEC | XHS_FILL);
		xhs->setVisible(false);
		xhs->setFrame(false);
		xhs->setFixedWidth(60);
		xhs->setAlignment(Qt::AlignCenter);
		dbgRegLabs.append(lab);
		dbgRegEdit.append(xhs);
		ui.formRegs->insertRow(i, lab, xhs);
		connect(lab, SIGNAL(clicked(QMouseEvent*)), this, SLOT(regClick(QMouseEvent*)));
		connect(xhs, SIGNAL(textChanged(QString)), this, SLOT(setCPU()));
	}
// create flags group (for cpu->f, 16 bit max)
	flagrp = new QButtonGroup;
	flagrp->setExclusive(false);
	for(i = 0; i < 16; i++) {
		qlb = new QLabel;
		qcb = new QCheckBox;
		qlb->setVisible(false);
		qcb->setVisible(false);
		dbgFlagLabs.append(qlb);
		dbgFlagBox.append(qcb);
		flagrp->addButton(qcb);
		ui.flagsGrid->addWidget(qlb, ((15 - i) & 0x0c) >> 1, (15 - i) & 3);
		ui.flagsGrid->addWidget(qcb, (((15 - i) & 0x0c) >> 1) + 1, (15 - i) & 3);
	}
	connect(flagrp, SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(setFlags()));

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
	ui.tbSaveDasm->addAction(ui.actSaveDump);
	ui.tbSaveDasm->addAction(ui.actLoadLabels);
	ui.tbSaveDasm->addAction(ui.actSaveLabels);
	ui.tbSaveDasm->addAction(ui.actLoadMap);
	ui.tbSaveDasm->addAction(ui.actSaveMap);

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
	ui.tbTool->addAction(ui.actLabelsList);

	ui.tbDbgOpt->addAction(ui.actShowLabels);
	ui.tbDbgOpt->addAction(ui.actHideAddr);
	ui.tbDbgOpt->addAction(ui.actShowSeg);
	ui.tbDbgOpt->addAction(ui.actRomWr);
	ui.tbDbgOpt->addAction(ui.actMaping);
	ui.tbDbgOpt->addAction(ui.actMapingClear);

// connections
	connect(this,SIGNAL(needStep()),this,SLOT(doStep()));
	connect(ui.cbAccT, SIGNAL(toggled(bool)), this, SLOT(resetTCount()));

	connect(ui.actMapingClear,SIGNAL(triggered()),this,SLOT(mapClear()));

	connect(ui.dasmTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDump()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(updateScreen()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),ui.bpList,SLOT(update()));
	connect(ui.dasmTable,SIGNAL(rqRefillAll()),this,SLOT(fillAll()));

	connect(ui.dasmTable,SIGNAL(s_adrch(int)),ui.dasmScroll,SLOT(setValue(int)));
	connect(ui.dasmScroll,SIGNAL(valueChanged(int)),ui.dasmTable,SLOT(setAdr(int)));

	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(fillDump()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),ui.bpList,SLOT(update()));
	connect(ui.dumpTable,SIGNAL(rqRefill()),this,SLOT(updateScreen()));

	connect(ui.dumpTable, SIGNAL(s_adrch(int)), this, SLOT(dumpChadr(int)));
	connect(ui.dumpScroll, SIGNAL(valueChanged(int)), ui.dumpTable, SLOT(setAdr(int)));

	connect(ui.bpList,SIGNAL(rqDisasm(int)),ui.dasmTable,SLOT(setAdr(int)));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDisasm()));
	connect(ui.bpList,SIGNAL(rqDasmDump()),this,SLOT(fillDump()));

	connect(ui.actSearch,SIGNAL(triggered(bool)),this,SLOT(doFind()));
	connect(ui.actFill,SIGNAL(triggered(bool)),this,SLOT(doFill()));
	connect(ui.actSprScan,SIGNAL(triggered(bool)),this,SLOT(doMemView()));
	connect(ui.actShowKeys,SIGNAL(triggered(bool)),this,SIGNAL(wannaKeys()));
	connect(ui.actWutcha,SIGNAL(triggered(bool)),this,SIGNAL(wannaWutch()));

	connect(ui.actLabelsList, SIGNAL(triggered(bool)), labswin, SLOT(show()));
	connect(labswin, SIGNAL(labSelected(QString)), this, SLOT(jumpToLabel(QString)));

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
	connect(ui.actLoadMap, SIGNAL(triggered(bool)),this,SLOT(loadMap()));
	connect(ui.actSaveMap, SIGNAL(triggered(bool)),this,SLOT(saveMap()));
	connect(ui.actDisasm, SIGNAL(triggered(bool)),this,SLOT(saveDasm()));
	connect(ui.tbRefresh, SIGNAL(released()), this, SLOT(reload()));

// dump table

	ui.cbCodePage->addItem("WIN1251", XCP_1251);
	ui.cbCodePage->addItem("CP866", XCP_866);
	ui.cbCodePage->addItem("KOI8R", XCP_KOI8R);

	ui.cbDumpView->addItem("CPU", XVIEW_CPU);
	ui.cbDumpView->addItem("RAM", XVIEW_RAM);
	ui.cbDumpView->addItem("ROM", XVIEW_ROM);

	ui.leDumpPageBase->setMin(0);
	ui.leDumpPageBase->setMax(0xffff);
	ui.cbDumpPageSize->addItem("256", MEM_256);
	ui.cbDumpPageSize->addItem("512", MEM_512);
	ui.cbDumpPageSize->addItem("1KB", MEM_1K);
	ui.cbDumpPageSize->addItem("2KB", MEM_2K);
	ui.cbDumpPageSize->addItem("4KB", MEM_4K);
	ui.cbDumpPageSize->addItem("8KB", MEM_8K);
	ui.cbDumpPageSize->addItem("16KB", MEM_16K);
	ui.cbDumpPageSize->addItem("32KB", MEM_32K);
	ui.cbDumpPageSize->addItem("64KB", MEM_64K);

	ui.dumpTable->setColumnWidth(0,70);
	ui.dumpTable->setItemDelegate(xid_byte);
	ui.dumpTable->setItemDelegateForColumn(0, xid_labl);
	ui.dumpTable->setItemDelegateForColumn(9, xid_none);
	ui.cbDumpPageSize->setCurrentIndex(6);	// 16K

	connect(ui.dumpTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	connect(ui.cbCodePage, SIGNAL(currentIndexChanged(int)), this, SLOT(setDumpCP()));
	connect(ui.cbDumpView, SIGNAL(currentIndexChanged(int)), this, SLOT(chDumpView()));
	connect(ui.sbDumpPage, SIGNAL(valueChanged(int)), this, SLOT(chDumpView()));
	connect(ui.leDumpPageBase, SIGNAL(valueChanged(int)), this, SLOT(chDumpView()));
	connect(ui.cbDumpPageSize, SIGNAL(currentIndexChanged(int)), this, SLOT(chDumpView()));

	ui.cbDumpView->setCurrentIndex(0);

	ui.tabDiskDump->setColumnWidth(0, 70);
	ui.tabDiskDump->horizontalHeader()->setStretchLastSection(true);
	connect(ui.cbDrive, SIGNAL(currentIndexChanged(int)), ui.tabDiskDump, SLOT(setDrive(int)));
	connect(ui.sbTrack, SIGNAL(valueChanged(int)), ui.tabDiskDump, SLOT(setTrack(int)));

// registers
	connect(ui.boxIM,SIGNAL(valueChanged(int)),this,SLOT(setCPU()));
	connect(ui.flagIFF1,SIGNAL(stateChanged(int)),this,SLOT(setCPU()));
	connect(ui.flagIFF2,SIGNAL(stateChanged(int)),this,SLOT(setCPU()));
//	connect(ui.flagGroup,SIGNAL(buttonClicked(int)),this,SLOT(setFlags()));
// infoslots
	scrImg = QImage(256, 192, QImage::Format_RGB888);
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

	connect(ui.tabsPanel, SIGNAL(currentChanged(int)), this, SLOT(fillTabs()));

	block = 0;
	tCount = 0;
	trace = 0;
// nes tab
	ui.nesScrType->addItem("BG off", NES_SCR_OFF);
	ui.nesScrType->addItem("BG scr 0", NES_SCR_0);
	ui.nesScrType->addItem("BG scr 1", NES_SCR_1);
	ui.nesScrType->addItem("BG scr 2", NES_SCR_2);
	ui.nesScrType->addItem("BG scr 3", NES_SCR_3);
	ui.nesScrType->addItem("All in 1", NES_SCR_ALL);
	ui.nesScrType->addItem("Tiles", NES_SCR_TILES);

	ui.nesBGTileset->addItem("Tiles #0000", NES_TILE_0000);
	ui.nesBGTileset->addItem("Tiles #1000", NES_TILE_1000);
	ui.nesSPTileset->addItem("No sprites", -1);
	ui.nesSPTileset->addItem("Sprites #0000", NES_TILE_0000);
	ui.nesSPTileset->addItem("Sprites #1000", NES_TILE_1000);

	connect(ui.nesScrType,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));
	connect(ui.nesBGTileset,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));
	connect(ui.nesSPTileset,SIGNAL(currentIndexChanged(int)), this, SLOT(drawNes()));
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
// pit tab
	ui.tabPit->setModel(new xPitModel());

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
	connect(memFiller, SIGNAL(rqRefill()),this,SLOT(fillNotCPU()));

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
	cellMenu->addAction(ui.actLabelsList);
	cellMenu->addAction(ui.actTraceHere);
	cellMenu->addAction(ui.actShowLabels);
	// NOTE: actions already connected to slots by main menu. no need to double it here

	resize(minimumSize());
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
	fillDump();
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
	int mode,page,pbase,psize;
	mode = getRFIData(ui.cbDumpView);
	page = ui.sbDumpPage->value();
	pbase = ui.leDumpPageBase->getValue();
	if (mode == XVIEW_CPU) {
		psize = (comp->hw->id == HW_IBM_PC) ? MEM_4M : MEM_64K;
	} else {
		psize = getRFIData(ui.cbDumpPageSize);
	}
	ui.widDumpPage->setDisabled(mode == XVIEW_CPU);
	ui.dumpTable->setMode(mode, page, pbase, psize);
	ui.dumpScroll->setMaximum(psize-1);
	ui.dumpTable->setLimit(psize);
}

static QFile logfile;

void DebugWin::doStep() {
		if (!ui.cbAccT->isChecked())
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
			stop();
			*ptr |= MEM_BRK_TFETCH;
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

void DebugWin::fillTabs() {
	fillMem();
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
		QPixmap img(256, 192);
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
				pnt.fillRect(x * 12, y * 12, 11, 11, col);
				pg++;
			}
		}
		pnt.setPen(Qt::yellow);
		pnt.drawLine(0, 47, 256, 47);
		pnt.drawLine(0, 95, 256, 95);
		pnt.drawLine(0, 143, 256, 143);
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
	// pit
	emit ui.tabPit->model()->dataChanged(ui.tabPit->model()->index(0,0), ui.tabPit->model()->index(5,3));
}

bool DebugWin::fillNotCPU() {
	ui.labTcount->setText(QString("%0 / %1").arg(comp->tickCount - tCount).arg(comp->frmtCount));
	fillTabs();
	ui.tabDiskDump->update();
	setSignal(ui.labDOS, comp->dos);
	setSignal(ui.labROM, comp->rom);
	setSignal(ui.labCPM, comp->cpm);
	setSignal(ui.labINT, comp->cpu->intrq & comp->cpu->inten);
	if (memViewer->isVisible())
		memViewer->fillImage();
	fillDump();
	return fillDisasm();
}

bool DebugWin::fillAll() {
	fillCPU();
	return fillNotCPU();
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
	unsigned char col, colidx;
	xColor xcol;
	int adr = 0;
	for (y = 0; y < img.height(); y++) {
		for (x = 0; x < img.width(); x++) {
			colidx = buf[adr];
			if (!(colidx & 3)) colidx = 0;
			col = vid->ram[0x3f00 | (colidx & 0x3f)];
			xcol = nesPal[col & 0x3f];
			if (trn && !(colidx & 3)) {
				img.setPixel(x, y, qRgba(255, 0, 0, 0));
			} else {
				img.setPixel(x, y, qRgba(xcol.r, xcol.g, xcol.b, 0xff));
			}
			adr++;
		}
	}
}

QImage dbgNesScreenImg(Video* vid, unsigned short adr, unsigned short tadr) {
	QImage img(256, 240, QImage::Format_RGB888);
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
		ppuRenderSpriteLine(vid, y + 2, scrmap + (y << 8), NULL, tadr, 8);
	}
	dbgNesConvertColors(vid, scrmap, img, 1);
	return img;
}

QImage dbgNesTilesImg(Video* vid, unsigned short tadr) {
	QImage img(256, 256, QImage::Format_RGB888);
	unsigned char scrmap[256 * 256];
	int x,y,lin,bit;
	int adr = tadr;
	int oadr = 0;
	unsigned char col;
	unsigned short bt;
	img.fill(Qt::black);
	for (y = 0; y < 256; y += 8) {
		for (x = 0; x < 256; x += 8) {
			for (lin = 0; lin < 8; lin++) {
				bt = nes_ppu_ext_rd(adr + lin, conf.prof.cur->zx) & 0xff;
				bt |= (nes_ppu_ext_rd(adr + lin + 8, conf.prof.cur->zx) << 8) & 0xff00;
				for (bit = 0; bit < 8; bit++) {
					col = ((bt >> 7) & 1) | ((bt >> 14) & 2);
					scrmap[oadr + (lin << 8) + bit] = col;
					bt <<= 1;
				}
			}
			oadr += 8;
			adr += 16;		// next sprite
		}
		oadr += 0x700;			// next line (8 lines / sprite)
	}
	dbgNesConvertColors(vid, scrmap, img, 0);
	return img;
}

void DebugWin::drawNes() {
	if (ui.tabsPanel->currentWidget() != ui.nesTab) return;
	unsigned short adr = 0;
	unsigned short tadr = 0;
	QPixmap pic;

	unsigned short vidvadr = comp->vid->vadr;
	unsigned short vidtadr = comp->vid->tadr;

	// screen
	int type = getRFIData(ui.nesScrType);
	int sprt = getRFIData(ui.nesSPTileset);
	switch(type) {
		case NES_SCR_OFF: adr = 0; break;
		case NES_SCR_0:	adr = 0x2000; break;
		case NES_SCR_1: adr = 0x2400; break;
		case NES_SCR_2: adr = 0x2800; break;
		case NES_SCR_3: adr = 0x2c00; break;
	}
	tadr = getRFIData(ui.nesBGTileset) & 0xffff;
//	int px = (comp->vid->sc.x << 3) | (comp->vid->finex & 7);
//	if (comp->vid->nt & 1) px += 256;
//	int py = (comp->vid->sc.y << 3) | (comp->vid->finey & 7);
//	if (comp->vid->nt & 2) py += 240;
	pic = QPixmap(256, 240);
	pic.fill(Qt::black);
	QPainter pnt(&pic);
	switch (type) {
		case NES_SCR_ALL:
			pnt.drawImage(0, 0, dbgNesScreenImg(comp->vid, 0x2000, tadr).scaled(128, 120));
			pnt.drawImage(128, 0, dbgNesScreenImg(comp->vid, 0x2400, tadr).scaled(128, 120));
			pnt.drawImage(0, 120, dbgNesScreenImg(comp->vid, 0x2800, tadr).scaled(128, 120));
			pnt.drawImage(128, 120, dbgNesScreenImg(comp->vid, 0x2c00, tadr).scaled(128, 120));
//			pnt.setPen(qRgba(0, 0, 0, 128));
//			pnt.setBrush(Qt::NoBrush);
//			pnt.drawRect(px / 2, py / 2, 128, 120);
//			pnt.drawRect(px / 2 - 256, py / 2, 128, 120);
//			pnt.drawRect(px / 2, py / 2 - 128, 128, 120);
//			pnt.drawRect(px / 2 - 256, py / 2 - 128, 128, 120);
			break;
		case NES_SCR_TILES:
			pnt.drawImage(0, 0, dbgNesTilesImg(comp->vid, tadr));
			break;
		case NES_SCR_OFF:
			if (sprt > -1)
				pnt.drawImage(0, 0, dbgNesSpriteImg(comp->vid, sprt & 0xffff));
			break;
		case NES_SCR_0:
		case NES_SCR_1:
		case NES_SCR_2:
		case NES_SCR_3:
			pnt.drawImage(0, 0, dbgNesScreenImg(comp->vid, (comp->vid->tadr & ~0x2c00) | adr, tadr));
			if (sprt > -1)
				pnt.drawImage(0, 0, dbgNesSpriteImg(comp->vid, sprt & 0xffff));
			break;
	}
	pnt.end();
	ui.nesScreen->setPixmap(pic);

	comp->vid->vadr = vidvadr;
	comp->vid->tadr = vidtadr;

	// registers
	ui.lab_ppu_r0->setText(gethexbyte(comp->vid->reg[0]));
	ui.lab_ppu_r1->setText(gethexbyte(comp->vid->reg[1]));
	ui.lab_ppu_r2->setText(gethexbyte(comp->vid->reg[2]));
	ui.lab_ppu_r3->setText(gethexbyte(comp->vid->reg[3]));
	ui.lab_ppu_r4->setText(gethexbyte(comp->vid->reg[4]));
	ui.lab_ppu_r5->setText(gethexbyte(comp->vid->reg[5]));
	ui.lab_ppu_r6->setText(gethexbyte(comp->vid->reg[6]));
	ui.lab_ppu_r7->setText(gethexbyte(comp->vid->reg[7]));
	// vadr
	ui.labVAdr->setText(gethexword(comp->vid->vadr & 0x7fff));
	ui.labTAdr->setText(gethexword(comp->vid->tadr & 0x7fff));
}

// ...

/*
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
*/

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
	xLabel* lab = qobject_cast<xLabel*>(sender());
	int id = lab->id;
	if (id < 0) return;
	xRegBunch bunch = cpuGetRegs(comp->cpu);
	xRegister reg = bunch.regs[id];
	switch (ev->button()) {
		case Qt::RightButton:
			if (reg.type & REG_SEG) {
				ui.dumpTable->setAdr(reg.base);
			} else if (comp->cpu->type == CPU_I80286) {
				switch(reg.id) {
					case I286_IP: id = comp->cpu->cs.base; break;
					case I286_SP: id = comp->cpu->ss.base; break;
					case I286_BP: id = comp->cpu->ss.base; break;
					default: id = comp->cpu->ds.base; break;
				}
				ui.dumpTable->setAdr(id + reg.value);
			} else {
				ui.dumpTable->setAdr(reg.value);
			}
			break;
		case Qt::LeftButton:
			ui.dasmTable->setAdr(reg.value, 1);
			break;
		default:
			break;
	}
}

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
	ui.flpPosL->setText(gethexword(comp->dif->fdc->flp->pos));
	ui.flpIdxL->setText(comp->dif->fdc->flp->index ? "1" : "0");
	ui.flpDataL->setText(comp->dif->fdc->flp->insert ? gethexbyte(flpRd(comp->dif->fdc->flp, comp->dif->fdc->side)): "--"); comp->dif->fdc->flp->rd = 0;
	ui.flpMotL->setText(comp->dif->fdc->flp->motor ? "1" : "0");
}

// CPU

void DebugWin::fillFlags(const char* fnam) {
	if (fnam == NULL)
		fnam = cpuGetRegs(comp->cpu).flags;
	int flgcnt = strlen(fnam);
	QString allflags = QString(fnam).rightJustified(16, '-');
	for (int i = 0; i < 16; i++) {
		if (i < flgcnt) {
			dbgFlagBox[i]->setVisible(true);
			dbgFlagLabs[i]->setVisible(true);
			dbgFlagLabs[i]->setText(allflags.at(15 - i));
			dbgFlagBox[i]->setChecked(comp->cpu->f & (1 << i));
		} else {
			dbgFlagBox[i]->setVisible(false);
			dbgFlagLabs[i]->setVisible(false);
		}
	}
}

void DebugWin::fillCPU() {
	block = 1;
	CPU* cpu = comp->cpu;
	xRegBunch bunch = cpuGetRegs(cpu);
	int i;
	for (i = 0; i < dbgRegLabs.size(); i++) {
		switch (bunch.regs[i].id) {
			case REG_EMPTY:
			case REG_NONE:
				dbgRegLabs[i]->setVisible(false);
				dbgRegEdit[i]->setVisible(false);
				break;
			default:
				dbgRegLabs[i]->setText(bunch.regs[i].name);
				dbgRegEdit[i]->setProperty("regid", bunch.regs[i].id);
				switch (bunch.regs[i].type & REG_TMASK) {
					case REG_BIT: dbgRegEdit[i]->setMax(1); break;
					case REG_BYTE: dbgRegEdit[i]->setMax(0xff); break;
					case REG_24: dbgRegEdit[i]->setMax(0xffffff); break;
					default: dbgRegEdit[i]->setMax(0xffff); break;
				}
				dbgRegEdit[i]->setReadOnly(bunch.regs[i].type & REG_RO);
				dbgRegEdit[i]->setValue(bunch.regs[i].value);
				dbgRegEdit[i]->setVisible(true);
				dbgRegLabs[i]->setVisible(true);
				break;
		}
	}
	fillFlags(bunch.flags);
	ui.boxIM->setValue(cpu->imode);
	ui.flagIFF1->setChecked(cpu->iff1);
	ui.flagIFF2->setChecked(cpu->iff2);
	fillStack();
	block = 0;
}

// called only from connection. checkboxes to cpu->f
void DebugWin::setFlags() {
	if (block) return;
	unsigned short f = 0;
	for (int i = 0; i < 16; i++) {
		if (dbgFlagBox[i]->isVisible() && dbgFlagBox[i]->isChecked())
			f |= (1 << i);
	}
	comp->cpu->f = f;
	fillCPU();
}

void DebugWin::setCPU() {
	if (block) return;
	CPU* cpu = comp->cpu;
	int i = 0;
	xRegBunch bunch;
	foreach(xHexSpin* xhs, dbgRegEdit) {
		if (xhs->isEnabled()) {
			bunch.regs[i].id = xhs->property("regid").toInt();
			bunch.regs[i].value = xhs->getValue() & 0xffff;
			i++;
		} else {
			bunch.regs[i].id = REG_NONE;
		}
	}
	cpuSetRegs(cpu, bunch);
	cpu->imode = ui.boxIM->value() & 0xff;
	cpu->iff1 = ui.flagIFF1->isChecked() ? 1 : 0;
	cpu->iff2 = ui.flagIFF2->isChecked() ? 1 : 0;
	fillFlags(NULL);
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

void DebugWin::loadMap() {
	QString path = QFileDialog::getOpenFileName(this, "Open the universe", "", "Xpeccy memory map (*.xmap)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	load_xmap(path);
	brkInstallAll();
	fillAll();
}

void DebugWin::saveMap() {
	QString path = QFileDialog::getSaveFileName(this, "Save the universe", "", "Xpeccy memory map (*.xmap)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	if (!path.endsWith(".xmap",Qt::CaseInsensitive))
		path.append(".xmap");
	save_xmap(path);

}

// labels

void DebugWin::dbgLLab() {
	if (!loadLabels(NULL)) {
		shitHappens("Can't open file");
	}
	fillDisasm();
}
void DebugWin::dbgSLab() {saveLabels(NULL);}

void DebugWin::jumpToLabel(QString lab) {
	if (conf.prof.cur->labels.contains(lab))
		ui.dasmTable->setAdr(conf.prof.cur->labels[lab].adr, 1);
}

// disasm table

int rdbyte(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = -1;
	if (comp->hw->id == HW_IBM_PC) {
		res = comp->hw->mrd(comp, adr, 0);
	} else {
		MemPage* pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[(adr >> 8) & 0xff];
		int fadr = mem_get_phys_adr(comp->mem, adr);	// = pg->num << 8) | (adr & 0xff);
		switch (pg->type) {
			case MEM_RAM: res = comp->mem->ramData[fadr & comp->mem->ramMask]; break;
			case MEM_ROM: res = comp->mem->romData[fadr & comp->mem->romMask]; break;
			case MEM_SLOT:
				if (!comp->slot) break;
				if (!comp->slot->data) break;
				res = sltRead(comp->slot, SLT_PRG, adr & 0xffff); break;
		}
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
					if (drow.iscom) {
						strm << drow.aname;
					} else {
						strm << drow.aname << ":";
					}
				} else {
					strm << "\t" << drow.command;
				}
				strm << "\n";
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
	dumpChadr(ui.dumpTable->getAdr());
	block = 0;
}

void DebugWin::dumpChadr(int adr) {
	ui.dumpScroll->setValue(adr);
	QModelIndex idx = ui.dumpTable->selectionModel()->currentIndex();
	int col = idx.column();
	adr += idx.row() << 3;
	if ((col > 0) && (col < 9)) {
		 adr += (col - 1);
	}
	if (ui.dumpTable->mode != XVIEW_CPU) {
		adr &= 0x3fff;
	} else {
		adr %= ui.dumpTable->limit();
	}
	ui.tabsDump->setTabText(0, QString::number(adr, 16).right(6).toUpper().rightJustified(6,'0'));
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
	if (comp->cpu->type == CPU_I80286) {
		adr += comp->cpu->ss.base;
	}
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

unsigned int DebugWin::getAdr() {
	int adr;
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

// TODO: breakpoints on block
void DebugWin::chaCellProperty(QAction* act) {
	int data = act->data().toInt();
	unsigned int adr = getAdr();
	unsigned int bgn, end;
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
	int proc = 1;
	while ((adr <= end) && proc) {
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
					brkSet(BRK_CPUADR, bt, bgn, end);
					proc = 0;			// stop processing
					break;
			}
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
	ui.bpList->update();
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
	MemPage* pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[(adr >> 8) & 0xff];
	int fadr = mem_get_phys_adr(comp->mem, adr);	// = pg->num << 8) | (adr & 0xff);
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
		oui.leLen->setValue(inf.size() & 0xffff);
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
	ui.labCurScr->setText(QString::number(comp->vid->curscr, 16).rightJustified(2, '0'));
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

bool qmidx_greater(const QModelIndex idx1, const QModelIndex idx2) {
	return (idx1.row() > idx2.row());
}

void DebugWin::delBrk() {
	QModelIndexList idxl = ui.bpList->selectionModel()->selectedRows();
	std::sort(idxl.begin(), idxl.end(), qmidx_greater); // qGreater<QModelIndex>());
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
						if (brk.eadr > brk.adr) {
							ar1.append("-");
							ar1.append(gethexword(brk.eadr & 0xffff));
						}
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
	QStringList splt;
	QStringList list;
	xBrkPoint brk;
	bool b0,b1;
	if (file.open(QFile::ReadOnly)) {
		conf.prof.cur->brkList.clear();
		while(!file.atEnd()) {
			line = tr(file.readLine());
			if (!line.startsWith(";")) {
				b0 = true;
				b1 = true;
				list = line.split(":", X_KeepEmptyParts);
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
					if (list.at(1).contains("-")) {		// 1234-ABCD
						splt = list.at(1).split(QLatin1Char('-'), X_SkipEmptyParts);
						list[1] = splt.first();
						list[2] = splt.last();
					}
					brk.adr = list.at(1).toInt(&b0, 16) & 0xffff;
					if (list.at(2).isEmpty()) {
						brk.eadr = brk.adr;
					} else {
						brk.eadr = list.at(2).toInt(&b1, 16) & 0xffff;
						if (brk.eadr < brk.adr)
							brk.eadr = brk.adr;
					}
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
