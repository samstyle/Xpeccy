#ifndef _MEMOR_H
#define _MEMOR_H

#include <stdint.h>

#define	MEM_PROFMASK	0
#define	MEM_RAM		1
#define	MEM_ROM		2
#define	MEM_MEMSIZE	3

#define	MEM_BANK0	0
#define	MEM_BANK1	1
#define	MEM_BANK2	2
#define	MEM_BANK3	3

typedef struct {
	uint8_t ram[64][16384];
	uint8_t rom[32][16384];
	uint8_t *pt0,*pt1,*pt2,*pt3;
	uint8_t cram,crom;
	int32_t	mask;
	int32_t profMask;	// profrom (0 - 64K, 1 - 128K, 3 - 256K)
} Memory;

Memory* memCreate();
void memDestroy(Memory*);

uint8_t memRd(Memory*,uint16_t);
void memWr(Memory*,uint16_t,uint8_t);

void memSetBank(Memory*,int,int,int);

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

int memGet(Memory*,int);
void memSet(Memory*,int,int);

uint8_t* memGetPagePtr(Memory*,int,int);

#endif
