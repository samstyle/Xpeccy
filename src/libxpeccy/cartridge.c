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

unsigned char slt_nes_all_rd(xCartridge* slot, int mt, unsigned short adr, int radr) {
	unsigned char res = 0xff;
	switch (mt) {
		case SLT_PRG:
			if (slot->data)
				res = slot->data[radr & slot->memMask];
			break;
		case SLT_CHR:
			if (slot->chrrom)
				res = slot->chrrom[radr & slot->chrMask];
			break;
		case SLT_RAM:
			if (slot->ramen)
				res = slot->ram[radr & slot->ramMask];
			break;
	}
	return res;
}

// translate ppu nt vadr

//_Name Table____________NT0___NT1___NT2___NT3
// Horizontal Mirroring  BLK0  BLK0  BLK1  BLK1
// Vertical Mirroring    BLK0  BLK1  BLK0  BLK1
// Four-screen           BLK0  BLK1  BLK2  BLK3
unsigned short nes_nt_vadr(xCartridge* slot, unsigned short adr) {
	switch (slot->mirror) {
		case NES_NT_SINGLE:
			adr = (adr & 0x33ff) | (slot->blk1 ? 0x800 : 0x000);
			break;
		case NES_NT_VERT:			// CIRAM A10 = VA10
			adr = (adr & 0x37ff) | (slot->blk1 ? 0x800 : 0x000);
			break;
		case NES_NT_HORIZ:			// CIRAM A10 = VA11
			adr = (adr & 0x33ff) | ((adr >> 1) & 0x400) | (slot->blk1 ? 0x800 : 0x000);
			break;
		// case NES_NT_QUATRO: no changes
	}
	return adr;
}

// nrom

int slt_nes_nrom_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = 0;
	switch (mt) {
		case SLT_PRG:
			radr = adr & 0x7fff;
			radr &= slot->memMask;
			break;
		case SLT_CHR:
			radr = adr & 0x1fff;
			radr &= slot->chrMask;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return radr;
}


// mmc1

int slt_nes_mmc1_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch (mt) {
		case SLT_PRG:
			switch (slot->reg00 & 0x0c) {
				case 0x00:		// 1x32K (reg3)
				case 0x04:
					radr = (slot->reg03 << 15) | (adr & 0x7fff);
					break;
				case 0x08:		// 2x16K; 8000:fix0; c000:reg3
					if (adr & 0x4000) {
						radr = (slot->reg03 << 14) | (adr & 0x3fff);
					} else {
						radr = adr & 0x3fff;
					}
					break;
				case 0x0c:		// 2x16K; 8000:reg3; c000:last
					if (adr & 0x4000) {
						radr = (slot->prglast << 14) | (adr & 0x3fff);
					} else {
						radr = (slot->reg03 << 14) | (adr & 0x3fff);
					}
					break;
			}
			radr &= slot->memMask;
			break;
		case SLT_CHR:
			if (slot->reg00 & 0x10) {		// 2x4K (reg1,reg2)
				if (adr & 0x1000) {
					radr = (slot->reg02 << 12) | (adr & 0xfff);
				} else {
					radr = (slot->reg01 << 12) | (adr & 0xfff);
				}
			} else {				// 1x8K (reg1)
				radr = (slot->reg01 << 13) | (adr & 0x1fff);
			}
			radr &= slot->chrMask;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return  radr;
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
							slot->blk1 = 0;
							switch (slot->shift & 3) {
								case 0:	slot->mirror = NES_NT_SINGLE;		// 1 screen (lower bank)
									slot->blk1 = 1;
									break;
								case 1:	slot->mirror = NES_NT_SINGLE;		// 1 screen (upper bank)
									break;
								case 2: slot->mirror = NES_NT_VERT;		// vertical
									break;
								case 3: slot->mirror = NES_NT_HORIZ;		// horisontal
									break;
							}
							break;
						case 0x2000:		// chr bank 0 (4K | 8K @ 0000)
							slot->reg01 = slot->shift & 0x1f;
							break;
						case 0x4000:		// chr bank 1 (4K @ 1000)
							slot->reg02 = slot->shift & 0x1f;
							break;
						case 0x6000:		// prg bank/ramen
							slot->reg03 = slot->shift & 0x0f;
							slot->ramen = (slot->shift & 0x10) ? 0 : 1;
							break;
					}
					slot->shift = 0x10;
					slot->bitcount = 0;
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

int slt_nes_unrom_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch (mt) {
		case SLT_PRG:			// 2x16K banked
			if (adr & 0x4000) {		// c000:fix.last
				radr = (slot->prglast << 14) | (adr & 0x3fff);
			} else {			// 8000:reg0
				radr = (slot->reg00 << 14) | (adr & 0x3fff);
			}
			radr &= slot->memMask;
			break;
		case SLT_CHR:			// 1x8K non-banked
			radr = adr & 0x1fff;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return  radr;
}

void slt_nes_unrom_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt != SLT_PRG) return;
	slot->reg00 = val;
}

// maper 003 (CNROM) : no PRG banking, up to 256 8K CHR banks

int slt_nes_cnrom_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch (mt) {
		case SLT_PRG:
			radr = adr & 0x7fff;
			break;
		case SLT_CHR:
			radr = (slot->reg00 << 13) | (adr & 0x1fff);
			radr &= slot->chrMask;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return  radr;
}

void slt_nes_cnrom_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt != SLT_PRG) return;
	slot->reg00 = val & 3;		// chr 8K bank
}

// maper 004 (MMC3) : PRGROM 4x8K, PRGRAM 8K, CHRROM 2x2K+4x1K
// PRGROM: 4x8K pages (0 and 2 can be switched)
// CHRROM 2x2K + 4x1K pages (blocks can be switched)

int slt_nes_mmc3_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch (mt) {
		case SLT_PRG:
			if ((slot->regCT & 0x40) && !(adr & 0x2000))					// switch 8000..9fff <-> c000..dfff
				adr ^= 0x4000;
			switch(adr & 0x6000) {
				case 0x0000: radr = (slot->reg06 << 13) | (adr & 0x1fff); break;
				case 0x2000: radr = (slot->reg07 << 13) | (adr & 0x1fff); break;
				default: radr = (slot->prglast << 14) | (adr & 0x3fff); break;		// c000..ffff (unswitched):allways last 16K page
			}
			radr &= slot->memMask;
			break;
		case SLT_CHR:
			if (slot->regCT & 0x80)								// switch 0000..0fff <-> 1000..1fff
				adr ^= 0x1000;
			switch (adr & 0x1c00) {
				case 0x0000:
				case 0x0400: radr = (slot->reg00 << 10) | (adr & 0x7ff); break;		// b0 of 2K regs ignored
				case 0x0800:
				case 0x0c00: radr = (slot->reg01 << 10) | (adr & 0x7ff); break;
				case 0x1000: radr = (slot->reg02 << 10) | (adr & 0x3ff); break;
				case 0x1400: radr = (slot->reg03 << 10) | (adr & 0x3ff); break;
				case 0x1800: radr = (slot->reg04 << 10) | (adr & 0x3ff); break;
				case 0x1c00: radr = (slot->reg05 << 10) | (adr & 0x3ff); break;
			}
			radr &= slot->chrMask;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return radr;
}

void slt_nes_mmc3_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	switch (mt) {
		case SLT_PRG:
			switch(adr & 0xe001) {
				case 0x8000:
					slot->regCT = val;
					break;
				case 0x8001:
					switch (slot->regCT & 0x07) {
						case 0:	slot->reg00 = val & 0xfe; break;	// 2K CHR @ 0000 (1000)
						case 1:	slot->reg01 = val & 0xfe; break;	// 2K CHR @ 0800 (1800)
						case 2: slot->reg02 = val; break;		// 1K CHR @ 1000 (0000)
						case 3: slot->reg03 = val; break;		// 1K CHR @ 1400 (0400)
						case 4: slot->reg04 = val; break;		// 1K CHR @ 1800 (0800)
						case 5: slot->reg05 = val; break;		// 1K CHR @ 1c00 (0c00)
						case 6: slot->reg06 = val; break;		// 8K PRG @ 8000 (C000)
						case 7: slot->reg07 = val; break;		// 8K PRG @ A000
					}
					break;
				case 0xa000:
					slot->mirror = (val & 1) ? NES_NT_HORIZ : NES_NT_VERT;
					slot->blk1 = 0;
					break;
				case 0xa001:
					slot->ramwe = (val & 0x40) ? 0 : 1;
					slot->ramen = (val & 0x80) ? 1 : 0;
					break;
				case 0xc000: slot->ival = val; break;			// irq counter reload value
				case 0xc001: slot->irqrl = 1; break;			// reload irq counter
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

// maper 007 (AxROM) : 32K PRG pages

int slt_nes_aorom_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch(mt) {
		case SLT_PRG:
			radr = (slot->reg00 << 15) | (adr & 0x7fff);
			break;
		case SLT_CHR:
			radr = adr & 0x1fff;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return  radr;
}

void slt_nes_aorom_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt != SLT_PRG) return;
	slot->reg00 = val & 7;
	slot->blk1 = (val & 0x10) ? 1 : 0;
}

// maper 071 : 2x16K PRG

int slt_nes_camerica_adr(xCartridge* slot, int mt, unsigned short adr) {
	int radr = -1;
	switch(mt) {
		case SLT_PRG:
			if (adr & 0x4000) {
				radr = (slot->prglast << 14) | (adr & 0x3fff);
			} else {
				radr = (slot->reg00 << 14) | (adr & 0x3fff);
			}
			radr &= slot->memMask;
			break;
		case SLT_CHR:
			radr = adr & 0x1fff;
			break;
		case SLT_RAM:
			radr = adr & 0x1fff;
			break;
	}
	return radr;
}

void slt_nes_camerica_wr(xCartridge* slot, int mt, unsigned short adr, int radr, unsigned char val) {
	if (mt == SLT_PRG) {
		switch(adr & 0xe000) {			// c000..dfff bank switch
			case 0xc000:
				slot->reg00 = val;
				break;
			case 0xe000:			// e000..ffff A0 = CIC circuit latch (TODO: to learn wut iz it)
				break;
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

	{MAP_NES_NROM, slt_nes_all_rd, slt_wr_dum, slt_nes_nrom_adr},
	{MAP_NES_MMC1, slt_nes_all_rd, slt_nes_mmc1_wr, slt_nes_mmc1_adr},
	{MAP_NES_UNROM, slt_nes_all_rd, slt_nes_unrom_wr, slt_nes_unrom_adr},
	{MAP_NES_CNROM, slt_nes_all_rd, slt_nes_cnrom_wr, slt_nes_cnrom_adr},
	{MAP_NES_MMC3, slt_nes_all_rd, slt_nes_mmc3_wr, slt_nes_mmc3_adr},
	{MAP_NES_AOROM, slt_nes_all_rd, slt_nes_aorom_wr, slt_nes_aorom_adr},
	{MAP_NES_CAMERICA, slt_nes_all_rd, slt_nes_camerica_wr, slt_nes_camerica_adr},

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
	if (slot->data) {
		free(slot->data);
		slot->data = NULL;
	}
	slot->name[0] = 0x00;
	// free brk map
	if (slot->brkMap) {
		free(slot->brkMap);
		slot->brkMap = NULL;
	}
	// free chr-rom
	if (slot->chrrom) {
		free(slot->chrrom);
		slot->chrrom = NULL;
	}
	sltSetMaper(slot, MAP_UNKNOWN);
}

unsigned char sltRead(xCartridge* slt, int mt, unsigned short adr) {
	unsigned char res = 0xff;
	if (!slt->core) return res;
	if (!slt->core->rd) return res;
	if (!slt->data) return res;
	int radr = slt->core->adr(slt, mt, adr);
	res = slt->core->rd(slt, mt, adr, radr);
	if (mt != SLT_PRG) return res;
	if (!slt->brkMap) return res;
	if (slt->brkMap[radr & slt->memMask] & MEM_BRK_RD)
		slt->brk = 1;
	return res;
}

void sltWrite(xCartridge* slt, int mt, unsigned short adr, unsigned char val) {
	if (!slt->core) return;
	if (!slt->core->wr) return;
	if (!slt->data) return;
	int radr = slt->core->adr(slt, mt, adr);
	slt->core->wr(slt, mt, adr, radr, val);
	if (slt->brkMap[radr & slt->memMask] & MEM_BRK_WR)
		slt->brk = 1;
}

