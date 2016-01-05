#include "filetypes.h"

int loadCARD(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char* ptr = drv ? comp->msx.slotB : comp->msx.slotA;
	fseek(file,0,SEEK_END);
	size_t siz = ftell(file);
	rewind(file);
	int err = ERR_OK;
	if (siz > (4 * 1024 * 1024)) {
		err = ERR_RAW_LONG;
	} else {
		ptr = realloc(ptr, siz);
		if (drv) {
			comp->msx.slotB = ptr;
			strcpy(comp->msx.slotBname, name);
		} else {
			comp->msx.slotA = ptr;
			strcpy(comp->msx.slotAname, name);
		}
		fread(ptr, siz, 1, file);
		err = ERR_OK;
	}
	fclose(file);
	return err;
}
