#ifndef _BDI_H
#define _BDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "floppy.h"

// disk interface type
#define	DISK_NONE	0
#define	DISK_BDI	1
#define	DISK_PLUS3	2
// bdi flags
#define	BDI_ACTIVE	(1<<1)
#define BDI_TURBO	(1<<2)
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
	int turbo;
	int idle;
	int mr;
	int crchi;
	int block,mfm,irq,drq,sdir;
	int idxold,idx,strb;
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
	int type;
	int flag;
	Floppy* flop[4];
	FDC* fdc;
} BDI;

BDI* bdiCreate();
void bdiDestroy(BDI*);
void bdiReset(BDI*);
void bdiSync(BDI*,int);
int bdiIn(BDI*,int, uint8_t*);
int bdiOut(BDI*,int, uint8_t);

#ifdef __cplusplus
}
#endif

#endif
