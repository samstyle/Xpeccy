#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Memory* memCreate() {
	int i;
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	memset(mem->ramFlag, 0x00, 0x400000);
	memset(mem->romFlag, 0x00, 0x80000);
	mem->romMask = 0x03;
	for (i = 0; i < 32; i++) {
		mem->rom[i].type = MEM_ROM;
		mem->rom[i].num = i & 0xff;
		mem->rom[i].dptr = mem->romData + (i << 14);
		mem->rom[i].fptr = mem->romFlag + (i << 14);
	}
	for (i = 0; i < 256; i++) {
		mem->ram[i].type = MEM_RAM;
		mem->ram[i].num = i & 0xff;
		mem->ram[i].dptr = mem->ramData + (i << 14);
		mem->ram[i].fptr = mem->ramFlag + (i << 14);
	}
	memSetSize(mem,48);
	mem->pt[0] = &mem->rom[0];
	mem->pt[1] = &mem->ram[5];
	mem->pt[2] = &mem->ram[2];
	mem->pt[3] = &mem->ram[0];

	return mem;
}

void memDestroy(Memory* mem) {
	free(mem->ramData);
	free(mem->ramFlag);
	free(mem->romData);
	free(mem->romFlag);
	free(mem);
}

MemPage* memGetBankPtr(Memory* mem, unsigned short adr) {
	return mem->pt[adr >> 14];
}

MemPage* ptr;

unsigned char memRd(Memory* mem, unsigned short adr) {
	ptr = mem->pt[adr >> 14];
	mem->flag |= (ptr->fptr[adr & 0x3fff] & MEM_BRK_RD);
	return ptr->dptr[adr & 0x3fff];
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
	ptr = mem->pt[adr >> 14];
	mem->flag |= (ptr->fptr[adr & 0x3fff] & MEM_BRK_WR);
	if (ptr->type == MEM_RAM) ptr->dptr[adr & 0x3fff] = val;
}

unsigned char* memGetFptr(Memory* mem, unsigned short adr) {
	return mem->pt[adr >> 14]->fptr + (adr & 0x3fff);
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
	if (wut == MEM_ROM) nr &= mem->romMask;
	if (wut == MEM_RAM) nr &= mem->memMask;
	switch (bank) {
		case MEM_BANK0:
			switch (wut) {
				case MEM_ROM: mem->pt[0] = (nr == 0xff) ? &mem->ram[0] : &mem->rom[nr]; break;
				case MEM_RAM: mem->pt[0] = &mem->ram[nr]; break;
			}
			break;
		case MEM_BANK1:
			switch (wut) {
				case MEM_ROM: mem->pt[1] = &mem->rom[nr]; break;
				case MEM_RAM: mem->pt[1] = &mem->ram[nr]; break;
			}
			break;
		case MEM_BANK2:
			switch (wut) {
				case MEM_ROM: mem->pt[2] = &mem->rom[nr]; break;
				case MEM_RAM: mem->pt[2] = &mem->ram[nr]; break;
			}
			break;
		case MEM_BANK3:
			switch (wut) {
				case MEM_ROM: mem->pt[3] = &mem->rom[nr]; break;
				case MEM_RAM: mem->pt[3] = &mem->ram[nr]; break;
			}
			break;
	}
}

void memSetPage(Memory* mem, int type, int page, char* src) {
	switch(type) {
		case MEM_ROM:
			memcpy(mem->rom[page & 31].dptr,src,0x4000);
			break;
		case MEM_RAM:
			memcpy(mem->ram[page & 255].dptr,src,0x4000);
			break;
	}
}

void memGetPage(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			memcpy(dst,mem->rom[page & 31].dptr,0x4000);
			break;
		case MEM_RAM:
			memcpy(dst,mem->ram[page & 255].dptr,0x4000);
			break;
	}
}

unsigned char* memGetPagePtr(Memory* mem, int type, int page) {
	unsigned char* res = NULL;
	switch (type) {
		case MEM_ROM:
			res = mem->rom[page & 31].dptr;
			break;
		case MEM_RAM:
			res = mem->ram[page & 255].dptr;
			break;
	}
	return res;
}
