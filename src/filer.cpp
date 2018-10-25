#include "filer.h"
#include "xcore/xcore.h"
#include "xgui/xgui.h"

static QFileDialog* filer;

#include <QIcon>
#include <QMessageBox>

// new

typedef struct {
	int id;
	int ch;
	const char* ext;
	const char* filt;
	int(*load)(Computer*, const char*, int);
	int(*save)(Computer*, const char*, int);
	const char* name;
} xFileTypeInfo;

typedef struct {
	int id;
	int drv;
	const char* name;
	int child[32];
} xFileGroupInfo;

typedef struct {
	int id;
	int child[32];
} xFileHWInfo;

static xFileTypeInfo ft_tab[] = {
	{FL_SNA, 0, ".sna", "*.sna", loadSNA, saveSNA, "SNA snapshot"},
	{FL_Z80, 0, ".z80", "*.z80", loadZ80, NULL, "Z80 snapshot"},
	{FL_SPG, 0, ".spg", "*.spg", loadSPG, NULL, "SPG snapshot"},
	{FL_TAP, 0, ".tap", "*.tap", loadTAP, saveTAP, "TAP tape image"},
	{FL_TZX, 0, ".tzx", "*.tzx", loadTZX, NULL, "TZX tape image"},
	{FL_WAV, 0, ".wav", "*.wav", loadWAV, NULL, "WAV tape image"},
	{FL_SCL, 1, ".scl", "*.scl", loadSCL, saveSCL, "SCL disk image"},
	{FL_TRD, 1, ".trd", "*.trd", loadTRD, saveTRD, "TRD disk image"},
	{FL_TD0, 1, ".td0", "*.td0", loadTD0, NULL, "TD0 disk image"},
	{FL_FDI, 1, ".fdi", "*.fdi", loadFDI, NULL, "FDI disk image"},
	{FL_UDI, 1, ".udi", "*.udi", loadUDI, saveUDI, "UDI disk image"},
	{FL_DSK, 1, ".dsk", "*.dsk", loadDSK, NULL, "DSK disk image"},
	{FL_HOBETA, 0, ".$", "*.$?", loadHobeta, NULL, "Hobeta file"},
	{FL_GB, 0, ".gb", "*.gb", loadSlot, NULL, "GB cartrige"},
	{FL_GBC, 0, ".gbc", "*.gbc", loadSlot, NULL, "GBC cartrige"},
	{FL_MSX, 0, ".rom", "*.rom", loadSlot, NULL, "MSX cartrige"},
	{FL_MX1, 0, ".mx1", "*.mx1", loadSlot, NULL, "MSX1 cartrige"},
	{FL_MX2, 0, ".mx2", "*.mx2", loadSlot, NULL, "MSX2 cartrige"},
	{FL_NES, 0, ".nes", "*.nes", loadNes, NULL, "NES cartrige"},
//	{FL_T64, 0, ".t64", "*.t64", loadT64, NULL, "T64 tape image"},
	{FL_BKBIN, 0, ".bin", "*.bin", loadBIN, NULL, "BK bin data"},
#ifdef HAVEZLIB
	{FL_RZX, 0, ".rzx", "*.rzx", loadRZX, NULL, "RZX playback"},
#endif
	{FL_RAW, 0, NULL, "*.*", loadRaw, NULL, "RAW file"},
	{0, 0, NULL, NULL, NULL, NULL, NULL}
};

static xFileTypeInfo ft_raw = {FL_RAW, 0, NULL, NULL, loadRaw, NULL, "RAW file to disk A"};
static xFileTypeInfo ft_dum = {FL_NONE, 0, NULL, NULL, NULL, NULL, "Dummy entry"};

// 2nd parameter
// < 0 : allways on
// 0..3 : disk (must be inserted for save)
// 4 : tape (block count > 0)
static xFileGroupInfo fg_tab[] = {
	{FG_SNAPSHOT, -1, "Snapshot", {FL_SNA, FL_Z80, FL_SPG, 0}},
	{FG_TAPE, 4, "Tape", {FL_TAP, FL_TZX, FL_WAV, 0}},
	{FG_DISK_A, 0,"Disk A", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_DISK_B, 1, "Disk B", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_DISK_C, 2, "Disk C", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_DISK_D, 3, "Disk D", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_RAW, 0, "Raw file to disk A",{FL_RAW, 0}},
	{FG_RZX, -1, "RZX playback", {FL_RZX, 0}},
	{FG_GAMEBOY, -1, "GB cartrige", {FL_GB, FL_GBC, 0}},
	{FG_MSX, -1, "MSX cartrige", {FL_MSX, FL_MX1, FL_MX2, 0}},
	{FG_NES, -1, "NES cartrige", {FL_NES, 0}},
	{FG_CMDTAPE, -1, "Comodore tape", {FL_T64, 0}},
	{FG_BKDATA, -1, "BK bin data", {FL_BKBIN, 0}},
	{0, -1, NULL, {0}}
};

static xFileGroupInfo fg_dum = {0, -1, NULL, {0}};

static xFileHWInfo fh_tab[] = {
	{FH_SPECTRUM, {FG_SNAPSHOT, FG_TAPE, FG_DISK_A, FG_DISK_B, FG_DISK_C, FG_DISK_D, FG_RAW, FG_RZX, 0}},
	{FH_GAMEBOY, {FG_GAMEBOY, 0}},
	{FH_MSX, {FG_MSX, 0}},
	{FH_NES, {FG_NES, 0}},
	{FH_CMD, {FG_CMDTAPE, 0}},
	{FH_BK, {FG_BKDATA, 0}},
	{FH_DISKS, {FG_DISK_A, FG_DISK_B, FG_DISK_C, FG_DISK_D, 0}},
	{FH_SLOTS, {FG_GAMEBOY, FG_NES, FG_MSX, 0}},
	{0, {0}}
};

static xFileHWInfo hw_tab[] = {
	{FH_SPECTRUM, {HW_ATM1, HW_ATM2, HW_P1024, HW_PENT, HW_PENTEVO, HW_PHOENIX, HW_PLUS2, HW_PLUS3, HW_PROFI, HW_SCORP, HW_TSLAB, HW_ZX48, 0}},
	{FH_GAMEBOY, {HW_GBC, 0}},
	{FH_MSX, {HW_MSX, HW_MSX2, 0}},
	{FH_NES, {HW_NES, 0}},
	{FH_CMD, {HW_C64, 0}},
	{FH_BK, {HW_BK0010, 0}},
	{0, {0}}
};

int detect_hw_id(int hwid) {
	int i = 0;
	int x;
	int id = 0;
	while (hw_tab[i].id > 0) {
		x = 0;
		while (hw_tab[i].child[x] > 0) {
			if (hw_tab[i].child[x] == hwid)
				id = hw_tab[i].id;
			x++;
		}
		i++;
	}
	return id;
}

xFileHWInfo* file_find_hw(int id) {
	int i = 0;
	while ((fh_tab[i].id > 0) && (fh_tab[i].id != id))
		i++;
	return &fh_tab[i];
}

xFileGroupInfo* file_find_group(int id) {
	int i = 0;
	while ((fg_tab[i].id > 0) && (fg_tab[i].id != id))
		i++;
	return &fg_tab[i];
}

xFileGroupInfo* file_detect_grp(QString flt) {
	int i = 0;
	xFileGroupInfo* inf = &fg_dum;
	while (fg_tab[i].id > 0) {
		if (flt.startsWith(fg_tab[i].name, Qt::CaseInsensitive))
			inf = &fg_tab[i];
		i++;
	}
	return inf;
}

xFileTypeInfo* file_find_type(int id) {
	int i = 0;
	while ((ft_tab[i].id > 0) && (ft_tab[i].id != id))
		i++;
	return &ft_tab[i];
}

#include <QDebug>

xFileTypeInfo* file_ext_type(QString path) {
	int i = 0;
	QString ext = path.split(".").last();
	xFileTypeInfo* inf = &ft_dum;
	while (ft_tab[i].id > 0) {
		if (ft_tab[i].id == FL_HOBETA) {
			if (ext.startsWith("$"))
				inf = &ft_tab[i];
		} else if (ft_tab[i].ext == NULL) {
			// skip it
		} else if (path.endsWith(ft_tab[i].ext, Qt::CaseInsensitive)) {
			inf = &ft_tab[i];
		}
		i++;
	}
	return inf;
}

static QString allfilt;

QString file_get_type_filter(int id, int sv) {
	QString flt;
	xFileTypeInfo* inf = file_find_type(id);
	if (inf->filt && ((!sv && inf->load) || (sv && inf->save))) {
		flt = QString(inf->filt);
		if (!allfilt.contains(inf->filt) && inf->ext)
			allfilt.append(inf->filt).append(" ");
	}
	return flt;
}

QString file_get_group_filter(Computer* comp, int id, int sv) {
	QString flt;
	QString ftf;
	QString ofl;
	int i;
	xFileGroupInfo* inf = file_find_group(id);
	if (inf->id > 0) {
		if (!sv || (inf->drv < 0) || (sv && !(inf->drv & ~3) && comp->dif->fdc->flop[inf->drv & 3]->insert) || ((inf->drv == 4) && comp->tape->blkCount > 0)) {
			i = 0;
			while(inf->child[i] > 0) {
				ofl = file_get_type_filter(inf->child[i], sv);
				if (!ofl.isEmpty())
					ftf.append(ofl).append(" ");
				i++;
			}
			ftf = ftf.trimmed();
			if (!ftf.isEmpty())
				flt = QString("%0 (%1)").arg(inf->name, ftf);
		}
	}
	return flt;
}

QString file_get_hw_filter(Computer* comp, int id, int sv) {
	QStringList flt;
	QString gfl;
	allfilt.clear();
	int i;
	xFileHWInfo* inf = file_find_hw(id);
	if (inf->id > 0) {
		i = 0;
		while (inf->child[i] > 0) {
			gfl = file_get_group_filter(comp, inf->child[i], sv);
			if (!gfl.isEmpty())
				flt.append(gfl);
			i++;
		}
	}
	if (flt.size() > 1)
		flt.prepend(QString("All files (%0)").arg(allfilt));
	return flt.join(";;");
}

typedef struct {
	int err;
	const char* text;
} xFilerError;

static xFilerError err_tab[] = {
	{ERR_CANT_OPEN, "Can't open file"},
	{ERR_RZX_SIGN, "Wrong RZX signature"},
	{ERR_RZX_CRYPT, "Xpeccy cannot into crypted RZX"},
	{ERR_RZX_UNPACK, "RZX unpack error"},
	{ERR_TZX_SIGN, "Wrong TZX signature"},
	{ERR_TZX_UNKNOWN, "Unknown TZX block"},
	{ERR_TRD_LEN, "Incorrect TRD size"},
	{ERR_TRD_SIGN, "Not TRDOS disk"},
	{ERR_UDI_SIGN, "Wrong UDI signature"},
	{ERR_FDI_SIGN, "Wrong FDI signature"},
	{ERR_FDI_HEAD, "Wrong FDI heads count"},
	{ERR_HOB_CANT, "Can't create file at disk"},
	{ERR_SCL_SIGN, "Wrong SCL signature"},
	{ERR_SCL_MANY, "Too many files in SCL"},
	{ERR_RAW_LONG, "File is too big"},
	{ERR_DSK_SIGN, "Wrong DSK signature"},
	{ERR_TD0_SIGN, "Wrong TD0 signature"},
	{ERR_TD0_TYPE, "Unsupported TD0"},
	{ERR_TD0_VERSION, "Unsupported TD0 version"},
	{ERR_WAV_HEAD, "Wrong WAV header"},
	{ERR_WAV_FORMAT, "Unsupported WAV format"},
	{ERR_NES_HEAD, "Wrong NES header"},
	{ERR_NES_MAPPER, "Unsupported mapper"},
	{ERR_T64_SIGN, "Wrong T64 header"},
	{ERR_TRD_SNF, "Wrong disk structure for TRD file"},
	{ERR_OK, ""}
};

void file_errors(int err) {
	int i = 0;
	while ((err_tab[i].err != ERR_OK) && (err_tab[i].err != err))
		i++;
	if (err_tab[i].err != ERR_OK) {
		shitHappens(err_tab[i].text);
	}
}

int grp_by_disk(int drv) {
	int id = -1;
	switch (drv & 3) {
		case 0: id = FG_DISK_A; break;
		case 1: id = FG_DISK_B; break;
		case 2: id = FG_DISK_C; break;
		case 3: id = FG_DISK_D; break;
	}
	return id;
}

int load_file(Computer* comp, const char* name, int id, int drv) {
	QString path(name);
	QString flt;
	QString ext;
	xFileTypeInfo* inf;
	xFileGroupInfo* grp = &fg_dum;
	if (id == FG_DISK)
		id = grp_by_disk(id);
	if (id == FG_ALL)
		id = detect_hw_id(comp->hw->id);
	int err = ERR_OK;
	if (path.isEmpty()) {
		flt = file_get_hw_filter(comp, id, 0);
		if (flt.isEmpty()) {
			flt = file_get_group_filter(comp, id, 0);
			if (flt.isEmpty())
				flt = file_get_type_filter(id, 0);
		}
		if (!flt.isEmpty()) {
			filer->setWindowTitle("Open file");
			filer->setNameFilter(flt);
			filer->setDirectory(conf.path.lastDir);
			filer->setAcceptMode(QFileDialog::AcceptOpen);
			filer->setHistory(QStringList());
			if (filer->exec()) {
				path = filer->selectedFiles().first();
				flt = filer->selectedNameFilter();
				grp = file_detect_grp(flt);
				drv = grp->drv;
			}
			strcpy(conf.path.lastDir, filer->directory().absolutePath().toLocal8Bit().data());
		}
	}
	if (path.isEmpty()) return err;
	inf = file_ext_type(path);
	if (grp->id == FG_RAW)
		inf = &ft_raw;
	if (drv < 0) drv = 0;
	if (inf) {
		if (inf->load) {
			if (inf->ch) {
				if (saveChangedDisk(comp, drv)) {
					err = inf->load(comp, path.toLocal8Bit().data(), drv);
				} else {
					err = ERR_OK;
				}
			} else {
				err = inf->load(comp, path.toLocal8Bit().data(), drv);
			}
		}
	}
	file_errors(err);
	return err;
}

int save_file(Computer* comp, const char* name, int id, int drv) {
	QString path(name);
	QString flt;
	QString ext;
	xFileTypeInfo* inf;
	xFileGroupInfo* grp;
	if (id == FG_DISK)
		id = grp_by_disk(drv);
	if (id == FG_ALL)
		id = detect_hw_id(comp->hw->id);
	int err = ERR_OK;
	if (path.isEmpty()) {
		flt = file_get_hw_filter(comp, id, 1);
		if (flt.isEmpty()) {
			flt = file_get_group_filter(comp, id, 1);
			if (flt.isEmpty())
				flt = file_get_type_filter(id, 1);
		}
		if (!flt.isEmpty()) {
			filer->setWindowTitle("Save file");
			filer->setNameFilter(flt);
			filer->setAcceptMode(QFileDialog::AcceptSave);
			filer->setDirectory(conf.path.lastDir);
			filer->setHistory(QStringList());
			if (filer->exec()) {
				path = filer->selectedFiles().first();
				flt = filer->selectedFilter();
				grp = file_detect_grp(flt);
				if (grp)
					drv = grp->drv;
			}
			strcpy(conf.path.lastDir, filer->directory().absolutePath().toLocal8Bit().data());
		}
	}
	if (path.isEmpty()) return err;
	inf = file_ext_type(path);
	if (inf) {
		if (inf->save)
			err = inf->save(comp, path.toLocal8Bit().data(), drv);
	}
	file_errors(err);
	return err;
}

// old

void initFileDialog(QWidget* par) {
	filer = new QFileDialog(par);
	filer->setWindowIcon(QIcon(":/images/logo.png"));
	filer->setWindowModality(Qt::ApplicationModal);
	filer->setNameFilterDetailsVisible(true);
	filer->setConfirmOverwrite(true);
	filer->setOption(QFileDialog::DontUseNativeDialog,0);
#ifdef _WIN32
	strcpy(conf.path.lastDir, ".");
#else
	strcpy(conf.path.lastDir, getenv("HOME"));
#endif
}

bool saveChangedDisk(Computer* comp,int id) {
	bool res=true;
	id &= 3;
	Floppy* flp = comp->dif->fdc->flop[id];
	if (flp->changed) {
		QMessageBox mbox;
		mbox.setText(QString("<b>Disk %0: has been changed</b>").arg(QChar('A'+id)));
		mbox.setInformativeText("Do you want to save it?");
		mbox.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
		mbox.setIcon(QMessageBox::Warning);
		switch (mbox.exec()) {
			case QMessageBox::Yes: res = save_file(comp, flp->path, FG_DISK, id); break;	// save
			case QMessageBox::No: res=true; break;						// don't save
			case QMessageBox::Cancel: res=false; break;					// cancel
		}
	}
	return res;
}

/*
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
	if (flags & FT_SLOT_A) res.append(" *.rom *.mx1 *.mx2 *.gb *.gbc");
	if (flags & FT_NES) res.append(" *.nes");
	if (flags & FT_T64) res.append(" *.t64");
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
	if (path.endsWith(".rom",Qt::CaseInsensitive)) return FT_SLOT_A;
	if (path.endsWith(".mx1",Qt::CaseInsensitive)) return FT_SLOT_A;
	if (path.endsWith(".mx2",Qt::CaseInsensitive)) return FT_SLOT_A;
	if (path.endsWith(".gb",Qt::CaseInsensitive)) return FT_SLOT_A;
	if (path.endsWith(".gbc",Qt::CaseInsensitive)) return FT_SLOT_A;
	if (path.endsWith(".nes",Qt::CaseInsensitive)) return FT_NES;
	if (path.endsWith(".t64",Qt::CaseInsensitive)) return FT_T64;
#ifdef HAVEZLIB
	if (path.endsWith(".rzx",Qt::CaseInsensitive)) return FT_RZX;
#endif
	QStringList pspl = path.split(".");
	if (pspl.size() > 0) {
		if (pspl.last().startsWith("$")) return FT_HOBETA;
	}
	return FT_NONE;
}
*/

int testSlotOn(Computer* comp) {
	int res = 0;
	for (int i = 0; i < 256; i++) {
		if (comp->mem->map[i].type == MEM_SLOT) {
			res = 1;
			i = 256;
		}
	}
	return res;
}

/*
void loadFile(Computer* comp,const char* name, int flags, int drv) {
	QString opath = QDialog::trUtf8(name);
	filer->setDirectory(conf.path.lastDir);
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
		if (flags & FT_SLOT_A) filters.append(QString(";;Cartrige data (%0)").arg(getFilter(flags & FT_SLOT_A)));
		if (flags & FT_NES) filters.append(QString(";;NES cartrige (%0)").arg(getFilter(flags & FT_NES)));
		if (flags & FT_SPG) filters.append(";;SPG file (*.spg)");
#ifdef HAVEZLIB
		if (flags & FT_RZX) filters.append(";;RZX file (").append(getFilter(flags & FT_RZX)).append(")");
#endif
		if (flags & FT_RAW) filters.append(";;Raw file to disk A (*.*)");
		if (flags & FT_T64) filters.append(";;C64 tape file (*.t64)");
		if (filters.startsWith(";;")) filters.remove(0,2);
		filer->setWindowTitle("Open file");
		filer->setNameFilter(filters);
		filer->setDirectory(conf.path.lastDir);
		filer->setAcceptMode(QFileDialog::AcceptOpen);
		if (!filer->exec()) return;
		filters = filer->selectedNameFilter();
		if (filters.contains("Disk A")) drv = 0;
		if (filters.contains("Disk B")) drv = 1;
		if (filters.contains("Disk C")) drv = 2;
		if (filters.contains("Disk D")) drv = 3;
//		if (filters.contains("Cartrige slot A")) drv = 0;
		if (filters.contains("Raw")) drv = 10;
		opath = filer->selectedFiles().first();
		strcpy(conf.path.lastDir, filer->directory().absolutePath().toLocal8Bit().data());
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
	rzxStop(comp);
//	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	// xCartridge* slot = drv ? &comp->msx.slotB : &comp->msx.slotA;
	switch (type) {
		case FT_SNA: ferr = loadSNA(comp,sfnam.c_str(), -1); break;
		case FT_Z80: ferr = loadZ80(comp,sfnam.c_str(), -1); break;
		case FT_TAP: ferr = loadTAP(comp,sfnam.c_str(),-1); break;
		case FT_TZX: ferr = loadTZX(comp,sfnam.c_str(),-1); break;
		case FT_WAV: ferr = loadWAV(comp,sfnam.c_str(),-1); break;
		case FT_SCL: if (saveChangedDisk(comp,drv)) {ferr = loadSCL(comp, sfnam.c_str(), drv);} break;
		case FT_TRD: if (saveChangedDisk(comp,drv)) {ferr = loadTRD(comp, sfnam.c_str(), drv);} break;
		case FT_FDI: if (saveChangedDisk(comp,drv)) {ferr = loadFDI(comp, sfnam.c_str(), drv);} break;
		case FT_UDI: if (saveChangedDisk(comp,drv)) {ferr = loadUDI(comp, sfnam.c_str(), drv);} break;
		case FT_HOBETA: ferr = loadHobeta(comp, sfnam.c_str(), drv); break;
		case FT_RAW: ferr = loadRaw(comp, sfnam.c_str(), 0); break;
		case FT_DSK: ferr = loadDSK(comp, sfnam.c_str(), drv); break;
		case FT_TD0: ferr = loadTD0(comp, sfnam.c_str(), drv); break;
		case FT_SPG: ferr = loadSPG(comp, sfnam.c_str(),-1); break;
		case FT_SLOT_A: ferr = loadSlot(comp, sfnam.c_str(),-1); break;
		case FT_NES: ferr = loadNes(comp, sfnam.c_str(), -1); break;
		case FT_T64: ferr = loadT64(comp,sfnam.c_str(), -1); break;
#ifdef HAVEZLIB
		case FT_RZX: ferr = loadRZX(comp, sfnam.c_str(), -1); break;
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
		case ERR_NES_HEAD: shitHappens("Wrong NES header"); break;
		case ERR_NES_MAPPER: shitHappens("Unsupported mapper"); break;
		case ERR_T64_SIGN: shitHappens("Wrong T64 header"); break;
		case ERR_OK:
			if (type & FT_DISK) loadBoot(comp, conf.path.boot, drv);
			if ((type & FT_SLOT) && testSlotOn(comp)) compReset(comp, RES_DEFAULT);
			break;
	}
}

bool saveFile(Computer* comp,const char* name,int flags,int drv) {
	QString path = QDialog::trUtf8(name);
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
	filer->setDirectory(conf.path.lastDir);
	if (path != "") filer->selectFile(path);
	if (!filer->exec()) return false;
	filters = filer->selectedNameFilter();
	if (filters.contains("Disk A")) drv = 0;
	if (filters.contains("Disk B")) drv = 1;
	if (filters.contains("Disk C")) drv = 2;
	if (filters.contains("Disk D")) drv = 3;
	if (drv == -1) drv = 0;
	path = filer->selectedFiles().first();
	strcpy(conf.path.lastDir, filer->directory().absolutePath().toLocal8Bit().data());
	std::string sfnam(path.toUtf8().data());
	int type = getFileType(path);
	int err = ERR_OK;
	if (filters.contains("Disk")) {
//		Floppy* flp = comp->dif->fdc->flop[drv];
		switch (type) {
			case FT_SCL: err = saveSCL(comp, sfnam.c_str(), drv); break;
			case FT_TRD: err = saveTRD(comp, sfnam.c_str(), drv); break;
			case FT_UDI: err = saveUDI(comp, sfnam.c_str(), drv); break;
			default: sfnam += ".trd"; err = saveTRD(comp, sfnam.c_str(), drv); break;
		}
	}
	if (filters.contains("Tape")) {
		switch (type) {
			case FT_TAP: err = saveTAP(comp, sfnam.c_str(), -1); break;
			default: sfnam += ".tap"; err = saveTAP(comp, sfnam.c_str(), -1); break;
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
*/
