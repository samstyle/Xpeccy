#ifndef _TAPE_H
#define _TAPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define	PILOTLEN	2168
#define	SYNC1LEN	667
#define	SYNC2LEN	735
#define	SIGN0LEN	855
#define	SIGN1LEN	1710
#define	SYNC3LEN	954

#define	TAPE_HEAD	0
#define	TAPE_DATA	1

typedef struct {
	unsigned breakPoint:1;

	int type;
	const char* name;
	int size;
	int time;
	int curtime;
} TapeBlockInfo;

typedef struct {
	unsigned breakPoint:1;
	unsigned hasBytes:1;
	unsigned isHeader:1;

	int pause;
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
	unsigned on:1;
	unsigned rec:1;
	unsigned isData:1;
	unsigned wait:1;
	unsigned blkChange:1;
	unsigned newBlock:1;

	int block;
	int pos;
	int signal;
	int toutold;
	int outsig;
	int sigLen;
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

void tapAddFile(Tape*,const char*,int,unsigned short,unsigned short,unsigned short,unsigned char*,int);
void addBlockByte(TapeBlock*, unsigned char);
void blkClear(TapeBlock*);
void blkAddSignal(TapeBlock*, int);

#ifdef __cplusplus
}
#endif

#endif
