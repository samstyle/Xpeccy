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

int loadNes(xCartridge* slot, const char* name) {
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
		while (tsiz < (hd.nprg << 14)) {tsiz <<= 1;}		// up to 2^n - 1

		slot->data = realloc(slot->data, tsiz);		// PRG-ROM / 16K
		slot->memMask = tsiz - 1;
		fread(slot->data, tsiz, 1, file);

		tsiz = 1;
		while (tsiz < (hd.nchr << 13)) {tsiz <<= 1;}
		slot->chrrom = realloc(slot->chrrom, tsiz);	// CHR-ROM / 8K
		slot->chrMask = tsiz - 1;
		fread(slot->chrrom, tsiz, 1, file);

		slot->memMap[0] = 0x00;						// 0x8000 : 1st page
		slot->memMap[1] = hd.nprg - 1;					// 0xc000 : last page

		printf("memMask %X; %i %i\n", slot->memMask, slot->memMap[0], slot->memMap[1]);

		int maper = ((hd.flag1 >> 4) & 0x0f) | (hd.flag2 & 0xf0);
		sltSetMaper(slot, maper | 0x100);				// #01xx = nes mapers

		printf("maper %X\n", maper | 0x100);
	}

	fclose(file);

	return res;
}
