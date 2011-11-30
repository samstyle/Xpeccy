#include "filer.h"
#include "spectrum.h"
#include "common.h"

#include "filetypes/filetypes.h"

extern ZXComp* zx;
QFileDialog *filer;
QDir lastDir;

#include <QIcon>

void initFileDialog(QWidget* par) {
	filer = new QFileDialog(par);
	filer->setWindowIcon(QIcon(":/images/logo.png"));
	filer->setWindowModality(Qt::ApplicationModal);
	filer->setNameFilterDetailsVisible(true);
	filer->setConfirmOverwrite(true);
	filer->setOptions(QFileDialog::DontUseNativeDialog);
	lastDir = QDir::home();
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
	if (flags & FT_HOBETA) res.append(" *.$?");
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
	QStringList pspl = path.split(".");
	if (pspl.size() > 0) {
		if (pspl.last().startsWith("$")) return FT_HOBETA;
	}
	return FT_NONE;
}

void loadFile(const char* name, int flags, int drv) {
	QString path = QDialog::trUtf8(name);
	filer->setDirectory(lastDir);
	if (path == "") {
		QString filters = "";
		if (drv == -1) filters = QString("All known types (").append(getFilter(flags)).append(")");
		if (flags & FT_DISK) {
			if ((drv == -1) || (drv == 0)) filters.append(";;Disk A (").append(getFilter(flags & (FT_DISK | FT_HOBETA))).append(")");
			if ((drv == -1) || (drv == 1)) filters.append(";;Disk B (").append(getFilter(flags & (FT_DISK | FT_HOBETA))).append(")");
			if ((drv == -1) || (drv == 2)) filters.append(";;Disk C (").append(getFilter(flags & (FT_DISK | FT_HOBETA))).append(")");
			if ((drv == -1) || (drv == 3)) filters.append(";;Disk D (").append(getFilter(flags & (FT_DISK | FT_HOBETA))).append(")");
		}
		if (flags & FT_SNAP) filters.append(";;Snapshot (").append(getFilter(flags & FT_SNAP)).append(")");
		if (flags & FT_TAPE) filters.append(";;Tape (").append(getFilter(flags & FT_TAPE)).append(")");
		if (flags & FT_RZX) filters.append(";;RZX file (").append(getFilter(flags & FT_RZX)).append(")");
		if (filters.startsWith(";;")) filters.remove(0,2);
		filer->setWindowTitle("Open file");
		filer->setNameFilter(filters);
		filer->setDirectory(lastDir);
		filer->setAcceptMode(QFileDialog::AcceptOpen);
		if (!filer->exec()) return;
		filters = filer->selectedNameFilter();
		if (filters.contains("Disk A")) drv = 0;
		if (filters.contains("Disk B")) drv = 1;
		if (filters.contains("Disk C")) drv = 2;
		if (filters.contains("Disk D")) drv = 3;
		path = filer->selectedFiles().first();
		lastDir = filer->directory().absolutePath();
	}
	if (drv == -1) drv = 0;
	int type = getFileType(path);
	std::string sfnam(path.toUtf8().data());
	int err = ERR_OK;
	switch (type) {
		case FT_SNA: err = loadSNA(zx,sfnam.c_str()); break;
		case FT_Z80: err = loadZ80(zx,sfnam.c_str()); break;
		case FT_TAP: err = loadTAP(zx->tape,sfnam.c_str()); break;
		case FT_TZX: err = loadTZX(zx->tape,sfnam.c_str()); break;
		case FT_SCL: zx->bdi->flop[drv].load(sfnam,TYPE_SCL); break;
		case FT_TRD: zx->bdi->flop[drv].load(sfnam,TYPE_TRD); break;
		case FT_FDI: zx->bdi->flop[drv].load(sfnam,TYPE_FDI); break;
		case FT_UDI: zx->bdi->flop[drv].load(sfnam,TYPE_UDI); break;
		case FT_HOBETA: zx->bdi->flop[drv].load(sfnam,TYPE_HOBETA); break;
#if HAVEZLIB
		case FT_RZX: err = loadRZX(zx,sfnam.c_str()); break;
#endif
	}
	switch (err) {
		case ERR_CANT_OPEN: shithappens("Can't open file"); break;
		case ERR_RZX_SIGN: shithappens("Wrong RZX signature"); break;
		case ERR_RZX_CRYPT: shithappens("Xpeccy cannot into crypted RZX"); break;
		case ERR_RZX_UNPACK: shithappens("RZX unpack error"); break;
		case ERR_TZX_SIGN: shithappens("Wrong TZX signature"); break;
		case ERR_TZX_UNKNOWN: shithappens("Unknown TZX block"); break;
	}
}

bool saveFile(const char* name,int flags,int drv) {
	QString path(name);
	QString filters = "";
	if (flags & FT_DISK) {
		if (((drv == -1) || (drv == 0)) && (zx->bdi->flop[0].insert)) filters.append(";;Disk A (*.scl *.trd *.udi)");
		if ((drv == 1) && (zx->bdi->flop[1].insert)) filters.append(";;Disk B (*.scl *.trd *.udi)");
		if ((drv == 2) && (zx->bdi->flop[2].insert)) filters.append(";;Disk C (*.scl *.trd *.udi)");
		if ((drv == 3) && (zx->bdi->flop[3].insert)) filters.append(";;Disk D (*.scl *.trd *.udi)");
	}
	if (flags & FT_SNAP) filters.append(";;Snapshot (*.sna)");
	if ((flags & FT_TAPE) && (tapGet(zx->tape,TAPE_BLOCKS) != 0)) filters.append(";;Tape (*.tap)");
	if (filters.startsWith(";;")) filters.remove(0,2);
	filer->setWindowTitle("Save file");
	filer->setNameFilter(filters);
	filer->setAcceptMode(QFileDialog::AcceptSave);
	filer->setDirectory(lastDir);
	if (path != "") filer->selectFile(path);
	if (!filer->exec()) return false;
	filters = filer->selectedNameFilter();
	if (filters.contains("Disk A")) drv = 0;
	if (filters.contains("Disk B")) drv = 1;
	if (filters.contains("Disk C")) drv = 2;
	if (filters.contains("Disk D")) drv = 3;
	if (drv == -1) drv = 0;
	path = filer->selectedFiles().first();
	lastDir = filer->directory().absolutePath();
	std::string sfnam(path.toUtf8().data());
	int type = getFileType(path);
	if (filters.contains("Disk")) {
		switch (type) {
			case FT_SCL: zx->bdi->flop[drv].save(sfnam,TYPE_SCL); break;
			case FT_TRD: zx->bdi->flop[drv].save(sfnam,TYPE_TRD); break;
			case FT_UDI: zx->bdi->flop[drv].save(sfnam,TYPE_UDI); break;
			default: sfnam += ".trd"; zx->bdi->flop[drv].save(sfnam, TYPE_TRD); break;
		}
	}
	if (filters.contains("Tape")) {
		switch (type) {
			case FT_TAP: saveTAP(zx->tape,sfnam.c_str()); break;
			default: sfnam += ".tap"; saveTAP(zx->tape,sfnam.c_str()); break;
		}
	}
	if (filters.contains("Snap")) {
		bool mt = (zx->opt.hwName == "ZX48K");
		switch (type) {
			case FT_SNA: saveSNA(zx,sfnam.c_str(),mt); break;
			default: sfnam += ".sna"; saveSNA(zx,sfnam.c_str(),mt); break;
		}
	}
	return true;
}
