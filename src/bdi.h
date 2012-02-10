#ifndef _BDI_H
#define _BDI_H

#include <string>
#include <vector>
#include <stdint.h>

#include "floppy.h"

#define BDI_ENABLE	1
#define	BDI_ACTIVE	(1<<1)
#define BDI_TURBO	(1<<2)
// bdi get
#define	FDC_STATUS	0
// fdc mode
#define FDC_IDLE	0
#define	FDC_READ	1
#define	FDC_WRITE	2

struct BDI;

BDI* bdiCreate();
void bdiDestroy(BDI*);
void bdiReset(BDI*);
void bdiSync(BDI*,int);
bool bdiIn(BDI*,int, uint8_t*);
bool bdiOut(BDI*,int, uint8_t);

bool bdiGetFlag(BDI*,int);
void bdiSetFlag(BDI*,int,bool);
int bdiGet(BDI*,int);
Floppy* bdiGetFloppy(BDI*,int);

#endif
