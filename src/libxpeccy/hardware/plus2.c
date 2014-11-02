#include "../spectrum.h"

// layout for +2a extend memory mode

unsigned char plus2Lays[4][4] = {
	{0,1,2,3},
	{4,5,6,7},
	{4,5,6,3},
	{4,7,6,3}
};

void pl2MapMem(ZXComp* comp) {
	if (comp->prt1 & 1) {
		// extend mem mode
		int rp = ((comp->prt1 & 0x06) >> 1);	// b1,2 of 1ffd
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,plus2Lays[rp][0]);
		memSetBank(comp->mem,MEM_BANK1,MEM_RAM,plus2Lays[rp][1]);
		memSetBank(comp->mem,MEM_BANK2,MEM_RAM,plus2Lays[rp][2]);
		memSetBank(comp->mem,MEM_BANK3,MEM_RAM,plus2Lays[rp][3]);
	} else {
		// normal mem mode
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((comp->prt0 & 0x10) >> 4) | ((comp->prt1 & 0x04) >> 1));
		memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
		memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
		memSetBank(comp->mem,MEM_BANK3,MEM_RAM,comp->prt0 & 7);
	}
}

int pl2GetPort(Z80EX_WORD port, int bdiz) {
	if ((port & 0x0003) == 0x0002) return 0xfe;
	if ((port & 0xc002) == 0x4000) return 0x7ffd;
	if ((port & 0xf002) == 0x1000) return 0x1ffd;
	if ((port & 0xc002) == 0x8000) return 0xbffd;
	if ((port & 0xc002) == 0xc000) return 0xfffd;
	return 0;
}

void pl2Out(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int ptype = pl2GetPort(port,bdiz);
	switch (ptype) {
		case 0xfe:
			comp->vid->nextbrd = val & 0x07;
			if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
			comp->beeplev = (val & 0x10) ? 1 : 0;
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
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val);
			break;
	}
}

Z80EX_BYTE pl2In(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = pl2GetPort(port,bdiz);
	switch (ptype) {
		case 0xfe:
			res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
	}
	return res;
}
