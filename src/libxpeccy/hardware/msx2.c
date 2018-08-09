#include "hardware.h"

// primary slot

static int msx2mtabA[4][4] = {
	{MEM_ROM, MEM_ROM, MEM_RAM, MEM_RAM},
	{MEM_SLOT, MEM_SLOT, MEM_SLOT, MEM_SLOT},
	{MEM_SLOT, MEM_SLOT, MEM_SLOT, MEM_SLOT},
	{0,0,0,0}				// not used
};

static int msx2mtabB[4][4] = {
	{0,1,1,0},
	{0,0,0,0},
	{1,1,1,1},
	{0,0,0,0}	// not used
};

// secondary slot

static int msx2mtabC[4][4] = {
	{MEM_ROM, MEM_ROM, MEM_ROM, MEM_ROM},
	{MEM_ROM, MEM_ROM, MEM_ROM, MEM_ROM},
	{MEM_RAM, MEM_RAM, MEM_RAM, MEM_RAM},
	{MEM_ROM, MEM_ROM, MEM_ROM, MEM_ROM}
};

static int msx2mtabD[4][4] = {
	{2,3,2,3},
	{0,0,0,0},
	{-1,-1,-1,-1},
	{0,0,0,0}
};

void msxSlotWr(unsigned short, unsigned char, void*);
unsigned char msxSlotRd(unsigned short, void*);

void msx2mapper(Computer* comp) {
	int slot;
	int bt;
	int bn;
	for (int i = 0; i < 4; i++) {
		slot = comp->msx.pslot[i];
		if (slot < 3) {
			bt = msx2mtabA[slot][i];
			bn = msx2mtabB[slot][i];
		} else {
			slot = comp->msx.sslot[i];
			bt = msx2mtabC[slot][i];
			bn = msx2mtabD[slot][i];
		}
		if (bn < 0) {
			bn = comp->msx.memMap[i];
		}
		if (bt == MEM_SLOT) {
			memSetBank(comp->mem, i << 6, MEM_SLOT, comp->slot->memMap[i], MEM_16K, msxSlotRd, msxSlotWr, comp->slot);
		} else {
			memSetBank(comp->mem, i << 6, bt, bn, MEM_16K, NULL, NULL, NULL);
		}
	}
}

unsigned char msx2mrd(Computer* comp, unsigned short adr, int m1) {
	unsigned char res = 0xff;
	if (0 & ((adr & 0xfff8) == 0x7ff8)) {					// fdc io
		printf("rd %X\n",adr & 7);
		comp->brk = 1;
		switch(adr & 7) {
			case 0: difIn(comp->dif, FDC_COM, &res, 1); break;
			case 1: difIn(comp->dif, FDC_TRK, &res, 1); break;
			case 2: difIn(comp->dif, FDC_SEC, &res, 1); break;
			case 3: difIn(comp->dif, FDC_DATA, &res, 1); break;
			case 7: res = (comp->dif->fdc->drq ? 0x80 : 0) | (comp->dif->fdc->irq ? 0x40 : 0); break;
		}
	} else if ((adr == 0xffff) && (comp->msx.pslot[3] == 3)) {	// sslot
		res = comp->msx.mFFFF ^ 0xff;
	} else {
		res = stdMRd(comp, adr, m1);
	}
	return res;
}

void msx2mwr(Computer* comp, unsigned short adr, unsigned char val) {
	if (0 & ((adr & 0xfff8) == 0x7ff8)) {					// fdc io
	// printf("wr %X,%.2X\n",adr & 7, val);
		switch(adr & 7) {
			case 0: difOut(comp->dif, FDC_COM, val, 1); break;
			case 1: difOut(comp->dif, FDC_TRK, val, 1); break;
			case 2: difOut(comp->dif, FDC_SEC, val, 1); break;
			case 3: difOut(comp->dif, FDC_DATA, val, 1); break;
			case 4: comp->dif->fdc->side = (val & 1) ? 1 : 0; break;
			case 5: comp->dif->fdc->flp = comp->dif->fdc->flop[val & 1]; break;
		}
	} else if ((adr == 0xffff) && (comp->msx.pslot[3] == 3)) {	// sslot
		comp->msx.mFFFF = val;
		comp->msx.sslot[0] = val & 3;
		comp->msx.sslot[1] = (val >> 2) & 3;
		comp->msx.sslot[2] = (val >> 4) & 3;
		comp->msx.sslot[3] = (val >> 6) & 3;
		msx2mapper(comp);
	} else {
		stdMWr(comp, adr, val);
	}
}

// mapper

unsigned char msx2_A8in(Computer* comp, unsigned short port) {
	return comp->msx.pA8;
}

void msx2_A8out(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.pA8 = val;
	comp->msx.pslot[0] = val & 3;
	comp->msx.pslot[1] = (val >> 2) & 3;
	comp->msx.pslot[2] = (val >> 4) & 3;
	comp->msx.pslot[3] = (val >> 6) & 3;
//	printf("A8 out %.2X (%i,%i,%i,%i)\n",comp->msx.pA8, comp->msx.pslot[0], comp->msx.pslot[1], comp->msx.pslot[2], comp->msx.pslot[3]);
	msx2mapper(comp);
}

void msx2mapOut(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.memMap[port & 3] = val;
	msx2mapper(comp);
}

unsigned char msx2mapIn(Computer* comp, unsigned short port) {
	return comp->msx.memMap[port & 3];
}

// reset

void msxResetSlot(xCartridge*);
void msx2Reset(Computer* comp) {
	kbdSetMode(comp->keyb, KBD_MSX);
	comp->vid->memMask = 0x1ffff;
	vidSetMode(comp->vid, VDP_TEXT1);
	vdpReset(comp->vid);
	msxResetSlot(comp->slot);
	msx2_A8out(comp, 0xa8, 0xf0);
	msx2mapper(comp);
}

// devices

void msx2_F5out(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.pF5 = val;
}

// rtc

void msx2_b4out(Computer* comp, unsigned short port, unsigned char val) {
	comp->cmos.adr = val & 15;
}

void msx2_b5out(Computer* comp, unsigned short port, unsigned char val) {
	int block = comp->cmos.data[0x0d];
	int adr = comp->cmos.adr & 15;
	if (adr < 0x0d)
		adr |= (block << 4);
	comp->cmos.data[adr] = val & 15;
}

unsigned char msx2_b5in(Computer* comp, unsigned short port) {
	unsigned char res = 0x0f;
	int block;
	int adr = comp->cmos.adr & 15;
	switch(adr) {
		case 0x0d:
			res = comp->cmos.data[adr];
			break;
		case 0x0e:
		case 0x0f:
			res = 0x0f;
			break;
		default:
			block = comp->cmos.data[0x0d] & 3;
			adr = (block << 4) | (comp->cmos.adr & 15);
			if (block == 0) {
				switch(adr & 15) {
					case 0: res = comp->cmos.data[0]; break;
					case 1: res = (comp->cmos.data[0] >> 4); break;
					case 2: res = comp->cmos.data[2]; break;
					case 3: res = (comp->cmos.data[2] >> 4); break;
					case 4: res = comp->cmos.data[4]; break;
					case 5: res = (comp->cmos.data[4] >> 4); break;
					case 6: res = comp->cmos.data[6];
					case 7: res = comp->cmos.data[7]; break;
					case 8: res = (comp->cmos.data[7] >> 4); break;
					case 9: res = comp->cmos.data[8]; break;
					case 10: res = (comp->cmos.data[8] >> 4); break;
					case 11: res = comp->cmos.data[9]; break;		// 00 = 1980 !!!
					case 12: res = (comp->cmos.data[9] >> 4); break;
				}
			} else {
				res = comp->cmos.data[adr];	// 1:alarm, 2,3:user data
			}
			break;
	}
	return (res & 0x0f);
}


// tab

/*
void msx98Out(Computer*,unsigned short,unsigned char);
unsigned char msx98In(Computer*, unsigned short);
void msx99Out(Computer*,unsigned short,unsigned char);
unsigned char msx99In(Computer*, unsigned short);
*/

void msx9938wr(Computer*, unsigned short, unsigned char);
unsigned char msx9938rd(Computer*, unsigned short);

unsigned char msxA9In(Computer*, unsigned short);
unsigned char msxAAIn(Computer*, unsigned short);
void msxAAOut(Computer*,unsigned short,unsigned char);
void msxABOut(Computer*,unsigned short,unsigned char);

void msxAYIdxOut(Computer*,unsigned short,unsigned char);
void msxAYDataOut(Computer*,unsigned short,unsigned char);
unsigned char msxAYDataIn(Computer*, unsigned short);

static xPort msx2ioTab[] = {
//	{0xff,0x60,2,2,2,dummyIn,NULL},				// 60? really?

	{0xf8,0x80,2,2,2,dummyIn,	dummyOut},		// 80..87 : rs232

	{0xff,0x90,2,2,2,dummyIn,	dummyOut},		// printer
	{0xff,0x91,2,2,2,NULL,		dummyOut},

	{0xfc,0x98,2,2,2,msx9938rd,	msx9938wr},

	{0xff,0xa0,2,2,2,NULL,		msxAYIdxOut},		// PSG
	{0xff,0xa1,2,2,2,NULL,		msxAYDataOut},
	{0xff,0xa2,2,2,2,msxAYDataIn,	NULL},

	{0xff,0xa8,2,2,2,msx2_A8in,	msx2_A8out},
	{0xff,0xa9,2,2,2,msxA9In,	NULL},
	{0xff,0xaa,2,2,2,msxAAIn,	msxAAOut},
	{0xff,0xab,2,2,2,NULL,		msxABOut},

	{0xff,0xb4,2,2,2,NULL,		msx2_b4out},		// b4 : RTC : RP 5C01 (Not in 738) RTC Register select
	{0xff,0xb5,2,2,2,msx2_b5in,	msx2_b5out},		// b5 : RTC : RP 5C01 (Not in 738) RTC data
	{0xfc,0xb8,2,2,2,dummyIn,	dummyOut},		// b8..bb : light pen
	{0xfe,0xc0,2,2,2,dummyIn,	dummyOut},		// c0,c1 : MSX audio
	{0xf8,0xc8,2,2,2,dummyIn,	dummyOut},		// c8..cf : MSX interface
	{0xfc,0xd8,2,2,2,dummyIn,	dummyOut},		// TODO: d8..db : kanji rom
	{0xff,0xf5,2,2,2,NULL,		msx2_F5out},		// f5 : enable/disable internal devices
	{0xff,0xf7,2,2,2,dummyIn,	dummyOut},		// TODO: f7: a/v control
	{0xfc,0xf8,2,2,2,dummyIn,	dummyOut},		// f8..fb : "Reserved (But somehow accessed by MSX2 BIOS ???)"
	{0xfc,0xfc,2,2,2,msx2mapIn,	msx2mapOut},		// fe..ff : memory mapper
	{0x00,0x00,2,2,2,brkIn,brkOut}
};

void msx2Out(Computer* comp, unsigned short port, unsigned char val, int dos) {
//	printf("msx2 out %.4X,%.2X\n",port,val);
	hwOut(msx2ioTab, comp, port, val, dos);
}

unsigned char msx2In(Computer* comp, unsigned short port, int dos) {
//	printf("msx2 in %.4X\n",port);
	return hwIn(msx2ioTab, comp, port, dos);
}
