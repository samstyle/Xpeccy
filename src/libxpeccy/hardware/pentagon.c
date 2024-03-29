#include "../spectrum.h"

void penMapMem(Computer* comp) {
	int pg = (comp->dos ? 2 : 0) | ((comp->rom) ? 1 : 0);
	memSetBank(comp->mem, 0x00, MEM_ROM, pg, MEM_16K, NULL, NULL, NULL);
	pg = (comp->p7FFD & 7) | ((comp->p7FFD & 0xc0) >> 3);
	memSetBank(comp->mem, 0x40, MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xc0, MEM_RAM, pg, MEM_16K, NULL, NULL, NULL);
}

// in

int penInFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

// out

void penOut7FFD(Computer* comp, int port, int val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	penMapMem(comp);
}

static xPort penPortMap[] = {
	{0x0001,0x00fe,2,2,2,xInFE,	xOutFE},
	{0x8002,0x7ffd,2,2,2,NULL,	penOut7FFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x00ff,0x001f,0,2,2,xIn1F,	NULL},		// joystick
	{0x05a3,0xfadf,0,2,2,xInFADF,	NULL},		// mouse
	{0x05a3,0xfbdf,0,2,2,xInFBDF,	NULL},
	{0x05a3,0xffdf,0,2,2,xInFFDF,	NULL},
	{0x0000,0x0000,2,2,2,penInFF,	NULL}
};

void penOut(Computer* comp, int port, int val) {
	difOut(comp->dif, port, val, comp->bdiz);
	zx_dev_wr(comp, port, val);
	hwOut(penPortMap, comp, port, val, 1);
}

int penIn(Computer* comp, int port) {
	int res = -1;
	if (difIn(comp->dif, port, &res, comp->bdiz)) return res;
	if (zx_dev_rd(comp, port, &res)) return res;
	res = hwIn(penPortMap, comp, port);
	return res;
}
