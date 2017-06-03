#include "cartridge.h"
#include <stdlib.h>
#include <string.h>

// dummy

unsigned char slt_rd_dum(xCartridge* slt, unsigned short adr) {
	return 0xff;
}

void slt_wr_dum(xCartridge* slt, unsigned short adr, unsigned char val) {
}

// MSX mappers

// no mapper
unsigned char slt_msx_nomap_rd(xCartridge* slot, unsigned short adr) {
	int radr = (adr & 0x3fff) | ((adr & 0x8000) >> 1);
	return slot->data[radr & slot->memMask];
}

// konami 4
unsigned char slt_msx_kon4_rd(xCartridge* slot, unsigned short adr) {
	int bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
	bnk = bnk ? slot->memMap[bnk] : 0;
	int radr = (bnk << 13) | (adr & 0x1fff);
	return slot->data[radr & slot->memMask];
}

void slt_msx_kon4_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch(adr) {
		case 0x6000: slot->memMap[1] = val; break;
		case 0x8000: slot->memMap[2] = val; break;
		case 0xa000: slot->memMap[3] = val; break;
	}
}

// konami 5
unsigned char slt_msx_kon5_rd(xCartridge* slot, unsigned short adr) {
	int bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
	bnk = slot->memMap[bnk];
	int radr = (bnk << 13) | (adr & 0x1fff);
	return slot->data[radr & slot->memMask];
}

void slt_msx_kon5_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x5000: slot->memMap[0] = val; break;
		case 0x7000: slot->memMap[1] = val; break;
		case 0x9000: slot->memMap[2] = val; break;		// TODO: SCC
		case 0xb000: slot->memMap[3] = val; break;
	}
}

// ascii8
// rd = konami5 rd
void slt_msx_asc8_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x6000: slot->memMap[0] = val; break;
		case 0x6800: slot->memMap[1] = val; break;
		case 0x7000: slot->memMap[2] = val; break;
		case 0x7800: slot->memMap[3] = val; break;
	}
}

// ascii16
unsigned char slt_msx_asc16_rd(xCartridge* slot, unsigned short adr) {
	int bnk = slot->memMap[(adr & 0x8000) >> 15];
	int radr = (bnk << 14) | (adr & 0x3fff);
	return slot->data[radr & slot->memMask];
}

void slt_msx_asc16_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x6000: slot->memMap[0] = val; break;	// #4000..#7FFF
		case 0x7000: slot->memMap[1] = val; break;	// #8000..#bfff
	}
}

// GAMEBOY

// reading is same for all mappers
unsigned char slt_gb_all_rd(xCartridge* slot, unsigned short adr) {
	unsigned char res = 0xff;
	int radr;
	if (adr & 0x8000) {			// ram
		if (slot->ramen) {
			radr = (slot->memMap[1] << 13) | (adr & 0x1fff);
			res = slot->ram[radr & 0x7fff];
		}
	} else {				// rom
		radr = adr & 0x3fff;
		if (adr & 0x4000)
			radr |= (slot->memMap[0] << 14);
		res = slot->data[radr & slot->memMask];
	}
	return res;
}

void slt_gb_mbc1_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch (adr & 0xe000) {
		case 0x0000:			// 0000..1fff : xA = ram enable
			slot->ramen = ((val & 0x0f) == 0x0a) ? 1 : 0;
			break;
		case 0x2000:			// 2000..3fff : & 1F = ROM bank (0 -> 1)
			val &= 0x1f;
			val |= (slot->memMap[0] & 0x60);
			if (val == 0) val++;
			slot->memMap[0] = val;
			break;
		case 0x4000:			// 4000..5fff : 2bits : b0,1 ram bank | b5,6 rom bank (depends on banking mode)
			val &= 3;
			if (slot->ramod) {
				slot->memMap[1] = val;
			} else {
				slot->memMap[0] = (slot->memMap[0] & 0x1f) | (val << 5);
			}
			break;
		case 0x6000:			// 6000..7fff : b0 = 0:rom banking, 1:ram banking
			slot->ramod = (val & 1);
			if (slot->ramod) {
				slot->memMap[0] &= 0x1f;
			} else {
				slot->memMap[1] = 0;
			}
			break;
		case 0xa000:			// a000..bfff : cartrige ram
			if (!slot->ramen) break;
			adr = (slot->memMap[1] << 13) | (adr & 0x1fff);
			slot->ram[adr & 0x7fff] = val;
			break;
	}
}

void slt_gb_mbc2_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch (adr & 0xe000) {
		case 0x0000:				// 0000..1fff : 4bit ram enabling
			if (adr & 0x100) return;
			slot->ramen = ((val & 0x0f) == 0x0a) ? 1 : 0;
			break;
		case 0x2000:				// 2000..3fff : rom bank nr
			if (adr & 0x100) {		// hi LSBit must be 1
				val &= 0x0f;
				if (val == 0) val++;
				slot->memMap[0] = val;
			}
			break;
		case 0xa000:			// a000..bfff : cartrige ram
			if (!slot->ramen) break;
			adr = (slot->memMap[1] << 13) | (adr & 0x1fff);
			slot->ram[adr & 0x7fff] = val;
			break;
	}
}

void slt_gb_mbc3_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch (adr & 0xe000) {
		case 0x0000:		// 0000..1fff : xA = ram enable
			slot->ramen = ((val & 0x0f) == 0x0a) ? 1 : 0;
			break;
		case 0x2000:		// 2000..3fff : rom bank (1..127)
			val = 0x7f;
			if (val == 0) val++;
			slot->memMap[0] = val;
			break;
		case 0x4000:		// 4000..5fff : ram bank / rtc register
			if (val < 4) {
				slot->memMap[1] = val;
			}
			break;
		case 0x6000:		// b0: 0->1 latch rtc registers
			break;
		case 0xa000:			// a000..bfff : cartrige ram
			if (!slot->ramen) break;
			adr = (slot->memMap[1] << 13) | (adr & 0x1fff);
			slot->ram[adr & 0x7fff] = val;
			break;
	}
}

void slt_gb_mbc5_wr(xCartridge* slot, unsigned short adr, unsigned char val) {
	switch(adr & 0xf000) {
		case 0x0000:		// 0000..1fff : 0A enable ram, 00 disable ram
		case 0x1000:
			if (val == 0x0a) {
				slot->ramen = 1;
			} else if (val == 0x00) {
				slot->ramen = 0;
			}
			break;
		case 0x2000:
			slot->memMap[0] &= 0x100;
			slot->memMap[0] |= val;
			break;
		case 0x3000:
			slot->memMap[0] &= 0xff;
			slot->memMap[0] |= (val & 1) << 8;
			break;
		case 0x4000:
		case 0x5000:
			slot->memMap[1] = val & 0x0f;
			break;
		case 0xa000:			// a000..bfff : cartrige ram
		case 0xb000:
			if (!slot->ramen) break;
			adr = (slot->memMap[1] << 13) | (adr & 0x1fff);
			slot->ram[adr & 0x7fff] = val;
			break;
	}
}

// table

xCardCallback maperTab[] = {
	{MAP_MSX_NOMAPPER, slt_msx_nomap_rd, slt_wr_dum},
	{MAP_MSX_KONAMI4, slt_msx_kon4_rd, slt_msx_kon4_wr},
	{MAP_MSX_KONAMI5, slt_msx_kon5_rd, slt_msx_kon5_wr},
	{MAP_MSX_ASCII8, slt_msx_kon5_rd, slt_msx_asc8_wr},
	{MAP_MSX_ASCII16, slt_msx_asc16_rd, slt_msx_asc16_wr},

	{MAP_GB_NOMAP, slt_gb_all_rd, slt_wr_dum},
	{MAP_GB_MBC1, slt_gb_all_rd, slt_gb_mbc1_wr},
	{MAP_GB_MBC2, slt_gb_all_rd, slt_gb_mbc2_wr},
	{MAP_GB_MBC3, slt_gb_all_rd, slt_gb_mbc3_wr},
	{MAP_GB_MBC5, slt_gb_all_rd, slt_gb_mbc5_wr},

	{MAP_UNKNOWN, slt_rd_dum, slt_wr_dum}
};

void sltSetMaper(xCartridge* slt, int id) {
	int idx = 0;
	while ((maperTab[idx].id != id) && (maperTab[idx].id != MAP_UNKNOWN)) {
		idx++;
	}
	slt->core = &maperTab[idx];
}

// common

xCartridge* sltCreate() {
	xCartridge* slt = (xCartridge*)malloc(sizeof(xCartridge));
	memset(slt, 0x00, sizeof(xCartridge));
	sltSetMaper(slt, MAP_UNKNOWN);
	return slt;
}

void sltDestroy(xCartridge* slot) {
	if (slot == NULL) return;
	sltEject(slot);
	free(slot);
}

void sltEject(xCartridge* slot) {
	if (slot->data == NULL) return;
	// save cartrige ram
	char rname[FILENAME_MAX];
	strcpy(rname, slot->name);
	strcat(rname, ".ram");
	FILE* file = fopen(rname, "wb");
	if (file) {
		fwrite(slot->ram, 0x8000, 1, file);
		fclose(file);
	}
	// free rom
	if (slot->data)
		free(slot->data);
	slot->data = NULL;
	slot->name[0] = 0x00;
}
