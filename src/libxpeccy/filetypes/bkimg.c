#include "filetypes.h"
#include <stdio.h>

int loadBkIMG(Computer* comp, const char* fname, int drv) {
	int err = ERR_OK;
	int trk;
	FILE* file = fopen(fname, "rb");
	Floppy* flp;
	char buf[5120];	// 10 sectors x 512 bytes
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		flp = comp->dif->fdc->flop[drv & 3];
		trk = 0;
		while (!feof(file) && (trk < 160)) {
			fread(buf, 5120, 1, file);
			flp_format_trk(flp, trk, 10, 512, buf);
			trk++;
		}
		fclose(file);
		flp->insert = 1;
		flp_set_path(flp, fname);
	}
	return err;
}
