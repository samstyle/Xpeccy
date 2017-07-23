#include "cartridge.h"
#include <stdlib.h>
#include <string.h>

// dummy

int slt_adr_dum(xCartridge* slt, unsigned short adr) {
	return 0;
}

unsigned char slt_rd_dum(xCartridge* slt, unsigned short adr, int radr) {
	return 0xff;
}

void slt_wr_dum(xCartridge* slt, unsigned short adr, int radr, unsigned char val) {
}

// MSX mappers

unsigned char slt_msx_all_rd(xCartridge* slot, unsigned short adr, int radr) {
	return slot->data[radr & slot->memMask];
}

// no mapper
int slt_msx_nomap_adr(xCartridge* slot, unsigned short adr) {
	int radr = (adr & 0x3fff) | ((adr & 0x8000) >> 1);
	radr &= slot->memMask;
	return radr;
}

// konami 4
int slt_msx_kon4_adr(xCartridge* slot, unsigned short adr) {
	int bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
	bnk = bnk ? slot->memMap[bnk] : 0;
	int radr = (bnk << 13) | (adr & 0x1fff);
	radr &= slot->memMask;
	return radr;
}

void slt_msx_kon4_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
	switch(adr) {
		case 0x6000: slot->memMap[1] = val; break;
		case 0x8000: slot->memMap[2] = val; break;
		case 0xa000: slot->memMap[3] = val; break;
	}
}

// konami 5
int slt_msx_kon5_adr(xCartridge* slot, unsigned short adr) {
	int bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
	bnk = slot->memMap[bnk];
	int radr = (bnk << 13) | (adr & 0x1fff);
	radr &= slot->memMask;
	return radr;
}

void slt_msx_kon5_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x5000: slot->memMap[0] = val; break;
		case 0x7000: slot->memMap[1] = val; break;
		case 0x9000: slot->memMap[2] = val; break;		// TODO: SCC
		case 0xb000: slot->memMap[3] = val; break;
	}
}

// ascii8
// rd = konami5 rd
void slt_msx_asc8_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x6000: slot->memMap[0] = val; break;
		case 0x6800: slot->memMap[1] = val; break;
		case 0x7000: slot->memMap[2] = val; break;
		case 0x7800: slot->memMap[3] = val; break;
	}
}

// ascii16
int slt_msx_asc16_adr(xCartridge* slot, unsigned short adr) {
	int bnk = slot->memMap[(adr & 0x8000) >> 15];
	int radr = (bnk << 14) | (adr & 0x3fff);
	radr &= slot->memMask;
	return radr;
}

void slt_msx_asc16_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x6000: slot->memMap[0] = val; break;	// #4000..#7FFF
		case 0x7000: slot->memMap[1] = val; break;	// #8000..#bfff
	}
}

// GAMEBOY

// reading is same for all mappers
int slt_gb_all_adr(xCartridge* slot, unsigned short adr) {
	int radr;
	if (adr & 0x8000) {		// ram
		radr = (slot->memMap[1] << 13) | (adr & 0x1fff);
		radr &= 0x7fff;
	} else {
		radr = adr & 0x3fff;
		if (adr & 0x4000) {
			radr |= (slot->memMap[0] << 14);
			radr &= slot->memMask;
		}
	}
	return radr;
}

unsigned char slt_gb_all_rd(xCartridge* slot, unsigned short adr, int radr) {
	unsigned char res = 0xff;
	if (adr & 0x8000) {
		if (slot->ramen)
			res = slot->ram[radr & 0x7fff];
	} else {
		res = slot->data[radr & slot->memMask];
	}
	return res;
}

void slt_gb_mbc1_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
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

void slt_gb_mbc2_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
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

void slt_gb_mbc3_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
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

void slt_gb_mbc5_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
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

// nes

// common/nomaper
int slt_nes_all_adr(xCartridge* slot, unsigned short adr) {
	int radr;
	if (adr & 0x4000) {		// page 0 (0x8000..0xbfff)
		radr = (slot->memMap[1] << 14) | (adr & 0x3fff);
	} else {			// page 1 (0xc000..0xffff)
		radr = (slot->memMap[0] << 14) | (adr & 0x3fff);
	}
	radr &= slot->memMask;
	return radr;
}

unsigned char slt_nes_all_rd(xCartridge* slot, unsigned short adr, int radr) {
	radr &= slot->memMask;
	return slot->data[radr];
}

// mmc1

void slt_nes_mmc1_map(xCartridge* slot) {
	switch(slot->reg00 & 0x0c) {
		case 0x0:
		case 0x4:				// 1 x 32K page
			slot->memMap[0] = slot->reg03 & 0x0e;
			slot->memMap[1] = slot->memMap[0] + 1;
			break;
		case 0x8:				// 8000:fixed page 0, c000:switchable
			slot->memMap[0] = 0;
			slot->memMap[1] = slot->reg03 & 0x0f;
			break;
		case 0xc:				// 8000:switchable, c000:fixed last page
			slot->memMap[0] = slot->reg03 & 0x0f;
			slot->memMap[1] = slot->memMask >> 14;
			break;
	}
	if (slot->reg00 & 0x10) {			// 2 x 4K pages
		slot->memMap[2] = slot->reg01 & 0x1f;
		slot->memMap[3] = slot->reg02 & 0x1f;
	} else {					// 1 x 8K page
		slot->memMap[2] = slot->reg01 & 0x1e;
		slot->memMap[3] = slot->memMap[2] + 1;
	}
}

void slt_nes_mmc1_wr(xCartridge* slot, unsigned short adr, int radr, unsigned char val) {
	if (val & 0x80) {
		slot->shift = 0;
		slot->smask = 1;
		slot->reg00 |= 0x0c;
	} else {
		// lsb first
		if (val & 1)
			slot->shift |= slot->smask;
		slot->smask <<= 1;
		if (slot->smask > 0x10) {
			switch (adr & 0x6000) {
				case 0x0000:		// control
					slot->reg00 = slot->shift & 0x1f;
					break;
				case 0x2000:		// chr bank 0
					slot->reg01 = slot->shift & 0x1f;
					break;
				case 0x4000:		// chr bank 1
					slot->reg02 = slot->shift & 0x1f;
					break;
				case 0x6000:		// prg bank
					slot->reg03 = slot->shift & 0x0f;
					slot->ramen = (slot->shift & 0x10) ? 1 : 0;
					break;
			}
			slot->shift = 0;
			slot->smask = 1;
		}
	}
	slt_nes_mmc1_map(slot);
}

// table

xCardCallback maperTab[] = {
	{MAP_MSX_NOMAPPER, slt_msx_all_rd, slt_wr_dum, slt_msx_nomap_adr},
	{MAP_MSX_KONAMI4, slt_msx_all_rd, slt_msx_kon4_wr, slt_msx_kon4_adr},
	{MAP_MSX_KONAMI5, slt_msx_all_rd, slt_msx_kon5_wr, slt_msx_kon5_adr},
	{MAP_MSX_ASCII8, slt_msx_all_rd, slt_msx_asc8_wr, slt_msx_kon5_adr},
	{MAP_MSX_ASCII16, slt_msx_all_rd, slt_msx_asc16_wr, slt_msx_asc16_adr},

	{MAP_GB_NOMAP, slt_gb_all_rd, slt_wr_dum, slt_gb_all_adr},
	{MAP_GB_MBC1, slt_gb_all_rd, slt_gb_mbc1_wr, slt_gb_all_adr},
	{MAP_GB_MBC2, slt_gb_all_rd, slt_gb_mbc2_wr, slt_gb_all_adr},
	{MAP_GB_MBC3, slt_gb_all_rd, slt_gb_mbc3_wr, slt_gb_all_adr},
	{MAP_GB_MBC5, slt_gb_all_rd, slt_gb_mbc5_wr, slt_gb_all_adr},

	{MAP_NES_NOMAP, slt_nes_all_rd, slt_wr_dum, slt_nes_all_adr},
	{MAP_NES_MMC1, slt_nes_all_rd, slt_nes_mmc1_wr, slt_nes_all_adr},

	{MAP_UNKNOWN, slt_rd_dum, slt_wr_dum, slt_adr_dum}
};

int sltSetMaper(xCartridge* slt, int id) {
	int idx = 0;
	while ((maperTab[idx].id != id) && (maperTab[idx].id != MAP_UNKNOWN)) {
		idx++;
	}
	slt->core = &maperTab[idx];
	return (slt->core->id == MAP_UNKNOWN) ? 0 : 1;
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
	// free brk map
	if (slot->brkMap)
		free(slot->brkMap);
	slot->brkMap = NULL;
	// free chr-rom
	if (slot->chrrom)
		free(slot->chrrom);
	slot->chrrom = NULL;
}

unsigned char sltRead(xCartridge* slt, unsigned short adr) {
	if (!slt->data) return 0xff;
	if (!slt->core) return 0xff;
	if (!slt->core->rd) return 0xff;
	int radr = slt->core->adr(slt, adr);
	if (slt->brkMap[radr] & MEM_BRK_RD)
		slt->brk = 1;
	return slt->core->rd(slt, adr, radr);
}

void sltWrite(xCartridge* slt, unsigned short adr, unsigned char val) {
	if (!slt->data) return;
	if (!slt->core) return;
	if (!slt->core->wr) return;
	int radr = slt->core->adr(slt, adr);
	slt->core->wr(slt, adr, radr, val);
}
