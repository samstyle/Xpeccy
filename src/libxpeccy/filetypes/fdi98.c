#include "filetypes.h"

int loadFDI98(Computer* comp, const char* name, int drv) {
	int res = ERR_OK;
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) {
		res = ERR_CANT_OPEN;
	} else {
		fseek(file, 4, SEEK_CUR);	// +0, 4 bytes is 0
		int type = fgeti(file);		// disk type
		int dpos = fgeti(file);		// header size (data start)
		int dsiz = fgeti(file);		// data size
		int ssz = fgeti(file);		// sector size
		int spt = fgeti(file);		// sec/trk
		int hds = fgeti(file);		// heads
		int trks = fgeti(file);		// tracks
		// set hd/dd
		switch(type) {
			case 0x90:
			case 0x30:
				flp_set_hd(flp, 1);
				break;
			default:
				flp_set_hd(flp, 0);
				break;
		}
		fseek(file, dpos, SEEK_SET);
		int trksz = ssz * spt;		// 1 track data size (bytes)
		int tcnt = trks * 2;		// logical tracks on both sides. if image is single-side, every 2nd track is skipped
		char* buf = malloc(trksz);	// one track data
		flpClearDisk(flp);
		printf("tcnt = %i\n", tcnt);
		for (int trk = 0; trk < tcnt; trk++) {
			fread(buf, trksz, 1, file);
			flp_format_trk(flp, trk, spt, ssz, buf);
			if (hds < 2) trk++;	// single-side, skip even tracks
		}
		free(buf);
		fclose(file);
		flp_insert(flp, name);
	}
	return res;
}
