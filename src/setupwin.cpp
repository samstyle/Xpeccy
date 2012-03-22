#include <QStandardItemModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QVector3D>
#include <QDebug>
#include <SDL.h>

#include "common.h"
#include "sound.h"
#include "spectrum.h"
#include "setupwin.h"
#include "emulwin.h"
#include "settings.h"
#include "filer.h"
#include "filetypes/filetypes.h"

//#include "ui_selname.h"
#include "ui_setupwin.h"
#include "ui_umadial.h"

extern ZXComp* zx;

Ui::SetupWin ui;
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
	return std::string(res.toUtf8().data());
}

// OBJECT

SetupWin::SetupWin(QWidget* par):QDialog(par) {
	setModal(true);
	ui.setupUi(this);

	umadial = new QDialog;
	uia.setupUi(umadial);
	umadial->setModal(true);

	uint32_t i;
	std::vector<std::string> list;
// machine
	list = getHardwareNames();
	for (i=0; i < list.size(); i++) {
		ui.machbox->addItem(QDialog::trUtf8(list[i].c_str()));
	}
	ui.resbox->addItems(QStringList()<<"0:Basic 128"<<"1:Basic48"<<"2:Shadow"<<"3:DOS");
	ui.mszbox->addItems(QStringList()<<"48K"<<"128K"<<"256K"<<"512K"<<"1024K");
	ui.rssel->hide();
	QTableWidgetItem* itm;
	for (i=0; i<6; i++) {
		itm = new QTableWidgetItem; ui.rstab->setItem(i,1,itm);
		itm = new QTableWidgetItem; ui.rstab->setItem(i,2,itm);
	}
// video
	OptName* ptr = getGetPtr(OPT_SHOTFRM);
	i = 0; while (ptr[i].id != -1) {
		ui.ssfbox->addItem(QString(ptr[i].name.c_str()),QVariant(ptr[i].id));
		i++;
	}
	std::vector<VidLayout> lays = getLayoutList();
	for (i=0; i<lays.size(); i++) {ui.geombox->addItem(QDialog::trUtf8(lays[i].name.c_str()));}
// sound
	list = sndGetList();
	for (i=0;i<list.size();i++) {ui.outbox->addItem(QDialog::trUtf8(list[i].c_str()));}
	ui.ratbox->addItems(QStringList()<<"44100"<<"22050"<<"11025");
	ui.schip1box->addItem(QIcon(":/images/cancel.png"),"none",QVariant(SND_NONE));
	ui.schip1box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",QVariant(SND_AY));
	ui.schip1box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",QVariant(SND_YM));
	ui.schip2box->addItem(QIcon(":/images/cancel.png"),"none",QVariant(SND_NONE));
	ui.schip2box->addItem(QIcon(":/images/MicrochipLogo.png"),"AY-3-8910",QVariant(SND_AY));
	ui.schip2box->addItem(QIcon(":/images/YamahaLogo.png"),"Yamaha 2149",QVariant(SND_YM));
	ui.stereo1box->addItem("Mono",QVariant(AY_MONO)); ui.stereo2box->addItem("Mono",QVariant(AY_MONO));
	ui.stereo1box->addItem("ABC",QVariant(AY_ABC)); ui.stereo2box->addItem("ABC",QVariant(AY_ABC));
	ui.stereo1box->addItem("ACB",QVariant(AY_ACB)); ui.stereo2box->addItem("ACB",QVariant(AY_ACB));
	ui.stereo1box->addItem("BAC",QVariant(AY_BAC)); ui.stereo2box->addItem("BAC",QVariant(AY_BAC));
	ui.stereo1box->addItem("BCA",QVariant(AY_BCA)); ui.stereo2box->addItem("BCA",QVariant(AY_BCA));
	ui.stereo1box->addItem("CAB",QVariant(AY_CAB)); ui.stereo2box->addItem("CAB",QVariant(AY_CAB));
	ui.stereo1box->addItem("CBA",QVariant(AY_CBA)); ui.stereo2box->addItem("CBA",QVariant(AY_BCA));
	ui.tsbox->addItem("None",QVariant(TS_NONE));
	ui.tsbox->addItem("NedoPC",QVariant(TS_NEDOPC));
	ui.gstereobox->addItem("Mono",QVariant(GS_MONO));
	ui.gstereobox->addItem("L:1,2; R:3,4",QVariant(GS_12_34));
// bdi
// WTF? QtDesigner doesn't save this properties
	ui.disklist->horizontalHeader()->setVisible(true);
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
	ui.hiface->addItem("None",QVariant(IDE_NONE));
	ui.hiface->addItem("Nemo",QVariant(IDE_NEMO));
	ui.hiface->addItem("Nemo A8",QVariant(IDE_NEMOA8));
	ui.hiface->addItem("SMUC",QVariant(IDE_SMUC));
	ui.hm_type->addItem(QIcon(":/images/cancel.png"),"Not connected",QVariant(IDE_NONE));
	ui.hm_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",QVariant(IDE_ATA));
//	ui.hm_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",QVariant(IDE_ATAPI));
	ui.hs_type->addItem(QIcon(":/images/cancel.png"),"Not connected",QVariant(IDE_NONE));
	ui.hs_type->addItem(QIcon(":/images/hdd.png"),"HDD (ATA)",QVariant(IDE_ATA));
//	ui.hs_type->addItem(QIcon(":/images/cd.png"),"CD (ATAPI) not working yet",QVariant(IDE_ATAPI));

// all
	QObject::connect(ui.okbut,SIGNAL(released()),this,SLOT(okay()));
	QObject::connect(ui.apbut,SIGNAL(released()),this,SLOT(apply()));
	QObject::connect(ui.cnbut,SIGNAL(released()),this,SLOT(reject()));
// machine
	QObject::connect(ui.rsetbox,SIGNAL(currentIndexChanged(int)),this,SLOT(buildrsetlist()));
	QObject::connect(ui.machbox,SIGNAL(currentIndexChanged(int)),this,SLOT(setmszbox(int)));
	QObject::connect(ui.mszbox,SIGNAL(currentIndexChanged(int)),this,SLOT(okbuts()));
	QObject::connect(ui.cpufrq,SIGNAL(valueChanged(int)),this,SLOT(updfrq()));
	QObject::connect(ui.rstab,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(editrset(QModelIndex)));
	QObject::connect(ui.rse_cancel,SIGNAL(released()),this,SLOT(hidersedit()));
	QObject::connect(ui.rse_apply,SIGNAL(released()),this,SLOT(setrpart()));
	QObject::connect(ui.rse_grp_single,SIGNAL(toggled(bool)),this,SLOT(recheck_separate(bool)));
	QObject::connect(ui.rse_grp_separate,SIGNAL(toggled(bool)),this,SLOT(recheck_single(bool)));
	QObject::connect(ui.addrset,SIGNAL(released()),this,SLOT(addNewRomset()));
	QObject::connect(ui.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
// video
	QObject::connect(ui.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	QObject::connect(ui.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));
	QObject::connect(ui.brgslide,SIGNAL(valueChanged(int)),this,SLOT(chabrg()));
// sound
	QObject::connect(ui.bvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(ui.tvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(ui.avsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(ui.gvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
// input
	connect(ui.addBind,SIGNAL(released()),this,SLOT(addJoyBind()));
	connect(ui.delBind,SIGNAL(released()),this,SLOT(delJoyBind()));
// dos
	QObject::connect(ui.newatb,SIGNAL(released()),this,SLOT(newa()));
	QObject::connect(ui.newbtb,SIGNAL(released()),this,SLOT(newb()));
	QObject::connect(ui.newctb,SIGNAL(released()),this,SLOT(newc()));
	QObject::connect(ui.newdtb,SIGNAL(released()),this,SLOT(newd()));

	QObject::connect(ui.loadatb,SIGNAL(released()),this,SLOT(loada()));
	QObject::connect(ui.loadbtb,SIGNAL(released()),this,SLOT(loadb()));
	QObject::connect(ui.loadctb,SIGNAL(released()),this,SLOT(loadc()));
	QObject::connect(ui.loaddtb,SIGNAL(released()),this,SLOT(loadd()));

	QObject::connect(ui.saveatb,SIGNAL(released()),this,SLOT(savea()));
	QObject::connect(ui.savebtb,SIGNAL(released()),this,SLOT(saveb()));
	QObject::connect(ui.savectb,SIGNAL(released()),this,SLOT(savec()));
	QObject::connect(ui.savedtb,SIGNAL(released()),this,SLOT(saved()));

	QObject::connect(ui.remoatb,SIGNAL(released()),this,SLOT(ejcta()));
	QObject::connect(ui.remobtb,SIGNAL(released()),this,SLOT(ejctb()));
	QObject::connect(ui.remoctb,SIGNAL(released()),this,SLOT(ejctc()));
	QObject::connect(ui.remodtb,SIGNAL(released()),this,SLOT(ejctd()));

	QObject::connect(ui.disktabs,SIGNAL(currentChanged(int)),this,SLOT(fillDiskCat()));
	connect(ui.actCopyToTape,SIGNAL(triggered()),this,SLOT(copyToTape()));
	connect(ui.actSaveHobeta,SIGNAL(triggered()),this,SLOT(diskToHobeta()));
	connect(ui.actSaveRaw,SIGNAL(triggered()),this,SLOT(diskToRaw()));
	connect(ui.tbToTape,SIGNAL(released()),this,SLOT(copyToTape()));
	connect(ui.tbToHobeta,SIGNAL(released()),this,SLOT(diskToHobeta()));
	connect(ui.tbToRaw,SIGNAL(released()),this,SLOT(diskToRaw()));
// tape
	QObject::connect(ui.tapelist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chablock(QModelIndex)));
	QObject::connect(ui.tapelist,SIGNAL(cellClicked(int,int)),this,SLOT(setTapeBreak(int,int)));
	QObject::connect(ui.tloadtb,SIGNAL(released()),this,SLOT(loatape()));
	QObject::connect(ui.tsavetb,SIGNAL(released()),this,SLOT(savtape()));
	QObject::connect(ui.tremotb,SIGNAL(released()),this,SLOT(ejctape()));
	QObject::connect(ui.blkuptb,SIGNAL(released()),this,SLOT(tblkup()));
	QObject::connect(ui.blkdntb,SIGNAL(released()),this,SLOT(tblkdn()));
	QObject::connect(ui.blkrmtb,SIGNAL(released()),this,SLOT(tblkrm()));
	connect(ui.actCopyToDisk,SIGNAL(triggered()),this,SLOT(copyToDisk()));
	connect(ui.tbToDisk,SIGNAL(released()),this,SLOT(copyToDisk()));
// hdd
	QObject::connect(ui.hm_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hm_pathtb,SIGNAL(released()),this,SLOT(hddMasterImg()));
	QObject::connect(ui.hs_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	connect(ui.hs_pathtb,SIGNAL(released()),this,SLOT(hddSlaveImg()));
//tools
	QObject::connect(ui.sjselptb,SIGNAL(released()),this,SLOT(ssjapath()));
	QObject::connect(ui.pdselptb,SIGNAL(released()),this,SLOT(sprjpath()));
	QObject::connect(ui.umlist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(umedit(QModelIndex)));
	QObject::connect(ui.umaddtb,SIGNAL(released()),this,SLOT(umadd()));
	QObject::connect(ui.umdeltb,SIGNAL(released()),this,SLOT(umdel()));
	QObject::connect(ui.umuptb,SIGNAL(released()),this,SLOT(umup()));
	QObject::connect(ui.umdntb,SIGNAL(released()),this,SLOT(umdn()));
//usermenu add dialog
	QObject::connect(uia.umasptb,SIGNAL(released()),this,SLOT(umaselp()));
	QObject::connect(uia.umaok,SIGNAL(released()),this,SLOT(umaconf()));
	QObject::connect(uia.umacn,SIGNAL(released()),umadial,SLOT(hide()));
}

void SetupWin::recheck_single(bool st) {
	ui.rse_grp_single->setChecked(!st);
}
void SetupWin::recheck_separate(bool st) {ui.rse_grp_separate->setChecked(!st);}

void SetupWin::okay() {apply();reject();}

void SetupWin::start() {
	uint32_t i;
	emulPause(true,PR_OPTS);
// machine
	ui.rstab->show();
	ui.rssel->hide();
	ui.rsetbox->setEnabled(true);
	ui.addrset->setEnabled(true);
	ui.rmrset->setEnabled(true);
	ui.rsetbox->clear();
	rsl = getRomsetList();
	GSRom = zx->opt.GSRom;
	for (i=0; i < rsl.size(); i++) {
		ui.rsetbox->addItem(QDialog::trUtf8(rsl[i].name.c_str()));
	}
	ui.machbox->setCurrentIndex(ui.machbox->findText(QDialog::trUtf8(zx->hw->name.c_str())));
	int cbx = -1;
	RomSet* rset = memGetRomset(zx->mem);
	if (rset != NULL) cbx = ui.rsetbox->findText(QDialog::trUtf8(rset->name.c_str()));
	ui.rsetbox->setCurrentIndex(cbx);
	ui.reschk->setChecked(emulGetFlags() & FL_RESET);
	ui.resbox->setCurrentIndex(zx->resbank);
	switch(memGet(zx->mem,MEM_MEMSIZE)) {
		case 48: ui.mszbox->setCurrentIndex(0); break;
		case 128: ui.mszbox->setCurrentIndex(1); break;
		case 256: ui.mszbox->setCurrentIndex(2); break;
		case 512: ui.mszbox->setCurrentIndex(3); break;
		case 1024: ui.mszbox->setCurrentIndex(4); break;
	}
	ui.cpufrq->setValue(zx->cpuFrq * 2); updfrq();
	ui.scrpwait->setChecked(zx->hwFlags & WAIT_ON);
// video
	ui.dszchk->setChecked((zx->vid->flags & VF_DOUBLE));
//	ui.fscchk->setChecked(vid->fscreen);
	ui.bszsld->setValue((int)(zx->vid->brdsize * 100));
	ui.pathle->setText(QDialog::trUtf8(optGetString(OPT_SHOTDIR).c_str()));
	ui.ssfbox->setCurrentIndex(ui.ssfbox->findData(optGetInt(OPT_SHOTFRM)));
	ui.scntbox->setValue(optGetInt(OPT_SHOTCNT));
	ui.sintbox->setValue(optGetInt(OPT_SHOTINT));
	ui.geombox->setCurrentIndex(ui.geombox->findText(QDialog::trUtf8(zx->vid->curlay.c_str())));
	ui.brgslide->setValue(optGetInt(OPT_BRGLEV));
// sound
	ui.senbox->setChecked(sndGet(SND_ENABLE) != 0);
	ui.mutbox->setChecked(sndGet(SND_MUTE) != 0);
	ui.gsrbox->setChecked(gsGet(zx->gs,GS_FLAG) & GS_RESET);
	ui.outbox->setCurrentIndex(ui.outbox->findText(QDialog::trUtf8(sndGetOutputName().c_str())));
	ui.ratbox->setCurrentIndex(ui.ratbox->findText(QString::number(sndGet(SND_RATE))));
	ui.bvsld->setValue(sndGet(SND_BEEP));
	ui.tvsld->setValue(sndGet(SND_TAPE));
	ui.avsld->setValue(sndGet(SND_AYVL));
	ui.gvsld->setValue(sndGet(SND_GSVL));
	ui.schip1box->setCurrentIndex(ui.schip1box->findData(QVariant(tsGet(zx->ts,AY_TYPE,0))));
	ui.schip2box->setCurrentIndex(ui.schip2box->findData(QVariant(tsGet(zx->ts,AY_TYPE,1))));
	ui.stereo1box->setCurrentIndex(ui.stereo1box->findData(QVariant(tsGet(zx->ts,AY_STEREO,0))));
	ui.stereo2box->setCurrentIndex(ui.stereo2box->findData(QVariant(tsGet(zx->ts,AY_STEREO,1))));
	ui.gstereobox->setCurrentIndex(ui.gstereobox->findData(QVariant(gsGet(zx->gs,GS_STEREO))));
	ui.gsgroup->setChecked(gsGet(zx->gs,GS_FLAG) & GS_ENABLE);
	ui.tsbox->setCurrentIndex(ui.tsbox->findData(QVariant(tsGet(zx->ts,TS_TYPE,0))));
// input
	buildkeylist();
	buildjmaplist();
	std::string kmname = optGetString(OPT_KEYNAME);
	int idx = ui.keyMapBox->findText(QString(kmname.c_str()));
	if (idx < 1) idx = 0;
	ui.keyMapBox->setCurrentIndex(idx);
#ifdef XQTPAINT
	ui.joyBox->setEnabled(false);
#else
	ui.joyBox->setEnabled(true);
	ui.inpDevice->clear();
	ui.inpDevice->addItem("None");

	int jnum=SDL_NumJoysticks();
	for (int cnt=0; cnt<jnum; cnt++) {
		ui.inpDevice->addItem(QString(SDL_JoystickName(cnt)));
	}
	idx = ui.inpDevice->findText(QString(optGetString(OPT_JOYNAME).c_str()));
	if (idx < 0) idx = 0;
	ui.inpDevice->setCurrentIndex(idx);
#endif
// dos
	ui.bdebox->setChecked(bdiGetFlag(zx->bdi,BDI_ENABLE));
	ui.bdtbox->setChecked(bdiGetFlag(zx->bdi,BDI_TURBO));
	Floppy* flp = bdiGetFloppy(zx->bdi,0);
	ui.apathle->setText(QDialog::trUtf8(flpGetPath(flp).c_str()));
		ui.a80box->setChecked(flpGetFlag(flp,FLP_TRK80));
		ui.adsbox->setChecked(flpGetFlag(flp,FLP_DS));
		ui.awpbox->setChecked(flpGetFlag(flp,FLP_PROTECT));
	flp = bdiGetFloppy(zx->bdi,1);
	ui.bpathle->setText(QDialog::trUtf8(flpGetPath(flp).c_str()));
		ui.b80box->setChecked(flpGetFlag(flp,FLP_TRK80));
		ui.bdsbox->setChecked(flpGetFlag(flp,FLP_DS));
		ui.bwpbox->setChecked(flpGetFlag(flp,FLP_PROTECT));
	flp = bdiGetFloppy(zx->bdi,2);
	ui.cpathle->setText(QDialog::trUtf8(flpGetPath(flp).c_str()));
		ui.c80box->setChecked(flpGetFlag(flp,FLP_TRK80));
		ui.cdsbox->setChecked(flpGetFlag(flp,FLP_DS));
		ui.cwpbox->setChecked(flpGetFlag(flp,FLP_PROTECT));
	flp = bdiGetFloppy(zx->bdi,3);
	ui.dpathle->setText(QDialog::trUtf8(flpGetPath(flp).c_str()));
		ui.d80box->setChecked(flpGetFlag(flp,FLP_TRK80));
		ui.ddsbox->setChecked(flpGetFlag(flp,FLP_DS));
		ui.dwpbox->setChecked(flpGetFlag(flp,FLP_PROTECT));
	fillDiskCat();
// hdd
	ui.hiface->setCurrentIndex(ui.hiface->findData(ideGet(zx->ide,IDE_NONE,IDE_TYPE)));

	ui.hm_type->setCurrentIndex(ui.hm_type->findData(ideGet(zx->ide,IDE_MASTER,IDE_TYPE)));
	ATAPassport pass = ideGetPassport(zx->ide,IDE_MASTER);
	ui.hm_model->setText(QDialog::trUtf8(pass.model.c_str()));
	ui.hm_ser->setText(QDialog::trUtf8(pass.serial.c_str()));
	ui.hm_path->setText(QDialog::trUtf8(ideGetPath(zx->ide,IDE_MASTER).c_str()));
	ui.hm_islba->setChecked(ideGet(zx->ide,IDE_MASTER,IDE_FLAG) & ATA_LBA);
	ui.hm_gsec->setValue(pass.spt);
	ui.hm_ghd->setValue(pass.hds);
	ui.hm_gcyl->setValue(pass.cyls);
	ui.hm_glba->setValue(ideGet(zx->ide,IDE_MASTER,IDE_MAXLBA));

	ui.hs_type->setCurrentIndex(ui.hm_type->findData(ideGet(zx->ide,IDE_SLAVE,IDE_TYPE)));
	pass = ideGetPassport(zx->ide,IDE_SLAVE);
	ui.hs_model->setText(QDialog::trUtf8(pass.model.c_str()));
	ui.hs_ser->setText(QDialog::trUtf8(pass.serial.c_str()));
	ui.hs_path->setText(QDialog::trUtf8(ideGetPath(zx->ide,IDE_SLAVE).c_str()));
	ui.hs_islba->setChecked(ideGet(zx->ide,IDE_SLAVE,IDE_FLAG) & ATA_LBA);
	ui.hs_gsec->setValue(pass.spt);
	ui.hs_ghd->setValue(pass.hds);
	ui.hs_gcyl->setValue(pass.cyls);
	ui.hs_glba->setValue(ideGet(zx->ide,IDE_SLAVE,IDE_MAXLBA));
// tape
	ui.cbTapeAuto->setChecked(optGetFlag(OF_TAPEAUTO));
	ui.cbTapeFast->setChecked(optGetFlag(OF_TAPEFAST));
	ui.tpathle->setText(QDialog::trUtf8(tapGetPath(zx->tape).c_str()));
	buildtapelist();
// tools
	ui.sjpathle->setText(QDialog::trUtf8(optGetString(OPT_ASMPATH).c_str()));
	ui.prjdirle->setText(QDialog::trUtf8(optGetString(OPT_PROJDIR).c_str()));
	buildmenulist();
// leds
	ui.diskLed->setChecked(emulGetFlags() & FL_LED_DISK);
	ui.shotLed->setChecked(emulGetFlags() & FL_LED_SHOT);

	show();
}

void SetupWin::apply() {
// machine
	HardWare *oldmac = zx->hw;
	zx->opt.hwName = std::string(ui.machbox->currentText().toUtf8().data()); setHardware(zx,zx->opt.hwName);
	zx->opt.rsName = std::string(ui.rsetbox->currentText().toUtf8().data()); setRomset(zx, zx->opt.rsName);
	emulSetFlag(FL_RESET, ui.reschk->isChecked());
	zx->resbank = ui.resbox->currentIndex();
	switch(ui.mszbox->currentIndex()) {
		case 0: memSet(zx->mem,MEM_MEMSIZE,48); break;
		case 1: memSet(zx->mem,MEM_MEMSIZE,128); break;
		case 2: memSet(zx->mem,MEM_MEMSIZE,256); break;
		case 3: memSet(zx->mem,MEM_MEMSIZE,512); break;
		case 4: memSet(zx->mem,MEM_MEMSIZE,1024); break;
	}
	zxSetFrq(zx,ui.cpufrq->value() / 2.0);
	setFlagBit(ui.scrpwait->isChecked(),&zx->hwFlags,WAIT_ON);
	zx->opt.GSRom = GSRom;
	setRomsetList(rsl);
	if (zx->hw != oldmac) zxReset(zx,RES_DEFAULT);
// video
	setFlagBit(ui.dszchk->isChecked(),&zx->vid->flags,VF_DOUBLE);
	zx->vid->brdsize = ui.bszsld->value()/100.0;
	optSet(OPT_SHOTDIR,std::string(ui.pathle->text().toUtf8().data()));
	optSet(OPT_SHOTFRM,ui.ssfbox->itemData(ui.ssfbox->currentIndex()).toInt());
	optSet(OPT_SHOTCNT,ui.scntbox->value());
	optSet(OPT_SHOTINT,ui.sintbox->value());
	vidSetLayout(zx->vid,std::string(ui.geombox->currentText().toUtf8().data()));
	optSet(OPT_BRGLEV,ui.brgslide->value());
// sound
	std::string oname = sndGetOutputName();
	std::string nname(ui.outbox->currentText().toUtf8().data());
	int orate = sndGet(SND_RATE);
	sndSet(SND_ENABLE, ui.senbox->isChecked());
	sndSet(SND_MUTE, ui.mutbox->isChecked());
	sndSet(SND_RATE, ui.ratbox->currentText().toInt());
	sndSet(SND_BEEP, ui.bvsld->value());
	sndSet(SND_TAPE, ui.tvsld->value());
	sndSet(SND_AYVL, ui.avsld->value());
	sndSet(SND_GSVL, ui.gvsld->value());
	if ((oname != nname) || (orate != sndGet(SND_RATE))) setOutput(nname);
	tsSet(zx->ts,AY_TYPE,0,ui.schip1box->itemData(ui.schip1box->currentIndex()).toInt());
	tsSet(zx->ts,AY_TYPE,1,ui.schip2box->itemData(ui.schip2box->currentIndex()).toInt());
	tsSet(zx->ts,AY_STEREO,0,ui.stereo1box->itemData(ui.stereo1box->currentIndex()).toInt());
	tsSet(zx->ts,AY_STEREO,1,ui.stereo2box->itemData(ui.stereo2box->currentIndex()).toInt());
	tsSet(zx->ts,TS_TYPE,0,ui.tsbox->itemData(ui.tsbox->currentIndex()).toInt());
	int gsf = 0;
	if (ui.gsgroup->isChecked()) gsf |= GS_ENABLE;
	if (ui.gsrbox->isChecked()) gsf |= GS_RESET;
	gsSet(zx->gs,GS_FLAG,gsf);
	gsSet(zx->gs,GS_STEREO,ui.gstereobox->itemData(ui.gstereobox->currentIndex()).toInt());
// input
	if (ui.inpDevice->currentIndex() < 1) {
		optSet(OPT_JOYNAME,std::string(""));
	} else {
		optSet(OPT_JOYNAME,std::string(ui.inpDevice->currentText().toUtf8().data()));
	}
	std::string kmname = getRFText(ui.keyMapBox);
	if (kmname == "none") kmname = "default";
	optSet(OPT_KEYNAME,kmname);
	loadKeys();
// bdi
	bdiSetFlag(zx->bdi,BDI_ENABLE,ui.bdebox->isChecked());
	bdiSetFlag(zx->bdi,BDI_TURBO,ui.bdtbox->isChecked());

	Floppy* flp = bdiGetFloppy(zx->bdi,0);
	flpSetFlag(flp,FLP_TRK80,ui.a80box->isChecked());
	flpSetFlag(flp,FLP_DS,ui.adsbox->isChecked());
	flpSetFlag(flp,FLP_PROTECT,ui.awpbox->isChecked());

	flp = bdiGetFloppy(zx->bdi,1);
	flpSetFlag(flp,FLP_TRK80,ui.b80box->isChecked());
	flpSetFlag(flp,FLP_DS,ui.bdsbox->isChecked());
	flpSetFlag(flp,FLP_PROTECT,ui.bwpbox->isChecked());

	flp = bdiGetFloppy(zx->bdi,2);
	flpSetFlag(flp,FLP_TRK80,ui.c80box->isChecked());
	flpSetFlag(flp,FLP_DS,ui.cdsbox->isChecked());
	flpSetFlag(flp,FLP_PROTECT,ui.cwpbox->isChecked());

	flp = bdiGetFloppy(zx->bdi,3);
	flpSetFlag(flp,FLP_TRK80,ui.d80box->isChecked());
	flpSetFlag(flp,FLP_DS,ui.ddsbox->isChecked());
	flpSetFlag(flp,FLP_PROTECT,ui.dwpbox->isChecked());

// hdd
	ideSet(zx->ide,IDE_NONE,IDE_TYPE,ui.hiface->itemData(ui.hiface->currentIndex()).toInt());

	int flg = ideGet(zx->ide,IDE_MASTER,IDE_FLAG);
	ATAPassport pass = ideGetPassport(zx->ide,IDE_MASTER);
	ideSet(zx->ide,IDE_MASTER,IDE_TYPE,ui.hm_type->itemData(ui.hm_type->currentIndex()).toInt());
	pass.model = std::string(ui.hm_model->text().toUtf8().data(),40);
	pass.serial = std::string(ui.hm_ser->text().toUtf8().data(),20);
	ideSetPath(zx->ide,IDE_MASTER,std::string(ui.hm_path->text().toUtf8().data()));
	setFlagBit(ui.hm_islba->isChecked(),&flg,ATA_LBA);
	ideSet(zx->ide,IDE_MASTER,IDE_FLAG,flg);
	pass.spt = ui.hm_gsec->value();
	pass.hds = ui.hm_ghd->value();
	pass.cyls = ui.hm_gcyl->value();
	ideSet(zx->ide,IDE_MASTER,IDE_MAXLBA,ui.hm_glba->value());
	ideSetPassport(zx->ide,IDE_MASTER,pass);

	pass = ideGetPassport(zx->ide,IDE_SLAVE);
	flg = ideGet(zx->ide,IDE_SLAVE,IDE_FLAG);
	ideSet(zx->ide,IDE_SLAVE,IDE_TYPE,ui.hs_type->itemData(ui.hs_type->currentIndex()).toInt());
	pass.model = std::string(ui.hs_model->text().toUtf8().data(),40);
	pass.serial = std::string(ui.hs_ser->text().toUtf8().data(),20);
	ideSetPath(zx->ide,IDE_SLAVE,std::string(ui.hs_path->text().toUtf8().data()));
	setFlagBit(ui.hs_islba->isChecked(),&flg,ATA_LBA);
	ideSet(zx->ide,IDE_SLAVE,IDE_FLAG,flg);
	pass.spt = ui.hs_gsec->value();
	pass.hds = ui.hs_ghd->value();
	pass.cyls = ui.hs_gcyl->value();
	ideSet(zx->ide,IDE_SLAVE,IDE_MAXLBA,ui.hs_glba->value());
	ideSetPassport(zx->ide,IDE_SLAVE,pass);
// tape
	optSetFlag(OF_TAPEAUTO,ui.cbTapeAuto->isChecked());
	optSetFlag(OF_TAPEFAST,ui.cbTapeFast->isChecked());
// tools
	optSet(OPT_ASMPATH,std::string(ui.sjpathle->text().toUtf8().data()));
	optSet(OPT_PROJDIR,std::string(ui.prjdirle->text().toUtf8().data()));
// leds
	emulSetFlag(FL_LED_DISK,ui.diskLed->isChecked());
	emulSetFlag(FL_LED_SHOT,ui.shotLed->isChecked());

	saveConfig();
	sndCalibrate();
	emulSetColor(ui.brgslide->value());
	emulOpenJoystick(optGetString(OPT_JOYNAME));
	emulUpdateWindow();
}

void SetupWin::reject() {
	hide();
	fillBookmarkMenu();
	emulPause(false,PR_OPTS);
}

void SetupWin::rmRomset() {
	int idx = ui.rsetbox->currentIndex();
	if (idx < 0) return;
	if (areSure("Do you really want to delete this romset?")) {
		rsl.erase(rsl.begin() + idx);
		ui.rsetbox->removeItem(idx);
	}
}

void SetupWin::addNewRomset() {
//	bool ok = false;
	QString nam = QInputDialog::getText(this,"Enter...","Input romset name"); //,QLineEdit::Normal,"",&ok);
	if (nam.isEmpty()) return;
	RomSet nrs;
	nrs.name = std::string(nam.toUtf8().data());
	uint i;
	for (i=0; i<8; i++) {
		nrs.roms[i].path = "";
		nrs.roms[i].part = 0;
	}
	for (i=0; i<rsl.size(); i++) {
		if (rsl[i].name == nrs.name) return;
	}
	rsl.push_back(nrs);
	ui.rsetbox->addItem(QDialog::trUtf8(nrs.name.c_str()));
	ui.rsetbox->setCurrentIndex(ui.rsetbox->count() - 1);
}

// machine

void SetupWin::editrset(QModelIndex idx) {
	int cbx = ui.rsetbox->currentIndex();
	if (cbx < 0) return;
	std::string rpth = optGetString(OPT_ROMDIR);
	QDir rdir(QString(rpth.c_str()));
	QStringList rlst = rdir.entryList(QStringList() << "*.rom",QDir::Files,QDir::Name);
	fillRFBox(ui.rse_singlefile,rlst);
	fillRFBox(ui.rse_file0,rlst);
	fillRFBox(ui.rse_file1,rlst);
	fillRFBox(ui.rse_file2,rlst);
	fillRFBox(ui.rse_file3,rlst);
	fillRFBox(ui.rse_gsfile,rlst);
	ui.rse_singlefile->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].file.c_str())) + 1);
	ui.rse_file0->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[0].path.c_str())) + 1);
	ui.rse_file1->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[1].path.c_str())) + 1);
	ui.rse_file2->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[2].path.c_str())) + 1);
	ui.rse_file3->setCurrentIndex(rlst.indexOf(QString(rsl[cbx].roms[3].path.c_str())) + 1);
	ui.rse_part0->setValue(rsl[cbx].roms[0].part);
	ui.rse_part1->setValue(rsl[cbx].roms[1].part);
	ui.rse_part2->setValue(rsl[cbx].roms[2].part);
	ui.rse_part3->setValue(rsl[cbx].roms[3].part);
	ui.rse_gsfile->setCurrentIndex(rlst.indexOf(QString(GSRom.c_str())) + 1);
	ui.rse_grp_single->setChecked(rsl[cbx].file != "");
	ui.rstab->hide();
	ui.rsetbox->setEnabled(false);
	ui.addrset->setEnabled(false);
	ui.rmrset->setEnabled(false);
	ui.rssel->show();
}

void SetupWin::setrpart() {
	int cbx = ui.rsetbox->currentIndex();
	GSRom = getRFText(ui.rse_gsfile);
	if (ui.rse_grp_single->isChecked()) {
		rsl[cbx].file = getRFText(ui.rse_singlefile);
	} else {
		rsl[cbx].file = "";
	}
	rsl[cbx].roms[0].path = getRFText(ui.rse_file0); rsl[cbx].roms[0].part = ui.rse_part0->value();
	rsl[cbx].roms[1].path = getRFText(ui.rse_file1); rsl[cbx].roms[1].part = ui.rse_part1->value();
	rsl[cbx].roms[2].path = getRFText(ui.rse_file2); rsl[cbx].roms[2].part = ui.rse_part2->value();
	rsl[cbx].roms[3].path = getRFText(ui.rse_file3); rsl[cbx].roms[3].part = ui.rse_part3->value();
	buildrsetlist();
	hidersedit();
}

void SetupWin::hidersedit() {
	ui.rssel->hide();
	ui.rstab->show();
	ui.rsetbox->setEnabled(true);
	ui.addrset->setEnabled(true);
	ui.rmrset->setEnabled(true);
}

// lists

void SetupWin::buildkeylist() {
	std::string wdir = optGetString(OPT_WORKDIR);
	QDir dir(wdir.c_str());
	QStringList lst = dir.entryList(QStringList() << "*.map",QDir::Files,QDir::Name);
	fillRFBox(ui.keyMapBox,lst);
}

void SetupWin::buildjmaplist() {
	std::vector<joyPair> jmap = getJMap();
	ui.bindTable->setRowCount(jmap.size());
	ui.bindTable->setColumnCount(2);
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
		ui.bindTable->setItem(i,1,it);
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
		ui.bindTable->setItem(i,0,it);
	}
}

void SetupWin::okbuts() {
	std::vector<HardWare> list = getHardwareList();
	int t = list[ui.machbox->currentIndex()].mask;

	if (t == 0x00) {
		ui.okbut->setEnabled(ui.mszbox->currentIndex()==0);
	} else {
		ui.okbut->setEnabled((1<<(ui.mszbox->currentIndex()-1)) & t);
	}
	ui.apbut->setEnabled(ui.okbut->isEnabled());
}

void SetupWin::setmszbox(int idx) {
	std::vector<HardWare> list = getHardwareList();
	int t = list[idx].mask;
	QIcon okicon = QIcon(":/images/ok-apply.png");
	QIcon ericon = QIcon(":/images/cancel.png");
	if (t == 0x00) {
		ui.mszbox->setItemIcon(0,okicon);
		ui.mszbox->setItemIcon(1,ericon);
		ui.mszbox->setItemIcon(2,ericon);
		ui.mszbox->setItemIcon(3,ericon);
		ui.mszbox->setItemIcon(4,ericon);
	} else {
		ui.mszbox->setItemIcon(0,ericon);
		ui.mszbox->setItemIcon(1,(t & 1)?okicon:ericon);
		ui.mszbox->setItemIcon(2,(t & 2)?okicon:ericon);
		ui.mszbox->setItemIcon(3,(t & 4)?okicon:ericon);
		ui.mszbox->setItemIcon(4,(t & 8)?okicon:ericon);
	}
	okbuts();
}

void SetupWin::buildrsetlist() {
//	int i;
	if (ui.rsetbox->currentIndex() < 0) {
		ui.rstab->setEnabled(false);
		return;
	}
	ui.rstab->setEnabled(true);
	RomSet rset = rsl[ui.rsetbox->currentIndex()];
	if (rset.file == "") {
		ui.rstab->hideRow(4);
		for (int i=0; i<4; i++) {
			ui.rstab->showRow(i);
			QString rsf = QDialog::trUtf8(rset.roms[i].path.c_str());
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
		ui.rstab->item(4,1)->setText(QDialog::trUtf8(rset.file.c_str()));
		ui.rstab->item(4,2)->setText("");
	}
	ui.rstab->item(5,1)->setText(QDialog::trUtf8(GSRom.c_str()));
	ui.rstab->setColumnWidth(0,100);
	ui.rstab->setColumnWidth(1,300);
	ui.rstab->setColumnWidth(2,50);
}

void SetupWin::buildtapelist() {
	buildTapeList();
	std::vector<TapeBlockInfo> inf = tapGetBlocksInfo(zx->tape);
	ui.tapelist->setRowCount(inf.size());
	if (inf.size() == 0) {
		ui.tapelist->setEnabled(false);
		return;
	}
	ui.tapelist->setEnabled(true);
	QTableWidgetItem* itm;
	uint tm,ts;
	for (int i=0; i < (int)inf.size(); i++) {
		if (tapGet(zx->tape,TAPE_BLOCK) == i) {
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
		if (inf[i].flags & TBF_BREAK) itm->setIcon(QIcon(":/images/cancel.png"));
		ui.tapelist->setItem(i,1,itm);
		ts = inf[i].time;
		tm = ts/60;
		ts -= tm * 60;
		itm = new QTableWidgetItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
		ui.tapelist->setItem(i,2,itm);
		itm = new QTableWidgetItem(QString::number(inf[i].size));
		ui.tapelist->setItem(i,4,itm);
		itm = new QTableWidgetItem(QDialog::trUtf8(inf[i].name.c_str()));
		ui.tapelist->setItem(i,5,itm);
	}
	ui.tapelist->selectRow(0);
}

void SetupWin::buildmenulist() {
	std::vector<XBookmark> bml = getBookmarkList();
	ui.umlist->setRowCount(bml.size());
	QTableWidgetItem* itm;
	for (uint i=0; i<bml.size(); i++) {
		itm = new QTableWidgetItem(QString(bml[i].name.c_str()));
		ui.umlist->setItem(i,0,itm);
		itm = new QTableWidgetItem(QString(bml[i].path.c_str()));
		ui.umlist->setItem(i,1,itm);
	}
	ui.umlist->setColumnWidth(0,100);
	ui.umlist->selectRow(0);
};

void SetupWin::copyToTape() {
	int dsk = ui.disktabs->currentIndex();
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	std::vector<TRFile> cat = flpGetTRCatalog(bdiGetFloppy(zx->bdi,dsk));
	int row;
	uint8_t* buf = new uint8_t[0xffff];
	uint16_t line,start,len;
	std::string name;
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		row = idx[i].row();
		if (flpGetSectorsData(bdiGetFloppy(zx->bdi,dsk),cat[row].trk, cat[row].sec+1, buf, cat[row].slen)) {
			if (cat[row].slen == (cat[row].hlen + ((cat[row].llen == 0) ? 0 : 1))) {
				start = (cat[row].hst << 8) + cat[row].lst;
				len = (cat[row].hlen << 8) + cat[row].llen;
				line = (cat[row].ext == 'B') ? (buf[start] + (buf[start+1] << 8)) : 0x8000;
				name = std::string((char*)&cat[row].name[0],8) + std::string(".") + std::string((char*)&cat[row].ext,1);
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
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...",QDir::homePath());
	if (dir == "") return;
	std::string sdir = std::string(dir.toUtf8().data()) + std::string(SLASH);
	Floppy* flp = bdiGetFloppy(zx->bdi,ui.disktabs->currentIndex());		// selected floppy
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveHobetaFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = int2str(savedFiles) + std::string(" of ") + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

void SetupWin::diskToRaw() {
	QModelIndexList idx = ui.disklist->selectionModel()->selectedRows();
	if (idx.size() == 0) return;
	QString dir = QFileDialog::getExistingDirectory(this,"Save file(s) to...",QDir::homePath());
	if (dir == "") return;
	std::string sdir = std::string(dir.toUtf8().data()) + std::string(SLASH);
	Floppy* flp = bdiGetFloppy(zx->bdi,ui.disktabs->currentIndex());
	int savedFiles = 0;
	for (int i=0; i<idx.size(); i++) {
		if (saveRawFile(flp,idx[i].row(),sdir.c_str()) == ERR_OK) savedFiles++;
	}
	std::string msg = int2str(savedFiles) + std::string(" of ") + int2str(idx.size()) + " files saved";
	showInfo(msg.c_str());
}

TRFile getHeadInfo(int blk) {
	TRFile res;
	std::vector<uint8_t> dt = tapGetBlockData(zx->tape,blk);
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
	if (~tapGet(zx->tape,blk,TAPE_BFLAG) & TBF_BYTES) {
		shitHappens("This is not standard block");
		return;
	}
	if (tapGet(zx->tape,blk,TAPE_BFLAG) & TBF_HEAD) {
		if (tapGet(zx->tape,TAPE_BLOCKS) == blk + 1) {
			shitHappens("Header without data? Hmm...");
		} else {
			if (~tapGet(zx->tape,blk + 1,TAPE_BFLAG) & TBF_BYTES) {
				shitHappens("Data block is not standard");
			} else {
				headBlock = blk;
				dataBlock = blk + 1;
			}
		}
	} else {
		dataBlock = blk;
		if (blk != 0) {
			if (tapGet(zx->tape,blk - 1,TAPE_BFLAG) & TBF_HEAD) {
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
	} else {
		dsc = getHeadInfo(headBlock);
		if (dsc.ext == 0x00) {
			shitHappens("Yes, it happens");
			return;
		}
	}
	if (!flpGetFlag(bdiGetFloppy(zx->bdi,dsk),FLP_INSERT)) newdisk(dsk);
	std::vector<uint8_t> dt = tapGetBlockData(zx->tape,dataBlock);
	uint8_t* buf = new uint8_t[256];
	uint pos = 1;	// skip block type mark
	switch(flpCreateFile(bdiGetFloppy(zx->bdi,dsk),&dsc)) {
		case ERR_SHIT: shitHappens("Yes, it happens"); break;
		case ERR_MANYFILES: shitHappens("Too many files @ disk"); break;
		case ERR_NOSPACE: shitHappens("Not enough space @ disk"); break;
		case ERR_OK:
			while (pos < dt.size()) {
				do {
					buf[(pos-1) & 0xff] = (pos < dt.size()) ? dt[pos] : 0x00;
					pos++;
				} while ((pos & 0xff) != 1);
				flpPutSectorData(bdiGetFloppy(zx->bdi,dsk),dsc.trk, dsc.sec+1, buf, 256);
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
	if (!flpGetFlag(bdiGetFloppy(zx->bdi,dsk),FLP_INSERT)) {
		wid->setEnabled(false);
		wid->setRowCount(0);
	} else {
		wid->setEnabled(true);
		if (flpGet(bdiGetFloppy(zx->bdi,dsk),FLP_DISKTYPE) == DISK_TYPE_TRD) {
			std::vector<TRFile> cat = flpGetTRCatalog(bdiGetFloppy(zx->bdi,dsk));
			wid->setRowCount(cat.size());
			for (uint i=0; i<cat.size(); i++) {
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
	double f = ui.cpufrq->value() / 2.0;
	ui.cpufrqlab->setText(QString::number(f,'f',2).append(" MHz"));
}

// video

void SetupWin::chabsz() {ui.bszlab->setText(QString::number(ui.bszsld->value()).append("%"));}
void SetupWin::chabrg() {ui.brglab->setText(QString::number(ui.brgslide->value()));}

void SetupWin::selsspath() {
	QString fpath = QFileDialog::getExistingDirectory(this,"Screenshots folder",QDialog::trUtf8(optGetString(OPT_SHOTDIR).c_str()),QFileDialog::ShowDirsOnly);
	if (fpath!="") ui.pathle->setText(fpath);
}

// sound

void SetupWin::updvolumes() {
	ui.bvlab->setText(QString::number(ui.bvsld->value()));
	ui.tvlab->setText(QString::number(ui.tvsld->value()));
	ui.avlab->setText(QString::number(ui.avsld->value()));
	ui.gslab->setText(QString::number(ui.gvsld->value()));
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
					intb.name = box->itemData(box->currentIndex()).toString().toUtf8().data();
					optSetJMap(extb,intb);
					doWork = false;
					break;
				case SDL_JOYAXISMOTION:
					if ((ev.jaxis.value < -5000) || (ev.jaxis.value > 5000)) {
						extb.type = XJ_AXIS;
						extb.num = ev.jaxis.axis;
						extb.dir = (ev.jaxis.value > 0);
						intb.dev = XJ_JOY;
						intb.name = box->itemData(box->currentIndex()).toString().toUtf8().data();
						optSetJMap(extb,intb);
						doWork = false;
					}
					break;
			}
		}
	} while (doWork);
	dia->hide();
	buildjmaplist();
}

void SetupWin::delJoyBind() {
	int row = ui.bindTable->currentRow();
	if (row < 0) return;
	if (ui.bindTable->isRowHidden(row)) return;
	QVector3D vec = ui.bindTable->item(row,0)->data(Qt::UserRole).value<QVector3D>();
	extButton extb;
	extb.type = vec.x();
	extb.num = vec.y();
	extb.dir = (vec.z() != 0);
	optDelJMap(extb);
	buildjmaplist();
}

// disk

void SetupWin::newdisk(int idx) {
	Floppy *flp = bdiGetFloppy(zx->bdi,idx);
	if (!saveChangedDisk(idx & 3)) return;
	flpFormat(flp);
	flpSetPath(flp,"");
	flpSetFlag(flp,FLP_INSERT | FLP_CHANGED,true);
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

void SetupWin::savea() {Floppy* flp = bdiGetFloppy(zx->bdi,0); if (flpGetFlag(flp,FLP_INSERT)) saveFile(flpGetPath(flp).c_str(),FT_DISK,0);}
void SetupWin::saveb() {Floppy* flp = bdiGetFloppy(zx->bdi,1); if (flpGetFlag(flp,FLP_INSERT)) saveFile(flpGetPath(flp).c_str(),FT_DISK,1);}
void SetupWin::savec() {Floppy* flp = bdiGetFloppy(zx->bdi,2); if (flpGetFlag(flp,FLP_INSERT)) saveFile(flpGetPath(flp).c_str(),FT_DISK,2);}
void SetupWin::saved() {Floppy* flp = bdiGetFloppy(zx->bdi,3); if (flpGetFlag(flp,FLP_INSERT)) saveFile(flpGetPath(flp).c_str(),FT_DISK,3);}

void SetupWin::ejcta() {saveChangedDisk(0); flpEject(bdiGetFloppy(zx->bdi,0)); updatedisknams();}
void SetupWin::ejctb() {saveChangedDisk(1); flpEject(bdiGetFloppy(zx->bdi,1)); updatedisknams();}
void SetupWin::ejctc() {saveChangedDisk(2); flpEject(bdiGetFloppy(zx->bdi,2)); updatedisknams();}
void SetupWin::ejctd() {saveChangedDisk(3); flpEject(bdiGetFloppy(zx->bdi,3)); updatedisknams();}

void SetupWin::updatedisknams() {
	ui.apathle->setText(QDialog::trUtf8(flpGetPath(bdiGetFloppy(zx->bdi,0)).c_str()));
	ui.bpathle->setText(QDialog::trUtf8(flpGetPath(bdiGetFloppy(zx->bdi,1)).c_str()));
	ui.cpathle->setText(QDialog::trUtf8(flpGetPath(bdiGetFloppy(zx->bdi,2)).c_str()));
	ui.dpathle->setText(QDialog::trUtf8(flpGetPath(bdiGetFloppy(zx->bdi,3)).c_str()));
	fillDiskCat();
}

// tape

void SetupWin::loatape() {
	loadFile("",FT_TAPE,1);
	ui.tpathle->setText(QDialog::trUtf8(tapGetPath(zx->tape).c_str()));
	buildtapelist();
}

void SetupWin::savtape() {
	if (tapGet(zx->tape,TAPE_BLOCKS) != 0) saveFile(tapGetPath(zx->tape).c_str(),FT_TAP,-1);
}

void SetupWin::ejctape() {
	tapEject(zx->tape);
	ui.tpathle->setText(QDialog::trUtf8(tapGetPath(zx->tape).c_str()));
	buildtapelist();
}

void SetupWin::tblkup() {
	int ps = ui.tapelist->currentIndex().row();
	if (ps > 0) {
		tapSwapBlocks(zx->tape,ps,ps-1);
		buildtapelist();
		ui.tapelist->selectRow(ps-1);
	}
}

void SetupWin::tblkdn() {
	int ps = ui.tapelist->currentIndex().row();
	if ((ps != -1) && (ps < tapGet(zx->tape,TAPE_BLOCKS) - 1)) {
		tapSwapBlocks(zx->tape,ps,ps+1);
		buildtapelist();
		ui.tapelist->selectRow(ps+1);
	}
}

void SetupWin::tblkrm() {
	int ps = ui.tapelist->currentIndex().row();
	if (ps != -1) {
		tapDelBlock(zx->tape,ps);
		buildtapelist();
		ui.tapelist->selectRow(ps);
	}
}

void SetupWin::chablock(QModelIndex idx) {
	int row = idx.row();
	tapRewind(zx->tape,row);
	buildtapelist();
	ui.tapelist->selectRow(row);
}

void SetupWin::setTapeBreak(int row,int col) {
	if ((row < 0) || (col != 1)) return;
	int flg = tapGet(zx->tape,row,TAPE_BFLAG);
	flg ^= TBF_BREAK;
	tapSet(zx->tape,row,TAPE_BFLAG,flg);
	buildtapelist();
	ui.tapelist->selectRow(row);
}

// hdd

void SetupWin::hddMasterImg() {
	QString path = QFileDialog::getSaveFileName(this,"Image for master HDD",QDir::homePath(),"All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") ui.hm_path->setText(path);
}

void SetupWin::hddSlaveImg() {
	QString path = QFileDialog::getSaveFileName(this,"Image for slave HDD",QDir::homePath(),"All files (*.*)",NULL,QFileDialog::DontConfirmOverwrite);
	if (path != "") ui.hs_path->setText(path);
}

void SetupWin::hddcap() {
	uint32_t sz;
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

// tools

void SetupWin::ssjapath() {
	QString fnam = QFileDialog::getOpenFileName(NULL,"Select SJAsm executable",QDir::homePath(),"All files (*)");
	if (fnam!="") ui.sjpathle->setText(fnam);
}

void SetupWin::sprjpath() {
	QString fnam = QFileDialog::getExistingDirectory(this,"Projects file",QDialog::trUtf8(optGetString(OPT_PROJDIR).c_str()),QFileDialog::ShowDirsOnly);
	if (fnam!="") ui.prjdirle->setText(fnam);
}

void SetupWin::umup() {
	int ps = ui.umlist->currentRow();
	if (ps>0) {
		swapBookmarks(ps,ps-1);
		buildmenulist();
		ui.umlist->selectRow(ps-1);
	}
}

void SetupWin::umdn() {
	int ps = ui.umlist->currentIndex().row();
	if ((ps!=-1) && (ps < getBookmarksCount()-1)) {
		swapBookmarks(ps,ps+1);
		buildmenulist();
		ui.umlist->selectRow(ps+1);
	}
}

void SetupWin::umdel() {
	int ps = ui.umlist->currentIndex().row();
	if (ps!=-1) {
		delBookmark(ps);
		buildmenulist();
		if (ps == getBookmarksCount()) {
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
		addBookmark(std::string(uia.namele->text().toUtf8().data()),std::string(uia.pathle->text().toUtf8().data()));
	} else {
		setBookmark(umidx,std::string(uia.namele->text().toUtf8().data()),std::string(uia.pathle->text().toUtf8().data()));
	}
	umadial->hide();
	buildmenulist();
	ui.umlist->selectRow(ui.umlist->rowCount()-1);
}
