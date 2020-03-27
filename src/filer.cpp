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
	const char* defext;
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
	{FL_WAV, 0, ".wav", "*.wav", loadWAV, saveWAV, "WAV tape image"},
	{FL_SCL, 1, ".scl", "*.scl", loadSCL, saveSCL, "SCL disk image"},
	{FL_TRD, 1, ".trd", "*.trd", loadTRD, saveTRD, "TRD disk image"},
	{FL_TD0, 1, ".td0", "*.td0", loadTD0, NULL, "TD0 disk image"},
	{FL_FDI, 1, ".fdi", "*.fdi", loadFDI, NULL, "FDI disk image"},
	{FL_UDI, 1, ".udi", "*.udi", loadUDI, saveUDI, "UDI disk image"},
	{FL_DSK, 1, ".dsk", "*.dsk", loadDSK, saveDSK, "DSK disk image"},
	{FL_HOBETA, 0, ".$", "*.$?", loadHobeta, NULL, "Hobeta file"},
	{FL_GB, 0, ".gb", "*.gb", loadSlot, NULL, "GB cartrige"},
	{FL_GBC, 0, ".gbc", "*.gbc", loadSlot, NULL, "GBC cartrige"},
	{FL_MSX, 0, ".rom", "*.rom", loadSlot, NULL, "MSX cartrige"},
	{FL_MX1, 0, ".mx1", "*.mx1", loadSlot, NULL, "MSX1 cartrige"},
	{FL_MX2, 0, ".mx2", "*.mx2", loadSlot, NULL, "MSX2 cartrige"},
	{FL_CAS, 0, ".cas", "*.cas", loadCAS, NULL, "MSX cassette"},
	{FL_NES, 0, ".nes", "*.nes", loadNes, NULL, "NES cartrige"},
//	{FL_T64, 0, ".t64", "*.t64", loadT64, NULL, "T64 tape image"},
	{FL_C64TAP, 0, ".tap", "*.tap", loadC64RawTap, NULL, "C64 raw tape image"},
	{FL_C64PRG, 0, ".prg", "*.prg", loadC64prg, NULL, "C64 PRG snapshot"},
	{FL_BKBIN, 0, ".bin", "*.bin", loadBIN, NULL, "BK bin data"},
	{FL_BKIMG, 0, ".img", "*.img", loadBkIMG, NULL, "BK disk image"},
//	{FL_BKBKD, 0, ".bkd", "*.bkd", loadBkIMG, NULL, "BK disk image"},
#ifdef HAVEZLIB
	{FL_RZX, 0, ".rzx", "*.rzx", loadRZX, NULL, "RZX playback"},
#endif
	{FL_RAW, 0, NULL, "*", loadRaw, NULL, "RAW file"},			// * for all files; *.* for all files that have extension
	{0, 0, NULL, NULL, NULL, NULL, NULL}
};

static xFileTypeInfo ft_raw = {FL_RAW, 0, NULL, NULL, loadRaw, NULL, "RAW file to disk A"};
static xFileTypeInfo ft_dum = {FL_NONE, 0, NULL, NULL, NULL, NULL, "Dummy entry"};

// 2nd parameter
// < 0 : allways on
// 0..3 : disk (must be inserted for save)
// 4 : tape (block count > 0)
static xFileGroupInfo fg_tab[] = {
	{FG_SNAPSHOT, ".sna", -1, "Snapshot", {FL_SNA, FL_Z80, FL_SPG, 0}},
	{FG_TAPE, ".tap", 4, "Tape", {FL_TAP, FL_TZX, FL_WAV, 0}},
	{FG_DISK_A, ".trd", 0,"Disk A", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_DISK_B, ".trd", 1, "Disk B", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_DISK_C, ".trd", 2, "Disk C", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_DISK_D, ".trd", 3, "Disk D", {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, FL_DSK, FL_HOBETA, 0}},
	{FG_RAW, "", 0, "Raw file to disk A",{FL_RAW, 0}},
	{FG_RZX, "", -1, "RZX playback", {FL_RZX, 0}},
	{FG_GAMEBOY, "", -1, "GB cartrige", {FL_GB, FL_GBC, 0}},
	{FG_MSX, "", -1, "MSX cartrige", {FL_MSX, FL_MX1, FL_MX2, 0}},
	{FG_MSXTAPE, "", 4, "MSX cassette", {FL_CAS, 0}},
	{FG_NES, "", -1, "NES cartrige", {FL_NES, 0}},
	{FG_CMDTAPE, "", -1, "Commodore tape", {FL_T64, FL_C64TAP, 0}},
	{FG_CMDSNAP, "", -1, "Commodore snapshot", {FL_C64PRG, 0}},
	{FG_BKDATA, "", -1, "BK bin data", {FL_BKBIN, 0}},
	{FG_BKDISK, "", 0, "BK disk image", {FL_BKIMG, FL_BKBKD, FL_UDI, 0}},
	{0, "", -1, NULL, {0}}
};

static xFileGroupInfo fg_dum = {0, "", -1, NULL, {0}};

static xFileHWInfo fh_tab[] = {
	{FH_SPECTRUM, {FG_SNAPSHOT, FG_TAPE, FG_DISK_A, FG_DISK_B, FG_DISK_C, FG_DISK_D, FG_RAW, FG_RZX, 0}},
	{FH_GAMEBOY, {FG_GAMEBOY, 0}},
	{FH_MSX, {FG_MSX, FG_MSXTAPE, 0}},
	{FH_NES, {FG_NES, 0}},
	{FH_CMD, {FG_CMDTAPE, FG_CMDSNAP, 0}},
	{FH_BK, {FG_BKDATA, FG_BKDISK, 0}},
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
	{FH_BK, {HW_BK0010, HW_BK0011M, 0}},
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

xFileTypeInfo* file_detect_type(QString flt) {
	int i = 0;
	xFileTypeInfo* inf = &ft_dum;
	while (ft_tab[i].id > 0) {
		if (flt.startsWith(ft_tab[i].name, Qt::CaseInsensitive)) {
			inf = &ft_tab[i];
		}
		i++;
	}
	return inf;
}

xFileTypeInfo* file_find_hw_ext(int hw, QString path) {
	xFileTypeInfo* inf = NULL;
	xFileHWInfo* hwinf;
	xFileGroupInfo* fginf;
	xFileTypeInfo* ftinf;
	QString ext = path.split(".").last();
	int ig;
	int iz;
	hw = detect_hw_id(hw);
	if (hw != 0) {
		hwinf = file_find_hw(hw);
		ig = 0;
		while ((hwinf->child[ig] != 0) && (inf == NULL)) {
			fginf = file_find_group(hwinf->child[ig]);
			iz = 0;
			while ((fginf->child[iz] != 0) && (inf == NULL)) {
				ftinf = file_find_type(fginf->child[iz]);
				if (ftinf->id != 0) {
					if (ftinf->id == FL_HOBETA) {
						if (ext.startsWith("$"))
							inf = ftinf;
					} else if (ftinf->ext == NULL) {

					} else if (path.endsWith(ftinf->ext, Qt::CaseInsensitive)) {
						inf = ftinf;
					}
				}
				iz++;
			}
			ig++;
		}
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
	if ((flt.size() > 1) && !sv)
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
	{ERR_C64T_SIGN, "Wrong C64 raw tape header"},
	{ERR_TRD_SNF, "Wrong disk structure for TRD file"},
	{ERR_CAS_EOF, "CAS: unexpected end of file"},
	{ERR_CAS_SIGN, "CAS: wrong block signature"},
	{ERR_CAS_TYPE, "CAS: wrong block type"},
	{ERR_OK, ""}
};

void file_errors(int err) {
	int i = 0;
	char buf[1024];
	while ((err_tab[i].err != ERR_OK) && (err_tab[i].err != err))
		i++;
	if (err_tab[i].err != ERR_OK) {
		shitHappens(err_tab[i].text);
	} else if (err != ERR_OK) {
		sprintf(buf, "Error #%i", err);
	}
}

static int disk_id[] = {FG_DISK_A, FG_DISK_B, FG_DISK_C, FG_DISK_D};
static int boot_ft[] = {FL_SCL, FL_TRD, FL_TD0, FL_FDI, FL_UDI, 0};

void disk_boot(Computer* comp, int drv, int id) {
	int idx = 0;
	while (boot_ft[idx] && (boot_ft[idx] != id))
		idx++;
	if (boot_ft[idx])
		loadBoot(comp, conf.path.boot, drv);
}

int load_file(Computer* comp, const char* name, int id, int drv) {
	QString path = QDialog::trUtf8(name);
	QString flt;
	QString ext;
	xFileTypeInfo* inf;
	xFileGroupInfo* grp = &fg_dum;
	if (id == FG_DISK)
		id = disk_id[drv & 3];
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
			filer->setDirectory(conf.prof.cur->lastDir.c_str());
			filer->setAcceptMode(QFileDialog::AcceptOpen);
			filer->setHistory(QStringList());
			if (filer->exec()) {
				path = filer->selectedFiles().first();
				flt = filer->selectedNameFilter();
				grp = file_detect_grp(flt);
				drv = grp->drv;
				conf.prof.cur->lastDir = std::string(QFileInfo(path).dir().absolutePath().toLocal8Bit().data());
			}
		}
	}
	if (path.isEmpty()) return err;
	inf = file_find_hw_ext(comp->hw->id, path);
	switch(grp->id) {
		case FG_RAW: inf = &ft_raw; break;
	}
	if (drv < 0) drv = 0;
	if (inf) {
		if (inf->load) {
			if (inf->ch) {
				if (saveChangedDisk(comp, drv) == ERR_OK) {
					err = inf->load(comp, path.toLocal8Bit().data(), drv);
					disk_boot(comp, drv, inf->id);
				} else {
					err = ERR_OK;
				}
			} else {
				err = inf->load(comp, path.toLocal8Bit().data(), drv);
				disk_boot(comp, drv, inf->id);
			}
		}
	}
	file_errors(err);
	return err;
}

int save_file(Computer* comp, const char* name, int id, int drv) {
	QString path = QDialog::trUtf8(name);
	QString flt;
	QString ext;
	xFileTypeInfo* inf = NULL;
	xFileTypeInfo* tin;
	xFileGroupInfo* grp;
	int i;
	int flg;
	if (id == FG_DISK)
		id = disk_id[drv & 3];
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
			filer->setDirectory(conf.prof.cur->lastDir.c_str());
			filer->setHistory(QStringList());
			if (filer->exec()) {
				path = filer->selectedFiles().first();
				flt = filer->selectedNameFilter();
				grp = file_detect_grp(flt);
				if (grp->id != FL_NONE) {
					drv = grp->drv;
					i = 0;
					flg = 1;
					// scan group file types and check if path extension is the same
					while ((grp->child[i] != FL_NONE) && flg) {
						tin = file_find_type(grp->child[i]);
						if (tin) {
							if (path.endsWith(tin->ext, Qt::CaseInsensitive)) {
								flg = 0;
								inf = tin;
							}
						}
						i++;
					}
					// if no filetypes found, add default extension
					if (flg) {
						path.append(grp->defext);
					}
				} else {
					tin = file_detect_type(flt);
					if (tin->id != FL_NONE) {
						path.append(tin->ext);
					}
				}
				conf.prof.cur->lastDir = std::string(QFileInfo(path).dir().absolutePath().toLocal8Bit().data());
			}
		}
	}
	if (path.isEmpty()) return err;
	if (drv < 0)
		drv = 0;
	if (!inf)
		// inf = file_ext_type(path);
		inf = file_find_hw_ext(comp->hw->id, path);
	if (inf) {
		printf("filetype: %s\n", inf->name);
		if (inf->save) {
			err = inf->save(comp, path.toLocal8Bit().data(), drv);
		} else {
			shitHappens("Can't save that");
		}
	} else {
		shitHappens("Don't know such extension");
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
	filer->setOption(QFileDialog::DontUseNativeDialog, 1);
}

int saveChangedDisk(Computer* comp,int id) {
	int res = ERR_OK;
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
			case QMessageBox::No: res = ERR_OK; break;						// don't save
			case QMessageBox::Cancel: res = ERR_CANCEL; break;					// cancel
		}
	}
	return res;
}

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
