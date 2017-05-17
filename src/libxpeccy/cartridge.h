#ifndef _CARTRIDGE_H
#define _CARTRIDGE_H

#if __cplusplus
extern "C" {
#endif

#include <stdio.h>

// MSX cartridge mapper type
enum {
	MSX_UNKNOWN = -1,
	MSX_NOMAPPER = 0,
	MSX_KONAMI4,
	MSX_KONAMI5,
	MSX_ASCII8,
	MSX_ASCII16
};

// GameBoy cartrige MBC
enum {
	GB_NOMAP = 0,
	GB_MBC1,
	GB_MBC2,
	GB_MBC3,
};

struct xCartridge {
	unsigned ramen:1;		// ram enabling (gb)
	unsigned ramod:1;		// ram banking mode (gb)
	unsigned char ram[0x8000];	// onboard ram
	unsigned short ramMask;
	unsigned char* data;		// onboard rom (malloc)
	char name[FILENAME_MAX];
	int memMask;
	int memMap[8];		// 8 of 8kb pages
	int mapType;		// user defined mapper type, if auto-detect didn't worked (msx)
	int mapAuto;		// auto detected map type OR user defined (msx)
	void (*wr)(struct xCartridge*, unsigned short, unsigned char);		// write callback
} ;
typedef struct xCartridge xCartridge;

xCartridge* sltCreate();
void sltDestroy(xCartridge*);
void sltEject(xCartridge*);

#if __cplusplus
}
#endif

#endif
