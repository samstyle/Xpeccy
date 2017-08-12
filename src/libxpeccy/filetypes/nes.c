#include <filetypes.h>

#pragma pack(push, 1)

typedef struct {
	char sign[3];			// "NES"
	char id;			// 0x1a
	unsigned char nprg;
	unsigned char nchr;
	unsigned char flag1;
	unsigned char flag2;
	unsigned char res[8];
} xNesHeader;

#pragma pack(pop)

int loadNes(Computer* comp, const char* name) {
	xCartridge* slot = comp->slot;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	int res = ERR_OK;

	xNesHeader hd;
	fread((char*)&hd, sizeof(xNesHeader), 1, file);


	if (strncmp(hd.sign, "NES", 3) || (hd.id != 0x1a)) {
		res = ERR_NES_HEAD;
	} else {

		printf("\nPRGROM:%i CHRROM:%i\n", hd.nprg, hd.nchr);
		int tsiz = 1;
		while (tsiz < (hd.nprg << 14)) {tsiz <<= 1;}	// up to 2^n - 1

		slot->data = realloc(slot->data, tsiz);		// PRG-ROM / 16K
		slot->brkMap = realloc(slot->brkMap, tsiz);
		memset(slot->brkMap, 0x00, tsiz);
		slot->memMask = tsiz - 1;
		fread(slot->data, hd.nprg << 14, 1, file);

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
		slot->memMap[1] = hd.nprg - 1;					// 0xc000 : last page

		printf("memMask %X; %i %i\n", slot->memMask, slot->memMap[0], slot->memMap[1]);

		if (hd.flag1 & 8) {
			comp->slot->ntmask = 0x3fff;		// full 4-screen nametable
		} else if (hd.flag1 & 1) {
			comp->slot->ntmask = 0x37ff;		// down screens (2800, 2c00) = upper screens (2000, 2400) : ignore bit 11
		} else {
			comp->slot->ntmask = 0x3bff;		// right sceens (2400, 2c00) = left sceens (2000, 2800) : ignore bit 10
		}

		printf("mirroring mask %.4X\n", comp->slot->ntmask);

		int maper = ((hd.flag1 >> 4) & 0x0f) | (hd.flag2 & 0xf0);
		if (!sltSetMaper(slot, maper | 0x100))				// #01xx = nes mapers
			res = ERR_NES_MAPPER;

		printf("maper %X\n", maper | 0x100);
	}

	fclose(file);

	return res;
}
