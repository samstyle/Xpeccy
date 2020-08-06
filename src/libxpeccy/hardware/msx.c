#include "hardware.h"

// MSX...
// master clock		MSX2:21.48MHz | MSX1:10.74
// v99xx clock		master/4 = 5.37MHz : 2 dots/period	MSX2. MSX1: master/2
// CPU clock		master/6 = 3.58MHz : 1T = 3 dots	MSX2. MSX1: master/3

typedef struct {
	int type;
	int num;
} mPageNr;

static mPageNr msxMemTab[4][4] = {
	{{MEM_ROM, 0}, {MEM_ROM, 1}, {MEM_RAM, 1}, {MEM_RAM, 0}},
	{{MEM_SLOT,0},{MEM_SLOT,0},{MEM_SLOT,0},{MEM_SLOT,0}},
	{{MEM_SLOT,1},{MEM_SLOT,1},{MEM_SLOT,1},{MEM_SLOT,1}},
	{{MEM_RAM, 3},{MEM_RAM, 2},{MEM_RAM, 1},{MEM_RAM, 0}}
};

int msxSlotRd(int adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, SLT_PRG, adr);
}

void msxSlotWr(int adr, int val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, SLT_PRG, adr, val);
}

void msxSetMem(Computer* comp, int bank, int slot) {
	slot &= 3;
	bank &= 3;
	int type = msxMemTab[slot][bank].type;
	int num = msxMemTab[slot][bank].num;
	switch(type) {
		case MEM_SLOT:
			memSetBank(comp->mem, bank << 6, MEM_SLOT, comp->slot->memMap[bank], MEM_16K, msxSlotRd, msxSlotWr, comp->slot);
			break;
		case MEM_RAM:
			memSetBank(comp->mem, bank << 6, MEM_RAM, comp->reg[0xfc | bank] & 7, MEM_16K, NULL, NULL, NULL);
			break;
		case MEM_ROM:
			memSetBank(comp->mem, bank << 6, MEM_ROM, num, MEM_16K, NULL, NULL, NULL);
			break;
	}
}

void msxMapMem(Computer* comp) {
	msxSetMem(comp, 0, comp->ppi->a.val & 0x03);
	msxSetMem(comp, 1, (comp->ppi->a.val & 0x0c) >> 2);
	msxSetMem(comp, 2, (comp->ppi->a.val & 0x30) >> 4);
	msxSetMem(comp, 3, (comp->ppi->a.val & 0xc0) >> 6);
}

void msxResetSlot(xCartridge* slot) {
	slot->memMap[0] = 0;
	slot->memMap[1] = 0;
	slot->memMap[2] = 0;
	slot->memMap[3] = 0;
}

void msxReset(Computer* comp) {
	kbdSetMode(comp->keyb, KBD_MSX);
	ppi_reset(comp->ppi);
	// comp->msx.pA8 = 0x00;
	comp->reg[0xfc] = 3;
	comp->reg[0xfd] = 2;
	comp->reg[0xfe] = 1;
	comp->reg[0xff] = 0;
	msxResetSlot(comp->slot);
	vdpReset(comp->vid);
	comp->vid->memMask = MEM_16K - 1;
	msxMapMem(comp);
}

// AY

void msxAYIdxOut(Computer* comp, int port, int val) {
	tsOut(comp->ts, 0xfffd, val & 0xff);
}

void msxAYDataOut(Computer* comp, int port, int val) {
	tsOut(comp->ts, 0xbffd, val & 0xff);
}

int msxAYDataIn(Computer* comp, int port) {
	int res = 0xff;
	if (comp->ts->curChip->curReg == 0x0e) {		// b7:tape in, b6:?, b0..5 = joystick u/d/l/r/fa/fb (0:active)
		res = 0x7f | (comp->tape->volPlay & 0x80);
	} else {
		res = tsIn(comp->ts, 0xfffd);
	}
	return res;
}

// 8255A

// write ppi chan A (memory control)
void msx_ppi_a_wr(int val, void* p) {
	Computer* comp = (Computer*)p;
	msxMapMem(comp);
}

// read ppi chan B (keyboard scan)
int msx_ppi_b_rd(void* p) {
	Computer* comp = (Computer*)p;
	return comp->keyb->msxMap[comp->msx.keyLine];
}

// write ppi chan C (select keyboard row, tape control, tape out, beeper)
void msx_ppi_cl_wr(int val, void* p) {
	Computer* comp = (Computer*)p;
	comp->msx.keyLine = val & 0x0f;
}


void msx_ppi_ch_wr(int val, void* p) {
	Computer* comp = (Computer*)p;
	if (val & 0x10) {
		tapStop(comp->tape);
	} else {
		tapPlay(comp->tape);
	}
	comp->tape->levRec = (val & 0x20) ? 1 : 0;
	comp->beep->lev = (val & 0x80) ? 1 : 0;
}

int msx_ppi_rd(Computer* comp, int port) {
	return ppi_rd(comp->ppi, port & 3);
}

void msx_ppi_wr(Computer* comp, int port, int val) {
	ppi_wr(comp->ppi, port & 3, val);
}

// memory

void msxMemOut(Computer* comp, int port, int val) {
	comp->reg[port] = val & 0xff;
}

// v9938

void msx9938wr(Computer* comp, int adr, int val) {
	vdpWrite(comp->vid, adr & 3, val & 0xff);
}

int msx9938rd(Computer* comp, int adr) {
	return vdpRead(comp->vid, adr & 3);
}

void msx_init(Computer* comp) {
	comp->fps = 60;
	vidUpdateTimings(comp->vid, comp->nsPerTick * 2 / 3);
	ppi_set_cb(comp->ppi, comp, NULL, msx_ppi_a_wr,\
				msx_ppi_b_rd, NULL,\
				NULL, msx_ppi_ch_wr, \
				NULL, msx_ppi_cl_wr);
}

// Port map

static xPort msxPortMap[] = {

	{0xff,0x90,2,2,2,dummyIn,	dummyOut},	// 90	RW	ULA5RA087 Centronic BUSY state (bit 1=1) / ULA5RA087 Centronic STROBE output (bit 0=0)
	{0xff,0x91,2,2,2,NULL,		dummyOut},	// 91	W	ULA5RA087 Centronic Printer Data

	{0xfc,0x88,2,2,2,msx9938rd,	msx9938wr},	// 88..8D	VDP9938 extension
	{0xfe,0x98,2,2,2,msx9938rd,	msx9938wr},	// 98/99	VDP9918

	{0xff,0xa0,2,2,2,NULL,		msxAYIdxOut},	// A0	W	I AY-3-8910 PSG Sound Generator Index
	{0xff,0xa1,2,2,2,NULL,		msxAYDataOut},	// A1	W	I AY-3-8910 PSG Sound Generator Data write
	{0xff,0xa2,2,2,2,msxAYDataIn,	NULL},		// A2	R	I AY-3-8910 PSG Sound Generator Data read

	{0xfc,0xa8,2,2,2,msx_ppi_rd,	msx_ppi_wr},

	{0xfc,0xfc,2,2,2,NULL,	msxMemOut},		// FC..FF W	RAM pages for memBanks

	{0x00,0x00,2,2,2,dummyIn,dummyOut},
};

int msxIn(Computer* comp, int port, int dos) {
	return hwIn(msxPortMap, comp, port, dos);
}

void msxOut(Computer* comp, int port, int val, int dos) {
	hwOut(msxPortMap,comp, port, val, dos);
}

void msx_sync(Computer* comp, int ns) {
	int irq = (comp->vid->inth || comp->vid->intf) ? 1 : 0;
	if (irq && !(comp->cpu->intrq & Z80_INT)) {			// 0->1 : TESTED 20ms
		comp->cpu->intrq |= Z80_INT;
		comp->intVector = 0xff;
	} else if (!irq && (comp->cpu->intrq & Z80_INT)) {		// 1->0 : clear
		comp->cpu->intrq &= ~Z80_INT;
	}
	tsSync(comp->ts, ns);
	tapSync(comp->tape, ns);
}

void msx_keyp(Computer* comp, keyEntry ent) {
	kbdPress(comp->keyb, ent);
}

void msx_keyr(Computer* comp, keyEntry ent) {
	kbdRelease(comp->keyb, ent);
}

sndPair msx_vol(Computer* comp, sndVolume* sv) {
	int amp = 0;
	if (comp->tape->on)
		amp = (comp->tape->volPlay << 8) * sv->tape / 1600;
	sndPair vol;
	vol.left = amp;
	vol.right = amp;
	sndPair tv = aymGetVolume(comp->ts->chipA);
	vol.left += tv.left * sv->ay / 100;
	vol.right += tv.right * sv->ay / 100;
	return vol;
}
