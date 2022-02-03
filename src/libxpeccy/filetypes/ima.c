#include "filetypes.h"

// 160trk x 18sec x 512bytes hd fdd image (1,44MB)

int load_ima(Computer* comp, const char* path, int drv) {
	int res = ERR_OK;
	int trk;
	char buf[512 * 18];
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(path, "rb");
	if (file) {
		for (trk = 0; trk < 160; trk++) {
			fread(buf, 512*18, 1, file);
			flp_format_trk(flp, trk, 18, 512, buf);
		}
		fclose(file);
		flp_set_path(flp, path);
		flp->insert = 1;
		flp->door = 0;
		flp->changed = 0;
	} else {
		res = ERR_CANT_OPEN;
	}
	return res;
}
