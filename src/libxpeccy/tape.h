#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

#define TAPTICKNS	100		// ns in one tape tick
#define TAPTPS		1e9/TAPTICKNS	// ticks per second
#define TAPCPUNS	284		// ns per cpu tick @ 3.51MHz

// ZX spectrum signal timings
// 1T ~ 284ns ~ 0.284mks @ 3.51MHz
// pilot	2168T	615 mks
// sync1	667T	189 mks
// sync2	735T	208 mks
// 0		855T	242 mks
// 1		1710T	485 mks
// sync3	954T	270 mks
#define	PILOTLEN	2168*TAPCPUNS/TAPTICKNS
#define	SYNC1LEN	667*TAPCPUNS/TAPTICKNS
#define	SYNC2LEN	735*TAPCPUNS/TAPTICKNS
#define	SIGN0LEN	855*TAPCPUNS/TAPTICKNS
#define	SIGN1LEN	1710*TAPCPUNS/TAPTICKNS

enum {
	TAPE_HEAD = 0,
	TAPE_DATA
};

enum {
	TFRM_ZX = 0,
	TFRM_BK
};

typedef struct {
	unsigned breakPoint:1;

	int type;
	char name[32];
	int size;
	int time;
	int curtime;
} TapeBlockInfo;

typedef struct {
	unsigned int size;		// tape ticks
	unsigned char vol;
} TapeSignal;

typedef struct {
	unsigned breakPoint:1;
	unsigned hasBytes:1;
	unsigned isHeader:1;
	unsigned vol:1;

	int plen;
	int s1len;
	int s2len;
	int len0;
	int len1;
	int pdur;
	int dataPos;
	int sigCount;
	int time;
	int crc;
	TapeSignal* data;
} TapeBlock;

typedef struct {
	unsigned on:1;
	unsigned rec:1;
	unsigned isData:1;
	unsigned wait:1;
	unsigned blkChange:1;
	unsigned newBlock:1;
	int levRec;	// signal to tape
	unsigned oldRec:1;	// previous rec signal

	int time;
	unsigned char volPlay;
	int block;
	int pos;
	int sigLen;
	char* path;
	TapeBlock tmpBlock;
	int blkCount;
	TapeBlock* blkData;

	unsigned xen:1;
	cbirq xirq;
	void* xptr;
} Tape;

Tape* tape_create(cbirq, void*);
void tape_destroy(Tape*);

void tape_set_path(Tape*, const char*);

void tapEject(Tape*);
int tapPlay(Tape*);
void tapRec(Tape*);
void tapStop(Tape*);
void tapRewind(Tape*,int);

void tapSync(Tape*,int);
void tapNextBlock(Tape*);

TapeBlockInfo tapGetBlockInfo(Tape*,int,int);
int tapGetBlocksInfo(Tape*,TapeBlockInfo*, int);
int tapGetBlockData(Tape*,int,unsigned char*,int);
int tapGetBlockTime(Tape*,int,int);

void tap_add_block(Tape*,TapeBlock);
void tapDelBlock(Tape*,int);
void tapSwapBlocks(Tape*,int,int);

void tapAddFile(Tape*,const char*,int,unsigned short,unsigned short,unsigned short,unsigned char*,int);
TapeBlock makeTapeBlock(unsigned char*, int, int);

void blkClear(TapeBlock*);
void blkAddPulse(TapeBlock* blk, int len, int vol);
void blkAddWave(TapeBlock*, int);
void blkAddByte(TapeBlock*, unsigned char, int, int);
void blkAddPause(TapeBlock*, int);

#ifdef __cplusplus
}
#endif
