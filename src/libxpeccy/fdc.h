#ifndef _FDC_H
#define _FDC_H

#include <stdlib.h>
#include "floppy.h"

#define FDC_FAST	1

#define FDC_NONE	0
#define FDC_VG93	1
#define FDC_UPD765	2

#define DIF_NONE	0
#define DIF_BDI		1
#define DIF_P3DOS	2
#define DIF_END		-1

#define	BDI_SYS		0xff
#define FDC_COM		0x1f
#define FDC_TRK		0x3f
#define FDC_SEC		0x5f
#define FDC_DATA	0x7f

#define BYTEDELAY 32000		// 300 turns/min = 5 turns/sec = 31250 bytes/sec = 32mks/byte
#define turbo (fdcFlag & FDC_FAST)

typedef struct FDC FDC;
typedef void(*fdcCall)(FDC*);
struct FDC {
	const int id;
	unsigned irq:1;		// VG93:irq ; uPD765:exec
	unsigned drq:1;		// 0:data request
	unsigned dir:1;		// drq dir: 0 - from CPU; 1 - to CPU
	unsigned mr:1;		// master reset
	unsigned block:1;
	unsigned side:1;
	unsigned step:1;	// step dir
	unsigned mfm:1;
	unsigned idle:1;
	unsigned crchi:1;
	unsigned char trk;	// registers
	unsigned char sec;
	unsigned char data;
	unsigned char com;
	unsigned char state;
	unsigned char tmp;
	Floppy* flop[4];
	Floppy* flp;		// current floppy ptr
	unsigned short crc;	// calculated crc
	unsigned short fcrc;	// crc get from floppy
	unsigned char buf[6];	
	int fmode;
	int cnt;
	int wait;		// pause (ns)
	fdcCall* plan;		// current task
	int pos;		// pos in plan

	unsigned dma:1;		// not implemented yet
	int hlt;	// head load time (all ns)
	int hut;	// head unload time
	int srt;	// step rate time
	unsigned char comBuf[8]; // uPD765 command args
	int comCnt;		// arg count for command
	int comPos;		// pos in comBuf
	unsigned char resBuf[8]; // uPD765 response
	int resCnt;
	int resPos;
	unsigned char sr0,sr1,sr2,sr3;	// status registers
};

typedef struct DiskHW DiskHW;

struct DiskIF {
	int type;
	DiskHW* hw;
	FDC* fdc;
};

typedef struct DiskIF DiskIF;

extern int fdcFlag;

DiskIF* difCreate(int);
void difDestroy(DiskIF*);

void difReset(DiskIF*);
void difSync(DiskIF*,int);
int difOut(DiskIF*,int,unsigned char,int);
int difIn(DiskIF*,int,unsigned char*,int);

void difSetHW(DiskIF*, int);

#endif
