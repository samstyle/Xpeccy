#ifndef _TAPE_H
#define _TAPE_H

#include <vector>
#include <string>
#include <stdint.h>

#define	PILOTLEN	2168
#define	SYNC1LEN	667
#define	SYNC2LEN	735
#define	SIGN0LEN	855
#define	SIGN1LEN	1710
#define	SYNC3LEN	954

#define	TAPE_ON		1
#define TAPE_REC	(1<<1)
#define TAPE_CANSAVE	(1<<2)	// can be saved as tap
#define TAPE_WAIT	(1<<3)	// wait for 1st impulse (at save)

#define	TBF_BREAK	1	// stop tape on start of this block
#define	TBF_BYTES	(1<<1)	// this is block of standard structure (pilot,sync1,sync2,1,0,sync3 is present)
#define	TBF_HEAD	(1<<2)	// block is header

#define	TAPE_HEAD	0
#define	TAPE_DATA	1

typedef struct {
	int type;
	int flag;
	std::string name;
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
	std::vector<int> data;
} TapeBlock;

typedef struct {
	int flag;
	int block;
	int pos;
	bool signal;
	bool toutold;
	bool outsig;
	int sigLen;
	double sigCount;
	std::string path;
	TapeBlock tmpBlock;
	std::vector<TapeBlock> data;
} Tape;

Tape* tapCreate();
void tapDestroy(Tape*);

void tapEject(Tape*);
bool tapPlay(Tape*);
void tapRec(Tape*);
void tapStop(Tape*);
void tapRewind(Tape*,int);

void tapSync(Tape*,int);
//int tapGet(Tape*,int);
//int tapGet(Tape*,int,int);
void tapNextBlock(Tape*);

TapeBlockInfo tapGetBlockInfo(Tape*,int);
std::vector<TapeBlockInfo> tapGetBlocksInfo(Tape*);
std::vector<uint8_t> tapGetBlockData(Tape*,int);
int tapGetBlockTime(Tape* tape, int blk, int pos);

void tapAddBlock(Tape*,TapeBlock);
void tapDelBlock(Tape*,int);
void tapSwapBlocks(Tape*,int,int);

void tapAddFile(Tape*,std::string,int,uint16_t,uint16_t,uint16_t,uint8_t*,bool);

#endif
