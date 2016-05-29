#include "../spectrum.h"

// layout for +2a extend memory mode

unsigned char plus2Lays[4][4] = {
	{0,1,2,3},
	{4,5,6,7},
	{4,5,6,3},
	{4,7,6,3}
};

void pl2MapMem(Computer* comp) {
	if (comp->p1FFD & 1) {
		// extend mem mode
		int rp = ((comp->p1FFD & 0x06) >> 1);	// b1,2 of 1ffd
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,plus2Lays[rp][0]);
		memSetBank(comp->mem,MEM_BANK1,MEM_RAM,plus2Lays[rp][1]);
		memSetBank(comp->mem,MEM_BANK2,MEM_RAM,plus2Lays[rp][2]);
		memSetBank(comp->mem,MEM_BANK3,MEM_RAM,plus2Lays[rp][3]);
	} else {
		// normal mem mode
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->rom ? 1 : 0) | ((comp->p1FFD & 0x04) >> 1));
		memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
		memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
		memSetBank(comp->mem,MEM_BANK3,MEM_RAM,comp->p7FFD & 7);
	}
}

// out

/*
void p2OutFE(ZXComp* comp, unsigned short port, unsigned char val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}
*/

void p2Out1FFD(Computer* comp, unsigned short port, unsigned char val) {
	comp->p1FFD = val;
	pl2MapMem(comp);
}

void p2Out7FFD(Computer* comp, unsigned short port, unsigned char val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	pl2MapMem(comp);
}

xPort p2PortMap[] = {
	{0x0003,0x00fe,2,2,2,xInFE,	xOutFE},
	{0xc002,0x7ffd,2,2,2,NULL,	p2Out7FFD},
	{0xf002,0x1ffd,2,2,2,NULL,	p2Out1FFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

void pl2Out(Computer* comp, unsigned short port, unsigned char val, int dos) {
	hwOut(p2PortMap, comp, port, val, 0);
}

unsigned char pl2In(Computer* comp, unsigned short port, int bdiz) {
	return hwIn(p2PortMap, comp, port, 0);
}
