#include "../spectrum.h"

void penMapMem(ZXComp* comp) {
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen ? 2 : 0) | ((comp->prt0 & 0x10) ? 1 : 0));
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt0 & 0xc0) >> 3));
}

// in

Z80EX_BYTE penIn1F(ZXComp* comp, Z80EX_WORD port) {
	return joyInput(comp->joy);
}

// out

void penOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt0 & 0x20) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	penMapMem(comp);
}

xPort penPortMap[] = {
	{0x0001,0x00fe,2,&xInFE,	&xOutFE},
	{0x8002,0x7ffd,2,NULL,		&penOut7FFD},
	{0xc002,0xbffd,2,NULL,		&xOutBFFD},
	{0xc002,0xfffd,2,&xInFFFD,	&xOutFFFD},
	{0x00ff,0x001f,0,&penIn1F,	NULL},		// joystick
	{0x05a3,0xfadf,0,&xInFADF,	NULL},		// mouse
	{0x05a3,0xfbdf,0,&xInFBDF,	NULL},
	{0x05a3,0xffdf,0,&xInFFDF,	NULL},
	{0x0000,0x0000,2,NULL, NULL}			// end
};

void penOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	difOut(comp->dif, port, val, dos);
	sdrvOut(comp->sdrv,port & 0x00ff,val);
	hwOut(penPortMap, comp, port, val, dos);
}

Z80EX_BYTE penIn(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(penPortMap, comp, port, dos);
	return res;
}
