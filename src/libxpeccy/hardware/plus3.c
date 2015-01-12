#include "../spectrum.h"

// in

Z80EX_BYTE p3InFE(ZXComp* comp, Z80EX_WORD port) {
	return keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->levPlay ? 0x40 : 0x00);
}

Z80EX_BYTE p3InFFFD(ZXComp* comp, Z80EX_WORD port) {
	return tsIn(comp->ts, 0xfffd);
}

// out

void p3OutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t)
		comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void p3Out1FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->prt1 = val;
	comp->dif->fdc->flp->motor = (val & 8) ? 1 : 0;
	pl2MapMem(comp);
}

void p3Out7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt0 & 0x20) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	pl2MapMem(comp);
}

void p3OutBFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xbffd, val);
}

void p3OutFFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xfffd, val);
}

xPort p3PortMap[] = {
	{0x0003,0x00fe,1,0,&p3InFE,	&p3OutFE},
	{0xc002,0x7ffd,1,0,NULL,	&p3Out7FFD},
	{0xf002,0x1ffd,1,0,NULL,	&p3Out1FFD},
	{0xc002,0xbffd,1,0,NULL,	&p3OutBFFD},
	{0xc002,0xfffd,1,0,&p3InFFFD,	&p3OutFFFD},
	{0x0000,0x0000,1,0,NULL,	NULL}
};

void pl3Out(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	difOut(comp->dif, port, val, 0);
	hwOut(p3PortMap, comp, port, val, 0);
}

Z80EX_BYTE pl3In(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (difIn(comp->dif, port, &res, 0)) return res;
	res = hwIn(p3PortMap, comp, port, 0);
	return res;
}
