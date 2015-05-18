#include "../spectrum.h"

// layout for +2a extend memory mode

unsigned char plus2Lays[4][4] = {
	{0,1,2,3},
	{4,5,6,7},
	{4,5,6,3},
	{4,7,6,3}
};

void pl2MapMem(ZXComp* comp) {
	if (comp->prt1 & 1) {
		// extend mem mode
		int rp = ((comp->prt1 & 0x06) >> 1);	// b1,2 of 1ffd
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,plus2Lays[rp][0]);
		memSetBank(comp->mem,MEM_BANK1,MEM_RAM,plus2Lays[rp][1]);
		memSetBank(comp->mem,MEM_BANK2,MEM_RAM,plus2Lays[rp][2]);
		memSetBank(comp->mem,MEM_BANK3,MEM_RAM,plus2Lays[rp][3]);
	} else {
		// normal mem mode
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((comp->prt0 & 0x10) >> 4) | ((comp->prt1 & 0x04) >> 1));
		memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
		memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
		memSetBank(comp->mem,MEM_BANK3,MEM_RAM,comp->prt0 & 7);
	}
}

// out

/*
void p2OutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}
*/

void p2Out1FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->prt1 = val;
	pl2MapMem(comp);
}

void p2Out7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt0 & 0x20) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	pl2MapMem(comp);
}

xPort p2PortMap[] = {
	{0x0003,0x00fe,2,&xInFE,	&xOutFE},
	{0xc002,0x7ffd,2,NULL,		&p2Out7FFD},
	{0xf002,0x1ffd,2,NULL,		&p2Out1FFD},
	{0xc002,0xbffd,2,NULL,		&xOutBFFD},
	{0xc002,0xfffd,2,&xInFFFD,	&xOutFFFD},
	{0x0000,0x0000,2,NULL, NULL}
};

void pl2Out(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	hwOut(p2PortMap, comp, port, val, 0);
}

Z80EX_BYTE pl2In(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	return hwIn(p2PortMap, comp, port, 0);
}
