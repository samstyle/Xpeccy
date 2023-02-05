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

// tape signal
// ~13568 waves of lead-in (short)
// L/M: byte boundary
// 8 bits (S/M, M/S)
// parity bit
// L/M: next byte
// ...

static int wavs,wavm,wavl;

void t64_add_bit(TapeBlock* blk, int bit) {
	if (bit) {
		blkAddWave(blk, wavm);
		blkAddWave(blk, wavs);
	} else {
		blkAddWave(blk, wavs);
		blkAddWave(blk, wavm);
	}
}

// 8 bits + parity
void t64_add_byte(TapeBlock* blk, int dat) {
	int prt = parity(dat);
	blkAddWave(blk,wavl);		// byte start: L/M
	blkAddWave(blk,wavm);
	for (int j = 0; j < 8; j++) {	// bits
		t64_add_bit(blk, dat & 1);
		dat >>= 1;
	}
	t64_add_bit(blk, prt);	// parity
}

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

	TapeBlock blk;
	blk.data = NULL;
	blkClear(&blk);

	// length of half-wave for small,medium,long waves
	wavs = 0x30 * comp->nsPerTick / 250;
	wavm = 0x44 * comp->nsPerTick / 250;
	wavl = 0x56 * comp->nsPerTick / 250;

	int i;
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
				ft_1541 = fgetc(file) & 0xff;		// 0x82 PRG
				start = fgetw(file);
				end = fgetw(file);
				fseek(file, 2, SEEK_SET);
				dataoff = fgeti(file);
				fseek(file, 4, SEEK_SET);
				fread(name, 16, 1, file);
				offset = ftell(file);
				fseek(file, dataoff, SEEK_SET);
#if 0
				for (j = 0; j < 5000; j++) {		// pilot
					blkAddWave(&blk, wavs);
					blkAddWave(&blk, wavs);
				}
				while (start <= end) {
					data = fgetc(file) & 0xff;
					t64_add_byte(&blk, data);
					start++;
				}
				blkAddWave(&blk, wavl);		// L/S end of file
				blkAddWave(&blk, wavs);
				blkAddWave(&blk, 5e5);		// pause
				tap_add_block(comp->tape, blk);
				blkClear(&blk);
#endif
				fseek(file, offset, SEEK_SET);
			}
		}
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
// (N * 8) / Fcpu	Fcpu in Hz, result in secs

int loadC64RawTap(Computer* comp, const char* name, int dsk) {
	char buf[20];
	int err = ERR_OK;
	int ver;
	int len;
	int per;
	TapeBlock blk;
	blk.data = NULL;
	blkClear(&blk);
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
				blk.vol = 1;
				while (len > 0) {
					per = fgetc(file) & 0xff;
					if (per == 0) {
						switch (ver) {
							case 0: per = 256; break;
							case 1: per = fgett(file); break;
						}
						// printf("%i\n",per);
					}
					per = per * comp->nsPerTick / 125;	// 8 * per * nspt / 1000	mks
					blkAddPulse(&blk, per/2, 0x50);		// add full wave 0/1
					blkAddPulse(&blk, per/2, 0xb0);
					len--;
				}
				tap_add_block(comp->tape, blk);
				blkClear(&blk);
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
