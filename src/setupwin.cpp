#include "setupwin.h"
#include "emulwin.h"
#include "develwin.h"
#include "settings.h"
#include "filer.h"

#include "sound.h"
#include "spectrum.h"

#include <QStandardItemModel>
#include <QFileDialog>

extern Settings* sets;
extern EmulWin* mwin;
extern Sound* snd;
extern DevelWin* dwin;
extern ZXComp* zx;

void setFlagBit(bool,int32_t*,int32_t);

SetupWin::SetupWin(QWidget* par):QDialog(par) {
	setModal(true);
	ui.setupUi(this);

	umadial = new QDialog;
	uia.setupUi(umadial);
	umadial->setModal(true);

	uint32_t i;
// machine
	for (i=0; i < zx->hwlist.size(); i++) {
		ui.machbox->addItem(QDialog::trUtf8(zx->hwlist[i].name.c_str()));
	}
/*
	ui.rsetbox->clear();
	for (i=0; i < zx->sys->mem->rsetlist.size(); i++) {
		ui.rsetbox->addItem(QDialog::trUtf8(zx->sys->mem->rsetlist[i].name.c_str()));
	}
*/
	ui.resbox->addItems(QStringList()<<"0:Basic 128"<<"1:Basic48"<<"2:Shadow"<<"3:DOS");
	ui.mszbox->addItems(QStringList()<<"48K"<<"128K"<<"256K"<<"512K"<<"1024K");
// video
	ui.ssfbox->addItems(QStringList()<<"bmp"<<"png"<<"jpg"<<"scr");
	for (i=0;i<zx->vid->layout.size();i++) {ui.geombox->addItem(QDialog::trUtf8(zx->vid->layout[i].name.c_str()));}
// sound
	for (i=0;i<snd->outsyslist.size();i++) {ui.outbox->addItem(QDialog::trUtf8(snd->outsyslist[i].name.c_str()));}
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
	mwin->repause(true,PR_OPTS);
// machine
	ui.rsetbox->clear();
	for (i=0; i < zx->sys->mem->rsetlist.size(); i++) {
		ui.rsetbox->addItem(QDialog::trUtf8(zx->sys->mem->rsetlist[i].name.c_str()));
	}
	ui.machbox->setCurrentIndex(ui.machbox->findText(QDialog::trUtf8(zx->hw->name.c_str())));
	ui.rsetbox->setCurrentIndex(ui.rsetbox->findText(QDialog::trUtf8(zx->sys->mem->romset->name.c_str())));
	ui.reschk->setChecked(mwin->flags & FL_RESET);
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
	ui.pathle->setText(QDialog::trUtf8(sets->opt.scrshotDir.c_str()));
	ui.ssfbox->setCurrentIndex(ui.ssfbox->findText(QDialog::trUtf8(sets->opt.scrshotFormat.c_str())));
	ui.scntbox->setValue(sets->sscnt);
	ui.sintbox->setValue(sets->ssint);
	ui.geombox->setCurrentIndex(ui.geombox->findText(QDialog::trUtf8(zx->vid->curlay.c_str())));
// sound
	ui.senbox->setChecked(snd->enabled);
	ui.mutbox->setChecked(snd->mute);
	ui.gsrbox->setChecked(zx->gs->flags & GS_RESET);
	ui.outbox->setCurrentIndex(ui.outbox->findText(QDialog::trUtf8(sets->opt.sndOutputName.c_str())));
	ui.ratbox->setCurrentIndex(ui.ratbox->findText(QString::number(snd->rate)));
	ui.bvsld->setValue(snd->beepvol);
	ui.tvsld->setValue(snd->tapevol);
	ui.avsld->setValue(snd->ayvol);
	ui.gvsld->setValue(snd->gsvol);
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
	ui.sjpathle->setText(QDialog::trUtf8(sets->opt.asmPath.c_str()));
	ui.prjdirle->setText(QDialog::trUtf8(sets->opt.projectsDir.c_str()));
	buildmenulist();

	show();
}

void SetupWin::apply() {
// machine
	HardWare *oldmac = zx->hw;
	zx->opt.hwName = std::string(ui.machbox->currentText().toUtf8().data());
	zx->setHardware(zx->opt.hwName);
	zx->opt.romsetName = std::string(ui.rsetbox->currentText().toUtf8().data());
	zx->sys->mem->setromptr(zx->opt.romsetName);
	zx->sys->mem->loadromset(sets->opt.romDir);
	if (ui.reschk->isChecked()) mwin->flags |= FL_RESET; else mwin->flags &= ~FL_RESET;
	zx->sys->mem->res = ui.resbox->currentIndex();
	switch(ui.mszbox->currentIndex()) {
		case 0: zx->sys->mem->mask = 0x00; break;
		case 1: zx->sys->mem->mask = 0x07; break;
		case 2: zx->sys->mem->mask = 0x0f; break;
		case 3: zx->sys->mem->mask = 0x1f; break;
		case 4: zx->sys->mem->mask = 0x3f; break;
	}
	if (zx->hw != oldmac) mwin->reset();
	zx->sys->cpu->frq = ui.cpufrq->value() / 2.0;
	if (ui.scrpwait->isChecked()) zx->sys->hwflags |= WAIT_ON; else zx->sys->hwflags &= ~WAIT_ON;
// video
	if (ui.dszchk->isChecked()) zx->vid->flags |= VF_DOUBLE; else zx->vid->flags &= ~VF_DOUBLE;
//	vid->fscreen = ui.fscchk->isChecked();
	zx->vid->brdsize = ui.bszsld->value()/100.0;
	sets->opt.scrshotDir = std::string(ui.pathle->text().toUtf8().data());
	sets->opt.scrshotFormat = std::string(ui.ssfbox->currentText().toUtf8().data());
	sets->sscnt = ui.scntbox->value();
	sets->ssint = ui.sintbox->value();
	zx->vid->setlayout(std::string(ui.geombox->currentText().toUtf8().data()));
	mwin->updateWin();
// sound
	std::string oname = sets->opt.sndOutputName;
	int orate = snd->rate;
	snd->enabled = ui.senbox->isChecked();
	snd->mute = ui.mutbox->isChecked();
	if (ui.gsrbox->isChecked()) zx->gs->flags |= GS_RESET; else zx->gs->flags &= ~GS_RESET;
	sets->opt.sndOutputName = std::string(ui.outbox->currentText().toUtf8().data());
	snd->rate = ui.ratbox->currentText().toInt();
	snd->beepvol = ui.bvsld->value();
	snd->tapevol = ui.tvsld->value();
	snd->ayvol = ui.avsld->value();
	snd->gsvol = ui.gvsld->value();
	if ((oname != sets->opt.sndOutputName) || (orate != snd->rate)) snd->setoutptr(sets->opt.sndOutputName);
	zx->aym->sc1->settype(ui.schip1box->itemData(ui.schip1box->currentIndex()).toInt());
	zx->aym->sc2->settype(ui.schip2box->itemData(ui.schip2box->currentIndex()).toInt());
	zx->aym->sc1->stereo = ui.stereo1box->itemData(ui.stereo1box->currentIndex()).toInt();
	zx->aym->sc2->stereo = ui.stereo2box->itemData(ui.stereo2box->currentIndex()).toInt();
	zx->gs->stereo = ui.gstereobox->itemData(ui.gstereobox->currentIndex()).toInt();
	if (ui.gsgroup->isChecked()) zx->gs->flags |= GS_ENABLE; else zx->gs->flags &= ~GS_ENABLE;
	zx->aym->tstype = ui.tsbox->itemData(ui.tsbox->currentIndex()).toInt();
// dos
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
	sets->opt.asmPath = std::string(ui.sjpathle->text().toUtf8().data());
	sets->opt.projectsDir = std::string(ui.prjdirle->text().toUtf8().data());

	snd->defpars();
	zx->vid->update();
	sets->save();
}

void SetupWin::reject() {
	hide();
	mwin->makeBookmarkMenu();
	mwin->repause(false,PR_OPTS);
}

// lists

void SetupWin::okbuts() {
	int t = zx->hwlist[ui.machbox->currentIndex()].mask;
	if (t == 0x00) {
		ui.okbut->setEnabled(ui.mszbox->currentIndex()==0);
	} else {
		ui.okbut->setEnabled((1<<(ui.mszbox->currentIndex()-1)) & t);
	}
	ui.apbut->setEnabled(ui.okbut->isEnabled());
}

void SetupWin::setmszbox(int idx) {
	int t = zx->hwlist[idx].mask;
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
	int i;
	if (ui.rsetbox->currentIndex() < 0) {
		ui.rstab->setEnabled(false);
		return;
	} else {
		ui.rstab->setEnabled(true);
	}
	RomSet rset = zx->sys->mem->rsetlist[ui.rsetbox->currentIndex()];
	QStringList lst = QStringList()<<"Basic128"<<"Basic48"<<"Shadow"<<"DOS"<<"ext0"<<"ext1"<<"ext2"<<"ext3";
	QStandardItemModel *model = new QStandardItemModel(8,3);
	QStandardItem *itm;
	for (i=0; i<8; i++) {
		itm = new QStandardItem(lst[i]); model->setItem(i,0,itm);
		itm = new QStandardItem(QDialog::trUtf8(rset.roms[i].path.c_str())); model->setItem(i,1,itm);
		if (rset.roms[i].path!="") {
			itm = new QStandardItem(QString::number(rset.roms[i].part));
		} else {
			itm = new QStandardItem();
		}
		model->setItem(i,2,itm);
	}
	itm = new QStandardItem("GS"); model->setItem(i,0,itm);
	itm = new QStandardItem(QDialog::trUtf8(zx->opt.GSRom.c_str())); model->setItem(i,1,itm);
	ui.rstab->setModel(model);
	ui.rstab->setColumnWidth(0,100);
	ui.rstab->setColumnWidth(1,300);
	ui.rstab->setColumnWidth(2,50);
}

void SetupWin::buildtapelist() {
	int i,tm,ts;
	QStandardItemModel *model = new QStandardItemModel(zx->tape->data.size(),5);
	QStandardItem *itm;
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

void SetupWin::buildmenulist() {
	QStandardItemModel *model = new QStandardItemModel(sets->umenu.data.size(),2);
	QStandardItem *itm;
	uint8_t i;
	for(i=0;i<sets->umenu.data.size();i++) {
		itm = new QStandardItem(QDialog::trUtf8(sets->umenu.data[i].name.c_str()));
		model->setItem(i,0,itm);
		itm = new QStandardItem(QDialog::trUtf8(sets->umenu.data[i].path.c_str()));
		model->setItem(i,1,itm);
	}
	ui.umlist->setModel(model);
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
	QString fpath = QFileDialog::getExistingDirectory(this,"Screenshots folder",QDialog::trUtf8(sets->opt.scrshotDir.c_str()),QFileDialog::ShowDirsOnly);
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
	QString fnam = QFileDialog::getOpenFileName(NULL,"Select SJAsm executable","","All files (*)");
	if (fnam!="") ui.sjpathle->setText(fnam);
}

void SetupWin::sprjpath() {
	QString fnam = QFileDialog::getExistingDirectory(this,"Projects file",QDialog::trUtf8(sets->opt.projectsDir.c_str()),QFileDialog::ShowDirsOnly);
	if (fnam!="") ui.prjdirle->setText(fnam);
}

void SetupWin::umup() {
	int ps = ui.umlist->currentIndex().row();
	if (ps>0) {sets->umenu.swap(ps,ps-1); buildmenulist(); ui.umlist->selectRow(ps-1);}
}

void SetupWin::umdn() {
	int ps = ui.umlist->currentIndex().row();
	if ((ps!=-1) && (ps < (int)sets->umenu.data.size()-1)) {sets->umenu.swap(ps,ps+1); buildmenulist(); ui.umlist->selectRow(ps+1);}
}

void SetupWin::umdel() {
	int ps = ui.umlist->currentIndex().row();
	if (ps!=-1) {
		sets->umenu.del(ps); buildmenulist();
		if (ps==(int)sets->umenu.data.size()) ui.umlist->selectRow(ps-1); else ui.umlist->selectRow(ps);
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
	uia.namele->setText(QDialog::trUtf8(sets->umenu.data[umidx].name.c_str()));
	uia.pathle->setText(QDialog::trUtf8(sets->umenu.data[umidx].path.c_str()));
	umadial->show();
}

void SetupWin::umaselp() {
	QString fpath = getFileName(NULL,"Select file","","Known formats (*.sna *.z80 *.tap *.tzx *.trd *.scl *.fdi *.udi)");
	if (fpath!="") uia.pathle->setText(fpath);
}

void SetupWin::umaconf() {
	if ((uia.namele->text()=="") || (uia.pathle->text()=="")) return;
	if (umidx == -1) {
		sets->umenu.add(std::string(uia.namele->text().toUtf8().data()),std::string(uia.pathle->text().toUtf8().data()));
	} else {
		sets->umenu.set(umidx,std::string(uia.namele->text().toUtf8().data()),std::string(uia.pathle->text().toUtf8().data()));
	}
	umadial->hide();
	buildmenulist();
	ui.umlist->selectRow(sets->umenu.data.size()-1);
}
