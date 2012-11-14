#include "../spectrum.h"

void speMapMem(ZXComp* comp) {
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen) ? 3 : 1);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
}

int speGetPort(Z80EX_WORD port, int bdiz) {
	if ((port & 0x0001) == 0x0000) return 0xfe;
	if (bdiz) {
		if ((port & 0x0083) == 0x0083) return 0xff;
		if ((port & 0x00e3) == 0x0003) return 0x1f;
		if ((port & 0x00e3) == 0x0023) return 0x3f;
		if ((port & 0x00e3) == 0x0043) return 0x5f;
		if ((port & 0x00e3) == 0x0063) return 0x7f;
	} else {
		if ((port & 0x0021) == 0x0001) return 0x1f;
	}
	return 0;
}

void speOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int ptype = speGetPort(port,bdiz);
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
			if (!(comp->vid->flags & VID_BORDER_4T))
				comp->vid->brdcol = val & 0x07;
			comp->beeplev = val & 0x10;
			comp->tape->outsig = (val & 0x08) ? 1 : 0;
			break;
	}
}

Z80EX_BYTE speIn(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = speGetPort(port,bdiz);
	switch (ptype) {
		case 0: res = vidGetAttr(comp->vid); break;
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
	}
	return res;
}
