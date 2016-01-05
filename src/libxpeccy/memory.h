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
	int type;
	int num;
	unsigned char* dptr;
//	unsigned char* fptr;
} MemPage;

typedef struct {
	MemPage ram[256];
	MemPage rom[32];
	MemPage* pt[4];
	MemPage ext[4];
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
void memSetExternal(Memory*,int,int,unsigned char*);

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

unsigned char* memGetPagePtr(Memory*,int,int);

MemPage* memGetBankPtr(Memory*,unsigned short);

#if __cplusplus
}
#endif

#endif
