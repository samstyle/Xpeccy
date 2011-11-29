#include "memory.h"

#include <stdlib.h>
#include <string.h>

// NEW

struct Memory {
	uint8_t ram[64][16384];
	uint8_t rom[32][16384];
	uint8_t *pt0,*pt1,*pt2,*pt3;
	uint8_t cram,crom;
	int32_t	mask;
	int32_t profMask;	// profrom (0 - 64K, 1 - 128K, 3 - 256K)
	RomSet *romset;
};

Memory* memCreate() {
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	mem->pt0 = mem->rom[0];
	mem->pt1 = mem->ram[5];
	mem->pt2 = mem->ram[2];
	mem->pt3 = mem->ram[0];
	mem->cram = 0;
	mem->crom = 0;
	mem->mask = 0;
	mem->romset = NULL;
	return mem;
}

void memDestroy(Memory* mem) {
	free(mem);
}

uint8_t memRd(Memory* mem, uint16_t adr) {
	uint8_t res;
	switch (adr & 0xc000) {
		case 0x0000: res = *(mem->pt0 + (adr & 0x3fff)); break;
		case 0x4000: res = *(mem->pt1 + (adr & 0x3fff)); break;
		case 0x8000: res = *(mem->pt2 + (adr & 0x3fff)); break;
		default: res = *(mem->pt3 + (adr & 0x3fff)); break;
	}
	return res;
}

void memWr(Memory* mem, uint16_t adr, uint8_t val) {
	switch (adr & 0xc000) {
		case 0x0000:
			if (mem->crom==0xff) {
				*(mem->pt0 + (adr & 0x3fff)) = val;
			}
			break;
		case 0x4000: *(mem->pt1 + (adr & 0x3fff)) = val; break;
		case 0x8000: *(mem->pt2 + (adr & 0x3fff)) = val; break;
		default: *(mem->pt3 + (adr & 0x3fff)) = val; break;
	}
}

int memGet(Memory* mem, int wut) {
	int res = 0;
	switch (wut) {
		case MEM_PROFMASK: res = mem->profMask; break;
		case MEM_ROM: res = mem->crom; break;
		case MEM_RAM: res = mem->cram; break;
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
		case MEM_PROFMASK: mem->profMask = val; break;
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

void memSetRomset(Memory* mem, RomSet* rs) {
	mem->romset = rs;
}

RomSet* memGetRomset(Memory* mem) {
	return mem->romset;
}

void memSetBank(Memory* mem, int bank, int wut, int nr) {
	switch (bank) {
		case MEM_BANK0:
			switch (wut) {
				case MEM_ROM:
					mem->crom = nr;
					mem->pt0 = (nr == 0xff) ? mem->ram[0] : mem->rom[nr];
					break;
				case MEM_RAM:
					mem->pt0 = mem->ram[nr & mem->mask];
					break;
			}
			break;
		case MEM_BANK1:
			switch (wut) {
				case MEM_ROM: mem->pt1 = mem->rom[nr]; break;
				case MEM_RAM: mem->pt1 = mem->ram[nr & mem->mask]; break;
			}
			break;
		case MEM_BANK2:
			switch (wut) {
				case MEM_ROM: mem->pt2 = mem->rom[nr]; break;
				case MEM_RAM: mem->pt2 = mem->ram[nr & mem->mask]; break;
			}
			break;
		case MEM_BANK3:
			switch (wut) {
				case MEM_ROM: mem->pt3 = mem->rom[nr]; break;
				case MEM_RAM:
					mem->cram = nr;
					mem->pt3 = mem->ram[nr & mem->mask];
					break;
			}
			break;
	}
}

void memSetPage(Memory* mem, int type, int page, char* src) {
	switch(type) {
		case MEM_ROM:
			memcpy(mem->rom[page],src,0x4000);
			break;
		case MEM_RAM:
			memcpy(mem->ram[page],src,0x4000);
			break;
	}
}

void memGetPage(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			memcpy(dst,mem->rom[page],0x4000);
			break;
		case MEM_RAM:
			memcpy(dst,mem->ram[page],0x4000);
			break;
	}
}

uint8_t* memGetPagePtr(Memory* mem, int type, int page) {
	uint8_t* res = NULL;
	switch (type) {
		case MEM_ROM:
			res = mem->rom[page];
			break;
		case MEM_RAM:
			res = mem->ram[page];
			break;
	}
	return res;
}