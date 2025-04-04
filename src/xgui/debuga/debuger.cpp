#include "xcore/xcore.h"

#include <stdio.h>

#include <QIcon>
#include <QDebug>
#include <QBuffer>
#include <QPainter>
#include <QToolBar>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTextCodec>

#include "debuger.h"
#include "dbg_sprscan.h"
#include "filer.h"
#include "../xgui.h"

int blockStart = -1;
int blockEnd = -1;

int tmpcnt = 0;

int getRFIData(QComboBox*);
void setRFIndex(QComboBox* box, QVariant data);
int dasmSome(Computer*, int, dasmData&);

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

void DebugWin::updateStyle() {
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
	ui_cpu.labHeadCpu->setStyleSheet(str);
	ui_asm.labHeadDisasm->setStyleSheet(str);
	ui_misc.labHeadMem->setStyleSheet(str);
	ui_misc.labHeadRay->setStyleSheet(str);
	ui_misc.labHeadStack->setStyleSheet(str);
	ui_misc.labHeadSignal->setStyleSheet(str);
	ui_misc.labPorts->setStyleSheet(str);
	xDockWidget* dw;
	void* ptr;
	foreach(ptr, dockWidgets) {
		dw = static_cast<xDockWidget*>(ptr);
		dw->titleBarWidget()->setStyleSheet(str);
	}
	setFont(conf.dbg.font);
	foreach(xHexSpin* xhs, dbgRegEdit) {
		xhs->updatePal();
	}
	ui_asm.dasmTable->update();
	wid_dump->draw();
	//ui.dumpTable->update();
	wid_disk_dump->draw();
}

void DebugWin::save_mem_map() {
	Computer* comp = conf.prof.cur->zx;
	for (int i = 0; i < 256; i++) {
		wid_mmap->mem_map[i] = comp->mem->map[i];
	}
}

void DebugWin::rest_mem_map() {
	Computer* comp = conf.prof.cur->zx;
	for (int i = 0; i < 256; i++) {
		 comp->mem->map[i] = wid_mmap->mem_map[i];
	}
	fillAll();
}

void DebugWin::d_remap(int _b, int _t, int _n) {
	Computer* comp = conf.prof.cur->zx;
	memSetBank(comp->mem, _b, _t, _n, MEM_16K, NULL, NULL, NULL);
	wid_dump->draw();
	ui_asm.dasmTable->updContent();
	wid_mmap->draw();
}

void DebugWin::start() {
	if (isVisible()) {
		activateWindow();
		return;
	}
	blockStart = -1;
	blockEnd = -1;
	save_mem_map();
	Computer* comp = conf.prof.cur->zx;
	if (comp->hw->grp != tabMode) {
		onPrfChange();		// update tabs
	}
	chLayout();
	if (!comp->vid->tail)
		vid_dark_tail(comp->vid);

	this->move(conf.dbg.pos);
	comp->vid->debug = 1;
	comp->debug = 1;
	comp->brk = 0;

	brk_clear_tmp(comp);		// clear temp breakpoints

//	ui.tabDiskDump->setDrive(ui.cbDrive->currentIndex());
	updateStyle();		// this will call fillAll
	show();
// fillall redrawing all vivisble widgets
	if (!fillAll()) {
		ui_asm.dasmTable->setAdr(comp->cpu->pc + comp->cpu->cs.base);
		// fillDisasm();
	}
//	wid_zxscr->draw();
//	updateScreen();

	if (memViewer->vis) {
		memViewer->move(memViewer->winPos);
		memViewer->show();
		memViewer->fillImage();
	}
//	wid_dump->draw();
	wid_brk->moved();		// to redraw all icons
//	chDumpView();
	activateWindow();
}

void DebugWin::stop() {
	Computer* comp = conf.prof.cur->zx;
	if (!ui_asm.cbAccT->isChecked())
		tCount = comp->tickCount;	// before compExec to add current opcode T
	if (comp->debug) compExec(comp);			// to prevent double breakpoint catch
	comp->debug = 0;		// back to normal work, turn breakpoints on
	comp->vid->debug = 0;
	comp->maping = ui_asm.actMaping->isChecked() ? 1 : 0;
	stopTrace();

	memViewer->vis = memViewer->isVisible() ? 1 : 0;
	memViewer->winPos = memViewer->pos();

	void* ptr;
	xDockWidget* dw;
	foreach(ptr, dockWidgets) {
		dw = (xDockWidget*)ptr;
		dw->setFloating(false);
	}

	memViewer->hide();
	hide();
	emit closed();
}

void DebugWin::resetTCount() {
	Computer* comp = conf.prof.cur->zx;
	if (ui_asm.cbAccT->isChecked()) {
		tCount = comp->tickCount;
		ui_asm.labTcount->setText(QString("%0 / %1").arg(comp->tickCount - tCount).arg(comp->frmtCount));
	}
}

void DebugWin::onPrfChange() {
	xProfile* prf = conf.prof.cur;
	if (!prf) return;
	Computer* comp = prf->zx;
	save_mem_map();

	tabMode = comp->hw->grp;
	xDockWidget* ptr;
	foreach (void* dw, dockWidgets) {
		ptr = (xDockWidget*)dw;
		ptr->setHidden(!(ptr->hwList.isEmpty() || ptr->hwList.contains(tabMode)));
	}

	// set input line base
	foreach(xHexSpin* xhs, dbgRegEdit) {
		xhs->setBase(comp->hw->base);
	}
	unsigned int lim = (comp->hw->id == HW_IBM_PC) ? MEM_4M : MEM_64K;
	wid_dump->setLimit(lim);
	ui_asm.dasmScroll->setMaximum(lim - 1);

	dui.leStart->setMax(lim - 1);
	//dui.leEnd->setMax(lim - 1);		// TODO:why segfault
	dui.leLen->setMax(lim);

	// ui.tabDiskDump->setDrive(ui.cbDrive->currentIndex());
	wid_disk_dump->draw();
	wid_vmem_dump->setVMem(conf.prof.cur->zx->vid->ram);

	wid_dump->setBase(comp->hw->base, comp->hw->id);
	// TODO: move imm/iff1/iff2 in registers
	bool z80like = (comp->cpu->type == CPU_Z80);
	z80like |= (comp->cpu->type == CPU_LR35902);
	z80like |= (comp->cpu->type == CPU_I8080);
	ui_cpu.labIMM->setVisible(z80like); ui_cpu.boxIM->setVisible(z80like);
	ui_cpu.labIFF1->setVisible(z80like); ui_cpu.flagIFF1->setVisible(z80like);
	ui_cpu.labIFF2->setVisible(z80like); ui_cpu.flagIFF2->setVisible(z80like);

	wid_brk->moved();

	fillAll();
}

// void DebugWin::reject() {stop();}
void DebugWin::closeEvent(QCloseEvent*) {stop();}

DebugWin::DebugWin(QWidget* par):QMainWindow(par) {
	int i;

	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
	setWindowTitle("Xpeccy deBUGa");
	setWindowIcon(QIcon(":/images/bug.png"));

	cw = new QWidget;
	QWidget* wid_cpu = new QWidget;
	QWidget* wid_dasm = new QWidget;
	ui_cpu.setupUi(wid_cpu);
	ui_asm.setupUi(wid_dasm);
	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(wid_cpu);
	lay->addWidget(wid_dasm);
	lay->setStretchFactor(wid_dasm, 10);
	cw->setLayout(lay);
	setCentralWidget(cw);

	wid_dump = new xDumpWidget("","DUMP");
	wid_rdump = new xRDumpWidget("","REG-DUMP");
	wid_disk_dump = new xDiskDumpWidget(":/images/floppy.png","FDD");
	wid_cmos_dump = new xCmosDumpWidget("","CMOS");
	wid_vmem_dump = new xVMemDumpWidget("","VMEM");
	wid_zxscr = new xZXScrWidget(":/images/rulers.png","Screen");
	wid_dma = new xDmaWidget("","DMA");
	wid_pit = new xPitWidget("","PIT");
	wid_pic = new xPicWidget("","PIC");
	wid_vga = new xVgaWidget(":/images/display.png","VGA");
	wid_ay = new xAYWidget(":/images/note.png","AY");
	wid_tape = new xTapeWidget(":/images/tape.png","Tape");
	wid_fdd = new xFDDWidget(":/images/floppy.png","FDC");
	wid_brk = new xBreakWidget(":/images/stop.png","Breakpoints");
	wid_gb = new xGameboyWidget(":/images/gameboy.png","GameBoy");
	wid_ppu = new xPPUWidget(":/images/nespad.png","NES PPU");
	wid_cia = new xCiaWidget("","CIA");
	wid_vic = new xVicWidget("","VIC");
	wid_mmap = new xMMapWidget(":/images/memory.png","Memory map");
	wid_ps2 = new xPS2Widget("","PS/2");

	dockWidgets << wid_dump << wid_rdump << wid_disk_dump << wid_vmem_dump << wid_cmos_dump;
	dockWidgets << wid_brk << wid_zxscr << wid_ay << wid_tape;
	dockWidgets << wid_fdd << wid_mmap << wid_gb << wid_ppu;
	dockWidgets << wid_cia << wid_dma << wid_pic << wid_pit << wid_vga << wid_ps2;

	addDockWidget(Qt::RightDockWidgetArea, wid_dump);
	tabifyDockWidget(wid_dump, wid_rdump);
	tabifyDockWidget(wid_dump, wid_disk_dump);
	tabifyDockWidget(wid_dump, wid_vmem_dump);
	tabifyDockWidget(wid_dump, wid_cmos_dump);
	addDockWidget(Qt::RightDockWidgetArea, wid_brk);
	tabifyDockWidget(wid_brk, wid_zxscr);
	tabifyDockWidget(wid_brk, wid_ay);
	tabifyDockWidget(wid_brk, wid_tape);
	tabifyDockWidget(wid_brk, wid_fdd);
	tabifyDockWidget(wid_brk, wid_mmap);
	tabifyDockWidget(wid_brk, wid_gb);
	tabifyDockWidget(wid_brk, wid_ppu);
	tabifyDockWidget(wid_brk, wid_cia);
	tabifyDockWidget(wid_brk, wid_dma);
	tabifyDockWidget(wid_brk, wid_pic);
	tabifyDockWidget(wid_brk, wid_pit);
	tabifyDockWidget(wid_brk, wid_vga);
	tabifyDockWidget(wid_brk, wid_ps2);
	wid_dump->raise();
	wid_brk->raise();

	QToolBar* rtbar = new QToolBar;
	QWidget* rtbw = new QWidget;
	rtbar->setObjectName("MISCTOOLBAR");
	ui_misc.setupUi(rtbw);
	rtbar->addWidget(rtbw);
	rtbar->setFloatable(false);
	rtbar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
	addToolBar(Qt::RightToolBarArea, rtbar);
	rtbar->setContextMenuPolicy(Qt::PreventContextMenu);

	dumpwin = new QDialog(this);
	labswin = new xLabeList(this);
// create registers group
	xLabel* lab;
	xHexSpin* xhs;
	QLabel* qlb;
	QCheckBox* qcb;
	for(i = 0; i < 32; i++) {		// set max registers here
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
		ui_cpu.formRegs->insertRow(i, lab, xhs);
		connect(lab, &xLabel::clicked, this, &DebugWin::regClick);
		connect(xhs, &xHexSpin::textChanged, this, &DebugWin::setCPU);
	}
// create ports group
	ui_misc.formPort->setVerticalSpacing(0);
	for (i = 0; i < 32; i++) {
		ui_misc.formPort->insertRow(i, new QLabel, new QLabel);
		ui_misc.formPort->itemAt(i, QFormLayout::LabelRole)->widget()->setVisible(false);
		ui_misc.formPort->itemAt(i, QFormLayout::FieldRole)->widget()->setVisible(false);
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
		ui_cpu.flagsGrid->addWidget(qlb, ((15 - i) & 0x0c) >> 1, (15 - i) & 3);
		ui_cpu.flagsGrid->addWidget(qcb, (((15 - i) & 0x0c) >> 1) + 1, (15 - i) & 3);
	}
	connect(flagrp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(setFlags()));

	conf.dbg.labels = 1;
	conf.dbg.segment = 0;
	ui_asm.actShowLabels->setChecked(conf.dbg.labels);
	ui_asm.actShowSeg->setChecked(conf.dbg.segment);

	xid_none = new xItemDelegate(XTYPE_NONE);
	xid_byte = new xItemDelegate(XTYPE_BYTE);
	xid_labl = new xItemDelegate(XTYPE_LABEL);
	xid_octw = new xItemDelegate(XTYPE_OCTWRD);
	xid_dump = new xItemDelegate(XTYPE_DUMP);

// actions data
	ui_asm.actFetch->setData(MEM_BRK_FETCH);
	ui_asm.actRead->setData(MEM_BRK_RD);
	ui_asm.actWrite->setData(MEM_BRK_WR);

	ui_asm.actViewOpcode->setData(DBG_VIEW_EXEC);
	ui_asm.actViewByte->setData(DBG_VIEW_BYTE);
	ui_asm.actViewWord->setData(DBG_VIEW_WORD);
	ui_asm.actViewAddr->setData(DBG_VIEW_ADDR);
	ui_asm.actViewText->setData(DBG_VIEW_TEXT);

	ui_asm.actTrace->setData(DBG_TRACE_ALL);
	ui_asm.actTraceHere->setData(DBG_TRACE_HERE);
	ui_asm.actTraceINT->setData(DBG_TRACE_INT);
	ui_asm.actTraceLog->setData(DBG_TRACE_LOG);

	ui_asm.dasmTable->setFocus();

// disasm table
	ui_asm.dasmTable->setItemDelegateForColumn(0, xid_labl);
	ui_asm.dasmTable->setItemDelegateForColumn(1, xid_dump);

// actions
	ui_asm.tbBreak->addAction(ui_asm.actFetch);
	ui_asm.tbBreak->addAction(ui_asm.actRead);
	ui_asm.tbBreak->addAction(ui_asm.actWrite);

	ui_asm.tbView->addAction(ui_asm.actViewOpcode);
	ui_asm.tbView->addAction(ui_asm.actViewByte);
	ui_asm.tbView->addAction(ui_asm.actViewText);
	ui_asm.tbView->addAction(ui_asm.actViewWord);
	ui_asm.tbView->addAction(ui_asm.actViewAddr);

	ui_asm.tbSaveDasm->addAction(ui_asm.actDisasm);
	ui_asm.tbSaveDasm->addAction(ui_asm.actLoadDump);
	ui_asm.tbSaveDasm->addAction(ui_asm.actSaveDump);
	ui_asm.tbSaveDasm->addAction(ui_asm.actLoadLabels);
	ui_asm.tbSaveDasm->addAction(ui_asm.actSaveLabels);
	ui_asm.tbSaveDasm->addAction(ui_asm.actLoadMap);
	ui_asm.tbSaveDasm->addAction(ui_asm.actSaveMap);

	ui_asm.tbTrace->addAction(ui_asm.actTrace);
	ui_asm.tbTrace->addAction(ui_asm.actTraceHere);
	ui_asm.tbTrace->addAction(ui_asm.actTraceINT);
	ui_asm.tbTrace->addAction(ui_asm.actTraceLog);

	ui_asm.tbTool->addAction(ui_asm.actSearch);
	ui_asm.tbTool->addAction(ui_asm.actFill);
	ui_asm.tbTool->addAction(ui_asm.actSprScan);
	ui_asm.tbTool->addAction(ui_asm.actShowKeys);
	ui_asm.tbTool->addAction(ui_asm.actWutcha);
	ui_asm.tbTool->addAction(ui_asm.actLabelsList);

	ui_asm.tbDbgOpt->addAction(ui_asm.actShowLabels);
	ui_asm.tbDbgOpt->addAction(ui_asm.actHideAddr);
	ui_asm.tbDbgOpt->addAction(ui_asm.actShowSeg);
	ui_asm.tbDbgOpt->addAction(ui_asm.actRomWr);
	ui_asm.tbDbgOpt->addAction(ui_asm.actMaping);
	ui_asm.tbDbgOpt->addAction(ui_asm.actMapingClear);

// connections
	connect(this, &DebugWin::needStep, this, &DebugWin::doStep);
	connect(ui_asm.cbAccT, &QCheckBox::toggled, this, &DebugWin::resetTCount);
	connect(ui_asm.actMapingClear, &QAction::triggered, this, &DebugWin::mapClear);
	connect(ui_asm.dasmTable, &xDisasmTable::customContextMenuRequested, this, &DebugWin::putBreakPoint);
	connect(ui_asm.dasmTable, &xDisasmTable::rqRefill, this, &DebugWin::fillDisasm);
	connect(ui_asm.dasmTable, &xDisasmTable::rqRefill, wid_dump, &xDumpWidget::draw);
	connect(ui_asm.dasmTable, &xDisasmTable::rqRefill, wid_zxscr, &xZXScrWidget::draw);
	connect(ui_asm.dasmTable, &xDisasmTable::rqRefill, wid_brk, &xBreakWidget::draw);
	connect(ui_asm.dasmTable, &xDisasmTable::rqRefillAll, this, &DebugWin::fillAll);
	connect(ui_asm.dasmTable, &xDisasmTable::s_adrch, ui_asm.dasmScroll, &QScrollBar::setValue);
	connect(ui_asm.dasmScroll, &QScrollBar::valueChanged, ui_asm.dasmTable, &xDisasmTable::setAdrX);

	connect(wid_dump, &xDumpWidget::s_blockch, this, &DebugWin::fillDisasm);
	connect(wid_dump, &xDumpWidget::s_datach, this, &DebugWin::fillAll);
	connect(wid_dump, &xDumpWidget::s_brkrq, this, &DebugWin::brkRequest);

	connect(wid_brk, &xBreakWidget::rqDisasm, ui_asm.dasmTable, &xDisasmTable::setAdrX);
	connect(wid_brk, &xBreakWidget::updated, this, &DebugWin::fillDisasm);
	connect(wid_brk, &xBreakWidget::updated, wid_dump, &xDumpWidget::draw);

	connect(ui_asm.actSearch, &QAction::triggered, this, &DebugWin::doFind);
	connect(ui_asm.actFill, &QAction::triggered, this, &DebugWin::doFill);
	connect(ui_asm.actSprScan, &QAction::triggered, this, &DebugWin::doMemView);
	connect(ui_asm.actShowKeys, &QAction::triggered, this, &DebugWin::wannaKeys);
	connect(ui_asm.actWutcha, &QAction::triggered, this, &DebugWin::wannaWutch);

	connect(ui_asm.actLabelsList, &QAction::triggered, labswin, &xLabeList::show);
	connect(labswin, &xLabeList::labSelected, this, &DebugWin::jumpToLabel);

	connect(ui_asm.actShowLabels, &QAction::toggled, this, &DebugWin::setShowLabels);
	connect(ui_asm.actHideAddr, &QAction::toggled, this, &DebugWin::fillDisasm);
	connect(ui_asm.actShowSeg, &QAction::toggled, this, &DebugWin::setShowSegment);
	connect(ui_asm.actRomWr, &QAction::toggled, this, &DebugWin::setRomWriteable);

	connect(ui_asm.tbView, &QToolButton::triggered, this, &DebugWin::chaCellProperty);
	connect(ui_asm.tbBreak, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui_asm.tbTrace, SIGNAL(triggered(QAction*)),this,SLOT(doTrace(QAction*)));

	connect(ui_asm.actLoadDump, SIGNAL(triggered(bool)),this,SLOT(doOpenDump()));
	connect(ui_asm.actSaveDump, SIGNAL(triggered(bool)),dumpwin,SLOT(show()));
	connect(ui_asm.actLoadLabels, SIGNAL(triggered(bool)),this,SLOT(dbgLLab()));
	connect(ui_asm.actSaveLabels, SIGNAL(triggered(bool)),this,SLOT(dbgSLab()));
	connect(ui_asm.actLoadMap, SIGNAL(triggered(bool)),this,SLOT(loadMap()));
	connect(ui_asm.actSaveMap, SIGNAL(triggered(bool)),this,SLOT(saveMap()));
	connect(ui_asm.actDisasm, SIGNAL(triggered(bool)),this,SLOT(saveDasm()));
	connect(ui_asm.tbRefresh, SIGNAL(released()), this, SLOT(reload()));

	connect(wid_mmap, SIGNAL(s_remap(int, int, int)), this, SLOT(d_remap(int,int,int)));
	connect(wid_mmap, &xMMapWidget::s_restore, this, &DebugWin::rest_mem_map);

//	connect (ui.tbSaveVRam, SIGNAL(released()), this, SLOT(saveVRam()));
// registers
	connect(ui_cpu.boxIM,SIGNAL(valueChanged(int)),this,SLOT(setCPU()));
	connect(ui_cpu.flagIFF1,SIGNAL(stateChanged(int)),this,SLOT(setCPU()));
	connect(ui_cpu.flagIFF2,SIGNAL(stateChanged(int)),this,SLOT(setCPU()));
//	connect(ui.flagGroup,SIGNAL(buttonClicked(int)),this,SLOT(setFlags()));

	block = 0;
	tCount = 0;
	trace = 0;
// subwindows
	dui.setupUi(dumpwin);
	dui.tbSave->addAction(dui.aSaveBin);
	dui.tbSave->addAction(dui.aSaveHobeta);
	dui.tbSave->addAction(dui.aSaveToA);
	dui.tbSave->addAction(dui.aSaveToB);
	dui.tbSave->addAction(dui.aSaveToC);
	dui.tbSave->addAction(dui.aSaveToD);
	dui.leStart->setMin(0);
	dui.leStart->setMax(0xffffff);
	dui.leEnd->setMin(0);
	dui.leEnd->setMax(0xffffff);
	dui.leLen->setMin(1);
	dui.leLen->setMax(0x1000000);

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
	connect(memFiller, &xMemFiller::rqRefill, this, &DebugWin::fillDisasm);

// context menu
	cellMenu = new QMenu(this);
	QMenu* bpMenu = new QMenu("Breakpoints");
	bpMenu->setIcon(QIcon(":/images/stop.png"));
	cellMenu->addMenu(bpMenu);
	bpMenu->addAction(ui_asm.actFetch);
	bpMenu->addAction(ui_asm.actRead);
	bpMenu->addAction(ui_asm.actWrite);
	QMenu* viewMenu = new QMenu("View");
	viewMenu->setIcon(QIcon(":/images/bars.png"));
	cellMenu->addMenu(viewMenu);
	viewMenu->addAction(ui_asm.actViewOpcode);
	viewMenu->addAction(ui_asm.actViewByte);
	viewMenu->addAction(ui_asm.actViewText);
	viewMenu->addAction(ui_asm.actViewWord);
	viewMenu->addAction(ui_asm.actViewAddr);
	cellMenu->addSeparator();
	cellMenu->addAction(ui_asm.actLabelsList);
	cellMenu->addAction(ui_asm.actTraceHere);
	cellMenu->addAction(ui_asm.actShowLabels);
	// NOTE: actions already connected to slots by main menu. no need to double it here

	resize(minimumSize());

	QString path = conf.path.confDir.c_str();
	path.append("/debuga.layout");
	QFile file(path);
	if (file.open(QFile::ReadOnly)) {
		QByteArray state = file.readAll();
		file.close();
		restoreState(state);
	}
}

DebugWin::~DebugWin() {
	QByteArray state = saveState();
	QString path = conf.path.confDir.c_str();
	path.append("/debuga.layout");
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(state);
		file.close();
	}
	dumpwin->deleteLater();
	openDumpDialog->deleteLater();
	memViewer->deleteLater();
	memFiller->deleteLater();
	memFinder->deleteLater();
}

void DebugWin::setShowLabels(bool f) {
	conf.dbg.labels = !!f;
	fillDisasm();
}

void DebugWin::setShowSegment(bool f) {
	conf.dbg.segment = !!f;
	fillDisasm();
	wid_dump->draw();
	//fillDump();
}

void DebugWin::setRomWriteable(bool f) {
	conf.dbg.romwr = !!f;
}

/*
void DebugWin::setDumpCP() {
	int cp = getRFIData(ui.cbCodePage);
	ui.dumpTable->setCodePage(cp);
	fillDump();
}

void DebugWin::chDumpView() {
	int mode,page,pbase,psize;
	Computer* comp = conf.prof.cur->zx;
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
*/

static QFile logfile;

void DebugWin::doStep() {
	Computer* comp = conf.prof.cur->zx;
	if (!ui_asm.cbAccT->isChecked())
		tCount = comp->tickCount;
	compExec(comp);
	if (!fillAll()) {
		ui_asm.dasmTable->setAdr(comp->cpu->pc + comp->cpu->cs.base);
		//fillDisasm();
	}
}

void DebugWin::doTraceHere() {
	doTrace(ui_asm.actTraceHere);
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
	ui_asm.tbTrace->setEnabled(false);
	QApplication::postEvent(this, new QEvent((QEvent::Type)DBG_EVENT_STEP));
}

void DebugWin::stopTrace() {
	trace = 0;
	ui_asm.tbTrace->setEnabled(true);
	if (logfile.isOpen()) logfile.close();
}

void DebugWin::reload() {
	Computer* comp = conf.prof.cur->zx;
	if (comp->mem->snapath) {
		load_file(comp, comp->mem->snapath, FG_SNAPSHOT, 0);
		ui_asm.dasmTable->setAdr(comp->cpu->pc + comp->cpu->cs.base);
	}
	qDebug() << conf.labpath;
	if (!conf.labpath.isEmpty()) {
		loadLabels(conf.labpath.toLocal8Bit().data());
	}
	fillAll();
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
	Computer* comp = conf.prof.cur->zx;
	switch (key) {
		case XCUT_OPTIONS:
			emit wannaOptions();
			break;
		case XCUT_LOAD:
			load_file(comp, NULL, FG_ALL, -1);
			ui_asm.dasmTable->setAdr(comp->cpu->pc + comp->cpu->cs.base);
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
				doTrace(ui_asm.actTrace);
			}
			break;
		case XCUT_STEPOVER:
			len = dasmSome(comp, comp->cpu->pc + comp->cpu->cs.base, drow);
			if (drow.oflag & OF_SKIPABLE) {
				ptr = getBrkPtr(comp, comp->cpu->pc + comp->cpu->cs.base + len);
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
			if (!ui_asm.dasmTable->hasFocus()) break;
			idx = ui_asm.dasmTable->currentIndex();
			i = ui_asm.dasmTable->getData(idx.row(), 0, Qt::UserRole).toInt();
			ptr = getBrkPtr(comp, i);
			stop();
			*ptr |= MEM_BRK_TFETCH;
			break;
		case XCUT_RESET:
			rzxStop(comp);
			compReset(comp, RES_DEFAULT);
			if (!fillAll()) {
				ui_asm.dasmTable->setAdr(comp->cpu->pc + comp->cpu->cs.base);
				//fillDisasm();
			}
			break;
		case XCUT_TRACE:
			doTrace(ui_asm.actTrace);
			break;
		case XCUT_LABELS:
			ui_asm.actShowLabels->setChecked(!conf.dbg.labels);
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

static xRegBunch traceregs;
static QString tracestr;
static dasmData tracemnm;
extern int dasmrd(int, void*);

void DebugWin::customEvent(QEvent* ev) {
	Computer* comp = conf.prof.cur->zx;
	switch(ev->type()) {
		case DBG_EVENT_STEP:
			if ((traceType == DBG_TRACE_LOG) && logfile.isOpen()) {
				dasmSome(comp, comp->cpu->pc + comp->cpu->cs.base, tracemnm);
				doStep();
				traceregs = cpuGetRegs(comp->cpu);
				tracestr = tracemnm.command.leftJustified(24,' ');
				int i = 0;
				while (traceregs.regs[i].id != REG_NONE) {
					if (i!=0) tracestr.append(" ");
					if (traceregs.regs[i].id != REG_EMPTY) {			// mustn't be visible
						tracestr.append(traceregs.regs[i].name).append(":");
						switch(traceregs.regs[i].type & REG_TMASK) {
							case REG_BIT: tracestr.append(traceregs.regs[i].value ? "1" : "0"); break;
							case REG_BYTE: tracestr.append(gethexbyte(traceregs.regs[i].value)); break;
							case REG_WORD: tracestr.append(gethexword(traceregs.regs[i].value)); break;
							case REG_24: tracestr.append(gethex6(traceregs.regs[i].value)); break;
							case REG_32: tracestr.append(gethexint(traceregs.regs[i].value)); break;
						}
					}
					i++;
				}
				tracestr.append("\n");
				logfile.write(tracestr.toUtf8());
			} else {
				doStep();
			}
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

void DebugWin::moveEvent(QMoveEvent* ev) {
	if (!isVisible()) return;
	conf.dbg.pos = ev->pos();
}

void DebugWin::resizeEvent(QResizeEvent* ev) {
	if (!isVisible()) return;
	conf.dbg.siz = ev->size();
}

void setSignal(QLabel* lab, int on) {
	QFont fnt = lab->font();
	fnt.setBold(on);
	lab->setFont(fnt);
}

/*
void DebugWin::fillTabs() {
}
*/

void DebugWin::fillNotCPU() {
	Computer* comp = conf.prof.cur->zx;
	ui_asm.labTcount->setText(QString("%0 / %1").arg(comp->tickCount - tCount).arg(comp->frmtCount));

	fillMem();
	xDockWidget* dw;
	foreach(void* ptr, dockWidgets) {
		dw = static_cast<xDockWidget*>(ptr);
		if (!dw->isHidden())
			dw->draw();
	}

	setSignal(ui_misc.labDOS, comp->dos);
	setSignal(ui_misc.labROM, comp->rom);
	setSignal(ui_misc.labCPM, comp->cpm);
	setSignal(ui_misc.labINT, comp->cpu->intrq & comp->cpu->inten);

	ui_misc.labRX->setNum(comp->vid->ray.x);
	setSignal(ui_misc.labRX, comp->vid->hblank);
	ui_misc.labRY->setNum(comp->vid->ray.y);
	setSignal(ui_misc.labRY, comp->vid->vblank);

	if (memViewer->isVisible())
		memViewer->fillImage();
	fillStack();
	fillPorts();
}

bool DebugWin::fillAll() {
	fillCPU();
	fillNotCPU();
	return fillDisasm();
}


void DebugWin::setScrAtr(int adr, int atr) {
	wid_zxscr->setAddress(adr, atr);
//	ui.leScr->setValue(adr);
//	ui.leAtr->setValue(atr);
}

// ...

void DebugWin::chLayout() {
	switch (conf.prof.cur->zx->cpu->type) {
		case CPU_Z80:
			ui_cpu.boxIM->setEnabled(true);
			break;
		case CPU_LR35902:
			ui_cpu.boxIM->setEnabled(false);
			break;
		case CPU_6502:
			ui_cpu.boxIM->setEnabled(false);
			break;
		default:
			break;
	}
}

int dbg_get_reg_adr(CPU* cpu, xRegister* reg) {
	int a = reg->value;
	if (reg->type & REG_SEG) {
		a = reg->base;
	} else if (cpu->type == CPU_I80286) {
		a += reg->base;
	}
	return a;
}

void DebugWin::regClick(QMouseEvent* ev) {
	xLabel* lab = qobject_cast<xLabel*>(sender());
	int id = lab->id;
	if (id < 0) return;
	Computer* comp = conf.prof.cur->zx;
	xRegBunch bunch = cpuGetRegs(comp->cpu);
	xRegister reg = bunch.regs[id];
	int adr = dbg_get_reg_adr(comp->cpu, &reg);
	qDebug() << adr;
	switch (ev->button()) {
		case Qt::RightButton:
			//ui.dumpTable->setAdr(adr);
			wid_dump->setAdr(adr);
			break;
		case Qt::LeftButton:
			ui_asm.dasmTable->setAdr(adr, 1);
			break;
		default:
			break;
	}
}

// fdc

/*
void DebugWin::fillFDC() {
	if (ui.tabsPanel->currentWidget() != ui.fdcTab) return;
	Computer* comp = conf.prof.cur->zx;
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
	ui.flpInt->setText(comp->dif->fdc->intr ? "1" : "0");
	ui.flpDma->setText(comp->dif->fdc->dma ? "1" : "0");
	ui.flpIntEn->setText(comp->dif->fdc->inten ? "1" : "0");

	ui.flpCurL->setText(QString(QChar(('A' + comp->dif->fdc->flp->id) & 0xff)));
	ui.flpRdyL->setText((comp->dif->fdc->flp->insert && comp->dif->fdc->flp->door) ? "1" : "0");
	ui.flpTrkL->setText(gethexbyte(comp->dif->fdc->flp->trk));
	ui.flpPosL->setText(gethexword(comp->dif->fdc->flp->pos));
	ui.flpIdxL->setText(comp->dif->fdc->flp->index ? "1" : "0");
	ui.flpDataL->setText(comp->dif->fdc->flp->insert ? gethexbyte(flpRd(comp->dif->fdc->flp, comp->dif->fdc->side)): "--"); comp->dif->fdc->flp->rd = 0;
	ui.flpMotL->setText(comp->dif->fdc->flp->motor ? "1" : "0");
}
*/
// CPU

void DebugWin::fillFlags(const char* fnam) {
	if (fnam == NULL)
		fnam = cpuGetRegs(conf.prof.cur->zx->cpu).flags;
	int flgcnt = strlen(fnam);
	QString allflags = QString(fnam).rightJustified(16, '-');
	for (int i = 0; i < 16; i++) {
		if (i < flgcnt) {
			dbgFlagBox[i]->setVisible(true);
			dbgFlagLabs[i]->setVisible(true);
			dbgFlagLabs[i]->setText(allflags.at(15 - i));
			dbgFlagBox[i]->setChecked(conf.prof.cur->zx->cpu->f & (1 << i));
		} else {
			dbgFlagBox[i]->setVisible(false);
			dbgFlagLabs[i]->setVisible(false);
		}
	}
}

void DebugWin::fillCPU() {
	block = 1;
//	Computer* comp = conf.prof.cur->zx;
	CPU* cpu = conf.prof.cur->zx->cpu;
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
	ui_cpu.boxIM->setValue(cpu->imode);
	ui_cpu.flagIFF1->setChecked(cpu->iff1);
	ui_cpu.flagIFF2->setChecked(cpu->iff2);
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
	conf.prof.cur->zx->cpu->f = f;
	fillCPU();
}

void DebugWin::setCPU() {
	if (block) return;
	Computer* comp = conf.prof.cur->zx;
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
	cpu->imode = ui_cpu.boxIM->value() & 0xff;
	cpu->iff1 = ui_cpu.flagIFF1->isChecked() ? 1 : 0;
	cpu->iff2 = ui_cpu.flagIFF2->isChecked() ? 1 : 0;
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
	Computer* comp = conf.prof.cur->zx;
	ui_misc.labPG0->setText(getPageName(comp->mem->map[0x00]));
	ui_misc.labPG1->setText(getPageName(comp->mem->map[0x40]));
	ui_misc.labPG2->setText(getPageName(comp->mem->map[0x80]));
	ui_misc.labPG3->setText(getPageName(comp->mem->map[0xc0]));
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
		ui_asm.dasmTable->setAdr(conf.prof.cur->labels[lab].adr, 1);
}

// disasm table

int rdbyte(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = -1;
//	if (comp->hw->id == HW_IBM_PC) {
//		res = comp->hw->mrd(comp, adr, 0);
//	} else {
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
//	}
	return res;
}

int DebugWin::fillDisasm() {
	conf.dbg.hideadr = ui_asm.actHideAddr->isChecked() ? 1 : 0;
	conf.dbg.labels = ui_asm.actShowLabels->isChecked() ? 1 : 0;
	return ui_asm.dasmTable->updContent();
}

void DebugWin::saveDasm() {
	QString path = QFileDialog::getSaveFileName(this, "Save disasm",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QFile file(path);
	dasmData drow;
	QList<dasmData> list;
	Computer* comp = conf.prof.cur->zx;
	if (file.open(QFile::WriteOnly)) {
		QTextStream strm(&file);
		int adr = (blockStart < 0) ? 0 : (blockStart & comp->mem->busmask);
		int end = (blockEnd < 0) ? comp->mem->busmask : (blockEnd & comp->mem->busmask);
		int work = 1;
		strm << "; Created by Xpeccy deBUGa\n\n";
		strm << "\tORG 0x" << gethexword(adr) << "\n\n";
		while ((adr <= end) && work) {
			list = getDisasm(comp, adr);
			foreach (drow, list) {
				if (adr > comp->mem->busmask)
					work = 0;		// address overfill (FFFF+)
				if (drow.isequ) {
					strm << drow.aname << ":";
					strm << drow.command;
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

/*
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
*/

// maping

void DebugWin::mapClear() {
	if (!areSure("Clear memory mapping?")) return;
	Computer* comp = conf.prof.cur->zx;
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
	Computer* comp = conf.prof.cur->zx;
	int adr = comp->cpu->sp + comp->cpu->ss.base;
	QString str;
	for (int i = -2; i < 10; i+=2) {
		str.append(gethexbyte(rdbyte(adr+i+1, comp)));
		str.append(gethexbyte(rdbyte(adr+i, comp)));
	}
	ui_misc.labSPm2->setText(str.left(4));
	ui_misc.labSP->setText(str.mid(4,4));
	ui_misc.labSP2->setText(str.mid(8,4));
	ui_misc.labSP4->setText(str.mid(12,4));
	ui_misc.labSP6->setText(str.mid(16,4));
	ui_misc.labSP8->setText(str.mid(20,4));
}

// ports

void DebugWin::fillPorts() {
	Computer* comp = conf.prof.cur->zx;
	xPortValue* tab = hwGetPorts(comp);
	int i = 0;
	if (tab) {
		if (tab[0].port > 0) {
			QLabel* wid;
			while ((tab[i].port > 0) && (i < 32)) {
				wid = (QLabel*)(ui_misc.formPort->itemAt(i, QFormLayout::LabelRole)->widget());
				wid->setVisible(true);
				wid->setText(gethexword(tab[i].port));
				wid = (QLabel*)(ui_misc.formPort->itemAt(i, QFormLayout::FieldRole)->widget());
				wid->setVisible(true);
				wid->setText(gethexbyte(tab[i].value));
				i++;
			}
			ui_misc.labPorts->setVisible(true);
		} else {
			ui_misc.labPorts->setVisible(false);
		}
	} else {
		ui_misc.labPorts->setVisible(false);
	}
	while (i < 32) {
		ui_misc.formPort->itemAt(i, QFormLayout::LabelRole)->widget()->setVisible(false);
		ui_misc.formPort->itemAt(i, QFormLayout::FieldRole)->widget()->setVisible(false);
		i++;
	}
}

// breakpoint

int DebugWin::getAdr() {
	int adr;
//	int col;
	QModelIndex idx;
	Computer* comp = conf.prof.cur->zx;

//	if (ui.dumpTable->hasFocus()) {
//		idx = ui.dumpTable->currentIndex();
//		col = idx.column();
//		adr = (ui.dumpTable->getAdr() + (idx.row() << 3));
//		if ((col > 0) && (col < 9)) {
//			adr += idx.column() - 1;
//		}
//	} else {
		idx = ui_asm.dasmTable->currentIndex();
		adr = ui_asm.dasmTable->getData(idx.row(), 0, Qt::UserRole).toInt();		// already +cs.base
//	}

	adr &= comp->mem->busmask;
	return adr;
}

void DebugWin::brkRequest(int t, int m, int a) {
	int bgn, end, fadr;
	if ((a < blockStart) || (a > blockEnd)) {	// pointer outside block : process 1 cell
		bgn = a;
		end = a;
	} else {								// pointer inside block : process all block
		bgn = blockStart;
		end = blockEnd;
	}
	if (end < bgn) {
		fadr = bgn;
		bgn = end;
		end = fadr;
	}
	brkSet(t, m, bgn, end);
	fillDisasm();
	wid_dump->draw();
	wid_brk->draw();
}

void DebugWin::putBreakPoint() {
	int adr = getAdr();
	if (adr < 0) return;
	doBreakPoint(adr);
	cellMenu->move(QCursor::pos());
	cellMenu->show();
}

void DebugWin::doBreakPoint(unsigned short adr) {
//	bpAdr = adr;
	unsigned char flag = getBrk(conf.prof.cur->zx, adr);
	ui_asm.actFetch->setChecked(flag & MEM_BRK_FETCH);
	ui_asm.actRead->setChecked(flag & MEM_BRK_RD);
	ui_asm.actWrite->setChecked(flag & MEM_BRK_WR);
}

// TODO: breakpoints on block
void DebugWin::chaCellProperty(QAction* act) {
	int data = act->data().toInt();		// flag to change. b4..7 = type, b0..3 = brk
	int adr = getAdr();
	int bgn, end;
	unsigned char bt;
	int fadr;
	unsigned char* ptr;
	if ((adr < blockStart) || (adr > blockEnd)) {	// pointer outside block : process 1 cell
		bgn = adr;
		end = adr;
	} else {								// pointer inside block : process all block
		bgn = blockStart;
		end = blockEnd;
	}
	if (end < bgn) {
		fadr = bgn;
		bgn = end;
		end = fadr;
	}
//	int proc = 1;
	bt = 0;
	xAdr xadr;
	xAdr xend;
	Computer* comp = conf.prof.cur->zx;
	if (ui_asm.actFetch->isChecked()) bt |= MEM_BRK_FETCH;
	if (ui_asm.actRead->isChecked()) bt |= MEM_BRK_RD;
	if (ui_asm.actWrite->isChecked()) bt |= MEM_BRK_WR;
	adr = bgn;
	if (data & MEM_BRK_ANY) {				// if set breakpoint
		if (ui_asm.dasmTable->hasFocus()) {			// from disasm table
			xadr = mem_get_xadr(comp->mem, bgn);
			xend = mem_get_xadr(comp->mem, end);
			if (xadr.type == MEM_ROM) {
				bt |= MEM_BRK_ROM;
			} else if (xadr.type == MEM_RAM) {
				bt |= MEM_BRK_RAM;
			} else {
				bt |= MEM_BRK_SLT;
			}
			brkSet(BRK_MEMCELL, bt, xadr.abs, xend.abs);
//		} else if (ui.dumpTable->hasFocus()) {		// from dump table
//			int fadr = getRFIData(ui.cbDumpView);
//			switch(fadr) {	// XVIEW_RAM/ROM/CPU
//				case XVIEW_CPU:
//					xadr = mem_get_xadr(comp->mem, bgn);
//					xend = mem_get_xadr(comp->mem, end);
//					switch (xadr.type) {
//						case MEM_ROM: bt |= MEM_BRK_ROM; break;
//						case MEM_RAM: bt |= MEM_BRK_RAM; break;
//						default: bt |= MEM_BRK_SLT; break;
//					}
//					brkSet(BRK_MEMCELL, bt, xadr.abs, xend.abs);
//					break;
//				case XVIEW_ROM:
//				case XVIEW_RAM: bt |= (fadr == XVIEW_ROM) ? MEM_BRK_ROM : MEM_BRK_RAM;
//					brkSet(BRK_MEMCELL, bt, bgn, end);
//					break;
//			}
		}
	} else {						// change cell type
		while (adr <= end) {
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
			adr++;
		}
	}
	wid_brk->draw();
	//ui.bpList->update();
	fillDisasm();
	wid_dump->draw();
	//fillDump();
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
	int end = dui.leEnd->getMax();
	if (start + len >= end) {
		len = end - start;
		dui.leLen->setValue(len);
	}
	end = start + len - 1;
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
		res.append(rdbyte(adr, conf.prof.cur->zx));
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
	Floppy* flp = conf.prof.cur->zx->dif->fdc->flop[idx & 3];
	if (!flp->insert) {
		flp_insert(flp, NULL);
		trd_format(flp);
	}
	TRFile dsc = diskMakeDescriptor(name.toStdString().c_str(), 'C', start, len);
	if (diskCreateFile(flp, dsc, (unsigned char*)data.data(), data.size()) == ERR_OK)
		dumpwin->hide();

}

// videoram

void DebugWin::saveVRam() {
	QString path = QFileDialog::getSaveFileName(this, "Save video ram", "", "All files (*)", nullptr, QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write((char*)conf.prof.cur->zx->vid->ram, MEM_256K);
		file.close();
	}
}

// memfinder

void DebugWin::doFind() {
	Computer* comp = conf.prof.cur->zx;
	memFinder->mem = comp->mem;
	if (memFinder->adr < 0)
		memFinder->adr = (ui_asm.dasmTable->getAdr() + 1) & comp->mem->busmask;
	memFinder->show();
}

void DebugWin::onFound(int adr) {
	ui_asm.dasmTable->setAdr(adr);
	wid_dump->setAdr(adr);
	// ui.dumpTable->setAdr(adr);
}

// memfiller

void DebugWin::doFill() {
	Computer* comp = conf.prof.cur->zx;
	memFiller->start(comp->mem, blockStart, blockEnd);
}

// spr scanner

void DebugWin::doMemView() {
	Computer* comp = conf.prof.cur->zx;
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
	int end = oui.leEnd->getMax();
	if (start + len > end) {
		start = end - len + 1;
	} else {
		end = start + len - 1;
	}
	oui.leStart->setValue(start);
	oui.leEnd->setValue(end);
	oui.leStart->setCursorPosition(pos);
}

void DebugWin::loadDump() {
	if (dumpPath.isEmpty()) return;
	int res = loadDUMP(conf.prof.cur->zx, dumpPath.toLocal8Bit().data(),oui.leStart->text().toInt(NULL,16));
	fillAll();
	if (res == ERR_OK) {
		openDumpDialog->hide();
	} else {
		shitHappens("Can't open file");
	}
}

// ps/2 widget (tmp here)

xPS2Widget::xPS2Widget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("PS2WIDGET");
	hwList << HWG_PC;
}

QString get_hex_queue_z(unsigned long d) {
	QString r;
	while (d & 0xff) {
		if (!r.isEmpty()) r.append(",");
		r.append(gethexbyte(d & 0xff));
		d >>= 8;
	}
	return r;
}

QString get_hex_queue_n(unsigned long d, int l) {
	QString r;
	while (l > 0) {
		if (!r.isEmpty()) r.append(",");
		r.append(gethexbyte(d & 0xff));
		d >>= 8;
		l--;
	}
	return r;
}

void xPS2Widget::draw() {
	PS2Ctrl* ctrl = conf.prof.cur->zx->ps2c;
	Keyboard* k = ctrl->kbd;
	Mouse* m = ctrl->mouse;
	ui.lab_ps2ctrl->setText(getbinbyte(ctrl->ram[0x00]));
	ui.lab_ps2status->setText(getbinbyte(ctrl->status));
	ui.lab_ps2outbuf->setText(gethexbyte(ctrl->outbuf));
	ui.lab_ps2inbuf->setText(gethexbyte(ctrl->inbuf));
	ui.lab_ps2kdata->setText(k->outbuf ? get_hex_queue_z(k->outbuf) : "-");
	ui.lab_ps2mdata->setText(m->queueSize ? get_hex_queue_n(m->outbuf, m->queueSize) : "-");
}
