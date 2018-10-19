#ifndef X_FILER_H
#define X_FILER_H

#include "libxpeccy/filetypes/filetypes.h"
#include "libxpeccy/spectrum.h"
#include <QFileDialog>

/*
#define	FT_ALL		-1
#define	FT_NONE		0
#define	FT_TAP		(1<<0)
#define	FT_TZX		(1<<1)
#define FT_WAV		(1<<2)
#define	FT_TAPE		(FT_TAP | FT_TZX | FT_WAV)
#define	FT_SCL		(1<<3)
#define	FT_TRD		(1<<4)
#define	FT_FDI		(1<<5)
#define	FT_UDI		(1<<6)
#define	FT_DSK		(1<<7)
#define FT_TD0		(1<<8)
#define	FT_DISK		(FT_SCL | FT_TRD | FT_FDI | FT_UDI | FT_DSK | FT_TD0)
#define	FT_SNA		(1<<9)
#define	FT_Z80		(1<<10)
#define FT_SNAP		(FT_SNA | FT_Z80)
#define	FT_RZX		(1<<11)
#define	FT_HOBETA	(1<<12)
#define	FT_RAW		(1<<13)
#define	FT_SPG		(1<<14)
#define FT_SLOT_A	(1<<15)			// raw images (msx, gbc)
#define FT_NES		(1<<16)			// +nes header
#define FT_SLOT		(FT_SLOT_A | FT_NES)
#define FT_T64		(1<<17)
*/

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
	FL_NES,
	FL_T64,
	FL_BKBIN,
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
	FG_NES,
	FG_CMDTAPE,
	FG_BKDATA
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
//void loadFile(Computer*,const char*, int, int);
//bool saveFile(Computer*,const char*, int, int);
int load_file(Computer* comp, const char* name, int id, int drv);
int save_file(Computer* comp, const char* name, int id, int drv);

bool saveChangedDisk(Computer*,int);

#endif
