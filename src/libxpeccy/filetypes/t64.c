#include "filetypes.h"

#include <string.h>

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

void t64_add_pilot(TapeBlock* blk, int cnt) {
	while (cnt > 0) {
		blkAddWave(blk, wavs);
		blkAddWave(blk, wavs);
		cnt--;
	}
}

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
	int prt = parity(dat & 0xff);
	blk->crc ^= dat;
	blkAddWave(blk,wavl);		// byte start: L/M
	blkAddWave(blk,wavm);
	for (int j = 0; j < 8; j++) {	// bits
		t64_add_bit(blk, dat & 1);
		dat >>= 1;
	}
	t64_add_bit(blk, prt);	// parity
}

void t64_add_192(TapeBlock* blk, char* buf, int eod) {
	int j;
	for (j = 0x89; j > 0x80; j--) {		// sequence 89 to 81
		t64_add_byte(blk, j);
	}
	blk->crc = 0;				// 1st copy of data
	for (j = 0; j < 192; j++) {
		t64_add_byte(blk, buf[j]);
	}
	t64_add_byte(blk, blk->crc);		// crc
	blkAddWave(blk, wavl);			// long wave
	t64_add_pilot(blk, 60);			// 60 sync pulses
	for (j = 0x09; j > 0x00; j--) {		// sequence 09 to 01
		t64_add_byte(blk, j);
	}
	blk->crc = 0;				// 2nd copy of data
	for (j = 0; j < 192; j++) {
		t64_add_byte(blk, buf[j]);
	}
	t64_add_byte(blk, blk->crc);		// crc
	if (eod) {				// if end of data, L/S marker
		blkAddWave(blk, wavl);
		blkAddWave(blk, wavs);
	} else {
		blkAddWave(blk, wavl);			// long wave
	}
	t64_add_pilot(blk, 60);			// 60 sync pulses
}

int loadT64(Computer* comp, const char* fname, int drv) {
	char buf[192];
	int err = ERR_OK;
	unsigned short ver;
	unsigned short maxent;
	unsigned short totent;

	unsigned char filetype;
	unsigned char ft_1541;
	int start;
	int end;
	int len;
	int dataoff;
	char name[16];

	TapeBlock blk;
	blk.data = NULL;
	blkClear(&blk);

	// length of half-wave for small,medium,long waves
	// TODO: recalculate for TAPTICKNS
	wavs = 0x30 * comp->nsPerTick * 4 / TAPTICKNS;	// / 250;
	wavm = 0x44 * comp->nsPerTick * 4 / TAPTICKNS;	// / 250;
	wavl = 0x56 * comp->nsPerTick * 4 / TAPTICKNS;	// / 250;

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
			maxent = fgetw(file);		// number of directory entries
			totent = fgetw(file);		// number of used entries
			fgetw(file);			// not used
			fread(buf, 24, 1, file);	// description
			buf[24] = 0x00;
			printf("ver: %.4X\n",ver);
			printf("max ent: %i\n",maxent);
			printf("tot ent: %i\n",totent);
			printf("container: %s\n",buf);
			for(i = 0; i < totent; i++) {
				filetype = fgetc(file) & 0xff;		// 00:free entry(skip), 01:normal tape file, 02:block with header, 03:snapshot, 04:tape block, 05:digitize
				ft_1541 = fgetc(file) & 0xff;		// C64 file type (82:prg)
				start = fgetw(file);			// end-start+1 = data len
				end = fgetw(file);
				if (end == 0) end += 0x10000;
				fseek(file, 2, SEEK_CUR);
				dataoff = fgeti(file);
				fseek(file, 4, SEEK_CUR);
				fread(name, 16, 1, file);
				if (filetype != 0x00) {
					offset = ftell(file);
					fseek(file, dataoff, SEEK_SET);
#if 1
					t64_add_pilot(&blk, 14000);		// pilot (usually 10 sec)
					memset(buf, 0x20, 192);			// header
					buf[0] = filetype & 0xff;
					buf[1] = start & 0xff;
					buf[2] = (start >> 8) & 0xff;
					buf[3] = end & 0xff;
					buf[4] = (end >> 8) & 0xff;
					memcpy(buf+5, name, 16);
					t64_add_192(&blk, buf, 1);
					blkAddPause(&blk, TAPTPS);
					t64_add_pilot(&blk, 2800);		// 2sec pilot
					while (start <= end) {			// data
						if (end - start > 192) {
							len = 192;
						} else {
							len = end + 1 - start;
						}
						memset(buf, 0x00, 192);
						fread(buf, len, 1, file);
						start += len;
						t64_add_192(&blk, buf, start > end);
					}
					blkAddWave(&blk, TAPTPS);		// pause (1sec)
					tap_add_block(comp->tape, blk);
					blkClear(&blk);
#endif
					fseek(file, offset, SEEK_SET);
				}
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
				fseek(file, 3, SEEK_CUR);	// skip 3 bytes (platform, video standard, reserved)
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
					per = per * comp->nsPerTick * 8 / TAPTICKNS;	// 8 * per * nspt / 1000	mks
					blkAddPulse(&blk, per/2, 0x50);			// add full wave 0/1
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
