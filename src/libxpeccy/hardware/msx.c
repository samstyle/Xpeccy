#include "hardware.h"
#include "assert.h"

typedef struct {
	int type;
	unsigned char num;
} mPageNr;

mPageNr msxMemTab[4][4] = {
	{{MEM_ROM, 0}, {MEM_ROM, 1}, {MEM_RAM, 1}, {MEM_RAM, 0}},
	{{MEM_EXT,0},{MEM_EXT,0},{MEM_EXT,1},{MEM_EXT,2}},
	{{MEM_EXT,0},{MEM_EXT,0},{MEM_EXT,1},{MEM_EXT,2}},
	{{MEM_RAM, 3},{MEM_RAM, 2},{MEM_RAM, 1},{MEM_RAM, 0}}
};

int bankID[4] = {MEM_BANK0, MEM_BANK1, MEM_BANK2, MEM_BANK3};
unsigned char emptyPage[0x4000];

void msxSetMem(Computer* comp, int bank, unsigned char slot) {
	mPageNr pg = msxMemTab[slot][bank];
	if (pg.type == MEM_EXT) {
		xCartridge* cart = (slot & 1) ? &comp->msx.slotA : &comp->msx.slotB;
		unsigned char* ptr = cart->data ? cart->data + ((pg.num << 14) & cart->memMask) : emptyPage;
		memSetExternal(comp->mem, bankID[bank], pg.num, ptr);
	} else {
		memSetBank(comp->mem, bankID[bank], pg.type, pg.num);
	}
}

void msxMapMem(Computer* comp) {
	msxSetMem(comp, 0, comp->msx.slot[0] & 3);
	msxSetMem(comp, 1, comp->msx.slot[1] & 3);
	msxSetMem(comp, 2, comp->msx.slot[2] & 3);
	msxSetMem(comp, 3, comp->msx.slot[3] & 3);
}

// colors was taken from wikipedia article
// https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_palettes#Original_MSX

xColor msxPalete[16] = {
	{0,0,0},	// 0 : transparent (black)
	{0,0,0},	// 1 : black
	{62,184,73},	// 2 : medium green
	{116,208,123},	// 3 : light green
	{89,85,224},	// 4 : dark blue
	{128,118,241},	// 5 : light blue
	{185,94,81},	// 6 : dark red
	{101,219,239},	// 7 : cyan
	{219,101,89},	// 8 : medium red
	{255,137,125},	// 9 : light red
	{204,195,204},	// 10: dark yellow
	{222,208,135},	// 11: light yellow
	{58,162,165},	// 12: dark green
	{183,102,181},	// 13: magenta
	{204,204,204},	// 14: gray
	{255,255,255},	// 15: white
};

void msxReset(Computer* comp) {
	comp->vid->v9918.high = 0;
	comp->vid->v9918.vmode = -1;
	for (int i = 0; i < 16; i++) {
		comp->vid->pal[i] = msxPalete[i];
	}
}

void msxMWr(Computer* comp, unsigned short adr, unsigned char val) {
	MemPage* pg = memGetBankPtr(comp->mem, adr);
	if (pg->type != MEM_EXT) {
		stdMWr(comp, adr, val);
	} else {
		// cartridge mappers
	}
}

// AY

void msxAYIdxOut(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xfffd, val);
}

void msxAYDataOut(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xbffd, val);
}

unsigned char msxAYDataIn(Computer* comp, unsigned short port) {
	return tsIn(comp->ts, 0xbffd);
}

// 8255A

unsigned char msxA9In(Computer* comp, unsigned short port) {
	return comp->keyb->msxMap[comp->msx.keyLine];
}

void msxAAOut(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.pAA = val;
	comp->msx.keyLine = val & 0x0f;
	if (val & 0x10) {
		tapStop(comp->tape);
	} else {
		tapPlay(comp->tape);
	}
	comp->tape->levRec = (val & 0x20) ? 1 : 0;
	comp->beeplev = (val & 0x80) ? 1 : 0;
}

unsigned char msxAAIn(Computer* comp, unsigned short port) {
	return comp->msx.pAA;
}

void msxABOut(Computer* comp, unsigned short port, unsigned char val) {
	if (val & 0x80) {
		comp->msx.ppi.regC = val;
	} else {
		unsigned char mask = 0x01 << ((val >> 1) & 7);
		if (val & 1) {
			comp->msx.ppi.regC |= mask;
		} else {
			comp->msx.ppi.regC &= ~mask;
		}
	}
}


// memory

void msxA8Out(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.pA8 = val;
	comp->msx.slot[0] = val & 3;
	comp->msx.slot[1] = (val >> 2) & 3;
	comp->msx.slot[2] = (val >> 4) & 3;
	comp->msx.slot[3] = (val >> 6) & 3;
	msxMapMem(comp);
}

unsigned char msxA8In(Computer* comp, unsigned short port) {
	return comp->msx.pA8;
}

// v9918

void msx98Out(Computer* comp, unsigned short port, unsigned char val) {
	comp->vid->v9918.ram[comp->vid->v9918.vadr & 0x3fff] = val;
	comp->vid->v9918.vadr++;
}

void msx99Out(Computer* comp, unsigned short port, unsigned char val) {
	int reg;
	int vmode;
	if (comp->vid->v9918.high) {
		if (val & 0x80) {		// b7.hi = 1 : VDP register setup
			reg = val & 0x07;
			comp->vid->v9918.reg[reg] = comp->vid->v9918.data;
			comp->vid->nextbrd = comp->vid->v9918.reg[7] & 7;		// border color = BG in R7
			vmode = ((comp->vid->v9918.reg[1] & 0x10) >> 4)\
					| ((comp->vid->v9918.reg[1] & 0x08) >> 2)\
					| ((comp->vid->v9918.reg[0] & 0x0e) << 1);
			if ((vmode & 7) != comp->vid->v9918.vmode) {
				comp->vid->v9918.vmode = vmode & 7;
				switch (vmode & 7) {
					case 1: vidSetMode(comp->vid, VID_MSX_SCR0); break;	// text 40x24
					case 0: vidSetMode(comp->vid, VID_MSX_SCR1); break;	// text 32x24
					case 4: vidSetMode(comp->vid, VID_MSX_SCR2); break;	// 256x192
					case 2: vidSetMode(comp->vid, VID_MSX_SCR3); break;	// multicolor 4x4
					default: vidSetMode(comp->vid, VID_UNKNOWN); break;
				}
				// printf("MSX set video mode %i\n",vmode & 7);
			}
		} else {			// b7.hi = 0 : VDP address setup
			comp->vid->v9918.vadr = ((val & 0x3f) << 8) | comp->vid->v9918.data;
			comp->vid->v9918.wr = (val & 0x40) ? 1 : 0;
		}
	} else {
		comp->vid->v9918.data = val;
	}
	comp->vid->v9918.high ^= 1;
}

unsigned char msx98In(Computer* comp, unsigned short port) {
	unsigned char res = comp->vid->v9918.ram[comp->vid->v9918.vadr & 0x3fff];
	comp->vid->v9918.vadr++;
	return res;
}

unsigned char msx99In(Computer* comp, unsigned short port) {		// status register 0
	int res = comp->vid->v9918.sr0;
	comp->vid->v9918.sr0 &= 0x5f;	// reset b5, b7
	return res;
}

// Port map

xPort msxPortMap[] = {

	{0xff,0x90,2,2,2,dummyIn,	dummyOut},	// 90	RW	ULA5RA087 Centronic BUSY state (bit 1=1) / ULA5RA087 Centronic STROBE output (bit 0=0)
	{0xff,0x91,2,2,2,NULL,		dummyOut},	// 91	W	ULA5RA087 Centronic Printer Data

	{0xff,0x98,2,2,2,msx98In,	msx98Out},	// 98	RW	9918,9929,9938,9958,9978 VRAM Data Read/Write
	{0xff,0x99,2,2,2,msx99In,	msx99Out},	// 99	RW	9918,9929,9938,9958,9978 VDP Status Registers / VRAM Address setup (VDP Register write)

	{0xff,0xa0,2,2,2,NULL,		msxAYIdxOut},	// A0	W	I AY-3-8910 PSG Sound Generator Index
	{0xff,0xa1,2,2,2,NULL,		msxAYDataOut},	// A1	W	I AY-3-8910 PSG Sound Generator Data write
	{0xff,0xa2,2,2,2,msxAYDataIn,	NULL},		// A2	R	I AY-3-8910 PSG Sound Generator Data read

	{0xff,0xa8,2,2,2,msxA8In,	msxA8Out},	// A8	RW	I 8255A/ULA9RA041 PPI Port A Memory PSLOT Register (RAM/ROM)
	{0xff,0xa9,2,2,2,msxA9In,	NULL},		// A9	R	I 8255A/ULA9RA041 PPI Port B Keyboard column inputs
	{0xff,0xaa,2,2,2,msxAAIn,	msxAAOut},	// AA	RW	I 8255A/ULA9RA041 PPI Port C Kbd Row sel,LED,CASo,CASm
	{0xff,0xab,2,2,2,NULL,		msxABOut},	// AB	W	I 8255A/ULA9RA041 Mode select and I/O setup of A,B,C

	{0x00,0x00,2,2,2,brkIn,brkOut}
};

unsigned char msxIn(Computer* comp, unsigned short port, int dos) {
	return hwIn(msxPortMap, comp, port, dos);
}

void msxOut(Computer* comp, unsigned short port, unsigned char val, int dos) {
	hwOut(msxPortMap,comp, port, val, dos);
}
