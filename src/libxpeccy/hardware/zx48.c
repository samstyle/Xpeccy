#include "../spectrum.h"

void speReset(Computer* comp) {
	zx_set_pal(comp);
	vidSetMode(comp->vid, VID_NORMAL);
}

void speMapMem(Computer* comp) {
	memSetBank(comp->mem,0x00,MEM_ROM,(comp->dos) ? 1 : 0, MEM_16K,NULL,NULL,NULL);
	memSetBank(comp->mem,0x40,MEM_RAM,5,MEM_16K,NULL,NULL,NULL);		// 101 / x01
	memSetBank(comp->mem,0x80,MEM_RAM,2,MEM_16K,NULL,NULL,NULL);		// 010 / x10
	memSetBank(comp->mem,0xc0,MEM_RAM,0,MEM_16K,NULL,NULL,NULL);		// 000 / x00
}

// out

/*
void spOutFE(Computer* comp, unsigned short port, unsigned char val) {
	comp->vid->nextbrd = val & 0x07;
	comp->beep->lev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}
*/

// in

unsigned char spIn1F(Computer* comp, unsigned short port) {
	return joyInput(comp->joy);
}

unsigned char spInFF(Computer* comp, unsigned short port) {
	return comp->vid->atrbyte;
}

static xPort spePortMap[] = {
	{0x0001,0x00fe,2,2,2,xInFE,	xOutFE},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0x0320,0xfadf,2,2,2,xInFADF,	NULL},
	{0x0720,0xfbdf,2,2,2,xInFBDF,	NULL},
	{0x0720,0xffdf,2,2,2,xInFFDF,	NULL},
	{0x0021,0x001f,0,2,2,spIn1F,	NULL},
	{0x0000,0x0000,0,2,2,spInFF,	NULL},		// all unknown ports is FF (nodos)
	{0x0000,0x0000,2,2,2,spInFF,	NULL}
};

void speOut(Computer* comp, unsigned short port, unsigned char val, int dos) {
	difOut(comp->dif, port, val, dos);
	zx_dev_wr(comp, port, val, dos);
	hwOut(spePortMap, comp, port, val, dos);
}

unsigned char speIn(Computer* comp, unsigned short port, int dos) {
	unsigned char res;
	if (difIn(comp->dif, port, &res, dos)) return res;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	return hwIn(spePortMap, comp, port, dos);
}
