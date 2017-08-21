#include <filetypes.h>

#pragma pack(push, 1)

typedef struct {
	char sign[3];			// "NES"
	char id;			// 0x1a
	unsigned char nprg;		// x16K PRG
	unsigned char nchr;		// x8K CHR
	unsigned char flag6;
	unsigned char flag7;
	unsigned char flag8;
	unsigned char flag9;
	unsigned char flagA;
	unsigned char res[5];
} xNesHeader;

#pragma pack(pop)

enum {
	NES_HD_07 = 0,
	NES_HD_10,
	NES_HD_20
};

int loadNes(Computer* comp, const char* name) {
	xCartridge* slot = comp->slot;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	int res = ERR_OK;
	int type;
	int pal = 0;
	int maper;

	if (strstr(name, "(E)")) pal = 1;		// europe : pal

	xNesHeader hd;
	fread((char*)&hd, sizeof(xNesHeader), 1, file);

	if (strncmp(hd.sign, "NES", 3) || (hd.id != 0x1a)) {
		res = ERR_NES_HEAD;
	} else {
		switch (hd.flag7 & 0x0c) {
			case 0x08: type = NES_HD_20; break;
			case 0x00:
				// check bytes 12-15: if 0: iNES1.0
				type = NES_HD_10;
				for (int i = 1; i < 5; i++)
					if (hd.res[i]) type = NES_HD_07;
				break;
			default: type = NES_HD_07; break;
		}
		switch (type) {
			case NES_HD_10:
				maper = ((hd.flag6 >> 4) & 0x0f) | (hd.flag7 & 0xf0);
				break;
			case NES_HD_20:
				maper = ((hd.flag6 >> 4) & 0x0f) | (hd.flag7 & 0xf0);		// | 4 bits more
				pal = hd.flag9 & 1;
				break;
			default:
				maper = (hd.flag6 >> 4) & 0x0f;
				break;
		}

		printf("\nMapper #%.3X\n", maper);
		xCardCallback* core = sltFindMaper(maper | 0x100);
		if (core->id == MAP_UNKNOWN) {
			res = ERR_NES_MAPPER;
		} else {
			slot->core = core;

			int tsiz = 1;
			while (tsiz < (hd.nprg << 14))		// up to 2^n
				tsiz <<= 1;
			slot->data = realloc(slot->data, tsiz);		// PRGROM
			slot->brkMap = realloc(slot->brkMap, tsiz);	// PRGROM breakpoints map
			memset(slot->brkMap, 0x00, tsiz);		// init
			slot->memMask = tsiz - 1;
			slot->prglast = slot->memMask >> 14;		// last 16K page number
			printf("PRGROM:%i x 16K, mask %X\n", hd.nprg, slot->memMask);
			fread(slot->data, hd.nprg << 14, 1, file);

			if (hd.nchr == 0) {				// no CHR-ROM
				if (slot->chrrom)
					free(slot->chrrom);
				slot->chrrom = NULL;
				slot->chrMask = 0;
			} else {					// CHR-ROM must be mapped @ 1st 8K of PPU memory
				tsiz = 1;
				while (tsiz < (hd.nchr << 13)) {tsiz <<= 1;}
				slot->chrrom = realloc(slot->chrrom, tsiz);	// CHR-ROM / 8K
				slot->chrMask = tsiz - 1;
				fread(slot->chrrom, hd.nchr << 13, 1, file);
			}
			printf("CHRROM:%i x  8K, mask %X\n",hd.nchr, slot->chrMask);
/*
			slot->regCT = 0;
			slot->reg00 = 0;
			slot->reg01 = 0;
			slot->reg02 = 0;
			slot->reg03 = 0;
			slot->reg04 = 0;
			slot->reg05 = 0;
			slot->reg06 = 0;
			slot->reg07 = 0;
*/
			if (hd.flag6 & 8) {
				printf("Mirroring : quatro\n");
				slot->mirror = NES_NT_QUATRO;		// full 4-screen nametable
			} else if (hd.flag6 & 1) {
				printf("Mirroring : vert\n");
				slot->mirror = NES_NT_VERT;		// down screens (2800, 2c00) = upper screens (2000, 2400) : CIRAM A10 = VA10
			} else {
				printf("Mirroring : horiz\n");
				slot->mirror = NES_NT_HORIZ;		// right sceens (2400, 2c00) = left sceens (2000, 2800) : CIRAM A10 = VA11
			}
			slot->ramMask = 0x1fff;		// TODO: ram bankswitches
			slot->blk1 = 0;
			slot->ramen = 1;
			slot->ramwe = 1;
			slot->irqen = 0;
			slot->irq = 0;

			comp->nes.pal = pal ? 1 : 0;
			compUpdateTimings(comp);
		}
	}

	fclose(file);

	return res;
}
