#include "filetypes.h"

int loadRKSmem(Computer* comp, const char* name, int drv) {
	int err = ERR_OK;
	FILE* file = fopen(name, "rb");
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		unsigned short start, end, len;
		start = fgetw(file);
		end = fgetw(file);
		len = end - start + 1;
		int ch;
		comp->cpu->pc = start;
		while (len > 0) {
			ch = fgetc(file);
			comp->hw->mwr(comp, start, ch);
			start++;
			len--;
		}
		fclose(file);
	}
	return err;
}

// 1200 bit/s: ~833 mks/bit, 417 mks/halfwave
#define RKS_HALFWAVE 417

void rks_add_0(TapeBlock* blk) {
	blkAddPulse(blk, RKS_HALFWAVE, 0xb0);	// 1
	blkAddPulse(blk, RKS_HALFWAVE, 0x50);	// 0
}

void rks_add_1(TapeBlock* blk) {
	blkAddPulse(blk, RKS_HALFWAVE, 0x50);	// 0
	blkAddPulse(blk, RKS_HALFWAVE, 0xb0);	// 1
}

void rks_add_byte(TapeBlock* blk, int val) {
	for (int i = 0; i < 8; i++) {
		if (val & 0x80) {
			rks_add_1(blk);
		} else {
			rks_add_0(blk);
		}
		val <<= 1;
	}
}

int loadRKStap(Computer* comp, const char* name, int drv) {
	int err = ERR_OK;
	FILE* file = fopen(name, "rb");
	if (file) {
		int i;
		int ch;
		int start;
		int end;
		TapeBlock blk;
		tapEject(comp->tape);
		blk.data = NULL;
		blkClear(&blk);
		for (i = 0; i < 255-8; i++)	// 255 x bit 0
			rks_add_0(&blk);
		start = fgetw(file);		// start adr
		rks_add_byte(&blk, start);
		rks_add_byte(&blk, start >> 8);
		end = fgetw(file);		// end adr
		rks_add_byte(&blk, end);
		rks_add_byte(&blk, end >> 8);
		while (start < end) {		// data
			ch = fgetc(file);
			rks_add_byte(&blk, ch);
			start++;
		}
		rks_add_byte(&blk, 0x00);	// 00,00,e6
		rks_add_byte(&blk, 0x00);
		rks_add_byte(&blk, 0xe6);
		ch = fgetc(file);		// checksum
		rks_add_byte(&blk, ch);
		ch = fgetc(file);
		rks_add_byte(&blk, ch);
		fclose(file);
		tapAddBlock(comp->tape, blk);
	} else {
		err = ERR_CANT_OPEN;
	}
	return err;
}
