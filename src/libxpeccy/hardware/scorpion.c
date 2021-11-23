#include "../spectrum.h"

/*
ProfROM switch:
page 2,6,10,14
PC = E4B5 : ld l,(hl)
HL = 0110..0113
ProfROM table :
 adr | 0 1 2 3 <- current layer
-----+---------
0110 | 0 1 2 3 <- result layers
0111 | 3 3 3 2
0112 | 2 2 0 1
0113 | 1 0 1 0
*/

static int ZSLays[4][4] = {
	{0,1,2,3},
	{3,3,3,2},
	{2,2,0,1},
	{1,0,1,0}
};

void scoMapMem(Computer* comp) {
	int rp;
	if (comp->p1FFD & 0x01) {
		memSetBank(comp->mem,0x00,MEM_RAM,0, MEM_16K, NULL, NULL, NULL);
	} else {
		rp = (comp->p1FFD & 0x02) ? 2 : ((comp->dos ? 2 : 0) | ((comp->rom) ? 1 : 0));
		rp |= ((comp->prt2 & 3) << 2);
		memSetBank(comp->mem,0x00,MEM_ROM,rp, MEM_16K, NULL, NULL, NULL);
	}
	rp = (comp->p7FFD & 7) | ((comp->p1FFD & 0x10) >> 1) | ((comp->p1FFD & 0xc0) >> 2);
	memSetBank(comp->mem,0x40,MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem,0x80,MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem,0xc0,MEM_RAM, rp, MEM_16K, NULL, NULL, NULL);
}

int scoMRd(Computer* comp, int adr, int m1) {
	if ((comp->p1FFD & 2) && ((adr & 0xfff3) == 0x0100) && !m1) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3] & 0xff;
		comp->hw->mapMem(comp);
	}
	return stdMRd(comp,adr,m1);
}

// in

int scrpIn1F(Computer* comp, int port) {
	return joyInput(comp->joy);
}

int scrpIn1FFD(Computer* comp, int port) {
	compSetTurbo(comp, 1);
	return 0xff;
}

int scrpIn7FFD(Computer* comp, int port) {
	compSetTurbo(comp, 2);
	return 0xff;
}

int scrpInFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

// out

void scrpOutDD(Computer* comp, int port, int val) {
	sdrvWrite(comp->sdrv, 0xfb, val);
}

void scrpOut7FFD(Computer* comp, int port, int val) {
	if (comp->p7FFD & 0x20) return;
	comp->p7FFD = val;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	scoMapMem(comp);
}

void scrpOut1FFD(Computer* comp, int port, int val) {
	comp->p1FFD = val;
	comp->ext = (val & 2) ? 1 : 0;
	scoMapMem(comp);
}

static xPort scrpPortMap[] = {
	{0x0023,0x00fe,0,2,2,xInFE,	xOutFE},	// !dos cuz of SMUC
	{0xc023,0x1ffd,2,2,2,scrpIn1FFD,scrpOut1FFD},	// mem
	{0xc023,0x7ffd,2,2,2,scrpIn7FFD,scrpOut7FFD},
	{0xc023,0xbffd,2,2,2,NULL,	xOutBFFD},	// ay
	{0xc023,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x0023,0x00dd,2,2,2,NULL,	scrpOutDD},	// covox
	{0x00ff,0x001f,0,2,2,scrpIn1F,	NULL},		// kjoy
	{0x0523,0xfadf,0,2,2,xInFADF,	NULL},		// kmouse
	{0x0523,0xfbdf,0,2,2,xInFBDF,	NULL},
	{0x0523,0xffdf,0,2,2,xInFFDF,	NULL},
	{0x0000,0x0000,2,2,2,scrpInFF,NULL}
};

void scoOut(Computer* comp, int port, int val) {
	difOut(comp->dif, port, val, comp->bdiz);
	zx_dev_wr(comp, port, val);
	hwOut(scrpPortMap, comp, port, val, 1);
}

int scoIn(Computer* comp, int port) {
	int res = -1;
	if (difIn(comp->dif, port, &res, comp->bdiz)) return res;
	if (zx_dev_rd(comp, port, &res)) return res;
	res = hwIn(scrpPortMap, comp, port);
	return res;
}
