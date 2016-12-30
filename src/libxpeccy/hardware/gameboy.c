#include "hardware.h"

void gbReset(Computer* comp) {
	comp->msx.slotA.memMap[0] = 1;
	if (comp->msx.slotA.data == NULL) {
		comp->cpu->lock = 1;
	}
}

unsigned char gbSlotRd(unsigned short adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	if (!slot->data) return 0xff;
	int radr;
	if (adr < 0x4000) {
		radr = adr;
	} else {
		radr = (slot->memMap[0] << 14) | (adr & 0x3fff);
	}
	return slot->data[radr & slot->memMask];
}

void gbSlotWr(unsigned short adr, unsigned char val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	if (!slot->data) return;
	switch (adr & 0xf000) {
		case 0x2000:
		case 0x3000:
			slot->memMap[0] = val;
			break;
	}
}

void gbMaper(Computer* comp) {
	memSetExternal(comp->mem, MEM_BANK0, gbSlotRd, gbSlotWr, &comp->msx.slotA);
	memSetExternal(comp->mem, MEM_BANK1, gbSlotRd, gbSlotWr, &comp->msx.slotA);
	memSetBank(comp->mem, MEM_BANK2, MEM_RAM, 0);
	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, 1);
}

unsigned char gbMemRd(Computer* comp, unsigned short adr, int m1) {
	unsigned char res = 0xff;
	if ((adr & 0xff80) == 0xff00) {		// ff00..ff7f : IO mapping
		switch (adr & 0xff) {
			default:
				printf("read %.4X\n",adr);
				assert(0);
				break;
		}
	} else {
		res = memRd(comp->mem, adr);
	}
	return res;
}

void gbMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	if (adr >= 0xff00) {
		printf("write %.4X, %.2X\n",adr,val);
	} else {
		memWr(comp->mem, adr, val);
	}
}
