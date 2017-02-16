#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Memory* memCreate() {
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	mem->romMask = 0x03;
	memSetSize(mem,48);
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

// MemPage* ptr;

unsigned char memRd(Memory* mem, unsigned short adr) {
	MemPage* ptr = &mem->map[(adr & 0xc000) >> 14];
	return ptr->rd ? ptr->rd(adr, ptr->data) : 0xff;
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
	MemPage* ptr = &mem->map[(adr & 0xc000) >> 14];
	if (ptr->wr) ptr->wr(adr, val, ptr->data);
}

void memSetSize(Memory* mem, int val) {
	if (mem->memSize == val) return;
	mem->memSize = val;
	switch (val) {
		case 48:
			mem->memMask = 0x07;	// need 7, cuz it uses pages 2,5,0
			break;
		case 128:
			mem->memMask = 0x07;
			break;
		case 256:
			mem->memMask = 0x0f;
			break;
		case 512:
			mem->memMask = 0x1f;
			break;
		case 1024:
			mem->memMask = 0x3f;
			break;
		case 2048:
			mem->memMask = 0x7f;
			break;
		case 4096:
			mem->memMask = 0xff;
			break;
		default:
			mem->memMask = 0x07;
			mem->memSize = 48;
			break;
	}
}

unsigned char memPageRd(unsigned short adr, void* data) {
	unsigned char* ptr = (unsigned char*)data;
	return ptr[adr & 0x3fff];
}

void memPageWr(unsigned short adr, unsigned char val, void* data) {
	unsigned char* ptr = (unsigned char*)data;
	ptr[adr & 0x3fff] = val;
}

void memSetBank(Memory* mem, int bank, int type, int page, extmrd rd, extmwr wr, void* data) {
	MemPage pg;
	pg.type = type;
	switch(type) {
		case MEM_ROM:
			pg.num = page & mem->romMask;
			pg.data = mem->romData + (pg.num << 14);	// ptr of page begin
			pg.rd = memPageRd;
			pg.wr = NULL;					// write disabled
			break;
		case MEM_RAM:
			pg.num = page & mem->memMask;
			pg.data = mem->ramData + (pg.num << 14);
			pg.rd = memPageRd;
			pg.wr = memPageWr;
			break;
		default:
			pg.num = page;
			pg.data = data;
			pg.rd = rd;
			pg.wr = wr;
			break;
	}
	mem->map[bank & 3] = pg;
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
