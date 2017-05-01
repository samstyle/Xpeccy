#include "hardware.h"

// NOTE: NES is fukkeen trash. why did i start this?

// NOTE: Video PPU must have access to cartridge memory, as long as CPU
// NOTE: Cartridges have potentially 255 mappers

void nesReset(Computer* comp) {
	comp->cpu->pc = 0xfffc;
}

unsigned char nesMMrd(unsigned short adr, void* data) {
	unsigned char* mem = (unsigned char*)data;
	unsigned char res = 0xff;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		res = mem[adr & 0x7ff];
	} else if (adr < 0x4000) {
		// 8 video registers (use Video::nesPPU rd function)
	} else if (adr < 0x5000) {
		// audio, dma, io registers (0x17, other is unused)
	} else if (adr < 0x6000) {
		// expansion rom (4K)
	} else {
		// sram (8K)
	}
	return res;
}

void nesMMwr(unsigned short adr, unsigned char val, void* data) {
	unsigned char* mem = (unsigned char*)data;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		mem[adr & 0x7ff] = val;
	} else if (adr < 0x4000) {
		// write video registers
	} else if (adr < 0x5000) {
		// write audio/dma/io?
	} else if (adr < 0x6000) {
		// nothing?
	} else {
		// sram 8K
	}
}

unsigned char nesSLrd(unsigned short adr, void* data) {
//	xCartridge* slot = (xCartridge*)data;
	unsigned char res = 0;
	if (adr & 0x2000) {
		adr &= 0x3fff;
		// c000..ffff : last(?) 16K of cartrige
	} else {
		adr &= 0x3fff;
		// 8000..bfff : maped page(s) of cartrige
	}
	return res;
}

void nesSLwr(unsigned short adr, unsigned char val, void* data) {
//	xCartridge* slot = (xCartridge*)data;
	// write to slot? (maybe bank switching)
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, MEM_BANK0, MEM_EXT, 0, nesMMrd, nesMMwr, comp->mem->ramData);
	memSetBank(comp->mem, MEM_BANK1, MEM_EXT, 1, nesMMrd, nesMMwr, comp->mem->ramData);
	memSetBank(comp->mem, MEM_BANK2, MEM_SLOT, 0, nesSLrd, nesSLwr, comp->slot->data);
	memSetBank(comp->mem, MEM_BANK3, MEM_SLOT, 1, nesSLrd, nesSLwr, comp->slot->data);
}

void nesSync(Computer* comp, long ns) {

}

unsigned char nesMemRd(Computer* comp, unsigned short adr, int m1) {
	unsigned char res = 0xff;

	return res;
}

void nesMemWr(Computer* comp, unsigned short adr, unsigned char val) {

}
