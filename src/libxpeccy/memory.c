#include "memory.h"

#include <stdlib.h>
#include <string.h>

// NEW

#include <stdio.h>

Memory* memCreate() {
	int i;
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	mem->romMask = 0x03;
	memset(mem->romFlag,0x00,0x80000);
	memset(mem->ramFlag,0x00,0x400000);
	for (i = 0; i < 32; i++) {
		mem->rom[i].type = MEM_ROM;
		mem->rom[i].num = i & 0xff;
//		mem->rom[i].flags = MEM_RDONLY;
		mem->rom[i].data = mem->romData + (i << 14);
		mem->rom[i].flag = mem->romFlag + (i << 14);
	}
	for (i = 0; i < 256; i++) {
		mem->ram[i].type = MEM_RAM;
		mem->ram[i].num = i & 0xff;
//		mem->ram[i].flags = 0;
		mem->ram[i].data = mem->ramData + (i << 14);
		mem->ram[i].flag = mem->ramFlag + (i << 14);
	}
	mem->flags = MEM_ROM_WP;
	memSetSize(mem,48);
	mem->pt[0] = &mem->rom[0];
	mem->pt[1] = &mem->ram[5];
	mem->pt[2] = &mem->ram[2];
	mem->pt[3] = &mem->ram[0];

	return mem;
}

void memDestroy(Memory* mem) {
	free(mem);
}

MemPage* memGetBankPtr(Memory* mem, unsigned short adr) {
	return mem->pt[adr >> 14];
//	if (adr < 0x4000) return mem->pt0;
//	if (adr < 0x8000) return mem->pt1;
//	if (adr < 0xc000) return mem->pt2;
//	return mem->pt3;
}

MemPage* ptr;

unsigned char memRd(Memory* mem, unsigned short adr) {
//	if (adr < 0x4000) ptr = mem->pt0;
//	else if (adr < 0x8000) ptr = mem->pt1;
//	else if (adr < 0xc000) ptr = mem->pt2;
//	else ptr = mem->pt3;
	ptr = mem->pt[adr >> 14];
	mem->flags = ptr->flag[adr & 0x3fff];
	return ptr->data[adr & 0x3fff];
}

void memWr(Memory* mem, unsigned short adr, unsigned char val) {
//	if (adr < 0x4000) ptr = mem->pt0;
//	else if (adr < 0x8000) ptr = mem->pt1;
//	else if (adr < 0xc000) ptr = mem->pt2;
//	else ptr = mem->pt3;
	ptr = mem->pt[adr >> 14];
	mem->flags = ptr->flag[adr & 0x3fff];
	if (ptr->type == MEM_RAM) ptr->data[adr & 0x3fff] = val;
}

unsigned char memGetCellFlags(Memory* mem, unsigned short adr) {
	MemPage* ptr = memGetBankPtr(mem,adr);
	return (ptr->flag[adr & 0x3fff]);
}

void memSwitchCellFlags(Memory* mem, unsigned short adr, unsigned char msk) {
	MemPage* ptr = memGetBankPtr(mem,adr);
	ptr->flag[adr & 0x3fff] ^= msk;
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
/*
	for (int i = 0; i < 256; i++) {
		if (i <= mem->memMask) {
			if (mem->ram[i].data == NULL) {
				mem->ram[i].data = malloc(0x4000 * sizeof(unsigned char));
				mem->ram[i].flag = malloc(0x4000 * sizeof(unsigned char));
				memset(mem->ram[i].data,0x00,0x4000 * sizeof(unsigned char));
				memset(mem->ram[i].flag,0x00,0x4000 * sizeof(unsigned char));
			}
		} else {
			if (mem->ram[i].data != NULL) {
				free(mem->ram[i].data);
				mem->ram[i].data = NULL;
			}
			if (mem->ram[i].flag != NULL) {
				free(mem->ram[i].flag);
				mem->ram[i].flag = NULL;
			}
		}
	}
*/
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
			memcpy(mem->rom[page & 31].data,src,0x4000);
			break;
		case MEM_RAM:
			memcpy(mem->ram[page & 63].data,src,0x4000);
			break;
	}
}

void memGetPage(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			memcpy(dst,mem->rom[page & 31].data,0x4000);
			break;
		case MEM_RAM:
			memcpy(dst,mem->ram[page & 63].data,0x4000);
			break;
	}
}

unsigned char* memGetPagePtr(Memory* mem, int type, int page) {
	unsigned char* res = NULL;
	switch (type) {
		case MEM_ROM:
			res = &mem->rom[page & 31].data[0];
			break;
		case MEM_RAM:
			res = &mem->ram[page & 63].data[0];
			break;
	}
	return res;
}
