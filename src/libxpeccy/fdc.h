#pragma once

#include <stdlib.h>
#include "floppy.h"

#define FDC_FAST	1

enum {
	FDC_NONE = 0,
	FDC_VG93,
	FDC_UPD765,
	FDC_VP1_128,
	FDC_I8272
};

enum {
	DIF_NONE = 0,
	DIF_BDI,
	DIF_P3DOS,
	DIF_PC,
	DIF_SMK512,
	DIF_END	= -1
};

#define	BDI_SYS		0xff
#define FDC_COM		0x1f
#define FDC_TRK		0x3f
#define FDC_SEC		0x5f
#define FDC_DATA	0x7f

#define TURBOBYTE 500		// same for turbo
#define turbo (fdcFlag & FDC_FAST)

typedef struct {
	unsigned char trk;
	unsigned char head;
	unsigned char sec;
	unsigned char sz;	// 0..3 = 128..1024
	unsigned char type;
	int crc;
	unsigned char data[0x1800];
} Sector;

typedef struct FDC FDC;
typedef void(*fdcCall)(FDC*);

struct FDC {
	const int id;
	unsigned brk:1;		// signal for debug
	unsigned upd:1;		// 1 if uPD765, 0 if i8275
	unsigned seekend:1;	// uPD765: set at end of seek/recalibrate com
	unsigned irq:1;		// VG93:irq ; uPD765:exec
	unsigned drq:1;		// 1:data request
	unsigned dir:1;		// drq dir: 0 - cpu->fdc; 1 - fdc->cpu
	unsigned mr:1;		// master reset
	unsigned block:1;
	unsigned side:1;
	unsigned step:1;	// step dir
	unsigned mfm:1;
	unsigned idle:1;
	unsigned crchi:1;
	unsigned hd:1;		// HD mode (trklen 12500, bytedelay 16000)

	unsigned char trk;	// registers
	unsigned char sec;
	unsigned char data;
	unsigned char com;
	unsigned char state;	// vp1-128 : mode (seek/rd/wr)
	unsigned char tmp;
	unsigned short wdata;
	unsigned short tdata;
	int bytedelay;

	Floppy* flop[4];
	Floppy* flp;		// current floppy ptr

	unsigned short crc;	// calculated crc
	unsigned short fcrc;	// crc get from floppy
	unsigned char buf[6];
	int fmode;
	int cnt;
	int wait;		// pause (ns)
	int tns;
	int drdy;		// time (ns) to replace fdc->insert signal by 0 (time between opening and closing a flp gate)

	fdcCall* plan;		// current task
	int pos;		// pos in plan

	cbirq xirq;
	void* xptr;

	unsigned dma:1;		// not implemented yet
	unsigned intr:1;	// uPD765 interrupt pending. reset @ com08
	unsigned inten:1;	// uPD765 interrupt enabled
	int hlt;		// head load time (all ns)
	int hut;		// head unload time
	int srt;		// step rate time
	unsigned char comBuf[8]; // uPD765 command args
	int comCnt;		// arg count for command
	int comPos;		// pos in comBuf
	unsigned char resBuf[8]; // uPD765 response
	int resCnt;
	int resPos;
	unsigned char sr0,sr1,sr2,sr3;	// status registers

	Sector slst[64];
	int scnt;
};

void fdc_set_hd(FDC*, int);

typedef struct DiskHW DiskHW;

struct DiskIF {
	unsigned lirq:1;	// last int
	int type;
	DiskHW* hw;
	FDC* fdc;
};

typedef struct DiskIF DiskIF;

extern int fdcFlag;

DiskIF* difCreate(int, cbirq, void*);
void difDestroy(DiskIF*);

void difReset(DiskIF*);
void difSync(DiskIF*, int);
int difOut(DiskIF*, int, int, int);
int difIn(DiskIF*, int, int*, int);

void difSetHW(DiskIF*, int);

void add_crc_16(FDC*, unsigned char);
