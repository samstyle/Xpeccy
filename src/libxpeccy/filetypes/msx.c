#include "filetypes.h"

void detectType(xCartridge* slot) {
	int test, radr, adr;
	sltSetMaper(slot, MAPER_MSX, slot->mapType);
	if (slot->memMask < 0x8000) {
		printf("nomapper\n");
		sltSetMaper(slot, MAPER_MSX, MAP_MSX_NOMAPPER);		// 16/32K : no mapper
	} else {
		for (adr = 0; adr < 0x4000; adr++) {
			radr = adr & slot->memMask;
			test = slot->data[radr++] << 16;
			test |= slot->data[radr++] << 8;
			test |= slot->data[radr];
			if ((test == 0x320050) || (test == 0x3200b0)) {
				printf("konami 5\n");
				sltSetMaper(slot, MAPER_MSX, MAP_MSX_KONAMI5);
				break;
			} else if ((test == 0x320068) || (test == 0x320078)) {
				printf("ascii 8\n");
				sltSetMaper(slot, MAPER_MSX, MAP_MSX_ASCII8);
				break;
			} else if (test == 0x3200a0) {
				printf("konami 4\n");
				sltSetMaper(slot, MAPER_MSX, MAP_MSX_KONAMI4);
				break;
			}
		}
	}
}

int loadMSX(Computer* comp, const char* name, int drv) {
	int res = loadSlot(comp, name, drv);
	detectType(comp->slot);
	comp->slot->haveram = 0;
	return res;
}
