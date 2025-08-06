#include "filetypes.h"

int loadSlot(Computer* comp, const char* name, int drv) {
	printf("loadSlot %s\n", name);
	xCartridge* slot = comp->slot;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	fseek(file,0,SEEK_END);
	long siz = ftell(file);
	rewind(file);
	int err = ERR_OK;
	if (siz > MEM_4M) {
		err = ERR_RAW_LONG;
		fclose(file);
	} else {
		int tsiz = 1;
		while (tsiz < siz) {tsiz <<= 1;}		// get nearest 2^n >= siz
		slot->data = realloc(slot->data, tsiz);
		slot->brkMap = realloc(slot->brkMap, tsiz);
		memset(slot->brkMap, 0x00, tsiz);
		slot->memMask = tsiz - 1;
		slot->haveram = 0;
		sltSetPath(slot, name);
		fread(slot->data, tsiz, 1, file);
		for (tsiz = 0; tsiz < 4; tsiz++) {
			slot->memMap[tsiz] = 0;
		}
/*
		switch(comp->hw->grp) {
			case HWG_MSX: detectType(slot); break;
			case HWG_GB: sltSetMaper(slot, MAPER_GB, MAP_GB_NOMAP); break;	// it will be set on reset
			default: sltSetMaper(slot, MAPER_MSX, MAP_MSX_NOMAPPER); break;
		}
*/
		fclose(file);
		char rname[FILENAME_MAX];
		strcpy(rname, name);
		strcat(rname, ".ram");
		file = fopen(rname, "rb");
		if (file) {
			fread(slot->ram, 0x20000, 1, file);
			fclose(file);
		} else {
			memset(slot->ram, 0x00, 0x20000);
		}
		err = ERR_OK;
		compReset(comp, RES_DEFAULT);
		/*
		switch (comp->hw->grp) {
			case HWG_MSX:
			case HWG_NES:
			case HWG_GB:
				compReset(comp, RES_DEFAULT);
				break;
			default:
				comp->hw->mapMem(comp);
				break;
		}
		*/
	}
	return err;
}
