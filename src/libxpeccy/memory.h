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
// memory breaks
#define	MEM_BRK_FETCH	1
#define	MEM_BRK_RD	2
#define	MEM_BRK_WR	4
#define	MEM_BRK_ANY	(MEM_BRK_FETCH | MEM_BRK_RD | MEM_BRK_WR)

typedef struct {
	int type;
	int num;
	int flags;
	unsigned char* dptr;
	unsigned char* fptr;
} MemPage;

typedef struct {
	int flag;
	MemPage ram[256];
	MemPage rom[32];
	MemPage* pt[4];
	unsigned char* ramData;
	unsigned char* ramFlag;
	unsigned char* romData;
	unsigned char* romFlag;
	int memSize;
	int memMask;
	int romMask;	// 0:16K, 1:32K, 3:64K, 7:128K, 15:256K, 31:512K
} Memory;

Memory* memCreate(int, int);
void memDestroy(Memory*);

unsigned char memRd(Memory*,unsigned short);
void memWr(Memory*,unsigned short,unsigned char);

void memSetSize(Memory*,int);
void memSetBank(Memory*,int,int,unsigned char);

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

unsigned char* memGetPagePtr(Memory*,int,int);
unsigned char* memGetFptr(Memory*, unsigned short);

MemPage* memGetBankPtr(Memory*,unsigned short);

#if __cplusplus
}
#endif

#endif
