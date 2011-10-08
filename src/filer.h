#ifndef _FILER_H
#define _FILER_H

#include <stdint.h>
#include <QFileDialog>

#define	FT_ALL		-1
#define	FT_NONE		0
#define	FT_TAP		(1<<0)
#define	FT_TZX		(1<<1)
#define	FT_TAPE		(FT_TAP | FT_TZX)
#define	FT_SCL		(1<<2)
#define	FT_TRD		(1<<3)
#define	FT_FDI		(1<<4)
#define	FT_UDI		(1<<5)
#define	FT_DISK		(FT_SCL | FT_TRD | FT_FDI | FT_UDI)
#define	FT_SNA		(1<<6)
#define	FT_Z80		(1<<7)
#define FT_SNAP		(FT_SNA | FT_Z80)
#define	FT_RZX		(1<<8)
#define	FT_HOBETA	(1<<9)

void initFileDialog(QWidget*);
void loadFile(const char*, int, int);
bool saveFile(const char*, int, int);

#endif
