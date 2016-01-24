#include "filer.h"
#include "xcore/xcore.h"
#include "xgui/xgui.h"

QFileDialog *filer;
QDir lastDir;

#include <QIcon>
#include <QMessageBox>

void initFileDialog(QWidget* par) {
	filer = new QFileDialog(par);
	filer->setWindowIcon(QIcon(":/images/logo.png"));
	filer->setWindowModality(Qt::ApplicationModal);
	filer->setNameFilterDetailsVisible(true);
	filer->setConfirmOverwrite(true);
	filer->setOptions(QFileDialog::DontUseNativeDialog);
#ifdef _WIN32
	lastDir = ".";
#else
	lastDir = QDir::home();
#endif
}

bool saveChangedDisk(Computer* comp,int id) {
	bool res=true;
	Floppy* flp = comp->dif->fdc->flop[id];
	if (flp->changed) {
		QMessageBox mbox;
		mbox.setText(QString("<b>Disk ").append(QChar('A'+id)).append(": has been changed</b>"));
		mbox.setInformativeText("Do you want to save it?");
		mbox.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
		mbox.setIcon(QMessageBox::Warning);
		switch (mbox.exec()) {
			case QMessageBox::Yes: res = saveFile(comp,flp->path,FT_DISK,id); break;		// save
			case QMessageBox::No: res=true; break;						// don't save
			case QMessageBox::Cancel: res=false; break;					// cancel
		}
	}
	return res;
}

QString getFilter(int flags) {
	QString res = "";
	if (flags & FT_SNA) res.append(" *.sna");
	if (flags & FT_Z80) res.append(" *.z80");
	if (flags & FT_TAP) res.append(" *.tap");
	if (flags & FT_TZX) res.append(" *.tzx");
	if (flags & FT_WAV) res.append(" *.wav");
	if (flags & FT_SCL) res.append(" *.scl");
	if (flags & FT_TRD) res.append(" *.trd");
	if (flags & FT_FDI) res.append(" *.fdi");
	if (flags & FT_UDI) res.append(" *.udi");
	if (flags & FT_DSK) res.append(" *.dsk");
	if (flags & FT_TD0) res.append(" *.td0");
#ifdef HAVEZLIB
	if (flags & FT_RZX) res.append(" *.rzx");
#endif
	if (flags & FT_SPG) res.append(" *.spg");
	if (flags & FT_HOBETA) res.append(" *.$?");
	if (flags & FT_SLOT) res.append(" *.rom");
	if (res.startsWith(" ")) res.remove(0,1);
	return res;
}

int getFileType(QString path) {
	if (path.endsWith(".sna",Qt::CaseInsensitive)) return FT_SNA;
	if (path.endsWith(".z80",Qt::CaseInsensitive)) return FT_Z80;
	if (path.endsWith(".tap",Qt::CaseInsensitive)) return FT_TAP;
	if (path.endsWith(".tzx",Qt::CaseInsensitive)) return FT_TZX;
	if (path.endsWith(".wav",Qt::CaseInsensitive)) return FT_WAV;
	if (path.endsWith(".scl",Qt::CaseInsensitive)) return FT_SCL;
	if (path.endsWith(".trd",Qt::CaseInsensitive)) return FT_TRD;
	if (path.endsWith(".fdi",Qt::CaseInsensitive)) return FT_FDI;
	if (path.endsWith(".udi",Qt::CaseInsensitive)) return FT_UDI;
	if (path.endsWith(".dsk",Qt::CaseInsensitive)) return FT_DSK;
	if (path.endsWith(".td0",Qt::CaseInsensitive)) return FT_TD0;
	if (path.endsWith(".spg",Qt::CaseInsensitive)) return FT_SPG;
	if (path.endsWith(".rom",Qt::CaseInsensitive)) return FT_SLOT;
#ifdef HAVEZLIB
	if (path.endsWith(".rzx",Qt::CaseInsensitive)) return FT_RZX;
#endif
	QStringList pspl = path.split(".");
	if (pspl.size() > 0) {
		if (pspl.last().startsWith("$")) return FT_HOBETA;
	}
	return FT_NONE;
}

int testSlotOn(Computer* comp) {
	if (comp->hw->type != HW_MSX) return 0;
	if (comp->mem->map[0].type == MEM_EXT) return 1;
	if (comp->mem->map[1].type == MEM_EXT) return 1;
	if (comp->mem->map[2].type == MEM_EXT) return 1;
	if (comp->mem->map[3].type == MEM_EXT) return 1;
	return 0;
}

void loadFile(Computer* comp,const char* name, int flags, int drv) {
	QString opath = QDialog::trUtf8(name);
	filer->setDirectory(lastDir);
	if (opath == "") {
		QString filters = "";
		if (flags == FT_ALL) filters = QString("All known types (%0)").arg(getFilter(flags));
		if (flags & FT_DISK) {
			if ((drv == -1) || (drv == 0)) filters.append(QString(";;Disk A (%0)").arg(getFilter(flags & (FT_DISK | FT_HOBETA))));
			if ((drv == -1) || (drv == 1)) filters.append(QString(";;Disk B (%0)").arg(getFilter(flags & (FT_DISK | FT_HOBETA))));
			if ((drv == -1) || (drv == 2)) filters.append(QString(";;Disk C (%0)").arg(getFilter(flags & (FT_DISK | FT_HOBETA))));
			if ((drv == -1) || (drv == 3)) filters.append(QString(";;Disk D (%0)").arg(getFilter(flags & (FT_DISK | FT_HOBETA))));
		}
		if (flags & FT_SNAP) filters.append(QString(";;Snapshot (%0)").arg(getFilter(flags & FT_SNAP)));
		if (flags & FT_TAPE) filters.append(QString(";;Tape (%0)").arg(getFilter(flags & FT_TAPE)));
		if (flags & FT_SLOT_A) filters.append(QString(";;MSX slot A (%0)").arg(getFilter(flags & FT_SLOT)));
		if (flags & FT_SLOT_B) filters.append(QString(";;MSX slot B (%0)").arg(getFilter(flags & FT_SLOT)));
		if (flags & FT_SPG) filters.append(";;SPG file (*.spg)");
#ifdef HAVEZLIB
		if (flags & FT_RZX) filters.append(";;RZX file (").append(getFilter(flags & FT_RZX)).append(")");
#endif
		if (flags & FT_RAW) filters.append(";;Raw file to disk A (*.*)");
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
		if (filters.contains("MSX slot A")) drv = 0;
		if (filters.contains("MSX slot B")) drv = 1;
		if (filters.contains("Raw")) drv = 10;
		opath = filer->selectedFiles().first();
		lastDir = filer->directory().absolutePath();
	}
	if (drv == -1) drv = 0;
	int type;
	if (drv == 10) {
		type = FT_RAW;
		drv = 0;
	} else {
		type = getFileType(opath);
	}
	if (!QFile::exists(opath)) return;
	std::string sfnam(opath.toLocal8Bit().data());
	int ferr = ERR_OK;
	comp->rzxPlay = false;
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	xCartridge* slot = drv ? &comp->msx.slotB : &comp->msx.slotA;
	switch (type) {
		case FT_SNA: ferr = loadSNA(comp,sfnam.c_str()); break;
		case FT_Z80: ferr = loadZ80(comp,sfnam.c_str()); break;
		case FT_TAP: ferr = loadTAP(comp->tape,sfnam.c_str()); break;
		case FT_TZX: ferr = loadTZX(comp->tape,sfnam.c_str()); break;
		case FT_WAV: ferr = loadWAV(comp->tape,sfnam.c_str()); break;
		case FT_SCL: if (saveChangedDisk(comp,drv)) {ferr = loadSCL(flp,sfnam.c_str());} break;
		case FT_TRD: if (saveChangedDisk(comp,drv)) {ferr = loadTRD(flp,sfnam.c_str());} break;
		case FT_FDI: if (saveChangedDisk(comp,drv)) {ferr = loadFDI(flp,sfnam.c_str());} break;
		case FT_UDI: if (saveChangedDisk(comp,drv)) {ferr = loadUDI(flp,sfnam.c_str());} break;
		case FT_HOBETA: ferr = loadHobeta(flp,sfnam.c_str()); break;
		case FT_RAW: ferr = loadRaw(flp,sfnam.c_str()); break;
		case FT_DSK: ferr = loadDSK(flp,sfnam.c_str()); break;
		case FT_TD0: ferr = loadTD0(flp,sfnam.c_str()); break;
		case FT_SPG: ferr = loadSPG(comp,sfnam.c_str()); break;
		case FT_SLOT: ferr = loadSlot(slot,sfnam.c_str()); break;
#ifdef HAVEZLIB
		case FT_RZX: ferr = loadRZX(comp,sfnam.c_str()); break;
#endif
	}
	switch (ferr) {
		case ERR_CANT_OPEN: shitHappens("Can't open file"); break;
		case ERR_RZX_SIGN: shitHappens("Wrong RZX signature"); break;
		case ERR_RZX_CRYPT: shitHappens("Xpeccy cannot into crypted RZX"); break;
		case ERR_RZX_UNPACK: shitHappens("RZX unpack error"); break;
		case ERR_TZX_SIGN: shitHappens("Wrong TZX signature"); break;
		case ERR_TZX_UNKNOWN: shitHappens("Unknown TZX block"); break;
		case ERR_TRD_LEN: shitHappens("Incorrect TRD size"); break;
		case ERR_TRD_SIGN: shitHappens("Not TRDOS disk"); break;
		case ERR_UDI_SIGN: shitHappens("Wrong UDI signature"); break;
		case ERR_FDI_SIGN: shitHappens("Wrong FDI signature"); break;
		case ERR_FDI_HEAD: shitHappens("Wrong FDI heads count"); break;
		case ERR_HOB_CANT: shitHappens("Can't create file at disk"); break;
		case ERR_SCL_SIGN: shitHappens("Wrong SCL signature"); break;
		case ERR_SCL_MANY: shitHappens("Too many files in SCL"); break;
		case ERR_RAW_LONG: shitHappens("File is too big"); break;
		case ERR_DSK_SIGN: shitHappens("Wrong DSK signature"); break;
		case ERR_TD0_SIGN: shitHappens("Wrong TD0 signature"); break;
		case ERR_TD0_TYPE: shitHappens("Unsupported TD0"); break;
		case ERR_TD0_VERSION: shitHappens("Unsupported TD0 version"); break;
		case ERR_WAV_HEAD: shitHappens("Wrong WAV header"); break;
		case ERR_WAV_FORMAT: shitHappens("Unsupported WAV format"); break;
		case ERR_OK:
			if (type & FT_DISK) loadBoot(flp, conf.path.boot.c_str());
			if ((type & FT_SLOT) && testSlotOn(comp)) compReset(comp, RES_DEFAULT);
			break;
	}
}

bool saveFile(Computer* comp,const char* name,int flags,int drv) {
	QString path(name);
	QString filters = "";
	if (flags & FT_DISK) {
		if (((drv == -1) || (drv == 0)) && (comp->dif->fdc->flop[0]->insert)) filters.append(";;Disk A (*.scl *.trd *.udi)");
		if ((drv == 1) && (comp->dif->fdc->flop[1]->insert)) filters.append(";;Disk B (*.scl *.trd *.udi)");
		if ((drv == 2) && (comp->dif->fdc->flop[2]->insert)) filters.append(";;Disk C (*.scl *.trd *.udi)");
		if ((drv == 3) && (comp->dif->fdc->flop[3]->insert)) filters.append(";;Disk D (*.scl *.trd *.udi)");
	}
	if (flags & FT_SNAP) filters.append(";;Snapshot (*.sna)");
	if ((flags & FT_TAPE) && (comp->tape->blkCount != 0)) filters.append(";;Tape (*.tap)");
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
	int err = ERR_OK;
	if (filters.contains("Disk")) {
		Floppy* flp = comp->dif->fdc->flop[drv];
		switch (type) {
			case FT_SCL: err = saveSCL(flp,sfnam.c_str()); break;
			case FT_TRD: err = saveTRD(flp,sfnam.c_str()); break;
			case FT_UDI: err = saveUDI(flp,sfnam.c_str()); break;
			default: sfnam += ".trd"; err = saveTRD(flp,sfnam.c_str()); break;
		}
	}
	if (filters.contains("Tape")) {
		switch (type) {
			case FT_TAP: err = saveTAP(comp->tape,sfnam.c_str()); break;
			default: sfnam += ".tap"; err = saveTAP(comp->tape,sfnam.c_str()); break;
		}
	}
	if (filters.contains("Snap")) {
		bool mt = (conf.prof.cur->hwName == "ZX48K");
		switch (type) {
			case FT_SNA:
				err = saveSNA(comp,sfnam.c_str(),mt);
				break;
			default:
				sfnam += ".sna";
				err = saveSNA(comp,sfnam.c_str(),mt);
				break;
		}
	}
	switch (err) {
		case ERR_CANT_OPEN: shitHappens("Can't open file"); break;
		case ERR_TRD_SNF: shitHappens("Wrong disk structure for TRD file"); break;
	}
	return true;
}
