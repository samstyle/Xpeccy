#include "../spectrum.h"

void speReset(ZXComp* comp) {
	comp->prt0 = 0x10;
}

void speMapMem(ZXComp* comp) {
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen) ? 1 : 0);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
}

// out

void spOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t)
		comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

// in

Z80EX_BYTE spIn1F(ZXComp* comp, Z80EX_WORD port) {
	return joyInput(comp->joy);
}

Z80EX_BYTE spInFF(ZXComp* comp, Z80EX_WORD port) {
	return comp->vid->atrbyte;
}

xPort spePortMap[] = {
	{0x0001, 0x00fe, 2, &xInFE, &spOutFE},
	{0xc002, 0xfffd, 2, &xInFFFD, &xOutFFFD},
	{0xc002, 0xbffd, 2, NULL, &xOutBFFD},
	{0x0320, 0xfadf, 2, &xInFADF, NULL},
	{0x0720, 0xfbdf, 2, &xInFBDF, NULL},
	{0x0720, 0xffdf, 2, &xInFFDF, NULL},
	{0x0021, 0x001f, 0, &spIn1F, NULL},
	{0x0000, 0x0000, 0, &spInFF, NULL}		// all unknown ports is FF (nodos)
};

void speOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	difOut(comp->dif, port, val, dos);
	hwOut(spePortMap, comp, port, val, dos);
}

Z80EX_BYTE speIn(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res;
	if (difIn(comp->dif, port, &res, dos)) return res;
	return hwIn(spePortMap, comp, port, dos);
}
