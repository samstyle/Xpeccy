#include <QStandardItemModel>
#include <QFileDialog>
#include <QDebug>

#include "common.h"
#include "sound.h"
#include "spectrum.h"
#include "setupwin.h"
#include "emulwin.h"
#include "settings.h"
#include "filer.h"

#include "ui_selname.h"

extern ZXComp* zx;

Ui::IName nameui;

SetupWin* optWin;
QDialog* optName;

std::vector<RomSet> rsl;
std::string GSRom;

void optInit(QWidget* par) {
	optName = new QDialog((QWidget*)optWin);
	nameui.setupUi(optName);
	optName->setModal(true);
	optWin = new SetupWin(par);
}

void optShow() {
	optWin->start();
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
	QObject::connect(ui.addrset,SIGNAL(released()),optName,SLOT(show()));
	QObject::connect(ui.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
	QObject::connect(nameui.okbut,SIGNAL(released()),this,SLOT(addNewRomset()));
	QObject::connect(nameui.cnbut,SIGNAL(released()),optName,SLOT(hide()));
// video
	QObject::connect(ui.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	QObject::connect(ui.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));
	QObject::connect(ui.brgslide,SIGNAL(valueChanged(int)),this,SLOT(chabrg()));
// sound
	QObject::connect(ui.bvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(ui.tvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(ui.avsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
	QObject::connect(ui.gvsld,SIGNAL(valueChanged(int)),this,SLOT(updvolumes()));
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
	ui.resbox->setCurrentIndex(zx->res);
	switch(memGet(zx->mem,MEM_MEMSIZE)) {
		case 48: ui.mszbox->setCurrentIndex(0); break;
		case 128: ui.mszbox->setCurrentIndex(1); break;
		case 256: ui.mszbox->setCurrentIndex(2); break;
		case 512: ui.mszbox->setCurrentIndex(3); break;
		case 1024: ui.mszbox->setCurrentIndex(4); break;
	}
	ui.cpufrq->setValue(zx->cpuFreq * 2); updfrq();
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
	ui.gsrbox->setChecked(gsGetFlag(zx->gs) & GS_RESET);
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
	ui.gstereobox->setCurrentIndex(ui.gstereobox->findData(QVariant(gsGetParam(zx->gs,GS_STEREO))));
	ui.gsgroup->setChecked(gsGetFlag(zx->gs) & GS_ENABLE);
	ui.tsbox->setCurrentIndex(ui.tsbox->findData(QVariant(tsGet(zx->ts,TS_TYPE,0))));
// dos
	ui.bdebox->setChecked(zx->bdi->enable);
	ui.bdtbox->setChecked(zx->bdi->vg93.turbo);
	ui.apathle->setText(QDialog::trUtf8(zx->bdi->flop[0].path.c_str()));
		ui.a80box->setChecked(zx->bdi->flop[0].trk80);
		ui.adsbox->setChecked(zx->bdi->flop[0].dblsid);
		ui.awpbox->setChecked(zx->bdi->flop[0].protect);
	ui.bpathle->setText(QDialog::trUtf8(zx->bdi->flop[1].path.c_str()));
		ui.b80box->setChecked(zx->bdi->flop[1].trk80);
		ui.bdsbox->setChecked(zx->bdi->flop[1].dblsid);
		ui.bwpbox->setChecked(zx->bdi->flop[1].protect);
	ui.cpathle->setText(QDialog::trUtf8(zx->bdi->flop[2].path.c_str()));
		ui.c80box->setChecked(zx->bdi->flop[2].trk80);
		ui.cdsbox->setChecked(zx->bdi->flop[2].dblsid);
		ui.cwpbox->setChecked(zx->bdi->flop[2].protect);
	ui.dpathle->setText(QDialog::trUtf8(zx->bdi->flop[3].path.c_str()));
		ui.d80box->setChecked(zx->bdi->flop[3].trk80);
		ui.ddsbox->setChecked(zx->bdi->flop[3].dblsid);
		ui.dwpbox->setChecked(zx->bdi->flop[3].protect);
	fillDiskCat();
// hdd
	ui.hiface->setCurrentIndex(ui.hiface->findData(zx->ide->iface));
	
	ui.hm_type->setCurrentIndex(ui.hm_type->findData(QVariant(zx->ide->master.iface)));
	ui.hm_model->setText(QDialog::trUtf8(zx->ide->master.pass.model.c_str()));
	ui.hm_ser->setText(QDialog::trUtf8(zx->ide->master.pass.serial.c_str()));
	ui.hm_path->setText(QDialog::trUtf8(zx->ide->master.image.c_str()));
	ui.hm_islba->setChecked(zx->ide->master.flags & ATA_LBA);
	ui.hm_gsec->setValue(zx->ide->master.pass.spt);
	ui.hm_ghd->setValue(zx->ide->master.pass.hds);
	ui.hm_gcyl->setValue(zx->ide->master.pass.cyls);
	ui.hm_glba->setValue(zx->ide->master.maxlba);

	ui.hs_type->setCurrentIndex(ui.hm_type->findData(QVariant(zx->ide->slave.iface)));
	ui.hs_model->setText(QDialog::trUtf8(zx->ide->slave.pass.model.c_str()));
	ui.hs_ser->setText(QDialog::trUtf8(zx->ide->slave.pass.serial.c_str()));
	ui.hs_path->setText(QDialog::trUtf8(zx->ide->slave.image.c_str()));
	ui.hs_islba->setChecked(zx->ide->slave.flags & ATA_LBA);
	ui.hs_gsec->setValue(zx->ide->slave.pass.spt);
	ui.hs_ghd->setValue(zx->ide->slave.pass.hds);
	ui.hs_gcyl->setValue(zx->ide->slave.pass.cyls);
	ui.hs_glba->setValue(zx->ide->slave.maxlba);
// tape
	ui.tpathle->setText(QDialog::trUtf8(zx->tape->path.c_str()));
	buildtapelist();
// tools
	ui.sjpathle->setText(QDialog::trUtf8(optGetString(OPT_ASMPATH).c_str()));
	ui.prjdirle->setText(QDialog::trUtf8(optGetString(OPT_PROJDIR).c_str()));
	buildmenulist();

	show();
}

void SetupWin::apply() {
//	emulSetFlag(true,FL_BLOCK);
// machine
	HardWare *oldmac = zx->hw;
	zx->opt.hwName = std::string(ui.machbox->currentText().toUtf8().data()); setHardware(zx,zx->opt.hwName);
	zx->opt.rsName = std::string(ui.rsetbox->currentText().toUtf8().data()); setRomset(zx, zx->opt.rsName);
//	zx->mem->loadromset(optGetString(OPT_ROMDIR));
	emulSetFlag(FL_RESET, ui.reschk->isChecked());
	zx->res = ui.resbox->currentIndex();
	switch(ui.mszbox->currentIndex()) {
		case 0: memSet(zx->mem,MEM_MEMSIZE,48); break;
		case 1: memSet(zx->mem,MEM_MEMSIZE,128); break;
		case 2: memSet(zx->mem,MEM_MEMSIZE,256); break;
		case 3: memSet(zx->mem,MEM_MEMSIZE,512); break;
		case 4: memSet(zx->mem,MEM_MEMSIZE,1024); break;
	}
	zx->cpuFreq = ui.cpufrq->value() / 2.0;
	setFlagBit(ui.scrpwait->isChecked(),&zx->hwFlags,WAIT_ON);
	zx->opt.GSRom = GSRom;
	setRomsetList(rsl);
	if (zx->hw != oldmac) zx->reset(RES_DEFAULT);
// video
	setFlagBit(ui.dszchk->isChecked(),&zx->vid->flags,VF_DOUBLE);
	zx->vid->brdsize = ui.bszsld->value()/100.0;
	optSet(OPT_SHOTDIR,std::string(ui.pathle->text().toUtf8().data()));
	optSet(OPT_SHOTFRM,ui.ssfbox->itemData(ui.ssfbox->currentIndex()).toInt());
	optSet(OPT_SHOTCNT,ui.scntbox->value());
	optSet(OPT_SHOTINT,ui.sintbox->value());
	zx->vid->setLayout(std::string(ui.geombox->currentText().toUtf8().data()));
	optSet(OPT_BRGLEV,ui.brgslide->value());
//	emulUpdateWindow();
// sound
	std::string oname = sndGetOutputName();
	std::string nname(ui.outbox->currentText().toUtf8().data());
	int orate = sndGet(SND_RATE);
	sndSet(SND_ENABLE, ui.senbox->isChecked());
	sndSet(SND_MUTE, ui.mutbox->isChecked());
	int gsf = 0;
	if (ui.gsgroup->isChecked()) gsf |= GS_ENABLE;
	if (ui.gsrbox->isChecked()) gsf |= GS_RESET;
	gsSetFlag(zx->gs,gsf);
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
	gsSetParam(zx->gs,GS_STEREO,ui.gstereobox->itemData(ui.gstereobox->currentIndex()).toInt());
// bdi
	zx->bdi->enable = ui.bdebox->isChecked();
	zx->bdi->vg93.turbo = ui.bdtbox->isChecked();
	zx->bdi->flop[0].trk80 = ui.a80box->isChecked();
		zx->bdi->flop[0].dblsid = ui.adsbox->isChecked();
		zx->bdi->flop[0].protect = ui.awpbox->isChecked();
	zx->bdi->flop[1].trk80 = ui.b80box->isChecked();
		zx->bdi->flop[1].dblsid = ui.bdsbox->isChecked();
		zx->bdi->flop[1].protect = ui.bwpbox->isChecked();
	zx->bdi->flop[2].trk80 = ui.c80box->isChecked();
		zx->bdi->flop[2].dblsid = ui.cdsbox->isChecked();
		zx->bdi->flop[2].protect = ui.cwpbox->isChecked();
	zx->bdi->flop[3].trk80 = ui.d80box->isChecked();
		zx->bdi->flop[3].dblsid = ui.ddsbox->isChecked();
		zx->bdi->flop[3].protect = ui.dwpbox->isChecked();
// hdd
	zx->ide->iface = ui.hiface->itemData(ui.hiface->currentIndex()).toInt();

	zx->ide->master.iface = ui.hm_type->itemData(ui.hm_type->currentIndex()).toInt();
	zx->ide->master.pass.model = std::string(ui.hm_model->text().toUtf8().data(),40);
	zx->ide->master.pass.serial = std::string(ui.hm_ser->text().toUtf8().data(),20);
	zx->ide->master.image = std::string(ui.hm_path->text().toUtf8().data());
	setFlagBit(ui.hm_islba->isChecked(),&zx->ide->master.flags,ATA_LBA);
	zx->ide->master.pass.spt = ui.hm_gsec->value();
	zx->ide->master.pass.hds = ui.hm_ghd->value();
	zx->ide->master.pass.cyls = ui.hm_gcyl->value();
	zx->ide->master.maxlba = ui.hm_glba->value();

	zx->ide->slave.iface = ui.hs_type->itemData(ui.hs_type->currentIndex()).toInt();
	zx->ide->slave.pass.model = std::string(ui.hs_model->text().toUtf8().data(),40);
	zx->ide->slave.pass.serial = std::string(ui.hs_ser->text().toUtf8().data(),20);
	zx->ide->slave.image = std::string(ui.hs_path->text().toUtf8().data());
	setFlagBit(ui.hs_islba->isChecked(),&zx->ide->slave.flags,ATA_LBA);
	zx->ide->slave.pass.spt = ui.hs_gsec->value();
	zx->ide->slave.pass.hds = ui.hs_ghd->value();
	zx->ide->slave.pass.cyls = ui.hs_gcyl->value();
	zx->ide->slave.maxlba = ui.hs_glba->value();
	zx->ide->refresh();

// tools
	optSet(OPT_ASMPATH,std::string(ui.sjpathle->text().toUtf8().data()));
	optSet(OPT_PROJDIR,std::string(ui.prjdirle->text().toUtf8().data()));

	saveConfig();
	sndCalibrate();
	emulSetColor(ui.brgslide->value());
	emulUpdateWindow();
//	emulSetFlag(false,FL_BLOCK);
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
	QString nam = nameui.namele->text();
	if (nam == "") return;
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
	optName->hide();
}

// machine

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
	std::vector<TapeBlockInfo> inf = zx->tape->getInfo();
	ui.tapelist->setRowCount(inf.size());
	if (inf.size() == 0) {
		ui.tapelist->setEnabled(false);
		return;
	}
	ui.tapelist->setEnabled(true);
	QTableWidgetItem* itm;
	uint tm,ts;
	for (uint i=0; i<inf.size(); i++) {
		if (zx->tape->block == i) {
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
	std::vector<TRFile> cat = zx->bdi->flop[dsk].getTRCatalog();
	int row;
	uint8_t* buf = new uint8_t[0xffff];
	uint16_t line,start,len;
	std::string name;
	for (int i=0; i<idx.size(); i++) {
		row = idx[i].row();
		if (zx->bdi->flop[dsk].getSectorsData(cat[row].trk, cat[row].sec+1, buf, cat[row].slen)) {
			if (cat[row].slen == (cat[row].hlen + ((cat[row].llen == 0) ? 0 : 1))) {
				start = (cat[row].hst << 8) + cat[row].lst;
				len = (cat[row].hlen << 8) + cat[row].llen;
				line = (cat[row].ext == 'B') ? (buf[start] + (buf[start+1] << 8)) : 0x8000;
				name = std::string((char*)&cat[row].name[0],8) + std::string(".") + std::string((char*)&cat[row].ext,1);
				zx->tape->addFile(name,(cat[row].ext == 'B') ? 0 : 3, start, len, line, buf,true);
				buildtapelist();
			} else {
				shithappens("File seems to be joined, skip");
			}
		} else {
			shithappens("Can't get file data, skip");
		}
		
	}
}

TRFile getHeadInfo(int blk) {
	TRFile res;
	std::vector<uint8_t> dt = zx->tape->getdata(blk,-1,-1);
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
	if (~zx->tape->data[blk].flags & TBF_BYTES) {
		shithappens("This is not standard block");
		return;
	}
	if (zx->tape->data[blk].flags & TBF_HEAD) {
		if ((int)zx->tape->data.size() == blk + 1) {
			shithappens("Header without data? Hmm...");
		} else {
			if (~zx->tape->data[blk+1].flags & TBF_BYTES) {
				shithappens("Data block is not standard");
			} else {
				headBlock = blk;
				dataBlock = blk + 1;
			}
		}
	} else {
		dataBlock = blk;
		if (blk != 0) {
			if (zx->tape->data[blk-1].flags & TBF_HEAD) {
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
		int len = zx->tape->getInfo()[dataBlock].size;
		if (len > 0xff00) {
			shithappens("Too much data for TRDos file");
			return;
		}
		dsc.llen = len & 0xff; dsc.hlen = ((len & 0xff00) >> 8);
	} else {
		dsc = getHeadInfo(headBlock);
		if (dsc.ext == 0x00) {
			shithappens("Yes, it happens");
			return;
		}
	}
	if (!zx->bdi->flop[dsk].insert) newdisk(dsk);
	std::vector<uint8_t> dt = zx->tape->getdata(dataBlock,-1,-1);
	uint8_t* buf = new uint8_t[256];
	uint pos = 1;	// skip block type mark
	switch(zx->bdi->flop[dsk].createFile(&dsc)) {
		case ERR_SHIT: shithappens("Yes, it happens"); break;
		case ERR_MANYFILES: shithappens("Too many files @ disk"); break;
		case ERR_NOSPACE: shithappens("Not enough space @ disk"); break;
		case ERR_OK:
			while (pos < dt.size()) {
				do {
					buf[(pos-1) & 0xff] = (pos < dt.size()) ? dt[pos] : 0x00;
					pos++;
				} while ((pos & 0xff) != 1);
				zx->bdi->flop[dsk].putSectorData(dsc.trk, dsc.sec+1, buf, 256);
				dsc.sec++;
				if (dsc.sec > 15) {
					dsc.sec = 0;
					dsc.trk++;
				}
			}
			fillDiskCat();
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
	if (!zx->bdi->flop[dsk].insert) {
		wid->setEnabled(false);
		wid->setRowCount(0);
	} else {
		wid->setEnabled(true);
		if (zx->bdi->flop[dsk].getDiskType() == TYPE_TRD) {
			std::vector<TRFile> cat = zx->bdi->flop[dsk].getTRCatalog();
			wid->setRowCount(cat.size());
			for (uint i=0; i<cat.size(); i++) {
				itm = new QTableWidgetItem(QString(std::string((char*)cat[i].name,8).c_str())); wid->setItem(i,0,itm);
				itm = new QTableWidgetItem(QString(QChar(cat[i].ext))); wid->setItem(i,1,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].lst + (cat[i].hst << 8))); wid->setItem(i,2,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].llen + (cat[i].hlen << 8))); wid->setItem(i,3,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].slen)); wid->setItem(i,4,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].trk)); wid->setItem(i,5,itm);
				itm = new QTableWidgetItem(QString::number(cat[i].sec)); wid->setItem(i,6,itm);
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

// disk

void SetupWin::newdisk(int idx) {
	Floppy *flp = &zx->bdi->flop[idx & 3];
	if (!flp->savecha()) return;
	flp->format();
	flp->path = "";
	flp->insert = true;
	flp->changed = true;
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

void SetupWin::savea() {if (zx->bdi->flop[0].insert) saveFile(zx->bdi->flop[0].path.c_str(),FT_DISK,0);}
void SetupWin::saveb() {if (zx->bdi->flop[1].insert) saveFile(zx->bdi->flop[1].path.c_str(),FT_DISK,1);}
void SetupWin::savec() {if (zx->bdi->flop[2].insert) saveFile(zx->bdi->flop[2].path.c_str(),FT_DISK,2);}
void SetupWin::saved() {if (zx->bdi->flop[3].insert) saveFile(zx->bdi->flop[3].path.c_str(),FT_DISK,3);}

void SetupWin::ejcta() {zx->bdi->flop[0].eject(); updatedisknams();}
void SetupWin::ejctb() {zx->bdi->flop[1].eject(); updatedisknams();}
void SetupWin::ejctc() {zx->bdi->flop[2].eject(); updatedisknams();}
void SetupWin::ejctd() {zx->bdi->flop[3].eject(); updatedisknams();}

void SetupWin::updatedisknams() {
	ui.apathle->setText(QDialog::trUtf8(zx->bdi->flop[0].path.c_str()));
	ui.bpathle->setText(QDialog::trUtf8(zx->bdi->flop[1].path.c_str()));
	ui.cpathle->setText(QDialog::trUtf8(zx->bdi->flop[2].path.c_str()));
	ui.dpathle->setText(QDialog::trUtf8(zx->bdi->flop[3].path.c_str()));
	fillDiskCat();
}

// tape

void SetupWin::loatape() {loadFile("",FT_TAPE,1); ui.tpathle->setText(QDialog::trUtf8(zx->tape->path.c_str())); buildtapelist();}
void SetupWin::savtape() {
	if (zx->tape->data.size()!=0) saveFile(zx->tape->path.c_str(),FT_TAP,-1);
}
void SetupWin::ejctape() {zx->tape->eject(); ui.tpathle->setText(QDialog::trUtf8(zx->tape->path.c_str())); buildtapelist();}
void SetupWin::tblkup() {
	int ps = ui.tapelist->currentIndex().row();
	if (ps>0) {zx->tape->swapblocks(ps,ps-1); buildtapelist(); ui.tapelist->selectRow(ps-1);}
}
void SetupWin::tblkdn() {
	int ps = ui.tapelist->currentIndex().row();
	if ((ps!=-1) && (ps<(int)zx->tape->data.size()-1)) {zx->tape->swapblocks(ps,ps+1); buildtapelist(); ui.tapelist->selectRow(ps+1);}
}
void SetupWin::tblkrm() {
	int ps = ui.tapelist->currentIndex().row();
	if (ps!=-1) {zx->tape->data.erase(zx->tape->data.begin()+ps); buildtapelist(); ui.tapelist->selectRow(ps);}
}

void SetupWin::chablock(QModelIndex idx) {
	int row = idx.row();
	zx->tape->block = row;
	zx->tape->pos = 0;
	buildtapelist();
	ui.tapelist->selectRow(row);
}

void SetupWin::setTapeBreak(int row,int col) {
	if ((row < 0) || (col != 1)) return;
	zx->tape->data[row].flags ^= TBF_BREAK;
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
