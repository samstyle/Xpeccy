#include "filetypes.h"

int loadTRD(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	size_t len = fgetSize(file);
	if (((len & 0xff) != 0) || (len == 0) || (len > 0xa8000)) {
		err = ERR_TRD_LEN;
	} else {
//		diskFormat(flp);
		int trk = 0;
		char buf[0x1000];
		do {
			fread(buf, 0x1000, 1, file);
			flp_format_trk(flp, trk, 16, 256, buf);
			trk++;
		} while  (!feof(file));
		memset(buf, 0, 0x1000);
		while (trk < 160) {
			flp_format_trk(flp, trk, 16, 256, buf);
			trk++;
		}

		flp_set_path(flp, name);
		flp->insert = 1;
		flp->changed = 0;
	}
	fclose(file);
	return err;
}

int saveTRD(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	unsigned char img[0xa8000];
	unsigned char* dptr = img;
	for (int i = 0; i < 160; i++) {
		for (int j = 1; j < 17; j++) {
			if (!diskGetSectorData(flp, i, j, dptr, 256)) {
				return ERR_TRD_SNF;
			}
			dptr += 256;
		}
	}

	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	fwrite((char*)img, 0xa0000, 1, file);
	fclose(file);

	flp_set_path(flp, name);
	flp->changed = 0;
	return ERR_OK;
}
