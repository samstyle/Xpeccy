#ifndef _FILER_H
#define _FILER_H

#include "libxpeccy/filetypes/filetypes.h"
#include "libxpeccy/spectrum.h"
#include <QFileDialog>

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

void initFileDialog(QWidget*);
void loadFile(ZXComp*,const char*, int, int);
bool saveFile(ZXComp*,const char*, int, int);

bool saveChangedDisk(ZXComp*,int);

#endif
