#include "xcore/xcore.h"

#include <QIcon>
#include <QDebug>
#include <QFileDialog>
#include <QTemporaryFile>

#include "debuger.h"
#include "emulwin.h"
#include "filer.h"
#include "filetypes/filetypes.h"
#include "xgui/xgui.h"

#ifndef SELFZ80
	#include "z80ex_dasm.h"
#endif

#define NEWDEBUG

DebugWin* dbgWin;

unsigned long lastDbgTicks = 0;

QString logFileName;
QFile logFile;

void dbgInit(QWidget* par) {
	dbgWin = new DebugWin(par);
}

// TODO: extract from DebugWindow almost all

void dbgShow() {
	dbgWin->start();
}

bool dbgIsActive() {
	return dbgWin->active;
}

// OBJECT

void DebugWin::start() {
	emulPause(true,PR_DEBUG);
	disasmAdr = getPrevAdr(GETPC(zx->cpu));
	fillAll();
	show();
	vidFlag |= VF_DEBUG;
	vidDarkTail(zx->vid);
}

void DebugWin::stop() {
	vidFlag &= ~VF_DEBUG;
	zxExec(zx);
	hide();
	active = false;
	lastDbgTicks = zx->tickCount;
	emulPause(false,PR_DEBUG);
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
#ifdef NEWDEBUG
	int col,row;
	ui.setupUi(this);
// disasm table
	for (row = 0; row < ui.dasmTable->rowCount(); row++) {
		for (col = 0; col < ui.dasmTable->columnCount(); col++) {
			ui.dasmTable->setItem(row, col, new QTableWidgetItem);
		}
	}
	ui.dasmTable->setColumnWidth(0,50);
	ui.dasmTable->setColumnWidth(1,100);
	ui.dasmTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_ADR));
	ui.dasmTable->setItemDelegateForColumn(1, new xItemDelegate(XTYPE_DUMP));
	connect(ui.dasmTable,SIGNAL(cellChanged(int,int)),this,SLOT(dasmEdited(int,int)));
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

	setFixedSize(size());
#else
	QLabel *lab;
	int i,j;
	QVBoxLayout *mlay = new QVBoxLayout;
	QHBoxLayout *alay = new QHBoxLayout;
		QVBoxLayout *llay = new QVBoxLayout;
			QGroupBox *regbox = new QGroupBox("Registers");
				rglay = new QGridLayout;
					rglay->setVerticalSpacing(0);
					lab = new QLabel("AF"); rglay->addWidget(lab,0,0); lab = new QLabel; rglay->addWidget(lab,0,1); lab->setAutoFillBackground(true);
					lab = new QLabel("BC"); rglay->addWidget(lab,1,0); lab = new QLabel; rglay->addWidget(lab,1,1); lab->setAutoFillBackground(true);
					lab = new QLabel("DE"); rglay->addWidget(lab,2,0); lab = new QLabel; rglay->addWidget(lab,2,1); lab->setAutoFillBackground(true);
					lab = new QLabel("HL"); rglay->addWidget(lab,3,0); lab = new QLabel; rglay->addWidget(lab,3,1); lab->setAutoFillBackground(true);
					lab = new QLabel("AF'"); rglay->addWidget(lab,4,0); lab = new QLabel; rglay->addWidget(lab,4,1); lab->setAutoFillBackground(true);
					lab = new QLabel("BC'"); rglay->addWidget(lab,5,0); lab = new QLabel; rglay->addWidget(lab,5,1); lab->setAutoFillBackground(true);
					lab = new QLabel("DE'"); rglay->addWidget(lab,6,0); lab = new QLabel; rglay->addWidget(lab,6,1); lab->setAutoFillBackground(true);
					lab = new QLabel("HL'"); rglay->addWidget(lab,7,0); lab = new QLabel; rglay->addWidget(lab,7,1); lab->setAutoFillBackground(true);
					lab = new QLabel("IX"); rglay->addWidget(lab,8,0); lab = new QLabel; rglay->addWidget(lab,8,1); lab->setAutoFillBackground(true);
					lab = new QLabel("IY"); rglay->addWidget(lab,9,0); lab = new QLabel; rglay->addWidget(lab,9,1); lab->setAutoFillBackground(true);
					lab = new QLabel("IR"); rglay->addWidget(lab,10,0); lab = new QLabel; rglay->addWidget(lab,10,1); lab->setAutoFillBackground(true);
					lab = new QLabel("PC"); rglay->addWidget(lab,11,0); lab = new QLabel; rglay->addWidget(lab,11,1); lab->setAutoFillBackground(true);
					lab = new QLabel("SP"); rglay->addWidget(lab,12,0); lab = new QLabel; rglay->addWidget(lab,12,1); lab->setAutoFillBackground(true);
					lab = new QLabel("IM"); rglay->addWidget(lab,13,0); lab = new QLabel; rglay->addWidget(lab,13,1); lab->setAutoFillBackground(true);
					rglay->setColumnStretch(1,10);
					rglay->setRowStretch(100,10);
					rowincol[0] = 13;
				regbox->setLayout(rglay);
			QGroupBox *raybox = new QGroupBox("Mem");
				raylay = new QGridLayout;
					lab = new QLabel("Pg0"); raylay->addWidget(lab,0,0); lab = new QLabel; raylay->addWidget(lab,0,1);
					lab = new QLabel("Pg1"); raylay->addWidget(lab,1,0); lab = new QLabel; raylay->addWidget(lab,1,1);
					lab = new QLabel("Pg2"); raylay->addWidget(lab,2,0); lab = new QLabel; raylay->addWidget(lab,2,1);
					lab = new QLabel("Pg3"); raylay->addWidget(lab,3,0); lab = new QLabel; raylay->addWidget(lab,3,1);
					lab = new QLabel("DOSEN"); raylay->addWidget(lab,4,0); lab = new QLabel; raylay->addWidget(lab,4,1);
					lab = new QLabel("PRT0"); raylay->addWidget(lab,5,0); lab = new QLabel; raylay->addWidget(lab,5,1);
					lab = new QLabel("PRT1"); raylay->addWidget(lab,6,0); lab = new QLabel; raylay->addWidget(lab,6,1);
					lab = new QLabel("PRT2"); raylay->addWidget(lab,7,0); lab = new QLabel; raylay->addWidget(lab,7,1);
				raybox->setLayout(raylay);
			logLabel = new QLabel;
			llay->addWidget(regbox);
			llay->addWidget(raybox);
			llay->addWidget(logLabel);
			llay->addStretch(10);
		QGroupBox *asmbox = new QGroupBox("Disasm");
			asmlay = new QGridLayout;
				asmlay->setHorizontalSpacing(0);
				asmlay->setVerticalSpacing(0);
				for (i=0;i<DASMROW;i++) {
					lab = new QLabel; asmlay->addWidget(lab,i,0);
						lab->setAutoFillBackground(true);
					lab = new QLabel; asmlay->addWidget(lab,i,1);
						lab->setAutoFillBackground(true);
					lab = new QLabel; asmlay->addWidget(lab,i,2);
						lab->setAutoFillBackground(true);
				}
				rowincol[1] = rowincol[2] = rowincol[3] = DASMROW-1;
				asmlay->setColumnMinimumWidth(0,40);
				asmlay->setColumnMinimumWidth(1,80);
				asmlay->setColumnMinimumWidth(2,130);
			asmbox->setLayout(asmlay);
		QVBoxLayout *rlay = new QVBoxLayout;
			QGroupBox *dmpbox = new QGroupBox("Memory");
				dmplay = new QGridLayout;
				dmplay->setVerticalSpacing(0);
					for (i=0; i<DMPSIZE; i++) {
						lab = new QLabel; dmplay->addWidget(lab,i,0);
						lab->setAutoFillBackground(true);
						for (j=1; j<9; j++) {
							lab = new QLabel;
							lab->setAutoFillBackground(true);
							dmplay->addWidget(lab,i,j);
						}
					}
					for (i=4; i<13; i++) rowincol[i]=DMPSIZE-1;
				dmplay->setColumnMinimumWidth(0,40);
				dmpbox->setLayout(dmplay);
			QGroupBox *vgbox = new QGroupBox("BDI");
				vglay = new QGridLayout;
				vglay->setVerticalSpacing(0);
				lab = new QLabel("VG.TRK"); vglay->addWidget(lab,0,0); lab = new QLabel; vglay->addWidget(lab,0,1);
				lab = new QLabel("VG.SEC"); vglay->addWidget(lab,1,0); lab = new QLabel; vglay->addWidget(lab,1,1);
				lab = new QLabel("VG.DAT"); vglay->addWidget(lab,2,0); lab = new QLabel; vglay->addWidget(lab,2,1);
				lab = new QLabel("EmulVG.Com"); vglay->addWidget(lab,3,0); lab = new QLabel; vglay->addWidget(lab,3,1);
				lab = new QLabel("EmulVG.Wait"); vglay->addWidget(lab,4,0); lab = new QLabel; vglay->addWidget(lab,4,1);
				lab = new QLabel("VG.floppy"); vglay->addWidget(lab,5,0); lab = new QLabel; vglay->addWidget(lab,5,1);
				lab = new QLabel("VG.com"); vglay->addWidget(lab,6,0); lab = new QLabel; vglay->addWidget(lab,6,1);
				lab = new QLabel("FLP.TRK"); vglay->addWidget(lab,0,2); lab = new QLabel; vglay->addWidget(lab,0,3);
				lab = new QLabel("FLP.SID"); vglay->addWidget(lab,1,2); lab = new QLabel; vglay->addWidget(lab,1,3);
				lab = new QLabel("FLP.POS"); vglay->addWidget(lab,2,2); lab = new QLabel; vglay->addWidget(lab,2,3);
				lab = new QLabel("FLP.DAT"); vglay->addWidget(lab,3,2); lab = new QLabel; vglay->addWidget(lab,3,3);
				lab = new QLabel("FLP.FLD"); vglay->addWidget(lab,4,2); lab = new QLabel; vglay->addWidget(lab,4,3);
				vgbox->setLayout(vglay);
			rlay->addWidget(dmpbox);
			rlay->addWidget(vgbox);
			rlay->addStretch(10);
		alay->addLayout(llay);
		alay->addWidget(asmbox);
		alay->addLayout(rlay,10);
	mlay->addLayout(alay);
		QHBoxLayout *dlay = new QHBoxLayout;
			tlab = new QLabel;
			dlay->addWidget(tlab);
			dlay->addStretch(10);
	mlay->addLayout(dlay);
	setLayout(mlay);

	ledit = new QLineEdit(this);
	ledit->setWindowModality(Qt::ApplicationModal);
	ledit->setFrame(false);

	connect(ledit,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(editPosChanged(int,int)));
	connect(&logTimer,SIGNAL(timeout()),this,SLOT(logStep()));

	QPalette pal;
	pal.setColor(QPalette::ToolTipBase,QColor(128,255,128));
	pal.setColor(QPalette::Highlight,QColor(255,128,128));
	pal.setColor(QPalette::ToolTipText,QColor(255,0,0));
	setPalette(pal);

	setWindowTitle("Xpeccy deBUGa");
	setWindowIcon(QIcon(":/images/bug.png"));
	setFont(QFont("Monospace",9));
	setModal(true);
	setFixedSize(640,480);
#endif
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

	bpEditor = new QDialog();
	bui.setupUi(bpEditor);
	connect(bui.cbFetch,SIGNAL(stateChanged(int)),this,SLOT(chaBreakPoint()));
	connect(bui.cbRead,SIGNAL(stateChanged(int)),this,SLOT(chaBreakPoint()));
	connect(bui.cbWrite,SIGNAL(stateChanged(int)),this,SLOT(chaBreakPoint()));

	active = false;
	block = false;
	disasmAdr = 0;
	dumpAdr = 0;
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
		disasmAdr = ui.dasmTable->item(1,0)->text().toInt(NULL,16);
		fillDisasm();
	}
}

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	int i;
	int offset = (ui.dumpTable->columnCount() - 1) * (ui.dumpTable->rowCount() - 1);
	switch(ev->modifiers()) {
		case Qt::AltModifier:
			switch(ev->key()) {
				case Qt::Key_W:
					doSaveDump();
					break;
				case Qt::Key_R:
					doOpenDump();
					break;
				case Qt::Key_Z:
					if (!ui.dasmTable->hasFocus()) break;
					SETPC(zx->cpu, ui.dasmTable->item(ui.dasmTable->currentRow(), 0)->text().toInt(NULL,16));
					fillZ80();
					fillDisasm();
					break;
			}
			break;
		case Qt::ControlModifier:
			switch(ev->key()) {
				case Qt::Key_S:
					doSaveDump();
					break;
				case Qt::Key_O:
					doOpenDump();
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
					disasmAdr = GETPC(zx->cpu);
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
						disasmAdr = ui.dasmTable->item(ui.dasmTable->rowCount() - 1, 0)->text().toInt(NULL,16);
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
					loadFile(zx,"",FT_SNAP,0);
					disasmAdr = GETPC(zx->cpu);
					fillAll();
					break;
				case Qt::Key_F7:
					zxExec(zx);
					if (!fillAll()) {
						disasmAdr = GETPC(zx->cpu);
						fillDisasm();
					}
					break;
				case Qt::Key_F12:
					zxReset(zx,RES_DEFAULT);
					if (!fillAll()) {
						disasmAdr = GETPC(zx->cpu);
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
	fillZ80();
	fillMem();
	fillDump();
	return fillDisasm();
}

// z80 regs section

const char* flags = "SZ5H3PNC";

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

void DebugWin::fillZ80() {
	block = true;
	Z80EX_WORD af = GETAF(zx->cpu);
	ui.editAF->setText(gethexword(af));
	ui.editBC->setText(gethexword(GETBC(zx->cpu)));
	ui.editDE->setText(gethexword(GETDE(zx->cpu)));
	ui.editHL->setText(gethexword(GETHL(zx->cpu)));
	ui.editAFa->setText(gethexword(GETAF_(zx->cpu)));
	ui.editBCa->setText(gethexword(GETBC_(zx->cpu)));
	ui.editDEa->setText(gethexword(GETDE_(zx->cpu)));
	ui.editHLa->setText(gethexword(GETHL_(zx->cpu)));
	ui.editPC->setText(gethexword(GETPC(zx->cpu)));
	ui.editSP->setText(gethexword(GETSP(zx->cpu)));
	ui.editIX->setText(gethexword(GETIX(zx->cpu)));
	ui.editIY->setText(gethexword(GETIY(zx->cpu)));
	ui.editIR->setText(gethexword(GETIR(zx->cpu)));
	ui.boxIM->setValue(GETIM(zx->cpu));
	ui.flagIFF1->setChecked(GETIFF1(zx->cpu));
	ui.flagIFF2->setChecked(GETIFF1(zx->cpu));
	ui.labFlag->setText(flagString(af));
	block = false;
	fillStack();
}

void DebugWin::setZ80() {
	if (block) return;
	int af = ui.editAF->text().toInt(NULL,16);
	SETAF(zx->cpu, af);
	SETBC(zx->cpu, ui.editBC->text().toInt(NULL,16));
	SETDE(zx->cpu, ui.editDE->text().toInt(NULL,16));
	SETHL(zx->cpu, ui.editHL->text().toInt(NULL,16));
	SETAF_(zx->cpu, ui.editAFa->text().toInt(NULL,16));
	SETBC_(zx->cpu, ui.editBCa->text().toInt(NULL,16));
	SETDE_(zx->cpu, ui.editDEa->text().toInt(NULL,16));
	SETHL_(zx->cpu, ui.editHLa->text().toInt(NULL,16));
	SETPC(zx->cpu, ui.editPC->text().toInt(NULL,16));
	SETSP(zx->cpu, ui.editSP->text().toInt(NULL,16));
	SETIX(zx->cpu, ui.editIX->text().toInt(NULL,16));
	SETIY(zx->cpu, ui.editIY->text().toInt(NULL,16));
	SETIR(zx->cpu, ui.editIR->text().toInt(NULL,16));
	SETIM(zx->cpu, ui.boxIM->value());
	SETIFF1(zx->cpu, ui.flagIFF1->isChecked());
	SETIFF2(zx->cpu, ui.flagIFF2->isChecked());
	ui.labFlag->setText(flagString(af));
	fillStack();
}

// memory map section

QString getPageName(MemPage* pg) {
	QString res = (pg->type == MEM_RAM) ? "RAM-" : "ROM-";
	res.append(QString::number(pg->num));
	return res;
}

void DebugWin::fillMem() {
	ui.labPG0->setText(getPageName(zx->mem->pt[0]));
	ui.labPG1->setText(getPageName(zx->mem->pt[1]));
	ui.labPG2->setText(getPageName(zx->mem->pt[2]));
	ui.labPG3->setText(getPageName(zx->mem->pt[3]));
}

// disasm table

Z80EX_BYTE rdbyte(Z80EX_WORD adr,void*) {
	return memRd(zx->mem,adr);
}

DasmRow getDisasm(Z80EX_WORD& adr) {
	DasmRow drow;
	drow.adr = adr;
	drow.bytes.clear();
	drow.com.clear();
	char buf[256];
	int clen;
#ifdef SELFZ80
	clen = cpuDisasm(adr,buf,&rdbyte,NULL);
#else
	int t1,t2;
	clen = z80ex_dasm(buf,256,0,&t1,&t2,&rdbyte,adr,NULL);
#endif
	drow.com = QString(buf).toUpper();
	while (clen > 0) {
		drow.bytes.append(gethexbyte(memRd(zx->mem,adr)));
		clen--;
		adr++;
	}
	return drow;
}

bool DebugWin::fillDisasm() {
	block = true;
	Z80EX_WORD adr = disasmAdr;
	Z80EX_WORD pc = GETPC(zx->cpu);
	DasmRow drow;
	QColor bgcol,acol;
	bool res = false;
	for (int i = 0; i < ui.dasmTable->rowCount(); i++) {
		bgcol = (adr == pc) ? QColor(32,200,32) : QColor(255,255,255);
		acol = (memGetCellFlags(zx->mem, adr) & MEM_BRK_ANY) ? QColor(200,64,64) : bgcol;
		res |= (adr == pc);
		ui.dasmTable->item(i, 0)->setBackgroundColor(acol);
		ui.dasmTable->item(i, 1)->setBackgroundColor(bgcol);
		ui.dasmTable->item(i, 2)->setBackgroundColor(bgcol);
		ui.dasmTable->item(i, 0)->setText(gethexword(adr));
		drow = getDisasm(adr);
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
		getDisasm(tadr);			// shift tadr to next op
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
			memWr(zx->mem, adr, cbyte);
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
			bgcol = (memGetCellFlags(zx->mem, adr) & MEM_BRK_ANY) ? QColor(200,64,64) : QColor(255,255,255);
			ui.dumpTable->item(row,col)->setBackgroundColor(bgcol);
			ui.dumpTable->item(row,col)->setText(gethexbyte(memRd(zx->mem, adr)));
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
		memWr(zx->mem, adr, ui.dumpTable->item(row, col)->text().toInt(NULL,16) & 0xff);
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
	int adr = GETSP(zx->cpu);
	QString str;
	for (int i = 0; i < 5; i++) {
		str.append(gethexbyte(memRd(zx->mem, adr+1)));
		str.append(gethexbyte(memRd(zx->mem, adr)));
		adr += 2;
	}
	ui.labSP->setText(str.left(4));
	ui.labSP2->setText(str.mid(4,4));
	ui.labSP4->setText(str.mid(8,4));
	ui.labSP6->setText(str.mid(12,4));
	ui.labSP8->setText(str.mid(16,4));
}

// breakpoint

void DebugWin::putBreakPoint() {
	int adr;
	if (ui.dasmTable->hasFocus()) {
		adr = ui.dasmTable->item(ui.dasmTable->currentRow(),0)->text().toInt(NULL,16);
		doBreakPoint(adr);
	} else if (ui.dumpTable->hasFocus()) {
		adr = (dumpAdr + (ui.dumpTable->currentColumn() - 1) + ui.dumpTable->currentRow() * (ui.dumpTable->columnCount() - 1)) & 0xffff;
		doBreakPoint(adr);
	}
}

void DebugWin::doBreakPoint(Z80EX_WORD adr) {
	bpAdr = adr;
	MemPage* pg = zx->mem->pt[(adr >> 14) & 3];
	QString str = (pg->type == MEM_RAM) ? "RAM-" : "ROM-";
	str.append(gethexbyte(pg->num)).append(", ").append(gethexword(adr & 0x3fff));
	bui.bpAdr->setText(str);
	unsigned char flag = memGetCellFlags(zx->mem, adr);
	bui.cbFetch->setChecked(flag & MEM_BRK_FETCH);
	bui.cbRead->setChecked(flag & MEM_BRK_READ);
	bui.cbWrite->setChecked(flag & MEM_BRK_WRITE);
	bpEditor->show();
}

void DebugWin::chaBreakPoint() {
	unsigned char flag = memGetCellFlags(zx->mem, bpAdr) & ~MEM_BRK_ANY;
	if (bui.cbFetch->isChecked()) flag |= MEM_BRK_FETCH;
	if (bui.cbRead->isChecked()) flag |= MEM_BRK_READ;
	if (bui.cbWrite->isChecked()) flag |= MEM_BRK_WRITE;
	memSetCellFlags(zx->mem, bpAdr, flag);
	fillDisasm();
	fillDump();
}

/*
void DebugWin::filltick() {
	QString text = QString::number(zx->tickCount - lastDbgTicks);
	text.append(" | ").append(QString::number(zx->vid->x)).append(":").append(QString::number(zx->vid->y));
	text.append(" | ").append(QString::number(zx->nsCount)).append("/").append(QString::number(zx->nsPerFrame));

	tlab->setText(text);
}

void DebugWin::fillvg() {
	QLabel* lab;
	Floppy* flp = zx->bdi->fdc->fptr;	// current floppy
	lab = (QLabel*)vglay->itemAtPosition(0,1)->widget(); lab->setText(QString::number(zx->bdi->fdc->trk));
	lab = (QLabel*)vglay->itemAtPosition(1,1)->widget(); lab->setText(QString::number(zx->bdi->fdc->sec));
	lab = (QLabel*)vglay->itemAtPosition(2,1)->widget(); lab->setText(QString::number(zx->bdi->fdc->data));
	lab = (QLabel*)vglay->itemAtPosition(3,1)->widget();
	if (zx->bdi->fdc->wptr == NULL) {
		lab->setText("NULL");
	} else {
		lab->setText(QString::number(zx->bdi->fdc->cop,16));
	}
	lab = (QLabel*)vglay->itemAtPosition(4,1)->widget(); lab->setText(QString::number(zx->bdi->fdc->count));
	lab = (QLabel*)vglay->itemAtPosition(5,1)->widget(); lab->setText(QString::number(zx->bdi->fdc->fptr->id));
	lab = (QLabel*)vglay->itemAtPosition(6,1)->widget(); lab->setText(QString::number(zx->bdi->fdc->com,16));
	lab = (QLabel*)vglay->itemAtPosition(0,3)->widget(); lab->setText(QString::number(flp->trk));
	lab = (QLabel*)vglay->itemAtPosition(1,3)->widget(); lab->setText(zx->bdi->fdc->side ? "1" : "0");
	lab = (QLabel*)vglay->itemAtPosition(2,3)->widget(); lab->setText(QString::number(flp->pos));
	lab = (QLabel*)vglay->itemAtPosition(3,3)->widget(); lab->setText(QString::number(flpRd(flp),16));
	lab = (QLabel*)vglay->itemAtPosition(4,3)->widget(); lab->setText(QString::number(flp->field));
}

void DebugWin::fillrays() {
	QLabel *lab;
	lab = (QLabel*)raylay->itemAtPosition(0,1)->widget();
	lab->setText(QString((zx->mem->pt[0]->type == MEM_ROM) ? "ROM-" : "RAM-").append(QString::number(zx->mem->pt[0]->num)));
	lab = (QLabel*)raylay->itemAtPosition(1,1)->widget();
	lab->setText(QString((zx->mem->pt[1]->type == MEM_ROM) ? "ROM-" : "RAM-").append(QString::number(zx->mem->pt[1]->num)));
	lab = (QLabel*)raylay->itemAtPosition(2,1)->widget();
	lab->setText(QString((zx->mem->pt[2]->type == MEM_ROM) ? "ROM-" : "RAM-").append(QString::number(zx->mem->pt[2]->num)));
	lab = (QLabel*)raylay->itemAtPosition(3,1)->widget();
	lab->setText(QString((zx->mem->pt[3]->type == MEM_ROM) ? "ROM-" : "RAM-").append(QString::number(zx->mem->pt[3]->num)));
	lab = (QLabel*)raylay->itemAtPosition(4,1)->widget();
	lab->setText((zx->dosen & 1) ? "1" : "0");
	lab = (QLabel*)raylay->itemAtPosition(5,1)->widget(); lab->setText(getbinbyte(zx->prt0));
	lab = (QLabel*)raylay->itemAtPosition(6,1)->widget(); lab->setText(getbinbyte(zx->prt1));
	lab = (QLabel*)raylay->itemAtPosition(7,1)->widget(); lab->setText(getbinbyte(zx->prt2));
}

void DebugWin::filldump() {
	int i,j;
	adr = dmpadr;
	QLabel *lab;
	for (i=0; i<DMPSIZE; i++) {
		lab = (QLabel*)dmplay->itemAtPosition(i,0)->widget(); lab->setText(gethexword(adr));
		lab->setBackgroundRole((curcol==4 && currow==i)?QPalette::Highlight:QPalette::Window);
		for (j=1; j<9; j++) {
			lab = (QLabel*)dmplay->itemAtPosition(i,j)->widget();
			lab->setText(gethexbyte(memRd(zx->mem,adr)));
			adr++;
			lab->setBackgroundRole((curcol==4+j && currow==i)?QPalette::Highlight:QPalette::Window);
		}
	}
}

void DebugWin::fillregz() {
	QLabel *lab;
	lab = (QLabel*)rglay->itemAtPosition(0,1)->widget(); lab->setText(gethexword(GETAF(zx->cpu)));	// af
	lab->setBackgroundRole((curcol==0 && currow==0)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(1,1)->widget(); lab->setText(gethexword(GETBC(zx->cpu)));	// bc
	lab->setBackgroundRole((curcol==0 && currow==1)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(2,1)->widget(); lab->setText(gethexword(GETDE(zx->cpu)));	// de
	lab->setBackgroundRole((curcol==0 && currow==2)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(3,1)->widget(); lab->setText(gethexword(GETHL(zx->cpu)));	// hl
	lab->setBackgroundRole((curcol==0 && currow==3)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(4,1)->widget(); lab->setText(gethexword(GETAF_(zx->cpu)));	// af'
	lab->setBackgroundRole((curcol==0 && currow==4)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(5,1)->widget(); lab->setText(gethexword(GETBC_(zx->cpu)));	// bc'
	lab->setBackgroundRole((curcol==0 && currow==5)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(6,1)->widget(); lab->setText(gethexword(GETDE_(zx->cpu)));	// de'
	lab->setBackgroundRole((curcol==0 && currow==6)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(7,1)->widget(); lab->setText(gethexword(GETHL_(zx->cpu)));	// hl'
	lab->setBackgroundRole((curcol==0 && currow==7)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(8,1)->widget(); lab->setText(gethexword(GETIX(zx->cpu)));	// ix
	lab->setBackgroundRole((curcol==0 && currow==8)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(9,1)->widget(); lab->setText(gethexword(GETIY(zx->cpu)));	// iy
	lab->setBackgroundRole((curcol==0 && currow==9)?QPalette::Highlight:QPalette::Window);
	Z80EX_WORD ir = GETI(zx->cpu); // ((GETI(zx->cpu)) << 8) | (GETR(zx->cpu));
	lab = (QLabel*)rglay->itemAtPosition(10,1)->widget(); lab->setText(gethexword(ir));	// ir
	lab->setBackgroundRole((curcol==0 && currow==10)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(11,1)->widget(); lab->setText(gethexword(GETPC(zx->cpu)));		// pc
	lab->setBackgroundRole((curcol==0 && currow==11)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(12,1)->widget(); lab->setText(gethexword(GETSP(zx->cpu)));	// sp
	lab->setBackgroundRole((curcol==0 && currow==12)?QPalette::Highlight:QPalette::Window);
	lab = (QLabel*)rglay->itemAtPosition(13,1)->widget(); lab->setText(QString::number(GETIM(zx->cpu)).append(" ").append(QString::number(GETIFF1(zx->cpu))).append(QString::number(GETIFF2(zx->cpu))));
	lab->setBackgroundRole((curcol==0 && currow==13)?QPalette::Highlight:QPalette::Window);
}
*/

/*
DasmRow DebugWin::getdisasm() {
	DasmRow res;
	int clen;
	res.adr = adr;
	res.bytes = "";
	res.dasm = "";
	char* buf = new char[256];
#ifdef SELFZ80
	clen = cpuDisasm(adr,buf,&rdbyte,NULL);
#else
	int t1,t2;
	clen = z80ex_dasm(buf,256,0,&t1,&t2,&rdbyte,adr,NULL);
#endif
//	if (clen > 4) clen=4;		// FIXME: z80ex_dasm bug? for DDCB instructions length
	res.dasm = QString(buf);
	while (clen > 0) {
		res.bytes.append(gethexbyte(memRd(zx->mem,adr)));
		clen--;
		adr++;
	}
	free(buf);
	return res;
}

bool DebugWin::filldasm() {
	fdasm.clear();
	adr = upadr;
	uchar i;
	unsigned char idx;
	bool ispc = false;
	DasmRow res;
	QLabel *lab1,*lab2,*lab3;
	Z80EX_WORD pc = GETPC(zx->cpu);
	for (i=0; i<DASMROW; i++) {
		if (adr == pc) ispc=true;
		idx = memGetCellFlags(zx->mem,adr);
		lab1 = (QLabel*)asmlay->itemAtPosition(i,0)->widget();
		lab2 = (QLabel*)asmlay->itemAtPosition(i,1)->widget();
		lab3 = (QLabel*)asmlay->itemAtPosition(i,2)->widget();
		lab1->setBackgroundRole((pc == adr) ? QPalette::ToolTipBase : QPalette::Window);
		lab2->setBackgroundRole((pc == adr) ? QPalette::ToolTipBase : QPalette::Window);
		lab3->setBackgroundRole((pc == adr) ? QPalette::ToolTipBase : QPalette::Window);
		lab1->setForegroundRole((idx == 0) ? QPalette::WindowText : QPalette::ToolTipText);
		lab2->setForegroundRole((idx == 0) ? QPalette::WindowText : QPalette::ToolTipText);
		lab3->setForegroundRole((idx == 0) ? QPalette::WindowText : QPalette::ToolTipText);
		if (curcol==1 && currow==i) lab1->setBackgroundRole(QPalette::Highlight);
		if (curcol==2 && currow==i) lab2->setBackgroundRole(QPalette::Highlight);
		if (curcol==3 && currow==i) lab3->setBackgroundRole(QPalette::Highlight);
		res = getdisasm();
		fdasm.append(res);
		lab1->setText(gethexword(res.adr));
		lab2->setText(res.bytes);
		lab3->setText(res.dasm);
	}
	return ispc;
}

ushort DebugWin::getprevadr(ushort ad) {
	ushort i;
	for (i=16;i>0;i--) {
		adr = ad-i;
		getdisasm();
		if (adr==ad) return (ad-i);
	}
	return (ad-1);
}

void DebugWin::showedit(QLabel* lab,QString imsk) {
	ledit->resize(lab->size());// + QSize(10,10));
	ledit->setParent(lab->parentWidget());
	ledit->move(lab->pos() - QPoint(2,0));
	ledit->setInputMask(imsk);
	ledit->setText(lab->text());
	ledit->setCursorPosition(0);
	ledit->show();
	ledit->setFocus();
}
*/
/*

void DebugWin::keyPressEvent(QKeyEvent* ev) {
	if (logging) {
		stopLog();
		return;
	}
	qint32 cod = ev->key();
	QLabel *lab = NULL;
	uchar i;
//	int idx;
	if (!ledit->isVisible()) {
		switch (ev->modifiers()) {
		case Qt::ControlModifier:
			switch (cod) {
				case Qt::Key_S:
					doSaveDump();
					break;
				case Qt::Key_O:
					doOpenDump();
					break;
			}
			break;
		case Qt::AltModifier:
			switch (cod) {
				case Qt::Key_A: dmpadr = GETAF(zx->cpu); filldump(); break;
				case Qt::Key_B: dmpadr = GETBC(zx->cpu); filldump(); break;
				case Qt::Key_D: dmpadr = GETDE(zx->cpu); filldump(); break;
				case Qt::Key_H: dmpadr = GETHL(zx->cpu); filldump(); break;
				case Qt::Key_X: dmpadr = GETIX(zx->cpu); filldump(); break;
				case Qt::Key_Y: dmpadr = GETIY(zx->cpu); filldump(); break;
				case Qt::Key_S: dmpadr = GETSP(zx->cpu); filldump(); break;
				case Qt::Key_P: dmpadr = GETPC(zx->cpu); filldump(); break;
				case Qt::Key_L: startLog(); break;
				case Qt::Key_W:
					doSaveDump();
					break;
				case Qt::Key_R:
					doOpenDump();
					break;
			}
			break;
		case Qt::NoModifier:
			switch (cod) {
				case Qt::Key_Escape:
					if (!ev->isAutoRepeat()) stop();
					break;
				case Qt::Key_Return:

					if (ev->isAutoRepeat()) break;
					switch (curcol) {
						case 0: showedit((QLabel*)rglay->itemAtPosition(currow,1)->widget(),"HHHH"); break;
						case 1: showedit((QLabel*)asmlay->itemAtPosition(currow,0)->widget(),"HHHH"); break;
						case 2: showedit((QLabel*)asmlay->itemAtPosition(currow,1)->widget(),"HHHHHHHHHH"); break;
						case 3: showedit((QLabel*)asmlay->itemAtPosition(currow,2)->widget(),""); break;
						case 4: showedit((QLabel*)dmplay->itemAtPosition(currow,0)->widget(),"HHHH"); break;
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:
						case 10:
						case 11:
						case 12: showedit((QLabel*)dmplay->itemAtPosition(currow,curcol-4)->widget(),"HH"); break;
					}

					break;
				case Qt::Key_Space:
					if (!ev->isAutoRepeat() && curcol>0 && curcol<4) {
						memSwitchCellFlags(zx->mem,fdasm[currow].adr,MEM_BRK_FETCH);
						filldasm();
					}
					break;
				case Qt::Key_Z:
					if (curcol>0 && curcol<4) {
						SETPC(zx->cpu,fdasm[currow].adr);
						filldasm();
					}
					break;
				case Qt::Key_F2:
					doSaveDump();
					break;
				case Qt::Key_F3:
					loadFile(zx,"",FT_SNAP,0);
					upadr = GETPC(zx->cpu);
					fillall();
					break;
				case Qt::Key_F7:
					doStep();
					break;
				case Qt::Key_F8:
					lastDbgTicks = zx->tickCount;
					cpoint.adr = GETPC(zx->cpu);	// z80ex_get_reg(zx->cpu,regPC);
					cpoint.sp = GETSP(zx->cpu);	// z80ex_get_reg(zx->cpu,regSP);
					cpoint.active = true;
					stop();
					break;
				case Qt::Key_F12:
					zxReset(zx,RES_DEFAULT);
					fillall();
					break;
				case Qt::Key_Left:
					if (curcol>0) {
						curcol--;
						if (currow >= rowincol[curcol]) currow = rowincol[curcol];
					}
					fillall();
					break;
				case Qt::Key_Right:
					if (curcol<12) {
						curcol++;
						if (currow >= rowincol[curcol]) currow = rowincol[curcol];
					}
					fillall();
					break;
				case Qt::Key_Down:
					if (currow < rowincol[curcol]) {
						currow++;
					} else {
						if (curcol>0 && curcol<4) {
							lab = (QLabel*)asmlay->itemAtPosition(1,0)->widget();
							upadr = lab->text().toInt(NULL,16);
						}
						if (curcol>3 && curcol<12) dmpadr += 8;
					}
					fillall();
					break;
				case Qt::Key_Up:
					if (currow > 0) {
						currow--;
					} else {
						if (curcol>0 && curcol<4) upadr = getprevadr(upadr);
						if (curcol>3 && curcol<12) dmpadr -= 8;
					}
					fillall();
					break;
				case Qt::Key_PageDown:
					if (curcol>0 && curcol<4) {
						lab = (QLabel*)asmlay->itemAtPosition(DASMROW-1,0)->widget();
						upadr = lab->text().toInt(NULL,16);
						filldasm();
					}
					if (curcol>3 && curcol<12) {
						dmpadr += (DMPSIZE << 3);
						filldump();
					}
					break;
				case Qt::Key_PageUp:
					if (curcol>0 && curcol<4) {
						for (i=0; i<DASMROW-2; i++) upadr=getprevadr(upadr);
						filldasm();
					}
					if (curcol>3 && curcol<12) {
						dmpadr -= (DMPSIZE << 3);
						filldump();
					}
					break;
				case Qt::Key_Home:
					upadr = GETPC(zx->cpu);	// z80ex_get_reg(zx->cpu,regPC);
					filldasm();
					break;
			}
			break;
		}
	} else {
		int idx;
		switch (cod) {
			case Qt::Key_Escape:
				ledit->hide();
				setFocus();
				break;
			case Qt::Key_Return:
				tmpb = false;
				switch (curcol) {
					case 0: idx = ledit->text().toUShort(&tmpb,16);
						if (!tmpb) break;
						switch (currow) {
							case 0: SETAF(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regAF,idx); break;
							case 1: SETBC(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regBC,idx); break;
							case 2: SETDE(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regDE,idx); break;
							case 3: SETHL(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regHL,idx); break;
							case 4: SETAF_(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regAF_,idx); break;
							case 5: SETBC_(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regBC_,idx); break;
							case 6: SETDE_(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regDE_,idx); break;
							case 7: SETHL_(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regHL_,idx); break;
							case 8: SETIX(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regIX,idx); break;
							case 9: SETIY(zx->cpu,idx); break; //z80ex_set_reg(zx->cpu,regIY,idx); break;
							case 10: SETI(zx->cpu,(idx & 0xff00) >> 8); // z80ex_set_reg(zx->cpu,regI,(idx & 0xff00) >> 8);
								SETR(zx->cpu,idx & 0xff); //z80ex_set_reg(zx->cpu,regR,idx & 0xff);
								//z80ex_set_reg(zx->cpu,regR7,idx & 0x80);
								break;
							case 11: SETPC(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regPC,idx); break;
							case 12: SETSP(zx->cpu,idx); break; // z80ex_set_reg(zx->cpu,regSP,idx); break;
							case 13: if ((idx & 0xf00) > 0x200) {
									tmpb = false;
								} else {
									SETIM(zx->cpu,(idx & 0xf00) >> 8); // z80ex_set_reg(zx->cpu,regIM,(idx & 0xf00)>>8);
									SETIFF1(zx->cpu, (idx & 0xf0) ? 1 : 0); //z80ex_set_reg(zx->cpu,regIFF1,idx & 0xf0);
									SETIFF2(zx->cpu, (idx & 0x0f) ? 1 : 0); // z80ex_set_reg(zx->cpu,regIFF2,idx & 0x0f);
								}
								break;
						}
						fillregz();
						break;
					case 1:
						idx = ledit->text().toUShort(&tmpb,16);
						if (!tmpb) break;
						upadr = idx;
						for (i=0; i<currow; i++) upadr = getprevadr(upadr);
						filldasm();
						break;
					case 4:
						idx = ledit->text().toUShort(&tmpb,16);
						if (!tmpb) break;
						dmpadr = idx - ((currow)<<3);
						filldump();
						break;
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12: moveMemEdit();
						tmpb=false;
						break;
				}
				if (tmpb) {ledit->hide(); setFocus();}
				break;
		}
	}
}
*/

/*
void DebugWin::editPosChanged(int op, int np) {
	if (!ledit->isVisible()) return;
	if ((curcol < 5) || (curcol > 12)) return;
	if (np > 1) moveMemEdit();
}

void DebugWin::moveMemEdit() {
	bool tmpb;
	int idx = ledit->text().toShort(&tmpb,16);
	tmpb &= (idx<0x100);
	if (!tmpb) return;
	ledit->hide();
	memWr(zx->mem, dmpadr + (currow << 3) + curcol - 5, idx & 0xff);
	curcol++;
	if (curcol > 12) {
		curcol=5; currow++;
		if (currow > rowincol[5]) {currow--; dmpadr += 8;}
	}
	filldump();
	filldasm();
	showedit((QLabel*)dmplay->itemAtPosition(currow,curcol-4)->widget(),"HH");
}

void DebugWin::doStep() {
	lastDbgTicks = zx->tickCount;
	zxExec(zx);
	if (!fillall()) {
		upadr = GETPC(zx->cpu);	// z80ex_get_reg(zx->cpu,regPC);
		filldasm();
	}
}

// LOGGING

void DebugWin::startLog() {
	logFileName = QFileDialog::getSaveFileName(this,"Save log as...");
	if (!logFileName.isEmpty()) {
		logFile.setFileName(logFileName);
		if (logFile.open(QFile::WriteOnly)) {
			logging = true;
			logLabel->setText("LOG TRACE");
			logTimer.start(10);
		}
	}
}

void DebugWin::stopLog() {
	logging = false;
	logFile.close();
	logTimer.stop();
	logLabel->clear();
}

void DebugWin::logStep() {
	adr = GETPC(zx->cpu);
	DasmRow row = getdisasm();
	logFile.write(gethexword(row.adr).toUtf8());
	logFile.write("  ");
	logFile.write(row.dasm.toUtf8());
	doStep();
	logFile.write("\r\nAF:");
	logFile.write(gethexword(GETAF(zx->cpu)).toUtf8());
	logFile.write(" BC:");
	logFile.write(gethexword(GETBC(zx->cpu)).toUtf8());
	logFile.write(" DE:");
	logFile.write(gethexword(GETDE(zx->cpu)).toUtf8());
	logFile.write(" HL:");
	logFile.write(gethexword(GETHL(zx->cpu)).toUtf8());
	logFile.write(" AF'");
	logFile.write(gethexword(GETAF_(zx->cpu)).toUtf8());
	logFile.write(" BC'");
	logFile.write(gethexword(GETBC_(zx->cpu)).toUtf8());
	logFile.write(" DE'");
	logFile.write(gethexword(GETDE_(zx->cpu)).toUtf8());
	logFile.write(" HL'");
	logFile.write(gethexword(GETHL_(zx->cpu)).toUtf8());
	logFile.write(" IX:");
	logFile.write(gethexword(GETIX(zx->cpu)).toUtf8());
	logFile.write(" IY:");
	logFile.write(gethexword(GETIY(zx->cpu)).toUtf8());
	logFile.write(" SP:");
	logFile.write(gethexword(GETSP(zx->cpu)).toUtf8());
	logFile.write(" IR:");
	logFile.write(gethexbyte(GETI(zx->cpu)).toUtf8());
	logFile.write(gethexbyte(GETR(zx->cpu)).toUtf8());
	logFile.write("\r\n");
}
*/

// memDump

void DebugWin::doSaveDump() {
	dui.leBank->setText(QString::number(zx->mem->pt[3]->num,16));
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
	if (start + len > 0xffff) len = 0x10000 - start;
	int end = start + len - 1;
	start = dui.leLen->cursorPosition();
	dui.leEnd->setText(QString::number(end,16));
	dui.leLen->setText(QString::number(len,16));
	dui.leLen->setCursorPosition(start);
}

QByteArray DebugWin::getDumpData() {
	MemPage* curBank = zx->mem->pt[3];
	int bank = dui.leBank->text().toInt(NULL,16);
	int adr = dui.leStart->text().toInt(NULL,16);
	int len = dui.leLen->text().toInt(NULL,16);
	memSetBank(zx->mem,3,MEM_RAM,bank);
	QByteArray res;
	while (len > 0) {
		res.append(memRd(zx->mem,adr));
		adr++;
		len--;
	}
	zx->mem->pt[3] = curBank;
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
	Floppy* flp = zx->bdi->fdc->flop[idx & 3];
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
	oui.leBank->setText(QString::number(zx->mem->pt[3]->num,16));
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
	QFile file(dumpPath);
	if (file.open(QFile::ReadOnly)) {
		QByteArray data = file.readAll();
		int adr = oui.leStart->text().toInt(NULL,16);
		for (int i = 0; i < data.size(); i++) {
			memWr(zx->mem, adr, data[i]);
			adr++;
		}
		fillAll();
		openDumpDialog->hide();
	} else {
		shitHappens("Can't open file");
	}
}
