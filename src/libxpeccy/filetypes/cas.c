#include "filetypes.h"

// msx tape signal timings
// @ baud 1200 bps
// 1: 417 mks pulse x 2
// 0: 208 mks pulse x 2
// @ baud 2400 bps
// 1: 208 mks pulse x 2
// 0: 104 mks pulse x 2

#define MSX_CAS_0	417
#define MSX_CAS_1	208

// signature of each block (align by 8 bytes)
static const unsigned char cas_sgn[8] = {0x1f, 0xa6, 0xde, 0xba, 0xcc, 0x13, 0x7d, 0x74};

// check block signature. return 1 if found
// need to be aligned by 8 bytes
int cas_sgn_chk(FILE* file) {
	char sbuf[8];
	fread(sbuf, 8, 1, file);
	return !memcmp(sbuf, cas_sgn, 8);
}

void cas_align(FILE* file) {
	int skip = (8 - (ftell(file) & 7)) & 7;
	fseek(file, skip, SEEK_CUR);
}

// last readed byte (to detect 1A-terminator of ascii blocks)
static int data;

void cas_add_pilot(TapeBlock* blk, int len) {
	while (len > 0) {
		blkAddWave(blk, MSX_CAS_1);
		len--;
	}
}

void cas_add_byte(TapeBlock* blk, int vbyte) {
	int bit;
	blkAddWave(blk, MSX_CAS_0);		// '0', start bit
	for (bit = 0; bit < 8; bit++) {		// 8 data bits, starting from lower
		if (vbyte & 1) {
			blkAddWave(blk, MSX_CAS_1);	// '1' bit generates 2 waves: |^^|__|^^|
			blkAddWave(blk, MSX_CAS_1);
		} else {
			blkAddWave(blk, MSX_CAS_0);	// '0' bit generates 1 wave: |^^^^|____|
		}
		vbyte >>= 1;
	}
	blkAddWave(blk, MSX_CAS_1);		// '1', stop bit (2 waves). or 2 bits '1', 4 waves?
	blkAddWave(blk, MSX_CAS_1);
}

void cas_add_data(TapeBlock* blk, FILE* file, int len) {
	while(len > 0)	{
		data = fgetc(file) & 0xff;
		cas_add_byte(blk, data);
		len--;
	}
}

void cas_add_ascii(TapeBlock* blk, FILE* file) {
#if 1
	cas_add_data(blk, file, 0x100);
#else
	data = 0x20;
	for (int i = 0; i < 0x100; i++) {
		if (data != 0x1a) {
			data = fgetc(file);
			cas_add_byte(blk, data);
		} else {
			fgetc(file);
		}
	}
#endif
}

int loadCAS(Computer* comp, const char* name, int drv) {
	int res = ERR_OK;
	int t;
	unsigned short sadr, eadr;
	FILE* file = fopen(name, "rb");
	TapeBlock blk;
	blk.data = NULL;
	if (file) {
		tapEject(comp->tape);
		while (!feof(file) && (res == ERR_OK)) {
			// read header
			cas_align(file);
			if (cas_sgn_chk(file)) {
				blkClear(&blk);
				t = fgetc(file) & 0xff;
				printf("blk %.2X @ %li\n", t, ftell(file) - 1);
				switch (t) {
					case 0xd3:
						fseek(file, -1, SEEK_CUR);	// header
						cas_add_pilot(&blk, 0x1e00);
						cas_add_data(&blk, file, 16);
						blkAddPause(&blk, 1e6);
						tapAddBlock(comp->tape, blk);
						blkClear(&blk);
						cas_align(file);
						if (cas_sgn_chk(file)) {
							res = ERR_CAS_TYPE;
							printf("block D3\n");
							// read d3 block
						} else {
							res = ERR_CAS_SIGN;
						}
						blkAddPause(&blk, 2e6);
						tapAddBlock(comp->tape, blk);
						blkClear(&blk);
						break;
					case 0xd0:
						fseek(file, -1, SEEK_CUR);		// header
						cas_add_pilot(&blk, 0x1e00);
						cas_add_data(&blk, file, 16);
						blkAddPause(&blk, 1e6);
						tapAddBlock(comp->tape, blk);
						blkClear(&blk);
						cas_align(file);
						if (cas_sgn_chk(file)) {
							sadr = fgetw(file);			// binary data
							eadr = fgetw(file);
							fseek(file, -4, SEEK_CUR);
							cas_add_pilot(&blk, 0x780);
							cas_add_data(&blk, file, eadr - sadr + 7);
							blkAddPause(&blk, 2e6);
							tapAddBlock(comp->tape, blk);
							blkClear(&blk);
						} else {
							res = ERR_CAS_SIGN;
						}
						break;
					case 0xea:
						fseek(file, -1, SEEK_CUR);		// header
						cas_add_pilot(&blk, 0x1e00);
						cas_add_data(&blk, file, 16);
						blkAddPause(&blk, 1e6);
						tapAddBlock(comp->tape, blk);
						blkClear(&blk);
						cas_align(file);
						data = 0x20;
						while ((data != 0x1a) && (res == ERR_OK)) {
							if (cas_sgn_chk(file)) {
								cas_add_pilot(&blk, 0x780);
								cas_add_data(&blk, file, 0x100);
								blkAddPause(&blk, 1e6);
								tapAddBlock(comp->tape, blk);
								blkClear(&blk);
							} else {
								res = ERR_CAS_SIGN;
							}
						}
						break;
					default:
						res = ERR_CAS_TYPE;
						break;
				}
			} else if (!feof(file)) {
				res = ERR_CAS_SIGN;
			}
		}
		fclose(file);
	} else {
		res = ERR_CANT_OPEN;
	}
	blkClear(&blk);
	return res;
}
