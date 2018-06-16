#ifndef _MEMOR_H
#define _MEMOR_H

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_SMALL_PAGES 0

// mempage type
enum {
	MEM_RAM	= 1,
	MEM_ROM,
	MEM_SLOT,
	MEM_EXT
};
// memory banks
/*
enum {
	MEM_BANK0 = 0,
	MEM_BANK1,
	MEM_BANK2,
	MEM_BANK3
};
*/

// memory size
#define MEM_48K	(1<<0)		// "special" value
#define MEM_256	(1<<8)
#define MEM_512	(1<<9)
#define MEM_1K	(1<<10)
#define MEM_2K	(1<<11)
#define MEM_4K	(1<<12)
#define MEM_8K	(1<<13)
#define MEM_16K	(1<<14)
#define MEM_32K	(1<<15)
#define MEM_64K	(1<<16)
#define MEM_128K	(1<<17)
#define MEM_256K	(1<<18)
#define MEM_512K	(1<<19)
#define MEM_1M	(1<<20)
#define MEM_2M	(1<<21)
#define MEM_4M	(1<<22)

typedef struct {
    int type;
    int bank;
    int adr;
    int abs;
} xAdr;

typedef unsigned char(*extmrd)(unsigned short, void*);
typedef void(*extmwr)(unsigned short, unsigned char, void*);

typedef struct {
	int type;			// type of page data
	int num;			// 16K page number
	void* data;			// ptr for rd/wr func
	extmrd rd;			// external rd (for type MEM_EXT)
	extmwr wr;			// external wr (for type MEM_EXT)
} MemPage;

typedef struct {
	MemPage map[256];			// 4 x 16K | 256 x 256
	unsigned char ramData[0x400000];	// 4M
	unsigned char romData[0x80000];		// 512K
	int ramSize;
	int ramMask;
	int romSize;
	int romMask;
} Memory;

Memory* memCreate(void);
void memDestroy(Memory*);

unsigned char memRd(Memory*,unsigned short);
void memWr(Memory*,unsigned short,unsigned char);

void memSetSize(Memory*, int, int);
void memSetBank(Memory*, int, int, int, int, extmrd, extmwr, void*);

void memGetData(Memory*,int,int,int,char*);
void memPutData(Memory*,int,int,int,char*);

xAdr memGetXAdr(Memory*, unsigned short);
int memFindAdr(Memory*, int, int);

/*

unsigned char* memGetPagePtr(Memory*,int,int);
MemPage* memGetBankPtr(Memory*,unsigned short);

*/

#if __cplusplus
}
#endif

#endif
