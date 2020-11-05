#include "../spectrum.h"

// in

int p3InFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

// out

void p3Out1FFD(Computer* comp, int port, int val) {
	comp->p1FFD = val & 0xff;
	comp->dif->fdc->flp->motor = (val & 8) ? 1 : 0;
	pl2MapMem(comp);
}

void p3Out7FFD(Computer* comp, int port, int val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	pl2MapMem(comp);
}

static xPort p3PortMap[] = {
	{0xc002,0x7ffd,2,2,2,NULL,	p3Out7FFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0xf002,0x1ffd,2,2,2,NULL,	p3Out1FFD},
	{0x0003,0x00fe,2,2,2,xInFE,	xOutFE},
	{0x0000,0x0000,2,2,2,p3InFF,	NULL}
};

void pl3Out(Computer* comp, int port, int val, int dos) {
	difOut(comp->dif, port, val, 0);
	zx_dev_wr(comp, port, val, dos);
	hwOut(p3PortMap, comp, port, val, 0);
}

int pl3In(Computer* comp, int port, int dos) {
	int res = -1;
	if (difIn(comp->dif, port, &res, 0)) return res;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	res = hwIn(p3PortMap, comp, port, 0);
	return res;
}
