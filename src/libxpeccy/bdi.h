#ifndef _BDI_H
#define _BDI_H

#ifdef __cplusplus
extern "C" {
#endif

//#include <stdint.h>
#include <stdlib.h>

#include "floppy.h"

// bdi flags
//#define	BDI_ACTIVE	(1<<1)
//#define BDI_TURBO	(1<<2)
// fdc status
#define FDC_IDLE	0
#define	FDC_READ	1
#define	FDC_WRITE	2
#define	FDC_INPUT	3	// command (uPD765)
#define	FDC_OUTPUT	4	// result (uPD765)
#define	FDC_EXEC	5	// command execution (uPD765)
#define	FDC_BREAK	6	// signal to break rd/wr operation (uPD765)
// fdc type
#define	FDC_NONE	0	// no disk interface at all
#define	FDC_93		1	// KR1818VG93 (WD1793 clone)
#define	FDC_765		2	// uPD765
// fdc registers
#define	FDC_COM		0x1f	// 93:1f(reg0)
#define	FDC_STATE	FDC_COM	// 93:1f(reg0),765:reg0 (ro)
#define	FDC_TRK		0x3f	// 93:3f(reg1)
#define	FDC_SEC		0x5f	// 93:5f(reg2)
#define	FDC_DATA	0x7f	// 93:7f(reg3),765:reg1 (rw)
#define BDI_SYS		0xff	// bdi:ff(sys)
// 765 flags
#define	FDC_MT		(1<<7)	// com: multitrack
#define	FDC_MF		(1<<6)	// com: mfm
#define	FDC_SK		(1<<5)	// com: skip deleted

#define	FDC_BSY		(1<<4)	// srm: fdc is busy (irq = 0)
#define	FDC_EXM		(1<<5)	// srm: fdc in execution mode
#define	FDC_DIO		(1<<6)	// srm: i/o direction fdc->cpu: 0:cpu->fdc; 1:fdc->cpu
#define	FDC_RQM		(1<<7)	// srm: data request (drq = 1)

#define	FDC_NR		(1<<3)	// s0: not ready (drive not ready or non-existing head selected)
#define	FDC_EC		(1<<4)	// s0: recalibrate fails
#define	FDC_SE		(1<<5)	// s0: seek end

#define	FDC_MA		1	// s1: missing address
#define	FDC_ND		(1<<2)	// s1: no data
#define	FDC_EN		(1<<7)	// s1: EOT detected

#define	FDC_SN		(1<<2)	// s2: scan meet
#define	FDC_SH		(1<<3)	// s2: scan equal

#define	FDC_HD		(1<<2)	// s3: head
#define	FDC_DS		(1<<3)	// s3: two side (0=yes)
#define	FDC_T0		(1<<4)	// s3: track 0
#define	FDC_RY		(1<<5)	// s3: drive is ready
#define	FDC_WP		(1<<6)	// s3: write protect
#define	FDC_FT		(1<<7)	// s3: drive failure ???

extern int fdcFlag;

#define	FDC_FAST	1

typedef struct {
	int type;		// FDC_NONE | FDC_93 | FDC_765
	int idle;
	int mr;
	int crchi;
	int block,mfm,irq,drq,sdir;
	int idxold,idx,strb;
	int status;
	unsigned char com,cop;
	unsigned char trk,sec,side,data,flag,bus;
	unsigned char buf[6];
	unsigned char mode;
	unsigned char *wptr, *sp;
	unsigned short ic;
	unsigned short crc,fcrc;

	unsigned char sr0,sr1,sr2;		// status registers for uPD765
	unsigned char comBuf[7];		// command parameters
	unsigned char ioDir;		// i/o direction = FDC_DIO bit
	unsigned char hd,sz,ls,gp,sl,fb,nm;	// parameters for uPD765 commands

	int count;
	int t;
	int tf;
	Floppy* flop[4];
	Floppy* fptr;		// pointer to current floppy
} FDC;

// bdi is bdi interface only (works with VG93)

typedef struct {
	int flag;
	FDC* fdc;
} BDI;

BDI* bdiCreate();
void bdiDestroy(BDI*);
void bdiReset(BDI*);
void bdiSync(BDI*,int);

int bdiIn(BDI*, unsigned short, unsigned char*, unsigned);
int bdiOut(BDI*,unsigned short, unsigned char, unsigned);

unsigned char fdcRd(FDC*,int);
void fdcWr(FDC*,int,unsigned char);

#ifdef __cplusplus
}
#endif

#endif
