#include <QStandardItemModel>
#include <QInputDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QVector3D>
#include <QPainter>
#include <QDebug>
#include <stdlib.h>

#include <SDL.h>

#include "filer.h"
#include "setupwin.h"
#include "xgui/xgui.h"
#include "xcore/gamepad.h"
#include "xcore/xcore.h"
#include "xcore/sound.h"
#include "libxpeccy/spectrum.h"
#include "libxpeccy/filetypes/filetypes.h"
#include "libxpeccy/input.h"

void fillRFBox(QComboBox* box, QStringList lst) {
	box->clear();
	box->addItem("none","");
	foreach(QString str, lst) {
		box->addItem(str, str);
	}
}

void setRFIndex(QComboBox* box, QVariant data) {
	box->setCurrentIndex(box->findData(data));
}

int getRFIData(QComboBox* box) {
	return box->itemData(box->currentIndex()).toInt();
}

QString getRFSData(QComboBox* box) {
	return box->itemData(box->currentIndex()).toString();
}

std::string getRFText(QComboBox* box) {
	QString res = "";
	if (box->currentIndex() >= 0) res = box->currentText();
	return std::string(res.toLocal8Bit().data());
}

void fill_romset_list(QComboBox* box, QString txt = QString()) {
	if (txt.isEmpty())
		txt = box->currentText();
	box->clear();
	foreach(xRomset rs, conf.rsList) {
		box->addItem(QString::fromLocal8Bit(rs.name.c_str()));
	}
	box->setCurrentIndex(box->findText(txt));
}

void fill_layout_list(QComboBox* box, QString txt = QString()) {
	if (txt.isEmpty())
		txt = box->currentText();
	box->clear();
	foreach(xLayout ly, conf.layList) {
		box->addItem(QString::fromLocal8Bit(ly.name.c_str()));
	}
	box->setCurrentIndex(box->findText(txt));
}

void fill_shader_list(QComboBox* box) {
	QDir dir(conf.path.shdDir.c_str());
	QFileInfoList lst = dir.entryInfoList(QStringList() << "*.txt", QDir::Files, QDir::Name);
	QFileInfo inf;
	box->clear();
	box->addItem("none", 0);
#if defined(USEOPENGL)
	if (conf.vid.shd_support) {
		foreach(inf, lst) {
			box->addItem(inf.fileName(), 1);
		}
		box->setCurrentIndex(box->findText(conf.vid.shader.c_str()));
		if (box->currentIndex() < 0)
			box->setCurrentIndex(0);
	}
#else
	box->setCurrentIndex(0);
#endif
}

// OBJECT

void dbg_fill_chip_boxes(QComboBox* cbtype, QComboBox* cbstereo) {
	cbtype->clear();
	cbtype->addItem(QIcon(":/images/cancel.png"),"none",SND_NONE);
	cbtype->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",SND_AY);
	cbtype->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",SND_YM);
	cbstereo->clear();
	cbstereo->addItem("Mono",AY_MONO);
	cbstereo->addItem("ABC",AY_ABC);
	cbstereo->addItem("ACB",AY_ACB);
	cbstereo->addItem("BAC",AY_BAC);
	cbstereo->addItem("BCA",AY_BCA);
	cbstereo->addItem("CAB",AY_CAB);
	cbstereo->addItem("CBA",AY_CBA);
}

SetupWin::SetupWin(QWidget* par):QDialog(par) {
	setModal(true);
	ui.setupUi(this);

	umadial = new QDialog;
	uia.setupUi(umadial);
	umadial->setModal(true);

	rseditor = new xRomsetEditor(this);
	rsmodel = new xRomsetModel();
	ui.tvRomset->setModel(rsmodel);

	layeditor = new QDialog(this);
	layUi.setupUi(layeditor);
	layeditor->setModal(true);

	padial = new xPadBinder(this);
	kedit = new xKeyEditor(this);

	int i;
// machine
	i = 0;
	while (hwTab[i].name) {
		if (hwTab[i].id != HW_NULL) {
			ui.machbox->addItem(hwTab[i].optName,QString::fromLocal8Bit(hwTab[i].name));
		} else {
			ui.machbox->insertSeparator(i);
		}
		i++;
	}
	i = 0;
	while (cpuTab[i].type != CPU_NONE) {
		ui.cbCpu->addItem(cpuTab[i].name, cpuTab[i].type);
		i++;
	}
	ui.resbox->addItem("BASIC 48",RES_48);
	ui.resbox->addItem("BASIC 128",RES_128);
	ui.resbox->addItem("DOS",RES_DOS);
	ui.resbox->addItem("SHADOW",RES_SHADOW);

	ui.tvRomset->setColumnWidth(0,50);
	ui.tvRomset->setColumnWidth(1,200);
	ui.tvRomset->setColumnWidth(2,70);
	ui.tvRomset->setColumnWidth(3,70);
	ui.tvRomset->setColumnWidth(4,70);
// video
	std::map<std::string,int>::iterator it;
	for (it = shotFormat.begin(); it != shotFormat.end(); it++) {
		ui.ssfbox->addItem(QString(it->first.c_str()),it->second);
	}
#if defined(USEOPENGL)
	ui.cbScanlines->setVisible(false);
	fill_shader_list(ui.cbShader);
#else
	ui.labShader->setVisible(false);
	ui.cbShader->setVisible(false);
#endif
// sound
	i = 0;
	while (sndTab[i].name) {
		ui.outbox->addItem(QString::fromLocal8Bit(sndTab[i].name));
		i++;
	}
	ui.ratbox->addItem("48000",48000);
	ui.ratbox->addItem("44100",44100);
	ui.ratbox->addItem("22050",22050);
	ui.ratbox->addItem("11025",11025);
	dbg_fill_chip_boxes(ui.schip1box, ui.stereo1box);
	dbg_fill_chip_boxes(ui.schip2box, ui.stereo2box);
	dbg_fill_chip_boxes(ui.schip3box, ui.stereo3box);
	ui.tsbox->addItem("None",TS_NONE);
	ui.tsbox->addItem("NedoPC",TS_NEDOPC);
	ui.tsbox->addItem("ZXNext", TS_ZXNEXT);
	ui.sdrvBox->addItem("None",SDRV_NONE);
	ui.sdrvBox->addItem("Covox only",SDRV_COVOX);
	ui.sdrvBox->addItem("Soundrive 1.05 mode 1",SDRV_105_1);
	ui.sdrvBox->addItem("Soundrive 1.05 mode 2",SDRV_105_2);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	QRegExpValidator* vld = new QRegExpValidator(QRegExp("^[0-9]\\.\\d{0,6}$"));
#else
	QRegularExpressionValidator* vld = new QRegularExpressionValidator(QRegularExpression("^[0-9]\\.\\d{0,6}$"));
#endif
	ui.psg1frq->setValidator(vld);
	ui.psg2frq->setValidator(vld);
	ui.psg3frq->setValidator(vld);
// bdi
	ui.disklist->horizontalHeader()->setVisible(true);
	ui.diskTypeBox->addItem("None",DIF_NONE);
	ui.diskTypeBox->addItem("Beta disk (VG93)",DIF_BDI);
	ui.diskTypeBox->addItem("+3 DOS (uPD765)",DIF_P3DOS);
	ui.diskTypeBox->addItem("PC FDC (i8272)", DIF_PC);
	ui.diskTypeBox->addItem("SMK512 (VP1-128)",DIF_SMK512);
	ui.disklist->addAction(ui.actCopyToTape);
	ui.disklist->addAction(ui.actSaveHobeta);
	ui.disklist->addAction(ui.actSaveRaw);
// tape
	ui.tapelist->setColumnWidth(0,25);
	ui.tapelist->setColumnWidth(1,25);
	ui.tapelist->setColumnWidth(2,50);
	ui.tapelist->setColumnWidth(3,50);
	ui.tapelist->setColumnWidth(4,100);
	ui.tapelist->addAction(ui.actCopyToDisk);
// hdd
	ui.hiface->addItem("None",IDE_NONE);
	ui.hiface->addItem("Nemo",IDE_NEMO);
	ui.hiface->addItem("Nemo A8",IDE_NEMOA8);
	ui.hiface->addItem("Nemo Evo",IDE_NEMO_EVO);
	ui.hiface->addItem("SMUC",IDE_SMUC);
	ui.hiface->addItem("ATM",IDE_ATM);
	ui.hiface->addItem("Profi",IDE_PROFI);
	ui.hiface->addItem("SMK512",IDE_SMK);
	ui.hm_type->addItem(QIcon(":/images/cancel.png"),"Not connected",IDE_NONE);
	ui.hm_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",IDE_ATA);
	ui.hs_type->addItem(QIcon(":/images/cancel.png"),"Not connected",IDE_NONE);
	ui.hs_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",IDE_ATA);
// others
	ui.cSlotType->addItem("No mapper",MAP_MSX_NOMAPPER);
	ui.cSlotType->addItem("Konami 4",MAP_MSX_KONAMI4);
	ui.cSlotType->addItem("Konami 5",MAP_MSX_KONAMI5);
	ui.cSlotType->addItem("ASCII 8K",MAP_MSX_ASCII8);
	ui.cSlotType->addItem("ASCII 16K",MAP_MSX_ASCII16);
// input
	padModel = new xPadMapModel();
	ui.tvPadTable->setModel(padModel);
	ui.tvPadTable->addAction(ui.actAddBinding);
	ui.tvPadTable->addAction(ui.actEditBinding);
	ui.tvPadTable->addAction(ui.actDelBinding);
	ui.cbScanTab->addItem("Scanset 1 (XT)", KBD_XT);
	ui.cbScanTab->addItem("Scanset 2 (AT)", KBD_AT);
	ui.cbScanTab->addItem("Scanset 3 (PS/2)", KBD_PS2);
	ui.cbMouseType->addItem("Serial", MOUSE_SERIAL);
	ui.cbMouseType->addItem("PS/2", MOUSE_PS2);
// all
	connect(ui.okbut,SIGNAL(released()),this,SLOT(okay()));
	connect(ui.apbut,SIGNAL(released()),this,SLOT(apply()));
	connect(ui.cnbut,SIGNAL(released()),this,SLOT(reject()));
// machine
	connect(ui.rsetbox,SIGNAL(currentIndexChanged(int)),this,SLOT(buildrsetlist()));
	connect(ui.machbox,SIGNAL(currentIndexChanged(int)),this,SLOT(setmszbox(int)));
	connect(ui.tvRomset,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editRom()));
	connect(ui.addrset,SIGNAL(released()),this,SLOT(addNewRomset()));
	connect(ui.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
	connect(rseditor,SIGNAL(complete(xRomFile)),this,SLOT(setRom(xRomFile)));
	connect(ui.tbAddRom,SIGNAL(released()),this,SLOT(addRom()));
	connect(ui.tbEditRom,SIGNAL(released()),this,SLOT(editRom()));
	connect(ui.tbDelRom,SIGNAL(released()),this,SLOT(delRom()));
	connect(ui.tbPreset,SIGNAL(released()),this,SLOT(romPreset()));
// video
	connect(ui.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	connect(ui.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));
	connect(ui.sldNoflic,SIGNAL(valueChanged(int)),this,SLOT(chaflc()));

	connect(ui.layEdit,SIGNAL(released()),this,SLOT(edLayout()));
	connect(ui.layAdd,SIGNAL(released()),this,SLOT(addNewLayout()));
	connect(ui.layDel,SIGNAL(released()),this,SLOT(delLayout()));

	connect(layUi.layName,SIGNAL(textChanged(QString)),this,SLOT(layNameCheck(QString)));
	connect(layUi.lineBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.rowsBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.brdLBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.brdUBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.hsyncBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.vsyncBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.intLenBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.intPosBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.intRowBox,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.sbScrH,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.sbScrW,SIGNAL(valueChanged(int)),this,SLOT(layEditorChanged()));
	connect(layUi.okButton,SIGNAL(released()),this,SLOT(layEditorOK()));
	connect(layUi.cnButton,SIGNAL(released()),layeditor,SLOT(hide()));
// sound
	connect(ui.sldMasterVol,SIGNAL(valueChanged(int)),ui.sbMasterVol,SLOT(setValue(int)));
	connect(ui.sldBeepVol,SIGNAL(valueChanged(int)),ui.sbBeepVol,SLOT(setValue(int)));
	connect(ui.sldTapeVol,SIGNAL(valueChanged(int)),ui.sbTapeVol,SLOT(setValue(int)));
	connect(ui.sldAYVol,SIGNAL(valueChanged(int)),ui.sbAYVol,SLOT(setValue(int)));
	connect(ui.sldGSVol,SIGNAL(valueChanged(int)),ui.sbGSVol,SLOT(setValue(int)));
	connect(ui.sldSdrvVol,SIGNAL(valueChanged(int)),ui.sbSdrvVol,SLOT(setValue(int)));
	connect(ui.sldSAAVol,SIGNAL(valueChanged(int)),ui.sbSAAVol,SLOT(setValue(int)));

	connect(ui.sbMasterVol,SIGNAL(valueChanged(int)),ui.sldMasterVol,SLOT(setValue(int)));
	connect(ui.sbBeepVol,SIGNAL(valueChanged(int)),ui.sldBeepVol,SLOT(setValue(int)));
	connect(ui.sbTapeVol,SIGNAL(valueChanged(int)),ui.sldTapeVol,SLOT(setValue(int)));
	connect(ui.sbAYVol,SIGNAL(valueChanged(int)),ui.sldAYVol,SLOT(setValue(int)));
	connect(ui.sbGSVol,SIGNAL(valueChanged(int)),ui.sldGSVol,SLOT(setValue(int)));
	connect(ui.sbSdrvVol,SIGNAL(valueChanged(int)),ui.sldSdrvVol,SLOT(setValue(int)));
	connect(ui.sbSAAVol,SIGNAL(valueChanged(int)),ui.sldSAAVol,SLOT(setValue(int)));

// dos
	connect(ui.newatb,SIGNAL(released()),this,SLOT(newa()));
	connect(ui.newbtb,SIGNAL(released()),this,SLOT(newb()));
	connect(ui.newctb,SIGNAL(released()),this,SLOT(newc()));
	connect(ui.newdtb,SIGNAL(released()),this,SLOT(newd()));

	connect(ui.loadatb,SIGNAL(released()),this,SLOT(loada()));
	connect(ui.loadbtb,SIGNAL(released()),this,SLOT(loadb()));
	connect(ui.loadctb,SIGNAL(released()),this,SLOT(loadc()));
	connect(ui.loaddtb,SIGNAL(released()),this,SLOT(loadd()));

	connect(ui.saveatb,SIGNAL(released()),this,SLOT(savea()));
	connect(ui.savebtb,SIGNAL(released()),this,SLOT(saveb()));
	connect(ui.savectb,SIGNAL(released()),this,SLOT(savec()));
	connect(ui.savedtb,SIGNAL(released()),this,SLOT(saved()));

	connect(ui.remoatb,SIGNAL(released()),this,SLOT(ejcta()));
	connect(ui.remobtb,SIGNAL(released()),this,SLOT(ejctb()));
	connect(ui.remoctb,SIGNAL(released()),this,SLOT(ejctc()));
	connect(ui.remodtb,SIGNAL(released()),this,SLOT(ejctd()));

	connect(ui.disktabs,SIGNAL(currentChanged(int)),this,SLOT(fillDiskCat()));
	connect(ui.actCopyToTape,SIGNAL(triggered()),this,SLOT(copyToTape()));
	connect(ui.actSaveHobeta,SIGNAL(triggered()),this,SLOT(diskToHobeta()));
	connect(ui.actSaveRaw,SIGNAL(triggered()),this,SLOT(diskToRaw()));
	connect(ui.tbToTape,SIGNAL(released()),this,SLOT(copyToTape()));
	connect(ui.tbToHobeta,SIGNAL(released()),this,SLOT(diskToHobeta()));
	connect(ui.tbToRaw,SIGNAL(released()),this,SLOT(diskToRaw()));
// tape
	connect(ui.tapelist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chablock(QModelIndex)));
	connect(ui.tapelist,SIGNAL(clicked(QModelIndex)),this,SLOT(tlistclick(QModelIndex)));
	connect(ui.tloadtb,SIGNAL(released()),this,SLOT(loatape()));
	connect(ui.tsavetb,SIGNAL(released()),this,SLOT(savtape()));
	connect(ui.tremotb,SIGNAL(released()),this,SLOT(ejctape()));
	connect(ui.blkuptb,SIGNAL(released()),this,SLOT(tblkup()));
	connect(ui.blkdntb,SIGNAL(released()),this,SLOT(tblkdn()));
	connect(ui.blkrmtb,SIGNAL(released()),this,SLOT(tblkrm()));
	connect(ui.actCopyToDisk,SIGNAL(triggered()),this,SLOT(copyToDisk()));
	connect(ui.tbToDisk,SIGNAL(released()),this,SLOT(copyToDisk()));
// hdd
	connect(ui.hm_pathtb,SIGNAL(released()),this,SLOT(hddMasterImg()));
	connect(ui.hs_pathtb,SIGNAL(released()),this,SLOT(hddSlaveImg()));
// external
	connect(ui.tbSDCimg,SIGNAL(released()),this,SLOT(selSDCimg()));
	connect(ui.tbsdcfree,SIGNAL(released()),ui.sdPath,SLOT(clear()));
	connect(ui.cSlotOpen,SIGNAL(released()),this,SLOT(openSlot()));
	connect(ui.cSlotEject,SIGNAL(released()),this,SLOT(ejectSlot()));
// input
	connect(ui.tbPadNew, SIGNAL(released()),this,SLOT(newPadMap()));
	connect(ui.tbPadDelete,SIGNAL(released()),this,SLOT(delPadMap()));
	connect(ui.cbPadMap, SIGNAL(currentIndexChanged(int)),this,SLOT(chaPadMap(int)));
	connect(ui.tbAddBind,SIGNAL(clicked(bool)),this, SLOT(addBinding()));
	connect(ui.tbEditBind,SIGNAL(clicked(bool)),this,SLOT(editBinding()));
	connect(ui.tbDelBind,SIGNAL(clicked(bool)),this,SLOT(delBinding()));
	connect(ui.actAddBinding,SIGNAL(triggered()),this,SLOT(addBinding()));
	connect(ui.actEditBinding,SIGNAL(triggered()),this,SLOT(editBinding()));
	connect(ui.tvPadTable,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editBinding()));
	connect(ui.actDelBinding,SIGNAL(triggered()),this,SLOT(delBinding()));
	connect(padial, SIGNAL(bindReady(xJoyMapEntry)), this, SLOT(bindAccept(xJoyMapEntry)));
	connect(ui.cbGamepad, SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentGamepad(int)));
//tools
	connect(ui.umlist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(umedit(QModelIndex)));
	connect(ui.umaddtb,SIGNAL(released()),this,SLOT(umadd()));
	connect(ui.umdeltb,SIGNAL(released()),this,SLOT(umdel()));
	connect(ui.umuptb,SIGNAL(released()),this,SLOT(umup()));
	connect(ui.umdntb,SIGNAL(released()),this,SLOT(umdn()));
// bookmark add dialog
	connect(uia.umasptb,SIGNAL(released()),this,SLOT(umaselp()));
	connect(uia.umaok,SIGNAL(released()),this,SLOT(umaconf()));
	connect(uia.umacn,SIGNAL(released()),umadial,SLOT(hide()));
// debuga
	connect(ui.tbDbgFont,SIGNAL(released()),this,SLOT(selectDbgFont()));
// palette
	QToolButton* tbarr[] = {
		ui.tbDbgChaBG, ui.tbDbgChaFG, ui.tbDbgHeadBG, ui.tbDbgHeadFG,
		ui.tbDbgTxtCol, ui.tbDbgWinCol, ui.tbDbgInputBG, ui.tbDbgInputFG,
		ui.tbDbgTableBG, ui.tbDbgTableFG, ui.tbDbgPcBG, ui.tbDbgPcFG,
		ui.tbDbgSelBG, ui.tbDbgSelFG,
		ui.tbDbgBrkFG,
		NULL
	};
	i = 0;
	while (tbarr[i] != NULL) {
		connect(tbarr[i], SIGNAL(released()), this, SLOT(selectColor()));
		connect(tbarr[i], SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(triggerColor()));
		i++;
	}
// profiles manager
	connect(ui.tbNewProfile,SIGNAL(released()),this,SLOT(newProfile()));
	connect(ui.tbCopyProfile,SIGNAL(released()),this,SLOT(copyProf()));
	connect(ui.tbDelProfile,SIGNAL(released()),this,SLOT(rmProfile()));
	connect(ui.twProfileList,SIGNAL(cellDoubleClicked(int, int)),this,SLOT(chProfile(int, int)));
}

void SetupWin::okay() {
	apply();
	reject();
}

void setToolButtonColor(QToolButton* tb, QString nm, QString dc) {
	QColor col = conf.pal[nm];
	if (!col.isValid()) col = dc;
	if (!col.isValid()) col = QColor(0,0,0,0);	// transparent
	QPixmap pxm(16,16);
	pxm.fill(col);
	tb->setIcon(QIcon(pxm));
	tb->setProperty("colorName", nm);
	tb->setProperty("defaultColor", dc);
}

void SetupWin::setPadName() {
//	ui.lePadName->setText(conf.joy.gpad->name());
}

void SetupWin::start() {
	xProfile* prof = conf.prof.cur;
	Computer* comp = prof->zx;
// machine
	int idx;
	fill_romset_list(ui.rsetbox);
	ui.machbox->setCurrentIndex(ui.machbox->findData(QString::fromUtf8(comp->hw->name)));
	ui.rsetbox->setCurrentIndex(ui.rsetbox->findText(QString::fromUtf8(prof->rsName.c_str())));
	ui.resbox->setCurrentIndex(ui.resbox->findData(comp->resbank));
	setmszbox(ui.machbox->currentIndex());
	ui.mszbox->setCurrentIndex(ui.mszbox->findData(comp->mem->ramSize));
	if (ui.mszbox->currentIndex() < 0) ui.mszbox->setCurrentIndex(ui.mszbox->count() - 1);
	ui.cbCpu->setCurrentIndex(ui.cbCpu->findData(comp->cpu->type));
	ui.sbFreq->setValue(comp->cpuFrq);
	ui.sbMult->setValue(comp->frqMul);
	ui.scrpwait->setChecked(comp->evenM1);
// video
	ui.cbFullscreen->setChecked(conf.vid.fullScreen);
	ui.cbKeepRatio->setChecked(conf.vid.keepRatio);
	ui.sbScale->setValue(conf.vid.scale);
	ui.sldNoflic->setValue(noflic); chaflc();
	ui.grayscale->setChecked(greyScale);
	ui.cbScanlines->setChecked(scanlines);
	ui.border4T->setChecked(comp->vid->brdstep & 0x06);
	ui.contMem->setChecked(comp->contMem);
	ui.contIO->setChecked(comp->contIO);
	ui.bszsld->setValue(static_cast<int>(conf.brdsize * 100));
	ui.pathle->setText(QString::fromLocal8Bit(conf.scrShot.dir.c_str()));
	ui.ssfbox->setCurrentIndex(ui.ssfbox->findText(conf.scrShot.format.c_str()));
	ui.scntbox->setValue(conf.scrShot.count);
	ui.sintbox->setValue(conf.scrShot.interval);
	ui.ssNoLeds->setChecked(conf.scrShot.noLeds);
	ui.ssNoBord->setChecked(conf.scrShot.noBorder);
	ui.geombox->clear();
	foreach(xLayout lay, conf.layList) {
		ui.geombox->addItem(QString::fromLocal8Bit(lay.name.c_str()));
	}
	ui.geombox->setCurrentIndex(ui.geombox->findText(QString::fromLocal8Bit(conf.prof.cur->layName.c_str())));
	ui.ulaPlus->setChecked(comp->vid->ula->enabled);
	ui.cbDDp->setChecked(comp->ddpal);
	fill_shader_list(ui.cbShader);
// sound
	ui.cbGS->setChecked(comp->gs->enable);
	ui.gsrbox->setChecked(comp->gs->reset);

	ui.sdrvBox->setCurrentIndex(ui.sdrvBox->findData(comp->sdrv->type));

	ui.cbSAA->setChecked(comp->saa->enabled);

	ui.senbox->setChecked(conf.snd.enabled);
	ui.outbox->setCurrentIndex(ui.outbox->findText(QString::fromLocal8Bit(sndOutput->name)));
	ui.ratbox->setCurrentIndex(ui.ratbox->findData(QVariant(conf.snd.rate)));

	ui.sbMasterVol->setValue(conf.snd.vol.master);
	ui.sbBeepVol->setValue(conf.snd.vol.beep);
	ui.sbTapeVol->setValue(conf.snd.vol.tape);
	ui.sbAYVol->setValue(conf.snd.vol.ay);
	ui.sbGSVol->setValue(conf.snd.vol.gs);
	ui.sbSdrvVol->setValue(conf.snd.vol.sdrv);
	ui.sbSAAVol->setValue(conf.snd.vol.saa);

	ui.schip1box->setCurrentIndex(ui.schip1box->findData(QVariant(comp->ts->chipA->type)));
	ui.schip2box->setCurrentIndex(ui.schip2box->findData(QVariant(comp->ts->chipB->type)));
	ui.schip3box->setCurrentIndex(ui.schip3box->findData(QVariant(comp->ts->chipC->type)));
	ui.stereo1box->setCurrentIndex(ui.stereo1box->findData(QVariant(comp->ts->chipA->stereo)));
	ui.stereo2box->setCurrentIndex(ui.stereo2box->findData(QVariant(comp->ts->chipB->stereo)));
	ui.stereo3box->setCurrentIndex(ui.stereo3box->findData(QVariant(comp->ts->chipC->stereo)));
	ui.psg1frq->setCurrentText(QString::number(comp->ts->chipA->frq));
	ui.psg2frq->setCurrentText(QString::number(comp->ts->chipB->frq));
	ui.psg3frq->setCurrentText(QString::number(comp->ts->chipC->frq));
	ui.tsbox->setCurrentIndex(ui.tsbox->findData(QVariant(comp->ts->type)));
// input
	buildkeylist();
	setRFIndex(ui.cbScanTab, comp->keyb->pcmode);
	setRFIndex(ui.cbMouseType, comp->mouse->pcmode);
	idx = ui.keyMapBox->findText(QString(prof->kmapName.c_str()));
	if (idx < 1) idx = 0;
	ui.keyMapBox->setCurrentIndex(idx);
	ui.ratEnable->setChecked(comp->mouse->enable);
	ui.ratWheel->setChecked(comp->mouse->hasWheel);
	ui.cbSwapButtons->setChecked(comp->mouse->swapButtons);
	ui.sldSensitivity->setValue(comp->mouse->sensitivity * 1000.0f);
	ui.cbKbuttons->setChecked(comp->joy->extbuttons);
	ui.sldDeadZone->setValue(conf.joy.dead);
	ui.cbGamepad->blockSignals(true);
	fillRFBox(ui.cbGamepad, conf.joy.gpad->getList());
	setRFIndex(ui.cbGamepad, conf.joy.curName);
	ui.cbGamepad->blockSignals(false);
	padModel->update();
// dos
	ui.diskTypeBox->setCurrentIndex(ui.diskTypeBox->findData(comp->dif->type));
	ui.bdtbox->setChecked(fdcFlag & FDC_FAST);
	ui.mempaths->setChecked(conf.storePaths);
	ui.cbAddBoot->setChecked(conf.boot);
	Floppy* flp = comp->dif->fdc->flop[0];
	ui.apathle->setText(QString::fromLocal8Bit(flp->path));
		ui.a80box->setChecked(flp->trk80);
		ui.adsbox->setChecked(flp->doubleSide);
		ui.awpbox->setChecked(flp->protect);
	flp = comp->dif->fdc->flop[1];
	ui.bpathle->setText(QString::fromLocal8Bit(flp->path));
		ui.b80box->setChecked(flp->trk80);
		ui.bdsbox->setChecked(flp->doubleSide);
		ui.bwpbox->setChecked(flp->protect);
	flp = comp->dif->fdc->flop[2];
	ui.cpathle->setText(QString::fromLocal8Bit(flp->path));
		ui.c80box->setChecked(flp->trk80);
		ui.cdsbox->setChecked(flp->doubleSide);
		ui.cwpbox->setChecked(flp->protect);
	flp = comp->dif->fdc->flop[3];
	ui.dpathle->setText(QString::fromLocal8Bit(flp->path));
		ui.d80box->setChecked(flp->trk80);
		ui.ddsbox->setChecked(flp->doubleSide);
		ui.dwpbox->setChecked(flp->protect);
	fillDiskCat();
// hdd
	ui.hiface->setCurrentIndex(ui.hiface->findData(comp->ide->type));

	ui.hm_type->setCurrentIndex(ui.hm_type->findData(comp->ide->master->type));
	ATAPassport pass = ideGetPassport(comp->ide,IDE_MASTER);
	ui.hm_path->setText(QString::fromLocal8Bit(comp->ide->master->image));
	ui.hm_islba->setChecked(comp->ide->master->hasLBA);
	ui.hm_gsec->setValue(pass.spt);
	ui.hm_ghd->setValue(pass.hds);
	ui.hm_gcyl->setValue(pass.cyls);
	ui.hm_glba->setValue(comp->ide->master->maxlba);
	ui.hm_capacity->setValue(comp->ide->master->maxlba >> 11);

	ui.hs_type->setCurrentIndex(ui.hm_type->findData(comp->ide->slave->type));
	pass = ideGetPassport(comp->ide,IDE_SLAVE);
	ui.hs_path->setText(QString::fromLocal8Bit(comp->ide->slave->image));
	ui.hs_islba->setChecked(comp->ide->slave->hasLBA);
	ui.hs_gsec->setValue(pass.spt);
	ui.hs_ghd->setValue(pass.hds);
	ui.hs_gcyl->setValue(pass.cyls);
	ui.hs_glba->setValue(comp->ide->slave->maxlba);
	ui.hs_capacity->setValue(comp->ide->slave->maxlba >> 11);
// external
	ui.sdPath->setText(QString::fromLocal8Bit(comp->sdc->image));
//	ui.sdcapbox->setCurrentIndex(ui.sdcapbox->findData(comp->sdc->capacity));
//	if (ui.sdcapbox->currentIndex() < 0) ui.sdcapbox->setCurrentIndex(2);	// 128M
	ui.sdlock->setChecked(comp->sdc->lock);

	ui.cSlotName->setText(comp->slot->name);
	setRFIndex(ui.cSlotType, comp->slot->mapType);
// tape
	ui.cbTapeAuto->setChecked(conf.tape.autostart);
	ui.cbTapeFast->setChecked(conf.tape.fast);
	ui.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
// input
	buildpadlist();
	setRFIndex(ui.cbPadMap, conf.prof.cur->jmapName.c_str());
// tools
	ui.sbPort->setValue(conf.port);
	ui.cbConfexit->setChecked(conf.confexit);
	buildmenulist();
// leds
	ui.cbMouseLed->setChecked(conf.led.mouse);
	ui.cbJoyLed->setChecked(conf.led.joy);
	ui.cbKeysLed->setChecked(conf.led.keys);
	ui.cbTapeLed->setChecked(conf.led.tape);
	ui.cbDiskLed->setChecked(conf.led.disk);
	ui.cbMessage->setChecked(conf.led.message);
	ui.cbFpsLed->setChecked(conf.led.fps);
	ui.cbHaltLed->setChecked(conf.led.halt);
// debuga
	ui.sbDbSize->setValue(conf.dbg.dbsize);
	ui.sbDwSize->setValue(conf.dbg.dwsize);
	ui.sbTextSize->setValue(conf.dbg.dmsize);
	dbgfnt = conf.dbg.font;
	ui.leDbgFont->setText(QString("%0, %1 pt").arg(dbgfnt.family()).arg(dbgfnt.pointSize()));
	ui.leDbgFont->setFont(dbgfnt);
// palette
	setToolButtonColor(ui.tbDbgWinCol, "dbg.window","");
	setToolButtonColor(ui.tbDbgTxtCol, "dbg.text","");
	setToolButtonColor(ui.tbDbgChaBG, "dbg.changed.bg","#e0c0c0");
	setToolButtonColor(ui.tbDbgChaFG, "dbg.changed.txt","#000000");
	setToolButtonColor(ui.tbDbgHeadBG, "dbg.header.bg","#c0c0e0");
	setToolButtonColor(ui.tbDbgHeadFG, "dbg.header.txt","#000000");
	setToolButtonColor(ui.tbDbgInputBG, "dbg.input.bg","");
	setToolButtonColor(ui.tbDbgInputFG, "dbg.input.txt","");
	setToolButtonColor(ui.tbDbgTableBG, "dbg.table.bg","");
	setToolButtonColor(ui.tbDbgTableFG, "dbg.table.txt","");
	setToolButtonColor(ui.tbDbgPcBG, "dbg.pc.bg","#80e080");
	setToolButtonColor(ui.tbDbgPcFG, "dbg.pc.txt","#000000");
	setToolButtonColor(ui.tbDbgSelBG, "dbg.sel.bg","#c0e0c0");
	setToolButtonColor(ui.tbDbgSelFG, "dbg.sel.txt","#000000");
	setToolButtonColor(ui.tbDbgBrkFG, "dbg.brk.txt","#e08080");
// profiles
	ui.defstart->setChecked(conf.defProfile);
	buildproflist();

	show();
}

void SetupWin::apply() {
	xProfile* prof = conf.prof.cur;
	Computer* comp = prof->zx;
// machine
	HardWare *oldmac = comp->hw;
	std::string new_hwname = std::string(getRFSData(ui.machbox).toUtf8().data());
	if (prof->hwName != new_hwname) {
		prof->hwName = new_hwname;
		compSetHardware(prof->zx,prof->hwName.c_str());
	}
	prof->rsName = getRFText(ui.rsetbox);
	prfSetRomset(prof, prof->rsName);
	comp->resbank = getRFIData(ui.resbox);
	memSetSize(comp->mem, getRFIData(ui.mszbox), -1);
	cpuSetType(comp->cpu, getRFIData(ui.cbCpu));
	compSetBaseFrq(comp, ui.sbFreq->value());
	compSetTurbo(comp, ui.sbMult->value());
	comp->evenM1 = ui.scrpwait->isChecked() ? 1 : 0;
	if (comp->hw != oldmac) compReset(comp,RES_DEFAULT);
	if (comp->hw->id == HW_ZX48) comp->mem->ramMask = MEM_128K - 1;		// TODO: find a better way
// video
	conf.vid.fullScreen = ui.cbFullscreen->isChecked() ? 1 : 0;
	conf.vid.keepRatio = ui.cbKeepRatio->isChecked() ? 1 : 0;
	conf.vid.scale = ui.sbScale->value();
	noflic = ui.sldNoflic->value();
	vid_set_grey(ui.grayscale->isChecked() ? 1 : 0);
	scanlines = ui.cbScanlines->isChecked() ? 1 : 0;
	conf.scrShot.dir = std::string(ui.pathle->text().toLocal8Bit().data());
	conf.scrShot.format = getRFText(ui.ssfbox);
	conf.scrShot.count = ui.scntbox->value();
	conf.scrShot.interval = ui.sintbox->value();
	conf.scrShot.noLeds = ui.ssNoLeds->isChecked() ? 1 : 0;
	conf.scrShot.noBorder = ui.ssNoBord->isChecked() ? 1 : 0;
	conf.brdsize = ui.bszsld->value()/100.0;
	comp->vid->brdstep = ui.border4T->isChecked() ? 7 : 1;
	comp->contMem = ui.contMem->isChecked() ? 1 : 0;
	comp->contIO = ui.contIO->isChecked() ? 1 : 0;
	comp->vid->ula->enabled = ui.ulaPlus->isChecked() ? 1 : 0;
	comp->ddpal = ui.cbDDp->isChecked() ? 1 : 0;
	prfSetLayout(NULL, getRFText(ui.geombox));
	if (getRFIData(ui.cbShader) == 0) {
		conf.vid.shader.clear();
	} else {
		conf.vid.shader = std::string(ui.cbShader->currentText().toLocal8Bit().data());
	}
// sound
	conf.snd.enabled = ui.senbox->isChecked() ? 1 : 0;

	conf.snd.vol.master = ui.sbMasterVol->value();
	conf.snd.vol.beep = ui.sbBeepVol->value();
	conf.snd.vol.tape = ui.sbTapeVol->value();
	conf.snd.vol.ay = ui.sbAYVol->value();
	conf.snd.vol.gs = ui.sbGSVol->value();
	conf.snd.vol.sdrv = ui.sbSdrvVol->value();
	conf.snd.vol.saa = ui.sbSAAVol->value();

	std::string nname = getRFText(ui.outbox);
	int rate = getRFIData(ui.ratbox);
	if ((rate != conf.snd.rate) || (nname != sndGetName())) {
		conf.snd.rate = rate;
		setOutput(nname.c_str());
	}

	comp->ts->chipA->frq = ui.psg1frq->currentText().toDouble();
	comp->ts->chipB->frq = ui.psg2frq->currentText().toDouble();
	comp->ts->chipC->frq = ui.psg3frq->currentText().toDouble();
	chip_set_type(comp->ts->chipA, getRFIData(ui.schip1box));
	chip_set_type(comp->ts->chipB, getRFIData(ui.schip2box));
	chip_set_type(comp->ts->chipC, getRFIData(ui.schip3box));
	comp->ts->chipA->stereo = getRFIData(ui.stereo1box);
	comp->ts->chipB->stereo = getRFIData(ui.stereo2box);
	comp->ts->chipC->stereo = getRFIData(ui.stereo3box);
	comp->ts->type = getRFIData(ui.tsbox);

	comp->gs->enable = ui.cbGS->isChecked() ? 1 : 0;
	comp->gs->reset = ui.gsrbox->isChecked() ? 1 : 0;

	comp->sdrv->type = getRFIData(ui.sdrvBox);

	comp->saa->enabled = ui.cbSAA->isChecked() ? 1 : 0;
// input
	comp->keyb->pcmode = getRFIData(ui.cbScanTab);
	comp->mouse->pcmode = getRFIData(ui.cbMouseType);
	comp->mouse->enable = ui.ratEnable->isChecked() ? 1 : 0;
	comp->mouse->hasWheel = ui.ratWheel->isChecked() ? 1 : 0;
	comp->mouse->swapButtons = ui.cbSwapButtons->isChecked() ? 1 : 0;
	comp->mouse->sensitivity = ui.sldSensitivity->value() * 0.001f;
	comp->joy->extbuttons = ui.cbKbuttons->isChecked() ? 1 : 0;
	conf.joy.dead = ui.sldDeadZone->value();
	conf.joy.deadf = conf.joy.dead / 32768.0;
	// conf.joy.idx = ui.cbGamepad->currentIndex();
	conf.joy.curName = getRFSData(ui.cbGamepad);
	std::string kmname = getRFText(ui.keyMapBox);
	if (kmname == "none") kmname = "default";
	prof->kmapName = kmname;
	loadKeys();
// bdi
	difSetHW(comp->dif, getRFIData(ui.diskTypeBox));
	setFlagBit(ui.bdtbox->isChecked(),&fdcFlag,FDC_FAST);
	conf.boot = ui.cbAddBoot->isChecked() ? 1 : 0;
	conf.storePaths = ui.mempaths->isChecked() ? 1 : 0;

	Floppy* flp = comp->dif->fdc->flop[0];
	flp->trk80 = ui.a80box->isChecked() ? 1 : 0;
	flp->doubleSide = ui.adsbox->isChecked() ? 1 : 0;
	flp->protect = ui.awpbox->isChecked() ? 1 : 0;

	flp = comp->dif->fdc->flop[1];
	flp->trk80 = ui.b80box->isChecked() ? 1 : 0;
	flp->doubleSide = ui.bdsbox->isChecked() ? 1 : 0;
	flp->protect = ui.bwpbox->isChecked() ? 1 : 0;

	flp = comp->dif->fdc->flop[2];
	flp->trk80 = ui.c80box->isChecked() ? 1 : 0;
	flp->doubleSide = ui.cdsbox->isChecked() ? 1 : 0;
	flp->protect = ui.cwpbox->isChecked() ? 1 : 0;

	flp = comp->dif->fdc->flop[3];
	flp->trk80 = ui.d80box->isChecked() ? 1 : 0;
	flp->doubleSide = ui.ddsbox->isChecked() ? 1 : 0;
	flp->protect = ui.dwpbox->isChecked() ? 1 : 0;

// hdd
	comp->ide->type = getRFIData(ui.hiface);

	comp->ide->master->type = getRFIData(ui.hm_type);
	ideSetImage(comp->ide,IDE_MASTER,ui.hm_path->text().toLocal8Bit().data());
	comp->ide->master->hasLBA = ui.hm_islba->isChecked() ? 1 : 0;

	comp->ide->slave->type = getRFIData(ui.hs_type);
	ideSetImage(comp->ide,IDE_SLAVE,ui.hs_path->text().toLocal8Bit().data());
	comp->ide->slave->hasLBA = ui.hs_islba->isChecked() ? 1 : 0;
// others
	sdcSetImage(comp->sdc,ui.sdPath->text().isEmpty() ? "" : ui.sdPath->text().toLocal8Bit().data());
	comp->sdc->lock = ui.sdlock->isChecked() ? 1 : 0;

	comp->slot->mapType = getRFIData(ui.cSlotType);
// tape
	conf.tape.autostart = ui.cbTapeAuto->isChecked() ? 1 : 0;
	conf.tape.fast = ui.cbTapeFast->isChecked() ? 1 : 0;
// input
	conf.prof.cur->jmapName = getRFSData(ui.cbPadMap).toStdString();
// tools
	conf.port = ui.sbPort->value() & 0xffff;
	conf.confexit = ui.cbConfexit->isChecked() ? 1 : 0;
// leds
	conf.led.mouse = ui.cbMouseLed->isChecked() ? 1 : 0;
	conf.led.joy = ui.cbJoyLed->isChecked() ? 1 : 0;
	conf.led.keys = ui.cbKeysLed->isChecked() ? 1 : 0;
	conf.led.tape = ui.cbTapeLed->isChecked() ? 1 : 0;
	conf.led.disk = ui.cbDiskLed->isChecked() ? 1 : 0;
	conf.led.message = ui.cbMessage->isChecked() ? 1 : 0;
	conf.led.fps = ui.cbFpsLed->isChecked() ? 1 : 0;
	conf.led.halt = ui.cbHaltLed->isChecked() ? 1 : 0;
// debuga
	conf.dbg.dbsize = ui.sbDbSize->value();
	conf.dbg.dwsize = ui.sbDwSize->value();
	conf.dbg.dmsize = ui.sbTextSize->value();
	conf.dbg.font = dbgfnt;
// profiles
	conf.defProfile = ui.defstart->isChecked() ? 1 : 0;

	emit s_apply();

	saveConfig();
	prfSave("");
}

void SetupWin::reject() {
	hide();
	emit closed();
}

// LAYOUTS

void SetupWin::layNameCheck(QString nam) {
	layUi.okButton->setEnabled(!layUi.layName->text().isEmpty());
/*
	for (int i = 0; i < conf.layList.size(); i++) {
		if ((QString(conf.layList[i].name.c_str()) == nam) && (eidx != i)) {
			layUi.okButton->setEnabled(false);
		}
	}
*/
}

void SetupWin::editLayout() {
	layUi.lineBox->setValue(nlay.lay.full.x);
	layUi.rowsBox->setValue(nlay.lay.full.y);
	layUi.hsyncBox->setValue(nlay.lay.blank.x);
	layUi.vsyncBox->setValue(nlay.lay.blank.y);
	layUi.brdLBox->setValue(nlay.lay.bord.x);
	layUi.brdUBox->setValue(nlay.lay.bord.y);
	layUi.intRowBox->setValue(nlay.lay.intpos.y);
	layUi.intPosBox->setValue(nlay.lay.intpos.x);
	layUi.intLenBox->setValue(nlay.lay.intSize);
	layUi.sbScrW->setValue(nlay.lay.scr.x);
	layUi.sbScrH->setValue(nlay.lay.scr.y);
	layUi.okButton->setDisabled(eidx == 0);
	layUi.layWidget->setDisabled(eidx == 0);
	layUi.layName->setText(nlay.name.c_str());
	layeditor->show();
	layeditor->setFixedSize(layeditor->minimumSize());
}

void SetupWin::edLayout() {
	eidx = ui.geombox->currentIndex();
	nlay = conf.layList[eidx];
	editLayout();
}

void SetupWin::delLayout() {
	int eidx = ui.geombox->currentIndex();
	if (eidx < 1) {
		shitHappens("You can't delete this layout");
		return;
	}
	if (areSure("Do you really want to delete this layout?")) {
		conf.layList.erase(conf.layList.begin() + eidx);
		ui.geombox->removeItem(eidx);
	}
}

void SetupWin::addNewLayout() {
	eidx = -1;
	nlay = conf.layList[0];
	nlay.name = "";
	editLayout();
}

void SetupWin::layEditorChanged() {
	layUi.showField->setFixedSize(layUi.lineBox->value(),layUi.rowsBox->value());
	QPixmap pix(layUi.lineBox->value(),layUi.rowsBox->value());
	QPainter pnt;
	pnt.begin(&pix);
	pnt.fillRect(0,0,pix.width(),pix.height(),Qt::black);
	// visible screen = full - blank
	pnt.fillRect(layUi.hsyncBox->value(),layUi.vsyncBox->value(),
			layUi.lineBox->value() - layUi.hsyncBox->value(),
			layUi.rowsBox->value() - layUi.vsyncBox->value(),
			Qt::blue);
	// main screen area
	pnt.fillRect(layUi.brdLBox->value()+layUi.hsyncBox->value(), layUi.brdUBox->value()+layUi.vsyncBox->value(),
			layUi.sbScrW->value(),layUi.sbScrH->value(),
			Qt::gray);
	// INT signal
	pnt.setPen(Qt::red);
	pnt.drawLine(layUi.intPosBox->value(),
			layUi.intRowBox->value(),
			layUi.intPosBox->value() + layUi.intLenBox->value(),
			layUi.intRowBox->value());
	pnt.end();
	layUi.showField->setPixmap(pix);
	layeditor->setFixedSize(layeditor->minimumSize());
}

void SetupWin::layEditorOK() {
	QString nm = layUi.layName->text();
	std::string name = std::string(nm.toLocal8Bit().data());
	xLayout* exlay = findLayout(name);
	vLayout vlay;
	int ok = 1;
	vlay.full.x = layUi.lineBox->value();
	vlay.full.y = layUi.rowsBox->value();
	vlay.bord.x = layUi.brdLBox->value();
	vlay.bord.y = layUi.brdUBox->value();
	vlay.blank.x = layUi.hsyncBox->value();
	vlay.blank.y = layUi.vsyncBox->value();
	vlay.intpos.x = layUi.intPosBox->value();
	vlay.intpos.y = layUi.intRowBox->value();
	vlay.intSize = layUi.intLenBox->value();
	vlay.scr.x = layUi.sbScrW->value();
	vlay.scr.y = layUi.sbScrH->value();
	if (eidx < 0) {						// new layout
		if (nm == "default") {				// protected
			showInfo("'default' layout cannot be changed");
			ok = 0;
		} else if (exlay == NULL) {			// new name
			addLayout(name, vlay);
			fill_layout_list(ui.geombox, nm);
		} else {					// existing name
			ok = areSure("Replace existing layout?");
			if (ok) exlay->lay = vlay;
			fill_layout_list(ui.geombox, nm);
		}
	} else if (eidx > 0) {					// ==0 is 'default', not editable
		std::string onm = conf.layList[eidx].name;
		if (onm != name) {				// name changed
			if (exlay == NULL) {			// no existing layout with new name
				conf.layList[eidx].name = name;
				conf.layList[eidx].lay = vlay;
				prfChangeLayName(conf.layList[eidx].name, nlay.name);
				fill_layout_list(ui.geombox, nm);
			} else {
				ok = areSure("Replace existing layout?");
				if (ok) {
					prfChangeLayName(conf.layList[eidx].name, nlay.name);
					exlay->lay = vlay;		// replace new-name layout
					rmLayout(onm);
					fill_layout_list(ui.geombox, nm);
				}
			}
		} else {					// name doesn't changed, replace old layout
			conf.layList[eidx].lay = vlay;
		}
	}
	if (ok) layeditor->hide();
}

// ROMSETS

typedef struct {
	int hwid;
	std::string gsf;
	std::string fnf;
	xRomFile lst[8];
} xRomPreset;

static xRomPreset presets[] = {
	{HW_ZX48, "gs105a.rom", "", {{"1982.rom",0,0,0},{"trdos503.rom",0,0,16},{"",0,0,0}}},
	{HW_PENT, "gs105a.rom", "", {{"pentagon.rom",0,0,0},{"trdos503.rom",0,0,48},{"",0,0,0}}},
	{HW_P1024, "gs105a.rom", "", {{"glukpen.rom",0,0,0},{"",0,0,0}}},
	{HW_SCORP, "gs105a.rom", "", {{"scorpion.rom",0,0,0},{"",0,0,0}}},
	{HW_PLUS2, "", "", {{"plus2a.rom",0,0,0},{"",0,0,0}}},
	{HW_PLUS3, "", "", {{"plus3-41.rom",0,0,0},{"",0,0,0}}},
	{HW_ATM2, "gs105a.rom", "SGEN.ROM", {{"xbios135.rom",0,0,0},{"",0,0,0}}},
	{HW_PENTEVO, "gs105a.rom", "SGEN.ROM", {{"zxevo.rom",0,0,0},{"",0,0,0}}},
	{HW_TSLAB, "gs105a.rom", "", {{"ts-bios.rom",0,0,0},{"",0,0,0}}},
	{HW_PROFI, "gs105a.rom", "", {{"PROFI-P.ROM",0,0,0},{"",0,0,0}}},
	{HW_PHOENIX, "gs105a.rom", "", {{"zxm_bios_5_03.rom",0,0,0},{"",0,0,0}}},
	{HW_MSX, "", "", {{"MSX.ROM",0,0,0},{"",0,0,0}}},
	{HW_MSX2, "", "", {{"msx2.rom",0,0,0},{"",0,0,0}}},
	{HW_GBC, "", "", {{"GameBoyColorBIOS.rom",0,0,0},{"",0,0,0}}},
	{HW_NES, "", "", {{"",0,0,0}}},
	{HW_C64, "", "c64charset.rom", {{"commodore64.rom",0,0,0},{"",0,0,0}}},
	{HW_BK0010, "", "", {{"MONIT10.ROM",0,0,0},{"BASIC10.ROM",0,0,8},{"",0,0,0}}},
	{HW_BK0011M, "", "", {{"BAS11M_0.ROM",0,0,0},{"BAS11M_1.ROM",0,0,16},{"B11M_EXT.ROM",0,0,24},{"B11M_BOS.ROM",0,0,32},{"",0,0,0}}},
	{HW_SPCLST, "", "", {{"specialist_boot2_1.rom",0,0,0},{"specialist_monitor2_2.rom",0,0,2},{"",0,0,0}}},
	{HW_NULL, "", "", {{"",0,0,0}}}
};

void SetupWin::romPreset() {
	int idx = ui.rsetbox->currentIndex();
	if (idx < 0) return;
	QString hwn = getRFSData(ui.machbox);
	HardWare* hw = findHardware(hwn.toLocal8Bit().data());
	if (!hw) return;
	int i = 0;
	while ((presets[i].hwid != HW_NULL) && (presets[i].hwid != hw->id))
		i++;
	if (presets[i].hwid == HW_NULL) return;
	xRomset rs = conf.rsList[idx];
	rs.gsFile = presets[i].gsf;
	rs.fntFile = presets[i].fnf;
	rs.vBiosFile.clear();
	rs.roms.clear();
	int dx = 0;
	while (presets[i].lst[dx].name != "") {
		rs.roms.push_back(presets[i].lst[dx]);
		dx++;
	}
	conf.rsList[idx] = rs;
	rsmodel->fill(&conf.rsList[idx]);
}

void SetupWin::rmRomset() {
	int idx = ui.rsetbox->currentIndex();
	if (idx < 0) return;
	if (areSure("Do you really want to delete this romset?")) {
		delRomset(idx);
		ui.rsetbox->removeItem(idx);
	}
}

void SetupWin::addNewRomset() {
	QString nam = QInputDialog::getText(this, "Enter name", "Romset name");
	if (nam.isEmpty()) return;
	xRomset r;
	r.name = std::string(nam.toLocal8Bit().data());
	r.gsFile.clear();
	r.fntFile.clear();
	r.roms.clear();
	if (addRomset(r)) {
		fill_romset_list(ui.rsetbox, nam);
	} else {
		shitHappens("Can't create romset with such name");
	}
}

void SetupWin::addRom() {
	int idx = ui.rsetbox->currentIndex();
	if (idx < 0) return;
	xRomFile f;
	f.name[0] = 0;
	f.foffset = 0;
	f.fsize = 0;
	f.roffset = 0;
	eidx = -1;
	rseditor->edit(f);
}

void SetupWin::delRom() {
	int idx = ui.rsetbox->currentIndex();
	QModelIndexList qmil = ui.tvRomset->selectionModel()->selectedRows();
	int row = (qmil.size() > 0) ? qmil.first().row() : -1;
	if ((idx < 0) || (row < 0)) return;
	int sz = conf.rsList[idx].roms.size();
	if (row < sz) {
		conf.rsList[idx].roms.erase(conf.rsList[idx].roms.begin() + row);
	} else if (row == sz) {
		conf.rsList[idx].gsFile.clear();
	} else if (row == sz+1) {
		conf.rsList[idx].fntFile.clear();
	} else {
		conf.rsList[idx].vBiosFile.clear();
	}
	rsmodel->fill(&conf.rsList[idx]);
}

void SetupWin::editRom() {
	int idx = ui.rsetbox->currentIndex();
	QModelIndexList qmil = ui.tvRomset->selectionModel()->selectedRows();
	int row = (qmil.size() > 0) ? qmil.first().row() : -1;
	if ((idx < 0) || (row < 0)) return;
	xRomFile f;
	f.foffset = 0;
	f.fsize = 0;
	f.roffset = 0;
	int sz = conf.rsList[idx].roms.size();
	if (row < sz) {
		f = conf.rsList[idx].roms[row];
	} else if (row == sz) {
		f.name = conf.rsList[idx].gsFile;
	} else if (row == sz+1) {
		f.name = conf.rsList[idx].fntFile;
	} else {
		f.name = conf.rsList[idx].vBiosFile;
	}
	eidx = row;
	rseditor->edit(f);
}

void SetupWin::setRom(xRomFile f) {
	int idx = ui.rsetbox->currentIndex();
	if (idx < 0) return;
	int sz = conf.rsList[idx].roms.size();
	if (eidx < 0) {
		conf.rsList[idx].roms.push_back(f);
	} else if (eidx < sz) {
		conf.rsList[idx].roms[eidx] = f;
	} else if (eidx == sz) {
		conf.rsList[idx].gsFile = f.name;
	} else if (eidx == sz+1) {
		conf.rsList[idx].fntFile = f.name;
	} else {
		conf.rsList[idx].vBiosFile = f.name;
	}
	rsmodel->fill(&conf.rsList[idx]);
}

// lists

void SetupWin::buildpadlist() {
	QDir dir(conf.path.confDir.c_str());
	QStringList lst = dir.entryList(QStringList() << "*.pad",QDir::Files,QDir::Name);
	fillRFBox(ui.cbPadMap, lst);
}

void SetupWin::buildkeylist() {
	QDir dir(conf.path.confDir.c_str());
	QStringList lst = dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name);
	dir.setPath(dir.path().append("/keymaps/"));
	lst.append(dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name));
	lst.sort();
	fillRFBox(ui.keyMapBox,lst);
}

QList<HardWare> getHardwareList() {
	QList<HardWare> res;
	int idx = 0;
	while (hwTab[idx].name) {
		res.push_back(hwTab[idx]);
		idx++;
	}
	return res;
}

struct xMemName {
	int mask;
	const char* name;
};

static xMemName memNameTab[] = {
	{MEM_16K, "16K"},
	{MEM_32K, "32K"},
	{MEM_64K, "64K"},
	{MEM_128K, "128K"},
	{MEM_256K, "256K"},
	{MEM_512K, "512K"},
	{MEM_1M, "1024K"},
	{MEM_2M, "2048K"},
	{MEM_4M, "4096K"},
	{-1, ""}
};

void SetupWin::setmszbox(int idx) {
	QList<HardWare> list = getHardwareList();
	int t = list[idx].mask;
	QString oldText = ui.mszbox->currentText();
	ui.mszbox->clear();
	idx = 0;
	while (memNameTab[idx].mask > 0) {
		if (t & memNameTab[idx].mask)
			ui.mszbox->addItem(memNameTab[idx].name, memNameTab[idx].mask);
		idx++;
	}
	ui.mszbox->setCurrentIndex(ui.mszbox->findText(oldText));
	if (ui.mszbox->currentIndex() < 0) ui.mszbox->setCurrentIndex(ui.mszbox->count() - 1);
}

void SetupWin::buildrsetlist() {
	if (ui.rsetbox->currentIndex() < 0) {
		ui.tvRomset->setEnabled(false);
	} else {
		ui.tvRomset->setEnabled(true);
		xRomset* rset = &conf.rsList[ui.rsetbox->currentIndex()];
		rsmodel->fill(rset);
	}
}

void SetupWin::buildtapelist() {
	ui.tapelist->fill(conf.prof.cur->zx->tape);
}

// TODO : make bookmarks & profiles list as view-model

void SetupWin::buildmenulist() {
	ui.umlist->setRowCount(conf.bookmarkList.size());
	QTableWidgetItem* itm;
	for (int i = 0; i < conf.bookmarkList.size(); i++) {
		itm = new QTableWidgetItem(QString(conf.bookmarkList[i].name.c_str()));
		ui.umlist->setItem(i,0,itm);
		itm = new QTableWidgetItem(QString(conf.bookmarkList[i].path.c_str()));
		ui.umlist->setItem(i,1,itm);
	}
	ui.umlist->setColumnWidth(0,100);
	ui.umlist->selectRow(0);
};

void SetupWin::buildproflist() {
	ui.twProfileList->setRowCount(conf.prof.list.size());
	QTableWidgetItem* itm;
	for (int i = 0; i < conf.prof.list.size(); i++) {
		itm = new QTableWidgetItem(QString::fromLocal8Bit(conf.prof.list[i]->name.c_str()));
		if (conf.prof.list[i] == conf.prof.cur) {
			itm->setIcon(QIcon(":/images/checkbox.png"));
		}
		ui.twProfileList->setItem(i,0,itm);
		itm = new QTableWidgetItem(QString::fromLocal8Bit(conf.prof.list[i]->file.c_str()));
		ui.twProfileList->setItem(i,1,itm);
	}
}

void SetupWin::copyToTape() {
	int dsk = ui.disktabs->currentIndex();
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	Computer* comp = conf.prof.cur->zx;
	TRFile cat[128];
	diskGetTRCatalog(comp->dif->fdc->flop[dsk],cat);
	int row;
	unsigned char* buf = new unsigned char[0xffff];
	unsigned short line,start,len;
	char name[10];
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		row = idx[i].row();
		if (diskGetSectorsData(comp->dif->fdc->flop[dsk],cat[row].trk, cat[row].sec+1, buf, cat[row].slen)) {
			if (cat[row].slen == (cat[row].hlen + ((cat[row].llen == 0) ? 0 : 1))) {
				start = ((cat[row].hst << 8) + cat[row].lst) & 0xffff;
				len = ((cat[row].hlen << 8) + cat[row].llen) & 0xffff;
				line = (cat[row].ext == 'B') ? (buf[start] + (buf[start+1] << 8)) & 0xffff : 0x8000;
				memset(name,0x20,10);
				memcpy(name,(char*)cat[row].name,8);
				tapAddFile(comp->tape,name,(cat[row].ext == 'B') ? 0 : 3, start, len, line, buf,true);
				savedFiles++;
			} else {
				shitHappens("File seems to be joined, skip");
			}
		} else {
			shitHappens("Can't get file data, skip");
		}
	}
	buildtapelist();
	std::string msg = std::string(int2str(savedFiles)) + " of " + int2str(idx.size()) + " files copied";
	showInfo(msg.c_str());
}

// hobeta header crc = ((105 + 257 * std::accumulate(data, data + 15, 0u)) & 0xffff))

void SetupWin::diskToHobeta() {
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...","",QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
	if (dir == "") return;
	Computer* comp = conf.prof.cur->zx;
	std::string sdir = std::string(dir.toLocal8Bit().data()) + SLASH;
	Floppy* flp = comp->dif->fdc->flop[ui.disktabs->currentIndex()];		// selected floppy
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveHobetaFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = std::string(int2str(savedFiles)) + " of " + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

void SetupWin::diskToRaw() {
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...","",QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
	if (dir == "") return;
	Computer* comp = conf.prof.cur->zx;
	std::string sdir = std::string(dir.toLocal8Bit().data()) + SLASH;
	Floppy* flp = comp->dif->fdc->flop[ui.disktabs->currentIndex()];
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveRawFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = std::string(int2str(savedFiles)) + " of " + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

TRFile getHeadInfo(Tape* tape, int blk) {
	TRFile res;
	TapeBlockInfo inf = tapGetBlockInfo(tape,blk,TFRM_ZX);
	unsigned char* dt = (unsigned char*)malloc(inf.size + 2);
	tapGetBlockData(tape,blk,dt,inf.size+2);
	for (int i=0; i<8; i++) res.name[i] = dt[i+2];
	switch (dt[1]) {
		case 0:
			res.ext = 'B';
			res.lst = dt[12]; res.hst = dt[13];
			res.llen = dt[16]; res.hlen = dt[17];
			// autostart?
			break;
		case 3:
			res.ext = 'C';
			res.llen = dt[12]; res.hlen = dt[13];
			res.lst = dt[14]; res.hst = dt[15];
			break;
		default:
			res.ext = 0x00;
	}
	res.slen = res.hlen;
	if (res.llen != 0) res.slen++;
	free(dt);
	return res;
}

void SetupWin::copyToDisk() {
	unsigned char* dt;
	unsigned char buf[256];
	int pos;	// skip block type mark
	TapeBlockInfo inf;
	TRFile dsc;

	QModelIndexList idl = ui.tapelist->selectionModel()->selectedRows();
	if (idl.size() < 1) return;
	int blk = idl.first().row();
	if (blk < 0) return;
	int dsk = ui.disktabs->currentIndex();
	if (dsk < 0) dsk = 0;
	if (dsk > 3) dsk = 3;
	int headBlock = -1;
	int dataBlock = -1;
	Computer* comp = conf.prof.cur->zx;
	if (!comp->tape->blkData[blk].hasBytes) {
		shitHappens("This is not standard block");
		return;
	}
	if (comp->tape->blkData[blk].isHeader) {
		if ((int)comp->tape->blkCount == blk + 1) {
			shitHappens("Header without data? Hmm...");
		} else {
			if (!comp->tape->blkData[blk+1].hasBytes) {
				shitHappens("Data block is not standard");
			} else {
				headBlock = blk;
				dataBlock = blk + 1;
			}
		}
	} else {
		dataBlock = blk;
		if (blk != 0) {
			if (comp->tape->blkData[blk-1].isHeader) {
				headBlock = blk - 1;
			}
		}
	}
	if (headBlock < 0) {
		const char nm[] = "FILE    ";
		memcpy(&dsc.name[0],nm,8);
		dsc.ext = 'C';
		dsc.lst = dsc.hst = 0;
		TapeBlockInfo binf = tapGetBlockInfo(comp->tape,dataBlock,TFRM_ZX);
		int len = binf.size;
		qDebug() << len;
		if (len > 0xff00) {
			shitHappens("Too much data for TRDos file");
			return;
		}
		dsc.llen = len & 0xff;
		dsc.hlen = ((len & 0xff00) >> 8);
		dsc.slen = dsc.hlen;
		if (dsc.llen != 0) dsc.slen++;
	} else {
		dsc = getHeadInfo(comp->tape, headBlock);
		if (dsc.ext == 0x00) {
			shitHappens("Yes, it happens");
			return;
		}
	}
	Floppy* flp = comp->dif->fdc->flop[dsk];
	if (!flp->insert) {
		newdisk(dsk, 0);
		trd_format(flp);
	} else if (diskGetType(flp) != DISK_TYPE_TRD) {
		if (areSure("Not TRDOS disk. Format?<br>All data will be lost")) {
			trd_format(flp);
		} else {
			// shitHappens("As you wish...");
			return;
		}
	}
	inf = tapGetBlockInfo(comp->tape,dataBlock,TFRM_ZX);
	dt = (unsigned char*)malloc(inf.size+2);		// +2 = +mark +crc
	tapGetBlockData(comp->tape,dataBlock,dt,inf.size+2);
	switch(diskCreateDescriptor(flp,&dsc)) {
		case ERR_SHIT: shitHappens("Yes, it happens"); break;
		case ERR_MANYFILES: shitHappens("Too many files @ disk"); break;
		case ERR_NOSPACE: shitHappens("Not enough space @ disk"); break;
		case ERR_OK:
			pos = 0;
			while (pos < inf.size) {
				do {
					buf[pos & 0xff] = (pos < inf.size) ? dt[pos+1] : 0x00;
					pos++;
				} while (pos & 0xff);

				diskPutSectorData(flp,dsc.trk, dsc.sec+1, buf, 256);

				dsc.sec++;
				if (dsc.sec > 15) {
					dsc.sec = 0;
					dsc.trk++;
				}
			}
			fillDiskCat();
			showInfo("File(s) was copied");
			break;
	}
	free(dt);
}

void SetupWin::fillDiskCat() {
	int dsk = ui.disktabs->currentIndex();
	Computer* comp = conf.prof.cur->zx;
	Floppy* flp = comp->dif->fdc->flop[dsk];
	TRFile ct[128];
	QList<TRFile> cat;
	int catSize = 0;
	ui.disklist->setEnabled(flp->insert);
	if (flp->insert && (diskGetType(flp) == DISK_TYPE_TRD)) {
		catSize = diskGetTRCatalog(flp, ct);
		for(int i = 0; i < catSize; i++) {
			if (ct[i].name[0] > 0x1f)
				cat.append(ct[i]);
		}
	}
	ui.disklist->setCatalog(cat);
}

// video

void SetupWin::chabsz() {ui.bszlab->setText(QString("%0%").arg(ui.bszsld->value()));}
void SetupWin::chaflc() {ui.labNoflic->setText(QString("%0%").arg(ui.sldNoflic->value() * 2));}

void SetupWin::selsspath() {
	QString fpath = QFileDialog::getExistingDirectory(this,"Screenshots folder",QString::fromLocal8Bit(conf.scrShot.dir.c_str()),QFileDialog::ShowDirsOnly);
	if (fpath!="") ui.pathle->setText(fpath);
}

// sound

/*
void SetupWin::updvolumes() {
	ui.bvlab->setText(QString::number(ui.bvsld->value()));
	ui.tvlab->setText(QString::number(ui.tvsld->value()));
	ui.avlab->setText(QString::number(ui.avsld->value()));
	ui.gslab->setText(QString::number(ui.gvsld->value()));
}
*/

// disk

void SetupWin::newdisk(int idx, int ask) {
	Computer* comp = conf.prof.cur->zx;
	Floppy *flp = comp->dif->fdc->flop[idx];
	if (saveChangedDisk(comp,idx & 3) != ERR_OK) return;
	flp_insert(flp, NULL);
	// diskClear(flp);
	flp->changed = 1;
	if (ask && areSure("Format for TRDOS?")) {
		trd_format(flp);
	}
	updatedisknams();
}

void SetupWin::newa() {newdisk(0,1);}
void SetupWin::newb() {newdisk(1,1);}
void SetupWin::newc() {newdisk(2,1);}
void SetupWin::newd() {newdisk(3,1);}

void SetupWin::loada() {load_file(conf.prof.cur->zx, NULL, FH_DRIVE_A, 0); updatedisknams();}
void SetupWin::loadb() {load_file(conf.prof.cur->zx, NULL, FH_DRIVE_B, 1); updatedisknams();}
void SetupWin::loadc() {load_file(conf.prof.cur->zx, NULL, FH_DRIVE_C, 2); updatedisknams();}
void SetupWin::loadd() {load_file(conf.prof.cur->zx, NULL, FH_DRIVE_D, 3); updatedisknams();}

void SetupWin::savea() {Computer* comp = conf.prof.cur->zx; Floppy* flp = comp->dif->fdc->flop[0]; if (flp->insert) save_file(comp, flp->path, FG_DISK_A, 0); updatedisknams();}
void SetupWin::saveb() {Computer* comp = conf.prof.cur->zx; Floppy* flp = comp->dif->fdc->flop[1]; if (flp->insert) save_file(comp, flp->path, FG_DISK_B, 1); updatedisknams();}
void SetupWin::savec() {Computer* comp = conf.prof.cur->zx; Floppy* flp = comp->dif->fdc->flop[2]; if (flp->insert) save_file(comp, flp->path, FG_DISK_C, 2); updatedisknams();}
void SetupWin::saved() {Computer* comp = conf.prof.cur->zx; Floppy* flp = comp->dif->fdc->flop[3]; if (flp->insert) save_file(comp, flp->path, FG_DISK_D, 3); updatedisknams();}

void SetupWin::ejcta() {Computer* comp = conf.prof.cur->zx; saveChangedDisk(comp,0); flp_eject(comp->dif->fdc->flop[0]); updatedisknams();}
void SetupWin::ejctb() {Computer* comp = conf.prof.cur->zx; saveChangedDisk(comp,1); flp_eject(comp->dif->fdc->flop[1]); updatedisknams();}
void SetupWin::ejctc() {Computer* comp = conf.prof.cur->zx; saveChangedDisk(comp,2); flp_eject(comp->dif->fdc->flop[2]); updatedisknams();}
void SetupWin::ejctd() {Computer* comp = conf.prof.cur->zx; saveChangedDisk(comp,3); flp_eject(comp->dif->fdc->flop[3]); updatedisknams();}

void SetupWin::updatedisknams() {
	Computer* comp = conf.prof.cur->zx;
	ui.apathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[0]->path));
	ui.bpathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[1]->path));
	ui.cpathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[2]->path));
	ui.dpathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[3]->path));
	fillDiskCat();
}

// tape

void SetupWin::loatape() {
	Computer* comp = conf.prof.cur->zx;
	load_file(comp, NULL, FG_TAPE, -1);
	ui.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
}

void SetupWin::savtape() {
	Computer* comp = conf.prof.cur->zx;
	if (comp->tape->blkCount != 0) {
		save_file(comp, comp->tape->path, FG_TAPE, -1);
	}
}

void SetupWin::ejctape() {
	Computer* comp = conf.prof.cur->zx;
	tapEject(comp->tape);
	ui.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
}

void SetupWin::tblkup() {
	Computer* comp = conf.prof.cur->zx;
	int ps = ui.tapelist->currentIndex().row();
	if (ps > 0) {
		tapSwapBlocks(comp->tape,ps,ps-1);
		buildtapelist();
		ui.tapelist->selectRow(ps-1);
	}
}

void SetupWin::tblkdn() {
	Computer* comp = conf.prof.cur->zx;
	int ps = ui.tapelist->currentIndex().row();
	if ((ps != -1) && (ps < comp->tape->blkCount - 1)) {
		tapSwapBlocks(comp->tape,ps,ps+1);
		buildtapelist();
		ui.tapelist->selectRow(ps+1);
	}
}

void SetupWin::tblkrm() {
	Computer* comp = conf.prof.cur->zx;
	int ps = ui.tapelist->currentIndex().row();
	if (ps != -1) {
		tapDelBlock(comp->tape,ps);
		buildtapelist();
//		ui.tapelist->selectRow(ps);
	}
}

void SetupWin::chablock(QModelIndex idx) {
	Computer* comp = conf.prof.cur->zx;
	int row = idx.row();
	tapRewind(comp->tape,row);
	buildtapelist();
//	ui.tapelist->selectRow(row);
}

void SetupWin::tlistclick(QModelIndex idx) {
	int row = idx.row();
	int col = idx.column();
	Computer* comp = conf.prof.cur->zx;
	if ((row < 0) || (row >= comp->tape->blkCount)) return;
	if (col != 1) return;
	comp->tape->blkData[row].breakPoint ^= 1;
	buildtapelist();
//	ui.tapelist->selectRow(row);
}

// hdd

void SetupWin::hddMasterImg() {
	Computer* comp = conf.prof.cur->zx;
	QString path = QFileDialog::getOpenFileName(this,"Image for master HDD","","All files (*)",NULL,QFileDialog::DontUseNativeDialog | QFileDialog::DontConfirmOverwrite);
	if (path != "") {
		ui.hm_path->setText(path);
		ideSetImage(comp->ide, IDE_MASTER, path.toLocal8Bit().data());
		ui.hm_ghd->setValue(comp->ide->master->pass.cyls);
		ui.hm_gsec->setValue(comp->ide->master->pass.spt);
		ui.hm_ghd->setValue(comp->ide->master->pass.hds);
		ui.hm_glba->setValue(comp->ide->master->maxlba);
		ui.hm_capacity->setValue(comp->ide->master->maxlba >> 11);	// 512 (sector) -> 1024*1024 (Mb)
	}
}

void SetupWin::hddSlaveImg() {
	Computer* comp = conf.prof.cur->zx;
	QString path = QFileDialog::getOpenFileName(this,"Image for slave HDD","","All files (*)",NULL,QFileDialog::DontUseNativeDialog | QFileDialog::DontConfirmOverwrite);
	if (path != "") {
		ui.hs_path->setText(path);
		ideSetImage(comp->ide, IDE_SLAVE, path.toLocal8Bit().data());
		ui.hs_ghd->setValue(comp->ide->slave->pass.cyls);
		ui.hs_gsec->setValue(comp->ide->slave->pass.spt);
		ui.hs_ghd->setValue(comp->ide->slave->pass.hds);
		ui.hs_glba->setValue(comp->ide->slave->maxlba);
		ui.hs_capacity->setValue(comp->ide->slave->maxlba >> 11);	// 512 (sector) -> 1024*1024 (Mb)
	}
}

/*
void SetupWin::hddcap() {
	int sz;
	if (ui.hs_islba->isChecked()) {
		sz = (ui.hs_glba->value() >> 9);
	} else {
		sz = ((ui.hs_gsec->value() * (ui.hs_ghd->value() + 1) * (ui.hs_gcyl->value() + 1)) >> 11);
	}
	ui.hs_capacity->setValue(sz);
}
*/

// external

void SetupWin::selSDCimg() {
	QString fnam = QFileDialog::getOpenFileName(this,"Image for SD card","","All files (*.*)",nullptr,QFileDialog::DontUseNativeDialog);
	if (!fnam.isEmpty()) ui.sdPath->setText(fnam);
}

void SetupWin::openSlot() {
	Computer* comp = conf.prof.cur->zx;
//	QString fnam = QFileDialog::getOpenFileName(this,"Cartridge slot","","MSX cartridge (*.rom)");
//	if (fnam.isEmpty()) return;
//	ui.cSlotName->setText(fnam);
//	loadFile(comp, fnam.toLocal8Bit().data(), FT_SLOT_A, 0);
	if (load_file(comp, NULL, FH_SLOTS, 0) == ERR_OK) {
		ui.cSlotName->setText(comp->slot->name);
	}
}

int testSlotOn(Computer*);

void SetupWin::ejectSlot() {
	Computer* comp = conf.prof.cur->zx;
	sltEject(comp->slot);
	ui.cSlotName->clear();
	if (testSlotOn(comp))
		compReset(comp,RES_DEFAULT);
}

// input

void SetupWin::setCurrentGamepad(int idx) {
	if (idx > 0) {			// 0 is 'none'
		conf.joy.gpad->open(idx-1);
	} else {
		conf.joy.gpad->close();
	}
}

void SetupWin::newPadMap() {
	QString nam = QInputDialog::getText(this,"Enter...","New gamepad map name");
	if (nam.isEmpty()) return;
	nam.append(".pad");
	std::string name = nam.toStdString();
	if (padCreate(name)) {
		ui.cbPadMap->addItem(nam, nam);
		ui.cbPadMap->setCurrentIndex(ui.cbPadMap->count() - 1);
	} else {
		showInfo("Map with that name already exists");
	}
}

void SetupWin::delPadMap() {
	if (ui.cbPadMap->currentIndex() == 0) return;
	QString name = getRFSData(ui.cbPadMap);
	if (name.isEmpty()) return;
	if (!areSure("Delete this map?")) return;
	padDelete(name.toStdString());
	ui.cbPadMap->removeItem(ui.cbPadMap->currentIndex());
	ui.cbPadMap->setCurrentIndex(0);
}

void SetupWin::chaPadMap(int idx) {
	idx--;
	if (idx < 0) {
		conf.joy.map.clear();
	} else {
		padLoadConfig(getRFSData(ui.cbPadMap).toStdString());
	}
	padModel->update();
}

void SetupWin::addBinding() {
	if (getRFSData(ui.cbPadMap).isEmpty()) return;
	bindidx = -1;
	xJoyMapEntry jent;
	jent.dev = JOY_NONE;
	jent.dev = JMAP_JOY;
#if USE_SEQ_BIND
	jent.seq = QKeySequence();
#else
	jent.key = ENDKEY;
#endif
	jent.dir = XJ_NONE;
	jent.rpt = 0;
	padial->start(jent);
}

void SetupWin::editBinding() {
	bindidx = ui.tvPadTable->currentIndex().row();
	if (bindidx < 0) return;
	padial->start(conf.joy.map[bindidx]);
}

void SetupWin::bindAccept(xJoyMapEntry ent) {
	if (ent.type == JOY_NONE) return;
	if (ent.dev == JMAP_NONE) return;
	if ((bindidx < 0) || (bindidx >= (int)conf.joy.map.size())) {
		conf.joy.map.push_back(ent);
	} else {
		conf.joy.map[bindidx] = ent;
	}
	padSaveConfig(getRFSData(ui.cbPadMap).toStdString());
	padModel->update();
}

extern bool qmidx_greater(const QModelIndex, const QModelIndex);

void SetupWin::delBinding() {
	QModelIndexList lst = ui.tvPadTable->selectionModel()->selectedRows();
	if (!lst.isEmpty()) {
		std::sort(lst.begin(), lst.end(), qmidx_greater);
		if (areSure("Delete this binding(s)?")) {
			int row;
			foreach(QModelIndex idx, lst) {
				row = idx.row();
				conf.joy.map.erase(conf.joy.map.begin() + row);
			}
			padModel->update();
			padSaveConfig(getRFSData(ui.cbPadMap).toStdString());
		}
	}
/*
		int row = ui.tvPadTable->currentIndex().row();
		if (row < 0) return;
		if (!areSure("Delete this binding?")) return;
		conf.joy.map.erase(conf.joy.map.begin() + row);
		padModel->update();
		padSaveConfig(getRFSData(ui.cbPadMap).toStdString());
*/
}

// tools

void SetupWin::umup() {
	int ps = ui.umlist->currentRow();
	if (ps > 0) {
		swapBookmarks(ps,ps-1);
		buildmenulist();
		ui.umlist->selectRow(ps-1);
	}
}

void SetupWin::umdn() {
	int ps = ui.umlist->currentIndex().row();
	if ((ps != -1) && (ps < (int)conf.bookmarkList.size() - 1)) {
		swapBookmarks(ps, ps+1);
		buildmenulist();
		ui.umlist->selectRow(ps+1);
	}
}

void SetupWin::umdel() {
	int ps = ui.umlist->currentIndex().row();
	if (ps != -1) {
		delBookmark(ps);
		buildmenulist();
		if (ps == conf.bookmarkList.size()) {
			ui.umlist->selectRow(ps-1);
		} else {
			ui.umlist->selectRow(ps);
		}
	}
}

void SetupWin::umadd() {
	uia.namele->clear();
	uia.pathle->clear();
	umidx = -1;
	umadial->show();
}

void SetupWin::umedit(QModelIndex idx) {
	umidx = idx.row();
	uia.namele->setText(ui.umlist->item(umidx,0)->text());
	uia.pathle->setText(ui.umlist->item(umidx,1)->text());
	umadial->show();
}

void SetupWin::umaselp() {
	QString fpath = QFileDialog::getOpenFileName(NULL,"Select file","","Known formats (*.sna *.z80 *.tap *.tzx *.trd *.scl *.fdi *.udi)",nullptr,QFileDialog::DontUseNativeDialog);
	if (fpath!="") uia.pathle->setText(fpath);
}

void SetupWin::umaconf() {
	if ((uia.namele->text()=="") || (uia.pathle->text()=="")) return;
	if (umidx == -1) {
		addBookmark(std::string(uia.namele->text().toLocal8Bit().data()),std::string(uia.pathle->text().toLocal8Bit().data()));
	} else {
		setBookmark(umidx,std::string(uia.namele->text().toLocal8Bit().data()),std::string(uia.pathle->text().toLocal8Bit().data()));
	}
	umadial->hide();
	buildmenulist();
	ui.umlist->selectRow(ui.umlist->rowCount()-1);
}

// profiles

void SetupWin::newProfile() {
	QString nam = QInputDialog::getText(this,"Enter...","New profile name");
	if (nam.isEmpty()) return;
	std::string nm = std::string(nam.toLocal8Bit().data());
	std::string fp = nm + ".conf";
	if (!addProfile(nm,fp))
		shitHappens("Can't add such profile");
	buildproflist();
}

void SetupWin::copyProf() {
	int idx = ui.twProfileList->currentRow();
	if (idx < 0) return;
	QString nam = QInputDialog::getText(this,"Enter...","New profile name");
	if (nam.isEmpty()) return;
	std::string nm = std::string(nam.toLocal8Bit().data());
	std::string pnam(ui.twProfileList->item(idx,0)->text().toLocal8Bit().data());
	if (!copyProfile(pnam, nm))
		shitHappens("Copying failed");
	buildproflist();
}

void SetupWin::chProfile(int row, int col) {
	if (row < 0) return;
	if (row > conf.prof.list.size()) return;
	std::string nm = conf.prof.list[row]->name;
	prfSetCurrent(nm);
	start();
	emit s_prf_changed();
}

void SetupWin::rmProfile() {
	int idx = ui.twProfileList->currentRow();
	if (idx < 0) return;
//	block = 1;
	if (areSure("Do you really want to delete this profile?")) {
		std::string pnam(ui.twProfileList->item(idx,0)->text().toLocal8Bit().data());
		idx = delProfile(pnam);
		switch(idx) {
			case DELP_OK_CURR:
//				conf.prof.changed = 1;
				start();
				emit s_prf_changed();
				break;
			case DELP_ERR:
				shitHappens("Sorry, i can't delete this profile");
				break;
		}
	}
//	block = 0;
	buildproflist();
}

// debuga

void SetupWin::selectDbgFont() {
	bool ok;
	dbgfnt = QFontDialog::getFont(&ok, dbgfnt, this, "Select font", QFontDialog::DontUseNativeDialog);
	ui.leDbgFont->setText(QString("%0, %1 pt").arg(dbgfnt.family()).arg(dbgfnt.pointSize()));
	ui.leDbgFont->setFont(dbgfnt);
}

// debuga palette

void SetupWin::selectColor() {
	QToolButton* obj = (QToolButton*)sender();
	QString cn = obj->property("colorName").toString();
	QString dn = obj->property("defaultColor").toString();
	if (cn.isEmpty()) return;
	QColor col = QColorDialog::getColor(conf.pal[cn], this, "Select color", QColorDialog::DontUseNativeDialog);
	if (!col.isValid()) return;
	conf.pal[cn] = col;
	setToolButtonColor(obj, cn, dn);
}

void SetupWin::triggerColor() {
	QToolButton* obj = (QToolButton*)sender();
	QString cn = obj->property("colorName").toString();
	QString dn = obj->property("defaultColor").toString();
	if (cn.isEmpty()) return;
	if (dn.isEmpty()) {
		conf.pal.remove(cn);
	} else {
		QColor col(dn);
		if (col.isValid())
			conf.pal[cn] = col;
	}
	setToolButtonColor(obj, cn, dn);
}
