#ifndef _TAPE_H
#define _TAPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define	PILOTLEN	2168
#define	SYNC1LEN	667
#define	SYNC2LEN	735
#define	SIGN0LEN	855
#define	SIGN1LEN	1710
#define	SYNC3LEN	954

#define	TAPE_ON			1
#define TAPE_REC		(1<<1)
#define TAPE_CANSAVE		(1<<2)	// can be saved as tap
#define TAPE_WAIT		(1<<3)	// wait for 1st impulse (rec)
#define	TAPE_BLOCK_CHANGED	(1<<4)	// move to next block (play)
#define	TAPE_NEW_BLOCK		(1<<5)	// new block added (rec)

#define	TBF_BREAK	1	// stop tape on start of this block
#define	TBF_BYTES	(1<<1)	// this is block of standard structure (pilot,sync1,sync2,1,0,sync3 is present)
#define	TBF_HEAD	(1<<2)	// block is header

#define	TAPE_HEAD	0
#define	TAPE_DATA	1

typedef struct {
	int type;
	int flag;
	const char* name;
	int size;
	int time;
	int curtime;
} TapeBlockInfo;

typedef struct {
	int pause;
	int flag;
	int plen;
	int s1len;
	int s2len;
	int len0;
	int len1;
	int pdur;
	int dataPos;
	int sigCount;
	int* sigData;
} TapeBlock;

typedef struct {
	int flag;
	int block;
	int pos;
	int signal;
	int toutold;
	int outsig;
	int sigLen;
	double sigCount;
	char* path;
	TapeBlock tmpBlock;
	int blkCount;
	TapeBlock* blkData;
} Tape;

Tape* tapCreate();
void tapDestroy(Tape*);

void tapEject(Tape*);
int tapPlay(Tape*);
void tapRec(Tape*);
void tapStop(Tape*);
void tapRewind(Tape*,int);

void tapSync(Tape*,int);
void tapNextBlock(Tape*);

TapeBlockInfo tapGetBlockInfo(Tape*,int);
int tapGetBlocksInfo(Tape*,TapeBlockInfo*);
int tapGetBlockData(Tape*,int,unsigned char*);
int tapGetBlockTime(Tape*,int,int);

void tapAddBlock(Tape*,TapeBlock);
void tapDelBlock(Tape*,int);
void tapSwapBlocks(Tape*,int,int);

void tapAddFile(Tape*,const char*,int,uint16_t,uint16_t,uint16_t,uint8_t*,int);
void addBlockByte(TapeBlock*, uint8_t);
void blkClear(TapeBlock*);
void blkAddSignal(TapeBlock*, int);

#ifdef __cplusplus
}
#endif

#endif
