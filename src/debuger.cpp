#include "xcore/xcore.h"

#include <QIcon>
#include <QDebug>
#include <QFileDialog>
#include <QTemporaryFile>

#include "debuger.h"
#include "emulwin.h"
#include "filer.h"
#include "xgui/xgui.h"
#include "filer.h"

#ifndef SELFZ80
	#include "z80ex_dasm.h"
#endif


void DebugWin::start(ZXComp* c) {
	comp = c;
	if (!fillAll()) {
		disasmAdr = GETPC(comp->cpu);
		fillAll();
	}
	move(winPos);
	show();
	ui.dasmTable->setFocus();
	comp->vid->debug = 1;
	comp->debug = 1;
}

void DebugWin::stop() {
	comp->debug = 0;
	comp->vid->debug = 0;
	tCount = comp->tickCount;
//	zxExec(comp);		// to prevent immediatelly fetch break, if PC is on breakpoint
	hide();
	active = false;
	winPos = pos();
	emit closed();
}

void DebugWin::reject() {stop();}

xItemDelegate::xItemDelegate(int t) {type = t;}

QWidget* xItemDelegate::createEditor(QWidget* par, const QStyleOptionViewItem&, const QModelIndex&) const {
	QLineEdit* edt = new QLineEdit(par);
	switch (type) {
		case XTYPE_ADR: edt->setInputMask("Hhhh"); break;
		case XTYPE_DUMP: edt->setInputMask("Hhhhhhhhhh"); break;
		case XTYPE_BYTE: edt->setInputMask("Hh"); break;
	}
	return edt;
}

DebugWin::DebugWin(QWidget* par):QDialog(par) {
	int col,row;
	ui.setupUi(this);
// disasm table
	for (row = 0; row < ui.dasmTable->rowCount(); row++) {
		for (col = 0; col < ui.dasmTable->columnCount(); col++) {
			ui.dasmTable->setItem(row, col, new QTableWidgetItem);
		}
	}
	ui.dasmTable->setColumnWidth(0,75);
	ui.dasmTable->setColumnWidth(1,75);
	ui.dasmTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_ADR));
	ui.dasmTable->setItemDelegateForColumn(1, new xItemDelegate(XTYPE_DUMP));
	connect(ui.dasmTable,SIGNAL(cellChanged(int,int)),this,SLOT(dasmEdited(int,int)));
	connect(ui.dasmTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
// dump table
	for (row = 0; row < ui.dumpTable->rowCount(); row++) {
		for (col = 0; col < ui.dumpTable->columnCount(); col++) {
			ui.dumpTable->setItem(row, col, new QTableWidgetItem);
		}
	}
	ui.dumpTable->setColumnWidth(0,50);
	ui.dumpTable->setItemDelegate(new xItemDelegate(XTYPE_BYTE));
	ui.dumpTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_ADR));
	connect(ui.dumpTable,SIGNAL(cellChanged(int,int)),this,SLOT(dumpEdited(int,int)));
	connect(ui.dumpTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(putBreakPoint()));
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

	setFixedSize(size());
	active = false;
	block = false;
	disasmAdr = 0;
	dumpAdr = 0;
	tCount = 0;
	trace = 0;
	showLabels = 1;

	dumpwin = new QDialog();
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

	openDumpDialog = new QDialog();
	oui.setupUi(openDumpDialog);
	connect(oui.tbFile,SIGNAL(clicked()),this,SLOT(chDumpFile()));
	connect(oui.leStart,SIGNAL(textChanged(QString)),this,SLOT(dmpStartOpen()));
	connect(oui.butOk,SIGNAL(clicked()), this, SLOT(loadDump()));

	bpMenu = new QMenu(this);
	bpMenu->addAction(ui.actFetch);
	bpMenu->addAction(ui.actRead);
	bpMenu->addAction(ui.actWrite);
	connect(bpMenu,SIGNAL(triggered(QAction*)),this,SLOT(chaBreakPoint()));
}

void DebugWin::wheelEvent(QWheelEvent* ev) {
	if (ev->delta() < 0) {
		scrollDown();
	} else if (ev->delta() > 0) {
		scrollUp();
	}
}

void DebugWin::scrollUp() {
	if (ui.dumpTable->hasFocus()) {
		dumpAdr = (dumpAdr - ui.dumpTable->columnCount() + 1) & 0xffff;
		fillDump();
	} else if (ui.dasmTable->hasFocus()) {
		disasmAdr = getPrevAdr(disasmAdr);
		fillDisasm();
	}
}

void DebugWin::scrollDown() {
	if (ui.dumpTable->hasFocus()) {
		dumpAdr = (dumpAdr + ui.dumpTable->columnCount() - 1) & 0xffff;
		fillDump();
	} else if (ui.dasmTable->hasFocus()) {
		disasmAdr = ui.dasmTable->item(1,0)->data(Qt::UserRole).toInt();
		fillDisasm();
	}
}

void DebugWin::doStep() {
	tCount = comp->tickCount;
	zxExec(comp);
	if (!fillAll()) {
		disasmAdr = GETPC(comp->cpu);
		fillDisasm();
	}
	if (trace) QTimer::singleShot(10,this,SLOT(doStep()));
}

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	if (trace) {
		trace = 0;
		return;
	}
	int i;
	Z80EX_WORD pc = GETPC(comp->cpu);
	unsigned char* ptr;
	int offset = (ui.dumpTable->columnCount() - 1) * (ui.dumpTable->rowCount() - 1);
	switch(ev->modifiers()) {
		case Qt::ControlModifier:
			switch(ev->key()) {
				case Qt::Key_S:
					doSaveDump();
					break;
				case Qt::Key_O:
					doOpenDump();
					break;
				case Qt::Key_Z:
					if (!ui.dasmTable->hasFocus()) break;
					SETPC(comp->cpu, ui.dasmTable->item(ui.dasmTable->currentRow(), 0)->text().toInt(NULL,16));
					fillZ80();
					fillDisasm();
					break;
				case Qt::Key_T:
					trace = 1;
					doStep();
					break;
				case Qt::Key_L:
					showLabels ^= 1;
					fillDisasm();
					break;
			}
			break;
		default:
			switch(ev->key()) {
				case Qt::Key_Escape:
					if (!ev->isAutoRepeat()) stop();
					break;
				case Qt::Key_Return:
					putBreakPoint();
					break;
				case Qt::Key_Home:
					disasmAdr = pc;
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
					disasmAdr = GETPC(comp->cpu);
					fillAll();
					break;
				case Qt::Key_F7:
					doStep();
					break;
				case Qt::Key_F8:
					if (!ui.dasmTable->hasFocus()) break;
					i = memRd(comp->mem, pc);
					if (((i & 0xc7) == 0xc4) || (i == 0xcd)) {		// call
						ptr = memGetFptr(comp->mem, pc + 3);
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else if (((i & 0xc7) == 0xc7) || (i == 0x76)) {	// rst, halt
						ptr = memGetFptr(comp->mem, pc + 1);
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else if (i == 0x10) {
						ptr = memGetFptr(comp->mem, pc + 2);		// djnz
						*ptr ^= MEM_BRK_TFETCH;
						stop();
					} else if (i == 0xed) {
						i = memRd(comp->mem, pc + 1);
						if ((i & 0xf4) == 0xb0) {			// block cmds
							ptr = memGetFptr(comp->mem, pc + 2);
							*ptr ^= MEM_BRK_TFETCH;
							stop();
						} else {
							doStep();
						}
					} else {
						doStep();
					}
					break;
				case Qt::Key_F12:
					zxReset(comp, RES_DEFAULT);
					if (!fillAll()) {
						disasmAdr = GETPC(comp->cpu);
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

bool DebugWin::fillAll() {
	ui.labTcount->setText(QString::number(comp->tickCount - tCount));
	fillZ80();
	fillMem();
	fillDump();
	fillFDC();
	ui.rzxTab->setEnabled(comp->rzxPlay);
	if (comp->rzxPlay) fillRZX();
	ui.labRX->setNum(comp->vid->x);
	ui.labRY->setNum(comp->vid->y);
	return fillDisasm();
}

// rzx

void dbgSetRzxIO(QLabel* lab, ZXComp* comp, int pos) {
	if (pos < comp->rzx.data[comp->rzx.frame].frmSize) {
		lab->setText(gethexbyte(comp->rzx.data[comp->rzx.frame].frmData[pos]));
	} else {
		lab->setText("--");
	}
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
	ui.flpDataL->setText(comp->dif->fdc->flp->insert ? gethexbyte(flpRd(comp->dif->fdc->flp)): "--");
	ui.flpMotL->setText(comp->dif->fdc->flp->motor ? "1" : "0");
}

// z80 regs section

const char flags[] = "SZ5H3PNC";

QString flagString(int af) {
	QString flag;
	int i = 0;
	while (i < 8) {
		flag.append((af & 0x80) ? QString(flags[i]) : QString("."));
		af <<= 1;
		i++;
	}
	return flag;
}

void DebugWin::fillFlags() {
	Z80EX_WORD af = GETAF(comp->cpu);
	ui.cbFS->setChecked(af & 0x80);
	ui.cbFZ->setChecked(af & 0x40);
	ui.cbF5->setChecked(af & 0x20);
	ui.cbFH->setChecked(af & 0x10);
	ui.cbF3->setChecked(af & 0x08);
	ui.cbFP->setChecked(af & 0x04);
	ui.cbFN->setChecked(af & 0x02);
	ui.cbFC->setChecked(af & 0x01);
}

void DebugWin::fillZ80() {
	block = true;
	Z80EX_WORD af = GETAF(comp->cpu);
	ui.editAF->setText(gethexword(af));
	ui.editBC->setText(gethexword(GETBC(comp->cpu)));
	ui.editDE->setText(gethexword(GETDE(comp->cpu)));
	ui.editHL->setText(gethexword(GETHL(comp->cpu)));
	ui.editAFa->setText(gethexword(GETAF_(comp->cpu)));
	ui.editBCa->setText(gethexword(GETBC_(comp->cpu)));
	ui.editDEa->setText(gethexword(GETDE_(comp->cpu)));
	ui.editHLa->setText(gethexword(GETHL_(comp->cpu)));
	ui.editPC->setText(gethexword(GETPC(comp->cpu)));
	ui.editSP->setText(gethexword(GETSP(comp->cpu)));
	ui.editIX->setText(gethexword(GETIX(comp->cpu)));
	ui.editIY->setText(gethexword(GETIY(comp->cpu)));
	ui.editIR->setText(gethexword(GETIR(comp->cpu)));
	ui.boxIM->setValue(GETIM(comp->cpu));
	ui.flagIFF1->setChecked(GETIFF1(comp->cpu));
	ui.flagIFF2->setChecked(GETIFF1(comp->cpu));
	//ui.labFlag->setText(flagString(af));
	fillFlags();
	block = false;
	fillStack();
}

void DebugWin::setFlags() {
	if (block) return;
	Z80EX_WORD af = GETAF(comp->cpu);
	af &= 0xff00;
	if (ui.cbFS->isChecked()) af |= 0x80;
	if (ui.cbFZ->isChecked()) af |= 0x40;
	if (ui.cbF5->isChecked()) af |= 0x20;
	if (ui.cbFH->isChecked()) af |= 0x10;
	if (ui.cbF3->isChecked()) af |= 0x08;
	if (ui.cbFP->isChecked()) af |= 0x04;
	if (ui.cbFN->isChecked()) af |= 0x02;
	if (ui.cbFC->isChecked()) af |= 0x01;
	SETAF(comp->cpu, af);
	ui.editAF->setText(gethexword(af));
}

void DebugWin::setZ80() {
	if (block) return;
	int af = ui.editAF->text().toInt(NULL,16);
	SETAF(comp->cpu, af);
	SETBC(comp->cpu, ui.editBC->text().toInt(NULL,16));
	SETDE(comp->cpu, ui.editDE->text().toInt(NULL,16));
	SETHL(comp->cpu, ui.editHL->text().toInt(NULL,16));
	SETAF_(comp->cpu, ui.editAFa->text().toInt(NULL,16));
	SETBC_(comp->cpu, ui.editBCa->text().toInt(NULL,16));
	SETDE_(comp->cpu, ui.editDEa->text().toInt(NULL,16));
	SETHL_(comp->cpu, ui.editHLa->text().toInt(NULL,16));
	SETPC(comp->cpu, ui.editPC->text().toInt(NULL,16));
	SETSP(comp->cpu, ui.editSP->text().toInt(NULL,16));
	SETIX(comp->cpu, ui.editIX->text().toInt(NULL,16));
	SETIY(comp->cpu, ui.editIY->text().toInt(NULL,16));
	SETIR(comp->cpu, ui.editIR->text().toInt(NULL,16));
	SETIM(comp->cpu, ui.boxIM->value());
	SETIFF1(comp->cpu, ui.flagIFF1->isChecked());
	SETIFF2(comp->cpu, ui.flagIFF2->isChecked());
	fillFlags();
	fillStack();
}

// memory map section

QString getPageName(MemPage* pg) {
	QString res = (pg->type == MEM_RAM) ? "RAM-" : "ROM-";
	res.append(QString::number(pg->num));
	return res;
}

void DebugWin::fillMem() {
	ui.labPG0->setText(getPageName(comp->mem->pt[0]));
	ui.labPG1->setText(getPageName(comp->mem->pt[1]));
	ui.labPG2->setText(getPageName(comp->mem->pt[2]));
	ui.labPG3->setText(getPageName(comp->mem->pt[3]));
}

// disasm table

Z80EX_BYTE rdbyte(Z80EX_WORD adr, void* ptr) {
	return memRd(((ZXComp*)ptr)->mem,adr);
}

DasmRow getDisasm(ZXComp* comp, Z80EX_WORD& adr) {
	DasmRow drow;
	drow.adr = adr;
	drow.bytes.clear();
	drow.com.clear();
	char buf[256];
	int clen;
#ifdef SELFZ80
	clen = cpuDisasm(adr,buf,&rdbyte,comp);
#else
	int t1,t2;
	clen = z80ex_dasm(buf,256,0,&t1,&t2,&rdbyte,adr,comp);
#endif
	drow.com = QString(buf).toUpper();
	while (clen > 0) {
		drow.bytes.append(gethexbyte(memRd(comp->mem,adr)));
		clen--;
		adr++;
	}
	return drow;
}

xLabel* DebugWin::findLabel(int adr) {
	if (!showLabels) return NULL;
//	int bnk = comp->mem->pt[adr >> 14]->num;
//	adr &= 0x3fff;
	for (int i = 0; i < labels.size(); i++) {
		if (/*(labels[i].bank == bnk) && */(labels[i].adr == adr)) return &labels[i];
	}
	return NULL;
}

bool DebugWin::fillDisasm() {
	block = true;
	Z80EX_WORD adr = disasmAdr;
	Z80EX_WORD pc = GETPC(comp->cpu);
	DasmRow drow;
	QColor bgcol,acol;
	xLabel* lab = NULL;
	QFont fnt = ui.dasmTable->font();
	int pos;
	fnt.setBold(true);
	bool res = false;
	for (int i = 0; i < ui.dasmTable->rowCount(); i++) {
		bgcol = (adr == pc) ? QColor(32,200,32) : QColor(255,255,255);
		acol = (*memGetFptr(comp->mem, adr) & MEM_BRK_ANY) ? QColor(200,64,64) : bgcol;
		if (adr == pc) res = true;
		ui.dasmTable->item(i, 0)->setData(Qt::UserRole, adr);
		ui.dasmTable->item(i, 0)->setBackgroundColor(acol);
		ui.dasmTable->item(i, 1)->setBackgroundColor(bgcol);
		ui.dasmTable->item(i, 2)->setBackgroundColor(bgcol);
		lab = findLabel(adr);
		if (lab) {
			fnt.setBold(true);
			ui.dasmTable->item(i, 0)->setText(lab->name);
		} else {
			fnt.setBold(false);
			ui.dasmTable->item(i, 0)->setText(gethexword(adr));
		}
		ui.dasmTable->item(i, 0)->setFont(fnt);
		drow = getDisasm(comp, adr);
		pos = drow.com.indexOf(QRegExp("[0-9A-F]{4,4}"));
		if (pos > -1) {
			lab = findLabel(drow.com.mid(pos,4).toInt(NULL,16));
			if (lab) {
				drow.com.replace(pos, 4, lab->name);
			}
		}
		ui.dasmTable->item(i, 1)->setText(drow.bytes);
		ui.dasmTable->item(i, 2)->setText(drow.com);
	}
	fillStack();
	block = false;
	return res;
}

Z80EX_WORD DebugWin::getPrevAdr(Z80EX_WORD adr) {
	Z80EX_WORD tadr;
	for (int i = 16; i > 0; i--) {
		tadr = adr - i;
		getDisasm(comp, tadr);			// shift tadr to next op
		if (tadr == adr) return (adr - i);
	}
	return (adr - 1);
}

void DebugWin::dasmEdited(int row, int col) {
	if (block) return;
	int adr = ui.dasmTable->item(row, 0)->text().toInt(NULL,16);
	if (col == 0) {
		while (row > 0) {
			adr = getPrevAdr(adr);
			row--;
		}
		disasmAdr = adr;
	} else if (col == 1) {
		QString str = ui.dasmTable->item(row, col)->text();
		Z80EX_BYTE cbyte;
		while (!str.isEmpty()) {
			cbyte = str.left(2).toInt(NULL,16);
			memWr(comp->mem, adr, cbyte);
			adr++;
			str.remove(0,2);
		}
		fillDump();
	}
	fillDisasm();
}

// memory dump

void DebugWin::fillDump() {
	block = true;
	Z80EX_WORD adr = dumpAdr;
	int row,col;
	QColor bgcol;
	for (row = 0; row < ui.dumpTable->rowCount(); row++) {
		ui.dumpTable->item(row,0)->setText(gethexword(adr));
		for (col = 1; col < ui.dumpTable->columnCount(); col++) {
			bgcol = (*memGetFptr(comp->mem, adr) & MEM_BRK_ANY) ? QColor(200,64,64) : QColor(255,255,255);
			ui.dumpTable->item(row,col)->setBackgroundColor(bgcol);
			ui.dumpTable->item(row,col)->setText(gethexbyte(memRd(comp->mem, adr)));
			adr++;
		}
	}
	fillStack();
	block = false;
}

void DebugWin::dumpEdited(int row, int col) {
	if (block) return;
	if (col == 0) {
		dumpAdr = ui.dumpTable->item(row, 0)->text().toInt(NULL,16) - row * (ui.dumpTable->columnCount() - 1);
	} else {
		Z80EX_WORD adr = (dumpAdr + (col - 1) + row * (ui.dumpTable->columnCount() - 1)) & 0xffff;
		memWr(comp->mem, adr, ui.dumpTable->item(row, col)->text().toInt(NULL,16) & 0xff);
		fillDisasm();

		col++;
		if (col >= ui.dumpTable->columnCount()) {
			col = 1;
			row++;
			if (row >= ui.dumpTable->rowCount()) {
				row--;
				dumpAdr += (ui.dumpTable->columnCount() - 1);
			}
		}
		// ui.dumpTable->selectionModel()->select(ui.dumpTable->model()->index(row,col), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
		ui.dumpTable->setCurrentCell(row,col);
	}
	fillDump();
}

// stack

void DebugWin::fillStack() {
	int adr = GETSP(comp->cpu);
	QString str;
	for (int i = 0; i < 5; i++) {
		str.append(gethexbyte(memRd(comp->mem, adr+1)));
		str.append(gethexbyte(memRd(comp->mem, adr)));
		adr += 2;
	}
	ui.labSP->setText(str.left(4));
	ui.labSP2->setText(str.mid(4,4));
	ui.labSP4->setText(str.mid(8,4));
	ui.labSP6->setText(str.mid(12,4));
	ui.labSP8->setText(str.mid(16,4));
}

// breakpoint

int DebugWin::getAdr() {
	int adr = 0;
	if (ui.dasmTable->hasFocus()) {
		adr = ui.dasmTable->item(ui.dasmTable->currentRow(),0)->data(Qt::UserRole).toInt();
	} else if (ui.dumpTable->hasFocus()) {
		adr = (dumpAdr + (ui.dumpTable->currentColumn() - 1) + ui.dumpTable->currentRow() * (ui.dumpTable->columnCount() - 1)) & 0xffff;
	}
	return adr;
}

void DebugWin::putBreakPoint() {
	int adr = getAdr();
	doBreakPoint(adr);
	bpMenu->move(QCursor::pos());
	bpMenu->show();
}

void DebugWin::doBreakPoint(Z80EX_WORD adr) {
	bpAdr = adr;
	unsigned char flag = *memGetFptr(comp->mem, adr);
	ui.actFetch->setChecked(flag & MEM_BRK_FETCH);
	ui.actRead->setChecked(flag & MEM_BRK_RD);
	ui.actWrite->setChecked(flag & MEM_BRK_WR);
}

void DebugWin::chaBreakPoint() {
	unsigned char* ptr = memGetFptr(comp->mem, bpAdr);
	unsigned char flag = *ptr & ~MEM_BRK_ANY;
	if (ui.actFetch->isChecked()) flag |= MEM_BRK_FETCH;
	if (ui.actRead->isChecked()) flag |= MEM_BRK_RD;
	if (ui.actWrite->isChecked()) flag |= MEM_BRK_WR;
	*ptr = flag;
	fillDisasm();
	fillDump();
}

// memDump

void DebugWin::doSaveDump() {
	dui.leBank->setText(QString::number(comp->mem->pt[3]->num,16));
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
	MemPage* curBank = comp->mem->pt[3];
	int bank = dui.leBank->text().toInt(NULL,16);
	int adr = dui.leStart->text().toInt(NULL,16);
	int len = dui.leLen->text().toInt(NULL,16);
	memSetBank(comp->mem,3,MEM_RAM,bank);
	QByteArray res;
	while (len > 0) {
		res.append(memRd(comp->mem,adr));
		adr++;
		len--;
	}
	comp->mem->pt[3] = curBank;
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
		flpFormat(flp);
		flp->insert = 1;
	}
	TRFile dsc = flpMakeDescriptor(name.toStdString().c_str(), 'C', start, len);
	if (flpCreateFile(flp, dsc, (unsigned char*)data.data(), data.size()) == ERR_OK) dumpwin->hide();

}

// open dump

void DebugWin::doOpenDump() {
	dumpPath.clear();
	oui.laPath->clear();
	oui.leBank->setText(QString::number(comp->mem->pt[3]->num,16));
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
