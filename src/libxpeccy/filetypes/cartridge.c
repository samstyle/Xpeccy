#include "filetypes.h"

void detectType(xCartridge* slot) {
	slot->mapAuto = slot->mapType;
	if (slot->memMask < 0x8000) {
		slot->mapAuto = MSX_NOMAPPER;		// 16/32K : no mapper
	} else {
		int test, radr;
		for (int adr = 0; adr < 0x4000; adr++) {
			radr = adr & slot->memMask;
			test = slot->data[radr++] << 16;
			test |= slot->data[radr++] << 8;
			test |= slot->data[radr];
			if ((test == 0x320050) || (test == 0x3200b0)) {
				slot->mapAuto = MSX_KONAMI5;
				break;
			} else if ((test == 0x320068) || (test == 0x320078)) {
				slot->mapAuto = MSX_ASCII8;
				break;
			} else if (test == 0x3200a0) {
				slot->mapAuto = MSX_KONAMI4;
				break;
			}
		}
	}
}

int loadSlot(xCartridge* slot, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	fseek(file,0,SEEK_END);
	size_t siz = ftell(file);
	rewind(file);
	int err = ERR_OK;
	if (siz > (4 * 1024 * 1024)) {
		err = ERR_RAW_LONG;
	} else {
		int tsiz = 1;
		while (tsiz < siz) {tsiz <<= 1;}		// get nearest 2^n >= siz
		slot->data = realloc(slot->data, tsiz);
		slot->memMask = tsiz - 1;
		strcpy(slot->name, name);
		fread(slot->data, tsiz, 1, file);
		for (tsiz = 0; tsiz < 8; tsiz++) {
			slot->memMap[tsiz] = 0;
		}
		detectType(slot);
		err = ERR_OK;
	}
	fclose(file);
	return err;
}
