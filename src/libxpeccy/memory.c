#include "memory.h"

#include <stdlib.h>
#include <string.h>

// NEW

Memory* memCreate() {
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	mem->pt0 = &mem->rom[0];
	mem->pt1 = &mem->ram[5];
	mem->pt2 = &mem->ram[2];
	mem->pt3 = &mem->ram[0];
	mem->cram = 0;
	mem->crom = 0;
	mem->mask = 0;
	int i;
	for (i = 0; i < 32; i++) {
		mem->rom[i].flags |= MEM_RDONLY;
		memset(mem->rom[i].flag,0x00,0x4000 * sizeof(unsigned char));
	}
	for (i = 0; i < 64; i++) {
		mem->ram[i].flags = 0;
		memset(mem->ram[i].flag,0x00,0x4000 * sizeof(unsigned char));
	}
	return mem;
}

void memDestroy(Memory* mem) {
	free(mem);
}

MemPage* memGetBankPtr(Memory* mem, unsigned short adr) {
	MemPage* ptr;
	switch (adr & 0xc000) {
		case 0x0000: ptr = mem->pt0; break;
		case 0x4000: ptr = mem->pt1; break;
		case 0x8000: ptr = mem->pt2; break;
		default: ptr = mem->pt3; break;
	}
	return ptr;
}

unsigned char memRd(Memory* mem, unsigned short adr) {
	unsigned char res;
	MemPage* ptr = memGetBankPtr(mem,adr);
	res = ptr->data[adr & 0x3fff];
	mem->flags = ptr->flag[adr & 0x3fff];
	return res;
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
	MemPage* ptr = memGetBankPtr(mem,adr);
	mem->flags = ptr->flag[adr & 0x3fff];
	if (ptr->flags & MEM_RDONLY) return;
	ptr->data[adr & 0x3fff] = val;
}

unsigned char memGetCellFlags(Memory* mem, unsigned short adr) {
	MemPage* ptr = memGetBankPtr(mem,adr);
	return (ptr->flag[adr & 0x3fff]);
}

void memSwitchCellFlags(Memory* mem, unsigned short adr, unsigned char msk) {
	MemPage* ptr = memGetBankPtr(mem,adr);
	ptr->flag[adr & 0x3fff] ^= msk;
}

int memGet(Memory* mem, int wut) {
	int res = 0;
	switch (wut) {
		case MEM_MEMSIZE:
			switch (mem->mask) {
				case 0x00: res = 48; break;
				case 0x07: res = 128; break;
				case 0x0f: res = 256; break;
				case 0x1f: res = 512; break;
				case 0x3f: res = 1024; break;
			}
			break;
	}
	return res;
}

void memSet(Memory* mem, int wut, int val) {
	switch (wut) {
		case MEM_MEMSIZE:
			switch (val) {
				case 48: mem->mask = 0x00; break;
				case 128: mem->mask = 0x07; break;
				case 256: mem->mask = 0x0f; break;
				case 512: mem->mask = 0x1f; break;
				case 1024: mem->mask = 0x3f; break;
			}
	}
}

void memSetBank(Memory* mem, int bank, int wut, int nr) {
	switch (bank) {
		case MEM_BANK0:
			switch (wut) {
				case MEM_ROM:
					mem->crom = nr;
					mem->pt0 = (nr == 0xff) ? &mem->ram[0] : &mem->rom[nr];
					break;
				case MEM_RAM:
					mem->pt0 = &mem->ram[nr & mem->mask];
					break;
			}
			break;
		case MEM_BANK1:
			switch (wut) {
				case MEM_ROM: mem->pt1 = &mem->rom[nr]; break;
				case MEM_RAM: mem->pt1 = &mem->ram[nr & mem->mask]; break;
			}
			break;
		case MEM_BANK2:
			switch (wut) {
				case MEM_ROM: mem->pt2 = &mem->rom[nr]; break;
				case MEM_RAM: mem->pt2 = &mem->ram[nr & mem->mask]; break;
			}
			break;
		case MEM_BANK3:
			switch (wut) {
				case MEM_ROM: mem->pt3 = &mem->rom[nr]; break;
				case MEM_RAM:
					mem->cram = nr;
					mem->pt3 = &mem->ram[nr & mem->mask];
					break;
			}
			break;
	}
}

void memSetPage(Memory* mem, int type, int page, char* src) {
	switch(type) {
		case MEM_ROM:
			memcpy(mem->rom[page].data,src,0x4000);
			break;
		case MEM_RAM:
			memcpy(mem->ram[page].data,src,0x4000);
			break;
	}
}

void memGetPage(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			memcpy(dst,mem->rom[page].data,0x4000);
			break;
		case MEM_RAM:
			memcpy(dst,mem->ram[page].data,0x4000);
			break;
	}
}

unsigned char* memGetPagePtr(Memory* mem, int type, int page) {
	unsigned char* res = NULL;
	switch (type) {
		case MEM_ROM:
			res = &mem->rom[page].data[0];
			break;
		case MEM_RAM:
			res = &mem->ram[page].data[0];
			break;
	}
	return res;
}
