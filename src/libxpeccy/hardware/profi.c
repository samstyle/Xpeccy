#include "../spectrum.h"
#include <assert.h>

// Profi ROM: EXT,DOS,128,48
void prfMapMem(ZXComp* comp) {
	if (comp->pDFFD & 0x10) {
		memSetBank(comp->mem, MEM_BANK0, MEM_RAM, 0);
	} else {
		memSetBank(comp->mem, MEM_BANK0, MEM_ROM, (comp->dos ? 0 : 2) | (comp->rom ? 1 : 0));
	}
	int bank = ((comp->pDFFD & 7) << 3) | (comp->p7FFD & 7);
	memSetBank(comp->mem, MEM_BANK1, MEM_RAM, (comp->pDFFD & 0x08) ? bank : 5);
	memSetBank(comp->mem, MEM_BANK2, MEM_RAM, ((comp->pDFFD & 0x40) && (comp->p7FFD & 8)) ? 6 : 2);
	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, (comp->pDFFD & 0x08) ? 7 : bank);
}

// out

const unsigned char prfColB[4] = {0,80,160,255};
const unsigned char prfColTab[8] = {0,40,80,120,160,200,228,255};

void prfOutPal(ZXComp* comp, unsigned short port, unsigned char val) {
	if (comp->pDFFD & 0x80) {
		if (comp->profi.trig7E) {
			xColor col;
			int hi = ((port & 0xff00) >> 8);
			col.b = prfColB[hi & 3];
			col.r = prfColTab[(hi & 0x1c) >> 2];
			col.g = prfColTab[(hi & 0xe0) >> 5];
			comp->vid->pal[comp->profi.p7E] = col;
		} else {
			comp->profi.p7E = val & 15;
		}
		comp->profi.trig7E ^= 1;
	}
}

void prfOutFE(ZXComp* comp, unsigned short port, unsigned char val) {
	xOutFE(comp, port, val);
	if (comp->pDFFD & 0x80) {
		comp->vid->nextbrd ^= 7;
		comp->vid->brdcol = comp->vid->nextbrd;
	}
}

void prfOutAS(ZXComp* comp, unsigned short port, unsigned char val) {
	comp->cmos.adr = val;
}

void prfOutDS(ZXComp* comp, unsigned short port, unsigned char val) {
	cmsWr(comp, val);
}

void prfOutBDI(ZXComp* comp, unsigned short port, unsigned char val) {
	difOut(comp->dif, port, val, 1);
}

void prfOut7FFD(ZXComp* comp, unsigned short port, unsigned char val) {
	if ((~comp->pDFFD & 0x10) && (comp->p7FFD & 0x20)) return;	// 7FFD is blocked
	comp->p7FFD = val;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	prfMapMem(comp);
//	printf("OUT 7FFD,%.2X\n",val);
}

void prfOutDFFD(ZXComp* comp, unsigned short port, unsigned char val) {
	comp->pDFFD = val;
	comp->cpm = (val & 0x20) ? 1 : 0;
	vidSetMode(comp->vid, (val & 0x80) ? VID_PRF_MC : VID_NORMAL);
	prfMapMem(comp);
//	printf("OUT DFFD,%.2X\n",val);
}

// in

unsigned char prfInFE(ZXComp* comp, unsigned short port) {
	unsigned char res = keyInput(comp->keyb, (port & 0xff00) >> 8, 0);
	unsigned char ext = keyInput(comp->keyb, (port & 0xff00) >> 8, 1);
	if (ext != 0x3f) res = ext;
	res |= (comp->tape->levPlay ? 0x40 : 0x00);
	return res;
}

unsigned char prfInBDI(ZXComp* comp, unsigned short port) {
	unsigned char res = 0xff;
	difIn(comp->dif, port, &res, 1);
	return res;
}

unsigned char prfInDS(ZXComp* comp, unsigned short port) {
	return cmsRd(comp);
}

// debug

unsigned char prfBrkIn(ZXComp* comp, unsigned short port) {
	printf("CPM:%i, ROM:%i, DOS:%i\n", comp->cpm, comp->rom, comp->dos);
	return brkIn(comp, port);
}

void prfBrkOut(ZXComp* comp, unsigned short port, unsigned char val) {
	printf("CPM:%i, ROM:%i, DOS:%i\n", comp->cpm, comp->rom, comp->dos);
	brkOut(comp, port, val);
}

xPort prfPortMap[] = {
	// common
	{0x00f7,0x00fe,2,2,2,prfInFE,	prfOutFE},
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
#ifdef ISDEBUG
	{0x0000,0x0000,2,2,2,prfBrkIn,	prfBrkOut}
#else
	{0x0000,0x0000,2,2,2,dummyIn,	dummyOut}
#endif

};

void prfOut(ZXComp* comp, unsigned short port, unsigned char val, int dos) {
	if (difOut(comp->dif, port, val, dos)) return;
	hwOut(prfPortMap, comp, port, val, dos);
}

unsigned char prfIn(ZXComp* comp, unsigned short port, int dos) {
	unsigned char res = 0xff;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(prfPortMap, comp, port, dos);
	return res;
}

void prfReset(ZXComp* comp) {
	comp->profi.trig7E = 0;
}
