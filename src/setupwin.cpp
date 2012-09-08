#include <QStandardItemModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QVector3D>
#include <QDebug>
#include <stdlib.h>
#ifdef HAVESDL
	#include <SDL.h>
#endif

#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "sound.h"
#include "libxpeccy/spectrum.h"
#include "setupwin.h"
#include "emulwin.h"
#include "settings.h"
#include "filer.h"
#include "filetypes/filetypes.h"

//#include "ui_selname.h"
#include "ui_setupwin.h"
#include "ui_umadial.h"

Ui::SetupWin setupUi;
Ui::UmaDial uia;

SetupWin* optWin;

std::vector<RomSet> rsl;
std::string GSRom;

void optInit(QWidget* par) {
	optWin = new SetupWin(par);
}

void optShow() {
	optWin->start();
}

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

SetupWin::SetupWin(QWidget* par):QDialog(par) {
	setModal(true);
	setupUi.setupUi(this);

	umadial = new QDialog;
	uia.setupUi(umadial);
	umadial->setModal(true);

	uint32_t i;
	std::vector<std::string> list;
// machine
	list = getHardwareNames();
	for (i=0; i < list.size(); i++) {
		setupUi.machbox->addItem(QString::fromLocal8Bit(list[i].c_str()));
	}
	setupUi.resbox->addItems(QStringList()<<"0:Basic 128"<<"1:Basic48"<<"2:Shadow"<<"3:DOS");
	setupUi.rssel->hide();
	QTableWidgetItem* itm;
	for (i=0; i<6; i++) {
		itm = new QTableWidgetItem; setupUi.rstab->setItem(i,1,itm);
		itm = new QTableWidgetItem; setupUi.rstab->setItem(i,2,itm);
	}
// video
	OptName* ptr = getGetPtr(OPT_SHOTFRM);
	i = 0; while (ptr[i].id != -1) {
		setupUi.ssfbox->addItem(QString(ptr[i].name.c_str()),QVariant(ptr[i].id));
		i++;
	}
// sound
	list = sndGetList();
	for (i=0;i<list.size();i++) {setupUi.outbox->addItem(QString::fromLocal8Bit(list[i].c_str()));}
	setupUi.ratbox->addItems(QStringList()<<"44100"<<"22050"<<"11025");
	setupUi.schip1box->addItem(QIcon(":/images/cancel.png"),"none",QVariant(SND_NONE));
	setupUi.schip1box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",QVariant(SND_AY));
	setupUi.schip1box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",QVariant(SND_YM));
	setupUi.schip2box->addItem(QIcon(":/images/cancel.png"),"none",QVariant(SND_NONE));
	setupUi.schip2box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",QVariant(SND_AY));
	setupUi.schip2box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",QVariant(SND_YM));
	setupUi.stereo1box->addItem("Mono",QVariant(AY_MONO)); setupUi.stereo2box->addItem("Mono",QVariant(AY_MONO));
	setupUi.stereo1box->addItem("ABC",QVariant(AY_ABC)); setupUi.stereo2box->addItem("ABC",QVariant(AY_ABC));
	setupUi.stereo1box->addItem("ACB",QVariant(AY_ACB)); setupUi.stereo2box->addItem("ACB",QVariant(AY_ACB));
	setupUi.stereo1box->addItem("BAC",QVariant(AY_BAC)); setupUi.stereo2box->addItem("BAC",QVariant(AY_BAC));
	setupUi.stereo1box->addItem("BCA",QVariant(AY_BCA)); setupUi.stereo2box->addItem("BCA",QVariant(AY_BCA));
	setupUi.stereo1box->addItem("CAB",QVariant(AY_CAB)); setupUi.stereo2box->addItem("CAB",QVariant(AY_CAB));
	setupUi.stereo1box->addItem("CBA",QVariant(AY_CBA)); setupUi.stereo2box->addItem("CBA",QVariant(AY_BCA));
	setupUi.tsbox->addItem("None",QVariant(TS_NONE));
	setupUi.tsbox->addItem("NedoPC",QVariant(TS_NEDOPC));
	setupUi.gstereobox->addItem("Mono",QVariant(GS_MONO));
	setupUi.gstereobox->addItem("L:1,2; R:3,4",QVariant(GS_12_34));
// bdi
// WTF? QtDesigner doesn't save this properties
	setupUi.disklist->horizontalHeader()->setVisible(true);
	setupUi.diskTypeBox->addItem("None",QVariant(FDC_NONE));
	setupUi.diskTypeBox->addItem("Beta disk (VG93)",QVariant(FDC_93));
	setupUi.diskTypeBox->addItem("+3 DOS (uPD765)",QVariant(FDC_765));
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
	setupUi.hiface->addItem("None",QVariant(IDE_NONE));
	setupUi.hiface->addItem("Nemo",QVariant(IDE_NEMO));
	setupUi.hiface->addItem("Nemo A8",QVariant(IDE_NEMOA8));
	setupUi.hiface->addItem("SMUC",QVariant(IDE_SMUC));
	setupUi.hm_type->addItem(QIcon(":/images/cancel.png"),"Not connected",QVariant(IDE_NONE));
	setupUi.hm_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",QVariant(IDE_ATA));
//	setupUi.hm_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",QVariant(IDE_ATAPI));
	setupUi.hs_type->addItem(QIcon(":/images/cancel.png"),"Not connected",QVariant(IDE_NONE));
	setupUi.hs_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",QVariant(IDE_ATA));
//	setupUi.hs_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",QVariant(IDE_ATAPI));

// all
	QObject::connect(setupUi.okbut,SIGNAL(released()),this,SLOT(okay()));
	QObject::connect(setupUi.apbut,SIGNAL(released()),this,SLOT(apply()));
	QObject::connect(setupUi.cnbut,SIGNAL(released()),this,SLOT(reject()));
// machine
	QObject::connect(setupUi.rsetbox,SIGNAL(currentIndexChanged(int)),this,SLOT(buildrsetlist()));
	QObject::connect(setupUi.machbox,SIGNAL(currentIndexChanged(int)),this,SLOT(setmszbox(int)));
//	QObject::connect(setupUi.mszbox,SIGNAL(currentIndexChanged(int)),this,SLOT(okbuts()));
	QObject::connect(setupUi.cpufrq,SIGNAL(valueChanged(int)),this,SLOT(updfrq()));
	QObject::connect(setupUi.rstab,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editrset(QModelIndex)));
	QObject::connect(setupUi.rse_cancel,SIGNAL(released()),this,SLOT(hidersedit()));
	QObject::connect(setupUi.rse_apply,SIGNAL(released()),this,SLOT(setrpart()));
	QObject::connect(setupUi.rse_grp_single,SIGNAL(toggled(bool)),this,SLOT(recheck_separate(bool)));
	QObject::connect(setupUi.rse_grp_separate,SIGNAL(toggled(bool)),this,SLOT(recheck_single(bool)));
	QObject::connect(setupUi.addrset,SIGNAL(released()),this,SLOT(addNewRomset()));
	QObject::connect(setupUi.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
// video
	QObject::connect(setupUi.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	QObject::connect(setupUi.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));
	QObject::connect(setupUi.brgslide,SIGNAL(valueChanged(int)),this,SLOT(chabrg()));
// sound
	QObject::connect(setupUi.bvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(setupUi.tvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(setupUi.avsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(setupUi.gvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
// input
	connect(setupUi.addBind,SIGNAL(released()),this,SLOT(addJoyBind()));
	connect(setupUi.delBind,SIGNAL(released()),this,SLOT(delJoyBind()));
// dos
	QObject::connect(setupUi.newatb,SIGNAL(released()),this,SLOT(newa()));
	QObject::connect(setupUi.newbtb,SIGNAL(released()),this,SLOT(newb()));
	QObject::connect(setupUi.newctb,SIGNAL(released()),this,SLOT(newc()));
	QObject::connect(setupUi.newdtb,SIGNAL(released()),this,SLOT(newd()));

	QObject::connect(setupUi.loadatb,SIGNAL(released()),this,SLOT(loada()));
	QObject::connect(setupUi.loadbtb,SIGNAL(released()),this,SLOT(loadb()));
	QObject::connect(setupUi.loadctb,SIGNAL(released()),this,SLOT(loadc()));
	QObject::connect(setupUi.loaddtb,SIGNAL(released()),this,SLOT(loadd()));

	QObject::connect(setupUi.saveatb,SIGNAL(released()),this,SLOT(savea()));
	QObject::connect(setupUi.savebtb,SIGNAL(released()),this,SLOT(saveb()));
	QObject::connect(setupUi.savectb,SIGNAL(released()),this,SLOT(savec()));
	QObject::connect(setupUi.savedtb,SIGNAL(released()),this,SLOT(saved()));

	QObject::connect(setupUi.remoatb,SIGNAL(released()),this,SLOT(ejcta()));
	QObject::connect(setupUi.remobtb,SIGNAL(released()),this,SLOT(ejctb()));
	QObject::connect(setupUi.remoctb,SIGNAL(released()),this,SLOT(ejctc()));
	QObject::connect(setupUi.remodtb,SIGNAL(released()),this,SLOT(ejctd()));

	QObject::connect(setupUi.disktabs,SIGNAL(currentChanged(int)),this,SLOT(fillDiskCat()));
	connect(setupUi.actCopyToTape,SIGNAL(triggered()),this,SLOT(copyToTape()));
	connect(setupUi.actSaveHobeta,SIGNAL(triggered()),this,SLOT(diskToHobeta()));
	connect(setupUi.actSaveRaw,SIGNAL(triggered()),this,SLOT(diskToRaw()));
	connect(setupUi.tbToTape,SIGNAL(released()),this,SLOT(copyToTape()));
	connect(setupUi.tbToHobeta,SIGNAL(released()),this,SLOT(diskToHobeta()));
	connect(setupUi.tbToRaw,SIGNAL(released()),this,SLOT(diskToRaw()));
// tape
	QObject::connect(setupUi.tapelist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chablock(QModelIndex)));
	QObject::connect(setupUi.tapelist,SIGNAL(cellClicked(int,int)),this,SLOT(setTapeBreak(int,int)));
	QObject::connect(setupUi.tloadtb,SIGNAL(released()),this,SLOT(loatape()));
	QObject::connect(setupUi.tsavetb,SIGNAL(released()),this,SLOT(savtape()));
	QObject::connect(setupUi.tremotb,SIGNAL(released()),this,SLOT(ejctape()));
	QObject::connect(setupUi.blkuptb,SIGNAL(released()),this,SLOT(tblkup()));
	QObject::connect(setupUi.blkdntb,SIGNAL(released()),this,SLOT(tblkdn()));
	QObject::connect(setupUi.blkrmtb,SIGNAL(released()),this,SLOT(tblkrm()));
	connect(setupUi.actCopyToDisk,SIGNAL(triggered()),this,SLOT(copyToDisk()));
	connect(setupUi.tbToDisk,SIGNAL(released()),this,SLOT(copyToDisk()));
// hdd
	QObject::connect(setupUi.hm_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hm_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hm_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hm_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hm_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hm_pathtb,SIGNAL(released()),this,SLOT(hddMasterImg()));
	QObject::connect(setupUi.hs_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hs_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hs_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hs_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(setupUi.hs_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(setupUi.hs_pathtb,SIGNAL(released()),this,SLOT(hddSlaveImg()));
//tools
	QObject::connect(setupUi.sjselptb,SIGNAL(released()),this,SLOT(ssjapath()));
	QObject::connect(setupUi.pdselptb,SIGNAL(released()),this,SLOT(sprjpath()));
	QObject::connect(setupUi.umlist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(umedit(QModelIndex)));
	QObject::connect(setupUi.umaddtb,SIGNAL(released()),this,SLOT(umadd()));
	QObject::connect(setupUi.umdeltb,SIGNAL(released()),this,SLOT(umdel()));
	QObject::connect(setupUi.umuptb,SIGNAL(released()),this,SLOT(umup()));
	QObject::connect(setupUi.umdntb,SIGNAL(released()),this,SLOT(umdn()));
// bookmark add dialog
	QObject::connect(uia.umasptb,SIGNAL(released()),this,SLOT(umaselp()));
	QObject::connect(uia.umaok,SIGNAL(released()),this,SLOT(umaconf()));
	QObject::connect(uia.umacn,SIGNAL(released()),umadial,SLOT(hide()));
// profiles manager
	connect(setupUi.tbNewProfile,SIGNAL(released()),this,SLOT(newProfile()));
	connect(setupUi.tbDelProfile,SIGNAL(released()),this,SLOT(rmProfile()));
}

void SetupWin::recheck_single(bool st) {
	setupUi.rse_grp_single->setChecked(!st);
}
void SetupWin::recheck_separate(bool st) {setupUi.rse_grp_separate->setChecked(!st);}

void SetupWin::okay() {apply();reject();}

void SetupWin::start() {
	uint32_t i;
	emulPause(true,PR_OPTS);
	XProfile* curProf = getCurrentProfile();
// machine
	setupUi.rstab->show();
	setupUi.rssel->hide();
	setupUi.rsetbox->setEnabled(true);
	setupUi.addrset->setEnabled(true);
	setupUi.rmrset->setEnabled(true);
	setupUi.rsetbox->clear();
	rsl = getRomsetList();
	GSRom = curProf->gsFile;
	for (i=0; i < rsl.size(); i++) {
		setupUi.rsetbox->addItem(QString::fromLocal8Bit(rsl[i].name.c_str()));
	}
	setupUi.machbox->setCurrentIndex(setupUi.machbox->findText(QString::fromUtf8(zx->hw->name)));
	int cbx = -1;
	RomSet* rset = findRomset(curProf->rsName);
	if (rset != NULL) cbx = setupUi.rsetbox->findText(QString::fromUtf8(rset->name.c_str()));
	setupUi.rsetbox->setCurrentIndex(cbx);
	setupUi.reschk->setChecked(emulGetFlags() & FL_RESET);
	setupUi.resbox->setCurrentIndex(zx->resbank);
	setmszbox(setupUi.machbox->currentIndex());
	setupUi.mszbox->setCurrentIndex(setupUi.mszbox->findData(zx->mem->memSize));
	if (setupUi.mszbox->currentIndex() < 0) setupUi.mszbox->setCurrentIndex(setupUi.mszbox->count() - 1);
	setupUi.cpufrq->setValue(zx->cpuFrq * 2); updfrq();
	setupUi.scrpwait->setChecked(zx->hwFlags & WAIT_ON);
// video
	setupUi.dszchk->setChecked((vidFlag & VF_DOUBLE));
	setupUi.fscchk->setChecked(vidFlag & VF_FULLSCREEN);
	setupUi.noflichk->setChecked(vidFlag & VF_NOFLIC);
	setupUi.bszsld->setValue((int)(brdsize * 100));
	setupUi.pathle->setText(QString::fromLocal8Bit(optGetString(OPT_SHOTDIR).c_str()));
	setupUi.ssfbox->setCurrentIndex(setupUi.ssfbox->findData(optGetInt(OPT_SHOTFRM)));
	setupUi.scntbox->setValue(optGetInt(OPT_SHOTCNT));
	setupUi.sintbox->setValue(optGetInt(OPT_SHOTINT));
	setupUi.brgslide->setValue(optGetInt(OPT_BRGLEV));
	std::vector<VidLayout> lays = getLayoutList();
	setupUi.geombox->clear();
	for (i=0; i<lays.size(); i++) {setupUi.geombox->addItem(QString::fromLocal8Bit(lays[i].name.c_str()));}
	setupUi.geombox->setCurrentIndex(setupUi.geombox->findText(QString::fromLocal8Bit(getCurrentProfile()->layName.c_str())));
// sound
	setupUi.senbox->setChecked(sndGet(SND_ENABLE) != 0);
	setupUi.mutbox->setChecked(sndGet(SND_MUTE) != 0);
	setupUi.gsrbox->setChecked(zx->gs->flag & GS_RESET);
	setupUi.outbox->setCurrentIndex(setupUi.outbox->findText(QString::fromLocal8Bit(sndGetOutputName().c_str())));
	setupUi.ratbox->setCurrentIndex(setupUi.ratbox->findText(QString::number(sndGet(SND_RATE))));
	setupUi.bvsld->setValue(sndGet(SND_BEEP));
	setupUi.tvsld->setValue(sndGet(SND_TAPE));
	setupUi.avsld->setValue(sndGet(SND_AYVL));
	setupUi.gvsld->setValue(sndGet(SND_GSVL));
	setupUi.schip1box->setCurrentIndex(setupUi.schip1box->findData(QVariant(zx->ts->chipA->type)));
	setupUi.schip2box->setCurrentIndex(setupUi.schip2box->findData(QVariant(zx->ts->chipB->type)));
	setupUi.stereo1box->setCurrentIndex(setupUi.stereo1box->findData(QVariant(zx->ts->chipA->stereo)));
	setupUi.stereo2box->setCurrentIndex(setupUi.stereo2box->findData(QVariant(zx->ts->chipB->stereo)));
	setupUi.gstereobox->setCurrentIndex(setupUi.gstereobox->findData(QVariant(zx->gs->stereo)));
	setupUi.gsgroup->setChecked(zx->gs->flag & GS_ENABLE);
	setupUi.tsbox->setCurrentIndex(setupUi.tsbox->findData(QVariant(zx->ts->type)));
	setupUi.covoxBox->setChecked(zx->flags & ZX_COVOX);
// input
	buildkeylist();
	buildjmaplist();
	std::string kmname = optGetString(OPT_KEYNAME);
	int idx = setupUi.keyMapBox->findText(QString(kmname.c_str()));
	if (idx < 1) idx = 0;
	setupUi.keyMapBox->setCurrentIndex(idx);
#ifdef XQTPAINT
	setupUi.joyBox->setEnabled(false);
#else
	setupUi.joyBox->setEnabled(true);
	setupUi.inpDevice->clear();
	setupUi.inpDevice->addItem("None");

	int jnum=SDL_NumJoysticks();
	for (int cnt=0; cnt<jnum; cnt++) {
		setupUi.inpDevice->addItem(QString(SDL_JoystickName(cnt)));
	}
	idx = setupUi.inpDevice->findText(QString(optGetString(OPT_JOYNAME).c_str()));
	if (idx < 0) idx = 0;
	setupUi.inpDevice->setCurrentIndex(idx);
#endif
// dos
	setupUi.diskTypeBox->setCurrentIndex(setupUi.diskTypeBox->findData(zx->bdi->fdc->type));
	setupUi.bdtbox->setChecked(zx->bdi->fdc->turbo != 0);
	Floppy* flp = zx->bdi->fdc->flop[0];
	setupUi.apathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.a80box->setChecked(flp->flag & FLP_TRK80);
		setupUi.adsbox->setChecked(flp->flag & FLP_DS);
		setupUi.awpbox->setChecked(flp->flag & FLP_PROTECT);
	flp = zx->bdi->fdc->flop[1];
	setupUi.bpathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.b80box->setChecked(flp->flag & FLP_TRK80);
		setupUi.bdsbox->setChecked(flp->flag & FLP_DS);
		setupUi.bwpbox->setChecked(flp->flag & FLP_PROTECT);
	flp = zx->bdi->fdc->flop[2];
	setupUi.cpathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.c80box->setChecked(flp->flag & FLP_TRK80);
		setupUi.cdsbox->setChecked(flp->flag & FLP_DS);
		setupUi.cwpbox->setChecked(flp->flag & FLP_PROTECT);
	flp = zx->bdi->fdc->flop[3];
	setupUi.dpathle->setText(QString::fromLocal8Bit(flp->path));
		setupUi.d80box->setChecked(flp->flag & FLP_TRK80);
		setupUi.ddsbox->setChecked(flp->flag & FLP_DS);
		setupUi.dwpbox->setChecked(flp->flag & FLP_PROTECT);
	fillDiskCat();
// hdd
	setupUi.hiface->setCurrentIndex(setupUi.hiface->findData(zx->ide->type));

	setupUi.hm_type->setCurrentIndex(setupUi.hm_type->findData(zx->ide->master->type));
	ATAPassport pass = ideGetPassport(zx->ide,IDE_MASTER);
	setupUi.hm_model->setText(QString::fromLocal8Bit(pass.model));
	setupUi.hm_ser->setText(QString::fromLocal8Bit(pass.serial));
	setupUi.hm_path->setText(QString::fromLocal8Bit(zx->ide->master->image));
	setupUi.hm_islba->setChecked(zx->ide->master->flags & ATA_LBA);
	setupUi.hm_gsec->setValue(pass.spt);
	setupUi.hm_ghd->setValue(pass.hds);
	setupUi.hm_gcyl->setValue(pass.cyls);
	setupUi.hm_glba->setValue(zx->ide->master->maxlba);

	setupUi.hs_type->setCurrentIndex(setupUi.hm_type->findData(zx->ide->slave->type));
	pass = ideGetPassport(zx->ide,IDE_SLAVE);
	setupUi.hs_model->setText(QString::fromLocal8Bit(pass.model));
	setupUi.hs_ser->setText(QString::fromLocal8Bit(pass.serial));
	setupUi.hs_path->setText(QString::fromLocal8Bit(zx->ide->slave->image));
	setupUi.hs_islba->setChecked(zx->ide->slave->flags & ATA_LBA);
	setupUi.hs_gsec->setValue(pass.spt);
	setupUi.hs_ghd->setValue(pass.hds);
	setupUi.hs_gcyl->setValue(pass.cyls);
	setupUi.hs_glba->setValue(zx->ide->slave->maxlba);
// tape
	setupUi.cbTapeAuto->setChecked(optGetFlag(OF_TAPEAUTO));
	setupUi.cbTapeFast->setChecked(optGetFlag(OF_TAPEFAST));
	setupUi.tpathle->setText(QString::fromLocal8Bit(zx->tape->path));
	buildtapelist();
// tools
	setupUi.sjpathle->setText(QString::fromLocal8Bit(optGetString(OPT_ASMPATH).c_str()));
	setupUi.prjdirle->setText(QString::fromLocal8Bit(optGetString(OPT_PROJDIR).c_str()));
	buildmenulist();
// leds
	setupUi.diskLed->setChecked(emulGetFlags() & FL_LED_DISK);
	setupUi.shotLed->setChecked(emulGetFlags() & FL_LED_SHOT);
// profiles
	buildproflist();

	show();
}

void SetupWin::apply() {
	XProfile* curProf = getCurrentProfile();
// machine
	HardWare *oldmac = zx->hw;
	curProf->hwName = std::string(setupUi.machbox->currentText().toUtf8().data());
	setHardware(curProf->zx,curProf->hwName);
	curProf->rsName = std::string(setupUi.rsetbox->currentText().toUtf8().data());
	setRomset(curProf->name, curProf->rsName);
	emulSetFlag(FL_RESET, setupUi.reschk->isChecked());
	zx->resbank = setupUi.resbox->currentIndex();

	memSetSize(zx->mem,setupUi.mszbox->itemData(setupUi.mszbox->currentIndex()).toInt());
	zxSetFrq(zx,setupUi.cpufrq->value() / 2.0);
	setFlagBit(setupUi.scrpwait->isChecked(),&zx->hwFlags,WAIT_ON);
	curProf->gsFile = GSRom;
	setRomsetList(rsl);
	if (zx->hw != oldmac) zxReset(zx,RES_DEFAULT);
// video
	setFlagBit(setupUi.dszchk->isChecked(),&vidFlag,VF_DOUBLE);
	setFlagBit(setupUi.fscchk->isChecked(),&vidFlag,VF_FULLSCREEN);
	setFlagBit(setupUi.noflichk->isChecked(),&vidFlag,VF_NOFLIC);
	brdsize = setupUi.bszsld->value()/100.0;
	optSet(OPT_SHOTDIR,std::string(setupUi.pathle->text().toLocal8Bit().data()));
	optSet(OPT_SHOTFRM,setupUi.ssfbox->itemData(setupUi.ssfbox->currentIndex()).toInt());
	optSet(OPT_SHOTCNT,setupUi.scntbox->value());
	optSet(OPT_SHOTINT,setupUi.sintbox->value());
	emulSetLayout(zx->vid,std::string(setupUi.geombox->currentText().toLocal8Bit().data()));
	optSet(OPT_BRGLEV,setupUi.brgslide->value());
// sound
	std::string oname = sndGetOutputName();
	std::string nname(setupUi.outbox->currentText().toLocal8Bit().data());
	int orate = sndGet(SND_RATE);
	sndSet(SND_ENABLE, setupUi.senbox->isChecked());
	sndSet(SND_MUTE, setupUi.mutbox->isChecked());
	sndSet(SND_RATE, setupUi.ratbox->currentText().toInt());
	sndSet(SND_BEEP, setupUi.bvsld->value());
	sndSet(SND_TAPE, setupUi.tvsld->value());
	sndSet(SND_AYVL, setupUi.avsld->value());
	sndSet(SND_GSVL, setupUi.gvsld->value());
	if ((oname != nname) || (orate != sndGet(SND_RATE))) setOutput(nname);
	aymSetType(zx->ts->chipA,setupUi.schip1box->itemData(setupUi.schip1box->currentIndex()).toInt());
	aymSetType(zx->ts->chipB,setupUi.schip2box->itemData(setupUi.schip2box->currentIndex()).toInt());
	zx->ts->chipA->stereo = setupUi.stereo1box->itemData(setupUi.stereo1box->currentIndex()).toInt();
	zx->ts->chipB->stereo = setupUi.stereo2box->itemData(setupUi.stereo2box->currentIndex()).toInt();
	zx->ts->type = setupUi.tsbox->itemData(setupUi.tsbox->currentIndex()).toInt();
	zx->gs->flag = 0;
	if (setupUi.gsgroup->isChecked()) zx->gs->flag |= GS_ENABLE;
	if (setupUi.gsrbox->isChecked()) zx->gs->flag |= GS_RESET;
	zx->gs->stereo = setupUi.gstereobox->itemData(setupUi.gstereobox->currentIndex()).toInt();
	if (setupUi.covoxBox->isChecked()) zx->flags |= ZX_COVOX; else zx->flags &= ~ZX_COVOX;
// input
	if (setupUi.inpDevice->currentIndex() < 1) {
		optSet(OPT_JOYNAME,std::string(""));
	} else {
		optSet(OPT_JOYNAME,std::string(setupUi.inpDevice->currentText().toLocal8Bit().data()));
	}
	std::string kmname = getRFText(setupUi.keyMapBox);
	if (kmname == "none") kmname = "default";
	optSet(OPT_KEYNAME,kmname);
	loadKeys();
// bdi
	zx->bdi->fdc->type = setupUi.diskTypeBox->itemData(setupUi.diskTypeBox->currentIndex()).toInt();
	zx->bdi->fdc->turbo = setupUi.bdtbox->isChecked() ? 1 : 0;

	Floppy* flp = zx->bdi->fdc->flop[0];
	flp->flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (setupUi.a80box->isChecked()) flp->flag |= FLP_TRK80;
	if (setupUi.adsbox->isChecked()) flp->flag |= FLP_DS;
	if (setupUi.awpbox->isChecked()) flp->flag |= FLP_PROTECT;

	flp = zx->bdi->fdc->flop[1];
	flp->flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (setupUi.b80box->isChecked()) flp->flag |= FLP_TRK80;
	if (setupUi.bdsbox->isChecked()) flp->flag |= FLP_DS;
	if (setupUi.bwpbox->isChecked()) flp->flag |= FLP_PROTECT;

	flp = zx->bdi->fdc->flop[2];
	flp->flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (setupUi.c80box->isChecked()) flp->flag |= FLP_TRK80;
	if (setupUi.cdsbox->isChecked()) flp->flag |= FLP_DS;
	if (setupUi.cwpbox->isChecked()) flp->flag |= FLP_PROTECT;

	flp = zx->bdi->fdc->flop[3];
	flp->flag &= ~(FLP_TRK80 | FLP_DS | FLP_PROTECT);
	if (setupUi.d80box->isChecked()) flp->flag |= FLP_TRK80;
	if (setupUi.ddsbox->isChecked()) flp->flag |= FLP_DS;
	if (setupUi.dwpbox->isChecked()) flp->flag |= FLP_PROTECT;

// hdd
	zx->ide->type = setupUi.hiface->itemData(setupUi.hiface->currentIndex()).toInt();

	int flg = zx->ide->master->flags;
	ATAPassport pass = ideGetPassport(zx->ide,IDE_MASTER);

	zx->ide->master->type = setupUi.hm_type->itemData(setupUi.hm_type->currentIndex()).toInt();
	memcpy(pass.model,std::string(setupUi.hm_model->text().toLocal8Bit().data(),40).c_str(),40);
	memcpy(pass.serial,std::string(setupUi.hm_ser->text().toLocal8Bit().data(),20).c_str(),20);
	ideSetImage(zx->ide,IDE_MASTER,setupUi.hm_path->text().toLocal8Bit().data());
	setFlagBit(setupUi.hm_islba->isChecked(),&flg,ATA_LBA);
	zx->ide->master->flags = flg;
	pass.spt = setupUi.hm_gsec->value();
	pass.hds = setupUi.hm_ghd->value();
	pass.cyls = setupUi.hm_gcyl->value();
	zx->ide->master->maxlba = setupUi.hm_glba->value();
	ideSetPassport(zx->ide,IDE_MASTER,pass);

	pass = ideGetPassport(zx->ide,IDE_SLAVE);
	flg = zx->ide->slave->flags;
	zx->ide->slave->type = setupUi.hs_type->itemData(setupUi.hs_type->currentIndex()).toInt();
	memcpy(pass.model,std::string(setupUi.hs_model->text().toLocal8Bit().data(),40).c_str(),40);
	memcpy(pass.serial,std::string(setupUi.hs_ser->text().toLocal8Bit().data(),20).c_str(),20);
	ideSetImage(zx->ide,IDE_SLAVE,setupUi.hm_path->text().toLocal8Bit().data());
	setFlagBit(setupUi.hs_islba->isChecked(),&flg,ATA_LBA);
	zx->ide->slave->flags = flg;
	pass.spt = setupUi.hs_gsec->value();
	pass.hds = setupUi.hs_ghd->value();
	pass.cyls = setupUi.hs_gcyl->value();
	zx->ide->slave->maxlba = setupUi.hs_glba->value();
	ideSetPassport(zx->ide,IDE_SLAVE,pass);
// tape
	optSetFlag(OF_TAPEAUTO,setupUi.cbTapeAuto->isChecked());
	optSetFlag(OF_TAPEFAST,setupUi.cbTapeFast->isChecked());
// tools
	optSet(OPT_ASMPATH,std::string(setupUi.sjpathle->text().toLocal8Bit().data()));
	optSet(OPT_PROJDIR,std::string(setupUi.prjdirle->text().toLocal8Bit().data()));
// leds
	emulSetFlag(FL_LED_DISK,setupUi.diskLed->isChecked());
	emulSetFlag(FL_LED_SHOT,setupUi.shotLed->isChecked());

	saveConfig();
	sndCalibrate();
	emulSetColor(setupUi.brgslide->value());
	emulOpenJoystick(optGetString(OPT_JOYNAME));
	emulUpdateWindow();
}

void SetupWin::reject() {
	hide();
	fillBookmarkMenu();
	emulPause(false,PR_OPTS);
}

void SetupWin::rmRomset() {
	int idx = setupUi.rsetbox->currentIndex();
	if (idx < 0) return;
	if (areSure("Do you really want to delete this romset?")) {
		rsl.erase(rsl.begin() + idx);
		setupUi.rsetbox->removeItem(idx);
	}
}

void SetupWin::addNewRomset() {
//	bool ok = false;
	QString nam = QInputDialog::getText(this,"Enter...","Input romset name"); //,QLineEdit::Normal,"",&ok);
	if (nam.isEmpty()) return;
	RomSet nrs;
	nrs.name = std::string(nam.toLocal8Bit().data());
	uint i;
	for (i=0; i<8; i++) {
		nrs.roms[i].path = "";
		nrs.roms[i].part = 0;
	}
	for (i=0; i<rsl.size(); i++) {
		if (rsl[i].name == nrs.name) return;
	}
	rsl.push_back(nrs);
	setupUi.rsetbox->addItem(QString::fromLocal8Bit(nrs.name.c_str()));
	setupUi.rsetbox->setCurrentIndex(setupUi.rsetbox->count() - 1);
}

// machine

void SetupWin::editrset(QModelIndex idx) {
	int cbx = setupUi.rsetbox->currentIndex();
	if (cbx < 0) return;
	std::string rpth = optGetString(OPT_ROMDIR);
	QDir rdir(QString(rpth.c_str()));
	QStringList rlst = rdir.entryList(QStringList() << "*.rom",QDir::Files,QDir::Name);
	fillRFBox(setupUi.rse_singlefile,rlst);
	fillRFBox(setupUi.rse_file0,rlst);
	fillRFBox(setupUi.rse_file1,rlst);
	fillRFBox(setupUi.rse_file2,rlst);
	fillRFBox(setupUi.rse_file3,rlst);
	fillRFBox(setupUi.rse_gsfile,rlst);
	setupUi.rse_singlefile->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].file.c_str())) + 1);
	setupUi.rse_file0->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[0].path.c_str())) + 1);
	setupUi.rse_file1->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[1].path.c_str())) + 1);
	setupUi.rse_file2->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[2].path.c_str())) + 1);
	setupUi.rse_file3->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[3].path.c_str())) + 1);
	setupUi.rse_part0->setValue(rsl[cbx].roms[0].part);
	setupUi.rse_part1->setValue(rsl[cbx].roms[1].part);
	setupUi.rse_part2->setValue(rsl[cbx].roms[2].part);
	setupUi.rse_part3->setValue(rsl[cbx].roms[3].part);
	setupUi.rse_gsfile->setCurrentIndex(rlst.indexOf(QString(GSRom.c_str())) + 1);
	setupUi.rse_grp_single->setChecked(rsl[cbx].file != "");
	setupUi.rstab->hide();
	setupUi.rsetbox->setEnabled(false);
	setupUi.addrset->setEnabled(false);
	setupUi.rmrset->setEnabled(false);
	setupUi.rssel->show();
}

void SetupWin::setrpart() {
	int cbx = setupUi.rsetbox->currentIndex();
	GSRom = getRFText(setupUi.rse_gsfile);
	if (setupUi.rse_grp_single->isChecked()) {
		rsl[cbx].file = getRFText(setupUi.rse_singlefile);
	} else {
		rsl[cbx].file = "";
	}
	rsl[cbx].roms[0].path = getRFText(setupUi.rse_file0); rsl[cbx].roms[0].part = setupUi.rse_part0->value();
	rsl[cbx].roms[1].path = getRFText(setupUi.rse_file1); rsl[cbx].roms[1].part = setupUi.rse_part1->value();
	rsl[cbx].roms[2].path = getRFText(setupUi.rse_file2); rsl[cbx].roms[2].part = setupUi.rse_part2->value();
	rsl[cbx].roms[3].path = getRFText(setupUi.rse_file3); rsl[cbx].roms[3].part = setupUi.rse_part3->value();
	buildrsetlist();
	hidersedit();
}

void SetupWin::hidersedit() {
	setupUi.rssel->hide();
	setupUi.rstab->show();
	setupUi.rsetbox->setEnabled(true);
	setupUi.addrset->setEnabled(true);
	setupUi.rmrset->setEnabled(true);
}

// lists

void SetupWin::buildkeylist() {
	std::string wdir = optGetString(OPT_WORKDIR);
	QDir dir(wdir.c_str());
	QStringList lst = dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name);
	fillRFBox(setupUi.keyMapBox,lst);
}

void SetupWin::buildjmaplist() {
	std::vector<joyPair> jmap = getJMap();
	setupUi.bindTable->setRowCount(jmap.size());
	setupUi.bindTable->setColumnCount(2);
	QTableWidgetItem* it;
	QString qstr;
	for (uint i=0;i<jmap.size();i++) {
		it = new QTableWidgetItem;
		switch(jmap[i].second.dev) {
			case XJ_JOY:
				it->setText(QString("Kempston ").append(QString(jmap[i].second.name)));
				break;
			case XJ_KEY:
				it->setText("Key");
				break;
			default:
				it->setText("Unknown");
				break;
		}
		setupUi.bindTable->setItem(i,1,it);
		it = new QTableWidgetItem;
		switch (jmap[i].first.type) {
			case XJ_BUTTON:
				it->setText(QString("Button ").append(QString::number(jmap[i].first.num)));
				break;
			case XJ_AXIS:
				it->setText(QString("Axis ").append(QString::number(jmap[i].first.num)).append(jmap[i].first.dir ? " +" : " -"));
				break;
			default:
				it->setText("Unknown");
				break;
		}
		it->setData(Qt::UserRole,QVector3D(jmap[i].first.type,jmap[i].first.num,jmap[i].first.dir));
		setupUi.bindTable->setItem(i,0,it);
	}
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
	RomSet rset = rsl[setupUi.rsetbox->currentIndex()];
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
	setupUi.rstab->item(5,1)->setText(QString::fromLocal8Bit(GSRom.c_str()));
	setupUi.rstab->setColumnWidth(0,100);
	setupUi.rstab->setColumnWidth(1,300);
	setupUi.rstab->setColumnWidth(2,50);
}

void SetupWin::buildtapelist() {
//	buildTapeList();
	TapeBlockInfo inf[zx->tape->blkCount];
	tapGetBlocksInfo(zx->tape,inf);
	setupUi.tapelist->setRowCount(zx->tape->blkCount);
	if (zx->tape->blkCount == 0) {
		setupUi.tapelist->setEnabled(false);
		return;
	}
	setupUi.tapelist->setEnabled(true);
	QTableWidgetItem* itm;
	uint tm,ts;
	for (int i=0; i < zx->tape->blkCount; i++) {
		if (zx->tape->block == i) {
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
		if (inf[i].flag & TBF_BREAK) itm->setIcon(QIcon(":/images/cancel.png"));
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
	std::vector<XBookmark> bml = getBookmarkList();
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
	std::vector<XProfile> prList = getProfileList();
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
	flpGetTRCatalog(zx->bdi->fdc->flop[dsk],cat);
	// std::vector<TRFile> cat = flpGetTRCatalog(zx->bdi->flop[dsk]);
	int row;
	uint8_t* buf = new uint8_t[0xffff];
	uint16_t line,start,len;
	char name[10];
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		row = idx[i].row();
		if (flpGetSectorsData(zx->bdi->fdc->flop[dsk],cat[row].trk, cat[row].sec+1, buf, cat[row].slen)) {
			if (cat[row].slen == (cat[row].hlen + ((cat[row].llen == 0) ? 0 : 1))) {
				start = (cat[row].hst << 8) + cat[row].lst;
				len = (cat[row].hlen << 8) + cat[row].llen;
				line = (cat[row].ext == 'B') ? (buf[start] + (buf[start+1] << 8)) : 0x8000;
				strncpy(name,(char*)&cat[row].name[0],8);
				strcat(name,".");
				strncat(name,(char*)&cat[row].ext,1);
				// name = std::string((char*)&cat[row].name[0],8) + std::string(".") + std::string((char*)&cat[row].ext,1);
				tapAddFile(zx->tape,name,(cat[row].ext == 'B') ? 0 : 3, start, len, line, buf,true);
				savedFiles++;
			} else {
				shitHappens("File seems to be joined, skip");
			}
		} else {
			shitHappens("Can't get file data, skip");
		}
	}
	buildtapelist();
	std::string msg = int2str(savedFiles) + std::string(" of ") + int2str(idx.size()) + " files copied";
	showInfo(msg.c_str());
}

#ifdef WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

// hobeta header crc = ((105 + 257 * std::accumulate(data, data + 15, 0u)) & 0xffff))

void SetupWin::diskToHobeta() {
	QModelIndexList idx = setupUi.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...",QDir::homePath());
	if (dir == "") return;
	std::string sdir = std::string(dir.toLocal8Bit().data()) + std::string(SLASH);
	Floppy* flp = zx->bdi->fdc->flop[setupUi.disktabs->currentIndex()];		// selected floppy
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveHobetaFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = int2str(savedFiles) + std::string(" of ") + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

void SetupWin::diskToRaw() {
	QModelIndexList idx = setupUi.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...",QDir::homePath());
	if (dir == "") return;
	std::string sdir = std::string(dir.toLocal8Bit().data()) + std::string(SLASH);
	Floppy* flp = zx->bdi->fdc->flop[setupUi.disktabs->currentIndex()];
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveRawFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = int2str(savedFiles) + std::string(" of ") + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

TRFile getHeadInfo(int blk) {
	TRFile res;
	TapeBlockInfo inf = tapGetBlockInfo(zx->tape,blk);
	uint8_t dt[inf.size];
	tapGetBlockData(zx->tape,blk,dt);
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
	if (~zx->tape->blkData[blk].flag & TBF_BYTES) {
		shitHappens("This is not standard block");
		return;
	}
	if (zx->tape->blkData[blk].flag & TBF_HEAD) {
		if ((int)zx->tape->blkCount == blk + 1) {
			shitHappens("Header without data? Hmm...");
		} else {
			if (~zx->tape->blkData[blk+1].flag & TBF_BYTES) {
				shitHappens("Data block is not standard");
			} else {
				headBlock = blk;
				dataBlock = blk + 1;
			}
		}
	} else {
		dataBlock = blk;
		if (blk != 0) {
			if (zx->tape->blkData[blk-1].flag & TBF_HEAD) {
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
		TapeBlockInfo binf = tapGetBlockInfo(zx->tape,dataBlock);
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
		dsc = getHeadInfo(headBlock);
		if (dsc.ext == 0x00) {
			shitHappens("Yes, it happens");
			return;
		}
	}
	if (!(zx->bdi->fdc->flop[dsk]->flag & FLP_INSERT)) newdisk(dsk);
	TapeBlockInfo inf = tapGetBlockInfo(zx->tape,dataBlock);
	uint8_t dt[inf.size];
	tapGetBlockData(zx->tape,dataBlock,dt);
	uint8_t* buf = new uint8_t[256];
	int pos = 1;	// skip block type mark
	switch(flpCreateFile(zx->bdi->fdc->flop[dsk],&dsc)) {
		case ERR_SHIT: shitHappens("Yes, it happens"); break;
		case ERR_MANYFILES: shitHappens("Too many files @ disk"); break;
		case ERR_NOSPACE: shitHappens("Not enough space @ disk"); break;
		case ERR_OK:
			while (pos < inf.size) {
				do {
					buf[(pos-1) & 0xff] = (pos < inf.size) ? dt[pos] : 0x00;
					pos++;
				} while ((pos & 0xff) != 1);
				flpPutSectorData(zx->bdi->fdc->flop[dsk],dsc.trk, dsc.sec+1, buf, 256);
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
	if (!(zx->bdi->fdc->flop[dsk]->flag & FLP_INSERT)) {
		wid->setEnabled(false);
		wid->setRowCount(0);
	} else {
		wid->setEnabled(true);
		if (flpGet(zx->bdi->fdc->flop[dsk],FLP_DISKTYPE) == DISK_TYPE_TRD) {
			TRFile cat[128];
			int catSize = flpGetTRCatalog(zx->bdi->fdc->flop[dsk],cat);
			// std::vector<TRFile> cat = flpGetTRCatalog(zx->bdi->flop[dsk]);
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
void SetupWin::chabrg() {setupUi.brglab->setText(QString::number(setupUi.brgslide->value()));}

void SetupWin::selsspath() {
	QString fpath = QFileDialog::getExistingDirectory(this,"Screenshots folder",QString::fromLocal8Bit(optGetString(OPT_SHOTDIR).c_str()),QFileDialog::ShowDirsOnly);
	if (fpath!="") setupUi.pathle->setText(fpath);
}

// sound

void SetupWin::updvolumes() {
	setupUi.bvlab->setText(QString::number(setupUi.bvsld->value()));
	setupUi.tvlab->setText(QString::number(setupUi.tvsld->value()));
	setupUi.avlab->setText(QString::number(setupUi.avsld->value()));
	setupUi.gslab->setText(QString::number(setupUi.gvsld->value()));
}

// input

QWidget* dia;
QComboBox* box;

void SetupWin::addJoyBind() {
	if (!emulIsJoystickOpened()) return;
	dia = new QWidget(this);
	QHBoxLayout* lay = new QHBoxLayout;
	box = new QComboBox;
	box->addItem("Kempston up","up");
	box->addItem("Kempston down","down");
	box->addItem("Kempston left","left");
	box->addItem("Kempston right","right");
	box->addItem("Kempston fire","fire");
	QPushButton* but = new QPushButton("Scan");
	lay->addWidget(box);
	lay->addWidget(but);
	dia->setLayout(lay);
	connect (but,SIGNAL(released()),this,SLOT(scanJoyBind()));
	dia->setEnabled(true);
	dia->show();
}

void SetupWin::scanJoyBind() {
#ifndef XQTPAINT
	dia->setEnabled(false);
	SDL_Event ev;
	bool doWork = true;
	extButton extb;
	intButton intb;
	do {
		SDL_Delay(100);
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_JOYBUTTONUP:
					extb.type = XJ_BUTTON;
					extb.num = ev.jbutton.button;
					extb.dir = true;
					intb.dev = XJ_JOY;
					intb.name = box->itemData(box->currentIndex()).toString().toLocal8Bit().data();
					optSetJMap(extb,intb);
					doWork = false;
					break;
				case SDL_JOYAXISMOTION:
					if ((ev.jaxis.value < -5000) || (ev.jaxis.value > 5000)) {
						extb.type = XJ_AXIS;
						extb.num = ev.jaxis.axis;
						extb.dir = (ev.jaxis.value > 0);
						intb.dev = XJ_JOY;
						intb.name = box->itemData(box->currentIndex()).toString().toLocal8Bit().data();
						optSetJMap(extb,intb);
						doWork = false;
					}
					break;
			}
		}
	} while (doWork);
	dia->hide();
	buildjmaplist();
#endif
}

void SetupWin::delJoyBind() {
	int row = setupUi.bindTable->currentRow();
	if (row < 0) return;
	if (setupUi.bindTable->isRowHidden(row)) return;
	QVector3D vec = setupUi.bindTable->item(row,0)->data(Qt::UserRole).value<QVector3D>();
	extButton extb;
	extb.type = vec.x();
	extb.num = vec.y();
	extb.dir = (vec.z() != 0);
	optDelJMap(extb);
	buildjmaplist();
}

// disk

void SetupWin::newdisk(int idx) {
	Floppy *flp = zx->bdi->fdc->flop[idx];
	if (!saveChangedDisk(idx & 3)) return;
	flpFormat(flp);
	flp->path = (char*)realloc(flp->path,sizeof(char));
	flp->path[0] = 0x00;
	flp->flag |= (FLP_INSERT | FLP_CHANGED);
	updatedisknams();
}

void SetupWin::newa() {newdisk(0);}
void SetupWin::newb() {newdisk(1);}
void SetupWin::newc() {newdisk(2);}
void SetupWin::newd() {newdisk(3);}

void SetupWin::loada() {loadFile("",FT_DISK,0); updatedisknams();}
void SetupWin::loadb() {loadFile("",FT_DISK,1); updatedisknams();}
void SetupWin::loadc() {loadFile("",FT_DISK,2); updatedisknams();}
void SetupWin::loadd() {loadFile("",FT_DISK,3); updatedisknams();}

void SetupWin::savea() {Floppy* flp = zx->bdi->fdc->flop[0]; if (flp->flag & FLP_INSERT) saveFile(flp->path,FT_DISK,0);}
void SetupWin::saveb() {Floppy* flp = zx->bdi->fdc->flop[1]; if (flp->flag & FLP_INSERT) saveFile(flp->path,FT_DISK,1);}
void SetupWin::savec() {Floppy* flp = zx->bdi->fdc->flop[2]; if (flp->flag & FLP_INSERT) saveFile(flp->path,FT_DISK,2);}
void SetupWin::saved() {Floppy* flp = zx->bdi->fdc->flop[3]; if (flp->flag & FLP_INSERT) saveFile(flp->path,FT_DISK,3);}

void SetupWin::ejcta() {saveChangedDisk(0); flpEject(zx->bdi->fdc->flop[0]); updatedisknams();}
void SetupWin::ejctb() {saveChangedDisk(1); flpEject(zx->bdi->fdc->flop[1]); updatedisknams();}
void SetupWin::ejctc() {saveChangedDisk(2); flpEject(zx->bdi->fdc->flop[2]); updatedisknams();}
void SetupWin::ejctd() {saveChangedDisk(3); flpEject(zx->bdi->fdc->flop[3]); updatedisknams();}

void SetupWin::updatedisknams() {
	setupUi.apathle->setText(QString::fromLocal8Bit(zx->bdi->fdc->flop[0]->path));
	setupUi.bpathle->setText(QString::fromLocal8Bit(zx->bdi->fdc->flop[1]->path));
	setupUi.cpathle->setText(QString::fromLocal8Bit(zx->bdi->fdc->flop[2]->path));
	setupUi.dpathle->setText(QString::fromLocal8Bit(zx->bdi->fdc->flop[3]->path));
	fillDiskCat();
}

// tape

void SetupWin::loatape() {
	loadFile("",FT_TAPE,1);
	setupUi.tpathle->setText(QString::fromLocal8Bit(zx->tape->path));
	buildtapelist();
}

void SetupWin::savtape() {
	if (zx->tape->blkCount != 0) saveFile(zx->tape->path,FT_TAP,-1);
}

void SetupWin::ejctape() {
	tapEject(zx->tape);
	setupUi.tpathle->setText(QString::fromLocal8Bit(zx->tape->path));
	buildtapelist();
}

void SetupWin::tblkup() {
	int ps = setupUi.tapelist->currentIndex().row();
	if (ps > 0) {
		tapSwapBlocks(zx->tape,ps,ps-1);
		buildtapelist();
		setupUi.tapelist->selectRow(ps-1);
	}
}

void SetupWin::tblkdn() {
	int ps = setupUi.tapelist->currentIndex().row();
	if ((ps != -1) && (ps < zx->tape->blkCount - 1)) {
		tapSwapBlocks(zx->tape,ps,ps+1);
		buildtapelist();
		setupUi.tapelist->selectRow(ps+1);
	}
}

void SetupWin::tblkrm() {
	int ps = setupUi.tapelist->currentIndex().row();
	if (ps != -1) {
		tapDelBlock(zx->tape,ps);
		buildtapelist();
		setupUi.tapelist->selectRow(ps);
	}
}

void SetupWin::chablock(QModelIndex idx) {
	int row = idx.row();
	tapRewind(zx->tape,row);
	buildtapelist();
	setupUi.tapelist->selectRow(row);
}

void SetupWin::setTapeBreak(int row,int col) {
	if ((row < 0) || (col != 1)) return;
	zx->tape->blkData[row].flag ^= TBF_BREAK;
	buildtapelist();
	setupUi.tapelist->selectRow(row);
}

// hdd

void SetupWin::hddMasterImg() {
	QString path = QFileDialog::getSaveFileName(this,"Image for master HDD",QDir::homePath(),"All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") setupUi.hm_path->setText(path);
}

void SetupWin::hddSlaveImg() {
	QString path = QFileDialog::getSaveFileName(this,"Image for slave HDD",QDir::homePath(),"All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") setupUi.hs_path->setText(path);
}

void SetupWin::hddcap() {
	uint32_t sz;
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

// tools

void SetupWin::ssjapath() {
	QString fnam = QFileDialog::getOpenFileName(NULL,"Select SJAsm executable",QDir::homePath(),"All files (*)");
	if (fnam!="") setupUi.sjpathle->setText(fnam);
}

void SetupWin::sprjpath() {
	QString fnam = QFileDialog::getExistingDirectory(this,"Projects file",QString::fromLocal8Bit(optGetString(OPT_PROJDIR).c_str()),QFileDialog::ShowDirsOnly);
	if (fnam!="") setupUi.prjdirle->setText(fnam);
}

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
	fillProfileMenu();
}

void SetupWin::rmProfile() {
	int idx = setupUi.twProfileList->currentRow();
	if (idx < 0) return;
	if (!areSure("Do you really want to delete this profile?")) return;
	std::string pnam(setupUi.twProfileList->item(idx,0)->text().toLocal8Bit().data());
	idx = delProfile(pnam);
	switch(idx) {
		case DELP_OK_CURR:
			start();
			break;
		case DELP_ERR:
			shitHappens("Sorry, i can't delete this profile");
			break;
	}
	buildproflist();
	fillProfileMenu();
}
