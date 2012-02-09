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

#define	TAPE_FLAGS	0
#define TAPE_BLOCKS	1
#define	TAPE_BLOCK	2
#define	TAPE_BFLAG	3
#define	TAPE_BFTIME	4
#define	TAPE_BCTIME	5

struct TapeBlockInfo {
	int type;
	int flags;
	std::string name;
	int size;
	int time;
	int curtime;
};

struct TapeBlock {
	int pause;
	int flags;
	int plen;
	int s1len;
	int s2len;
	int len0;
	int len1;
	int pdur;
	int dataPos;
	std::vector<int> data;
};

struct Tape;

Tape* tapCreate();
void tapDestroy(Tape*);

void tapEject(Tape*);
bool tapPlay(Tape*);
void tapRec(Tape*);
void tapStop(Tape*);
void tapRewind(Tape*,int);

bool tapGetSignal(Tape*);
bool tapGetOutsig(Tape*);
void tapSetSignal(Tape*,bool);
void tapSync(Tape*,int);
int tapGet(Tape*,int);
std::string tapGetPath(Tape*);
void tapSetPath(Tape*,const char*);
int tapGet(Tape*,int,int);
void tapSet(Tape*,int,int,int);
void tapNextBlock(Tape*);

void tapSetFlag(Tape*,int,bool);

TapeBlockInfo tapGetBlockInfo(Tape*,int);
std::vector<TapeBlockInfo> tapGetBlocksInfo(Tape*);
std::vector<uint8_t> tapGetBlockData(Tape*,int);

void tapAddBlock(Tape*,TapeBlock);
void tapDelBlock(Tape*,int);
void tapSwapBlocks(Tape*,int,int);

void tapAddFile(Tape*,std::string,int,uint16_t,uint16_t,uint16_t,uint8_t*,bool);

#endif
