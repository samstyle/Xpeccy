#include "filetypes.h"

int loadSlot(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char* ptr = drv ? comp->msx.slotB.data : comp->msx.slotA.data;
	fseek(file,0,SEEK_END);
	size_t siz = ftell(file);
	rewind(file);
	int err = ERR_OK;
	if (siz > (4 * 1024 * 1024)) {
		err = ERR_RAW_LONG;
	} else {
		int tsiz = 0x4000;
		while (tsiz < siz) {
			tsiz <<= 1;
		}
		ptr = realloc(ptr, tsiz);
		if (drv) {
			comp->msx.slotB.data = ptr;
			comp->msx.slotB.memMask = tsiz - 1;
			strcpy(comp->msx.slotB.name, name);
		} else {
			comp->msx.slotA.data = ptr;
			comp->msx.slotA.memMask = tsiz - 1;
			strcpy(comp->msx.slotA.name, name);
		}
		fread(ptr, tsiz, 1, file);
		err = ERR_OK;
	}
	fclose(file);
	return err;
}
