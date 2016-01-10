#include <QStandardItemModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QVector3D>
#include <QPainter>
#include <QDebug>
#include <stdlib.h>
#ifdef HAVESDL
	#include <SDL.h>
	#undef main
#endif

#include "sound.h"
#include "filer.h"
#include "setupwin.h"
#include "xgui/xgui.h"
#include "xcore/xcore.h"
#include "libxpeccy/spectrum.h"
#include "libxpeccy/filetypes/filetypes.h"

void fillRFBox(QComboBox* box, QStringList lst) {
	box->clear();
	box->addItem("none");
	box->addItems(lst);
}

std::string getRFText(QComboBox* box) {
	QString res = "";
	if (box->currentIndex() >= 0) res = box->currentText();
	return std::string(res.toLocal8Bit().data());
}

// OBJECT

SetupWin::SetupWin(QWidget* par):QDialog(par) {
	setModal(true);
	ui.setupUi(this);

	umadial = new QDialog;
	uia.setupUi(umadial);
	umadial->setModal(true);

	rseditor = new QDialog(this);
	rseUi.setupUi(rseditor);
	rseditor->setModal(true);

	layeditor = new QDialog(this);
	layUi.setupUi(layeditor);
	layeditor->setModal(true);

	block = 0;
	prfChanged = 0;

	unsigned int i;
// machine
	i = 0;
	while (hwTab[i].name != NULL) {
		ui.machbox->addItem(QString::fromLocal8Bit(hwTab[i].optName),QString::fromLocal8Bit(hwTab[i].name));
		i++;
	}
	ui.resbox->addItem("BASIC 48",RES_48);
	ui.resbox->addItem("BASIC 128",RES_128);
	ui.resbox->addItem("DOS",RES_DOS);
	ui.resbox->addItem("SHADOW",RES_SHADOW);
	QTableWidgetItem* itm;
	for (i = 0; i < (unsigned)ui.rstab->rowCount(); i++) {
		itm = new QTableWidgetItem; ui.rstab->setItem(i,1,itm);
		itm = new QTableWidgetItem; ui.rstab->setItem(i,2,itm);
	}
// video
	std::map<std::string,int>::iterator it;
	for (it = shotFormat.begin(); it != shotFormat.end(); it++) {
		ui.ssfbox->addItem(QString(it->first.c_str()),it->second);

	}
// sound
	i = 0;
	while (sndTab[i].name != NULL) {
		ui.outbox->addItem(QString::fromLocal8Bit(sndTab[i].name));
		i++;
	}
	ui.ratbox->addItem("48000",48000);
	ui.ratbox->addItem("44100",44100);
	ui.ratbox->addItem("22050",22050);
	ui.ratbox->addItem("11025",11025);
	ui.schip1box->addItem(QIcon(":/images/cancel.png"),"none",SND_NONE);
	ui.schip1box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",SND_AY);
	ui.schip1box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",SND_YM);
	ui.schip2box->addItem(QIcon(":/images/cancel.png"),"none",SND_NONE);
	ui.schip2box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",SND_AY);
	ui.schip2box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",SND_YM);
#ifdef ISDEBUG
	ui.schip1box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2203",SND_YM2203);
	ui.schip2box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2203",SND_YM2203);
#endif
	ui.stereo1box->addItem("Mono",AY_MONO); ui.stereo2box->addItem("Mono",AY_MONO);
	ui.stereo1box->addItem("ABC",AY_ABC); ui.stereo2box->addItem("ABC",AY_ABC);
	ui.stereo1box->addItem("ACB",AY_ACB); ui.stereo2box->addItem("ACB",AY_ACB);
	ui.stereo1box->addItem("BAC",AY_BAC); ui.stereo2box->addItem("BAC",AY_BAC);
	ui.stereo1box->addItem("BCA",AY_BCA); ui.stereo2box->addItem("BCA",AY_BCA);
	ui.stereo1box->addItem("CAB",AY_CAB); ui.stereo2box->addItem("CAB",AY_CAB);
	ui.stereo1box->addItem("CBA",AY_CBA); ui.stereo2box->addItem("CBA",AY_BCA);
	ui.tsbox->addItem("None",TS_NONE);
	ui.tsbox->addItem("NedoPC",TS_NEDOPC);
	ui.gstereobox->addItem("Mono",GS_MONO);
	ui.gstereobox->addItem("L:1,2; R:3,4",GS_12_34);
	ui.sdrvBox->addItem("None",SDRV_NONE);
	ui.sdrvBox->addItem("Covox only",SDRV_COVOX);
	ui.sdrvBox->addItem("Soundrive 1.05 mode 1",SDRV_105_1);
	ui.sdrvBox->addItem("Soundrive 1.05 mode 2",SDRV_105_2);
	ui.cbSaa->addItem("None",SAA_OFF);
	ui.cbSaa->addItem("Mono",SAA_MONO);
	ui.cbSaa->addItem("Stereo",SAA_STEREO);
// bdi
// WTF? QtDesigner doesn't save this properties
	ui.disklist->horizontalHeader()->setVisible(true);
	ui.diskTypeBox->addItem("None",DIF_NONE);
	ui.diskTypeBox->addItem("Beta disk (VG93)",DIF_BDI);
	ui.diskTypeBox->addItem("+3 DOS (uPD765)",DIF_P3DOS);
	ui.disklist->addAction(ui.actCopyToTape);
	ui.disklist->addAction(ui.actSaveHobeta);
	ui.disklist->addAction(ui.actSaveRaw);
// tape
	ui.tapelist->setColumnWidth(0,20);
	ui.tapelist->setColumnWidth(1,20);
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
	ui.hm_type->addItem(QIcon(":/images/cancel.png"),"Not connected",IDE_NONE);
	ui.hm_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",IDE_ATA);
//	setupUi.hm_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",IDE_ATAPI);
	ui.hs_type->addItem(QIcon(":/images/cancel.png"),"Not connected",IDE_NONE);
	ui.hs_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",IDE_ATA);
//	setupUi.hs_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",IDE_ATAPI);
// sdcard
	ui.sdcapbox->addItem("32 M",SDC_32M);
	ui.sdcapbox->addItem("64 M",SDC_64M);
	ui.sdcapbox->addItem("128 M",SDC_128M);
	ui.sdcapbox->addItem("256 M",SDC_256M);
	ui.sdcapbox->addItem("512 M",SDC_512M);
	ui.sdcapbox->addItem("1024 M",SDC_1G);

// all
	connect(ui.okbut,SIGNAL(released()),this,SLOT(okay()));
	connect(ui.apbut,SIGNAL(released()),this,SLOT(apply()));
	connect(ui.cnbut,SIGNAL(released()),this,SLOT(reject()));
// machine
	connect(ui.rsetbox,SIGNAL(currentIndexChanged(int)),this,SLOT(buildrsetlist()));
	connect(ui.machbox,SIGNAL(currentIndexChanged(int)),this,SLOT(setmszbox(int)));
	// connect(ui.cpufrq,SIGNAL(valueChanged(int)),this,SLOT(updfrq()));
	// connect(ui.sbFreq,SIGNAL(valueChanged(double)),this,SLOT(updfrq2()));
	connect(ui.rstab,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editrset()));
	connect(ui.addrset,SIGNAL(released()),this,SLOT(addNewRomset()));
	connect(ui.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
	connect(ui.rsedit,SIGNAL(released()),this,SLOT(editrset()));

	connect(rseUi.rse_cancel,SIGNAL(released()),rseditor,SLOT(hide()));
	connect(rseUi.rse_apply,SIGNAL(released()),this,SLOT(setrpart()));
	connect(rseUi.rse_grp_single,SIGNAL(toggled(bool)),this,SLOT(recheck_separate(bool)));
	connect(rseUi.rse_grp_separate,SIGNAL(toggled(bool)),this,SLOT(recheck_single(bool)));
	connect(rseUi.rsName,SIGNAL(textChanged(QString)),this,SLOT(rsNameCheck(QString)));
// video
	connect(ui.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	connect(ui.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));

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
	connect(layUi.okButton,SIGNAL(released()),this,SLOT(layEditorOK()));
	connect(layUi.cnButton,SIGNAL(released()),layeditor,SLOT(hide()));
// sound
	connect(ui.bvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	connect(ui.tvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	connect(ui.avsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	connect(ui.gvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
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
	connect(ui.tapelist,SIGNAL(cellClicked(int,int)),this,SLOT(setTapeBreak(int,int)));
	connect(ui.tloadtb,SIGNAL(released()),this,SLOT(loatape()));
	connect(ui.tsavetb,SIGNAL(released()),this,SLOT(savtape()));
	connect(ui.tremotb,SIGNAL(released()),this,SLOT(ejctape()));
	connect(ui.blkuptb,SIGNAL(released()),this,SLOT(tblkup()));
	connect(ui.blkdntb,SIGNAL(released()),this,SLOT(tblkdn()));
	connect(ui.blkrmtb,SIGNAL(released()),this,SLOT(tblkrm()));
	connect(ui.actCopyToDisk,SIGNAL(triggered()),this,SLOT(copyToDisk()));
	connect(ui.tbToDisk,SIGNAL(released()),this,SLOT(copyToDisk()));
// hdd
	connect(ui.hm_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	connect(ui.hm_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hm_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hm_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hm_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hm_pathtb,SIGNAL(released()),this,SLOT(hddMasterImg()));
	connect(ui.hs_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	connect(ui.hs_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hs_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hs_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hs_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hs_pathtb,SIGNAL(released()),this,SLOT(hddSlaveImg()));
// external
	connect(ui.tbSDCimg,SIGNAL(released()),this,SLOT(selSDCimg()));
	connect(ui.tbsdcfree,SIGNAL(released()),ui.sdPath,SLOT(clear()));
	connect(ui.cSlotAOpen,SIGNAL(released()),this,SLOT(openSlotA()));
	connect(ui.cSlotBOpen,SIGNAL(released()),this,SLOT(openSlotB()));
	connect(ui.cSlotAEject,SIGNAL(released()),this,SLOT(ejectSlotA()));
	connect(ui.cSlotBEject,SIGNAL(released()),this,SLOT(ejectSlotB()));
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
// profiles manager
	connect(ui.tbNewProfile,SIGNAL(released()),this,SLOT(newProfile()));
	connect(ui.tbDelProfile,SIGNAL(released()),this,SLOT(rmProfile()));

}

void SetupWin::recheck_single(bool st) {
	rseUi.rse_grp_single->setChecked(!st);
}
void SetupWin::recheck_separate(bool st) {
	rseUi.rse_grp_separate->setChecked(!st);
}

void SetupWin::okay() {
	apply();
	reject();
}

void SetupWin::start(xProfile* p) {
	prof = p;
	comp = p->zx;
	unsigned int i;
// machine
	ui.rsetbox->clear();
	foreach(xRomset rs, conf.rsList) {
		ui.rsetbox->addItem(QString::fromLocal8Bit(rs.name.c_str()));
	}

//	for (i=0; i < conf.rsList.size(); i++) {
//		ui.rsetbox->addItem(QString::fromLocal8Bit(rsList[i].name.c_str()));
//	}
	ui.machbox->setCurrentIndex(ui.machbox->findData(QString::fromUtf8(comp->hw->name)));
	ui.rsetbox->setCurrentIndex(ui.rsetbox->findText(QString::fromUtf8(prof->rsName.c_str())));
	ui.resbox->setCurrentIndex(ui.resbox->findData(comp->resbank));
	setmszbox(ui.machbox->currentIndex());
	ui.mszbox->setCurrentIndex(ui.mszbox->findData(comp->mem->memSize));
	if (ui.mszbox->currentIndex() < 0) ui.mszbox->setCurrentIndex(ui.mszbox->count() - 1);
	ui.sbFreq->setValue(comp->cpuFrq);
	// ui.cpufrq->setValue(comp->cpuFrq * 1000000);
	// updfrq();
	ui.scrpwait->setChecked(comp->scrpWait);
	ui.sysCmos->setChecked(conf.sysclock);
// video
	//ui.dszchk->setChecked(conf.vid.doubleSize);
	ui.sbScale->setValue(conf.vid.scale);
	ui.fscchk->setChecked(conf.vid.fullScreen);
	ui.noflichk->setChecked(conf.vid.noFlick);
	ui.grayscale->setChecked(conf.vid.grayScale);
	ui.border4T->setChecked(comp->vid->border4t);
	ui.contMem->setChecked(comp->contMem);
	ui.contIO->setChecked(comp->contIO);
	ui.bszsld->setValue((int)(conf.brdsize * 100));
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
// sound
	ui.senbox->setChecked(conf.snd.enabled);
	ui.mutbox->setChecked(conf.snd.mute);
	ui.gsrbox->setChecked(comp->gs->reset);
	ui.outbox->setCurrentIndex(ui.outbox->findText(QString::fromLocal8Bit(sndOutput->name)));
	ui.ratbox->setCurrentIndex(ui.ratbox->findData(QVariant(conf.snd.rate)));
	ui.bvsld->setValue(conf.snd.vol.beep);
	ui.tvsld->setValue(conf.snd.vol.tape);
	ui.avsld->setValue(conf.snd.vol.ay);
	ui.gvsld->setValue(conf.snd.vol.gs);
	ui.schip1box->setCurrentIndex(ui.schip1box->findData(QVariant(comp->ts->chipA->type)));
	ui.schip2box->setCurrentIndex(ui.schip2box->findData(QVariant(comp->ts->chipB->type)));
	ui.stereo1box->setCurrentIndex(ui.stereo1box->findData(QVariant(comp->ts->chipA->stereo)));
	ui.stereo2box->setCurrentIndex(ui.stereo2box->findData(QVariant(comp->ts->chipB->stereo)));
	ui.gstereobox->setCurrentIndex(ui.gstereobox->findData(QVariant(comp->gs->stereo)));
	ui.gsgroup->setChecked(comp->gs->enable);
	ui.tsbox->setCurrentIndex(ui.tsbox->findData(QVariant(comp->ts->type)));
	ui.sdrvBox->setCurrentIndex(ui.sdrvBox->findData(QVariant(comp->sdrv->type)));
	i = comp->saa->enabled ? (comp->saa->mono ? SAA_MONO : SAA_STEREO) : SAA_OFF;
	ui.cbSaa->setCurrentIndex(ui.cbSaa->findData(QVariant(i)));
// input
	buildkeylist();
	// buildjmaplist();
	int idx = ui.keyMapBox->findText(QString(conf.keyMapName.c_str()));
	if (idx < 1) idx = 0;
	ui.keyMapBox->setCurrentIndex(idx);
	ui.ratEnable->setChecked(comp->mouse->enable);
	ui.ratWheel->setChecked(comp->mouse->hasWheel);
	ui.cbSwapButtons->setChecked(comp->mouse->swapButtons);
	// setupUi.joyBox->setVisible(false);
// dos
	ui.diskTypeBox->setCurrentIndex(ui.diskTypeBox->findData(comp->dif->type));
	ui.bdtbox->setChecked(fdcFlag & FDC_FAST);
	ui.mempaths->setChecked(conf.storePaths);
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
//	setupUi.hm_model->setText(QString::fromLocal8Bit(pass.model));
//	setupUi.hm_ser->setText(QString::fromLocal8Bit(pass.serial));
	ui.hm_path->setText(QString::fromLocal8Bit(comp->ide->master->image));
	ui.hm_islba->setChecked(comp->ide->master->hasLBA);
	ui.hm_gsec->setValue(pass.spt);
	ui.hm_ghd->setValue(pass.hds);
	ui.hm_gcyl->setValue(pass.cyls);
	ui.hm_glba->setValue(comp->ide->master->maxlba);

	ui.hs_type->setCurrentIndex(ui.hm_type->findData(comp->ide->slave->type));
	pass = ideGetPassport(comp->ide,IDE_SLAVE);
//	setupUi.hs_model->setText(QString::fromLocal8Bit(pass.model));
//	setupUi.hs_ser->setText(QString::fromLocal8Bit(pass.serial));
	ui.hs_path->setText(QString::fromLocal8Bit(comp->ide->slave->image));
	ui.hs_islba->setChecked(comp->ide->slave->hasLBA);
	ui.hs_gsec->setValue(pass.spt);
	ui.hs_ghd->setValue(pass.hds);
	ui.hs_gcyl->setValue(pass.cyls);
	ui.hs_glba->setValue(comp->ide->slave->maxlba);
// external
	ui.sdPath->setText(QString::fromLocal8Bit(comp->sdc->image));
	ui.sdcapbox->setCurrentIndex(ui.sdcapbox->findData(comp->sdc->capacity));
	if (ui.sdcapbox->currentIndex() < 0) ui.sdcapbox->setCurrentIndex(2);	// 128M
	ui.sdlock->setChecked(comp->sdc->lock);

	ui.cSlotAName->setText(comp->msx.slotA.name);
	ui.cSlotBName->setText(comp->msx.slotB.name);
// tape
	ui.cbTapeAuto->setChecked(conf.tape.autostart);
	ui.cbTapeFast->setChecked(conf.tape.fast);
	ui.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
// tools
	buildmenulist();
// leds
	ui.cbMouseLed->setChecked(conf.led.mouse);
	ui.cbJoyLed->setChecked(conf.led.joy);
	ui.cbKeysLed->setChecked(conf.led.keys);
	ui.cbTapeLed->setChecked(conf.led.tape);
// profiles
	ui.defstart->setChecked(conf.defProfile);
	buildproflist();

	show();
}

void SetupWin::apply() {
// machine
	HardWare *oldmac = comp->hw;
	prof->hwName = std::string(ui.machbox->itemData(ui.machbox->currentIndex()).toString().toUtf8().data());
	compSetHardware(prof->zx,prof->hwName.c_str());
	prof->rsName = getRFText(ui.rsetbox); // std::string(ui.rsetbox->currentText().toUtf8().data());
	prfSetRomset(prof, prof->rsName);
	comp->resbank = ui.resbox->itemData(ui.resbox->currentIndex()).toInt();
	memSetSize(comp->mem,ui.mszbox->itemData(ui.mszbox->currentIndex()).toInt());
	compSetFrq(comp, ui.sbFreq->value());
	comp->scrpWait = ui.scrpwait->isChecked() ? 1 : 0;
	if (comp->hw != oldmac) compReset(comp,RES_DEFAULT);
	conf.sysclock = ui.sysCmos->isChecked() ? 1 : 0;
// video
	// conf.vid.doubleSize = ui.dszchk->isChecked() ? 1 : 0;
	conf.vid.scale = ui.sbScale->value();
	conf.vid.fullScreen = ui.fscchk->isChecked() ? 1 : 0;
	conf.vid.noFlick = ui.noflichk->isChecked() ? 1 : 0;
	conf.vid.grayScale = ui.grayscale->isChecked() ? 1 : 0;
	conf.scrShot.dir = std::string(ui.pathle->text().toLocal8Bit().data());
	conf.scrShot.format = getRFText(ui.ssfbox); // std::string(ui.ssfbox->currentText().toLocal8Bit().data());
	conf.scrShot.count = ui.scntbox->value();
	conf.scrShot.interval = ui.sintbox->value();
	conf.scrShot.noLeds = ui.ssNoLeds->isChecked() ? 1 : 0;
	conf.scrShot.noBorder = ui.ssNoBord->isChecked() ? 1 : 0;
	conf.brdsize = ui.bszsld->value()/100.0;
	comp->vid->border4t = ui.border4T->isChecked() ? 1 : 0;
	comp->contMem = ui.contMem->isChecked() ? 1 : 0;
	comp->contIO = ui.contIO->isChecked() ? 1 : 0;
	comp->vid->ula->enabled = ui.ulaPlus->isChecked() ? 1 : 0;
	prfSetLayout(NULL, getRFText(ui.geombox)); // std::string(ui.geombox->currentText().toLocal8Bit().data()));
// sound
	//std::string oname = std::string(sndOutput->name);
	std::string nname = getRFText(ui.outbox); // ui.outbox->currentText().toLocal8Bit().data());
	conf.snd.enabled = ui.senbox->isChecked() ? 1 : 0;
	conf.snd.mute = ui.mutbox->isChecked() ? 1 : 0;
	conf.snd.rate = ui.ratbox->itemData(ui.ratbox->currentIndex()).toInt();
	conf.snd.vol.beep = ui.bvsld->value();
	conf.snd.vol.tape = ui.tvsld->value();
	conf.snd.vol.ay = ui.avsld->value();
	conf.snd.vol.gs = ui.gvsld->value();
	setOutput(nname.c_str());
	aymSetType(comp->ts->chipA,ui.schip1box->itemData(ui.schip1box->currentIndex()).toInt());
	aymSetType(comp->ts->chipB,ui.schip2box->itemData(ui.schip2box->currentIndex()).toInt());
	comp->ts->chipA->stereo = ui.stereo1box->itemData(ui.stereo1box->currentIndex()).toInt();
	comp->ts->chipB->stereo = ui.stereo2box->itemData(ui.stereo2box->currentIndex()).toInt();
	comp->ts->type = ui.tsbox->itemData(ui.tsbox->currentIndex()).toInt();
	comp->gs->enable = ui.gsgroup->isChecked() ? 1 : 0;
	comp->gs->reset = ui.gsrbox->isChecked() ? 1 : 0;
	comp->gs->stereo = ui.gstereobox->itemData(ui.gstereobox->currentIndex()).toInt();
	comp->sdrv->type = ui.sdrvBox->itemData(ui.sdrvBox->currentIndex()).toInt();
	switch (ui.cbSaa->itemData(ui.cbSaa->currentIndex()).toInt()) {
		case SAA_OFF: comp->saa->enabled = 0; break;
		case SAA_MONO: comp->saa->enabled = 1; comp->saa->mono = 1; break;
		case SAA_STEREO: comp->saa->enabled = 1; comp->saa->mono = 0; break;
	}
// input
	comp->mouse->enable = ui.ratEnable->isChecked() ? 1 : 0;
	comp->mouse->hasWheel = ui.ratWheel->isChecked() ? 1 : 0;
	comp->mouse->swapButtons = ui.cbSwapButtons->isChecked() ? 1 : 0;
	//if (setupUi.inpDevice->currentIndex() < 1) {
	//	optSet(OPT_JOYNAME,std::string(""));
	//} else {
	//	optSet(OPT_JOYNAME,std::string(setupUi.inpDevice->currentText().toLocal8Bit().data()));
	//}
	std::string kmname = getRFText(ui.keyMapBox);
	if (kmname == "none") kmname = "default";
	conf.keyMapName = kmname;
	loadKeys();
// bdi
//	comp->bdi->fdc->type = ui.diskTypeBox->itemData(ui.diskTypeBox->currentIndex()).toInt();
	difSetHW(comp->dif, ui.diskTypeBox->itemData(ui.diskTypeBox->currentIndex()).toInt());
	setFlagBit(ui.bdtbox->isChecked(),&fdcFlag,FDC_FAST);
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
	comp->ide->type = ui.hiface->itemData(ui.hiface->currentIndex()).toInt();

	ATAPassport pass = ideGetPassport(comp->ide,IDE_MASTER);
	comp->ide->master->type = ui.hm_type->itemData(ui.hm_type->currentIndex()).toInt();
	ideSetImage(comp->ide,IDE_MASTER,ui.hm_path->text().toLocal8Bit().data());
	comp->ide->master->hasLBA = ui.hm_islba->isChecked() ? 1 : 0;
	pass.spt = ui.hm_gsec->value();
	pass.hds = ui.hm_ghd->value();
	pass.cyls = ui.hm_gcyl->value();
	comp->ide->master->maxlba = ui.hm_glba->value();
	ideSetPassport(comp->ide,IDE_MASTER,pass);

	pass = ideGetPassport(comp->ide,IDE_SLAVE);
	comp->ide->slave->type = ui.hs_type->itemData(ui.hs_type->currentIndex()).toInt();
	ideSetImage(comp->ide,IDE_SLAVE,ui.hs_path->text().toLocal8Bit().data());
	comp->ide->slave->hasLBA = ui.hs_islba->isChecked() ? 1 : 0;
	pass.spt = ui.hs_gsec->value();
	pass.hds = ui.hs_ghd->value();
	pass.cyls = ui.hs_gcyl->value();
	comp->ide->slave->maxlba = ui.hs_glba->value();
	ideSetPassport(comp->ide,IDE_SLAVE,pass);
// sdcard
	sdcSetImage(comp->sdc,ui.sdPath->text().isEmpty() ? "" : ui.sdPath->text().toLocal8Bit().data());
	sdcSetCapacity(comp->sdc,ui.sdcapbox->itemData(ui.sdcapbox->currentIndex()).toInt());
	comp->sdc->lock = ui.sdlock->isChecked() ? 1 : 0;
// tape
	conf.tape.autostart = ui.cbTapeAuto->isChecked() ? 1 : 0;
	conf.tape.fast = ui.cbTapeFast->isChecked() ? 1 : 0;
// leds
	conf.led.mouse = ui.cbMouseLed->isChecked() ? 1 : 0;
	conf.led.joy = ui.cbJoyLed->isChecked() ? 1 : 0;
	conf.led.keys = ui.cbKeysLed->isChecked() ? 1 : 0;
	conf.led.tape = ui.cbTapeLed->isChecked() ? 1 : 0;
// profiles
	conf.defProfile = ui.defstart->isChecked() ? 1 : 0;

	saveConfig();
	prfSave("");
	nsPerFrame = comp->nsPerFrame;
	sndCalibrate();
}

void SetupWin::reject() {
	hide();
	emit closed();
}

// LAYOUTS

void SetupWin::layNameCheck(QString nam) {
	layUi.okButton->setEnabled(!layUi.layName->text().isEmpty());
	for (uint i = 0; i < conf.layList.size(); i++) {
//		qDebug() << eidx << i << layList[i].name.c_str() << nam;
		if ((QString(conf.layList[i].name.c_str()) == nam) && (eidx != (int)i)) {
			layUi.okButton->setEnabled(false);
		}
	}
}

void SetupWin::editLayout() {
	layUi.lineBox->setValue(nlay.full.h);
	layUi.rowsBox->setValue(nlay.full.v);
	layUi.hsyncBox->setValue(nlay.sync.h);
	layUi.vsyncBox->setValue(nlay.sync.v);
	layUi.brdLBox->setValue(nlay.bord.h - nlay.sync.h);
	layUi.brdUBox->setValue(nlay.bord.v - nlay.sync.v);
	layUi.intRowBox->setValue(nlay.intpos.v);
	layUi.intPosBox->setValue(nlay.intpos.h);
	layUi.intLenBox->setValue(nlay.intsz);
	layUi.okButton->setEnabled(false);
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
	layUi.brdLBox->setMaximum(layUi.lineBox->value() - layUi.hsyncBox->value() - 256);
	layUi.brdUBox->setMaximum(layUi.rowsBox->value() - layUi.vsyncBox->value() - 192);
	layUi.hsyncBox->setMaximum(layUi.lineBox->value() - 256);
	layUi.vsyncBox->setMaximum(layUi.rowsBox->value() - 192);
	layUi.lineBox->setMinimum(layUi.brdLBox->value() + layUi.hsyncBox->value() + 256);
	layUi.rowsBox->setMinimum(layUi.brdUBox->value() + layUi.vsyncBox->value() + 192);
	layUi.intRowBox->setMaximum(layUi.rowsBox->value());
	layUi.intPosBox->setMaximum(layUi.lineBox->value());

	QString infText = QString::number((layUi.lineBox->value() >> 1)).append(" / ");
	int toscr = layUi.hsyncBox->value() + layUi.brdLBox->value() - layUi.intPosBox->value() +
			(layUi.vsyncBox->value() + layUi.brdUBox->value() - layUi.intRowBox->value()) * layUi.lineBox->value();
	infText.append(QString::number(toscr >> 1)).append(" / ");
	infText.append(QString::number((layUi.rowsBox->value() * layUi.lineBox->value()) >> 1));
	layUi.infLabel->setText(infText);

	layUi.showField->setFixedSize(layUi.lineBox->value(),layUi.rowsBox->value());
	QPixmap pix(layUi.lineBox->value(),layUi.rowsBox->value());
	QPainter pnt;
	pnt.begin(&pix);
	pnt.fillRect(0,0,pix.width(),pix.height(),Qt::black);
	pnt.fillRect(layUi.hsyncBox->value(),
				 layUi.vsyncBox->value(),
				 layUi.lineBox->value() - layUi.hsyncBox->value(),
				 layUi.rowsBox->value() - layUi.vsyncBox->value(),
				 Qt::blue);
	pnt.fillRect(layUi.hsyncBox->value() + layUi.brdLBox->value(),
				 layUi.vsyncBox->value() + layUi.brdUBox->value(),
				 256,192,Qt::gray);
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
	nlay.name = std::string(layUi.layName->text().toLocal8Bit().data());
	nlay.full.h = layUi.lineBox->value();
	nlay.full.v = layUi.rowsBox->value();
	nlay.bord.h = layUi.hsyncBox->value() + layUi.brdLBox->value();
	nlay.bord.v = layUi.vsyncBox->value() + layUi.brdUBox->value();
	nlay.sync.h = layUi.hsyncBox->value();
	nlay.sync.v = layUi.vsyncBox->value();
	nlay.intpos.h = layUi.intPosBox->value();
	nlay.intpos.v = layUi.intRowBox->value();
	nlay.intsz = layUi.intLenBox->value();
	if (eidx < 0) {
		addLayout(nlay);
		ui.geombox->addItem(QString::fromLocal8Bit(nlay.name.c_str()));
		ui.geombox->setCurrentIndex(ui.geombox->count() - 1);
	} else {
		if (conf.layList[eidx].name != nlay.name)
			prfChangeLayName(conf.layList[eidx].name, nlay.name);
		conf.layList[eidx] = nlay;
		ui.geombox->setItemText(eidx, nlay.name.c_str());
	}
	layeditor->hide();
}

// ROMSETS

void SetupWin::rsNameCheck(QString nam) {
	rseUi.rse_apply->setEnabled(!rseUi.rsName->text().isEmpty());
	xRomset rs;
	for (uint i = 0; i < conf.rsList.size(); i++) {
		if ((QString(conf.rsList[i].name.c_str()) == nam) && (eidx != (int)i)) {
			rseUi.rse_apply->setEnabled(false);
		}
	}
}

void SetupWin::rmRomset() {
	int idx = ui.rsetbox->currentIndex();
	if (idx < 0) return;
	if (areSure("Do you really want to delete this romset?")) {
		conf.rsList.erase(conf.rsList.begin() + idx);		// NOTE: should be moved to xcore/romsets.cpp as delRomset?
		ui.rsetbox->removeItem(idx);
	}
}

void SetupWin::addNewRomset() {
	nrs.name.clear();
	uint i;
	for (i = 0; i < 8; i++) {
		nrs.roms[i].path = "";
		nrs.roms[i].part = 0;
	}
	nrs.gsFile = "";
	nrs.fntFile = "";
	eidx = -1;
	editRomset();
}

// machine

void SetupWin::editrset() {
	eidx = ui.rsetbox->currentIndex();
	if (eidx < 0) return;
	nrs = conf.rsList[eidx];
	editRomset();
}

void SetupWin::editRomset() {
	QDir rdir(QString(conf.path.romDir.c_str()));
	QStringList rlst = rdir.entryList(QStringList() << "*.rom",QDir::Files,QDir::Name);
	fillRFBox(rseUi.rse_singlefile,rlst);
	fillRFBox(rseUi.rse_file0,rlst);
	fillRFBox(rseUi.rse_file1,rlst);
	fillRFBox(rseUi.rse_file2,rlst);
	fillRFBox(rseUi.rse_file3,rlst);
	fillRFBox(rseUi.rse_gsfile,rlst);
	fillRFBox(rseUi.rse_fntfile,rlst);
	rseUi.rsName->setText(nrs.name.c_str());
	rseUi.rse_singlefile->setCurrentIndex(rlst.indexOf(QString(nrs.file.c_str())) + 1);
	rseUi.rse_file0->setCurrentIndex(rlst.indexOf(QString(nrs.roms[0].path.c_str())) + 1);
	rseUi.rse_file1->setCurrentIndex(rlst.indexOf(QString(nrs.roms[1].path.c_str())) + 1);
	rseUi.rse_file2->setCurrentIndex(rlst.indexOf(QString(nrs.roms[2].path.c_str())) + 1);
	rseUi.rse_file3->setCurrentIndex(rlst.indexOf(QString(nrs.roms[3].path.c_str())) + 1);
	rseUi.rse_part0->setValue(nrs.roms[0].part);
	rseUi.rse_part1->setValue(nrs.roms[1].part);
	rseUi.rse_part2->setValue(nrs.roms[2].part);
	rseUi.rse_part3->setValue(nrs.roms[3].part);
	rseUi.rse_gsfile->setCurrentIndex(rlst.indexOf(QString(nrs.gsFile.c_str())) + 1);
	rseUi.rse_fntfile->setCurrentIndex(rlst.indexOf(QString(nrs.fntFile.c_str())) + 1);
	rseUi.rse_grp_single->setChecked(nrs.file != "");
	rseditor->show();
	rsNameCheck(rseUi.rsName->text());
}

// Romset editor OK pressed
void SetupWin::setrpart() {
	if (rseUi.rse_grp_single->isChecked()) {
		nrs.file = getRFText(rseUi.rse_singlefile);
	} else {
		nrs.file = "";
	}
	nrs.roms[0].path = getRFText(rseUi.rse_file0);
	nrs.roms[0].part = rseUi.rse_part0->value();
	nrs.roms[1].path = getRFText(rseUi.rse_file1);
	nrs.roms[1].part = rseUi.rse_part1->value();
	nrs.roms[2].path = getRFText(rseUi.rse_file2);
	nrs.roms[2].part = rseUi.rse_part2->value();
	nrs.roms[3].path = getRFText(rseUi.rse_file3);
	nrs.roms[3].part = rseUi.rse_part3->value();
	nrs.gsFile = getRFText(rseUi.rse_gsfile);
	nrs.fntFile = getRFText(rseUi.rse_fntfile);
	nrs.name = std::string(rseUi.rsName->text().toLocal8Bit().data());
	if (eidx < 0) {
		addRomset(nrs);
		ui.rsetbox->addItem(QString::fromLocal8Bit(nrs.name.c_str()));
		ui.rsetbox->setCurrentIndex(ui.rsetbox->count() - 1);
	} else {
		prfChangeRsName(conf.rsList[eidx].name, nrs.name);
		conf.rsList[eidx] = nrs;
		ui.rsetbox->setItemText(eidx, nrs.name.c_str());
	}
	rseditor->hide();
}

// lists

void SetupWin::buildkeylist() {
	QDir dir(conf.path.confDir.c_str());
	QStringList lst = dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name);
	fillRFBox(ui.keyMapBox,lst);
}

std::vector<HardWare> getHardwareList() {
	std::vector<HardWare> res;
	int idx = 0;
	while (hwTab[idx].name != NULL) {
		res.push_back(hwTab[idx]);
		idx++;
	}
	return res;
}

void SetupWin::setmszbox(int idx) {
	std::vector<HardWare> list = getHardwareList();
	int t = list[idx].mask;
	QString oldText = ui.mszbox->currentText();
	ui.mszbox->clear();
	if (t == 0x00) {
		ui.mszbox->addItem("48K",48);
	} else {
		if (t & 1) ui.mszbox->addItem("128K",128);
		if (t & 2) ui.mszbox->addItem("256K",256);
		if (t & 4) ui.mszbox->addItem("512K",512);
		if (t & 8) ui.mszbox->addItem("1024K",1024);
		if (t & 16) ui.mszbox->addItem("2048K",2048);
		if (t & 32) ui.mszbox->addItem("4096K",4096);
	}
	ui.mszbox->setCurrentIndex(ui.mszbox->findText(oldText));
	if (ui.mszbox->currentIndex() < 0) ui.mszbox->setCurrentIndex(ui.mszbox->count() - 1);
}

void SetupWin::buildrsetlist() {
//	int i;
	if (ui.rsetbox->currentIndex() < 0) {
		ui.rstab->setEnabled(false);
		return;
	}
	ui.rstab->setEnabled(true);
	xRomset rset = conf.rsList[ui.rsetbox->currentIndex()];
	if (rset.file == "") {
		ui.rstab->hideRow(4);
		for (int i=0; i<4; i++) {
			ui.rstab->showRow(i);
			QString rsf = QString::fromLocal8Bit(rset.roms[i].path.c_str());
			ui.rstab->item(i,1)->setText(rsf);
			if (rsf != "") {
				ui.rstab->item(i,2)->setText(QString::number(rset.roms[i].part));
			} else {
				ui.rstab->item(i,2)->setText("");
			}
		}
	} else {
		ui.rstab->hideRow(0);
		ui.rstab->hideRow(1);
		ui.rstab->hideRow(2);
		ui.rstab->hideRow(3);
		ui.rstab->showRow(4);
		ui.rstab->item(4,1)->setText(QString::fromLocal8Bit(rset.file.c_str()));
		ui.rstab->item(4,2)->setText("");
	}
	ui.rstab->item(5,1)->setText(QString::fromLocal8Bit(rset.gsFile.c_str()));
	ui.rstab->item(6,1)->setText(QString::fromLocal8Bit(rset.fntFile.c_str()));
	ui.rstab->setColumnWidth(0,100);
	ui.rstab->setColumnWidth(1,300);
	ui.rstab->setColumnWidth(2,50);
}

void SetupWin::buildtapelist() {
//	buildTapeList();
	TapeBlockInfo* inf = new TapeBlockInfo[comp->tape->blkCount];
	tapGetBlocksInfo(comp->tape,inf);
	ui.tapelist->setRowCount(comp->tape->blkCount);
	if (comp->tape->blkCount == 0) {
		ui.tapelist->setEnabled(false);
		return;
	}
	ui.tapelist->setEnabled(true);
	QTableWidgetItem* itm;
	uint tm,ts;
	for (int i=0; i < comp->tape->blkCount; i++) {
		if (comp->tape->block == i) {
			itm = new QTableWidgetItem(QIcon(":/images/checkbox.png"),"");
			ui.tapelist->setItem(i,0,itm);
			ts = inf[i].curtime;
			tm = ts/60;
			ts -= tm * 60;
			itm = new QTableWidgetItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
			ui.tapelist->setItem(i,3,itm);
		} else {
			itm = new QTableWidgetItem;
			ui.tapelist->setItem(i,0,itm);
			itm = new QTableWidgetItem;
			ui.tapelist->setItem(i,3,itm);
		}
		itm = new QTableWidgetItem;
		if (inf[i].breakPoint) itm->setIcon(QIcon(":/images/cancel.png"));
		ui.tapelist->setItem(i,1,itm);
		ts = inf[i].time;
		tm = ts/60;
		ts -= tm * 60;
		itm = new QTableWidgetItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
		ui.tapelist->setItem(i,2,itm);
		itm = new QTableWidgetItem(QString::number(inf[i].size));
		ui.tapelist->setItem(i,4,itm);
		itm = new QTableWidgetItem(QString::fromLocal8Bit(inf[i].name));
		ui.tapelist->setItem(i,5,itm);
	}
	ui.tapelist->selectRow(0);
}

void SetupWin::buildmenulist() {
	ui.umlist->setRowCount(conf.bookmarkList.size());
	QTableWidgetItem* itm;
	for (uint i = 0; i < conf.bookmarkList.size(); i++) {
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
	for (uint i = 0; i < conf.prof.list.size(); i++) {
		itm = new QTableWidgetItem(QString::fromLocal8Bit(conf.prof.list[i]->name.c_str()));
		ui.twProfileList->setItem(i,0,itm);
		itm = new QTableWidgetItem(QString::fromLocal8Bit(conf.prof.list[i]->file.c_str()));
		ui.twProfileList->setItem(i,1,itm);
	}
}

void SetupWin::copyToTape() {
	int dsk = ui.disktabs->currentIndex();
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	TRFile cat[128];
	diskGetTRCatalog(comp->dif->fdc->flop[dsk],cat);
	// std::vector<TRFile> cat = flpGetTRCatalog(comp->bdi->flop[dsk]);
	int row;
	unsigned char* buf = new unsigned char[0xffff];
	unsigned short line,start,len;
	char name[10];
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		row = idx[i].row();
		if (diskGetSectorsData(comp->dif->fdc->flop[dsk],cat[row].trk, cat[row].sec+1, buf, cat[row].slen)) {
			if (cat[row].slen == (cat[row].hlen + ((cat[row].llen == 0) ? 0 : 1))) {
				start = (cat[row].hst << 8) + cat[row].lst;
				len = (cat[row].hlen << 8) + cat[row].llen;
				line = (cat[row].ext == 'B') ? (buf[start] + (buf[start+1] << 8)) : 0x8000;
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
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...",QDir::homePath());
	if (dir == "") return;
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
	std::string sdir = std::string(dir.toLocal8Bit().data()) + std::string(SLASH);
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
	TapeBlockInfo inf = tapGetBlockInfo(tape,blk);
	unsigned char* dt = new unsigned char[inf.size];
	tapGetBlockData(tape,blk,dt);
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
	return res;
}

void SetupWin::copyToDisk() {
	int blk = ui.tapelist->currentRow();
	if (blk < 0) return;
	int dsk = ui.disktabs->currentIndex();
	int headBlock = -1;
	int dataBlock = -1;
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
	TRFile dsc;
	if (headBlock < 0) {
		const char* nm = "FILE    ";
		memcpy(&dsc.name[0],nm,8);
		dsc.ext = 'C';
		dsc.lst = dsc.hst = 0;
		TapeBlockInfo binf = tapGetBlockInfo(comp->tape,dataBlock);
		int len = binf.size;
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
	if (!(comp->dif->fdc->flop[dsk]->insert)) newdisk(dsk);
	TapeBlockInfo inf = tapGetBlockInfo(comp->tape,dataBlock);
	unsigned char* dt = new unsigned char[inf.size];
	tapGetBlockData(comp->tape,dataBlock,dt);
	unsigned char* buf = new unsigned char[256];
	int pos = 1;	// skip block type mark
	switch(diskCreateDescriptor(comp->dif->fdc->flop[dsk],&dsc)) {
		case ERR_SHIT: shitHappens("Yes, it happens"); break;
		case ERR_MANYFILES: shitHappens("Too many files @ disk"); break;
		case ERR_NOSPACE: shitHappens("Not enough space @ disk"); break;
		case ERR_OK:
			while (pos < inf.size) {
				do {
					buf[(pos-1) & 0xff] = (pos < inf.size) ? dt[pos] : 0x00;
					pos++;
				} while ((pos & 0xff) != 1);
				diskPutSectorData(comp->dif->fdc->flop[dsk],dsc.trk, dsc.sec+1, buf, 256);
				dsc.sec++;
				if (dsc.sec > 15) {
					dsc.sec = 0;
					dsc.trk++;
				}
			}
			fillDiskCat();
			showInfo("File was copied");
			break;
	}
}

void SetupWin::fillDiskCat() {
	int dsk = ui.disktabs->currentIndex();
	QTableWidget* wid = ui.disklist;
	wid->setColumnWidth(0,100);
	wid->setColumnWidth(1,30);
	wid->setColumnWidth(2,70);
	wid->setColumnWidth(3,70);
	wid->setColumnWidth(4,50);
	wid->setColumnWidth(5,50);
//	wid->setColumnWidth(6,40);
	QTableWidgetItem* itm;
	if (!(comp->dif->fdc->flop[dsk]->insert)) {
		wid->setEnabled(false);
		wid->setRowCount(0);
	} else {
		wid->setEnabled(true);
		if (diskGetType(comp->dif->fdc->flop[dsk]) == DISK_TYPE_TRD) {
			TRFile cat[128];
			int catSize = diskGetTRCatalog(comp->dif->fdc->flop[dsk],cat);
			// std::vector<TRFile> cat = flpGetTRCatalog(comp->bdi->flop[dsk]);
			wid->setRowCount(catSize);
			for (int i=0; i<catSize; i++) {
				itm = new QTableWidgetItem(QString(std::string((char*)cat[i].name,8).c_str())); wid->setItem(i,0,itm);
				itm = new QTableWidgetItem(QString(QChar(cat[i].ext))); wid->setItem(i,1,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].lst + (cat[i].hst << 8))); wid->setItem(i,2,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].llen + (cat[i].hlen << 8))); wid->setItem(i,3,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].slen)); wid->setItem(i,4,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].trk)); wid->setItem(i,5,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].sec)); wid->setItem(i,6,itm);
				wid->setRowHidden(i, cat[i].name[0] < 0x20);
			}
		} else {
			wid->setEnabled(false);
			wid->setRowCount(0);
		}
	}
}

// machine

/*
void SetupWin::updfrq() {
	double f = ui.cpufrq->value() / 1e6;
	ui.sbFreq->setValue(f);
	// ui.cpufrqlab->setText(QString::number(f,'f',2).append(" MHz"));
}

void SetupWin::updfrq2() {
	double f = ui.sbFreq->value();
	ui.cpufrq->setValue(int(f * 1e6));
}
*/

// video

void SetupWin::chabsz() {ui.bszlab->setText(QString::number(ui.bszsld->value()).append("%"));}

void SetupWin::selsspath() {
	QString fpath = QFileDialog::getExistingDirectory(this,"Screenshots folder",QString::fromLocal8Bit(conf.scrShot.dir.c_str()),QFileDialog::ShowDirsOnly);
	if (fpath!="") ui.pathle->setText(fpath);
}

// sound

void SetupWin::updvolumes() {
	ui.bvlab->setText(QString::number(ui.bvsld->value()));
	ui.tvlab->setText(QString::number(ui.tvsld->value()));
	ui.avlab->setText(QString::number(ui.avsld->value()));
	ui.gslab->setText(QString::number(ui.gvsld->value()));
}

// disk

void SetupWin::newdisk(int idx) {
	Floppy *flp = comp->dif->fdc->flop[idx];
	if (!saveChangedDisk(comp,idx & 3)) return;
	diskFormat(flp);
	flp->path = (char*)realloc(flp->path,sizeof(char));
	flp->path[0] = 0x00;
	flp->insert = 1;
	flp->changed = 1;
	updatedisknams();
}

void SetupWin::newa() {newdisk(0);}
void SetupWin::newb() {newdisk(1);}
void SetupWin::newc() {newdisk(2);}
void SetupWin::newd() {newdisk(3);}

void SetupWin::loada() {loadFile(comp,"",FT_DISK,0); updatedisknams();}
void SetupWin::loadb() {loadFile(comp,"",FT_DISK,1); updatedisknams();}
void SetupWin::loadc() {loadFile(comp,"",FT_DISK,2); updatedisknams();}
void SetupWin::loadd() {loadFile(comp,"",FT_DISK,3); updatedisknams();}

void SetupWin::savea() {Floppy* flp = comp->dif->fdc->flop[0]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,0);}
void SetupWin::saveb() {Floppy* flp = comp->dif->fdc->flop[1]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,1);}
void SetupWin::savec() {Floppy* flp = comp->dif->fdc->flop[2]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,2);}
void SetupWin::saved() {Floppy* flp = comp->dif->fdc->flop[3]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,3);}

void SetupWin::ejcta() {saveChangedDisk(comp,0); flpEject(comp->dif->fdc->flop[0]); updatedisknams();}
void SetupWin::ejctb() {saveChangedDisk(comp,1); flpEject(comp->dif->fdc->flop[1]); updatedisknams();}
void SetupWin::ejctc() {saveChangedDisk(comp,2); flpEject(comp->dif->fdc->flop[2]); updatedisknams();}
void SetupWin::ejctd() {saveChangedDisk(comp,3); flpEject(comp->dif->fdc->flop[3]); updatedisknams();}

void SetupWin::updatedisknams() {
	ui.apathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[0]->path));
	ui.bpathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[1]->path));
	ui.cpathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[2]->path));
	ui.dpathle->setText(QString::fromLocal8Bit(comp->dif->fdc->flop[3]->path));
	fillDiskCat();
}

// tape

void SetupWin::loatape() {
	loadFile(comp,"",FT_TAPE,1);
	ui.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
}

void SetupWin::savtape() {
	if (comp->tape->blkCount != 0) saveFile(comp,comp->tape->path,FT_TAP,-1);
}

void SetupWin::ejctape() {
	tapEject(comp->tape);
	ui.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
}

void SetupWin::tblkup() {
	int ps = ui.tapelist->currentIndex().row();
	if (ps > 0) {
		tapSwapBlocks(comp->tape,ps,ps-1);
		buildtapelist();
		ui.tapelist->selectRow(ps-1);
	}
}

void SetupWin::tblkdn() {
	int ps = ui.tapelist->currentIndex().row();
	if ((ps != -1) && (ps < comp->tape->blkCount - 1)) {
		tapSwapBlocks(comp->tape,ps,ps+1);
		buildtapelist();
		ui.tapelist->selectRow(ps+1);
	}
}

void SetupWin::tblkrm() {
	int ps = ui.tapelist->currentIndex().row();
	if (ps != -1) {
		tapDelBlock(comp->tape,ps);
		buildtapelist();
		ui.tapelist->selectRow(ps);
	}
}

void SetupWin::chablock(QModelIndex idx) {
	int row = idx.row();
	tapRewind(comp->tape,row);
	buildtapelist();
	ui.tapelist->selectRow(row);
}

void SetupWin::setTapeBreak(int row,int col) {
	if ((row < 0) || (col != 1)) return;
	comp->tape->blkData[row].breakPoint ^= 1;
	buildtapelist();
	ui.tapelist->selectRow(row);
}

// hdd

void SetupWin::hddMasterImg() {
	QString path = QFileDialog::getOpenFileName(this,"Image for master HDD","","All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") ui.hm_path->setText(path);
}

void SetupWin::hddSlaveImg() {
	QString path = QFileDialog::getOpenFileName(this,"Image for slave HDD","","All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") ui.hs_path->setText(path);
}

void SetupWin::hddcap() {
	unsigned int sz;
	if (ui.hm_islba->checkState() == Qt::Checked) {
		sz = (ui.hm_glba->value() >> 11);
	} else {
		sz = ((ui.hm_gsec->value() * (ui.hm_ghd->value() + 1) * (ui.hm_gcyl->value() + 1)) >> 11);
	}
	ui.hm_capacity->setValue(sz);
	if (ui.hs_islba->checkState() == Qt::Checked) {
		sz = (ui.hs_glba->value() >> 11);
	} else {
		sz = ((ui.hs_gsec->value() * (ui.hs_ghd->value() + 1) * (ui.hs_gcyl->value() + 1)) >> 11);
	}
	ui.hs_capacity->setValue(sz);
}

// external

void SetupWin::selSDCimg() {
	QString fnam = QFileDialog::getOpenFileName(this,"Image for SD card","","All files (*.*)");
	if (!fnam.isEmpty()) ui.sdPath->setText(fnam);
}

void SetupWin::openSlotA() {
	QString fnam = QFileDialog::getOpenFileName(this,"MSX cartridge A","","MSX cartridge (*.rom)");
	if (fnam.isEmpty()) return;
	ui.cSlotAName->setText(fnam);
	loadFile(comp, fnam.toLocal8Bit().data(), FT_SLOT_A, 0);
}

void SetupWin::openSlotB() {
	QString fnam = QFileDialog::getOpenFileName(this,"MSX cartridge B","","MSX cartridge (*.rom)");
	if (fnam.isEmpty()) return;
	ui.cSlotAName->setText(fnam);
	loadFile(comp, fnam.toLocal8Bit().data(), FT_SLOT_B, 1);
}

void ejectSlot(xCartridge* slot) {
	free(slot->data);
	slot->data = NULL;
	slot->name[0] = 0x00;
}

void SetupWin::ejectSlotA() {
	ejectSlot(&comp->msx.slotA);
	ui.cSlotAName->clear();
}

void SetupWin::ejectSlotB() {
	ejectSlot(&comp->msx.slotB);
	ui.cSlotBName->clear();
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
		if (ps == (int)conf.bookmarkList.size()) {
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
	QString fpath = QFileDialog::getOpenFileName(NULL,"Select file","","Known formats (*.sna *.z80 *.tap *.tzx *.trd *.scl *.fdi *.udi)");
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

void SetupWin::rmProfile() {
	int idx = ui.twProfileList->currentRow();
	if (idx < 0) return;
	block = 1;
	if (areSure("Do you really want to delete this profile?")) {
		std::string pnam(ui.twProfileList->item(idx,0)->text().toLocal8Bit().data());
		idx = delProfile(pnam);
		switch(idx) {
			case DELP_OK_CURR:
				prfChanged = 1;
				start(conf.prof.cur);
				break;
			case DELP_ERR:
				shitHappens("Sorry, i can't delete this profile");
				break;
		}
	}
	block = 0;
	buildproflist();
}
