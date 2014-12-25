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

unsigned char ZSLays[4][4] = {
	{0,1,2,3},
	{3,3,3,2},
	{2,2,0,1},
	{1,0,1,0}
};

void scoMapMem(ZXComp* comp) {
	int rp;
	if (comp->prt1 & 0x01) {
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0);
	} else {
		rp = (comp->prt1 & 0x02) ? 2 : ((comp->dosen ? 2 : 0) | ((comp->prt0 & 0x10) ? 1 : 0));
		rp |= ((comp->prt2 & 3) << 2);
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,rp);
	}
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt1 & 0x10) >> 1) | ((comp->prt1 & 0xc0) >> 2));
}

Z80EX_BYTE scoMRd(ZXComp* comp, Z80EX_WORD adr, int m1) {
	if (((comp->mem->pt[0]->num & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3];
		comp->hw->mapMem(comp);
	}
	return stdMRd(comp,adr,m1);
}

// in

Z80EX_BYTE scrpIn1F(ZXComp* comp, Z80EX_WORD port) {
	return joyInput(comp->joy);
}

Z80EX_BYTE scrpInFE(ZXComp* comp, Z80EX_WORD port) {
	return keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->levPlay ? 0x40 : 0x00);
}

Z80EX_BYTE scrpInFFFD(ZXComp* comp, Z80EX_WORD port) {
	return tsIn(comp->ts, 0xfffd);
}

Z80EX_BYTE scrpIn1FFD(ZXComp* comp, Z80EX_WORD port) {
	zxSetFrq(comp, 3.5);
	return 0xff;
}

Z80EX_BYTE scrpIn7FFD(ZXComp* comp, Z80EX_WORD port) {
	zxSetFrq(comp, 7.0);
	return 0xff;
}

Z80EX_BYTE scrpInMice(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return (port & 0x400) ? comp->mouse->ypos : ((port & 0x100) ? comp->mouse->xpos : comp->mouse->buttons);
}

// out

void scrpOutDD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	sdrvOut(comp->sdrv, 0xfb, val);
}

void scrpOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void scrpOutBFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xbffd, val);
}

void scrpOutFFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xfffd, val);
}

void scrpOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt0 & 0x20) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	scoMapMem(comp);
}

void scrpOut1FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->prt1 = val;
	scoMapMem(comp);
}

xPort scrpPortMap[] = {
	{0x0023,0x00fe,0,0,&scrpInFE,	&scrpOutFE},	// !dos cuz of SMUC
	{0xc023,0x1ffd,1,0,&scrpIn1FFD,	&scrpOut1FFD},	// mem
	{0xc023,0x7ffd,1,0,&scrpIn7FFD,	&scrpOut7FFD},
	{0xc023,0xbffd,1,0,NULL,	&scrpOutBFFD},	// ay
	{0xc023,0xfffd,1,0,&scrpInFFFD,	&scrpOutFFFD},
	{0x0023,0x00dd,1,0,NULL,	&scrpOutDD},	// covox
	{0x00ff,0x001f,0,0,&scrpIn1F,	NULL},		// kjoy
	{0x0523,0xfadf,0,0,&scrpInMice,	NULL},		// kmouse
	{0x0523,0xfbdf,0,0,&scrpInMice,	NULL},
	{0x0523,0xffdf,0,0,&scrpInMice,	NULL},
	{0x0000,0x0000,1,0,NULL,NULL}
};

void scoOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	bdiOut(comp->bdi, port, val, dos);
	hwOut(scrpPortMap, comp, port, val, dos);
}

Z80EX_BYTE scoIn(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (bdiIn(comp->bdi, port, &res, dos)) return res;
	res = hwIn(scrpPortMap, comp, port, dos);
	return res;
}
