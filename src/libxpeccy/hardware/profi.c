#include "../spectrum.h"

// Profi ROM: EXT,DOS,128,48
void prfMapMem(Computer* comp) {
	if (comp->pDFFD & 0x10) {
		memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_16K, NULL, NULL, NULL);
	} else {
		memSetBank(comp->mem, 0x00, MEM_ROM, (comp->dos ? 0 : 2) | (comp->rom ? 1 : 0), MEM_16K, NULL, NULL, NULL);
	}
	int bank = ((comp->pDFFD & 7) << 3) | (comp->p7FFD & 7);
	memSetBank(comp->mem, 0x40, MEM_RAM, (comp->pDFFD & 0x08) ? bank : 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_RAM, ((comp->pDFFD & 0x40) && (comp->p7FFD & 8)) ? 6 : 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xc0, MEM_RAM, (comp->pDFFD & 0x08) ? 7 : bank, MEM_16K, NULL, NULL, NULL);
}

// out

// INFO: out (xx7E),nn
// xx - inverted GGGRRRBB
// nn - inverted color index for next out (0..15)

const unsigned char prfColB[4] = {0,80,160,255};
const unsigned char prfColTab[8] = {0,40,80,120,160,200,228,255};

void prfOut7E(Computer* comp, unsigned short port, unsigned char val) {
	if (comp->pDFFD & 0x80) {
		xColor col;
		port ^= 0xff00;
		col.b = prfColTab[(port & 0x0300) >> 7];
		col.r = prfColTab[(port & 0x1c00) >> 10];
		col.g = prfColTab[(port & 0xe000) >> 13];
		comp->vid->pal[comp->profi.p7E & 15] = col;
		comp->profi.p7E = ~val & 15;
	}
}

void prfOutFE(Computer* comp, unsigned short port, unsigned char val) {
	xOutFE(comp, port, val);
	if (comp->pDFFD & 0x80) {
		comp->vid->nextbrd ^= 7;
		comp->vid->brdcol = comp->vid->nextbrd;
	}
}

void prfOutAS(Computer* comp, unsigned short port, unsigned char val) {
	comp->cmos.adr = val;
}

void prfOutDS(Computer* comp, unsigned short port, unsigned char val) {
	cmsWr(comp, val);
}

void prfOutBDI(Computer* comp, unsigned short port, unsigned char val) {
	difOut(comp->dif, (port & 0x60) | 0x1f, val, 1);
}

void prfOutBDIFF(Computer* comp, unsigned short port, unsigned char val) {
	difOut(comp->dif, 0xff, val, 1);
}

void prfOut7FFD(Computer* comp, unsigned short port, unsigned char val) {
	if ((~comp->pDFFD & 0x10) && (comp->p7FFD & 0x20)) return;	// 7FFD is blocked
	comp->p7FFD = val;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	prfMapMem(comp);
//	printf("OUT 7FFD,%.2X\n",val);
}

void prfOutDFFD(Computer* comp, unsigned short port, unsigned char val) {
	comp->pDFFD = val;
	comp->cpm = (val & 0x20) ? 1 : 0;
	vidSetMode(comp->vid, (val & 0x80) ? VID_PRF_MC : VID_NORMAL);
	prfMapMem(comp);
//	printf("OUT DFFD,%.2X\n",val);
}

// in

unsigned char prfInFE(Computer* comp, unsigned short port) {
	unsigned char res = kbdRead(comp->keyb, port);
	res |= ((comp->tape->volPlay & 0x80) ? 0x40 : 0x00);
	return res;
}

unsigned char prfInBDI(Computer* comp, unsigned short port) {
	unsigned char res = 0xff;
	difIn(comp->dif, (port & 0x60) | 0x1f, &res, 1);
	return res;
}

unsigned char prfInBDIFF(Computer* comp, unsigned short port) {
	unsigned char res = 0xff;
	difIn(comp->dif, 0xff, &res, 1);
	return res;
}

unsigned char prfInDS(Computer* comp, unsigned short port) {
	return cmsRd(comp);
}

// debug

unsigned char prfBrkIn(Computer* comp, unsigned short port) {
	printf("CPM:%i, ROM:%i, DOS:%i\n", comp->cpm, comp->rom, comp->dos);
	return brkIn(comp, port);
}

void prfBrkOut(Computer* comp, unsigned short port, unsigned char val) {
	printf("CPM:%i, ROM:%i, DOS:%i\n", comp->cpm, comp->rom, comp->dos);
	brkOut(comp, port, val);
}

static xPort prfPortMap[] = {
	// common
	{0x00f7,0x00fe,2,2,2,prfInFE,	prfOutFE},
	{0x8002,0x7ffd,2,2,2,NULL,	prfOut7FFD},
	{0x2002,0xdffd,2,2,2,NULL,	prfOutDFFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x00ff,0x00f7,2,2,2,dummyIn,	dummyOut},	// f7 (off?)

	// * * CPM
	{0x00ff,0x007e,2,2,1,NULL,	prfOut7E},	// 7e cpm:palete (?)

	// !DOS ROM CPM
	{0x009f,0x0083,0,1,1,prfInBDI,	prfOutBDI},
	{0x00ff,0x003f,0,1,1,prfInBDIFF,prfOutBDIFF},

	// !DOS !ROM CPM
	{0x00ff,0x00df,0,1,1,prfInDS,	prfOutDS},	// cmos DATA
	{0x00ff,0x00ff,0,1,1,NULL,	prfOutAS},	// cmos ADR
	{0x009f,0x008f,0,1,1,dummyIn,	dummyOut},	// 8f,af,cf,ef (BB51/3)

	// !DOS !ROM CPM
	{0x009f,0x001f,0,0,1,prfInBDI,	prfOutBDI},	// 1f,3f,5f,7f (fdc)
	{0x00ff,0x00bf,0,0,1,prfInBDIFF,prfOutBDIFF},	// bf (bdi ff)

	// !DOS !ROM !CPM
	{0x009f,0x001f,0,0,0,dummyIn,	dummyOut},	// 1f,3f,5f,7f (BB55)

	// !DOS * !CPM
	{0x00ff,0x00ff,0,2,0,dummyIn,	NULL},		// in FF
	{0x00ff,0x001f,0,2,0,xIn1F,	NULL},		// 1f (K-joy) : !dos !cpm
	{0xffff,0xfadf,0,2,0,xInFADF,	NULL},
	{0xffff,0xfbdf,0,2,0,xInFBDF,	NULL},
	{0xffff,0xffdf,0,2,0,xInFFDF,	NULL},

	// DOS * !CPM
	{0x009f,0x001f,1,2,0,prfInBDI,	prfOutBDI},	// BDI 1f,3f,5f,7d
	{0x00ff,0x00ff,1,2,0,prfInBDIFF,prfOutBDIFF},	// BDI ff

#ifdef ISDEBUG
	{0x0000,0x0000,2,2,2,prfBrkIn,	prfBrkOut}
#else
	{0x0000,0x0000,2,2,2,dummyIn,	dummyOut}
#endif

};

void prfOut(Computer* comp, unsigned short port, unsigned char val, int dos) {
	zx_dev_wr(comp, port, val, dos);
	hwOut(prfPortMap, comp, port, val, dos);
}

unsigned char prfIn(Computer* comp, unsigned short port, int dos) {
	unsigned char res = 0xff;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	res = hwIn(prfPortMap, comp, port, dos);
	return res;
}

void prfReset(Computer* comp) {
	kbdSetMode(comp->keyb, KBD_PROFI);
}

void prf_keyp(Computer* comp, keyEntry ent) {
	kbdPress(comp->keyb, ent);
//	keyPressXT(comp->keyb, ent.keyCode);
//	keyPress(comp->keyb, ent.zxKey, 0);
//	keyPress(comp->keyb, ent.extKey, 1);
}

void prf_keyr(Computer* comp, keyEntry ent) {
	kbdRelease(comp->keyb, ent);
//	keyReleaseXT(comp->keyb, ent.keyCode);
//	keyRelease(comp->keyb, ent.zxKey, 0);
//	keyRelease(comp->keyb, ent.extKey, 1);
}
