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
		memSetBank(comp->mem,0x00,MEM_RAM,plus2Lays[rp][0], MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x40,MEM_RAM,plus2Lays[rp][1], MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x80,MEM_RAM,plus2Lays[rp][2], MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0xc0,MEM_RAM,plus2Lays[rp][3], MEM_16K, NULL, NULL, NULL);
	} else {
		// normal mem mode
		memSetBank(comp->mem,0x00,MEM_ROM,(comp->rom ? 1 : 0) | ((comp->p1FFD & 0x04) >> 1), MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x40,MEM_RAM,5, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x80,MEM_RAM,2, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0xc0,MEM_RAM,comp->p7FFD & 7, MEM_16K, NULL, NULL, NULL);
	}
}

void plusRes(Computer* comp) {
	comp->p1FFD = 0;
	comp->p7FFD = 0;
	comp->rom = 0;
}

// in

int p2InFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

// no upd765 in +2
int p2_dos_rd(Computer* comp, int port) {
	return -1;
//	int res = -1;
//	difIn(comp->dif, port, &res, 0);
//	return res;
}

// out

void p2Out1FFD(Computer* comp, int port, int val) {
	comp->p1FFD = val & 0xff;
	pl2MapMem(comp);
}

void p2_dos_wr(Computer* comp, int port, int val) {
//	difOut(comp->dif, port, val, 0);
}

void p2Out7FFD(Computer* comp, int port, int val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	pl2MapMem(comp);
}

static xPort p2PortMap[] = {
	{0x0003,0x00fe,2,2,2,xInFE,	xOutFE},
	{0xc002,0x7ffd,2,2,2,NULL,	p2Out7FFD},
	{0xf002,0x1ffd,2,2,2,NULL,	p2Out1FFD},
	{0xe002,0x2ffd,2,2,2,p2_dos_rd,	p2_dos_wr},	// 2ffd/3ffd
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x0000,0x0000,2,2,2,p2InFF,	NULL}
};

void pl2Out(Computer* comp, int port, int val, int dos) {
	zx_dev_wr(comp, port, val, dos);
	hwOut(p2PortMap, comp, port, val, 0);
}

int pl2In(Computer* comp, int port, int dos) {
	int res = -1;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	res = hwIn(p2PortMap, comp, port, 0);
	return  res;
}
