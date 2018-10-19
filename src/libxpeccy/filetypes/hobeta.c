#include "filetypes.h"
#include <string.h>
// #include <numeric>

int stdAccumulate(unsigned char* ptr, int size, int res) {
	while (size > 0) {
		res += *ptr;
		ptr++;
		size--;
	}
	return res;
}

int loadHobeta(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	unsigned char buf[0x10000];
	TRFile nfle;

	printf("drv %i : %i\n", drv, flp->insert);
	if (!flp->insert) {
		diskFormat(flp);
		flp->insert = 1;
	}
	if (diskGetType(flp) != DISK_TYPE_TRD) {
		err = ERR_NOTRD;
	} else {
		fread((char*)buf, 17, 1, file);		// header
		memcpy((char*)&nfle,buf,14);
		if (!nfle.slen)
			nfle.slen = buf[14];
		int len = nfle.slen << 8;
		fread((char*)buf, len, 1, file);
		if (diskCreateFile(flp, nfle, buf, len) != ERR_OK) {
			err = ERR_HOB_CANT;
		} else {
			for (int i=0; i<256; i++) flpFillFields(flp, i, 1);
		}
	}
	fclose(file);
	return err;
}

int saveHobeta(TRFile dsc,char* data,const char* name) {
	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	unsigned short crc;
	unsigned char buf[17];			// header
	memcpy((char*)buf,(char*)&dsc,13);
	buf[13] = 0x00;
	buf[14] = dsc.slen;
	crc = ((105 + 257 * stdAccumulate(buf, 15, 0)) & 0xffff);
	buf[15] = crc & 0xff;
	buf[16] = ((crc & 0xff00) >> 8);
	fwrite((char*)buf, 17, 1, file);
	fwrite(data, dsc.slen << 8, 1, file);
	fclose(file);
	return ERR_OK;
}

int saveHobetaFile(Floppy* flp,int num,const char* dir) {
	TRFile dsc = diskGetCatalogEntry(flp,num);
	unsigned char buf[0x10000];
	if (!diskGetSectorsData(flp,dsc.trk, dsc.sec+1, buf, dsc.slen)) return ERR_TRD_SNF;	// get file data
	char name[9];
	memcpy(name, dsc.name, 8);
	cutSpaces(name);
	char path[strlen(dir) + 16];
	strcpy(path, dir);		// dir/name.$e
	strcat(path, SLASH);
	strcat(path, name);
	strcat(path, ".$");
	strncat(path, (char*)&dsc.ext, 1);
	return saveHobeta(dsc, (char*)buf, path);
}
