#include "hardware.h"
#undef NDEBUG
#include <assert.h>

int msx2mtabA[8][4] = {
	{MEM_ROM, MEM_ROM, MEM_RAM, MEM_RAM},
	{MEM_EXT, MEM_EXT, MEM_EXT, MEM_EXT},
	{MEM_EXT, MEM_EXT, MEM_EXT, MEM_EXT},
	{0,0,0,0},				// not used
	{MEM_ROM, MEM_ROM, MEM_ROM, MEM_ROM},
	{MEM_ROM, MEM_ROM, MEM_ROM, MEM_ROM},
	{MEM_RAM, MEM_RAM, MEM_RAM, MEM_RAM},
	{MEM_ROM, MEM_ROM, MEM_ROM, MEM_ROM}
};

unsigned char msx2mtabB[8][4] = {
	{0,1,1,0},
	{0,0,0,0},
	{1,1,1,1},
	{0,0,0,0},	// not used
	{2,2,2,2},
	{3,3,3,3},
	{0xfc,0xfd,0xfe,0xff},
	{4,4,4,4}
};

void msxSlotWr(unsigned short, unsigned char, void*);
unsigned char msxSlotRd(unsigned short, void*);

void msx2mapPage(Computer* comp, int bank, int pslot) {
	if (pslot == 3) {						// pslot:3
		switch (bank & 3) {
			case 0: pslot = comp->msx.mFFFF & 3; break;
			case 1: pslot = (comp->msx.mFFFF & 0x0c) >> 2; break;
			case 2: pslot = (comp->msx.mFFFF & 0x30) >> 4; break;
			case 3: pslot = (comp->msx.mFFFF & 0xc0) >> 6; break;
		}
		pslot += 4;
	}
	int bt = msx2mtabA[pslot][bank];			// page type (ram/rom/ext)
	unsigned char bn = msx2mtabB[pslot][bank];		// page num
	if (bn >= 0xfc) bn = comp->msx.memMap[bn & 3];		// ports fc..ff
	if (bt == MEM_EXT) {
		memSetExternal(comp->mem, bank, msxSlotRd, msxSlotWr, bn ? &comp->msx.slotB : &comp->msx.slotA);
	} else {
		memSetBank(comp->mem, bank, bt, bn);
	}
}

void msx2mapper(Computer* comp) {
//	printf("msx2 map pages: PS:%.2X SS:%.2X\n",comp->msx.pA8,comp->msx.mFFFF);
	msx2mapPage(comp, 0, comp->msx.pA8 & 0x03);
	msx2mapPage(comp, 1, (comp->msx.pA8 & 0x0c) >> 2);
	msx2mapPage(comp, 2, (comp->msx.pA8 & 0x30) >> 4);
	msx2mapPage(comp, 3, (comp->msx.pA8 & 0xc0) >> 6);
}

void msxResetSlot(xCartridge*);

void msx2Reset(Computer* comp) {
	comp->vid->v9938.memMask = 0x1ffff;
	comp->msx.pA8 = 0xf0;
	comp->msx.mFFFF = 0x00;
	vdpReset(&comp->vid->v9938);
	msxResetSlot(&comp->msx.slotA);
	msxResetSlot(&comp->msx.slotB);
	msx2mapper(comp);
}

unsigned char msx2mrd(Computer* comp, unsigned short adr, int m1) {
	unsigned char res = 0xff;
	if ((adr == 0xffff) && ((comp->msx.pA8 & 0xc0) == 0xc0)) {
		res = comp->msx.mFFFF ^ 0xff;
	} else {
		res = stdMRd(comp, adr, m1);
	}
	return res;
}

void msx2mwr(Computer* comp, unsigned short adr, unsigned char val) {
	if ((adr == 0xffff) && ((comp->msx.pA8 & 0xc0) == 0xc0)) {
		comp->msx.mFFFF = val;
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
	msx2mapper(comp);
}

void msx2mapOut(Computer* comp, unsigned short port, unsigned char val) {
	comp->msx.memMap[port & 3] = val;
	msx2mapper(comp);
}

unsigned char msx2mapIn(Computer* comp, unsigned short port) {
	return comp->msx.memMap[port & 3];
}

// vdp

void msx2_9AOut(Computer* comp, unsigned short port, unsigned char val) {
	vdpPalWr(comp->vid, val);
}

// tab

void msx98Out(Computer*,unsigned short,unsigned char);
unsigned char msx98In(Computer*, unsigned short);
void msx99Out(Computer*,unsigned short,unsigned char);
unsigned char msx99In(Computer*, unsigned short);

unsigned char msxA9In(Computer*, unsigned short);
unsigned char msxAAIn(Computer*, unsigned short);
void msxAAOut(Computer*,unsigned short,unsigned char);
void msxABOut(Computer*,unsigned short,unsigned char);

void msxAYIdxOut(Computer*,unsigned short,unsigned char);
void msxAYDataOut(Computer*,unsigned short,unsigned char);
unsigned char msxAYDataIn(Computer*, unsigned short);

xPort msx2ioTab[] = {
	{0xff,0x90,2,2,2,dummyIn,	dummyOut},		// printer
	{0xff,0x91,2,2,2,NULL,		dummyOut},

	{0xff,0x98,2,2,2,msx98In,	msx98Out},
	{0xff,0x99,2,2,2,msx99In,	msx99Out},
	{0xff,0x9a,2,2,2,NULL,		msx2_9AOut},

	{0xff,0xa0,2,2,2,NULL,		msxAYIdxOut},		// PSG
	{0xff,0xa1,2,2,2,NULL,		msxAYDataOut},
	{0xff,0xa2,2,2,2,msxAYDataIn,	NULL},

	{0xff,0xa8,2,2,2,msx2_A8in,	msx2_A8out},
	{0xff,0xa9,2,2,2,msxA9In,	NULL},
	{0xff,0xaa,2,2,2,msxAAIn,	msxAAOut},
	{0xff,0xab,2,2,2,NULL,		msxABOut},

	{0xfc,0xb8,2,2,2,dummyIn,	dummyOut},		// b8..bb : light pen
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
