#include "filetypes.h"

/*
#pragma pack (push, 1)

typedef struct {
	char type_c64;
	char type_1541;
	unsigned short start;
	unsigned short end;
	unsigned short nu01;
	unsigned short offset;
	unsigned short nu02;
	char name[16];
} t64file;

#pragma pack (pop)
*/

// There are three types of pulses: short (352 µs), medium (512 µs) and long (672 µs)
// S/M	0
// M/S	1
// L/M	byte boundary
// L/S	end of file

int loadT64(Computer* comp, const char* fname, int drv) {
	char buf[32];
	int err = ERR_OK;
	unsigned short ver;
	unsigned short maxent;
	unsigned short totent;

	unsigned char filetype;
	unsigned char ft_1541;
	unsigned short start;
	unsigned short end;
	int dataoff;
	char name[16];

	int i;
//	t64file desc;
	long offset;
	FILE* file = fopen(fname, "rb");
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		fread(buf, 32, 1, file);
		if (strcmp(buf, "C64 tape image file")) {
			err = ERR_T64_SIGN;
		} else {
			ver = fgetw(file);
			maxent = fgetw(file);
			totent = fgetw(file);
			fgetw(file);			// not used
			fread(buf, 24, 1, file);	// container name
			buf[24] = 0x00;
			printf("ver: %.4X\n",ver);
			printf("max ent: %i\n",maxent);
			printf("tot ent: %i\n",totent);
			printf("container: %s\n",buf);
			for(i = 0; i < totent; i++) {
				// fread((char*)(&desc), sizeof(t64file), 1, file);
				filetype = fgetc(file) & 0xff;
				ft_1541 = fgetc(file) & 0xff;
				start = fgetw(file);
				end = fgetw(file);
				fseek(file, 2, SEEK_SET);
				dataoff = fgeti(file);
				fseek(file, 2, SEEK_SET);
				fread(name, 16, 1, file);
				offset = ftell(file);
				fseek(file, dataoff, SEEK_SET);		// ???

				//fread(comp->mem->ramData + start, end - start + 1, 1, file);

				fseek(file, offset, SEEK_SET);
			}
		}
//		if (start)
//			comp->cpu->pc = start;
		fclose(file);
	}
	return err;
}

// tap : c64 raw tape file
// +0	C64-TAPE-RAW	signature
// +12	1		version
// +13	3		0,0,0
// +16	4		lenght
// +20	...		data
// each data byte = period of amplitude change its sign in 1/8 T
// pulse length (in seconds) = (8 * data byte) / (clock cycles)
// 0 = overflow. #00,#11,#22,#33 = period #332211
// (N * 8) / 985248 (pal) <-- use this
// (N * 8) / 1022730 (ntsc)

int loadC64RawTap(Computer* comp, const char* name, int dsk) {
	char buf[20];
	int err = ERR_OK;
	int ver;
	int len;
	int per;
	TapeBlock blk;
	blk.data = NULL;
	FILE* file = fopen(name, "rb");
	if (file) {
		fread(buf, 12, 1, file);
		if (!strncmp(buf, "C64-TAPE-RAW", 12)) {
			ver = fgetc(file);
			if (ver < 2) {
				fseek(file, 3, SEEK_CUR);	// skip 3 bytes
				len = fgeti(file);		// data length
				tapEject(comp->tape);		// clear old tape
				blkClear(&blk);
				blk.vol = 0;
				while (len > 0) {
					per = fgetc(file) & 0xff;
					if (per == 0) {
						per = fgett(file);
						// printf("%i\n",per);
					}
					per = 8 * per / comp->cpuFrq;		// comp->cpuFrq - MHz (1e6), result period in mks (1e-6)
					if (per > 1e5) {
						blkAddPause(&blk, per / 2);		// pause
						tapAddBlock(comp->tape, blk);
						blkClear(&blk);
					} else {
						blkAddWave(&blk, per/2);
					}
					len--;
				}
				tapAddBlock(comp->tape, blk);
			} else {
				printf("2:err\n");
				err = ERR_C64T_SIGN;	// version 2+
			}
		} else {
			printf("1:err\n");
			err = ERR_C64T_SIGN;
		}
		fclose(file);
	} else {
		err = ERR_CANT_OPEN;
	}
	return err;
}
