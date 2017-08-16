#include "cartridge.h"
#include <stdlib.h>
#include <string.h>

// dummy

int slt_adr_dum(xCartridge* slt, int mt, unsigned short adr) {
	return 0;
}

unsigned char slt_rd_dum(xCartridge* slt, int mt, unsigned short adr, int radr) {
	return 0xff;
}

void slt_wr_dum(xCartridge* slt, int mt, unsigned short adr, int radr, unsigned char val) {
}

// MSX mappers

unsigned char slt_msx_all_rd(xCartridge* slot, int mt, unsigned short adr, int radr) {
	if (!slot->data) return 0xff;
	return slot->data[radr & slot->memMask];
}

// no mapper
int slt_msx_nomap_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = (adr & 0x3fff) | ((adr & 0x8000) >> 1);
	radr &= slot->memMask;
	return radr;
}

// konami 4
int slt_msx_kon4_adr(xCartridge* slot, int mt, unsigned short adr) {
	int bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
	bnk = bnk ? slot->memMap[bnk] : 0;
	int radr = (bnk << 13) | (adr & 0x1fff);
	radr &= slot->memMask;
	return radr;
}

void slt_msx_kon4_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch(adr) {
		case 0x6000: slot->memMap[1] = val; break;
		case 0x8000: slot->memMap[2] = val; break;
		case 0xa000: slot->memMap[3] = val; break;
	}
}

// konami 5
int slt_msx_kon5_adr(xCartridge* slot, int mt, unsigned short adr) {
	int bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
	bnk = slot->memMap[bnk];
	int radr = (bnk << 13) | (adr & 0x1fff);
	radr &= slot->memMask;
	return radr;
}

void slt_msx_kon5_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x5000: slot->memMap[0] = val; break;
		case 0x7000: slot->memMap[1] = val; break;
		case 0x9000: slot->memMap[2] = val; break;		// TODO: SCC
		case 0xb000: slot->memMap[3] = val; break;
	}
}

// ascii8
// rd = konami5 rd
void slt_msx_asc8_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x6000: slot->memMap[0] = val; break;
		case 0x6800: slot->memMap[1] = val; break;
		case 0x7000: slot->memMap[2] = val; break;
		case 0x7800: slot->memMap[3] = val; break;
	}
}

// ascii16
int slt_msx_asc16_adr(xCartridge* slot, int mt, unsigned short adr) {
	int bnk = slot->memMap[(adr & 0x8000) >> 15];
	int radr = (bnk << 14) | (adr & 0x3fff);
	radr &= slot->memMask;
	return radr;
}

void slt_msx_asc16_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch (adr & 0xf800) {
		case 0x6000: slot->memMap[0] = val; break;	// #4000..#7FFF
		case 0x7000: slot->memMap[1] = val; break;	// #8000..#bfff
	}
}

// GAMEBOY

// reading is same for all mappers
int slt_gb_all_adr(xCartridge* slot, int mt, unsigned short adr) {
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

unsigned char slt_gb_all_rd(xCartridge* slot, int mt, unsigned short adr, int radr) {
	unsigned char res = 0xff;
	if (adr & 0x8000) {
		if (slot->ramen)
			res = slot->ram[radr & 0x7fff];
	} else if (slot->data) {
		res = slot->data[radr & slot->memMask];
	}
	return res;
}

void slt_gb_mbc1_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
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

void slt_gb_mbc2_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
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

void slt_gb_mbc3_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
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

void slt_gb_mbc5_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
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

// address for 4x8K PRG & 8x1K CHR banks
int slt_nes_all_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = 0;
	switch (mt) {
		case SLT_PRG:
			radr = (slot->memMap[(adr >> 13) & 3] << 13) | (adr & 0x1fff);
			radr &= slot->memMask;
			break;
		case SLT_CHR:
			radr = (slot->chrMap[(adr >> 10) & 7] << 10) | (adr & 0x3ff);
			radr &= slot->chrMask;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return radr;
}

unsigned char slt_nes_all_rd(xCartridge* slot, int mt, unsigned short adr, int radr) {
	unsigned char res = 0xff;
	switch (mt) {
		case SLT_PRG:
			radr &= slot->memMask;
			if (slot->data)
				res = slot->data[radr];
			break;
		case SLT_CHR:
			radr &= slot->chrMask;
			if (slot->chrrom)
				res = slot->chrrom[radr];
			break;
		case SLT_RAM:
			radr &= 0x1fff;
			res = slot->ramen ? slot->ram[radr] : 0xff;
			break;
	}
	return res;
}

// mmc1

void slt_nes_mmc1_map(xCartridge* slot) {
	int tmp;
	switch(slot->reg00 & 0x0c) {
		case 0x0:
		case 0x4:				// 1 x 32K page
			tmp = (slot->reg03 & 0x0e) << 1;
			slot->memMap[0] = tmp;
			slot->memMap[1] = tmp + 1;
			slot->memMap[2] = tmp + 2;
			slot->memMap[3] = tmp + 3;
			break;
		case 0x8:				// 8000:fixed page 0, c000:switchable
			tmp = (slot->reg03 & 0x0f) << 1;
			slot->memMap[0] = 0;
			slot->memMap[1] = 1;
			slot->memMap[2] = tmp;
			slot->memMap[3] = tmp + 1;
			break;
		case 0xc:				// 8000:switchable, c000:fixed last page
			tmp = (slot->reg03 & 0x0f) << 1;
			slot->memMap[0] = tmp;
			slot->memMap[1] = tmp + 1;
			slot->memMap[2] = slot->prglast - 1;
			slot->memMap[3] = slot->prglast;
			break;
	}
	if (slot->reg00 & 0x10) {			// 2 x 4K pages
		tmp = (slot->reg01 & 0x1f) << 2;
		slot->chrMap[0] = tmp;
		slot->chrMap[1] = tmp + 1;
		slot->chrMap[2] = tmp + 2;
		slot->chrMap[3] = tmp + 3;
		tmp = (slot->reg02 & 0x1f) << 2;
		slot->chrMap[4] = tmp;
		slot->chrMap[5] = tmp + 1;
		slot->chrMap[6] = tmp + 2;
		slot->chrMap[7] = tmp + 3;
	} else {					// 1 x 8K page
		tmp = (slot->reg01 & 0x1e) << 3;
		slot->chrMap[0] = tmp;
		slot->chrMap[1] = tmp + 1;
		slot->chrMap[2] = tmp + 2;
		slot->chrMap[3] = tmp + 3;
		slot->chrMap[0] = tmp + 4;
		slot->chrMap[1] = tmp + 5;
		slot->chrMap[2] = tmp + 6;
		slot->chrMap[3] = tmp + 7;
	}
}

void slt_nes_mmc1_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch (mt) {
		case SLT_PRG:
			if (val & 0x80) {
				slot->shift = 0x10;	// actually, if shift right causes bit carry, it means 'end of transmit'
				slot->bitcount = 0;	// but i using bits counter here. 5th transmited bit means 'end'
				slot->reg00 |= 0x0c;
			} else {
				// lsb first
				slot->shift >>= 1;
				if (val & 1)
					slot->shift |= 0x10;
				slot->bitcount++;
				if (slot->bitcount > 4) {
					switch (adr & 0x6000) {
						case 0x0000:		// control
							slot->reg00 = slot->shift & 0x1f;
							slot->ntorsk = 0x0000;
							switch (slot->shift & 3) {
								case 0:						// 1 screen (lower bank)
									slot->ntmask = 0x33ff;
									slot->ntorsk = 0x0400;
									break;
								case 1:	slot->ntmask = 0x33ff; break;		// 1 screen (upper bank)
								case 2: slot->ntmask = 0x37ff; break;		// vertical
								case 3: slot->ntmask = 0x3bff; break;		// horisontal
							}

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
					slot->shift = 0x10;
					slot->bitcount = 0;
					slt_nes_mmc1_map(slot);
				}
			}
			break;
		case SLT_RAM:
			if (slot->ramen)
				slot->ram[radr & 0x1fff] = val;
			break;
	}
}

// maper 002 :  8(16) PRG 16K pages @ 8000

void slt_nes_m002_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt != SLT_PRG) return;
	int tmp = (val & 0x0f) << 1;
	slot->memMap[0] = tmp;				// @ 8000 : switchable bank
	slot->memMap[1] = tmp + 1;
	slot->memMap[2] = slot->prglast - 1;		// @ c000 : last bank
	slot->memMap[3] = slot->prglast;
}

// maper 003 : no PRG banking, up to 256 8K CHR banks

void slt_nes_m003_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt != SLT_PRG) return;
	int tmp = val << 3;
	slot->chrMap[0] = tmp;
	slot->chrMap[1] = tmp + 1;
	slot->chrMap[2] = tmp + 2;
	slot->chrMap[3] = tmp + 3;
	slot->chrMap[0] = tmp + 4;
	slot->chrMap[1] = tmp + 5;
	slot->chrMap[2] = tmp + 6;
	slot->chrMap[3] = tmp + 7;
}

// mmc 3: PRGROM 8K, PRGRAM 8K, CHRROM 1K
// memmap 0-3 : 4 PRGROM 8K pages (0 and 2 can be switched)
// memmap 4-11 : 8 CHRROM 1K pages (0..3 and 4..7 blocks can be switched)

void slt_nes_mmc3_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch (mt) {
		case SLT_PRG:
			switch(adr & 0xe001) {
				case 0x8000:
					slot->reg00 = val;
					//printf("wr reg,%.2X\n",val);
					break;
				case 0x8001:
					//printf("wr r%i,%.2X\n",slot->reg00 & 7, val);
					switch (slot->reg00 & 0x07) {
						case 0:						// 2K CHR @ 0000 (1000)
							slot->chrMap[0] = val & 0xfe;
							slot->chrMap[1] = (val & 0xfe) | 1;
							break;
						case 1:						// 2K CHR @ 0800 (1800)
							slot->chrMap[2] = val & 0xfe;
							slot->chrMap[3] = (val & 0xfe) | 1;
							break;
						case 2: slot->chrMap[4] = val; break;		// 1K CHR @ 1000 (0000)
						case 3: slot->chrMap[5] = val; break;		// 1K CHR @ 1400 (0400)
						case 4: slot->chrMap[6] = val; break;		// 1K CHR @ 1800 (0800)
						case 5: slot->chrMap[7] = val; break;		// 1K CHR @ 1c00 (0c00)
						case 6: slot->memMap[0] = val & 0x3f; break;		// 8K PRG @ 8000 (C000)
						case 7: slot->memMap[1] = val & 0x3f; break;		// 8K PRG @ A000
					}
				case 0xa000:
					slot->ntmask = (val & 1) ? 0x3bff : 0x37ff;	// nametable mirroring
					break;
				case 0xa001:
					slot->ramwe = (val & 0x40) ? 0 : 1;
					slot->ramen = (val & 0x80) ? 1 : 0;
					break;
				case 0xc000: slot->reg03 = val; break;			// irq counter reload value
				case 0xc001: slot->irqrl = 1; break;		// reload irq counter
				case 0xe000: slot->irqen = 0; break;
				case 0xe001: slot->irqen = 1; break;
			}
			break;
		case SLT_RAM:
			if (slot->ramen && slot->ramwe)
				slot->ram[radr & 0x1fff] = val;
			break;
	}
}

int slt_nes_mmc3_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch (mt) {
		case SLT_PRG:
			if ((slot->reg00 & 0x40) && !(adr & 0x2000))		// 8000..9fff <-> c000..dfff
				adr ^= 0x4000;
			radr = (slot->memMap[(adr >> 13) & 3] << 13) | (adr & 0x1fff);
			break;
		case SLT_CHR:
			if (slot->reg00 & 0x80)
				adr ^= 0x1000;					// 0000..0fff <-> 1000..1fff
			radr = (slot->chrMap[(adr >> 10) & 7] << 10) | (adr & 0x03ff);
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return radr;
}

// maper 007 (AxROM) : 32K PRG pages

void slt_nes_m007_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt != SLT_PRG) return;
	int tmp = (val << 2) & 0x1c;		//
	slot->memMap[0] = tmp;
	slot->memMap[1] = tmp + 1;
	slot->memMap[2] = tmp + 2;
	slot->memMap[3] = tmp + 3;
	slot->ntorsk = (val & 0x40) ? 0x0400 : 0x0000;
}

// maper 071 : 2x16K PRG

void slt_nes_m071_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt == SLT_PRG) {
		if ((adr & 0xe000) == 0xc000) {
			slot->memMap[0] = (val << 1);
			slot->memMap[1] = (val << 1) + 1;
		}
	}
}

// table

static xCardCallback maperTab[] = {
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
	{MAP_NES_M002, slt_nes_all_rd, slt_nes_m002_wr, slt_nes_all_adr},
	{MAP_NES_M003, slt_nes_all_rd, slt_nes_m003_wr, slt_nes_all_adr},
	{MAP_NES_MMC3, slt_nes_all_rd, slt_nes_mmc3_wr, slt_nes_mmc3_adr},
	{MAP_NES_M007, slt_nes_all_rd, slt_nes_m007_wr, slt_nes_all_adr},
	{MAP_NES_M071, slt_nes_all_rd, slt_nes_m071_wr, slt_nes_all_adr},

	{MAP_UNKNOWN, slt_rd_dum, slt_wr_dum, slt_adr_dum}
};

xCardCallback* sltFindMaper(int id) {
	int idx = 0;
	while ((maperTab[idx].id != id) && (maperTab[idx].id != MAP_UNKNOWN)) {
		idx++;
	}
	return &maperTab[idx];
}

int sltSetMaper(xCartridge* slt, int id) {
	slt->core = sltFindMaper(id);
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
	sltSetMaper(slot, MAP_UNKNOWN);
}

unsigned char sltRead(xCartridge* slt, int mt, unsigned short adr) {
	unsigned char res = 0xff;
	if (!slt->core) return res;
	if (!slt->core->rd) return res;
	int radr = slt->core->adr(slt, mt, adr);
	res = slt->core->rd(slt, mt, adr, radr);
	if (mt != SLT_PRG) return res;
	if (!slt->brkMap) return res;
	if (slt->brkMap[radr] & MEM_BRK_RD)
		slt->brk = 1;
	return res;
}

void sltWrite(xCartridge* slt, int mt, unsigned short adr, unsigned char val) {
	if (!slt->core) return;
	if (!slt->core->wr) return;
	int radr = slt->core->adr(slt, mt, adr);
	slt->core->wr(slt, mt, adr, radr, val);
}

