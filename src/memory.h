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
