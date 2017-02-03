#include "xcore/xcore.h"

#include <stdio.h>

#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QTemporaryFile>

#include "debuger.h"
#include "emulwin.h"
#include "filer.h"
#include "xgui/xgui.h"
#include "filer.h"

// trace type
enum {
	DBG_TRACE_ALL = 0x100,
	DBG_TRACE_INT,
	DBG_TRACE_HERE
};

// memory cell type (bits 4..7)
enum {
	DBG_VIEW_CODE = 0x00,
	DBG_VIEW_BYTE = 0x10,
	DBG_VIEW_WORD = 0x20,
	DBG_VIEW_ADDR = 0x30,
	DBG_VIEW_TEXT = 0x40
};

void DebugWin::start(Computer* c) {
	comp = c;
	chLayout();
	if (!fillAll()) {
		disasmAdr = comp->cpu->pc;
		fillAll();
	}
	fillBrkTable();
	updateScreen();
	if (!comp->vid->tail)
		vidDarkTail(comp->vid);
	ui.tabsPanel->setTabEnabled(3, comp->hw->type == HW_GBC);

	move(winPos);
	ui.dasmTable->setFocus();
	comp->vid->debug = 1;
	comp->debug = 1;
	comp->brk = 0;
	show();
	memViewer->move(memViewer->winPos);
	if (memViewer->vis) memViewer->show();
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
		case XTYPE_DUMP: edt->setInputMask("Hhhhhhhhhh"); break;
		case XTYPE_BYTE: edt->setInputMask("Hh"); break;
	}
	return edt;
}

DebugWin::DebugWin(QWidget* par):QDialog(par) {
	int col,row;
	ui.setupUi(this);
	QTableWidgetItem* itm;

	showLabels = 1;
	ui.actShowLabels->setChecked(showLabels);
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
	for (row = 0; row < ui.dasmTable->rowCount(); row++) {
		for (col = 0; col < ui.dasmTable->columnCount(); col++) {
			itm = new QTableWidgetItem;
			if (col == 3) {
				itm->setTextAlignment(Qt::AlignRight);
			}
			ui.dasmTable->setItem(row, col, itm);
		}
	}
	ui.dasmTable->setColumnWidth(0,100);
	ui.dasmTable->setColumnWidth(1,75);
	ui.dasmTable->setColumnWidth(2,130);
	ui.dasmTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_LABEL));
	ui.dasmTable->setItemDelegateForColumn(1, new xItemDelegate(XTYPE_DUMP));
	row = ui.dasmTable->font().pixelSize();
	if (row < 0) row = 17;
	ui.dasmTable->setFixedHeight(ui.dasmTable->rowCount() * row + 8);

	ui.tbBreak->addAction(ui.actFetch);
	ui.tbBreak->addAction(ui.actRead);
	ui.tbBreak->addAction(ui.actWrite);

	ui.tbView->addAction(ui.actViewOpcode);
	ui.tbView->addAction(ui.actViewByte);
	ui.tbView->addAction(ui.actViewText);
	ui.tbView->addAction(ui.actViewWord);
	ui.tbView->addAction(ui.actViewAddr);

	ui.tbSaveDasm->addAction(ui.actLoadMap);
	ui.tbSaveDasm->addAction(ui.actSaveMap);
	ui.tbSaveDasm->addAction(ui.actDisasm);

	ui.tbTrace->addAction(ui.actTrace);
	ui.tbTrace->addAction(ui.actTraceHere);
	ui.tbTrace->addAction(ui.actTraceINT);

	connect(ui.dasmTable,SIGNAL(cellChanged(int,int)),this,SLOT(dasmEdited(int,int)));
	connect(ui.dasmTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	connect(ui.dasmTable,SIGNAL(rqRefill()),this,SLOT(fillDisasm()));

	connect(ui.tbLabels, SIGNAL(toggled(bool)),this,SLOT(setShowLabels(bool)));
	connect(ui.tbView, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui.tbBreak, SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(ui.tbTrace, SIGNAL(triggered(QAction*)),this,SLOT(doTrace(QAction*)));
	connect(ui.actLoadMap, SIGNAL(triggered(bool)),this,SLOT(loadMap()));
	connect(ui.actSaveMap, SIGNAL(triggered(bool)),this,SLOT(saveMap()));
	connect(ui.actDisasm, SIGNAL(triggered(bool)),this,SLOT(saveDasm()));

// dump table
	for (row = 0; row < ui.dumpTable->rowCount(); row++) {
		for (col = 0; col < ui.dumpTable->columnCount(); col++) {
			ui.dumpTable->setItem(row, col, new QTableWidgetItem);
		}
		ui.dumpTable->item(row, 9)->setTextAlignment(Qt::AlignRight);
	}
	ui.dumpTable->setColumnWidth(0,50);
	for (col = 1; col < 9; col++) {
		ui.dumpTable->setColumnWidth(col, 25);
	}
	ui.dumpTable->setItemDelegate(new xItemDelegate(XTYPE_BYTE));
	ui.dumpTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_ADR));
	ui.dumpTable->setItemDelegateForColumn(9, new xItemDelegate(XTYPE_NONE));
	connect(ui.dumpTable,SIGNAL(cellChanged(int,int)),this,SLOT(dumpEdited(int,int)));
	connect(ui.dumpTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
	row = ui.dumpTable->font().pixelSize();
	if (row < 0) row = 17;
	ui.dumpTable->setFixedHeight(ui.dumpTable->rowCount() * row + 8);
// registers
	connect(ui.editAF, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editBC, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editDE, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editHL, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editAFa, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editBCa, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editDEa, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editHLa, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editPC, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editSP, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editIX, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editIY, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.editIR, SIGNAL(textChanged(QString)), this, SLOT(setZ80()));
	connect(ui.boxIM,SIGNAL(valueChanged(int)),this,SLOT(setZ80()));
	connect(ui.flagIFF1,SIGNAL(stateChanged(int)),this,SLOT(setZ80()));
	connect(ui.flagIFF2,SIGNAL(stateChanged(int)),this,SLOT(setZ80()));
	connect(ui.flagGroup,SIGNAL(buttonClicked(int)),this,SLOT(setFlags()));
// infoslots
	scrImg = QImage(256, 192, QImage::Format_RGB888);
	ui.scrLabel->setFixedSize(256,192);
	connect(ui.sbScrBank,SIGNAL(valueChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrAlt,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));
	connect(ui.cbScrAtr,SIGNAL(stateChanged(int)),this,SLOT(updateScreen()));
	ui.tbAddBrk->setEnabled(false);
	ui.tbDelBrk->setEnabled(false);
	connect(ui.bpList,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(goToBrk(QModelIndex)));
// gb tab
	connect(ui.gbModeGroup, SIGNAL(buttonClicked(int)), this, SLOT(fillGBoy()));
	connect(ui.sbTileset, SIGNAL(valueChanged(int)), this, SLOT(fillGBoy()));
	connect(ui.sbTilemap, SIGNAL(valueChanged(int)), this, SLOT(fillGBoy()));


	setFixedSize(size());
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

	memViewer = new MemViewer();
	connect(ui.tbMemView, SIGNAL(clicked()),this,SLOT(doMemView()));
// context menu
	cellMenu = new QMenu(this);
	cellMenu->addAction(ui.actTraceHere);
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

	connect(ui.actTraceHere, SIGNAL(triggered(bool)),this,SLOT(doTraceHere()));
	connect(bpMenu,SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
	connect(viewMenu,SIGNAL(triggered(QAction*)),this,SLOT(chaCellProperty(QAction*)));
}

DebugWin::~DebugWin() {
	delete(dumpwin);
	delete(openDumpDialog);
	delete(memViewer);
}

void DebugWin::setShowLabels(bool f) {
	showLabels = f ? 1 : 0;
	fillDisasm();
}

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

void DebugWin::scrollUp() {
	if (ui.dumpTable->hasFocus()) {
		dumpAdr = (dumpAdr - 8) & 0xffff;
		fillDump();
	} else if (ui.dasmTable->hasFocus()) {
		disasmAdr = getPrevAdr(disasmAdr);
		fillDisasm();
	}
}

void DebugWin::scrollDown() {
	if (ui.dumpTable->hasFocus()) {
		dumpAdr = (dumpAdr + 8) & 0xffff;
		fillDump();
	} else if (ui.dasmTable->hasFocus()) {
		disasmAdr = ui.dasmTable->item(1,0)->data(Qt::UserRole).toInt();
		fillDisasm();
	}
}

void DebugWin::doStep() {
	tCount = comp->tickCount;
	compExec(comp);
	if (!fillAll()) {
		disasmAdr = comp->cpu->pc;
		fillDisasm();
	}
	if ((traceType == DBG_TRACE_INT) && (comp->cpu->inth))
		trace = 0;
	if ((traceType == DBG_TRACE_HERE) && (comp->cpu->pc == traceAdr))
		trace = 0;
	if (trace) {
		QTimer::singleShot(10,this,SLOT(doStep()));
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
	fillBrkTable();
}

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	if (trace) {
		trace = 0;
		return;
	}
	int i;
	unsigned short pc = comp->cpu->pc;
	unsigned char* ptr;
	int offset = 8 * (ui.dumpTable->rowCount() - 1);
	QString com;
	int row;
	int pos;
	switch(ev->modifiers()) {
		case Qt::ControlModifier:
			switch(ev->key()) {
				case Qt::Key_S:
					doSaveDump();
					break;
				case Qt::Key_O:
					doOpenDump();
					break;
				case Qt::Key_T:
					doTrace(ui.actTrace);
					doStep();
					break;
				case Qt::Key_L:
					setShowLabels(showLabels ? false : true);
					ui.actShowLabels->setChecked(showLabels);
					break;
				case Qt::Key_Space:
					switchBP(0);
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
		case Qt::AltModifier:
			switch (ev->key()) {
				case Qt::Key_F:
					switchBP(MEM_BRK_FETCH);
					break;
				case Qt::Key_R:
					switchBP(MEM_BRK_RD);
					break;
				case Qt::Key_W:
					switchBP(MEM_BRK_WR);
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
					comp->cpu->pc = getAdr();
					fillZ80();
					fillDisasm();
					break;
				case Qt::Key_F2:
					switchBP(MEM_BRK_FETCH);
					break;
				case Qt::Key_F4:
					row = ui.dasmTable->currentRow();
					if (row < 0) break;
					com = ui.dasmTable->item(row, 2)->data(Qt::UserRole).toString();
					pos = com.indexOf(QRegExp("[0-9A-F]{4,4}"));
					if (pos < 0) break;
					pos = com.mid(pos, 4).toInt(NULL, 16);
					//jumpHistory.append(ui.dasmTable->item(row, 0)->data(Qt::UserRole).toInt());
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
						for (i = 0; i < ui.dasmTable->rowCount() - 1; i++) {
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
						disasmAdr = ui.dasmTable->item(ui.dasmTable->rowCount() - 1, 0)->data(Qt::UserRole).toInt();
						fillDisasm();
					}
					break;
				case Qt::Key_Up:
					scrollUp();
					break;
				case Qt::Key_Down:
					scrollDown();
					break;
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
					i = ui.dasmTable->item(ui.dasmTable->currentRow(),0)->data(Qt::UserRole).toInt();
					ptr = getBrkPtr(comp, i);
					*ptr ^= MEM_BRK_TFETCH;
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

QString gethexword(int num) {return QString::number(num+0x10000,16).right(4).toUpper();}
QString gethexbyte(uchar num) {return QString::number(num+0x100,16).right(2).toUpper();}
QString getbinbyte(uchar num) {return QString::number(num+0x100,2).right(8).toUpper();}

void setSignal(QLabel* lab, int on) {
	QString col(on ? "160,255,160" : "255,160,160");
	// QString bld(on ? "font-weight:bold" : "");
	lab->setStyleSheet(QString("background-color: rgb(%0)").arg(col));
}

bool DebugWin::fillAll() {
	ui.labTcount->setText(QString::number(comp->tickCount - tCount));
	fillZ80();
	fillMem();
	fillDump();
	fillFDC();
	fillGBoy();
	if (ui.scrLabel->isVisible())
		updateScreen();
//	ui.rzxTab->setEnabled(comp->rzx.play);
	if (comp->rzx.play) fillRZX();
	ui.labRX->setNum(comp->vid->ray.x);
	ui.labRY->setNum(comp->vid->ray.y);
	setSignal(ui.labDOS, comp->dos);
	setSignal(ui.labROM, comp->rom);
	setSignal(ui.labCPM, comp->cpm);
	setSignal(ui.labHBlank, comp->vid->hblank);
	setSignal(ui.labVBlank, comp->vid->vblank);
	setSignal(ui.labINT, comp->cpu->inth);
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
			setFlagNames("SZ5H3PNC");
			ui.cpuGrid->setEnabled(true);
			ui.editAFa->setEnabled(true);
			ui.editBCa->setEnabled(true);
			ui.editDEa->setEnabled(true);
			ui.editHLa->setEnabled(true);
			ui.editIX->setEnabled(true);
			ui.editIY->setEnabled(true);
			ui.editIR->setEnabled(true);
			ui.boxIM->setEnabled(true);
			break;
		case CPU_LR35902:
			ui.cpuGrid->setEnabled(true);
			ui.editAFa->setEnabled(false);
			ui.editBCa->setEnabled(false);
			ui.editDEa->setEnabled(false);
			ui.editHLa->setEnabled(false);
			ui.editIX->setEnabled(false);
			ui.editIY->setEnabled(false);
			ui.editIR->setEnabled(false);
			ui.boxIM->setEnabled(false);
			setFlagNames("ZNHC----");
			break;
		default:
			ui.cpuGrid->setEnabled(false);
			setFlagNames("--------");
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

void DebugWin::fillZ80() {
	block = 1;
	CPU* cpu = comp->cpu;
	setLEReg(ui.editAF, cpu->af);
	setLEReg(ui.editBC, cpu->bc);
	setLEReg(ui.editDE, cpu->de);
	setLEReg(ui.editHL, cpu->hl);
	setLEReg(ui.editAFa, cpu->af_);
	setLEReg(ui.editBCa, cpu->bc_);
	setLEReg(ui.editDEa, cpu->de_);
	setLEReg(ui.editHLa, cpu->hl_);
	setLEReg(ui.editPC, cpu->pc);
	setLEReg(ui.editSP, cpu->sp);
	setLEReg(ui.editIX, cpu->ix);
	setLEReg(ui.editIY, cpu->iy);
	setLEReg(ui.editIR, (cpu->i << 8) | (cpu->r & 0x7f) | (cpu->r7 & 0x80));

	ui.boxIM->setValue(cpu->imode);
	ui.flagIFF1->setChecked(cpu->iff1);
	ui.flagIFF2->setChecked(cpu->iff2);
	fillFlags();
	block = 0;
	fillStack();
}

void DebugWin::setFlags() {
	if (block) return;
	unsigned short af = comp->cpu->af & 0xff00;
	if (ui.cbF7->isChecked()) af |= 0x80;
	if (ui.cbF6->isChecked()) af |= 0x40;
	if (ui.cbF5->isChecked()) af |= 0x20;
	if (ui.cbF4->isChecked()) af |= 0x10;
	if (ui.cbF3->isChecked()) af |= 0x08;
	if (ui.cbF2->isChecked()) af |= 0x04;
	if (ui.cbF1->isChecked()) af |= 0x02;
	if (ui.cbF0->isChecked()) af |= 0x01;
	comp->cpu->af = af;
	setLEReg(ui.editAF, af);
	fillDisasm();
}

void DebugWin::setZ80() {
	if (block) return;
	CPU* cpu = comp->cpu;
	cpu->af = ui.editAF->text().toInt(NULL,16);
	cpu->bc = ui.editBC->text().toInt(NULL,16);
	cpu->de = ui.editDE->text().toInt(NULL,16);
	cpu->hl = ui.editHL->text().toInt(NULL,16);
	cpu->af_ = ui.editAFa->text().toInt(NULL,16);
	cpu->bc_ = ui.editBCa->text().toInt(NULL,16);
	cpu->de_ = ui.editDEa->text().toInt(NULL,16);
	cpu->hl_ = ui.editHLa->text().toInt(NULL,16);
	cpu->pc = ui.editPC->text().toInt(NULL,16);
	cpu->sp = ui.editSP->text().toInt(NULL,16);
	cpu->ix = ui.editIX->text().toInt(NULL,16);
	cpu->iy = ui.editIY->text().toInt(NULL,16);
	int ir = ui.editIR->text().toInt(NULL,16);
	cpu->i = (ir & 0xff00) >> 8;
	cpu->r = ir & 0xff;
	cpu->r7 = cpu->r & 0x80;
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
		case MEM_RAM: res = "RAM-"; break;
		case MEM_ROM: res = "ROM-"; break;
		case MEM_EXT: res = "EXT-"; break;
		case MEM_SLOT: res = "SLT-"; break;
		default: res = "----"; break;
	}
	res.append(QString::number(pg.num));
	return res;
}

void DebugWin::fillMem() {
	ui.labPG0->setText(getPageName(comp->mem->map[0]));
	ui.labPG1->setText(getPageName(comp->mem->map[1]));
	ui.labPG2->setText(getPageName(comp->mem->map[2]));
	ui.labPG3->setText(getPageName(comp->mem->map[3]));
}

// labels

QString DebugWin::findLabel(int adr, int type, int bank) {
	QString lab;
	if (!showLabels)
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

void DebugWin::saveMap() {
	QString path = QFileDialog::getSaveFileName(this, "Save deBUGa project","","deBUGa project (*.xdbg)",NULL,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	if (!path.endsWith(".xdbg",Qt::CaseInsensitive))
		path.append(".xdbg");
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		QDataStream strm(&file);
		strm << QString("deBUGa");		// signature
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
		strm << 0x00 << 0x00 << 0x00;			// end of labels list
		strm << QString();
		fwritepack(strm, comp->brkRamMap, 0x400000);
		fwritepack(strm, comp->brkRomMap, 0x400000);
		file.close();
	}
}

void DebugWin::loadMap() {
	QString path = QFileDialog::getOpenFileName(this, "Open deBUGa project","","deBUGa project (*.xdbg)");
	if (path.isEmpty()) return;
	QFile file(path);
	QString key;
	// unsigned char bt;
	xAdr xadr;
	if (file.open(QFile::ReadOnly)) {
		QDataStream strm(&file);
		strm >> key;
		if (key != QString("deBUGa")) {
			shitHappens("Wrong signature");
		} else {
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
			freadpack(strm, comp->brkRamMap, 0x400000);
			freadpack(strm, comp->brkRomMap, 0x400000);
			fillAll();
		}
		file.close();
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

DasmRow getDisasm(Computer* comp, unsigned short& adr) {
	DasmRow drow;
	drow.mem = 0;
	drow.cond = 0;
	xMnem mn;
	drow.adr = adr;
	drow.ispc = (comp->cpu->pc == adr) ? 1 : 0;	// check if this is PC
	drow.bytes.clear();
	drow.com.clear();
	unsigned char bt;
	unsigned char fl;
	unsigned char* ptr;
	char buf[256];
	int clen = 0;
	drow.type = getBrk(comp, adr) & 0xf0;
	switch(drow.type) {
		case DBG_VIEW_CODE:			// opcode
			mn = cpuDisasm(comp->cpu, adr, buf, &rdbyte, comp);
			clen = mn.len;
			if (drow.ispc) {
				drow.cond = mn.cond & mn.met;
				drow.mem = mn.mem;
				drow.mop = mn.mop;
			}
			break;
		case DBG_VIEW_BYTE:			// db byte
			clen = 1;
			sprintf(buf, "db #%.2X", memRd(comp->mem, adr));
			break;
		case DBG_VIEW_TEXT:
			clen = 0;
			strcpy(buf, "DB \"");
			fl = getBrk(comp, adr);
			bt = memRd(comp->mem, adr);
			while (((fl & 0xc0) == DBG_VIEW_TEXT) && (bt > 31) && (bt < 128) && (clen < 250)) {
				buf[clen + 4] = bt;
				clen++;
				bt = memRd(comp->mem, (adr + clen) & 0xffff);
				fl = getBrk(comp, (adr + clen) & 0xffff);
			}
			if (clen == 0) {
				ptr = getBrkPtr(comp, adr);
				*ptr &= 0x0f;
				*ptr |= DBG_VIEW_BYTE;
				drow.type = DBG_VIEW_BYTE;
				clen = 1;
				sprintf(buf, "db #%.2X", memRd(comp->mem, adr));
			} else {
				buf[clen + 4] = '"';
				buf[clen + 5] = 0x00;
			}
			break;
		case DBG_VIEW_WORD:			// dw word
		case DBG_VIEW_ADDR:			// dw adr (label using)
			clen = memRd(comp->mem, adr) | (memRd(comp->mem, adr+1) << 8);
			sprintf(buf, "dw #%.4X", clen);
			clen = 2;
			break;
	}
	drow.com = QString(buf);
	if (drow.type != DBG_VIEW_TEXT)
		drow.com = drow.com.toUpper();
	for (int i = 0; i < clen; i++) {
		drow.bytes.append(rdbyte(adr, (void*)comp));
		adr++;
	}
/*
	if (drow.ispc && (drow.type == DBG_VIEW_CODE)) {		// !!! TODO:Z80 only : move it to cpuDisasm
		if (clen > 2) {
			bt = drow.bytes.at(clen - 3);		// jp, call
			if (((bt & 0xc7) == 0xc2) || ((bt & 0xc7) == 0xc4)) {
				drow.cond = checkCond(comp, (bt & 0x38) >> 3);
			}
		} else if (clen > 1) {
			bt = drow.bytes.at(clen - 2);
			if (bt == 0x10) {			// djnz
				drow.cond = (comp->cpu->b == 1) ? 1 : 0;
			} else if ((bt & 0xe7) == 0x20) {	// jr
				drow.cond = checkCond(comp, (bt & 0x18) >> 3);
			}
		} else {
			bt = drow.bytes.at(clen - 1);
			if ((bt & 0xc7) == 0xc0) {		// ret
				drow.cond = checkCond(comp, (bt & 0x38) >> 3);
			}
		}
		if (drow.com.endsWith("(HL)") && !drow.com.startsWith("JP")) {
			drow.mem = 1;
			drow.mop = memRd(comp->mem, comp->cpu->hl);
		} else if (drow.com.size() > 6) {
			bt = drow.bytes.at(clen - 1);
			if (clen > 2) {
				if ((unsigned char)drow.bytes.at(clen - 3) == 0xcb)
					bt = drow.bytes.at(clen - 2);
			}
			if (drow.com.indexOf("(IX") == (drow.com.size() - 7)) {
				drow.mem = 1;
				drow.mop = memRd(comp->mem, 0xffff & (comp->cpu->ix + (signed char)bt));
			} else if (drow.com.indexOf("(IY") == (drow.com.size() - 7)) {
				drow.mem = 1;
				drow.mop = memRd(comp->mem, 0xffff & (comp->cpu->iy + (signed char)bt));
			}
		}
	}
*/
	return drow;
}

const QColor colPC(32,200,32);		// pc
const QColor colBRK(200,128,128);	// breakpoint
const QColor colBG0(255,255,255);	// background
const QColor colBG1(230,230,230);	// background alternative
const QColor colSEL(128,255,128);	// background selected

void DebugWin::placeLabel(DasmRow& drow) {
	int pos;
	QString lab;
	if ((drow.type == DBG_VIEW_ADDR) || (drow.type == DBG_VIEW_CODE)) {
		pos = drow.com.indexOf(QRegExp("#[0-9A-F]{4,4}"));			// find addr position (#XXXX)
		if (pos > 0) {
			lab = findLabel(drow.com.mid(pos+1,4).toInt(NULL,16), -1, -1);	// find label for that addr
			if (!lab.isEmpty()) {
				drow.com.replace(pos, 5, lab);				// replace +1 char (#XXXX)
			}
		}
	}
}

int DebugWin::fillDisasm() {
	block = 1;
	unsigned char res = 0;
	unsigned short adr = disasmAdr;
	DasmRow drow;
	QColor bgcol,acol;
	QString lab;
	QFont fnt = ui.dasmTable->font();
	xAdr xadr;
	int pos;
	fnt.setBold(true);
	for (int i = 0; i < ui.dasmTable->rowCount(); i++) {
		drow = getDisasm(comp, adr);
		res |= drow.ispc;
		ui.dasmTable->item(i, 2)->setData(Qt::UserRole, drow.com);
		placeLabel(drow);
		if (drow.ispc) {
			bgcol = colPC;
		} else if ((drow.adr >= ui.dasmTable->blockStart) && (drow.adr <= ui.dasmTable->blockEnd)) {
			bgcol = colSEL;
		} else {
			bgcol = (i & 1) ? colBG0 : colBG1;
		}
		acol = (getBrk(comp, drow.adr) & MEM_BRK_ANY) ? colBRK : bgcol;
		ui.dasmTable->item(i, 0)->setData(Qt::UserRole, drow.adr);
		ui.dasmTable->item(i, 0)->setBackgroundColor(bgcol);
		ui.dasmTable->item(i, 1)->setBackgroundColor(bgcol);
		ui.dasmTable->item(i, 2)->setBackgroundColor(bgcol);
		ui.dasmTable->item(i, 3)->setBackgroundColor(acol);
		xadr = memGetXAdr(comp->mem, drow.adr);
		lab = findLabel(xadr.adr, xadr.type, xadr.bank);
		if (lab.isEmpty()) {
			fnt.setBold(false);
			ui.dasmTable->item(i, 0)->setText(gethexword(drow.adr));
		} else {
			fnt.setBold(true);
			ui.dasmTable->item(i, 0)->setText(lab);
		}
		ui.dasmTable->item(i, 0)->setFont(fnt);
		QString str;
		foreach(pos, drow.bytes)
			str.append(gethexbyte(pos));
		ui.dasmTable->item(i, 1)->setText(str);
		ui.dasmTable->item(i, 2)->setText(drow.com);
		if (drow.cond) {
			ui.dasmTable->item(i,3)->setText("+");
		} else if (drow.mem) {
			ui.dasmTable->item(i,3)->setText(gethexbyte(drow.mop));
		} else {
			ui.dasmTable->item(i,3)->setText("");
		}
	}
	fillStack();
	block = 0;
	return res;
}

unsigned short DebugWin::getPrevAdr(unsigned short adr) {
	unsigned short tadr;
	for (int i = 16; i > 0; i--) {
		tadr = adr - i;
		getDisasm(comp, tadr);			// shift tadr to next op
		if (tadr == adr) return (adr - i);
	}
	return (adr - 1);
}

void DebugWin::dasmEdited(int row, int col) {
	if (block) return;
	int adr = getAdr(); //ui.dasmTable->item(row, 0)->data(Qt::UserRole).toInt();
	char buf[8];
	int len;
	int idx;
	unsigned char cbyte;
	unsigned char* ptr;
	bool flag;
	QString str;
	xAdr xadr;
	switch (col) {
		case 0:
			str = ui.dasmTable->item(row, 0)->text();
			if (str.isEmpty()) {
				str = findLabel(adr,-1,-1);
				if (!str.isEmpty()) {
					labels.remove(str);
				}
			} else {
				idx = str.toInt(&flag, 16);
				if (flag) {
					adr = idx;
				} else if (labels.contains(str)) {
					adr = labels[str].adr;
				} else {
					xadr = memGetXAdr(comp->mem, adr);
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
			str = ui.dasmTable->item(row, col)->text();
			while (!str.isEmpty()) {
				cbyte = str.left(2).toInt(NULL,16);
				memWr(comp->mem, adr, cbyte);
				adr++;
				str.remove(0,2);
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
}

void DebugWin::saveDasm() {
	QString path = QFileDialog::getSaveFileName(this, "Save disasm");
	if (path.isEmpty()) return;
	QFile file(path);
	DasmRow drow;
	xAdr xadr;
	QString label;
	if (file.open(QFile::WriteOnly)) {
		QTextStream strm(&file);
		unsigned short adr = (ui.dasmTable->blockStart < 0) ? 0 : (ui.dasmTable->blockStart & 0xffff);
		unsigned short end = (ui.dasmTable->blockEnd < 0) ? 0 : (ui.dasmTable->blockEnd & 0xffff);
		int work = 1;
		while ((adr <= end) && work) {
			drow = getDisasm(comp, adr);
			if (adr < drow.adr) work = 0;		// address overfill (FFFF+)
			placeLabel(drow);
			xadr = memGetXAdr(comp->mem, drow.adr);
			label = findLabel(xadr.adr, xadr.type, xadr.bank);
			if (label.isEmpty()) {
				strm << QString("\t%0\n").arg(drow.com);
			} else {
				strm << QString("%0:\n\t%1\n").arg(label).arg(drow.com);
			}
		}
		file.close();
	} else {
		shitHappens("Can't write to file");
	}
}

// memory dump

QString getDumpString(Memory* mem, unsigned short adr) {
	QString res;
	unsigned char bte;
	for (int i = 0; i < 8; i++) {
		bte = memRd(mem, adr);
		adr = (adr + 1) & 0xffff;
		if ((bte < 32) || (bte > 127)) {
			res.append(".");
		} else {
			res.append(QChar(bte));
		}
	}
	return res;
}

void DebugWin::fillDump() {
	block = 1;
	unsigned short adr = dumpAdr;
	int row,col;
	QColor bgcol, ccol;
	for (row = 0; row < ui.dumpTable->rowCount(); row++) {
		ui.dumpTable->item(row,0)->setText(gethexword(adr));
		ui.dumpTable->item(row,9)->setText(getDumpString(comp->mem, adr));
		bgcol = (row & 1) ? QColor(255,255,255) : QColor(230,230,230);
		ui.dumpTable->item(row, 0)->setBackground(bgcol);
		ui.dumpTable->item(row, 9)->setBackground(bgcol);
		for (col = 1; col < 9; col++) {
			ccol = (*getBrkPtr(comp, adr) & MEM_BRK_ANY) ? QColor(200,64,64) : bgcol;
			ui.dumpTable->item(row,col)->setBackgroundColor(ccol);
			ui.dumpTable->item(row,col)->setText(gethexbyte(memRd(comp->mem, adr)));
			adr++;
		}
	}
	fillStack();
	block = 0;
}

void DebugWin::dumpEdited(int row, int col) {
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
	if (ui.dumpTable->hasFocus()) {
		adr = (dumpAdr + (ui.dumpTable->currentColumn() - 1) + ui.dumpTable->currentRow() * 8) & 0xffff;
	} else {
		adr = ui.dasmTable->item(ui.dasmTable->currentRow(),0)->data(Qt::UserRole).toInt();
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
	unsigned short eadr;
	unsigned char bt;
	unsigned char* ptr;
	if ((adr < ui.dasmTable->blockStart) || (adr > ui.dasmTable->blockEnd)) {	// pointer outside block : process 1 cell
		bgn = adr;
		end = adr;
	} else {									// pointer inside block : process all block
		bgn = ui.dasmTable->blockStart;
		end = ui.dasmTable->blockEnd;
	}
	eadr = end & 0xffff; getDisasm(comp, eadr); end = (eadr-1) & 0xffff;		// move end to last byte of block
	adr = bgn;
	while (adr <= end) {
		ptr = getBrkPtr(comp, adr);
		if (data & MEM_BRK_ANY) {
			*ptr &= 0xf0;
			if (ui.actFetch->isChecked()) *ptr |= MEM_BRK_FETCH;
			if (ui.actRead->isChecked()) *ptr |= MEM_BRK_RD;
			if (ui.actWrite->isChecked()) *ptr |= MEM_BRK_WR;
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
	fillBrkTable();
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

// memory viewer

MemViewer::MemViewer(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	vis = 0;

	connect(ui.sbAddr, SIGNAL(valueChanged(int)), this, SLOT(fillImage()));
	connect(ui.sbWidth, SIGNAL(valueChanged(int)), this, SLOT(fillImage()));
	connect(ui.sbHeight, SIGNAL(valueChanged(int)), this, SLOT(fillImage()));
	connect(ui.sbPage, SIGNAL(valueChanged(int)), this, SLOT(fillImage()));

	connect(ui.adrHex, SIGNAL(textChanged(QString)), this, SLOT(hexChanged()));
	connect(ui.sbAddr, SIGNAL(valueChanged(int)), this, SLOT(adrChanged(int)));
	connect(ui.scrollbar, SIGNAL(valueChanged(int)), this, SLOT(memScroll(int)));

	connect(ui.tbSave, SIGNAL(released()), this, SLOT(saveSprite()));
}

void MemViewer::wheelEvent(QWheelEvent* ev) {
	int adr = ui.sbAddr->value();
	if (ev->delta() < 0) {
		adr += (ui.sbWidth->value() << 3);
	} else {
		adr -= (ui.sbWidth->value() << 3);
	}
	ui.sbAddr->setValue(adr & 0xffff);
}

void MemViewer::saveSprite() {
	int adr = ui.sbAddr->value();
	int siz = ui.sbWidth->value() * ui.sbHeight->value() * 8;
	QByteArray spr;
	for(int i = 0; i < siz; i++) {
		spr.append(rdMem(adr));
		adr++;
	}
	QString path = QFileDialog::getSaveFileName(this, "Save sprite");
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(spr);
		file.close();
	} else {
		shitHappens("Can't write a file");
	}
}

unsigned char MemViewer::rdMem(int adr) {
	adr &= 0xffff;
	unsigned char res;
	int page = ui.sbPage->value();
	if (adr < 0xc000) {
		res = memRd(mem, adr);
	} else {
		res = mem->ramData[(page << 14) | (adr & 0x3fff)];
	}
	return res;
}

void MemViewer::fillImage() {
	QImage img(256,256, QImage::Format_RGB888);
	img.fill(qRgb(64,64,64));
	int adr = ui.sbAddr->value();
	int high = ui.sbHeight->value() << 3;
	unsigned char byt;
	int bit;
	int row,col;
	for (row = 0; row < high; row++) {
		for (col = 0; col < ui.sbWidth->value(); col++) {
			byt = rdMem(adr);
			adr++;
			for (bit = 0; bit < 8; bit++) {
				img.setPixel((col << 3) | bit, row, (byt & 0x80) ? qRgb(255,255,255) : qRgb(0,0,0));
				byt <<= 1;
			}
		}
	}
	QPixmap pxm = QPixmap::fromImage(img.scaled(512,512));
	ui.view->setPixmap(pxm);
	int pg = ui.sbWidth->value() << 3;
	ui.scrollbar->setPageStep(pg);
	ui.scrollbar->setSingleStep(pg);
}

void MemViewer::adrChanged(int adr) {
	ui.scrollbar->setValue(adr);
	QString hw = gethexword(adr);
	if (ui.adrHex->text() != hw)
		ui.adrHex->setText(hw);
}

void MemViewer::hexChanged() {
	ui.sbAddr->setValue(ui.adrHex->text().toInt(NULL, 16));
}

void MemViewer::memScroll(int adr) {
	adr = adr - ((adr - ui.sbAddr->value()) % ui.sbWidth->value());
	ui.sbAddr->setValue(adr);
}

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
	vidGetScreen(comp->vid, scrImg.bits(), ui.sbScrBank->value(), ui.cbScrAlt->isChecked() ? 0x2000 : 0, ui.cbScrAtr->isChecked() ? 0 : 1);
	ui.scrLabel->setPixmap(QPixmap::fromImage(scrImg));
}

// xtablewidget

xTableWidget::xTableWidget(QWidget* par):QTableWidget(par) {
	setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
	blockStart = -1;
	blockEnd = -1;
}

void xTableWidget::keyPressEvent(QKeyEvent *ev) {
	if (ev->modifiers() & Qt::ControlModifier) {
		ev->ignore();
	} else {
		switch (ev->key()) {
			case Qt::Key_Home:
			case Qt::Key_End:
			case Qt::Key_F2:
				ev->ignore();
				break;
			default:
				QTableWidget::keyPressEvent(ev);
		}
	}
}

void xTableWidget::mousePressEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	int adr = item(row,0)->data(Qt::UserRole).toInt();
	switch (ev->button()) {
		case Qt::MiddleButton:
			blockStart = -1;
			blockEnd = -1;
			emit rqRefill();
			ev->ignore();
			break;
		case Qt::LeftButton:
			if (ev->modifiers() & Qt::ControlModifier) {
				blockStart = adr;
				if (blockEnd < blockStart) blockEnd = blockStart;
				emit rqRefill();
				ev->ignore();
			} else if (ev->modifiers() & Qt::ShiftModifier) {
				blockEnd = adr;
				if (blockStart > blockEnd) blockStart = blockEnd;
				if (blockStart < 0) blockStart = 0;
				emit rqRefill();
				ev->ignore();
			} else {
				markAdr = adr;
				QTableWidget::mousePressEvent(ev);
			}
			break;
		default:
			QTableWidget::mousePressEvent(ev);
			break;
	}
}

void xTableWidget::mouseReleaseEvent(QMouseEvent* ev) {
	if (ev->button() == Qt::LeftButton) {
		markAdr = -1;
	}
}

void xTableWidget::mouseMoveEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	int adr = item(row,0)->data(Qt::UserRole).toInt();
	if ((ev->modifiers() == Qt::NoModifier) && (ev->buttons() & Qt::LeftButton) && (adr != blockStart) && (adr != blockEnd) && (markAdr >= 0)) {
		if (adr < blockStart) {
			blockStart = adr;
			blockEnd = markAdr;
		} else {
			blockStart = markAdr;
			blockEnd = adr;
		}
		emit rqRefill();
	}
	QTableWidget::mouseMoveEvent(ev);
}

// breakpoints

struct bPoint {
	unsigned rom:1;
	int bank;
	int adr;
	int flags;
};

enum {
	roleRom = Qt::UserRole,
	roleBank,
	roleAdr
};

void DebugWin::fillBrkTable() {
	QList<bPoint> list;
	bPoint bp;
	int adr;
	for (adr = 0; adr < 0x400000; adr++) {
		if (comp->brkRamMap[adr] & MEM_BRK_ANY) {
			bp.rom = 0;
			bp.bank = (adr >> 14);
			bp.adr = adr & 0x3fff;
			bp.flags = comp->brkRamMap[adr] & 0xff;
			list.append(bp);
		}
		if (adr < 0x80000) {
			if (comp->brkRomMap[adr] & MEM_BRK_ANY) {
				bp.rom = 1;
				bp.bank = (adr >> 14);
				bp.adr = adr & 0x3fff;
				bp.flags = comp->brkRomMap[adr] & 0xff;
				list.append(bp);
			}
		}
	}
	ui.bpList->clear();
	ui.bpList->setColumnCount(4);
	ui.bpList->setRowCount(list.size());
	ui.bpList->setColumnWidth(0,40);
	ui.bpList->setColumnWidth(1,40);
	ui.bpList->setColumnWidth(2,40);
	ui.bpList->setHorizontalHeaderLabels(QStringList() << "Fe" << "Rd" << "Wr" << "Addr");
	QTableWidgetItem* itm;
	adr = 0;
	foreach(bp, list) {
		itm = new QTableWidgetItem();
		itm->setData(roleRom, bp.rom);
		itm->setData(roleBank, bp.bank);
		itm->setData(roleAdr, bp.adr);
		if (bp.flags & MEM_BRK_FETCH) itm->setIcon(QIcon(":/images/checkbox.png"));
		ui.bpList->setItem(adr, 0, itm);
		itm = new QTableWidgetItem();
		if (bp.flags & MEM_BRK_RD) itm->setIcon(QIcon(":/images/checkbox.png"));
		ui.bpList->setItem(adr, 1, itm);
		itm = new QTableWidgetItem();
		if (bp.flags & MEM_BRK_WR) itm->setIcon(QIcon(":/images/checkbox.png"));
		ui.bpList->setItem(adr, 2, itm);
		itm = new QTableWidgetItem(tr(bp.rom ? "ROM:%0:%1" : "RAM:%0:%1").arg(gethexbyte(bp.bank)).arg(gethexword(bp.adr)));
		ui.bpList->setItem(adr, 3, itm);
		adr++;
	}
}

void DebugWin::goToBrk(QModelIndex idx) {
	QTableWidgetItem* itm = ui.bpList->item(idx.row(),0);
	int rom = itm->data(roleRom).toInt();
	int adr = itm->data(roleAdr).toInt();
	int bnk = itm->data(roleBank).toInt();
	int madr = -1;
	MemPage* pg;
	for (int i = 0; i < 4; i++) {
		pg = &comp->mem->map[i];
		if (pg->num == bnk) {
			if ((pg->type == MEM_RAM) && !rom) {
				madr = (i << 14) | adr;
			} else if ((pg->type == MEM_ROM) && rom) {
				madr = (i << 14) | adr;
			}
		}
	}
	if (madr > 0) {
		disasmAdr = madr;
		fillDisasm();
	}
}
