#include "filetypes.h"

int loadC64prg(Computer* comp, const char* name, int dsk) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	unsigned short adr = fgetw(file);
	fseek(file, 0, SEEK_END);
	long sz = ftell(file);
	fseek(file, 2, SEEK_SET);
	fread(comp->mem->ramData + adr, sz - 2, 1, file);
	fclose(file);
	return ERR_OK;
}
