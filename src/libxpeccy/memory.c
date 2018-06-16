#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Memory* memCreate() {
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	memset(mem, 0x00, sizeof(Memory));
	mem->romMask = 0x03;
	memSetSize(mem, 48);
	memSetBank(mem, MEM_BANK0, MEM_ROM, 0, NULL, NULL, NULL);
	memSetBank(mem, MEM_BANK1, MEM_RAM, 5, NULL, NULL, NULL);
	memSetBank(mem, MEM_BANK2, MEM_RAM, 2, NULL, NULL, NULL);
	memSetBank(mem, MEM_BANK3, MEM_RAM, 0, NULL, NULL, NULL);
	return mem;
}

void memDestroy(Memory* mem) {
	free(mem);
}

MemPage* memGetBankPtr(Memory* mem, unsigned short adr) {
	return &mem->map[adr >> 14];
}

unsigned char memRd(Memory* mem, unsigned short adr) {
#if MEM_SMALL_PAGES
	MemPage* ptr = &mem->map[(adr & 0xff00) >> 8];
	return ptr->rd ? ptr->rd(adr, ptr->data) : 0xff;
#else
	MemPage* ptr = &mem->map[(adr & 0xc000) >> 14];
	return ptr->rd ? ptr->rd(adr, ptr->data) : 0xff;
#endif
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
#if MEM_SMALL_PAGES
	MemPage* ptr = &mem->map[(adr & 0xff00) >> 8];
	if (ptr->wr) ptr->wr(adr, val, ptr->data);
#else
	MemPage* ptr = &mem->map[(adr & 0xc000) >> 14];
	if (ptr->wr) ptr->wr(adr, val, ptr->data);
#endif
}

typedef struct {
	int s1k;
	int s16k;
	int s256;
} xMemSizeInfo;

static xMemSizeInfo memSizeTab[] = {
	{48, 0x07, 0x1ff},
	{64, 0x07, 0x1ff},
	{128, 0x07, 0x1ff},
	{256, 0x0f, 0x3ff},
	{512, 0x1f, 0x7ff},
	{1024, 0x3f, 0xfff},
	{2048, 0x7f, 0x1fff},
	{4096, 0xff, 0x3fff},
	{-1, 0x07, 0x1ff}		// default:128K = 8x16K = 512x256
};

void memSetSize(Memory* mem, int val) {
	if (mem->memSize == val) return;
	int idx = 0;
	while ((memSizeTab[idx].s1k > 0) && (memSizeTab[idx].s1k != val))
		idx++;
	mem->memSize = memSizeTab[idx].s1k;
#if MEM_SMALL_PAGES
	mem->memMask = memSizeTab[idx].s256;
#else
	mem->memMask = memSizeTab[idx].s16k;
#endif
}

unsigned char memPageRd(unsigned short adr, void* data) {
	unsigned char* ptr = (unsigned char*)data;
#if MEM_SMALL_PAGES
	return ptr[adr & 0xff];
#else
	return ptr[adr & 0x3fff];
#endif
}

void memPageWr(unsigned short adr, unsigned char val, void* data) {
	unsigned char* ptr = (unsigned char*)data;
#if MEM_SMALL_PAGES
	ptr[adr & 0xff] = val;
#else
	ptr[adr & 0x3fff] = val;
#endif
}

void memSetBank(Memory* mem, int page, int type, int num, extmrd rd, extmwr wr, void* data) {
	MemPage pg;
	pg.type = type;
	switch(type) {
		case MEM_ROM:
			pg.num = num & mem->romMask;
			pg.data = mem->romData + (pg.num << 14);	// ptr of page begin
			pg.rd = memPageRd;
			pg.wr = NULL;					// write disabled
			break;
		case MEM_RAM:
			pg.num = num & mem->memMask;
			pg.data = mem->ramData + (pg.num << 14);
			pg.rd = memPageRd;
			pg.wr = memPageWr;
			break;
		default:
			pg.num = num;
			pg.data = data;
			pg.rd = rd;
			pg.wr = wr;
			break;
	}
	mem->map[page & 3] = pg;
}

void memSetPageData(Memory* mem, int type, int page, char* src) {
	switch(type) {
		case MEM_ROM:
			page &= 0x1f;
			memcpy(mem->romData + (page << 14), src, 0x4000);
			break;
		case MEM_RAM:
			page &= 0xff;
			memcpy(mem->ramData + (page << 14), src, 0x4000);
			break;
	}
}

void memGetPageData(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			page &= 0x1f;
			memcpy(dst, mem->romData + (page << 14), 0x4000);
			break;
		case MEM_RAM:
			page &= 0xff;
			memcpy(dst, mem->ramData + (page << 14), 0x4000);
			break;
	}
}

unsigned char* memGetPagePtr(Memory* mem, int type, int page) {
	unsigned char* res = NULL;
	switch (type) {
		case MEM_ROM:
			page &= 0x1f;
			res = mem->romData + (page << 14);
			break;
		case MEM_RAM:
			page &= 0xff;
			res = mem->ramData + (page << 14);
			break;
	}
	return res;
}

// get memory cell address
xAdr memGetXAdr(Memory* mem, unsigned short adr) {
	xAdr xadr;
	MemPage* ptr = &mem->map[(adr >> 14) & 3];
	xadr.type = ptr->type;
	xadr.bank = ptr->num;
	xadr.adr = adr;					// 16bit adr
	xadr.abs = (xadr.bank << 14) | (adr & 0x3fff);	// absolute addr
	return xadr;
}

int memFindAdr(Memory* mem, int type, int fadr) {
	int i;
	int adr = -1;
	for (i = 0; i < 4; i++) {
		if ((mem->map[i].type == type) && (mem->map[i].num == (fadr >> 14))) {
			adr = (i << 14) | (fadr & 0x3fff);
			break;
		}
	}
	return adr;
}
