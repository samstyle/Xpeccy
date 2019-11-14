#include "filetypes.h"

#pragma pack(push, 1)

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

#pragma pack(pop)

int loadT64(Computer* comp, const char* fname, int drv) {
	char buf[32];
	int err = ERR_OK;
	unsigned short ver;
	unsigned short maxent;
	unsigned short totent;
//	unsigned short start = 0;
	int i;
	t64file desc;
	long offset;
	FILE* file = fopen(fname, "rb");
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		fread(buf, 32, 1, file);
		if (memcmp(buf, "C64", 3)) {
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
				fread((char*)(&desc), sizeof(t64file), 1, file);
				offset = ftell(file);
				fseek(file, desc.offset, SEEK_SET);
				fread(comp->mem->ramData + desc.start, desc.end - desc.start + 1, 1, file);
				fseek(file, offset, SEEK_SET);
//				if (i == 0)
//					start = desc.start;
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
				while (len > 0) {
					per = fgetc(file) & 0xff;
					if (per == 0) {
						per = fgett(file);
					}
					per = 8e3 * per / comp->cpuFrq;		// comp->cpuFrq - MHz (1e6), result period in ns (1e-9)
					if (per > 5e8) {
						blkAddPause(&blk, 1e6);		// 1 sec pause
						tapAddBlock(comp->tape, blk);
						blkClear(&blk);
					} else {
						blkAddWave(&blk, per);
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
