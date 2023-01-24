#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

// mempage type
enum {
	MEM_RAM	= 1,
	MEM_ROM,
	MEM_SLOT,
	MEM_EXT,
	MEM_IO
};

typedef struct {
    int type;
    int bank;
    int adr;
    int abs;
} xAdr;

typedef int(*extmrd)(int, void*);
typedef void(*extmwr)(int, int, void*);

typedef struct {
	int type;			// type of page (ram/rom/slt/io/ext...)
	int num;			// page number
	void* data;			// ptr for rd/wr func
	extmrd rd;			// external rd (for type MEM_EXT)
	extmwr wr;			// external wr (for type MEM_EXT)
} MemPage;

typedef struct {
	MemPage map[256];			// 256 x 256 | 256 x 64K
	unsigned char ramData[0x400000];	// 4M
	unsigned char romData[0x80000];		// 512K
	int ramSize;
	int ramMask;
	int romSize;
	int romMask;
	int pgsize;	// size of page in bytes
	int pgmask;	// number of LSBits in address = page offset (FF or FFFF)
	int pgshift;	// = log2(page size), 8 for 256-pages, 16 for 64K-pages
	int busmask;	// cpu addr bus mask (todo: move to CPU)
	char* snapath;
} Memory;

Memory* memCreate(void);
void memDestroy(Memory*);

int memRd(Memory*, int);
void memWr(Memory*, int, int);

void memSetSize(Memory*, int, int);
void memSetBank(Memory* mem, int page, int type, int bank, int siz, extmrd rd, extmwr wr, void* data);

void memPutData(Memory*,int,int,int,char*);

xAdr mem_get_xadr(Memory*, int);
int memFindAdr(Memory*, int, int);

void mem_set_path(Memory*, const char*);
void mem_set_bus(Memory*, int);
int mem_get_phys_adr(Memory*, int);
MemPage* mem_get_page(Memory*, int);

#ifdef __cplusplus
}
#endif
