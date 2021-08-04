#include "filetypes.h"

int loadGB(Computer* comp, const char* name, int drv) {
	int res = loadSlot(comp, name, drv);
	if (comp->slot->data) {
		unsigned char type = comp->slot->data[0x147];		// slot type
		printf("Cartrige type %.2X\n",type);
		switch (type) {
			case 0x00:
				sltSetMaper(comp->slot, MAPER_GB, MAP_GB_NOMAP);	// rom only (up to 32K)
				break;
			case 0x01:
			case 0x02:
			case 0x03:
				sltSetMaper(comp->slot, MAPER_GB, MAP_GB_MBC1);		// mbc1
				break;
			case 0x05:
			case 0x06:
				sltSetMaper(comp->slot, MAPER_GB, MAP_GB_MBC2);		// mbc2
				break;
			case 0x0f:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
				sltSetMaper(comp->slot, MAPER_GB, MAP_GB_MBC3);		// mbc3
				break;
			case 0x19:
			case 0x1a:
			case 0x1b:
			case 0x1c:
			case 0x1d:
			case 0x1e:
				sltSetMaper(comp->slot, MAPER_GB, MAP_GB_MBC5);		// mbc5
				break;
			default:
				printf("unknown maper\n");
				sltSetMaper(comp->slot, MAPER_GB, MAP_UNKNOWN);
				break;
		}

	}
	return res;
}
