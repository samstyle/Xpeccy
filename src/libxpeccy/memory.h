#ifndef _MEMOR_H
#define _MEMOR_H

#ifdef __cplusplus
extern "C" {
#endif

// mempage type
enum {
	MEM_RAM	= 1,
	MEM_ROM,
	MEM_SLOT,
	MEM_EXT
};
// memory banks
enum {
	MEM_BANK0 = 0,
	MEM_BANK1,
	MEM_BANK2,
	MEM_BANK3
};

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
	MemPage map[4];				// 4 x 16K
	unsigned char ramData[0x400000];	// 4M
	unsigned char romData[0x80000];		// 512K
	int memSize;
	int memMask;
	int romMask;	// 0:16K, 1:32K, 3:64K, 7:128K, 15:256K, 31:512K
} Memory;

Memory* memCreate();
void memDestroy(Memory*);

unsigned char memRd(Memory*,unsigned short);
void memWr(Memory*,unsigned short,unsigned char);

void memSetSize(Memory*, int);
void memSetBank(Memory*, int, int, int, extmrd, extmwr, void*);

void memSetPageData(Memory*,int,int,char*);
void memGetPageData(Memory*,int,int,char*);

unsigned char* memGetPagePtr(Memory*,int,int);
MemPage* memGetBankPtr(Memory*,unsigned short);
xAdr memGetXAdr(Memory*, unsigned short);

#if __cplusplus
}
#endif

#endif
