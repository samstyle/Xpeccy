#include "hardware.h"

void phxMapMem(Computer* comp) {
	memSetBank(comp->mem, 0x40, MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	int bank = (comp->p7FFD & 7) | ((comp->p1FFD & 0xd0) << 1) | ((comp->p7FFD & 0x80) << 3);
	memSetBank(comp->mem, 0xc0, MEM_RAM, bank, MEM_16K, NULL, NULL, NULL);
	if (comp->p1FFD & 1) {
		memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_16K, NULL, NULL, NULL);
	} else if (comp->p1FFD & 2) {
		memSetBank(comp->mem, 0x00, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);
	} else if (comp->p1FFD & 8) {
		memSetBank(comp->mem, 0x00, MEM_ROM, comp->dos ? 3 : (comp->rom ? 1 : 0), MEM_16K, NULL, NULL, NULL);
	} else {
		memSetBank(comp->mem, 0x00, MEM_ROM, comp->dos ? 1 : (comp->rom ? 3 : 2), MEM_16K, NULL, NULL, NULL);
	}
}

void phxReset(Computer* comp) {
	comp->p7FFD = 0;
	comp->p1FFD = 0;
	comp->pEFF7 = 0;
	phxMapMem(comp);
}

// out

void phxOut1FFD(Computer* comp, unsigned short port, unsigned char val) {
	comp->p1FFD = val;
	phxMapMem(comp);
}

void phxOut7FFD(Computer* comp, unsigned short port, unsigned char val) {
	if (comp->p7FFD & 0x20) return;
	comp->p7FFD = val;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	phxMapMem(comp);
}

void phxOutEFF7(Computer* comp, unsigned short port, unsigned char val) {
	comp->pEFF7 = val;
}

// in

unsigned char phxInF7(Computer* comp, unsigned short port) {
	return 0x00;
}

static xPort phxPortMap[] = {
	{0x0007,0x00fe,2,2,2,xInFE,	xOutFE},	// FE
	{0xc007,0x1ffd,2,2,2,NULL,	phxOut1FFD},	// mem control
	{0xc007,0x7ffd,2,2,2,NULL,	phxOut7FFD},
	{0xffff,0xeff7,2,2,2,NULL,	phxOutEFF7},
	{0xc007,0xbffd,2,2,2,NULL,	xOutBFFD},	// ay
	{0xc007,0xfffd,2,2,2,xInFFDF,	xOutFFFD},
	{0xffff,0xfadf,2,2,2,xInFADF,	NULL},		// k-mouse
	{0xffff,0xfbdf,2,2,2,xInFBDF,	NULL},
	{0xffff,0xffdf,2,2,2,xInFFDF,	NULL},

	{0xffff,0x00f7,2,2,2,phxInF7,	NULL},		// version
#ifdef ISDEBUG
	{0,0,2,2,2,brkIn,brkOut}
#else
	{0,0,2,2,2,dummyIn,dummyOut}
#endif
	// NEMO IDE set as external device
};

void phxOut(Computer* comp, unsigned short port, unsigned char val, int dos) {
	if (comp->pEFF7 & 0x80) dos = 1;
	if (difOut(comp->dif, port, val, dos)) return;
	zx_dev_wr(comp, port, val, dos);
	hwOut(phxPortMap, comp, port, val, dos);
}

unsigned char phxIn(Computer* comp, unsigned short port, int dos) {
	if (comp->pEFF7 & 0x80) dos = 1;
	unsigned char res = 0xff;
	if (difIn(comp->dif, port, &res, dos)) return res;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	return hwIn(phxPortMap, comp, port, dos);
}
