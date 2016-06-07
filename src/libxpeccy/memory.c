#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Memory* memCreate() {
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	mem->romMask = 0x03;
	memSetSize(mem,48);
	memSetBank(mem, MEM_BANK0, MEM_ROM, 0);
	memSetBank(mem, MEM_BANK1, MEM_RAM, 5);
	memSetBank(mem, MEM_BANK2, MEM_RAM, 2);
	memSetBank(mem, MEM_BANK3, MEM_RAM, 0);
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
	MemPage* ptr = &mem->map[adr >> 14];
	if (ptr->type != MEM_EXT) return ptr->dptr[adr & 0x3fff];
	if (ptr->rd) return ptr->rd(adr, ptr->data);
	return 0xff;
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
	MemPage* ptr = &mem->map[adr >> 14];
	if (ptr->type == MEM_EXT) {
		if (ptr->wr) ptr->wr(adr, val, ptr->data);
	} else if (ptr->wren) {
		ptr->dptr[adr & 0x3fff] = val;
	}
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

void memSetBank(Memory* mem, int bank, int wut, unsigned char nr) {
	if (wut == MEM_ROM) {
		nr &= mem->romMask;
		mem->map[bank].type = MEM_ROM;
		mem->map[bank].num = nr;
		mem->map[bank].wren = 0;
		mem->map[bank].dptr = mem->romData + (nr << 14);
	} else if (wut == MEM_RAM) {
		nr &= mem->memMask;
		mem->map[bank].type = MEM_RAM;
		mem->map[bank].num = nr;
		mem->map[bank].wren = 1;
		mem->map[bank].dptr = mem->ramData + (nr << 14);
	}
}

void memSetExternal(Memory* mem, int bank, MemPage pg) {
	pg.type = MEM_EXT;
	pg.num = 0;
	mem->map[bank] = pg;
}

void memSetPage(Memory* mem, int type, int page, char* src) {
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

void memGetPage(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			page &= 0x1f;
			memcpy(dst, mem->romData + (page << 14), 0x4000);
			break;
		case MEM_RAM:
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
