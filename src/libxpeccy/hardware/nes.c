#include "hardware.h"

// NOTE: NES is fukkeen trash. why did i start this?

// NOTE: Video PPU must have access to cartridge memory, as long as CPU
// NOTE: Cartridges have potentially 255+ mappers

unsigned char nesChrRd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res;
	if (comp->slot->chrrom == NULL) {
		res = comp->vid->ppu->mem[adr & 0x1fff];
	} else if (adr & 0x1000) {
		res = comp->slot->chrrom[(comp->slot->memMap[3] << 12) | (adr & 0xfff)];
	} else {
		res = comp->slot->chrrom[(comp->slot->memMap[2] << 12) | (adr & 0xfff)];
	}
	return res;
}

void nesChrWr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (comp->slot->chrrom == NULL) {
		comp->vid->ppu->mem[adr & 0x1fff] = val;
	} else {
		// CHR-RAM?
	}
}

void nesReset(Computer* comp) {
	comp->vid->ppu->mrd = nesChrRd;
	comp->vid->ppu->mwr = nesChrWr;
	comp->vid->ppu->data = comp;
	ppuReset(comp->vid->ppu);
	vidSetMode(comp->vid, VID_NES);
	comp->slot->memMap[0] = 0;	// prg 2x16K pages
	comp->slot->memMap[1] = comp->slot->memMask >> 14;
	comp->slot->memMap[2] = 0;	// chr 2x4K pages
	comp->slot->memMap[3] = 1;
	comp->slot->memMap[7] = 0;	// ram
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
				ppu->latch = 0;
				break;
			case 4:
				res = ppu->oam[ppu->oamadr & 0xff];
				break;
			case 7:
				res = ppuRead(ppu);
				break;
		}
	} else if (adr < 0x5000) {
		// audio, dma, io registers (0x17, other is unused)
		switch (adr) {
			case 0x4016:		// joystick 1
				res = comp->nes.priJoy & 1;
				comp->nes.priJoy >>= 1;
				break;
			case 0x4017:		// joystick 2
				res = comp->nes.secJoy & 1;
				comp->nes.secJoy >>= 1;
				break;

		}
	} else if (adr < 0x6000) {
		// expansion rom (4K)
	} else if (comp->slot->data) {
		// cartrige ram (8K)
		res = comp->slot->ram[adr & 0x1fff];
	}
	return res;
}

void nesMMwr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		comp->mem->ramData[adr & 0x7ff] = val;
	} else if (adr < 0x4000) {
		// write video registers
		nesPPU* ppu = comp->vid->ppu;
		switch (adr & 7) {
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
				ppu->oam[ppu->oamadr & 0xff] = val;
				ppu->oamadr++;
				break;
			case 5:
				ppu->latch ^= 1;
				if (ppu->latch) {
					ppu->scx = val;
				} else {
					ppu->scy = val;
				}
				break;
			case 6:
				ppu->latch ^= 1;
				if (ppu->latch) {
					ppu->vah = val;
				} else {
					ppu->val = val;
				}
				break;
			case 7:
				ppuWrite(ppu, val);
				break;
		}
	} else if (adr < 0x5000) {
		// write audio/dma/io?
		switch (adr) {
			case 0x4014:		// OAMDMA
				adr = (val << 8);
				do {
					comp->vid->ppu->oam[comp->vid->ppu->oamadr & 0xff] = memRd(comp->mem, adr);
					comp->vid->ppu->oamadr++;
					adr++;
				} while (adr & 0xff);
				comp->cpu->t += 0x200;
				break;
			case 0x4016:
				if (val & 1) {		// b0: 0-1-0 = reload gamepads state
					comp->nes.priJoy = comp->nes.priPadState;
					comp->nes.secJoy = comp->nes.secPadState;
				}
				break;
			case 0x4017:
				break;
			default:
				//printf("write %.4X,%.2X\n",adr,val);
				break;
		}
	} else if (adr < 0x6000) {
		// nothing?
	} else if (comp->slot->data) {
		// sram 8K
		comp->slot->ram[adr & 0x1fff] = val;
	}
}

unsigned char nesSLrd(unsigned short adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, adr);
	//return slot->core->rd(slot, adr);
}

void nesSLwr(unsigned short adr, unsigned char val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, adr, val);
	//slot->core->wr(slot, adr, val);
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, MEM_BANK0, MEM_EXT, 0, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK1, MEM_EXT, 1, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK2, MEM_SLOT, 0, nesSLrd, nesSLwr, comp->slot);
	memSetBank(comp->mem, MEM_BANK3, MEM_SLOT, 1, nesSLrd, nesSLwr, comp->slot);
}

void nesSync(Computer* comp, long ns) {
	if (comp->vid->vbstrb) {			// @ start of VBlank
		comp->vid->vbstrb = 0;
		if (comp->vid->ppu->inten) {
			comp->cpu->intrq |= MOS6502_INT_NMI;	// request NMI...
		}
	}
	comp->nes.clock.ns += ns;
	if (comp->nes.clock.ns >= comp->nes.clock.per) {
		comp->nes.clock.ns -= comp->nes.clock.per;
		comp->nes.clock.lev ^= 1;
	}
}

unsigned char nesMemRd(Computer* comp, unsigned short adr, int m1) {
	return memRd(comp->mem, adr);
}

void nesMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}

// keypress

typedef struct {
	signed int id;
	int mask;
} nesKey;

nesKey nesKeyMap[8] = {
	{XKEY_Z,1},		// A
	{XKEY_X,2},		// B
	{XKEY_SPACE,4},		// select
	{XKEY_ENTER,8},		// start
	{XKEY_UP,16},		// up
	{XKEY_DOWN,32},		// down
	{XKEY_LEFT,64},		// left
	{XKEY_RIGHT,128}	// right
};

int nesGetInputMask(signed int keyid) {
	int idx = 0;
	unsigned char mask = 0;
	while (idx < 8) {
		if (nesKeyMap[idx].id == keyid) {
			mask = nesKeyMap[idx].mask;
		}
		idx++;
	}
	return mask;
}

void nes_keyp(Computer* comp, keyEntry ent) {
	int mask = nesGetInputMask(ent.key);
	if (mask == 0) return;
	comp->nes.priPadState |= mask;
}

void nes_keyr(Computer* comp, keyEntry ent) {
	int mask = nesGetInputMask(ent.key);
	if (mask == 0) return;
	comp->nes.priPadState &= ~mask;
}
