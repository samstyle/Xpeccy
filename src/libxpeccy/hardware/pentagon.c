#include "../spectrum.h"

void penMapMem(ZXComp* comp) {
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen ? 2 : 0) | ((comp->prt0 & 0x10) ? 1 : 0));
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt0 & 0xc0) >> 3));
}

// in

Z80EX_BYTE penIn1F(ZXComp* comp, Z80EX_WORD port) {
	return joyInput(comp->joy);
}

Z80EX_BYTE penInFE(ZXComp* comp, Z80EX_WORD port) {
	return keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->levPlay ? 0x40 : 0x00);
}

Z80EX_BYTE penInFFFD(ZXComp* comp, Z80EX_WORD port) {
	return tsIn(comp->ts, 0xfffd);
}

Z80EX_BYTE penInFADF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->buttons : 0xff;
}

Z80EX_BYTE penInFBDF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->xpos : 0xff;
}

Z80EX_BYTE penInFFDF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->ypos : 0xff;
}

// out

void penOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void penOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt0 & 0x20) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	penMapMem(comp);
}

void penOutBFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xbffd, val);
}

void penOutFFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xfffd, val);
}

xPort penPortMap[] = {
	{0x0001,0x00fe,1,0,&penInFE,	&penOutFE},
	{0x8002,0x7ffd,1,0,NULL,	&penOut7FFD},
	{0xc002,0xbffd,1,0,NULL,	&penOutBFFD},
	{0xc002,0xfffd,1,0,&penInFFFD,	&penOutFFFD},
	{0x00ff,0x001f,0,0,&penIn1F,	NULL},		// joystick
	{0x05a3,0xfadf,0,0,&penInFADF,	NULL},		// mouse
	{0x05a3,0xfbdf,0,0,&penInFBDF,	NULL},
	{0x05a3,0xffdf,0,0,&penInFFDF,	NULL},
	{0x0000,0x0000,1,0,NULL,	NULL}		// end
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
