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
// fdc type
#define	FDC_NONE	0
#define	FDC_VG93	1
// vg93 registers
#define	FDC_COM		0x1f
#define	FDC_TRK		0x3f
#define	FDC_SEC		0x5f
#define	FDC_DATA	0x7f
#define BDI_SYS		0xff

typedef struct {
	bool turbo;
	bool idle;
	bool mr;
	bool crchi;
	bool block,mfm,irq,drq,sdir;
	bool idxold,idx,strb;
	int type;
	int status;
	uint8_t com,cop;
	uint8_t trk,sec,side,data,flag,bus;
	uint8_t buf[6];
	uint8_t mode;
	uint8_t *wptr, *sp;
	uint16_t ic;
	uint16_t crc,fcrc;
	int count;
	int t;
	int tf;
	Floppy* fptr;		// pointer to current floppy
} FDC;

typedef struct {
	int flag;
	Floppy* flop[4];
	FDC* fdc;
} BDI;

BDI* bdiCreate();
void bdiDestroy(BDI*);
void bdiReset(BDI*);
void bdiSync(BDI*,int);
bool bdiIn(BDI*,int, uint8_t*);
bool bdiOut(BDI*,int, uint8_t);

//bool bdiGetFlag(BDI*,int);
//void bdiSetFlag(BDI*,int,bool);
//int bdiGet(BDI*,int);
//Floppy* bdiGetFloppy(BDI*,int);

#endif
