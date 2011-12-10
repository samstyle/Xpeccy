#ifndef _BDI_H
#define _BDI_H

#include <string>
#include <vector>
#include <stdint.h>

#include "floppy.h"

#define	ERR_OK		0
#define	ERR_MANYFILES	1
#define	ERR_NOSPACE	2
#define	ERR_SHIT	3

#define	FLP_INSERT	1
#define	FLP_PROTECT	(1<<1)
#define	FLP_TRK80	(1<<2)
#define	FLP_DS		(1<<3)
#define	FLP_INDEX	(1<<4)
#define	FLP_MOTOR	(1<<5)
#define	FLP_HEAD	(1<<6)
#define	FLP_CHANGED	(1<<7)
#define	FLP_SIDE	(1<<8)

class VG93 {
	public:
		VG93();
		bool turbo;
		bool idle;
		bool mr;
		bool crchi;
		bool block,mfm,irq,drq,sdir;
		bool idxold,idx,strb;
		uint8_t com,cop;
		uint8_t trk,sec,side,data,flag,bus;
		uint8_t buf[6];
		uint8_t dpos;
		uint8_t mode,sc;
		uint8_t *wptr, *sp;
		uint16_t ic;
		uint16_t crc,fcrc;
		int32_t count;
		uint32_t t,ti;
		uint32_t tf;
		Floppy* fptr;
		void tick();
		void command(uint8_t);
		void setmr(bool);
		void addcrc(uint8_t);
		uint8_t getflag();
};

struct BDI {
	bool enable;
	bool active;
	uint32_t tab;
	Floppy* flop[4];
	VG93 vg93;
};

typedef void(*VGOp)(VG93*);

BDI* bdiCreate();
void bdiDestroy(BDI*);
void bdiSync(BDI*,int);
bool bdiIn(BDI*,int, uint8_t*);
bool bdiOut(BDI*,int, uint8_t);

#endif
