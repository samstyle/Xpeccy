#include "filetypes.h"

int loadBIN(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	unsigned short start = fgetw(file);
	unsigned short len = fgetw(file);
	if (start + len > 0x8000) {
		err = ERR_RAW_LONG;
	} else {
		fread(comp->mem->ramData + start, len, 1, file);
		err = ERR_OK;
	}
	fclose(file);
	return err;
}
