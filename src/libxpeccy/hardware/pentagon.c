#include "../spectrum.h"

void penMapMem(ZXComp* comp) {
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((comp->dosen & 1) << 1) | ((comp->prt0 & 0x10) >> 4));
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt0 & 0xc0) >> 3));
}

int penGetPort(Z80EX_WORD port, int bdiz) {
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
		if ((port & 0x00ff) == 0x001f) return 0x1f;
		if ((port & 0x05a3) == 0x0083) return 0xfadf;
		if ((port & 0x05a3) == 0x0183) return 0xfbdf;
		if ((port & 0x05a3) == 0x0583) return 0xffdf;
	}
	return 0;
}

void penOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int ptype = penGetPort(port,bdiz);
	sdrvOut(comp->sdrv,port & 0x00ff,val);
	if ((port & 0x0001) == 0x0000) {
		comp->vid->nextbrd = val & 0x07;
		if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
		comp->beeplev = (val & 0x10) ? 1 : 0;
		comp->tape->outsig = (val & 0x08) ? 1 : 0;
	}
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
			break;
		case 0x7ffd:
			if (comp->prt0 & 0x20) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 7 : 5;
			penMapMem(comp);
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val);
			break;
	}
}

Z80EX_BYTE penIn(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = penGetPort(port,bdiz);
	switch (ptype) {
		case 0x1f:
			res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : joyInput(comp->joy);
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
			comp->mouse->used = 1;
			res = comp->mouse->enable ? comp->mouse->buttons : 0xff;
			break;
		case 0xfbdf:
			comp->mouse->used = 1;
			res = comp->mouse->enable ? comp->mouse->xpos : 0xff;
			break;
		case 0xffdf:
			comp->mouse->used = 1;
			res = comp->mouse->enable ? comp->mouse->ypos : 0xff;
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
	}
	return res;
}
