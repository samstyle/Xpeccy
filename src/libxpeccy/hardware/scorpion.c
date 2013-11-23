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
		rp = (comp->prt1 & 0x02) ? 2 : (((comp->dosen & 1) << 1) | ((comp->prt0 & 0x10) >> 4));
		rp |= ((comp->prt2 & 3) << 2);
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,rp);
	}
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt1 & 0x10) >> 1) | ((comp->prt1 & 0xc0) >> 2));
}

Z80EX_BYTE scoMRd(ZXComp* comp, Z80EX_WORD adr, int m1) {
	if (((comp->mem->pt0->num & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3];
		comp->hw->mapMem(comp);
	}
	return stdMRd(comp,adr,m1);
}

int scoGetPort(Z80EX_WORD port, int bdiz) {
	if ((port & 0x0023) == 0x0022) return 0xfe;
	if ((port & 0xc023) == 0x0021) return 0x1ffd;
	if ((port & 0xc023) == 0x4021) return 0x7ffd;
	if ((port & 0xc023) == 0x8021) return 0xbffd;
	if ((port & 0xc023) == 0xc021) return 0xfffd;
	if ((port & 0x0023) == 0x0001) return 0xdd;
	if (bdiz) {
		if ((port & 0x0083) == 0x0083) return 0xff;
		if ((port & 0x00e3) == 0x0003) return 0x1f;
		if ((port & 0x00e3) == 0x0023) return 0x3f;
		if ((port & 0x00e3) == 0x0043) return 0x5f;
		if ((port & 0x00e3) == 0x0063) return 0x7f;
	} else {
		if ((port & 0x00ff) == 0x001f) return 0x1f;
		if ((port & 0x0523) == 0x0003) return 0xfadf;
		if ((port & 0x0523) == 0x0103) return 0xfbdf;
		if ((port & 0x0523) == 0x0503) return 0xffdf;
	}
	return 0;
}

void scoOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int ptype = scoGetPort(port,bdiz);
	switch (ptype) {
		case 0x1f:
			if (bdiz) bdiOut(comp->bdi,FDC_COM,val);
			break;
		case 0x3f:
			if (bdiz) bdiOut(comp->bdi,FDC_TRK,val);
			break;
		case 0x5f:
			if (bdiz) bdiOut(comp->bdi,FDC_SEC,val);
			break;
		case 0x7f:
			if (bdiz) bdiOut(comp->bdi,FDC_DATA,val);
			break;
		case 0xff:
			if (bdiz) bdiOut(comp->bdi,BDI_SYS,val);
			break;
		case 0xdd:
			sdrvOut(comp->sdrv,0xfb,val);
			break;
		case 0xfe:
			comp->vid->nextbrd = val & 0x07;
			if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
			comp->beeplev = val & 0x10;
			comp->tape->outsig = (val & 0x08) ? 1 : 0;
			break;
		case 0x7ffd:
			if (comp->prt0 & 0x20) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 1 : 0;
			scoMapMem(comp);
			break;
		case 0x1ffd:
			comp->prt1 = val;
			scoMapMem(comp);
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val);
			break;
	}
}

Z80EX_BYTE scoIn(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = scoGetPort(port,bdiz);
	switch (ptype) {
		case 0x1f:
			res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xe0;
			break;
		case 0x3f:
			res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff;
			break;
		case 0x5f:
			res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff;
			break;
		case 0x7f:
			res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff;
			break;
		case 0xff:
			res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff;
			break;
		case 0xfe:
			res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
			break;
		case 0x1ffd:
			zxSetFrq(comp,3.5);
			break;
		case 0x7ffd:
			zxSetFrq(comp,7.0);
			break;
		case 0xfadf:
			res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->buttons : 0xff;
			break;
		case 0xfbdf:
			res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->xpos : 0xff;
			break;
		case 0xffdf:
			res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->ypos : 0xff;
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
	}
	return res;
}
