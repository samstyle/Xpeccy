#include "../spectrum.h"

void p1mMapMem(ZXComp* comp) {
	if (comp->prt1 & 8) {
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0);
	} else {
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((comp->dosen & 1) << 1) | ((comp->prt0 & 0x10) >> 4));
	}
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt1 & 4) ? 0 : ((comp->prt0 & 0x20) | ((comp->prt0 & 0xc0) >> 3))));
}

int p1mGetPort(Z80EX_WORD port, int bdiz) {
	if ((port & 0x0003) == 0x0002) return 0xfe;
	if ((port & 0x8002) == 0x0000) return 0x7ffd;
	if ((port & 0xc002) == 0x8000) return 0xbffd;
	if ((port & 0xc002) == 0xc000) return 0xfffd;
	if (bdiz) {
		if ((port & 0x0083) == 0x0083) return 0xff;
		if ((port & 0x00e3) == 0x0003) return 0x1f;
		if ((port & 0x00e3) == 0x0023) return 0x3f;
		if ((port & 0x00e3) == 0x0043) return 0x5f;
		if ((port & 0x00e3) == 0x0063) return 0x7f;
	} else {
		if ((port & 0x05a3) == 0x0083) return 0xfadf;
		if ((port & 0x05a3) == 0x0183) return 0xfbdf;
		if ((port & 0x05a3) == 0x0583) return 0xffdf;
	}
	if ((port & 0xf008) == 0xe000) return 0xeff7;
	if ((port & 0xf008) == 0xb000) return 0xbff7;	// cmos
	if ((port & 0xf008) == 0xd000) return 0xdff7;
	return 0;
}

void p1mOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int ptype = p1mGetPort(port,bdiz);
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
		case 0xfe:
			comp->vid->nextbrd = val & 0x07;
			if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
			comp->beeplev = (val & 0x10) ? 1 : 0;
			comp->tape->outsig = (val & 0x08) ? 1 : 0;
			break;
		case 0x7ffd:
			if ((comp->prt1 & 4) && (comp->prt0 & 0x20)) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 7 : 5;
			p1mMapMem(comp);
			break;
		case 0xeff7:
			comp->prt1 = val;
			vidSetMode(comp->vid,(val & 0x01) ? VID_ALCO : VID_NORMAL);
			zxSetFrq(comp, (val & 0x10) ? 7.0 : 3.5);
			p1mMapMem(comp);
			break;
		case 0xbff7:
			if (comp->prt1 & 0x80) cmsWr(comp,val);
			break;
		case 0xdff7:
			if (comp->prt1 & 0x80) comp->cmos.adr = val;
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val);
			break;
	}
}

Z80EX_BYTE p1mIn(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = p1mGetPort(port,bdiz);
	switch (ptype) {
		case 0x1f:
			res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xff;
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
		case 0xfadf:
			res = comp->mouse->enable ? comp->mouse->buttons : 0xff;
			break;
		case 0xfbdf:
			res = comp->mouse->enable ? comp->mouse->xpos : 0xff;
			break;
		case 0xffdf:
			res = comp->mouse->enable ? comp->mouse->ypos : 0xff;
			break;
		case 0xbff7:
			res = (comp->prt1 & 0x80) ? cmsRd(comp) : 0xff;
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
	}
	return res;
}
