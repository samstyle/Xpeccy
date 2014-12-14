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

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "sound.h"
#include "libxpeccy/spectrum.h"
#include "setupwin.h"
#include "filer.h"
#include "filetypes/filetypes.h"

void fillRFBox(QComboBox* box, QStringList lst) {
	box->clear();
	box->addItem("none");
	box->addItems(lst);
}

std::string getRFText(QComboBox* box) {
	QString res = "";
	if (box->currentIndex() > 0) res = box->currentText();
	return std::string(res.toLocal8Bit().data());
}

// OBJECT

std::vector<std::string> getHardwareNames() {
	int idx = 0;
	std::vector<std::string> res;
	while (hwTab[idx].name != NULL) {
		res.push_back(std::string(hwTab[idx].name));
		idx++;
	}
	return res;
}

SetupWin::SetupWin(QWidget* par):QDialog(par) {
	setModal(true);
	setupUi.setupUi(this);

	umadial = new QDialog;
	uia.setupUi(umadial);
	umadial->setModal(true);

	rseditor = new QDialog(this);
	rseUi.setupUi(rseditor);
	rseditor->setModal(true);

	layeditor = new QDialog(this);
	layUi.setupUi(layeditor);
	layeditor->setModal(true);

	unsigned int i;
	std::vector<std::string> list;
// machine
	i = 0;
	while (hwTab[i].name != NULL) {
		setupUi.machbox->addItem(QString::fromLocal8Bit(hwTab[i].optName),QString::fromLocal8Bit(hwTab[i].name));
		i++;
	}
	setupUi.resbox->addItem("BASIC 48",RES_48);
	setupUi.resbox->addItem("BASIC 128",RES_128);
	setupUi.resbox->addItem("DOS",RES_DOS);
	setupUi.resbox->addItem("SHADOW",RES_SHADOW);
//	setupUi.resbox->addItems(QStringList()<<"ROMPage0"<<"ROMPage1"<<"ROMPage2"<<"ROMPage3");
//	setupUi.rssel->hide();
	QTableWidgetItem* itm;
	for (i = 0; i < (unsigned)setupUi.rstab->rowCount(); i++) {
		itm = new QTableWidgetItem; setupUi.rstab->setItem(i,1,itm);
		itm = new QTableWidgetItem; setupUi.rstab->setItem(i,2,itm);
	}
// video
	std::map<std::string,int>::iterator it;
	for (it = shotFormat.begin(); it != shotFormat.end(); it++) {
		setupUi.ssfbox->addItem(QString(it->first.c_str()),it->second);

	}
// sound
	i = 0;
	while (sndTab[i].name != NULL) {
		setupUi.outbox->addItem(QString::fromLocal8Bit(sndTab[i].name));
		i++;
	}
	setupUi.ratbox->addItem("48000",48000);
	setupUi.ratbox->addItem("44100",44100);
	setupUi.ratbox->addItem("22050",22050);
	setupUi.ratbox->addItem("11025",11025);
	setupUi.schip1box->addItem(QIcon(":/images/cancel.png"),"none",SND_NONE);
	setupUi.schip1box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",SND_AY);
	setupUi.schip1box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",SND_YM);
	setupUi.schip2box->addItem(QIcon(":/images/cancel.png"),"none",SND_NONE);
	setupUi.schip2box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",SND_AY);
	setupUi.schip2box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",SND_YM);
#ifdef ISDEBUG
	setupUi.schip1box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2203",SND_YM2203);
	setupUi.schip2box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2203",SND_YM2203);
#endif
	setupUi.stereo1box->addItem("Mono",AY_MONO); setupUi.stereo2box->addItem("Mono",AY_MONO);
	setupUi.stereo1box->addItem("ABC",AY_ABC); setupUi.stereo2box->addItem("ABC",AY_ABC);
	setupUi.stereo1box->addItem("ACB",AY_ACB); setupUi.stereo2box->addItem("ACB",AY_ACB);
	setupUi.stereo1box->addItem("BAC",AY_BAC); setupUi.stereo2box->addItem("BAC",AY_BAC);
	setupUi.stereo1box->addItem("BCA",AY_BCA); setupUi.stereo2box->addItem("BCA",AY_BCA);
	setupUi.stereo1box->addItem("CAB",AY_CAB); setupUi.stereo2box->addItem("CAB",AY_CAB);
	setupUi.stereo1box->addItem("CBA",AY_CBA); setupUi.stereo2box->addItem("CBA",AY_BCA);
	setupUi.tsbox->addItem("None",TS_NONE);
	setupUi.tsbox->addItem("NedoPC",TS_NEDOPC);
	setupUi.gstereobox->addItem("Mono",GS_MONO);
	setupUi.gstereobox->addItem("L:1,2; R:3,4",GS_12_34);
	setupUi.sdrvBox->addItem("none",SDRV_NONE);
	setupUi.sdrvBox->addItem("Covox only",SDRV_COVOX);
	setupUi.sdrvBox->addItem("Soundrive 1.05 mode 1",SDRV_105_1);
	setupUi.sdrvBox->addItem("Soundrive 1.05 mode 2",SDRV_105_2);
// bdi
// WTF? QtDesigner doesn't save this properties
	setupUi.disklist->horizontalHeader()->setVisible(true);
	setupUi.diskTypeBox->addItem("None",FDC_NONE);
	setupUi.diskTypeBox->addItem("Beta disk (VG93)",FDC_93);
	setupUi.diskTypeBox->addItem("+3 DOS (uPD765)",FDC_765);
	setupUi.disklist->addAction(setupUi.actCopyToTape);
	setupUi.disklist->addAction(setupUi.actSaveHobeta);
	setupUi.disklist->addAction(setupUi.actSaveRaw);
// tape
	setupUi.tapelist->setColumnWidth(0,20);
	setupUi.tapelist->setColumnWidth(1,20);
	setupUi.tapelist->setColumnWidth(2,50);
	setupUi.tapelist->setColumnWidth(3,50);
	setupUi.tapelist->setColumnWidth(4,100);
	setupUi.tapelist->addAction(setupUi.actCopyToDisk);
// hdd
	setupUi.hiface->addItem("None",IDE_NONE);
	setupUi.hiface->addItem("Nemo",IDE_NEMO);
	setupUi.hiface->addItem("Nemo A8",IDE_NEMOA8);
	setupUi.hiface->addItem("Nemo Evo",IDE_NEMO_EVO);
	setupUi.hiface->addItem("SMUC",IDE_SMUC);
	setupUi.hiface->addItem("ATM",IDE_ATM);
	setupUi.hm_type->addItem(QIcon(":/images/cancel.png"),"Not connected",IDE_NONE);
	setupUi.hm_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",IDE_ATA);
//	setupUi.hm_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",IDE_ATAPI);
	setupUi.hs_type->addItem(QIcon(":/images/cancel.png"),"Not connected",IDE_NONE);
	setupUi.hs_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",IDE_ATA);
//	setupUi.hs_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",IDE_ATAPI);
// sdcard
	setupUi.sdcapbox->addItem("32 M",SDC_32M);
	setupUi.sdcapbox->addItem("64 M",SDC_64M);
	setupUi.sdcapbox->addItem("128 M",SDC_128M);
	setupUi.sdcapbox->addItem("256 M",SDC_256M);
	setupUi.sdcapbox->addItem("512 M",SDC_512M);
	setupUi.sdcapbox->addItem("1024 M",SDC_1G);

// all
	connect(setupUi.okbut,SIGNAL(released()),this,SLOT(okay()));
	connect(setupUi.apbut,SIGNAL(released()),this,SLOT(apply()));
	connect(setupUi.cnbut,SIGNAL(released()),this,SLOT(reject()));
// machine
	connect(setupUi.rsetbox,SIGNAL(currentIndexChanged(int)),this,SLOT(buildrsetlist()));
	connect(setupUi.machbox,SIGNAL(currentIndexChanged(int)),this,SLOT(setmszbox(int)));
	connect(setupUi.cpufrq,SIGNAL(valueChanged(int)),this,SLOT(updfrq()));
	connect(setupUi.rstab,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editrset()));
	connect(setupUi.addrset,SIGNAL(released()),this,SLOT(addNewRomset()));
	connect(setupUi.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
	connect(setupUi.rsedit,SIGNAL(released()),this,SLOT(editrset()));

	connect(rseUi.rse_cancel,SIGNAL(released()),rseditor,SLOT(hide()));
	connect(rseUi.rse_apply,SIGNAL(released()),this,SLOT(setrpart()));
	connect(rseUi.rse_grp_single,SIGNAL(toggled(bool)),this,SLOT(recheck_separate(bool)));
	connect(rseUi.rse_grp_separate,SIGNAL(toggled(bool)),this,SLOT(recheck_single(bool)));
	connect(rseUi.rsName,SIGNAL(textChanged(QString)),this,SLOT(rsNameCheck(QString)));
// video
	connect(setupUi.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	connect(setupUi.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));

	connect(setupUi.layEdit,SIGNAL(released()),this,SLOT(edLayout()));
	connect(setupUi.layAdd,SIGNAL(released()),this,SLOT(addNewLayout()));
	connect(setupUi.layDel,SIGNAL(released()),this,SLOT(delLayout()));

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
	connect(setupUi.bvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	connect(setupUi.tvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	connect(setupUi.avsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	connect(setupUi.gvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
// dos
	connect(setupUi.newatb,SIGNAL(released()),this,SLOT(newa()));
	connect(setupUi.newbtb,SIGNAL(released()),this,SLOT(newb()));
	connect(setupUi.newctb,SIGNAL(released()),this,SLOT(newc()));
	connect(setupUi.newdtb,SIGNAL(released()),this,SLOT(newd()));

	connect(setupUi.loadatb,SIGNAL(released()),this,SLOT(loada()));
	connect(setupUi.loadbtb,SIGNAL(released()),this,SLOT(loadb()));
	connect(setupUi.loadctb,SIGNAL(released()),this,SLOT(loadc()));
	connect(setupUi.loaddtb,SIGNAL(released()),this,SLOT(loadd()));

	connect(setupUi.saveatb,SIGNAL(released()),this,SLOT(savea()));
	connect(setupUi.savebtb,SIGNAL(released()),this,SLOT(saveb()));
	connect(setupUi.savectb,SIGNAL(released()),this,SLOT(savec()));
	connect(setupUi.savedtb,SIGNAL(released()),this,SLOT(saved()));

	connect(setupUi.remoatb,SIGNAL(released()),this,SLOT(ejcta()));
	connect(setupUi.remobtb,SIGNAL(released()),this,SLOT(ejctb()));
	connect(setupUi.remoctb,SIGNAL(released()),this,SLOT(ejctc()));
	connect(setupUi.remodtb,SIGNAL(released()),this,SLOT(ejctd()));

	connect(setupUi.disktabs,SIGNAL(currentChanged(int)),this,SLOT(fillDiskCat()));
	connect(setupUi.actCopyToTape,SIGNAL(triggered()),this,SLOT(copyToTape()));
	connect(setupUi.actSaveHobeta,SIGNAL(triggered()),this,SLOT(diskToHobeta()));
	connect(setupUi.actSaveRaw,SIGNAL(triggered()),this,SLOT(diskToRaw()));
	connect(setupUi.tbToTape,SIGNAL(released()),this,SLOT(copyToTape()));
	connect(setupUi.tbToHobeta,SIGNAL(released()),this,SLOT(diskToHobeta()));
	connect(setupUi.tbToRaw,SIGNAL(released()),this,SLOT(diskToRaw()));
// tape
	connect(setupUi.tapelist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chablock(QModelIndex)));
	connect(setupUi.tapelist,SIGNAL(cellClicked(int,int)),this,SLOT(setTapeBreak(int,int)));
	connect(setupUi.tloadtb,SIGNAL(released()),this,SLOT(loatape()));
	connect(setupUi.tsavetb,SIGNAL(released()),this,SLOT(savtape()));
	connect(setupUi.tremotb,SIGNAL(released()),this,SLOT(ejctape()));
	connect(setupUi.blkuptb,SIGNAL(released()),this,SLOT(tblkup()));
	connect(setupUi.blkdntb,SIGNAL(released()),this,SLOT(tblkdn()));
	connect(setupUi.blkrmtb,SIGNAL(released()),this,SLOT(tblkrm()));
	connect(setupUi.actCopyToDisk,SIGNAL(triggered()),this,SLOT(copyToDisk()));
	connect(setupUi.tbToDisk,SIGNAL(released()),this,SLOT(copyToDisk()));
// hdd
	connect(setupUi.hm_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hm_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hm_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hm_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hm_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hm_pathtb,SIGNAL(released()),this,SLOT(hddMasterImg()));
	connect(setupUi.hs_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hs_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hs_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hs_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hs_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hs_pathtb,SIGNAL(released()),this,SLOT(hddSlaveImg()));
// sdc
	connect(setupUi.tbSDCimg,SIGNAL(released()),this,SLOT(selSDCimg()));
	connect(setupUi.tbsdcfree,SIGNAL(released()),setupUi.sdPath,SLOT(clear()));
//tools
	connect(setupUi.umlist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(umedit(QModelIndex)));
	connect(setupUi.umaddtb,SIGNAL(released()),this,SLOT(umadd()));
	connect(setupUi.umdeltb,SIGNAL(released()),this,SLOT(umdel()));
	connect(setupUi.umuptb,SIGNAL(released()),this,SLOT(umup()));
	connect(setupUi.umdntb,SIGNAL(released()),this,SLOT(umdn()));
// bookmark add dialog
	connect(uia.umasptb,SIGNAL(released()),this,SLOT(umaselp()));
	connect(uia.umaok,SIGNAL(released()),this,SLOT(umaconf()));
	connect(uia.umacn,SIGNAL(released()),umadial,SLOT(hide()));
// profiles manager
	connect(setupUi.tbNewProfile,SIGNAL(released()),this,SLOT(newProfile()));
	connect(setupUi.tbDelProfile,SIGNAL(released()),this,SLOT(rmProfile()));

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

void SetupWin::start(ZXComp* c) {
	comp = c;
	unsigned int i;
	xProfile* curProf = getCurrentProfile();
	xRomset* rset = findRomset(curProf->rsName);
// machine
	setupUi.rsetbox->clear();
	for (i=0; i < rsList.size(); i++) {
		setupUi.rsetbox->addItem(QString::fromLocal8Bit(rsList[i].name.c_str()));
	}
	setupUi.machbox->setCurrentIndex(setupUi.machbox->findData(QString::fromUtf8(comp->hw->name)));
	int cbx = -1;
	if (rset != NULL)
		cbx = setupUi.rsetbox->findText(QString::fromUtf8(rset->name.c_str()));
	setupUi.rsetbox->setCurrentIndex(cbx);
	setupUi.resbox->setCurrentIndex(setupUi.resbox->findData(comp->resbank));
	setmszbox(setupUi.machbox->currentIndex());
	setupUi.mszbox->setCurrentIndex(setupUi.mszbox->findData(comp->mem->memSize));
	if (setupUi.mszbox->currentIndex() < 0) setupUi.mszbox->setCurrentIndex(setupUi.mszbox->count() - 1);
	setupUi.cpufrq->setValue(comp->cpuFrq * 2); updfrq();
	setupUi.scrpwait->setChecked(comp->scrpWait);
	setupUi.sysCmos->setChecked(conf.sysclock);
// video
	setupUi.dszchk->setChecked(conf.vid.doubleSize);
	setupUi.fscchk->setChecked(conf.vid.fullScreen);
	setupUi.noflichk->setChecked(conf.vid.noFlick);
	setupUi.grayscale->setChecked(conf.vid.grayScale);
	setupUi.border4T->setChecked(comp->vid->border4t);
	setupUi.contMem->setChecked(comp->contMem);
	setupUi.contIO->setChecked(comp->contIO);
	setupUi.bszsld->setValue((int)(conf.brdsize * 100));
	setupUi.pathle->setText(QString::fromLocal8Bit(conf.scrShot.dir.c_str()));
	setupUi.ssfbox->setCurrentIndex(setupUi.ssfbox->findText(conf.scrShot.format.c_str()));
	setupUi.scntbox->setValue(conf.scrShot.count);
	setupUi.sintbox->setValue(conf.scrShot.interval);
	setupUi.geombox->clear();
	for (i=0; i<layList.size(); i++) {
		setupUi.geombox->addItem(QString::fromLocal8Bit(layList[i].name.c_str()));
	}
	setupUi.geombox->setCurrentIndex(setupUi.geombox->findText(QString::fromLocal8Bit(getCurrentProfile()->layName.c_str())));
	setupUi.ulaPlus->setChecked(comp->vid->ula->enabled);
// sound
	setupUi.senbox->setChecked(conf.snd.enabled);
	setupUi.mutbox->setChecked(conf.snd.mute);
	setupUi.gsrbox->setChecked(comp->gs->reset);
	setupUi.outbox->setCurrentIndex(setupUi.outbox->findText(QString::fromLocal8Bit(sndOutput->name)));
	setupUi.ratbox->setCurrentIndex(setupUi.ratbox->findData(QVariant(conf.snd.rate)));
	setupUi.bvsld->setValue(conf.snd.vol.beep);
	setupUi.tvsld->setValue(conf.snd.vol.tape);
	setupUi.avsld->setValue(conf.snd.vol.ay);
	setupUi.gvsld->setValue(conf.snd.vol.gs);
	setupUi.schip1box->setCurrentIndex(setupUi.schip1box->findData(QVariant(comp->ts->chipA->type)));
	setupUi.schip2box->setCurrentIndex(setupUi.schip2box->findData(QVariant(comp->ts->chipB->type)));
	setupUi.stereo1box->setCurrentIndex(setupUi.stereo1box->findData(QVariant(comp->ts->chipA->stereo)));
	setupUi.stereo2box->setCurrentIndex(setupUi.stereo2box->findData(QVariant(comp->ts->chipB->stereo)));
	setupUi.gstereobox->setCurrentIndex(setupUi.gstereobox->findData(QVariant(comp->gs->stereo)));
	setupUi.gsgroup->setChecked(comp->gs->enable);
	setupUi.tsbox->setCurrentIndex(setupUi.tsbox->findData(QVariant(comp->ts->type)));
	setupUi.sdrvBox->setCurrentIndex(setupUi.sdrvBox->findData(QVariant(comp->sdrv->type)));
// input
	buildkeylist();
	// buildjmaplist();
	int idx = setupUi.keyMapBox->findText(QString(conf.keyMapName.c_str()));
	if (idx < 1) idx = 0;
	setupUi.keyMapBox->setCurrentIndex(idx);
	setupUi.ratEnable->setChecked(comp->mouse->enable);
	setupUi.ratWheel->setChecked(comp->mouse->hasWheel);
	setupUi.cbSwapButtons->setChecked(comp->mouse->swapButtons);
	// setupUi.joyBox->setVisible(false);
// dos
	setupUi.diskTypeBox->setCurrentIndex(setupUi.diskTypeBox->findData(comp->bdi->fdc->type));
	setupUi.bdtbox->setChecked(fdcFlag & FDC_FAST);
	setupUi.mempaths->setChecked(conf.storePaths);
	Floppy* flp = comp->bdi->fdc->flop[0];
	setupUi.apathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.a80box->setChecked(flp->trk80);
		setupUi.adsbox->setChecked(flp->doubleSide);
		setupUi.awpbox->setChecked(flp->protect);
	flp = comp->bdi->fdc->flop[1];
	setupUi.bpathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.b80box->setChecked(flp->trk80);
		setupUi.bdsbox->setChecked(flp->doubleSide);
		setupUi.bwpbox->setChecked(flp->protect);
	flp = comp->bdi->fdc->flop[2];
	setupUi.cpathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.c80box->setChecked(flp->trk80);
		setupUi.cdsbox->setChecked(flp->doubleSide);
		setupUi.cwpbox->setChecked(flp->protect);
	flp = comp->bdi->fdc->flop[3];
	setupUi.dpathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.d80box->setChecked(flp->trk80);
		setupUi.ddsbox->setChecked(flp->doubleSide);
		setupUi.dwpbox->setChecked(flp->protect);
	fillDiskCat();
// hdd
	setupUi.hiface->setCurrentIndex(setupUi.hiface->findData(comp->ide->type));

	setupUi.hm_type->setCurrentIndex(setupUi.hm_type->findData(comp->ide->master->type));
	ATAPassport pass = ideGetPassport(comp->ide,IDE_MASTER);
//	setupUi.hm_model->setText(QString::fromLocal8Bit(pass.model));
//	setupUi.hm_ser->setText(QString::fromLocal8Bit(pass.serial));
	setupUi.hm_path->setText(QString::fromLocal8Bit(comp->ide->master->image));
	setupUi.hm_islba->setChecked(comp->ide->master->hasLBA);
	setupUi.hm_gsec->setValue(pass.spt);
	setupUi.hm_ghd->setValue(pass.hds);
	setupUi.hm_gcyl->setValue(pass.cyls);
	setupUi.hm_glba->setValue(comp->ide->master->maxlba);

	setupUi.hs_type->setCurrentIndex(setupUi.hm_type->findData(comp->ide->slave->type));
	pass = ideGetPassport(comp->ide,IDE_SLAVE);
//	setupUi.hs_model->setText(QString::fromLocal8Bit(pass.model));
//	setupUi.hs_ser->setText(QString::fromLocal8Bit(pass.serial));
	setupUi.hs_path->setText(QString::fromLocal8Bit(comp->ide->slave->image));
	setupUi.hs_islba->setChecked(comp->ide->slave->hasLBA);
	setupUi.hs_gsec->setValue(pass.spt);
	setupUi.hs_ghd->setValue(pass.hds);
	setupUi.hs_gcyl->setValue(pass.cyls);
	setupUi.hs_glba->setValue(comp->ide->slave->maxlba);
// sdcard
	setupUi.sdPath->setText(QString::fromLocal8Bit(comp->sdc->image));
	setupUi.sdcapbox->setCurrentIndex(setupUi.sdcapbox->findData(comp->sdc->capacity));
	if (setupUi.sdcapbox->currentIndex() < 0) setupUi.sdcapbox->setCurrentIndex(2);	// 128M
	setupUi.sdlock->setChecked(comp->sdc->flag & SDC_LOCK);
// tape
	setupUi.cbTapeAuto->setChecked(conf.tape.autostart);
	setupUi.cbTapeFast->setChecked(conf.tape.fast);
	setupUi.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
// tools
	buildmenulist();
// leds
	setupUi.cbMouseLed->setChecked(conf.led.mouse);
	setupUi.cbJoyLed->setChecked(conf.led.joy);
	setupUi.cbKeysLed->setChecked(conf.led.keys);
// profiles
	setupUi.defstart->setChecked(conf.defProfile);
	buildproflist();

	show();
}

void SetupWin::apply() {
	xProfile* curProf = getCurrentProfile();
// machine
//	setRomsetList(rsList);
	HardWare *oldmac = comp->hw;
	curProf->hwName = std::string(setupUi.machbox->itemData(setupUi.machbox->currentIndex()).toString().toUtf8().data());
//	curProf->hwName = std::string(setupUi.machbox->currentText().toUtf8().data());
	zxSetHardware(curProf->zx,curProf->hwName.c_str());
	curProf->rsName = std::string(setupUi.rsetbox->currentText().toUtf8().data());
	prfSetRomset("", curProf->rsName);
//	RomSet* rset = findRomset(curProf->rsName);
//	emulSetFlag(FL_RESET, setupUi.reschk->isChecked());
	comp->resbank = setupUi.resbox->itemData(setupUi.resbox->currentIndex()).toInt();
	memSetSize(comp->mem,setupUi.mszbox->itemData(setupUi.mszbox->currentIndex()).toInt());
	zxSetFrq(comp,setupUi.cpufrq->value() / 2.0);
	comp->scrpWait = setupUi.scrpwait->isChecked() ? 1 : 0;
	if (comp->hw != oldmac) zxReset(comp,RES_DEFAULT);
	conf.sysclock = setupUi.sysCmos->isChecked() ? 1 : 0;
// video
//	setLayoutList(layList);
	conf.vid.doubleSize = setupUi.dszchk->isChecked() ? 1 : 0;
	conf.vid.fullScreen = setupUi.fscchk->isChecked() ? 1 : 0;
	conf.vid.noFlick = setupUi.noflichk->isChecked() ? 1 : 0;
	conf.vid.grayScale = setupUi.grayscale->isChecked() ? 1 : 0;
	conf.scrShot.dir = std::string(setupUi.pathle->text().toLocal8Bit().data());
	conf.scrShot.format = std::string(setupUi.ssfbox->currentText().toLocal8Bit().data());
	conf.scrShot.count = setupUi.scntbox->value();
	conf.scrShot.interval = setupUi.sintbox->value();
	conf.brdsize = setupUi.bszsld->value()/100.0;
	comp->vid->border4t = setupUi.border4T->isChecked() ? 1 : 0;
	comp->contMem = setupUi.contMem->isChecked() ? 1 : 0;
	comp->contIO = setupUi.contIO->isChecked() ? 1 : 0;
	comp->vid->ula->enabled = setupUi.ulaPlus->isChecked() ? 1 : 0;
	prfSetLayout(NULL, std::string(setupUi.geombox->currentText().toLocal8Bit().data()));
// sound
	//std::string oname = std::string(sndOutput->name);
	std::string nname(setupUi.outbox->currentText().toLocal8Bit().data());
	conf.snd.enabled = setupUi.senbox->isChecked() ? 1 : 0;
	conf.snd.mute = setupUi.mutbox->isChecked() ? 1 : 0;
	conf.snd.rate = setupUi.ratbox->itemData(setupUi.ratbox->currentIndex()).toInt();
	conf.snd.vol.beep = setupUi.bvsld->value();
	conf.snd.vol.tape = setupUi.tvsld->value();
	conf.snd.vol.ay = setupUi.avsld->value();
	conf.snd.vol.gs = setupUi.gvsld->value();
	setOutput(nname.c_str());
	aymSetType(comp->ts->chipA,setupUi.schip1box->itemData(setupUi.schip1box->currentIndex()).toInt());
	aymSetType(comp->ts->chipB,setupUi.schip2box->itemData(setupUi.schip2box->currentIndex()).toInt());
	comp->ts->chipA->stereo = setupUi.stereo1box->itemData(setupUi.stereo1box->currentIndex()).toInt();
	comp->ts->chipB->stereo = setupUi.stereo2box->itemData(setupUi.stereo2box->currentIndex()).toInt();
	comp->ts->type = setupUi.tsbox->itemData(setupUi.tsbox->currentIndex()).toInt();
	comp->gs->enable = setupUi.gsgroup->isChecked() ? 1 : 0;
	comp->gs->reset = setupUi.gsrbox->isChecked() ? 1 : 0;
	comp->gs->stereo = setupUi.gstereobox->itemData(setupUi.gstereobox->currentIndex()).toInt();
	comp->sdrv->type = setupUi.sdrvBox->itemData(setupUi.sdrvBox->currentIndex()).toInt();
// input
	comp->mouse->enable = setupUi.ratEnable->isChecked() ? 1 : 0;
	comp->mouse->hasWheel = setupUi.ratWheel->isChecked() ? 1 : 0;
	comp->mouse->swapButtons = setupUi.cbSwapButtons->isChecked() ? 1 : 0;
	//if (setupUi.inpDevice->currentIndex() < 1) {
	//	optSet(OPT_JOYNAME,std::string(""));
	//} else {
	//	optSet(OPT_JOYNAME,std::string(setupUi.inpDevice->currentText().toLocal8Bit().data()));
	//}
	std::string kmname = getRFText(setupUi.keyMapBox);
	if (kmname == "none") kmname = "default";
	conf.keyMapName = kmname;
	loadKeys();
// bdi
	comp->bdi->fdc->type = setupUi.diskTypeBox->itemData(setupUi.diskTypeBox->currentIndex()).toInt();
	setFlagBit(setupUi.bdtbox->isChecked(),&fdcFlag,FDC_FAST);
	conf.storePaths = setupUi.mempaths->isChecked() ? 1 : 0;

	Floppy* flp = comp->bdi->fdc->flop[0];
	flp->trk80 = setupUi.a80box->isChecked() ? 1 : 0;
	flp->doubleSide = setupUi.adsbox->isChecked() ? 1 : 0;
	flp->protect = setupUi.awpbox->isChecked() ? 1 : 0;

	flp = comp->bdi->fdc->flop[1];
	flp->trk80 = setupUi.b80box->isChecked() ? 1 : 0;
	flp->doubleSide = setupUi.bdsbox->isChecked() ? 1 : 0;
	flp->protect = setupUi.bwpbox->isChecked() ? 1 : 0;

	flp = comp->bdi->fdc->flop[2];
	flp->trk80 = setupUi.c80box->isChecked() ? 1 : 0;
	flp->doubleSide = setupUi.cdsbox->isChecked() ? 1 : 0;
	flp->protect = setupUi.cwpbox->isChecked() ? 1 : 0;

	flp = comp->bdi->fdc->flop[3];
	flp->trk80 = setupUi.d80box->isChecked() ? 1 : 0;
	flp->doubleSide = setupUi.ddsbox->isChecked() ? 1 : 0;
	flp->protect = setupUi.dwpbox->isChecked() ? 1 : 0;

// hdd
	comp->ide->type = setupUi.hiface->itemData(setupUi.hiface->currentIndex()).toInt();

	ATAPassport pass = ideGetPassport(comp->ide,IDE_MASTER);
	comp->ide->master->type = setupUi.hm_type->itemData(setupUi.hm_type->currentIndex()).toInt();
	ideSetImage(comp->ide,IDE_MASTER,setupUi.hm_path->text().toLocal8Bit().data());
	comp->ide->master->hasLBA = setupUi.hm_islba->isChecked() ? 1 : 0;
	pass.spt = setupUi.hm_gsec->value();
	pass.hds = setupUi.hm_ghd->value();
	pass.cyls = setupUi.hm_gcyl->value();
	comp->ide->master->maxlba = setupUi.hm_glba->value();
	ideSetPassport(comp->ide,IDE_MASTER,pass);

	pass = ideGetPassport(comp->ide,IDE_SLAVE);
	comp->ide->slave->type = setupUi.hs_type->itemData(setupUi.hs_type->currentIndex()).toInt();
	ideSetImage(comp->ide,IDE_SLAVE,setupUi.hs_path->text().toLocal8Bit().data());
	comp->ide->slave->hasLBA = setupUi.hs_islba->isChecked() ? 1 : 0;
	pass.spt = setupUi.hs_gsec->value();
	pass.hds = setupUi.hs_ghd->value();
	pass.cyls = setupUi.hs_gcyl->value();
	comp->ide->slave->maxlba = setupUi.hs_glba->value();
	ideSetPassport(comp->ide,IDE_SLAVE,pass);
// sdcard
	sdcSetImage(comp->sdc,setupUi.sdPath->text().isEmpty() ? "" : setupUi.sdPath->text().toLocal8Bit().data());
	sdcSetCapacity(comp->sdc,setupUi.sdcapbox->itemData(setupUi.sdcapbox->currentIndex()).toInt());
	setFlagBit(setupUi.sdlock->isChecked(),&comp->sdc->flag,SDC_LOCK);
// tape
	conf.tape.autostart = setupUi.cbTapeAuto->isChecked() ? 1 : 0;
	conf.tape.fast = setupUi.cbTapeFast->isChecked() ? 1 : 0;
// leds
	conf.led.mouse = setupUi.cbMouseLed->isChecked();
	conf.led.joy = setupUi.cbJoyLed->isChecked();
	conf.led.keys = setupUi.cbKeysLed->isChecked();
// profiles
	conf.defProfile = setupUi.defstart->isChecked() ? 1 : 0;

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
	for (uint i = 0; i < layList.size(); i++) {
		if ((QString(layList[i].name.c_str()) == nam) && (eidx != (int)i)) {
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
	eidx = setupUi.geombox->currentIndex();
	nlay = layList[eidx];
	editLayout();
}

void SetupWin::delLayout() {
	int eidx = setupUi.geombox->currentIndex();
	if (eidx < 1) {
		shitHappens("You can't delete this layout");
		return;
	}
	if (areSure("Do you really want to delete this layout?")) {
		layList.erase(layList.begin() + eidx);
		setupUi.geombox->removeItem(eidx);
	}
}

void SetupWin::addNewLayout() {
	eidx = -1;
	nlay = layList[0];
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
		setupUi.geombox->addItem(QString::fromLocal8Bit(nlay.name.c_str()));
		setupUi.geombox->setCurrentIndex(setupUi.geombox->count() - 1);
	} else {
		prfChangeLayName(layList[eidx].name, nlay.name);
		layList[eidx] = nlay;
		setupUi.geombox->setItemText(eidx, nlay.name.c_str());
	}
	layeditor->hide();
}

// ROMSETS

void SetupWin::rsNameCheck(QString nam) {
	rseUi.rse_apply->setEnabled(!rseUi.rsName->text().isEmpty());
	for (uint i = 0; i < rsList.size(); i++) {
		if ((QString(rsList[i].name.c_str()) == nam) && (eidx != (int)i)) {
			rseUi.rse_apply->setEnabled(false);
		}
	}
}

void SetupWin::rmRomset() {
	int idx = setupUi.rsetbox->currentIndex();
	if (idx < 0) return;
	if (areSure("Do you really want to delete this romset?")) {
		rsList.erase(rsList.begin() + idx);
		setupUi.rsetbox->removeItem(idx);
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
	eidx = setupUi.rsetbox->currentIndex();
	if (eidx < 0) return;
	nrs = rsList[eidx];
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
		setupUi.rsetbox->addItem(QString::fromLocal8Bit(nrs.name.c_str()));
		setupUi.rsetbox->setCurrentIndex(setupUi.rsetbox->count() - 1);
	} else {
		prfChangeRsName(rsList[eidx].name, nrs.name);
		rsList[eidx] = nrs;
		setupUi.rsetbox->setItemText(eidx, nrs.name.c_str());
	}
	rseditor->hide();
}

// lists

void SetupWin::buildkeylist() {
	QDir dir(conf.path.confDir.c_str());
	QStringList lst = dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name);
	fillRFBox(setupUi.keyMapBox,lst);
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
	QString oldText = setupUi.mszbox->currentText();
	setupUi.mszbox->clear();
	if (t == 0x00) {
		setupUi.mszbox->addItem("48K",48);
	} else {
		if (t & 1) setupUi.mszbox->addItem("128K",128);
		if (t & 2) setupUi.mszbox->addItem("256K",256);
		if (t & 4) setupUi.mszbox->addItem("512K",512);
		if (t & 8) setupUi.mszbox->addItem("1024K",1024);
		if (t & 16) setupUi.mszbox->addItem("2048K",2048);
		if (t & 32) setupUi.mszbox->addItem("4096K",4096);
	}
	setupUi.mszbox->setCurrentIndex(setupUi.mszbox->findText(oldText));
	if (setupUi.mszbox->currentIndex() < 0) setupUi.mszbox->setCurrentIndex(setupUi.mszbox->count() - 1);
}

void SetupWin::buildrsetlist() {
//	int i;
	if (setupUi.rsetbox->currentIndex() < 0) {
		setupUi.rstab->setEnabled(false);
		return;
	}
	setupUi.rstab->setEnabled(true);
	xRomset rset = rsList[setupUi.rsetbox->currentIndex()];
	if (rset.file == "") {
		setupUi.rstab->hideRow(4);
		for (int i=0; i<4; i++) {
			setupUi.rstab->showRow(i);
			QString rsf = QString::fromLocal8Bit(rset.roms[i].path.c_str());
			setupUi.rstab->item(i,1)->setText(rsf);
			if (rsf != "") {
				setupUi.rstab->item(i,2)->setText(QString::number(rset.roms[i].part));
			} else {
				setupUi.rstab->item(i,2)->setText("");
			}
		}
	} else {
		setupUi.rstab->hideRow(0);
		setupUi.rstab->hideRow(1);
		setupUi.rstab->hideRow(2);
		setupUi.rstab->hideRow(3);
		setupUi.rstab->showRow(4);
		setupUi.rstab->item(4,1)->setText(QString::fromLocal8Bit(rset.file.c_str()));
		setupUi.rstab->item(4,2)->setText("");
	}
	setupUi.rstab->item(5,1)->setText(QString::fromLocal8Bit(rset.gsFile.c_str()));
	setupUi.rstab->item(6,1)->setText(QString::fromLocal8Bit(rset.fntFile.c_str()));
	setupUi.rstab->setColumnWidth(0,100);
	setupUi.rstab->setColumnWidth(1,300);
	setupUi.rstab->setColumnWidth(2,50);
}

void SetupWin::buildtapelist() {
//	buildTapeList();
	TapeBlockInfo* inf = new TapeBlockInfo[comp->tape->blkCount];
	tapGetBlocksInfo(comp->tape,inf);
	setupUi.tapelist->setRowCount(comp->tape->blkCount);
	if (comp->tape->blkCount == 0) {
		setupUi.tapelist->setEnabled(false);
		return;
	}
	setupUi.tapelist->setEnabled(true);
	QTableWidgetItem* itm;
	uint tm,ts;
	for (int i=0; i < comp->tape->blkCount; i++) {
		if (comp->tape->block == i) {
			itm = new QTableWidgetItem(QIcon(":/images/checkbox.png"),"");
			setupUi.tapelist->setItem(i,0,itm);
			ts = inf[i].curtime;
			tm = ts/60;
			ts -= tm * 60;
			itm = new QTableWidgetItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
			setupUi.tapelist->setItem(i,3,itm);
		} else {
			itm = new QTableWidgetItem;
			setupUi.tapelist->setItem(i,0,itm);
			itm = new QTableWidgetItem;
			setupUi.tapelist->setItem(i,3,itm);
		}
		itm = new QTableWidgetItem;
		if (inf[i].breakPoint) itm->setIcon(QIcon(":/images/cancel.png"));
		setupUi.tapelist->setItem(i,1,itm);
		ts = inf[i].time;
		tm = ts/60;
		ts -= tm * 60;
		itm = new QTableWidgetItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
		setupUi.tapelist->setItem(i,2,itm);
		itm = new QTableWidgetItem(QString::number(inf[i].size));
		setupUi.tapelist->setItem(i,4,itm);
		itm = new QTableWidgetItem(QString::fromLocal8Bit(inf[i].name));
		setupUi.tapelist->setItem(i,5,itm);
	}
	setupUi.tapelist->selectRow(0);
}

void SetupWin::buildmenulist() {
	std::vector<xBookmark> bml = getBookmarkList();
	setupUi.umlist->setRowCount(bml.size());
	QTableWidgetItem* itm;
	for (uint i=0; i<bml.size(); i++) {
		itm = new QTableWidgetItem(QString(bml[i].name.c_str()));
		setupUi.umlist->setItem(i,0,itm);
		itm = new QTableWidgetItem(QString(bml[i].path.c_str()));
		setupUi.umlist->setItem(i,1,itm);
	}
	setupUi.umlist->setColumnWidth(0,100);
	setupUi.umlist->selectRow(0);
};

void SetupWin::buildproflist() {
	std::vector<xProfile> prList = getProfileList();
	setupUi.twProfileList->setRowCount(prList.size());
	QTableWidgetItem* itm;
	for (uint i=0; i<prList.size(); i++) {
		itm = new QTableWidgetItem(QString::fromLocal8Bit(prList[i].name.c_str()));
		setupUi.twProfileList->setItem(i,0,itm);
		itm = new QTableWidgetItem(QString::fromLocal8Bit(prList[i].file.c_str()));
		setupUi.twProfileList->setItem(i,1,itm);
	}
}

void SetupWin::copyToTape() {
	int dsk = setupUi.disktabs->currentIndex();
	QModelIndexList idx = setupUi.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	TRFile cat[128];
	flpGetTRCatalog(comp->bdi->fdc->flop[dsk],cat);
	// std::vector<TRFile> cat = flpGetTRCatalog(comp->bdi->flop[dsk]);
	int row;
	unsigned char* buf = new unsigned char[0xffff];
	unsigned short line,start,len;
	char name[10];
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		row = idx[i].row();
		if (flpGetSectorsData(comp->bdi->fdc->flop[dsk],cat[row].trk, cat[row].sec+1, buf, cat[row].slen)) {
			if (cat[row].slen == (cat[row].hlen + ((cat[row].llen == 0) ? 0 : 1))) {
				start = (cat[row].hst << 8) + cat[row].lst;
				len = (cat[row].hlen << 8) + cat[row].llen;
				line = (cat[row].ext == 'B') ? (buf[start] + (buf[start+1] << 8)) : 0x8000;
				strncpy(name,(char*)&cat[row].name[0],8);
				strcat(name,".");
				strncat(name,(char*)&cat[row].ext,1);
				// name = std::string((char*)&cat[row].name[0],8) + std::string(".") + std::string((char*)&cat[row].ext,1);
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
	QModelIndexList idx = setupUi.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...",QDir::homePath());
	if (dir == "") return;
	std::string sdir = std::string(dir.toLocal8Bit().data()) + std::string(SLASH);
	Floppy* flp = comp->bdi->fdc->flop[setupUi.disktabs->currentIndex()];		// selected floppy
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveHobetaFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = std::string(int2str(savedFiles)) + " of " + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

void SetupWin::diskToRaw() {
	QModelIndexList idx = setupUi.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...","",QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
	if (dir == "") return;
	std::string sdir = std::string(dir.toLocal8Bit().data()) + std::string(SLASH);
	Floppy* flp = comp->bdi->fdc->flop[setupUi.disktabs->currentIndex()];
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
	int blk = setupUi.tapelist->currentRow();
	if (blk < 0) return;
	int dsk = setupUi.disktabs->currentIndex();
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
	if (!(comp->bdi->fdc->flop[dsk]->insert)) newdisk(dsk);
	TapeBlockInfo inf = tapGetBlockInfo(comp->tape,dataBlock);
	unsigned char* dt = new unsigned char[inf.size];
	tapGetBlockData(comp->tape,dataBlock,dt);
	unsigned char* buf = new unsigned char[256];
	int pos = 1;	// skip block type mark
	switch(flpCreateDescriptor(comp->bdi->fdc->flop[dsk],&dsc)) {
		case ERR_SHIT: shitHappens("Yes, it happens"); break;
		case ERR_MANYFILES: shitHappens("Too many files @ disk"); break;
		case ERR_NOSPACE: shitHappens("Not enough space @ disk"); break;
		case ERR_OK:
			while (pos < inf.size) {
				do {
					buf[(pos-1) & 0xff] = (pos < inf.size) ? dt[pos] : 0x00;
					pos++;
				} while ((pos & 0xff) != 1);
				flpPutSectorData(comp->bdi->fdc->flop[dsk],dsc.trk, dsc.sec+1, buf, 256);
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
	int dsk = setupUi.disktabs->currentIndex();
	QTableWidget* wid = setupUi.disklist;
	wid->setColumnWidth(0,100);
	wid->setColumnWidth(1,30);
	wid->setColumnWidth(2,70);
	wid->setColumnWidth(3,70);
	wid->setColumnWidth(4,50);
	wid->setColumnWidth(5,50);
//	wid->setColumnWidth(6,40);
	QTableWidgetItem* itm;
	if (!(comp->bdi->fdc->flop[dsk]->insert)) {
		wid->setEnabled(false);
		wid->setRowCount(0);
	} else {
		wid->setEnabled(true);
		if (flpGet(comp->bdi->fdc->flop[dsk],FLP_DISKTYPE) == DISK_TYPE_TRD) {
			TRFile cat[128];
			int catSize = flpGetTRCatalog(comp->bdi->fdc->flop[dsk],cat);
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

void SetupWin::updfrq() {
	double f = setupUi.cpufrq->value() / 2.0;
	setupUi.cpufrqlab->setText(QString::number(f,'f',2).append(" MHz"));
}

// video

void SetupWin::chabsz() {setupUi.bszlab->setText(QString::number(setupUi.bszsld->value()).append("%"));}

void SetupWin::selsspath() {
	QString fpath = QFileDialog::getExistingDirectory(this,"Screenshots folder",QString::fromLocal8Bit(conf.scrShot.dir.c_str()),QFileDialog::ShowDirsOnly);
	if (fpath!="") setupUi.pathle->setText(fpath);
}

// sound

void SetupWin::updvolumes() {
	setupUi.bvlab->setText(QString::number(setupUi.bvsld->value()));
	setupUi.tvlab->setText(QString::number(setupUi.tvsld->value()));
	setupUi.avlab->setText(QString::number(setupUi.avsld->value()));
	setupUi.gslab->setText(QString::number(setupUi.gvsld->value()));
}

// disk

void SetupWin::newdisk(int idx) {
	Floppy *flp = comp->bdi->fdc->flop[idx];
	if (!saveChangedDisk(comp,idx & 3)) return;
	flpFormat(flp);
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

void SetupWin::savea() {Floppy* flp = comp->bdi->fdc->flop[0]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,0);}
void SetupWin::saveb() {Floppy* flp = comp->bdi->fdc->flop[1]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,1);}
void SetupWin::savec() {Floppy* flp = comp->bdi->fdc->flop[2]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,2);}
void SetupWin::saved() {Floppy* flp = comp->bdi->fdc->flop[3]; if (flp->insert) saveFile(comp,flp->path,FT_DISK,3);}

void SetupWin::ejcta() {saveChangedDisk(comp,0); flpEject(comp->bdi->fdc->flop[0]); updatedisknams();}
void SetupWin::ejctb() {saveChangedDisk(comp,1); flpEject(comp->bdi->fdc->flop[1]); updatedisknams();}
void SetupWin::ejctc() {saveChangedDisk(comp,2); flpEject(comp->bdi->fdc->flop[2]); updatedisknams();}
void SetupWin::ejctd() {saveChangedDisk(comp,3); flpEject(comp->bdi->fdc->flop[3]); updatedisknams();}

void SetupWin::updatedisknams() {
	setupUi.apathle->setText(QString::fromLocal8Bit(comp->bdi->fdc->flop[0]->path));
	setupUi.bpathle->setText(QString::fromLocal8Bit(comp->bdi->fdc->flop[1]->path));
	setupUi.cpathle->setText(QString::fromLocal8Bit(comp->bdi->fdc->flop[2]->path));
	setupUi.dpathle->setText(QString::fromLocal8Bit(comp->bdi->fdc->flop[3]->path));
	fillDiskCat();
}

// tape

void SetupWin::loatape() {
	loadFile(comp,"",FT_TAPE,1);
	setupUi.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
}

void SetupWin::savtape() {
	if (comp->tape->blkCount != 0) saveFile(comp,comp->tape->path,FT_TAP,-1);
}

void SetupWin::ejctape() {
	tapEject(comp->tape);
	setupUi.tpathle->setText(QString::fromLocal8Bit(comp->tape->path));
	buildtapelist();
}

void SetupWin::tblkup() {
	int ps = setupUi.tapelist->currentIndex().row();
	if (ps > 0) {
		tapSwapBlocks(comp->tape,ps,ps-1);
		buildtapelist();
		setupUi.tapelist->selectRow(ps-1);
	}
}

void SetupWin::tblkdn() {
	int ps = setupUi.tapelist->currentIndex().row();
	if ((ps != -1) && (ps < comp->tape->blkCount - 1)) {
		tapSwapBlocks(comp->tape,ps,ps+1);
		buildtapelist();
		setupUi.tapelist->selectRow(ps+1);
	}
}

void SetupWin::tblkrm() {
	int ps = setupUi.tapelist->currentIndex().row();
	if (ps != -1) {
		tapDelBlock(comp->tape,ps);
		buildtapelist();
		setupUi.tapelist->selectRow(ps);
	}
}

void SetupWin::chablock(QModelIndex idx) {
	int row = idx.row();
	tapRewind(comp->tape,row);
	buildtapelist();
	setupUi.tapelist->selectRow(row);
}

void SetupWin::setTapeBreak(int row,int col) {
	if ((row < 0) || (col != 1)) return;
	comp->tape->blkData[row].breakPoint ^= 1;
	buildtapelist();
	setupUi.tapelist->selectRow(row);
}

// hdd

void SetupWin::hddMasterImg() {
	QString path = QFileDialog::getOpenFileName(this,"Image for master HDD",QDir::homePath(),"All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") setupUi.hm_path->setText(path);
}

void SetupWin::hddSlaveImg() {
	QString path = QFileDialog::getOpenFileName(this,"Image for slave HDD",QDir::homePath(),"All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") setupUi.hs_path->setText(path);
}

void SetupWin::hddcap() {
	unsigned int sz;
	if (setupUi.hm_islba->checkState() == Qt::Checked) {
		sz = (setupUi.hm_glba->value() >> 11);
	} else {
		sz = ((setupUi.hm_gsec->value() * (setupUi.hm_ghd->value() + 1) * (setupUi.hm_gcyl->value() + 1)) >> 11);
	}
	setupUi.hm_capacity->setValue(sz);
	if (setupUi.hs_islba->checkState() == Qt::Checked) {
		sz = (setupUi.hs_glba->value() >> 11);
	} else {
		sz = ((setupUi.hs_gsec->value() * (setupUi.hs_ghd->value() + 1) * (setupUi.hs_gcyl->value() + 1)) >> 11);
	}
	setupUi.hs_capacity->setValue(sz);
}

// sdc

void SetupWin::selSDCimg() {
	QString fnam = QFileDialog::getOpenFileName(this,"Image for SD card","","All files (*.*)");
	if (!fnam.isEmpty()) setupUi.sdPath->setText(fnam);
}

// tools

void SetupWin::umup() {
	int ps = setupUi.umlist->currentRow();
	if (ps>0) {
		swapBookmarks(ps,ps-1);
		buildmenulist();
		setupUi.umlist->selectRow(ps-1);
	}
}

void SetupWin::umdn() {
	int ps = setupUi.umlist->currentIndex().row();
	if ((ps!=-1) && (ps < getBookmarksCount()-1)) {
		swapBookmarks(ps,ps+1);
		buildmenulist();
		setupUi.umlist->selectRow(ps+1);
	}
}

void SetupWin::umdel() {
	int ps = setupUi.umlist->currentIndex().row();
	if (ps!=-1) {
		delBookmark(ps);
		buildmenulist();
		if (ps == getBookmarksCount()) {
			setupUi.umlist->selectRow(ps-1);
		} else {
			setupUi.umlist->selectRow(ps);
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
	uia.namele->setText(setupUi.umlist->item(umidx,0)->text());
	uia.pathle->setText(setupUi.umlist->item(umidx,1)->text());
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
	setupUi.umlist->selectRow(setupUi.umlist->rowCount()-1);
}

// profiles

void SetupWin::newProfile() {
	QString nam = QInputDialog::getText(this,"Enter...","New profile name");
	if (nam.isEmpty()) return;
	std::string nm = std::string(nam.toLocal8Bit().data());
	std::string fp = nm + ".conf";
	if (!addProfile(nm,fp)) shitHappens("Can't add such profile");
	buildproflist();
}

void SetupWin::rmProfile() {
	int idx = setupUi.twProfileList->currentRow();
	if (idx < 0) return;
	if (!areSure("Do you really want to delete this profile?")) return;
	std::string pnam(setupUi.twProfileList->item(idx,0)->text().toLocal8Bit().data());
	idx = delProfile(pnam);
	switch(idx) {
		case DELP_OK_CURR:
			start(comp);
			break;
		case DELP_ERR:
			shitHappens("Sorry, i can't delete this profile");
			break;
	}
	buildproflist();
}
