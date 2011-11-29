#ifndef _MEMOR_H
#define _MEMOR_H

#include <string>
#include <vector>
#include <stdint.h>

#define	TYP_SNA		0
#define	TYP_Z80 	1
#define	TYP_RZX 	2

#define	MEM_PROFMASK	0
#define	MEM_RAM		1
#define	MEM_ROM		2
#define	MEM_MEMSIZE	3

#define	MEM_BANK0	0
#define	MEM_BANK1	1
#define	MEM_BANK2	2
#define	MEM_BANK3	3

struct RomSet {
	std::string name;
	std::string file;	// set when romfile is single file
	struct {
		std::string path;
		uint8_t part;
	} roms[32];
};

//struct RZXFrame {
//	int fetches;		// fetches till next int
//	std::vector<uint8_t> in;
//};

/*
class Memory {
	public:
	Memory();
	uint8_t ram[64][16384];
	uint8_t rom[32][16384];
	uint8_t *pt0,*pt1,*pt2,*pt3;
	uint8_t cram,crom;
	uint8_t prt0;		// 7ffd value
	uint8_t prt1;		// extend port value
	uint8_t prt2;		// scorpion ProfROM layer (0..3)
	uint8_t res;		// rompart active after reset
	int32_t	mask;
	int32_t profMask;	// profrom (0 - 64K, 1 - 128K, 3 - 256K)
	uint8_t rd(uint16_t);
	void wr(uint16_t, uint8_t);
//	void load(std::string,int32_t);
//	void parse(std::ifstream*,int32_t);
//	void save(std::string,int32_t,bool);
	RomSet *romset;
//	std::vector<RZXFrame> rzx;
//	int rzxFrame;
//	int rzxPos;
//	uint8_t getRZXIn();
//	void loadromset(std::string);
//	void setram(uint8_t);
//	void setrom(uint8_t);
};
*/

struct Memory;

Memory* memCreate();
void memDestroy(Memory*);

uint8_t memRd(Memory*,uint16_t);
void memWr(Memory*,uint16_t,uint8_t);

void memSetBank(Memory*,int,int,int);

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

void memSetRomset(Memory*,RomSet*);
RomSet* memGetRomset(Memory*);

int memGet(Memory*,int);
void memSet(Memory*,int,int);

uint8_t* memGetPagePtr(Memory*,int,int);

#endif
