#include "../spectrum.h"

void p1mMapMem(Computer* comp) {
	int pg;
	if (comp->pEFF7 & 8) {
		memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_16K, NULL, NULL, NULL);
	} else {
		pg = (comp->dos ? 2 : 0) | ((comp->rom) ? 1 : 0);
		memSetBank(comp->mem, 0x00, MEM_ROM, pg, MEM_16K, NULL, NULL, NULL);
	}
	pg = (comp->p7FFD & 7) | ((comp->pEFF7 & 4) ? 0 : ((comp->p7FFD & 0x20) | ((comp->p7FFD & 0xc0) >> 3)));
	memSetBank(comp->mem, 0x40, MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xc0, MEM_RAM, pg, MEM_16K, NULL, NULL, NULL);
}

// in

int p1mIn1F(Computer* comp, int port) {
	return joyInput(comp->joy);
}

int p1mInBFF7(Computer* comp, int port) {
	return (comp->pEFF7 & 0x80) ? cmsRd(comp) : 0xff;
}

int p1mInFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

// out

void p1mOut7FFD(Computer* comp, int port, int val) {
	if ((comp->pEFF7 & 4) && (comp->p7FFD & 0x20)) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	p1mMapMem(comp);
}

void p1mOutBFF7(Computer* comp, int port, int val) {
	if (comp->pEFF7 & 0x80)
		cmsWr(comp,val);
}

void p1mOutDFF7(Computer* comp, int port, int val) {
	if (comp->pEFF7 & 0x80)
		cmos_wr(&comp->cmos, CMOS_ADR, val);
}

void p1mOutEFF7(Computer* comp, int port, int val) {
	comp->pEFF7 = val & 0xff;
	vid_set_mode(comp->vid,(val & 0x01) ? VID_ALCO : VID_NORMAL);
	compSetTurbo(comp, (val & 0x10) ? 1.0 : 2.0);
	p1mMapMem(comp);
}

static xPort p1mPortMap[] = {
	{0x0003,0x00fe,2,2,2,xInFE,	xOutFE},
	{0x8002,0x7ffd,2,2,2,NULL,	p1mOut7FFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0xf008,0xeff7,2,2,2,NULL,	p1mOutEFF7},
	{0xf008,0xbff7,2,2,2,p1mInBFF7,	p1mOutBFF7},
	{0xf008,0xdff7,2,2,2,NULL,	p1mOutDFF7},
	{0x05a3,0xfadf,0,2,2,xInFADF,	NULL},
	{0x05a3,0xfbdf,0,2,2,xInFBDF,	NULL},
	{0x05a3,0xffdf,0,2,2,xInFFDF,	NULL},
	{0x00ff,0x001f,0,2,2,p1mIn1F,	NULL},		// TODO : ORLY (x & FF = 1F)
	{0x0000,0x0000,2,2,2,p1mInFF,	NULL}
};

void p1mOut(Computer* comp, int port, int val) {
	zx_dev_wr(comp, port, val);
	difOut(comp->dif, port, val, comp->bdiz);
	hwOut(p1mPortMap, comp, port, val, 1);
}

int p1mIn(Computer* comp, int port) {
	int res = -1;
	if (difIn(comp->dif, port, &res, comp->bdiz)) return res;
	if (zx_dev_rd(comp, port, &res)) return res;
	res = hwIn(p1mPortMap, comp, port);
	return res;
}
