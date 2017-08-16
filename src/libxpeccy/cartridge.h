#ifndef CARTRIDGE_H
#define CARTRIDGE_H

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
	MAP_NES_NOMAP = 0x100,
	MAP_NES_MMC1,
	MAP_NES_M002,
	MAP_NES_M003,
	MAP_NES_MMC3,
	MAP_NES_MMC5,
	MAP_NES_M007 = 0x107,
	MAP_NES_M047 = 0x12f,
	MAP_NES_M071 = 0x147
};

enum {
	SLT_PRG = 1,
	SLT_CHR,		// access by NES ppu
	SLT_RAM
};

// memory flags
#define	MEM_BRK_FETCH	1
#define	MEM_BRK_RD	(1<<1)
#define	MEM_BRK_WR	(1<<2)
#define	MEM_BRK_ANY	(MEM_BRK_FETCH | MEM_BRK_RD | MEM_BRK_WR)
#define MEM_BRK_TFETCH	(1<<3)
#define MEM_TYPE	(3<<6);		// b6,7 : memory cell type (for debugger)

typedef struct xCartridge xCartridge;

typedef struct {
	int id;
	unsigned char (*rd)(xCartridge*, int, unsigned short, int);
	void (*wr)(xCartridge*, int, unsigned short, int, unsigned char);
	int (*adr)(xCartridge*, int, unsigned short);
} xCardCallback;

struct xCartridge {
	unsigned ramen:1;		// ram enabled (gb, nes)
	unsigned ramwe:1;		// ram writing enable (nes)
	unsigned ramod:1;		// ram banking mode (gb)
	unsigned brk:1;
	unsigned irqen:1;		// irq enable (nes)
	unsigned irqrl:1;		// irq reload request (nes)
	unsigned irq:1;			// irq signal (nes)

	char name[FILENAME_MAX];
	int memMap[4];			// 4x8K PRG pages
	int chrMap[8];			// 8x1K CHR pages
	int prglast;
	int mapType;			// user defined mapper type, if auto-detect didn't worked (msx)

	unsigned char shift;		// shift register (mmc1)
	int bitcount;			// counter of bits writing to shift register (mmc1)
	unsigned char reg00;		// control registers
	unsigned char reg01;
	unsigned char reg02;
	unsigned char reg03;
	unsigned char icnt;		// irq counter (nes mmc3)

	unsigned char* data;		// onboard rom (malloc) = nes prg-rom
	unsigned char* brkMap;
	int memMask;
	unsigned char* chrrom;		// nes chr rom (malloc)
	int chrMask;
	unsigned short ntmask;		// nes nametables mirroring control (AND)
	unsigned short ntorsk;		// same (OR)
	unsigned char ram[0x8000];	// onboard ram (32K max)
	unsigned short ramMask;
	xCardCallback* core;
};

xCartridge* sltCreate();
void sltDestroy(xCartridge*);
void sltEject(xCartridge*);

xCardCallback* sltFindMaper(int);
int sltSetMaper(xCartridge*, int);

unsigned char sltRead(xCartridge*, int, unsigned short);
void sltWrite(xCartridge*, int, unsigned short, unsigned char);

#if __cplusplus
}
#endif

#endif
