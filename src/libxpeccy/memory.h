#ifndef _MEMOR_H
#define _MEMOR_H

#ifdef __cplusplus
extern "C" {
#endif

//#define	MEM_PROFMASK	0
#define	MEM_RAM		1
#define	MEM_ROM		2
#define	MEM_MEMSIZE	3

#define	MEM_BANK0	0
#define	MEM_BANK1	1
#define	MEM_BANK2	2
#define	MEM_BANK3	3

#define	MEM_RDONLY	1

typedef struct {
	int flags;
	unsigned char data[0x4000];
} MemPage;

typedef struct {
	MemPage ram[64];
	MemPage rom[32];
	MemPage* pt0;
	MemPage* pt1;
	MemPage* pt2;
	MemPage* pt3;
	unsigned char cram;
	unsigned char crom;
	int mask;
	int profMask;	// profrom (0 - 64K, 1 - 128K, 3 - 256K)
} Memory;

Memory* memCreate();
void memDestroy(Memory*);

unsigned char memRd(Memory*,unsigned short);
void memWr(Memory*,unsigned short,unsigned char);

void memSetBank(Memory*,int,int,int);

void memSetPage(Memory*,int,int,char*);
void memGetPage(Memory*,int,int,char*);

int memGet(Memory*,int);
void memSet(Memory*,int,int);

unsigned char* memGetPagePtr(Memory*,int,int);

#if __cplusplus
}
#endif

#endif
