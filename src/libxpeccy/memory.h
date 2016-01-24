#ifndef _MEMOR_H
#define _MEMOR_H

#ifdef __cplusplus
extern "C" {
#endif

// mempage type
#define	MEM_RAM		1
#define	MEM_ROM		2
#define MEM_EXT		3
// memory banks
#define	MEM_BANK0	0
#define	MEM_BANK1	1
#define	MEM_BANK2	2
#define	MEM_BANK3	3

typedef struct {
	unsigned wren:1;		// write enable
	int type;
	int num;
	unsigned char* dptr;
	void* data;						// ptr for rd/wr func
	unsigned char(*rd)(unsigned short, void*);		// external rd (for type MEM_EXT)
	void(*wr)(unsigned short, unsigned char, void*);	// external wr (for type MEM_EXT)
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

void memSetSize(Memory*,int);
void memSetBank(Memory*,int,int,unsigned char);
void memSetExternal(Memory*,int,MemPage);

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

unsigned char* memGetPagePtr(Memory*,int,int);

MemPage* memGetBankPtr(Memory*,unsigned short);

#if __cplusplus
}
#endif

#endif
