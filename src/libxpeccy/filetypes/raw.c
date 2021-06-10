#include <string.h>
#include "filetypes.h"

void cutSpaces(char* name) {
	for (int i = 7; i > 0; i--) {		// cut last spaces from 8-char name
		if (name[i] == ' ') {
			name[i] = 0x00;
		} else {
			i = 0;
		}
	}
}

static char hexnum[16] = "0123456789ABCDEF";

int loadRaw(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;

	fseek(file, 0, SEEK_END);
	int len = ftell(file);
	rewind(file);

//	if (len > 0xff00) {
//		err = ERR_RAW_LONG;
//	} else {
		if (!flp->insert) {
			diskFormat(flp);
			flp->insert = 1;
		}
		if (diskGetType(flp) != DISK_TYPE_TRD) {
			err = ERR_NOTRD;
		} else {
			char fpath[1024];
			char fnam[1024];
			char fext[1024];
			memset(fnam,' ',1024);
			memset(fext,' ',1024);
			char* ptr = strrchr(name, '/');		// slash here
			if (ptr) {
				strcpy(fpath, ptr + 1);
			} else {
				strcpy(fpath, name);
			}
			ptr = strrchr(fpath, '\\');		// slash there
			if (ptr) strcpy(fpath, ptr + 1);
			printf("%s\n",fpath);
			ptr = strrchr(fpath, '.');
			if (ptr) {
				memcpy(fnam, fpath, ptr - fpath);			// it doesn't put 0x00 at end, so unused fillnam will be filled with ' '
				memcpy(fext, ptr + 1, strlen(ptr + 1));
			} else {
				memcpy(fnam, fpath, strlen(fpath));
				fext[0] = 0x00;
			}
			TRFile nfle;
			int num = 0;
			int flen;
			unsigned char buf[0x10000];
			while ((len > 0) && (err == ERR_OK)) {
				flen = (len > 0xff00) ? 0xff00 : len;		// part size
				len -= flen;
				nfle = diskMakeDescriptor(fnam, fext[0], 0, flen);
				nfle.lst = fext[1];
				nfle.hst = fext[2];
				if (num) {
					nfle.ext = hexnum[num & 0x0f];
				}
				num++;
				fread((char*)buf, flen, 1, file);
				if (diskCreateFile(flp, nfle, buf, flen) != ERR_OK) {
					err = ERR_HOB_CANT;
				}
			}
			if (err == ERR_OK)
				for (int i = 0; i < 256; i++) flpFillFields(flp, i, 1);
		}
//	}
	fclose(file);
	return err;
}

int saveRawFile(Floppy* flp, int num, const char* dir) {
	TRFile dsc = diskGetCatalogEntry(flp,num);
	if (dsc.name[0] == 0x00) return ERR_TRD_SNF;
	unsigned char buf[0x10000];
	if (!diskGetSectorsData(flp,dsc.trk, dsc.sec+1, buf, dsc.slen)) return ERR_TRD_SNF;
	char name[9];
	memset(name, 0x00, 9);
	strncpy(name, (char*)dsc.name, 8);
	cutSpaces(name);
	char* path = (char*)malloc(strlen(dir) + strlen(name) + 4);
	strcpy(path, dir);		// dir/name.e
	strcat(path, SLASH);
	strcat(path, name);
	strcat(path, ".");
	strncat(path, (char*)&dsc.ext, 1);
	FILE* file = fopen(path, "wb");
	if (!file) return ERR_CANT_OPEN;
	int len;
	if (dsc.slen == (dsc.hlen + ((dsc.llen == 0) ? 0 : 1))) {
		len = (dsc.hlen << 8) + dsc.llen;
	} else {
		len = (dsc.slen << 8);
	}
	fwrite((char*)buf, len, 1, file);
	fclose(file);
	free(path);

	return ERR_OK;
}
