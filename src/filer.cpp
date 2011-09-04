#include "filer.h"
#include "spectrum.h"
#include "emulwin.h"

extern ZXComp* zx;
extern EmulWin* mwin;

#include <QDebug>

MFiler::MFiler(QWidget *pr):QFileDialog(pr) {
	setWindowIcon(QIcon(":/images/logo.png"));
	setWindowModality(Qt::ApplicationModal);
	setNameFilterDetailsVisible(true);
}

QString getFilter(int flags) {
	QString res = "";
	if (flags & FT_SNA) res.append(" *.sna");
	if (flags & FT_Z80) res.append(" *.z80");
	if (flags & FT_TAP) res.append(" *.tap");
	if (flags & FT_TZX) res.append(" *.tzx");
	if (flags & FT_SCL) res.append(" *.scl");
	if (flags & FT_TRD) res.append(" *.trd");
	if (flags & FT_FDI) res.append(" *.fdi");
	if (flags & FT_UDI) res.append(" *.udi");
#ifdef HAVEZLIB
	if (flags & FT_RZX) res.append(" *.rzx");
#endif
	if (res.startsWith(" ")) res.remove(0,1);
	return res;
}

int getFileType(QString path) {
	if (path.endsWith(".sna",Qt::CaseInsensitive)) return FT_SNA;
	if (path.endsWith(".z80",Qt::CaseInsensitive)) return FT_Z80;
	if (path.endsWith(".tap",Qt::CaseInsensitive)) return FT_TAP;
	if (path.endsWith(".tzx",Qt::CaseInsensitive)) return FT_TZX;
	if (path.endsWith(".scl",Qt::CaseInsensitive)) return FT_SCL;
	if (path.endsWith(".trd",Qt::CaseInsensitive)) return FT_TRD;
	if (path.endsWith(".fdi",Qt::CaseInsensitive)) return FT_FDI;
	if (path.endsWith(".udi",Qt::CaseInsensitive)) return FT_UDI;
#ifdef HAVEZLIB
	if (path.endsWith(".rzx",Qt::CaseInsensitive)) return FT_RZX;
#endif
	return FT_NONE;
}

void MFiler::loadFile(const char* name, int flags, int drv) {
	QString path(name);
	setDirectory(lastdir);
	if (path == "") {
		QString filters = QString("All known types (").append(getFilter(flags)).append(")");
		if ((flags & FT_DISK) && (drv == -1)) {
			filters.append(";;Disk A (").append(getFilter(flags & FT_DISK)).append(")");
			filters.append(";;Disk B (").append(getFilter(flags & FT_DISK)).append(")");
			filters.append(";;Disk C (").append(getFilter(flags & FT_DISK)).append(")");
			filters.append(";;Disk D (").append(getFilter(flags & FT_DISK)).append(")");
		}
		if (flags & FT_SNAP) filters.append(";;Snapshot (").append(getFilter(flags & FT_SNAP)).append(")");
		if (flags & FT_TAPE) filters.append(";;Tape (").append(getFilter(flags & FT_TAPE)).append(")");
		if (flags & FT_RZX) filters.append(";;RZX file (").append(getFilter(flags & FT_RZX)).append(")");
		setWindowTitle("Open file");
		setNameFilter(filters);
		setOptions(QFileDialog::DontUseNativeDialog);
		setDirectory(lastdir);
		setAcceptMode(QFileDialog::AcceptOpen);
		if (!exec()) return;
		filters = selectedNameFilter();
		if (filters.contains("Disk A")) drv = 0;
		if (filters.contains("Disk B")) drv = 1;
		if (filters.contains("Disk C")) drv = 2;
		if (filters.contains("Disk D")) drv = 3;
		path = selectedFiles().first();
		lastdir = directory().absolutePath();
	}
	if (drv == -1) drv = 0;
	int type = getFileType(path);
	std::string sfnam(path.toUtf8().data());
	switch (type) {
		case FT_SNA: zx->sys->mem->load(sfnam,TYP_SNA); break;
		case FT_Z80: zx->sys->mem->load(sfnam,TYP_Z80); break;
		case FT_TAP: zx->tape->load(sfnam,TYPE_TAP); break;
		case FT_TZX: zx->tape->load(sfnam,TYPE_TZX); break;
		case FT_SCL: zx->bdi->flop[drv].load(sfnam,TYPE_SCL); break;
		case FT_TRD: zx->bdi->flop[drv].load(sfnam,TYPE_TRD); break;
		case FT_FDI: zx->bdi->flop[drv].load(sfnam,TYPE_FDI); break;
		case FT_UDI: zx->bdi->flop[drv].load(sfnam,TYPE_UDI); break;
#if HAVEZLIB
		case FT_RZX: mwin->load(sfnam,TYP_RZX); break;
#endif
	}
}

// SAVE

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

bool MFiler::savedisk(std::string sfnam, uint8_t drv,bool sel) {
	if (!zx->bdi->flop[drv].insert) return true;
	QString fnam(sfnam.c_str());
	if (sel) {
		fnam = save(NULL,QString("Save disk %1:").arg(QChar('A'+drv)),fnam,QStringList()<<"Disk image (*.trd *.scl *.udi)").selfile;
		sfnam = std::string(fnam.toUtf8().data());
	}
	if (sfnam!="") {
		bool kformat=false;
		if (fnam.endsWith(".trd",Qt::CaseInsensitive)) {zx->bdi->flop[drv].save(sfnam,TYPE_TRD); kformat=true;}
		if (fnam.endsWith(".scl",Qt::CaseInsensitive)) {zx->bdi->flop[drv].save(sfnam,TYPE_SCL); kformat=true;}
		if (fnam.endsWith(".udi",Qt::CaseInsensitive)) {zx->bdi->flop[drv].save(sfnam,TYPE_UDI); kformat=true;}
		if (!kformat) {fnam.append(".trd"); zx->bdi->flop[drv].save(sfnam,TYPE_TRD);}
		zx->bdi->flop[drv].path=sfnam;
		zx->bdi->flop[drv].changed=false;
	}
	return (sfnam!="");
}

void MFiler::saveonf2() {
	mwin->repause(true,PR_FILE);
	QStringList filter;
	std::string sfnam;
	if (zx->bdi->flop[0].insert) {filter<<"Disk A (*.trd *.scl *.udi)"; sfnam = zx->bdi->flop[0].path;}
	if (zx->tape->data.size()!=0) {filter<<"Tape (*.tap)"; if (sfnam=="") sfnam = zx->tape->path;}
	filter<<"Snapshot (*.sna)"; if (sfnam=="") sfnam = "snapshot.sna";
	MFResult res = save(0,"Save something",QString(sfnam.c_str()),filter);
	if (res.selfile!="") {
		sfnam = std::string(res.selfile.toUtf8().data());
		QString filt = res.selfilt;
		if (filt.indexOf("Snapshot")!=-1) zx->sys->mem->save(sfnam,TYP_SNA,zx->opt.hwName=="ZX48K");
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
