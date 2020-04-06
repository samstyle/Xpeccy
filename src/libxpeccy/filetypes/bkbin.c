#include "filetypes.h"

int loadBIN(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	char buf[0x8000];
	unsigned short start = fgetw(file);
	unsigned short len = fgetw(file);
	if (start + len > 0x8000) {
		err = ERR_RAW_LONG;
	} else {
		fread(buf, len, 1, file);
		for (int i = 0; i < len; i++) {
			memWr(comp->mem, start + i, (unsigned char)buf[i]);
		}
		err = ERR_OK;
	}
	fclose(file);
	return err;
}

// lenght in mks
#define BKPILOT	300
#define BKSYNC	1200
#define BKONE	600		// 1 = BKONE + BKZERO  (pulses)
#define BKZERO	300		// 0 = BKZERO + BKZERO

void bk_write_bit(TapeBlock* blk, int bit) {
	blkAddWave(blk, bit ? BKONE : BKZERO);
	blkAddWave(blk, BKZERO);
}

void bk_write_byte(TapeBlock* blk, int byt) {
	for (int i = 0; i < 8; i++) {
		bk_write_bit(blk, byt & 1);
		byt >>= 1;
	}
}

void bk_write_word(TapeBlock* blk, int wrd) {
	bk_write_byte(blk, wrd & 0xff);
	bk_write_byte(blk, (wrd >> 8) & 0xff);
}

int bkLoadToTape(Computer* comp, const char* name, int drv) {
	int i;
	long sz;
	int bt;
	char buf[16];
	char* ptr;
	TapeBlock blk;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	fseek(file, 0, SEEK_END);
	sz = ftell(file);
	rewind(file);
	if (sz < 0x10000) {
		ptr = strrchr(name, SLSH);	// filename (max 16 chars) in buf
		memset(buf, ' ', 16);
		if (ptr == NULL) {
			memcpy(buf, name, (strlen(name) < 16) ? strlen(name) : 16);
		} else {
			memcpy(buf, ptr + 1, (strlen(ptr + 1) < 16) ? strlen(ptr + 1) : 16);
		}
		blkClear(&blk);
		for (i = 0; i < 4096; i++)	// long pilot
			blkAddWave(&blk, BKPILOT);
		blkAddWave(&blk, BKSYNC);	// sync
		bk_write_bit(&blk, 1);		// bit 1
		bk_write_word(&blk, 0);		// start
		bk_write_word(&blk, 16);	// size
		for(i = 0; i < 16; i++)		// filename
			bk_write_byte(&blk, buf[i]);
		for (i = 0; i < 8; i++)		// short pilot
			blkAddWave(&blk, BKPILOT);
		blkAddWave(&blk, BKSYNC);	// sync
		bk_write_bit(&blk, 1);		// bit 1
		int crc = 0;
		while (sz > 0) {		// all file data, byte to byte
			sz--;
			bt = fgetc(file) & 0xff;
			bk_write_byte(&blk, bt);
			crc += bt;
			if (crc > 0xffff) {
				crc &= 0xffff;
				crc++;
			}
		}
		bk_write_word(&blk, crc & 0xffff);
		for(i = 0; i < 256; i++)	// final pilot
			blkAddWave(&blk, BKPILOT);
		blkAddPause(&blk, 1000);
		tapAddBlock(comp->tape, blk);
	} else {
		err = ERR_RAW_LONG;
	}

	fclose(file);
	return err;
}
