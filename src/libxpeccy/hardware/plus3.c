#include "../spectrum.h"

int pl3GetPort(Z80EX_WORD port, int bdiz) {
	if ((port & 0x0003) == 0x0002) return 0xfe;
	if ((port & 0xc002) == 0x4000) return 0x7ffd;
	if ((port & 0xf002) == 0x1000) return 0x1ffd;
	if ((port & 0xf002) == 0x0000) return 0x0ffd;
	if ((port & 0xf002) == 0x2000) return 0x2ffd;
	if ((port & 0xf002) == 0x3000) return 0x3ffd;
	if ((port & 0xc002) == 0x8000) return 0xbffd;
	if ((port & 0xc002) == 0xc000) return 0xfffd;
	return 0;
}

void pl3Out(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int ptype = pl3GetPort(port,bdiz);
	switch (ptype) {
		case 0xfe:
			comp->vid->nextbrd = val & 0x07;
			if (!comp->vid->border4t)
				comp->vid->brdcol = val & 0x07;
			comp->beeplev = val & 0x10;
			comp->tape->outsig = (val & 0x08) ? 1 : 0;
			break;
		case 0x7ffd:
			if (comp->prt0 & 0x20) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 7 : 5;
			pl2MapMem(comp);
			break;
		case 0x1ffd:
			comp->prt1 = val;
			pl2MapMem(comp);
			break;
		case 0x2ffd:
			if (comp->bdi->fdc->type != FDC_765) break;
			comp->bdi->fdc->flop[0]->motor = (val & 1) ? 1 : 0;
			comp->bdi->fdc->flop[1]->motor = (val & 2) ? 1 : 0;
			comp->bdi->fdc->flop[2]->motor = (val & 4) ? 1 : 0;
			comp->bdi->fdc->flop[3]->motor = (val & 8) ? 1 : 0;
			break;
		case 0x3ffd:
			if (comp->bdi->fdc->type == FDC_765)
				fdcWr(comp->bdi->fdc,FDC_DATA,val);
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val);
			break;
	}
}

Z80EX_BYTE pl3In(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = pl3GetPort(port,bdiz);
	switch (ptype) {
		case 0xfe:
			res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
			break;
		case 0x2ffd:
			res = fdcRd(comp->bdi->fdc,FDC_STATE);
			break;
		case 0x3ffd:
			res = fdcRd(comp->bdi->fdc,FDC_DATA);
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
	}
	return res;
}
