#include "filetypes.h"

int loadTRD(Floppy* flp, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	size_t len = fgetSize(file);
	if (((len & 0xff) != 0) || (len == 0) || (len > 0xa8000)) {
		err = ERR_TRD_LEN;
	} else {
		diskFormat(flp);
		int i = 0;
		unsigned char trackBuf[0x1000];
		do {
			fread((char*)trackBuf, 0x1000, 1, file);
			diskFormTRDTrack(flp, i, trackBuf);
			i++;
		} while  (!feof(file));

		flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
		strcpy(flp->path,name);
		flp->insert = 1;
		flp->changed = 0;
	}
	fclose(file);
	return err;
}

int saveTRD(Floppy* flp, const char* name) {
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

	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->changed = 0;
	return ERR_OK;
}
