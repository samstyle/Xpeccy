#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Memory* memCreate() {
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	memset(mem, 0x00, sizeof(Memory));
	memSetSize(mem, MEM_128K, MEM_16K);
	memSetBank(mem, 0x00, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);
	memSetBank(mem, 0x40, MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(mem, 0x80, MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(mem, 0xc0, MEM_RAM, 0, MEM_16K, NULL, NULL, NULL);
	mem_set_bus(mem, 16);
	mem->snapath = NULL;
	return mem;
}

void memDestroy(Memory* mem) {
	free(mem);
}

void mem_set_path(Memory* mem, const char* path) {
	mem->snapath = realloc(mem->snapath, strlen(path) + 1);
	strcpy(mem->snapath, path);
}

// return nearest 2^n greater than argument
int getNearPower(int src) {
	int dst = 1;
	while (dst < src)
		dst <<= 1;
	return dst;
}

// correct value to min/max limits
int toLimits(int src, int min, int max) {
	if (src < min) return min;
	if (src > max) return max;
	return src;
}

void mem_set_bus(Memory* mem, int bw) {
	bw = toLimits(bw, 8, 24);
	mem->busmask = (1 << bw) - 1;		// FFFF for 16, FFFFFF for 24...
	int sz = 1 << (bw - 8);			// memory map contains 256 pages (each: 256 bytes for 16bit, 65546 bytes for 24 bit)
	mem->pgsize = sz;
	mem->pgshift = 8;
	mem->pgmask = sz - 1;
	while (sz > 256) {
		mem->pgshift++;
		sz >>= 1;
	}
}

void memSetSize(Memory* mem, int ramSz, int romSz) {
//	printf("setMemSize %i %i\n",ramSz,romSz);
	if (ramSz > 0) {
		ramSz = toLimits(ramSz, MEM_256, MEM_4M);
		ramSz = getNearPower(ramSz);
		mem->ramSize = ramSz;
		mem->ramMask = ramSz - 1;
	}
	if (romSz > 0) {
		romSz = toLimits(romSz, MEM_256, MEM_512K);
		romSz = getNearPower(romSz);
		mem->romSize = romSz;
		mem->romMask = romSz - 1;
	}
}

int memRd(Memory* mem, int adr) {
	int res = -1;
	MemPage* ptr = &mem->map[(adr >> mem->pgshift) & 0xff];
	if (ptr->rd)
		res = ptr->rd(adr, ptr->data);
	return res;
}

void memWr(Memory* mem, int adr, int val) {
	MemPage* ptr = &mem->map[(adr >> mem->pgshift) & 0xff];
	if (ptr->wr)
		ptr->wr(adr, val, ptr->data);
}

// TODO: remove memstdrd/wr, let every hw set standard callbacks by itself
int memStdRd(int adr, void* data) {
	unsigned char* ptr = (unsigned char*)data;
	return ptr[adr & 0xff];
}

void memStdWr(int adr, int val, void* data) {
	unsigned char* ptr = (unsigned char*)data;
	ptr[adr & 0xff] = val & 0xff;
}

void memSetBank(Memory* mem, int page, int type, int bank, int siz, extmrd rd, extmwr wr, void* data) {
	int cnt = 1;
	while (siz > MEM_256) {		// calculate 256b pages count and correct bank number to 256b
		cnt <<= 1;
		bank <<= 1;
		siz >>= 1;
	}
	MemPage pg;
	pg.type = type;
	pg.num = bank;
	if (type == MEM_RAM) {
		pg.rd = rd ? rd : (data ? NULL : memStdRd);	// (rd || data) ?
		pg.wr = wr ? wr : (data ? NULL : memStdWr);
	} else if (type == MEM_ROM) {
		pg.rd = rd ? rd : (data ? NULL : memStdRd);
		pg.wr = wr;
	} else {
		pg.rd = rd;
		pg.wr = wr;
		// pg.data = data;
	}
	while(cnt > 0) {
		page &= 0xff;
		if (data) {
			pg.data = data;
		} else if (type == MEM_RAM) {
			pg.data = mem->ramData + ((pg.num << mem->pgshift) & mem->ramMask);
		} else if (type == MEM_ROM) {
			pg.data = mem->romData + ((pg.num << mem->pgshift) & mem->romMask);
		} else {
			pg.data = NULL;
		}
		mem->map[page] = pg;
		page++;
		pg.num++;
		cnt--;
	}
}

MemPage* mem_get_page(Memory* mem, int adr) {
	return &mem->map[(adr >> mem->pgshift) & 0xff];
}

// set page data
void memPutData(Memory* mem, int type, int page, int sz, char* src) {
	if (type == MEM_RAM) {
		memcpy(mem->ramData + ((page * sz) & mem->ramMask), src, sz);
	} else if (type == MEM_ROM) {
		memcpy(mem->romData + ((page * sz) & mem->romMask), src, sz);
	}
}

// get cell full address
xAdr mem_get_xadr(Memory* mem, int adr) {
	adr &= mem->busmask;
	xAdr xadr;
	MemPage* ptr = mem_get_page(mem, adr);	// = &mem->map[(adr >> mem->pgshift) & 0xff];
	xadr.type = ptr->type;				// page type
	xadr.bank = ptr->num;				// 256(64K) page number
	xadr.adr = adr;					// cpu adr
	xadr.abs = (ptr->num << mem->pgshift) | (adr & mem->pgmask);	// absolute addr
	return xadr;
}

// convert cpu adr to full memory adr
int mem_get_phys_adr(Memory* mem, int adr) {
	MemPage* pg = mem_get_page(mem, adr);	// = &mem->map[(adr >> mem->pgshift) & 0xff];
	return (pg->num << mem->pgshift) | (adr & mem->pgmask);
}

// convert full memory type/adr to cpu adr, if its visible for cpu. return -1 if not visible
int memFindAdr(Memory* mem, int type, int fadr) {
	int i;
	int adr = -1;
	for (i = 0; i < 256; i++) {
		if ((mem->map[i].type == type) && (mem->map[i].num == (fadr >> mem->pgshift))) {
			adr = (i << mem->pgshift) | (fadr & mem->pgmask);
			break;
		}
	}
	return adr;
}
