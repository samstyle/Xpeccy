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

		printf("\nmaper %X\n", maper);
		xCardCallback* core = sltFindMaper(maper | 0x100);
		if (core->id == MAP_UNKNOWN) {
			res = ERR_NES_MAPPER;
		} else {
			slot->core = core;

			printf("PRGROM:%i\n", hd.nprg);
			int tsiz = 1;
			while (tsiz < (hd.nprg << 14)) {tsiz <<= 1;}	// up to 2^n - 1
			slot->data = realloc(slot->data, tsiz);		// PRG-ROM / 16K
			slot->brkMap = realloc(slot->brkMap, tsiz);
			memset(slot->brkMap, 0x00, tsiz);
			slot->memMask = tsiz - 1;
			slot->prglast = slot->memMask >> 13;
			printf("memMask:%X   last8K:%X\n",slot->memMask, slot->prglast);
			fread(slot->data, hd.nprg << 14, 1, file);

			printf("CHRROM:%i\n",hd.nchr);
			if (hd.nchr == 0) {				// no CHR-ROM
				if (slot->chrrom)
					free(slot->chrrom);
				slot->chrrom = NULL;
			} else {					// CHR-ROM must be mapped @ 1st 8K of PPU memory
				tsiz = 1;
				while (tsiz < (hd.nchr << 13)) {tsiz <<= 1;}
				slot->chrrom = realloc(slot->chrrom, tsiz);	// CHR-ROM / 8K
				slot->chrMask = tsiz - 1;
				fread(slot->chrrom, hd.nchr << 13, 1, file);
			}

			slot->memMap[0] = 0x00;						// 0x8000 : 1st page
			slot->memMap[1] = 0x01;
			slot->memMap[2] = slot->prglast - 1;				// 0xc000 : last page
			slot->memMap[3] = slot->prglast;
			for (int i = 0; i < 8; i++)
				slot->chrMap[i] = i;

			// printf("memMask %X; %i %i\n", slot->memMask, slot->memMap[0], slot->memMap[1]);

			if (hd.flag6 & 8) {
				comp->slot->ntmask = 0x3fff;		// full 4-screen nametable
			} else if (hd.flag6 & 1) {
				comp->slot->ntmask = 0x37ff;		// down screens (2800, 2c00) = upper screens (2000, 2400) : ignore bit 11
			} else {
				comp->slot->ntmask = 0x3bff;		// right sceens (2400, 2c00) = left sceens (2000, 2800) : ignore bit 10
			}
			comp->slot->ntorsk = 0x0000;

			comp->nes.pal = pal ? 1 : 0;
			compUpdateTimings(comp);
		}
	}

	fclose(file);

	return res;
}
