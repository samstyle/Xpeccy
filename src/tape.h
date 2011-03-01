#ifndef _TAPE_H
#define _TAPE_H

#include <vector>
#include <string>
#include <fstream>

#define	TYPE_TAP	0
#define	TYPE_TZX	1

#define	TAPE_ON		0x00000001
#define TAPE_REC	0x00000002
#define TAPE_CANSAVE	0x00000004	// can be saved as tap
#define TAPE_WAIT	0x00000008	// wait for 1st impulse (at save)

struct TapeBlock {
	int pause;					// pause in ms after block
	unsigned int plen,s1len,s2len,len0,len1;	// signals len
	unsigned int pdur;				// pilot tone length
	int datapos;					// 1st data signal pos
	std::vector<unsigned int> data;			// signals
	int gettime(int);
	int getsize();
	unsigned char getbyte(int);
	std::string getheader();
};

struct Tape {
	Tape();
	std::string path;
	unsigned int block;		// tape block
	unsigned int pos;		// tap position
	int flags;
	bool signal;
	bool toutold;
	bool outsig;
	int lastick;		// cpu.tick at last sync
	int siglen;		// остаток длины текущего сигнала
	TapeBlock tmpblock;	// временный блок. сюда ведется запись
	std::vector<TapeBlock> data;
	unsigned char getbyte(int,int);
	std::vector<unsigned char> getdata(int,int,int);
	void eject();
	void stop();
	void startplay();
	void startrec();
	void sync();
	void storeblock();
	void swapblocks(int,int);
	TapeBlock parse(std::ifstream*,unsigned int,std::vector<int>);
	void load(std::string,unsigned char);
	void save(std::string,unsigned char);
};

extern Tape *tape;

#endif
