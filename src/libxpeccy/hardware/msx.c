#include "hardware.h"

typedef struct {
	int type;
	unsigned char num;
} mPageNr;

unsigned char msxSlotRd(unsigned short adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	if (!slot->data) return 0xff;
	int bnk;
	int radr = 0;
	switch(slot->mapAuto) {
		case MSX_NOMAPPER:
			radr = (adr & 0x3fff) | ((adr & 0x8000) >> 1);
			break;
		case MSX_KONAMI4:
			bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
			bnk = bnk ? slot->memMap[bnk] : 0;
			radr = (bnk << 13) | (adr & 0x1fff);
			break;
		case MSX_ASCII8:
		case MSX_KONAMI5:
			bnk = ((adr & 0x2000) >> 13) | ((adr & 0x8000) >> 14);
			bnk = slot->memMap[bnk];
			radr = (bnk << 13) | (adr & 0x1fff);
			break;
		case MSX_ASCII16:
			bnk = slot->memMap[(adr & 0x8000) >> 15];
			radr = (bnk << 14) | (adr & 0x3fff);
			break;
	}
	return slot->data[radr & slot->memMask];
}

void msxSlotWr(unsigned short adr, unsigned char val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	switch (slot->mapAuto) {
		case MSX_KONAMI4:
			switch(adr) {
				case 0x6000: slot->memMap[1] = val; break;
				case 0x8000: slot->memMap[2] = val; break;
				case 0xa000: slot->memMap[3] = val; break;
			}
			break;
		case MSX_KONAMI5:
			switch (adr & 0xf800) {
				case 0x5000: slot->memMap[0] = val; break;
				case 0x7000: slot->memMap[1] = val; break;
				case 0x9000: slot->memMap[2] = val; break;		// TODO: SCC
				case 0xb000: slot->memMap[3] = val; break;
			}
			break;
		case MSX_ASCII8:
			switch (adr & 0xf800) {
				case 0x6000: slot->memMap[0] = val; break;
				case 0x6800: slot->memMap[1] = val; break;
				case 0x7000: slot->memMap[2] = val; break;
				case 0x7800: slot->memMap[3] = val; break;
			}
			break;
		case MSX_ASCII16:
			switch (adr & 0xf800) {
				case 0x6000: slot->memMap[0] = val; break;	// #4000..#7FFF
				case 0x7000: slot->memMap[1] = val; break;	// #8000..#bfff
			}
			break;
	}
}

mPageNr msxMemTab[4][4] = {
	{{MEM_ROM, 0}, {MEM_ROM, 1}, {MEM_RAM, 1}, {MEM_RAM, 0}},
	{{MEM_SLOT,0},{MEM_SLOT,0},{MEM_SLOT,0},{MEM_SLOT,0}},
	{{MEM_SLOT,1},{MEM_SLOT,1},{MEM_SLOT,1},{MEM_SLOT,1}},
	{{MEM_RAM, 3},{MEM_RAM, 2},{MEM_RAM, 1},{MEM_RAM, 0}}
};

// unsigned char emptyPage[0x4000];

void msxSetMem(Computer* comp, int bank, unsigned char slot) {
	mPageNr pg = msxMemTab[slot][bank];
	switch(pg.type) {
		case MEM_SLOT:
			memSetBank(comp->mem, bank, MEM_SLOT, comp->slot->memMap[bank], msxSlotRd, msxSlotWr, &comp->slot);
			break;
		case MEM_RAM:
			memSetBank(comp->mem, bank, MEM_RAM, comp->msx.memMap[bank & 3] & 7, NULL, NULL, NULL);
			break;
		case MEM_ROM:
			memSetBank(comp->mem, bank, MEM_ROM, pg.num, NULL, NULL, NULL);
			break;
	}
}

void msxMapMem(Computer* comp) {
	msxSetMem(comp, 0, comp->msx.pA8 & 0x03);
	msxSetMem(comp, 1, (comp->msx.pA8 & 0x0c) >> 2);
	msxSetMem(comp, 2, (comp->msx.pA8 & 0x30) >> 4);
	msxSetMem(comp, 3, (comp->msx.pA8 & 0xc0) >> 6);
}

// colors was taken from wikipedia article
// https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_palettes#Original_MSX

void msxResetSlot(xCartridge* slot) {
	slot->memMap[0] = 0;
	slot->memMap[1] = 0;
	slot->memMap[2] = 0;
	slot->memMap[3] = 0;
}

void msxReset(Computer* comp) {
	comp->vid->v9938.memMask = 0x3fff;
	comp->vid->v9938.high = 0;
	comp->vid->v9938.lines = 192;
	comp->msx.pA8 = 0x00;
	comp->msx.memMap[0] = 3;
	comp->msx.memMap[1] = 2;
	comp->msx.memMap[2] = 1;
	comp->msx.memMap[3] = 0;
	vidSetMode(comp->vid, VID_V9938);
	msxResetSlot(comp->slot);
	vdpReset(&comp->vid->v9938);
	msxMapMem(comp);
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
	comp->beep->lev = (val & 0x80) ? 1 : 0;
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
	msxMapMem(comp);
}

unsigned char msxA8In(Computer* comp, unsigned short port) {
	return comp->msx.pA8;
}

void msxMemOut(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.memMap[port & 3] = val;
}

unsigned char msxMemIn(Computer* comp, unsigned short port) {
	return comp->msx.memMap[port & 3];
}

// v9938

void msx98Out(Computer* comp, unsigned short port, unsigned char val) {
	vdpMemWr(&comp->vid->v9938, val);
}

void msx99Out(Computer* comp, unsigned short port, unsigned char val) {
	vdpRegWr(&comp->vid->v9938, val);
}

unsigned char msx98In(Computer* comp, unsigned short port) {
	unsigned char res = comp->vid->v9938.ram[comp->vid->v9938.vadr & comp->vid->v9938.memMask];
	comp->vid->v9938.vadr++;
	return res;
}

unsigned char msx99In(Computer* comp, unsigned short port) {		// status register 0
	return vdpReadSR(&comp->vid->v9938);
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

	{0xfc,0xfc,2,2,2,msxMemIn,	msxMemOut},	// FC..FF RW	RAM pages for memBanks

	{0x00,0x00,2,2,2,dummyIn,dummyOut},
//	{0x00,0x00,2,2,2,brkIn,brkOut}
};

unsigned char msxIn(Computer* comp, unsigned short port, int dos) {
	return hwIn(msxPortMap, comp, port, dos);
}

void msxOut(Computer* comp, unsigned short port, unsigned char val, int dos) {
	hwOut(msxPortMap,comp, port, val, dos);
}

// int zxINT(Computer*, unsigned char);
void msx_sync(Computer* comp, long ns) {
	if ((comp->vid->v9938.reg[1] & 0x40) && comp->vid->newFrame && comp->cpu->iff1) {
//		comp->cpu->inth = 1;
		comp->cpu->intrq |= 1;
		comp->intVector = 0xff;
	}
}
