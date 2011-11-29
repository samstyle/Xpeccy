#ifndef _TAPE_H
#define _TAPE_H

#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>

#define	TYPE_TAP	0
#define	TYPE_TZX	1

#define	TAPE_ON		1
#define TAPE_REC	(1<<1)
#define TAPE_CANSAVE	(1<<2)	// can be saved as tap
#define TAPE_WAIT	(1<<3)	// wait for 1st impulse (at save)

#define	TBF_BREAK	1	// stop tape on start of this block
#define	TBF_BYTES	(1<<1)	// this is block of standard structure (pilot,sync1,sync2,1,0,sync3 is present)
#define	TBF_HEAD	(1<<2)	// block is header

#define	TAPE_HEAD	0
#define	TAPE_DATA	1

struct TapeBlockInfo {
	int type;
	int flags;
	std::string name;
	int size;
	int time;
	int curtime;
};

class TapeBlock {
	public:
		TapeBlock();
		unsigned long pause;					// pause in ms after block
		int flags;
		uint32_t plen,s1len,s2len,len0,len1;	// signals len
		uint32_t pdur;				// pilot tone length
		int32_t datapos;					// 1st data signal pos
		std::vector<uint32_t> data;			// signals
		int32_t gettime(int32_t);
		int32_t getsize();
		uint8_t getbyte(int32_t);
		std::string getheader();
		void normSignals();
};

class Tape {
	public:
		Tape();
		std::string path;
		uint32_t block;
		uint32_t pos;
		int32_t flags;
		bool signal;
		bool toutold;
		bool outsig;
		int32_t lastick;
		int siglen;
		TapeBlock tmpblock;
		std::vector<TapeBlock> data;
		uint8_t getbyte(int32_t,int32_t);
		std::vector<uint8_t> getdata(int32_t,int32_t,int32_t);
		void eject();
		void stop(int);
		bool startplay();
		void startrec();
		void sync();
		void storeblock();
		void swapblocks(int32_t,int32_t);
		std::vector<TapeBlockInfo> getInfo();
		void addFile(std::string,int,uint16_t,uint16_t,uint16_t,uint8_t*,bool);
		void addBlock(uint8_t*, int, bool);
};

std::vector<uint8_t> tapGetBlockData(Tape* tape, int blockNum);

#endif
