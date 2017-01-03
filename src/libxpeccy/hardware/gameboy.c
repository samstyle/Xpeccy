#include "hardware.h"

void gbReset(Computer* comp) {
	comp->gb.boot = 1;
	comp->msx.slotA.memMap[0] = 1;
}

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

unsigned char gbvRd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char res;
	if (adr & 0x2000) {		// hi 8K: cartrige ram
		res = 0xff;
	} else {
		res = comp->vid->gbc.ram[adr & 0x1fff];
	}
	return res;
}

void gbvWr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	if (adr & 0x2000) {
		// ...
	} else {
		comp->vid->gbc.ram[adr & 0x1fff] = val;
	}
}

void gbMaper(Computer* comp) {
	memSetExternal(comp->mem, MEM_BANK0, gbSlotRd, gbSlotWr, comp);
	memSetExternal(comp->mem, MEM_BANK1, gbSlotRd, gbSlotWr, comp);
	memSetExternal(comp->mem, MEM_BANK2, gbvRd, gbvWr, comp);	// VRAM (8K), slot ram (8K)
	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, 0);		// internal RAM (TODO: 8K only, hi 8K = low 8K)
}

unsigned char gbMemRd(Computer* comp, unsigned short adr, int m1) {
	if (m1 && comp->gb.boot && (adr > 0xff))
		comp->gb.boot = 0;
	return memRd(comp->mem, adr);
}

void gbMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}
