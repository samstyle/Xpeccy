#include "../spectrum.h"
#include <assert.h>

// Profi ROM: 128,48,EXT,DOS
void prfMapMem(ZXComp* comp) {
	if (comp->pDFFD & 0x10) {
		memSetBank(comp->mem, MEM_BANK0, MEM_RAM, 0);
	} else {
		memSetBank(comp->mem, MEM_BANK0, MEM_ROM, (comp->dos ? 2 : 0) | (comp->rom ? 1 : 0));
	}
	int bank = ((comp->pDFFD & 7) << 3) | (comp->p7FFD & 7);
	memSetBank(comp->mem, MEM_BANK1, MEM_RAM, (comp->pDFFD & 0x08) ? bank : 5);
	memSetBank(comp->mem, MEM_BANK2, MEM_RAM, (comp->pDFFD & 0x40) ? 6 : 2);
	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, (comp->pDFFD & 0x08) ? 7 : bank);
}

// out

void prfOutPal(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if ((comp->pDFFD & 0x80) && (comp->p7FFD & 0x20)) {
		// write palete
	}
}

void prfOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	xOutFE(comp, port, val);
	if (comp->pDFFD & 0x80) {
		comp->vid->nextbrd ^= 7;
		comp->vid->brdcol = comp->vid->nextbrd;
	}
}

void prfOutAS(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->cmos.adr = val;
}

void prfOutDS(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	cmsWr(comp, val);
}

void prfOutBDI(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	difOut(comp->dif, port, val, 1);
}

void prfOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if ((~comp->pDFFD & 0x10) && (comp->p7FFD & 0x20)) return;	// 7FFD is blocked
	comp->p7FFD = val;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	prfMapMem(comp);
//	printf("OUT 7FFD,%.2X\n",val);
}

void prfOutDFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->pDFFD = val;
	comp->cpm = (val & 0x20) ? 1 : 0;
	vidSetMode(comp->vid, (val & 0x80) ? VID_PRF_MC : VID_NORMAL);
	prfMapMem(comp);
//	printf("OUT DFFD,%.2X\n",val);
}

// in

Z80EX_BYTE prfInBDI(ZXComp* comp, Z80EX_WORD port) {
	Z80EX_BYTE res = 0xff;
	difIn(comp->dif, port, &res, 1);
	return res;
}

Z80EX_BYTE prfInDS(ZXComp* comp, Z80EX_WORD port) {
	return cmsRd(comp);
}

// debug

Z80EX_BYTE prfBrkIn(ZXComp* comp, Z80EX_WORD port) {
	printf("CPM:%i, ROM:%i, DOS:%i\n", comp->cpm, comp->rom, comp->dos);
	return brkIn(comp, port);
}

void prfBrkOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	printf("CPM:%i, ROM:%i, DOS:%i\n", comp->cpm, comp->rom, comp->dos);
	brkOut(comp, port, val);
}

xPort prfPortMap[] = {
	// common
	{0x00f7,0x00fe,2,2,2,xInFE,	prfOutFE},
	{0x8002,0x7ffd,2,2,2,NULL,	prfOut7FFD},
	{0x2002,0xdffd,2,2,2,NULL,	prfOutDFFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x00ff,0x00f7,2,2,2,dummyIn,	dummyOut},	// f7 (off?)

	// * * CPM
	{0x00ff,0x007e,2,2,1,NULL,	prfOutPal},	// 7e cpm:palete (?)

	// BAS !ROM CPM
	{0x00ff,0x00df,0,1,1,prfInDS,	prfOutDS},	// cmos DATA
	{0x00ff,0x00ff,0,1,1,NULL,	prfOutAS},	// cmos ADR
	{0x009f,0x008f,0,1,1,dummyIn,	dummyOut},	// 8f,af,cf,ef (BB51/3)

	// BAS !ROM CPM
	{0x00ff,0x00bf,0,0,1,prfInBDI,	prfOutBDI},	// bf (dos: bdi ff)
	{0x009f,0x001f,0,0,1,prfInBDI,	prfOutBDI},	// 1f,3f,5f,7f (dos)

	// BAS !ROM !CPM
	{0x009f,0x001f,0,0,0,dummyIn,	dummyOut},	// 1f,3f,5f,7f (BB55)

	// BAS * !CPM
	{0x00ff,0x00ff,0,2,0,dummyIn,	NULL},		// in FF
	{0x00ff,0x001f,0,2,0,xIn1F,	NULL},		// 1f (K-joy) : !dos !cpm
	{0xffff,0xfadf,0,2,0,xInFADF,	NULL},
	{0xffff,0xfbdf,0,2,0,xInFBDF,	NULL},
	{0xffff,0xffdf,0,2,0,xInFFDF,	NULL},

	{0x0000,0x0000,2,2,2,prfBrkIn,	prfBrkOut}
};

void prfOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	if (difOut(comp->dif, port, val, dos)) return;
	hwOut(prfPortMap, comp, port, val, dos);
}

Z80EX_BYTE prfIn(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(prfPortMap, comp, port, dos);
	return res;
}

void prfReset(ZXComp* comp) {
}
