#ifndef X_FILER_H
#define X_FILER_H

#include "libxpeccy/filetypes/filetypes.h"
#include "libxpeccy/spectrum.h"
#include <QFileDialog>

enum {
	FL_NONE = 0,
	FL_TAP,
	FL_TZX,
	FL_WAV,
	FL_SCL,
	FL_TRD,
	FL_FDI,
	FL_UDI,
	FL_DSK,
	FL_TD0,
	FL_SNA,
	FL_Z80,
	FL_SPG,
	FL_RZX,
	FL_HOBETA,
	FL_RAW,
	FL_GB,
	FL_GBC,
	FL_MSX,
	FL_MX1,
	FL_MX2,
	FL_CAS,
	FL_NES,
	FL_T64,
	FL_C64TAP,
	FL_C64PRG,
	FL_BKBIN,
	FL_BKIMG,
	FL_BKBKD
};

enum {
	FG_DISK = -1,
	FG_ALL = 0,
	FG_TAPE = (1 << 10),
	FG_DISK_A,
	FG_DISK_B,
	FG_DISK_C,
	FG_DISK_D,
	FG_SNAPSHOT,
	FG_RZX,
	FG_HOBETA,
	FG_RAW,
	FG_GAMEBOY,
	FG_MSX,
	FG_MSXTAPE,
	FG_NES,
	FG_CMDTAPE,
	FG_CMDSNAP,
	FG_BKDATA,
	FG_BKDISK
};

enum {
	FH_SPECTRUM = (1 << 12),
	FH_MSX,
	FH_GAMEBOY,
	FH_NES,
	FH_CMD,
	FH_BK,
	FH_DISKS,
	FH_SLOTS
};

void initFileDialog(QWidget*);
int load_file(Computer* comp, const char* name, int id, int drv);
int save_file(Computer* comp, const char* name, int id, int drv);

int saveChangedDisk(Computer*,int);

#endif
