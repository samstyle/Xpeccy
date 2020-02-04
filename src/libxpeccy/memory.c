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

unsigned char memRd(Memory* mem, unsigned short adr) {
	unsigned char res = 0xff;
	MemPage* ptr = &mem->map[(adr >> 8) & 0xff];
	if (ptr->rd)
		res = ptr->rd(adr, ptr->data);
	return res;
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
	MemPage* ptr = &mem->map[(adr >> 8) & 0xff];
	if (ptr->wr)
		ptr->wr(adr, val, ptr->data);
}

unsigned char memStdRd(unsigned short adr, void* data) {
	unsigned char* ptr = (unsigned char*)data;
	return ptr[adr & 0xff];
}

void memStdWr(unsigned short adr, unsigned char val, void* data) {
	unsigned char* ptr = (unsigned char*)data;
	ptr[adr & 0xff] = val;
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
		pg.rd = rd ? rd : memStdRd;		// replace std callback if not NULL
		pg.wr = wr ? wr : memStdWr;
	} else if (type == MEM_ROM) {
		pg.rd = rd ? rd : memStdRd;
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
			pg.data = mem->ramData + ((pg.num << 8) & mem->ramMask);
		} else if (type == MEM_ROM) {
			pg.data = mem->romData + ((pg.num << 8) & mem->romMask);
		} else {
			pg.data = NULL;
		}
		mem->map[page] = pg;
		page++;
		pg.num++;
		cnt--;
	}
}

// get page data
/*
void memGetData(Memory* mem, int type, int sz, int page, char* dst) {
	if (type == MEM_RAM) {
		memcpy(dst, mem->ramData + ((page * sz) & mem->ramMask), sz);
	} else if (type == MEM_ROM) {
		memcpy(dst, mem->romData + ((page * sz) & mem->romMask), sz);
	}
}
*/

// set page data
void memPutData(Memory* mem, int type, int page, int sz, char* src) {
	if (type == MEM_RAM) {
		memcpy(mem->ramData + ((page * sz) & mem->ramMask), src, sz);
	} else if (type == MEM_ROM) {
		memcpy(mem->romData + ((page * sz) & mem->romMask), src, sz);
	}
}

// get cell full address
xAdr memGetXAdr(Memory* mem, unsigned short adr) {
	xAdr xadr;
	MemPage* ptr = &mem->map[(adr >> 8) & 0xff];
	xadr.type = ptr->type;				// page type
	xadr.bank = ptr->num;				// 256 page number
	xadr.adr = adr;					// cpu adr
	xadr.abs = (ptr->num << 8) | (adr & 0xff);	// absolute addr
	return xadr;
}

int memFindAdr(Memory* mem, int type, int fadr) {
	int i;
	int adr = -1;
	for (i = 0; i < 256; i++) {
		if ((mem->map[i].type == type) && (mem->map[i].num == (fadr >> 8))) {
			adr = (i << 8) | (fadr & 0xff);
			break;
		}
	}
	return adr;
}
