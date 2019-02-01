#include "filetypes.h"

int loadBIN(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	char buf[0x8000];
	unsigned short start = fgetw(file);
	unsigned short len = fgetw(file);
	if (start + len > 0x8000) {
		err = ERR_RAW_LONG;
	} else {
		fread(buf, len, 1, file);
		for (int i = 0; i < len; i++) {
			memWr(comp->mem, start + i, (unsigned char)buf[i]);
		}
		err = ERR_OK;
	}
	fclose(file);
	return err;
}
