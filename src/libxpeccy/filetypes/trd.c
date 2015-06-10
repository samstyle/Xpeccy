#include "filetypes.h"

void loadBoot(Floppy* flp, const char* path) {
	if (flpGetDiskType(flp) != DISK_TYPE_TRD) return;
	TRFile cat[128];
	int catSize = flpGetTRCatalog(flp, cat);
	int gotBoot = 0;
	for (int i = 0; i < catSize; i++) {
		if (strncmp((char*)cat[i].name, "boot    B", 9) == 0)
			gotBoot = 1;
	}
	if (gotBoot) return;
	loadHobeta(flp, path);
	flp->changed = 0;
}

size_t fgetSize(FILE* file) {
	fseek(file, 0, SEEK_END);
	size_t res = ftell(file);
	rewind(file);
	return res;
}

int loadTRD(Floppy* flp, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	size_t len = fgetSize(file);
	if (((len & 0xff) != 0) || (len == 0) || (len > 0xa8000)) {
		err = ERR_TRD_LEN;
	} else {
		flpFormat(flp);
		int i = 0;
		unsigned char trackBuf[0x1000];
		do {
			fread((char*)trackBuf, 0x1000, 1, file);
			flpFormTRDTrack(flp, i, trackBuf);
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
			if (!flpGetSectorData(flp, i, j, dptr, 256)) {
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
