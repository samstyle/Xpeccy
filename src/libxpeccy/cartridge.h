#ifndef _CARTRIDGE_H
#define _CARTRIDGE_H

#if __cplusplus
extern "C" {
#endif

#include <stdio.h>

// cartridge mapers
enum {
// MSX
	MAP_UNKNOWN = -1,
	MAP_MSX_NOMAPPER = 0,
	MAP_MSX_KONAMI4,
	MAP_MSX_KONAMI5,
	MAP_MSX_ASCII8,
	MAP_MSX_ASCII16,
// GameBoy
	MAP_GB_NOMAP,
	MAP_GB_MBC1,
	MAP_GB_MBC2,
	MAP_GB_MBC3,
	MAP_GB_MBC5,
// NES
	MAP_NES_NOMAP = 0x100
};

typedef struct xCartridge xCartridge;

typedef struct {
	int id;
	unsigned char (*rd)(xCartridge*, unsigned short);
	void (*wr)(xCartridge*, unsigned short, unsigned char);
} xCardCallback;

struct xCartridge {
	unsigned ramen:1;		// ram enabling (gb)
	unsigned ramod:1;		// ram banking mode (gb)
	char name[FILENAME_MAX];
	int memMap[8];		// 8 of 8kb pages
	int mapType;		// user defined mapper type, if auto-detect didn't worked (msx)
	unsigned short ramMask;
	unsigned char ram[0x8000];	// onboard ram
	int memMask;
	int chrMask;
	unsigned char* data;		// onboard rom (malloc) = nes prg-rom
	unsigned char* chrrom;		// nes chr rom
	xCardCallback* core;
};

xCartridge* sltCreate();
void sltDestroy(xCartridge*);
void sltSetMaper(xCartridge*, int);
void sltEject(xCartridge*);

#if __cplusplus
}
#endif

#endif
