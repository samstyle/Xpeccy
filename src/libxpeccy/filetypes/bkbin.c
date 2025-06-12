#include "filetypes.h"
#include "libxpeccy/cpu/1801vm1/1801vm1.h"

int loadBIN(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
//	char buf[0x8000];
	int adr = fgetw(file);
	int len = fgetw(file);
	adr &= ~1;
	if (len & 1) len++;
	int val;
	if (adr + len > 0x8000) {
		err = ERR_RAW_LONG;
	} else {
//		fread(buf, len, 1, file);
		comp->cpu->regNOD = 3;	// words
		while (len > 0) {
			val = fgetw(file);
			memWr(comp->mem, adr, val);
			adr += 2;
			len -= 2;
		}
//		err = ERR_OK;
	}
	fclose(file);
	return err;
}

// lenght in mks
#define BKPULSE 100 * 1000 / TAPTICKNS
#define BKPILOT	2*BKPULSE
#define BKSYNC	8*BKPULSE
#define BKONE	4*BKPULSE		// 1 = BKONE + BKZERO  (waves)
#define BKZERO	2*BKPULSE		// 0 = BKZERO + BKZERO

void bk_write_bit(TapeBlock* blk, int bit) {
	blkAddWave(blk, bit ? BKONE : BKZERO);		// data
	blkAddWave(blk, BKZERO);			// skip (0)
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

void bk_write_pilot(TapeBlock* blk, int len) {
	while (len > 0) {
		blkAddWave(blk, BKZERO);
		len--;
	}
	blkAddWave(blk, BKSYNC);
	bk_write_bit(blk, 1);
}

int bkLoadToTape(Computer* comp, const char* name, int drv) {
	int i;
	int bt;
	char buf[16];
	char* ptr;
	char* nptr;
	char* dptr;
	char* cname = (char*)malloc(strlen(name) + 1);
	strcpy(cname, name);
	unsigned short start;
	unsigned short len;
	TapeBlock blk;
	blk.data = NULL;
	blk.vol = 0;
	FILE* file = fopen(name, "rb");
	int err = ERR_OK;
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		start = fgetw(file);
		len = fgetw(file);
		memset(buf, ' ', 16);
		ptr = strrchr(cname, SLSH);	// last slash
		nptr = ptr ? ptr + 1 : cname;
		dptr = strrchr(nptr, '.');	// last dot
		if (dptr) *dptr = 0x00;		// cut extension
		memcpy(buf, nptr, (strlen(nptr) < 16) ? strlen(nptr) : 16);
		blkClear(&blk);

		blkAddPause(&blk, TAPTPS);

		bk_write_pilot(&blk, 4096);	// long pilot (4096)

		bk_write_pilot(&blk, 8);	// short pilot (8)
		bk_write_word(&blk, start);	// start (data)
		bk_write_word(&blk, len);	// size (data)
		for(i = 0; i < 16; i++)		// filename
			bk_write_byte(&blk, buf[i]);

		bk_write_pilot(&blk, 8);	// short pilot (8)
		int crc = 0;
		while (len > 0) {		// all file data, byte to byte, except 1st 4 bytes (start, len), LSB first
			len--;
			bt = fgetc(file) & 0xff;
			bk_write_byte(&blk, bt);
			crc += bt;
			if (crc > 0xffff) {
				crc &= 0xffff;
				crc++;
			}
		}
		bk_write_word(&blk, crc & 0xffff);	// crc
		bk_write_pilot(&blk, 256);	// final pilot (256 bytes)

		tap_add_block(comp->tape, blk);
		blkClear(&blk);
		fclose(file);
	}

	free(cname);
	return err;
}
