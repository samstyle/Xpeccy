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
	for (i=0; i<9; i++) {
		itm = new QTableWidgetItem; ui.rstab->setItem(i,1,itm);
		itm = new QTableWidgetItem; ui.rstab->setItem(i,2,itm);
	}
// video
	ui.ssfbox->addItems(QStringList()<<"bmp"<<"png"<<"jpg"<<"scr");
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
// tape
	ui.tapelist->setColumnWidth(0,20);
	ui.tapelist->setColumnWidth(1,20);
	ui.tapelist->setColumnWidth(2,50);
	ui.tapelist->setColumnWidth(3,50);
	ui.tapelist->setColumnWidth(4,100);
// hdd
	ui.hiface->addItem("None",QVariant(IDE_NONE));
	ui.hiface->addItem("Nemo",QVariant(IDE_NEMO));
	ui.hiface->addItem("Nemo A8",QVariant(IDE_NEMOA8));
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
	QObject::connect(ui.rsselcan,SIGNAL(released()),this,SLOT(hidersedit()));
	QObject::connect(ui.rsselok,SIGNAL(released()),this,SLOT(setrpart()));
	QObject::connect(ui.addrset,SIGNAL(released()),optName,SLOT(show()));
	QObject::connect(ui.rmrset,SIGNAL(released()),this,SLOT(rmRomset()));
	QObject::connect(nameui.okbut,SIGNAL(released()),this,SLOT(addNewRomset()));
	QObject::connect(nameui.cnbut,SIGNAL(released()),optName,SLOT(hide()));
// video
	QObject::connect(ui.pathtb,SIGNAL(released()),this,SLOT(selsspath()));
	QObject::connect(ui.bszsld,SIGNAL(valueChanged(int)),this,SLOT(chabsz()));
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
// tape
	QObject::connect(ui.tapelist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chablock(QModelIndex)));
	QObject::connect(ui.tapelist,SIGNAL(cellClicked(int,int)),this,SLOT(setTapeBreak(int,int)));
	QObject::connect(ui.tloadtb,SIGNAL(released()),this,SLOT(loatape()));
	QObject::connect(ui.tsavetb,SIGNAL(released()),this,SLOT(savtape()));
	QObject::connect(ui.tremotb,SIGNAL(released()),this,SLOT(ejctape()));
	QObject::connect(ui.blkuptb,SIGNAL(released()),this,SLOT(tblkup()));
	QObject::connect(ui.blkdntb,SIGNAL(released()),this,SLOT(tblkdn()));
	QObject::connect(ui.blkrmtb,SIGNAL(released()),this,SLOT(tblkrm()));
// hdd
	QObject::connect(ui.hm_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hm_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_islba,SIGNAL(stateChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_glba,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_gcyl,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_ghd,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
	QObject::connect(ui.hs_gsec,SIGNAL(valueChanged(int)),this,SLOT(hddcap()));
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
	ui.rsetbox->setCurrentIndex(ui.rsetbox->findText(QDialog::trUtf8(zx->sys->mem->romset->name.c_str())));
	ui.reschk->setChecked(emulGetFlags() & FL_RESET);
	ui.resbox->setCurrentIndex(zx->sys->mem->res);
	switch(zx->sys->mem->mask) {
		case 0x00: ui.mszbox->setCurrentIndex(0); break;
		case 0x07: ui.mszbox->setCurrentIndex(1); break;
		case 0x0f: ui.mszbox->setCurrentIndex(2); break;
		case 0x1f: ui.mszbox->setCurrentIndex(3); break;
		case 0x3f: ui.mszbox->setCurrentIndex(4); break;
	}
	ui.cpufrq->setValue(zx->sys->cpu->frq * 2);
	ui.scrpwait->setChecked(zx->sys->hwflags & WAIT_ON);
	updfrq();
// video
	ui.dszchk->setChecked((zx->vid->flags & VF_DOUBLE));
//	ui.fscchk->setChecked(vid->fscreen);
	ui.bszsld->setValue((int)(zx->vid->brdsize * 100));
	ui.pathle->setText(QDialog::trUtf8(optGetString(OPT_SHOTDIR).c_str()));
	ui.ssfbox->setCurrentIndex(ui.ssfbox->findText(QDialog::trUtf8(optGetString(OPT_SHOTEXT).c_str())));
	ui.scntbox->setValue(optGetInt(OPT_SHOTCNT));
	ui.sintbox->setValue(optGetInt(OPT_SHOTINT));
	ui.geombox->setCurrentIndex(ui.geombox->findText(QDialog::trUtf8(zx->vid->curlay.c_str())));
// sound
	ui.senbox->setChecked(sndGet(SND_ENABLE) != 0);
	ui.mutbox->setChecked(sndGet(SND_MUTE) != 0);
	ui.gsrbox->setChecked(zx->gs->flags & GS_RESET);
	ui.outbox->setCurrentIndex(ui.outbox->findText(QDialog::trUtf8(sndGetOutputName().c_str())));
	ui.ratbox->setCurrentIndex(ui.ratbox->findText(QString::number(sndGet(SND_RATE))));
	ui.bvsld->setValue(sndGet(SND_BEEP));
	ui.tvsld->setValue(sndGet(SND_TAPE));
	ui.avsld->setValue(sndGet(SND_AYVL));
	ui.gvsld->setValue(sndGet(SND_GSVL));
	ui.schip1box->setCurrentIndex(ui.schip1box->findData(QVariant(zx->aym->sc1->type)));
	ui.schip2box->setCurrentIndex(ui.schip2box->findData(QVariant(zx->aym->sc2->type)));
	ui.stereo1box->setCurrentIndex(ui.stereo1box->findData(QVariant(zx->aym->sc1->stereo)));
	ui.stereo2box->setCurrentIndex(ui.stereo2box->findData(QVariant(zx->aym->sc2->stereo)));
	ui.gstereobox->setCurrentIndex(ui.gstereobox->findData(QVariant(zx->gs->stereo)));
	ui.gsgroup->setChecked(zx->gs->flags & GS_ENABLE);
	ui.tsbox->setCurrentIndex(ui.tsbox->findData(QVariant(zx->aym->tstype)));
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
	zx->sys->mem->loadromset(optGetString(OPT_ROMDIR));
	emulSetFlag(FL_RESET, ui.reschk->isChecked());
	zx->sys->mem->res = ui.resbox->currentIndex();
	switch(ui.mszbox->currentIndex()) {
		case 0: zx->sys->mem->mask = 0x00; break;
		case 1: zx->sys->mem->mask = 0x07; break;
		case 2: zx->sys->mem->mask = 0x0f; break;
		case 3: zx->sys->mem->mask = 0x1f; break;
		case 4: zx->sys->mem->mask = 0x3f; break;
	}
	zx->sys->cpu->frq = ui.cpufrq->value() / 2.0;
	setFlagBit(ui.scrpwait->isChecked(),&zx->sys->hwflags,WAIT_ON);
	zx->opt.GSRom = GSRom;
	setRomsetList(rsl);
//	setRomset(zx, zx->opt.rsName);
//	zx->sys->mem->loadromset(optGetPath(OPT_ROMDIR));
	if (zx->hw != oldmac) zx->reset(RES_DEFAULT);
// video
	setFlagBit(ui.dszchk->isChecked(),&zx->vid->flags,VF_DOUBLE);
	zx->vid->brdsize = ui.bszsld->value()/100.0;
	optSet(OPT_SHOTDIR,std::string(ui.pathle->text().toUtf8().data()));
	optSet(OPT_SHOTEXT,std::string(ui.ssfbox->currentText().toUtf8().data()));
	optSet(OPT_SHOTCNT,ui.scntbox->value());
	optSet(OPT_SHOTINT,ui.sintbox->value());
	zx->vid->setLayout(std::string(ui.geombox->currentText().toUtf8().data()));
//	emulUpdateWindow();
// sound
	std::string oname = sndGetOutputName();
	std::string nname(ui.outbox->currentText().toUtf8().data());
	int orate = sndGet(SND_RATE);
	sndSet(SND_ENABLE, ui.senbox->isChecked());
	sndSet(SND_MUTE, ui.mutbox->isChecked());
	if (ui.gsrbox->isChecked()) zx->gs->flags |= GS_RESET; else zx->gs->flags &= ~GS_RESET;
	sndSet(SND_RATE, ui.ratbox->currentText().toInt());
	sndSet(SND_BEEP, ui.bvsld->value());
	sndSet(SND_TAPE, ui.tvsld->value());
	sndSet(SND_AYVL, ui.avsld->value());
	sndSet(SND_GSVL, ui.gvsld->value());
	if ((oname != nname) || (orate != sndGet(SND_RATE))) setOutput(nname);
	zx->aym->sc1->settype(ui.schip1box->itemData(ui.schip1box->currentIndex()).toInt());
	zx->aym->sc2->settype(ui.schip2box->itemData(ui.schip2box->currentIndex()).toInt());
	zx->aym->sc1->stereo = ui.stereo1box->itemData(ui.stereo1box->currentIndex()).toInt();
	zx->aym->sc2->stereo = ui.stereo2box->itemData(ui.stereo2box->currentIndex()).toInt();
	zx->gs->stereo = ui.gstereobox->itemData(ui.gstereobox->currentIndex()).toInt();
	setFlagBit(ui.gsgroup->isChecked(),&zx->gs->flags,GS_ENABLE);
	setFlagBit(ui.gsrbox->isChecked(),&zx->gs->flags,GS_RESET);
	zx->aym->tstype = ui.tsbox->itemData(ui.tsbox->currentIndex()).toInt();
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
	setFlagBit(ui.hm_islba->isChecked(),&(zx->ide->master.flags),ATA_LBA);
//	zx->ide->master.canlba = ui.hm_islba->isChecked();
	zx->ide->master.pass.spt = ui.hm_gsec->value();
	zx->ide->master.pass.hds = ui.hm_ghd->value();
	zx->ide->master.pass.cyls = ui.hm_gcyl->value();
	zx->ide->master.maxlba = ui.hm_glba->value();
	
	zx->ide->slave.iface = ui.hs_type->itemData(ui.hs_type->currentIndex()).toInt();
	zx->ide->slave.pass.model = std::string(ui.hs_model->text().toUtf8().data(),40);
	zx->ide->slave.pass.serial = std::string(ui.hs_ser->text().toUtf8().data(),20);
	zx->ide->slave.image = std::string(ui.hs_path->text().toUtf8().data());
	setFlagBit(ui.hs_islba->isChecked(),&zx->ide->master.flags,ATA_LBA);
//	zx->ide->slave.canlba = ui.hs_islba->isChecked();
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

void SetupWin::editrset(QModelIndex idx) {
	QString rcur;
	uint8_t pcur;
	if (idx.row() < 8) {
		RomSet rset = rsl[ui.rsetbox->currentIndex()];
		rcur = QString(rset.roms[idx.row()].path.c_str());
		if (rcur == "") rcur = "none";
		pcur = rset.roms[idx.row()].part;
	} else {
		rcur = QString(GSRom.c_str());
		pcur = 0;
	}
	std::string rpth = optGetString(OPT_ROMDIR);
	QDir rdir(QString(rpth.c_str()));
	QStringList rlst = rdir.entryList(QStringList() << "*.rom",QDir::Files,QDir::Name);
	ui.rsfiles->clear();
	ui.rsfiles->addItem("none");
	ui.rsfiles->addItems(rlst);
	ui.rsfiles->setCurrentIndex(ui.rsfiles->findText(rcur));
	ui.rsparts->setValue(pcur);
	ui.rstab->hide();
	ui.rsetbox->setEnabled(false);
	ui.addrset->setEnabled(false);
	ui.rmrset->setEnabled(false);
	ui.rssel->show();
}

void SetupWin::setrpart() {
	QString rnew = ui.rsfiles->itemText(ui.rsfiles->currentIndex());
	if (rnew == "none") rnew = "";
	int pnew = ui.rsparts->value();
	QModelIndex idx = ui.rstab->currentIndex();
	if (idx.isValid()) {
		int row = idx.row();
		if (row < 8) {
			rsl[ui.rsetbox->currentIndex()].roms[row].path = std::string(rnew.toUtf8().data());
			rsl[ui.rsetbox->currentIndex()].roms[row].part = pnew;
		} else {
			GSRom = std::string(rnew.toUtf8().data());
		}
	}
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
	QString rsf;
//	QStringList lst = QStringList()<<"Basic128"<<"Basic48"<<"Shadow"<<"DOS"<<"ext0"<<"ext1"<<"ext2"<<"ext3";
	for (int i=0; i<8; i++) {
		rsf = QDialog::trUtf8(rset.roms[i].path.c_str());
		ui.rstab->item(i,1)->setText(rsf);
		if (rsf != "") {
			ui.rstab->item(i,2)->setText(QString::number(rset.roms[i].part));
		} else {
			ui.rstab->item(i,2)->setText("");
		}
	}
	ui.rstab->item(8,1)->setText(QDialog::trUtf8(GSRom.c_str()));
	ui.rstab->setColumnWidth(0,100);
	ui.rstab->setColumnWidth(1,300);
	ui.rstab->setColumnWidth(2,50);
}

void SetupWin::buildtapelist() {
	buildTapeList();
	std::vector<TapeBlockInfo> inf = zx->tape->getInfo();
	QTableWidgetItem* itm;
	ui.tapelist->setRowCount(inf.size());
	uint tm,ts;
	for (uint i=0; i<inf.size(); i++) {
		if ((int)zx->tape->block == i) {
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

/*
//	int i,tm,ts;
//	QStandardItemModel *model = new QStandardItemModel(zx->tape->data.size(),5);
//	QStandardItem *itm;
	
	for(i=0;i<model->rowCount();i++) {
		if ((int)zx->tape->block == i) {
			itm = new QStandardItem(QIcon(":/images/checkbox.png"),"");
			model->setItem(i,0,itm);
			ts = zx->tape->data[i].gettime(zx->tape->pos); tm = ts/60; ts -= tm*60;
			itm = new QStandardItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
			model->setItem(i,2,itm);
		} else {
			itm = new QStandardItem(); model->setItem(i,0,itm);
			itm = new QStandardItem(); model->setItem(i,2,itm);
		}
		ts = zx->tape->data[i].gettime(-1); tm = ts/60; ts -= tm*60;
		itm = new QStandardItem(QString::number(tm).append(":").append(QString::number(ts+100).right(2)));
		model->setItem(i,1,itm);
		itm = new QStandardItem(QString::number(zx->tape->data[i].getsize()));
		model->setItem(i,3,itm);
		itm = new QStandardItem(QDialog::trUtf8(zx->tape->data[i].getheader().c_str()));
		model->setItem(i,4,itm);
	}
	ui.tapelist->setModel(model);
	ui.tapelist->setColumnWidth(0,20);
	ui.tapelist->setColumnWidth(1,50);
	ui.tapelist->setColumnWidth(2,50);
	ui.tapelist->setColumnWidth(3,100);
	ui.tapelist->selectRow(0);
}
*/

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

// machine

void SetupWin::updfrq() {
	double f = ui.cpufrq->value() / 2.0;
	ui.cpufrqlab->setText(QString::number(f,'f',2).append(" MHz"));
}

// video

void SetupWin::chabsz() {ui.bszlab->setText(QString::number(ui.bszsld->value()).append("%"));}

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
	flp->path = std::string(QDir::homePath().toUtf8().data()) + "/disk.trd";
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
