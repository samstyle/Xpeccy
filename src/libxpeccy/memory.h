#ifndef _MEMOR_H
#define _MEMOR_H

#ifdef __cplusplus
extern "C" {
#endif

// overall flags
#define	MEM_NOFLAG	1

// mempage type
#define	MEM_RAM		1
#define	MEM_ROM		2
// memory banks
#define	MEM_BANK0	0
#define	MEM_BANK1	1
#define	MEM_BANK2	2
#define	MEM_BANK3	3
// memory flags
#define	MEM_BREAK	1
// mempage flags
#define	MEM_RDONLY	1
// membyte flags
#define	MEM_BRK_FETCH	1
#define	MEM_BRK_READ	(1<<1)
#define	MEM_BRK_WRITE	(1<<2)

typedef struct {
	int type;
	int num;
	int flags;
	unsigned char* data;
	unsigned char* flag;
} MemPage;

typedef struct {
	int flags;
	MemPage ram[256];
	MemPage rom[32];
	MemPage* pt0;
	MemPage* pt1;
	MemPage* pt2;
	MemPage* pt3;
	unsigned char ramData[0x400000];	// 4M
	unsigned char ramFlag[0x400000];
	unsigned char romData[0x80000];		// 512K
	unsigned char romFlag[0x80000];
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

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

unsigned char* memGetPagePtr(Memory*,int,int);

unsigned char memGetCellFlags(Memory*, unsigned short);
void memSwitchCellFlags(Memory*,unsigned short,unsigned char);

MemPage* memGetBankPtr(Memory*,unsigned short);

#if __cplusplus
}
#endif

#endif
