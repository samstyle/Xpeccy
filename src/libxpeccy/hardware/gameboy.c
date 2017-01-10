#include <string.h>

#include "hardware.h"

void gbReset(Computer* comp) {
	comp->gb.boot = 1;
	comp->msx.slotA.memMap[0] = 1;
	vidSetMode(comp->vid, VID_GBC);
	comp->gb.intMask = 0;
	comp->vid->gbc->inten.hblank = 0;
	comp->vid->gbc->inten.vblank = 0;
	comp->vid->gbc->inten.oam = 0;
	comp->vid->gbc->inten.lyc = 0;
}

// IO ports

// video modes:
// [mode 2 : ~80T][mode 3 : ~170T][mode 0 : ~206T] = 456T
// [mode 1] during VBlank

unsigned char gbIORd(Computer* comp, unsigned short port) {
	port &= 0x7f;
	unsigned char res = comp->gb.iomap[port];
	switch(port) {
		case 0x00:
			switch(comp->gb.iomap[0] & 0x30) {
				case 0x10: res = (comp->gb.buttons & 0xf0) >> 4; break;
				case 0x20: res = comp->gb.buttons & 0x0f; break;
			}
			break;
		case 0x24: break;
		case 0x40: break;
		case 0x41:
			res = comp->vid->gbc->lyequal ? 4 : 0;
			if (comp->vid->ray.y >= comp->vid->lay.scr.y) {		// mode 1 : vblank
				res |= 1;
			} else if (comp->vid->ray.x < 40) {			// mode 2 : oam reading. ~80T, 19us
				res |= 2;
			} else if (comp->vid->ray.x < 125) {			// mode 3 : oam/vmem rd. ~170T, 41uS
				res |= 3;
			}							// mode 0 : hblank. ~206T, 48.6uS
			break;
		case 0x42: break;
		case 0x43: break;
		case 0x44: res = comp->vid->ray.y; break;
		case 0x45: break;
		case 0x47: break;
		case 0x48: break;
		case 0x49: break;
		case 0x4a: break;
		case 0x4b: break;
		default:
			printf("GB: in %.4X\n",port);
			assert(0);
			break;
	}
	return res;
}

extern xColor iniCol[4];
void setGrayScale(xColor pal[256], int base, unsigned char val) {
	pal[base] = iniCol[val & 3];
	pal[base + 1] = iniCol[(val >> 2) & 3];
	pal[base + 2] = iniCol[(val >> 4) & 3];
	pal[base + 3] = iniCol[(val >> 6) & 3];
}

void gbIOWr(Computer* comp, unsigned short port, unsigned char val) {
	port &= 0x7f;
	comp->gb.iomap[port] = val;
	unsigned short sadr;
	unsigned short dadr;
	switch (port) {
// JOYSTICK
		// b4: 0:select crest
		// b5: 0:select buttons
		case 0x00: break;
// VIDEO
		case 0x40:
			comp->vid->gbc->lcdon = (val & 0x80) ? 1 : 0;
			comp->vid->gbc->winmapadr = (val & 0x40) ? 0x9c00 : 0x9800;
			comp->vid->gbc->winen = (val & 0x20) ? 1 : 0;
			comp->vid->gbc->altile = (val & 0x10) ? 0 : 1;			// signed tile num
			comp->vid->gbc->tilesadr = (val & 0x10) ? 0x8000 : 0x8800;
			comp->vid->gbc->bgmapadr = (val & 0x08) ? 0x9c00 : 0x9800;
			comp->vid->gbc->bigspr = (val & 0x04) ? 1 : 0;
			comp->vid->gbc->spren  = (val & 0x02) ? 1 : 0;
			comp->vid->gbc->bgen = (val & 0x01) ? 1 : 0;
			break;
		case 0x41:
			comp->vid->gbc->inten.lyc = (val & 0x40) ? 1 : 0;
			comp->vid->gbc->inten.oam = (val & 0x20) ? 1 : 0;
			comp->vid->gbc->inten.vblank = (val & 0x10) ? 1 : 0;
			comp->vid->gbc->inten.hblank = (val & 0x08) ? 1 : 0;
			break;
		case 0x42:
			comp->vid->gbc->sc.y = val;
			break;
		case 0x43:
			comp->vid->gbc->sc.x = val;
			break;
		case 0x44:
			comp->vid->gbc->ray->y = 0;			// TODO: writing will reset the counter (?)
			break;
		case 0x45:
			comp->vid->gbc->lyc = val;
			break;
		case 0x46:						// TODO: block CPU memory access for 160 microsec (except ff80..fffe)
			sadr = val << 8;
			dadr = 0;
			while (dadr < 0xa0) {
				comp->vid->gbc->oem[dadr] = memRd(comp->mem, sadr);
				sadr++;
				dadr++;
			}
			break;
		case 0x47:						// palete for bg/win
			setGrayScale(comp->vid->gbc->pal, 0, val);
			break;
		case 0x48:
			setGrayScale(comp->vid->gbc->pal, 0x40, val);	// object pal 0
			break;
		case 0x49:
			setGrayScale(comp->vid->gbc->pal, 0x44, val);	// object pal 1
			break;
		case 0x4a:
			comp->vid->gbc->win.y = val;
			break;
		case 0x4b:
			comp->vid->gbc->win.x = val - 7;
			break;
// SOUND
		// chan 1
		case 0x10: break;
		case 0x11: break;
		case 0x12: break;
		case 0x13: break;
		case 0x14: break;
		// chan 2
		case 0x16: break;
		case 0x17: break;
		case 0x18: break;
		case 0x19: break;
		// chan 3
		case 0x1a: break;
		case 0x1b: break;
		case 0x1c: break;
		case 0x1d: break;
		case 0x1e: break;
		// chan 4
		case 0x20: break;
		case 0x21: break;
		case 0x22: break;
		case 0x23: break;
		// control
		case 0x24: break;
		case 0x25: break;
		case 0x26: break;
// MISC
		case 0x50: comp->gb.boot = 0; break;

		default:
			if ((port & 0xf0) == 0x30) break;		// ff30..ff3f : wave pattern ram for chan 3
			printf("GB: out %.4X,%.2X\n",port,val);
			assert(0);
			break;
	}
}

// 0000..7fff : slot

unsigned char gbSlotRd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	xCartridge* slot = &comp->msx.slotA;
	unsigned char res = 0xff;
	int radr = adr & 0x3fff;
	if ((adr < 0x100) && comp->gb.boot) {
		res = comp->mem->romData[radr];
	} else if (slot->data) {
		if (adr >= 0x4000) {
			radr |= (slot->memMap[0] << 14);
		}
		res = slot->data[radr & slot->memMask];
	}
	return res;
}

void gbSlotWr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	xCartridge* slot = &comp->msx.slotA;
	if (!slot->data) return;
	switch (adr & 0xf000) {			// TODO: different cartrige memory mappers
		case 0x2000:
		case 0x3000:
			slot->memMap[0] = val;
			break;
	}
}

// 8000..9fff : video mem
// a000..bfff : external ram

unsigned char gbvRd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char res;
	if (adr & 0x2000) {		// hi 8K: cartrige ram
		res = 0xff;
	} else {
		res = comp->vid->gbc->ram[adr & 0x1fff];
	}
	return res;
}

void gbvWr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	if (adr & 0x2000) {
		// ...
	} else {
		comp->vid->gbc->ram[adr & 0x1fff] = val;
	}
}

// c000..ffff : internal ram, oem, iomap, int mask

// C000..DFFF : RAM1
// E000..FDFF : mirror RAM1
// FE00..FE9F : OAM (sprites data)
// FEA0..FEFF : not used
// FF00..FF7F : IOmap
// FF80..FFFE : RAM2
// FFFF..FFFF : INT mask

unsigned char gbrRd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char res = 0xff;
	if (adr < 0xfe00) {
		res = comp->mem->ramData[adr & 0x1fff];			// 8K, [e000...fdff] -> [c000..ddff]
	} else if (adr < 0xfea0) {
		res = comp->vid->gbc->oem[adr & 0xff];			// video oem
	} else if (adr < 0xff00) {
		res = 0xff;						// unused
	} else if (adr < 0xff80) {
		res = gbIORd(comp, adr);				// io rd
	} else if (adr < 0xffff) {
		res = comp->mem->ramData[0x2000 | (adr & 0xff)];	// ram2
	} else {
		res = comp->gb.intMask;					// int mask
	}
	return res;
}

void gbrWr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	if (adr < 0xfe00) {
		comp->mem->ramData[adr & 0x1fff] = val;
	} else if (adr < 0xfea0) {
		comp->vid->gbc->oem[adr & 0xff] = val;
	} else if (adr < 0xff00) {
		// nothing
	} else if (adr < 0xff80) {
		gbIOWr(comp, adr, val);
	} else if (adr < 0xffff) {
		comp->mem->ramData[0x2000 | (adr & 0xff)] = val;
	} else {
		comp->gb.intMask = val;
	}
}

// maper

void gbMaper(Computer* comp) {
	memSetExternal(comp->mem, MEM_BANK0, gbSlotRd, gbSlotWr, comp);
	memSetExternal(comp->mem, MEM_BANK1, gbSlotRd, gbSlotWr, comp);
	memSetExternal(comp->mem, MEM_BANK2, gbvRd, gbvWr, comp);	// VRAM (8K), slot ram (8K)
	memSetExternal(comp->mem, MEM_BANK3, gbrRd, gbrWr, comp);	// internal RAM/OEM/IOMap
}

unsigned char gbMemRd(Computer* comp, unsigned short adr, int m1) {
	return memRd(comp->mem, adr);
}

void gbMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}

int gbINT(Computer* comp) {
	int res = 0;
	if (comp->vid->intFRAME && (comp->gb.intMask & 1)) {		// VBlank int
		comp->vid->intFRAME = 0;
		comp->cpu->inta = 0x40;
	} else if (comp->vid->gbc->intr && (comp->gb.intMask & 2)) {	// Video state int
		comp->vid->gbc->intr = 0;
		comp->cpu->inta = 0x48;
	} else if (comp->gb.inpint && (comp->gb.intMask & 0x10)) {	// button release int
		comp->gb.inpint = 0;
		comp->cpu->inta = 0x60;
	} else {							// no int
		comp->cpu->inta = 0;
	}
	if (comp->cpu->inta) {
		comp->cpu->intr(comp->cpu);
		comp->cpu->inta = 0;
		res = 1;
	}
	return res;
}

typedef struct {
	const char* name;
	int mask;
} gbKey;

gbKey gbKeyMap[8] = {
	{"RIGHT",1},{"LEFT",2},{"UP",4},{"DOWN",8},
	{"Z",16},{"X",32},{"SPC",64},{"ENT",128}
};

int gbGetInputMask(const char* key) {
	int idx = 0;
	int mask = 0xff;
	while (idx < 8) {
		if (!strcmp(key, gbKeyMap[idx].name)) {
			mask = ~gbKeyMap[idx].mask;
		}
		idx++;
	}
	return mask;
}

void gbPress(Computer* comp, const char* key) {
	int mask = gbGetInputMask(key);
	comp->gb.buttons &= ~mask;
	comp->gb.inpint = 1;			// input interrupt request
}

void gbRelease(Computer* comp, const char* key) {
	int mask = gbGetInputMask(key);
	if (mask == 0) return;
	comp->gb.buttons |= mask;
}
