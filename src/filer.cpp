#include "filer.h"
#include "bdi.h"
#include "tape.h"
#include "spectrum.h"
//#include "memory.h"
//#include "iosys.h"
#include "emulwin.h"
#include "settings.h"

//extern Tape* tape;
extern BDI* bdi;
extern ZXComp* zx;
extern Settings* sets;
extern EmulWin* mwin;

#include <QDebug>

MFiler::MFiler(QWidget *pr):QFileDialog(pr) {
	setWindowIcon(QIcon(":/images/logo.png"));
	setWindowModality(Qt::ApplicationModal);
}

void MFiler::loadtape(std::string sfnam,bool sel) {
	QString fnam(sfnam.c_str());
	if (sel) {
		fnam = open(NULL,"Load tape","",QStringList()<<"Tape images (*.tap *.tzx)").selfile;
		sfnam = std::string(fnam.toUtf8().data());
	}
	if (sfnam=="") return;
	if (fnam.endsWith(".tap",Qt::CaseInsensitive)) zx->tape->load(sfnam,TYPE_TAP);
	if (fnam.endsWith(".tzx",Qt::CaseInsensitive)) zx->tape->load(sfnam,TYPE_TZX);
}

void MFiler::savetape(std::string sfnam, bool sel=true) {
	QString fnam(sfnam.c_str());
	if (sel) {
		fnam = save(NULL,"Save tape",fnam,QStringList()<<"Tape image (*.tap)").selfile;
		sfnam = std::string(fnam.toUtf8().data());
	}
	if (sfnam=="") return;
	if (!fnam.endsWith(".tap",Qt::CaseInsensitive)) sfnam.append(".tap");
	zx->tape->save(sfnam,TYPE_TAP);
	zx->tape->path=sfnam;
}

void MFiler::loaddisk(std::string sfnam, uint8_t drv=0,bool sel=true) {
	QString fnam(sfnam.c_str());
	if (sel) {
		fnam = open(NULL,QString("Open disk %1:").arg(QChar('A'+drv)),"",QStringList()<<"Disk images (*.trd *.scl *.fdi *.udi)").selfile;
		sfnam = std::string(fnam.toUtf8().data());
	}
	if (sfnam == "") return;
	if (fnam.endsWith(".trd",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_TRD);
	if (fnam.endsWith(".scl",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_SCL);
	if (fnam.endsWith(".fdi",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_FDI);
	if (fnam.endsWith(".udi",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_UDI);
}

void MFiler::loadsomefile(std::string sfnam,uint8_t drv) {
	QString fnam = QDialog::trUtf8(sfnam.c_str());
	if (fnam.endsWith(".sna",Qt::CaseInsensitive)) zx->sys->mem->load(sfnam,TYP_SNA);
	if (fnam.endsWith(".z80",Qt::CaseInsensitive)) zx->sys->mem->load(sfnam,TYP_Z80);
	if (fnam.endsWith(".tap",Qt::CaseInsensitive)) zx->tape->load(sfnam,TYPE_TAP);
	if (fnam.endsWith(".tzx",Qt::CaseInsensitive)) zx->tape->load(sfnam,TYPE_TZX);
	if (fnam.endsWith(".trd",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_TRD);
	if (fnam.endsWith(".scl",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_SCL);
	if (fnam.endsWith(".fdi",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_FDI);
	if (fnam.endsWith(".udi",Qt::CaseInsensitive)) bdi->flop[drv].load(sfnam,TYPE_UDI);
	if (fnam.endsWith(".rzx",Qt::CaseInsensitive)) mwin->load(sfnam,TYP_RZX);
}

void MFiler::opensomewhat() {
	mwin->repause(true,PR_FILE);
	QStringList filters;
	filters<<"Known formats (*.sna *.z80 *.trd *.scl *.fdi *.udi *.tap *.tzx *.rzx)"<<"Disk B (*.trd *.scl *.fdi *.udi)"<<"Disk C (*.trd *.scl *.fdi *.udi)"<<"Disk D (*.trd *.scl *.fdi *.udi)";
	MFResult res = open(NULL,"Open somewhat","",filters);
	if (res.selfile!="") {
		std::string sfnam(res.selfile.toUtf8().data());
		uchar drv = res.fidx;
		loadsomefile(sfnam,drv);
	}
	mwin->repause(false,PR_FILE);
}

// SAVE

bool MFiler::savedisk(std::string sfnam, uint8_t drv,bool sel) {
	if (!bdi->flop[drv].insert) return true;
	QString fnam(sfnam.c_str());
	if (sel) {
		fnam = save(NULL,QString("Save disk %1:").arg(QChar('A'+drv)),fnam,QStringList()<<"Disk image (*.trd *.scl *.udi)").selfile;
		sfnam = std::string(fnam.toUtf8().data());
	}
	if (sfnam!="") {
		bool kformat=false;
		if (fnam.endsWith(".trd",Qt::CaseInsensitive)) {bdi->flop[drv].save(sfnam,TYPE_TRD); kformat=true;}
		if (fnam.endsWith(".scl",Qt::CaseInsensitive)) {bdi->flop[drv].save(sfnam,TYPE_SCL); kformat=true;}
		if (fnam.endsWith(".udi",Qt::CaseInsensitive)) {bdi->flop[drv].save(sfnam,TYPE_UDI); kformat=true;}
		if (!kformat) {fnam.append(".trd"); bdi->flop[drv].save(sfnam,TYPE_TRD);}
		bdi->flop[drv].path=sfnam;
		bdi->flop[drv].changed=false;
	}
	return (sfnam!="");
}

void MFiler::saveonf2() {
	mwin->repause(true,PR_FILE);
	QStringList filter;
	std::string sfnam;
	if (bdi->flop[0].insert) {filter<<"Disk A (*.trd *.scl *.udi)"; sfnam = bdi->flop[0].path;}
	if (zx->tape->data.size()!=0) {filter<<"Tape (*.tap)"; if (sfnam=="") sfnam = zx->tape->path;}
	filter<<"Snapshot (*.sna)"; if (sfnam=="") sfnam = "snapshot.sna";
	MFResult res = save(0,"Save something",QString(sfnam.c_str()),filter);
	if (res.selfile!="") {
		sfnam = std::string(res.selfile.toUtf8().data());
		QString filt = res.selfilt;
		if (filt.indexOf("Snapshot")!=-1) zx->sys->mem->save(sfnam,TYP_SNA,sets->machname=="ZX48K");
		if (filt.indexOf("Disk A")!=-1) savedisk(sfnam,0,false);
		if (filt.indexOf("Tape")!=-1) savetape(sfnam,false);
	}
	mwin->repause(false,PR_FILE);
}

MFResult MFiler::save(QWidget* par,QString name,QString fnam,QStringList filt) {
	setAcceptMode(QFileDialog::AcceptSave);
	setConfirmOverwrite(true);
	return execute(par,name,fnam,filt);
}

MFResult MFiler::open(QWidget* par,QString name,QString,QStringList filt) {
	setAcceptMode(QFileDialog::AcceptOpen);
	return execute(par,name,"",filt);
}

MFResult MFiler::execute(QWidget* par,QString name,QString fnam,QStringList filt) {
	MFResult res;
	res.selfile = "";
	res.selfilt = "";
	res.fidx = -1;
	setParent(par);
	setWindowTitle(name);
	if (fnam!="") {
		lastdir = QFileInfo(fnam).absoluteDir();
		if (acceptMode()==QFileDialog::AcceptSave) selectFile(fnam);
	} else {
		selectFile("");
	}
	setDirectory(lastdir);
	setNameFilters(filt);
	setOption(QFileDialog::DontUseNativeDialog,true);
	if (exec()) {
		res.selfilt = selectedNameFilter();
		res.fidx = filt.indexOf(res.selfilt);
		res.selfile = selectedFiles()[0];
		if (fnam=="") lastdir = directory().absolutePath();
	}
	return res;
}

void MFiler::savesnapshot(std::string sfnam,bool sna48=false) {
	bool kformat=false;
	QString fnam(sfnam.c_str());
	if (fnam.endsWith(".sna",Qt::CaseInsensitive)) {zx->sys->mem->save(sfnam,TYP_SNA,sna48); kformat=true;}
	if (!kformat) {sfnam.append(".sna"); zx->sys->mem->save(sfnam,TYP_SNA,sna48);}
}
