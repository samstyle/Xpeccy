#include "hardware.h"

// NOTE: NES is fukkeen trash. why did i start this?

// NOTE: Video PPU must have access to cartridge memory, as long as CPU
// NOTE: Cartridges have potentially 255 mappers

void nesReset(Computer* comp) {
	printf("nes reset\n");
}

unsigned char nesMMrd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char* mem = comp->mem->ramData;
	unsigned char res = 0xff;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		res = mem[adr & 0x7ff];
	} else if (adr < 0x4000) {
		adr &= 7;
		nesPPU* ppu = comp->vid->ppu;
		switch(adr) {
			case 2:
				if (!comp->vid->vblank) res ^= 0x80;
				// TODO : b6:spr0 hit, b5:spr overflow
				ppu->sclatch = 0;
				ppu->valatch = 0;
				break;
			case 4:
				res = ppu->oam[ppu->oamadr & 0xff];
				break;
			case 7:
				if (ppu->vidadr < 0x2000) {
					res = ppu->mem[ppu->vidadr];
				} else if (ppu->vidadr < 0x3f00) {
					res = ppu->mem[(ppu->vidadr & 0xfff) | 0x2000];
				} else {
					res = ppu->mem[(ppu->vidadr & 0x1f) | 0x3f00];
				}
				ppu->vidadr += ppu->vadrinc;
				break;
		}
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
	Computer* comp = (Computer*)data;
	unsigned char* mem = comp->mem->ramData;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		mem[adr & 0x7ff] = val;
	} else if (adr < 0x4000) {
		adr &= 7;
		nesPPU* ppu = comp->vid->ppu;
		switch (adr) {
			case 0:		// PPUCTRL
				ppu->ntadr = 0x2000 | ((val & 3) << 10);
				ppu->vadrinc = (val & 0x04) ? 32 : 1;
				ppu->spadr = (val & 0x08) ? 0x1000 : 0x0000;
				ppu->bgadr = (val & 0x10) ? 0x1000 : 0x0000;
				ppu->bigspr = (val & 0x20) ? 1 : 0;
				ppu->master = (val & 0x40) ? 1 : 0;
				ppu->inten = (val & 0x80) ? 1 : 0;
				break;
			case 1:		// PPUMASK
				ppu->greyscale = (val & 0x01) ? 1 : 0;
				ppu->bgleft8 = (val & 0x02) ? 1 : 0;
				ppu->spleft8 = (val & 0x04) ? 1 : 0;
				ppu->bgen = (val & 0x08) ? 1 : 0;
				ppu->spen = (val & 0x10) ? 1 : 0;
				// TODO: b5,6,7 = color tint
				break;
			case 3:
				ppu->oamadr = val;
				break;
			case 4:
				if (comp->vid->vblank) {
					ppu->oam[ppu->oamadr & 0xff] = val;
				}
				ppu->oamadr++;
				break;
			case 5:
				ppu->sclatch ^= 1;
				if (ppu->sclatch) {
					ppu->scx = val;
				} else {
					ppu->scy = val;
				}
				break;
			case 6:
				ppu->valatch ^= 1;
				if (ppu->valatch) {
					ppu->vidadr = (ppu->vidadr & 0xff) | ((val & 0x3f) << 8);
				} else {
					ppu->vidadr = (ppu->vidadr & 0x3f00) | (val & 0xff);
				}
				break;
			case 7:
				if (ppu->vidadr < 0x2000) {
					ppu->mem[ppu->vidadr] = val;
				} else if (ppu->vidadr < 0x3f00) {
					ppu->mem[(ppu->vidadr & 0xfff) | 0x2000] = val;
				} else {
					ppu->mem[(ppu->vidadr & 0x1f) | 0x3f00] = val;
				}
				ppu->vidadr += ppu->vadrinc;
				break;
		}

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
	xCartridge* slot = (xCartridge*)data;
	return slot->core->rd(slot, adr);
}

void nesSLwr(unsigned short adr, unsigned char val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	slot->core->wr(slot, adr, val);
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, MEM_BANK0, MEM_EXT, 0, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK1, MEM_EXT, 1, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK2, MEM_SLOT, 0, nesSLrd, nesSLwr, comp->slot);
	memSetBank(comp->mem, MEM_BANK3, MEM_SLOT, 1, nesSLrd, nesSLwr, comp->slot);
}

void nesSync(Computer* comp, long ns) {

}

unsigned char nesMemRd(Computer* comp, unsigned short adr, int m1) {
	return memRd(comp->mem, adr);
}

void nesMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}
